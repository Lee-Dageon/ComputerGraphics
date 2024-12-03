#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <ctime>

#define WIDTH 600
#define HEIGHT 600
GLfloat rotationX = 0.0f;
GLfloat rotationY = 0.0f;

struct Vertex {
	float x, y, z;
};

class Shader {		//���̴� �ҷ����� Ŭ����
public:
	GLuint programID;

	Shader(const char* vertexFilePath, const char* fragmentFilePath) {
		GLuint vertexShader = CompileShader(vertexFilePath, GL_VERTEX_SHADER);
		GLuint fragmentShader = CompileShader(fragmentFilePath, GL_FRAGMENT_SHADER);
		CreateProgram(vertexShader, fragmentShader);
	}

	void Use() const {
		glUseProgram(programID);
	}

private:
	GLuint CompileShader(const char* filePath, GLenum shaderType) {
		std::string code;
		std::ifstream file(filePath);

		if (!file.is_open()) {
			std::cerr << "Failed to open shader file: " << filePath << std::endl;
			return 0;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		code = buffer.str();
		file.close();

		const char* codeCStr = code.c_str();
		GLuint shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &codeCStr, NULL);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cerr << "ERROR: Shader compilation failed\n" << infoLog << std::endl;
			return 0;
		}

		return shader;
	}

	void CreateProgram(GLuint vertexShader, GLuint fragmentShader) {
		programID = glCreateProgram();
		glAttachShader(programID, vertexShader);
		glAttachShader(programID, fragmentShader);
		glLinkProgram(programID);

		GLint success;
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetProgramInfoLog(programID, 512, NULL, infoLog);
			std::cerr << "ERROR: Program linking failed\n" << infoLog << std::endl;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
};

class Cube {
public:
	std::vector<Vertex> vertices;
	GLuint VAO, VBO, EBO[7], colorVBO;
	std::vector<GLuint> faceIndices[7];  // ���� ���� ���� ���� �ε���

	Cube() {
		DefineVertices();
		DefineFaceIndices();
		InitBuffer();
	}

	void DefineVertices() {
		// ť���� ������ �����մϴ�.
		vertices = {
			{ -0.5f, -0.5f, -0.5f },  // 0: ���� �Ʒ� ��
			{ 0.5f, -0.5f, -0.5f },   // 1: ������ �Ʒ� ��
			{ 0.5f,  0.5f, -0.5f },   // 2: ������ �� ��
			{ -0.5f,  0.5f, -0.5f },  // 3: ���� �� ��
			{ -0.5f, -0.5f,  0.5f },  // 4: ���� �Ʒ� ��
			{ 0.5f, -0.5f,  0.5f },   // 5: ������ �Ʒ� ��
			{ 0.5f,  0.5f,  0.5f },   // 6: ������ �� ��
			{ -0.5f,  0.5f,  0.5f }   // 7: ���� �� ��
		};
	}

	void DefineFaceIndices() {
		// �ո��� ù ��° �ﰢ��
		faceIndices[0] = { 4, 5, 6 };

		// �ո��� �� ��° �ﰢ��
		faceIndices[1] = { 4, 6, 7 };

		// ������ ��
		faceIndices[2] = { 0, 1, 2, 0, 2, 3 };  // �޸�
		faceIndices[3] = { 3, 2, 6, 3, 6, 7 };  // ����
		faceIndices[4] = { 0, 1, 5, 5, 4, 0 };  // �Ʒ���
		faceIndices[5] = { 0, 3, 7, 7, 4, 0 };  // ���� ��
		faceIndices[6] = { 1, 2, 6, 6, 5, 1 };  // ������ ��
	}


	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(7, EBO);  // ���� ���� EBO ����
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// VBO ����
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// ���� VBO ����
		GLfloat colors[] = {
			1.0f, 0.0f, 0.0f, // ����
			0.0f, 1.0f, 0.0f, // �ʷ�
			0.0f, 0.0f, 1.0f, // �Ķ�
			1.0f, 1.0f, 0.0f, // ���
			1.0f, 0.0f, 1.0f, // ��ȫ
			0.0f, 1.0f, 1.0f  // û��
		};
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		// �� �鿡 ���� EBO ����
		for (int i = 0; i < 7; i++) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices[i].size() * sizeof(GLuint), faceIndices[i].data(), GL_STATIC_DRAW);
		}

		glBindVertexArray(0);
	}

	void RenderFace(int faceIndex) {
		if (faceIndex < 0 || faceIndex >= 8) {
			std::cerr << "Invalid face index: " << faceIndex << std::endl;
			return;
		}

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[faceIndex]);
		glDrawElements(GL_TRIANGLES, faceIndices[faceIndex].size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

class Axis {		//�� �ҷ����� Ŭ����
public:
	GLuint VAO, VBO;

	Axis() {
		InitBuffer();
	}

	void InitBuffer() {
		GLfloat axisVertices[] = {
			-1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 0.0f, -1.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 0.0f, 1.0f, 0.0f,
			 0.0f, 0.0f, 0.0f
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void Render() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 4);
		glBindVertexArray(0);
	}
};		//�� �ҷ����� Ŭ����



Cube* cube;
Axis* axis;
Shader* shader;


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();


	// ���� ��� ���� (���� ����)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	unsigned int projectionLocation = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// �� ��� ���� (ī�޶� ��ġ)
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 1.0f),  // ī�޶� ��ġ
		glm::vec3(0.0f, 0.0f, 0.0f),  // ī�޶� �ٶ󺸴� ����
		glm::vec3(0.0f, 1.0f, 0.0f)   // ����� ���� (y��)
	);
	unsigned int viewLocation = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));



	// �� ��� ���� (ť��)
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Y�� ���� �ݿ�
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ���� �ݿ�
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// ���õ� �鸸 ������
	for (int i = 2; i < 7; ++i) {
		cube->RenderFace(i);  // ��� �� ������ (�ʿ��� ��� Ư�� �鸸 �������� �� ����)
	}


	// �� ������ (��� ������ �״�� ����)
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	glutSwapBuffers();
}





int main(int argc, char** argv) {
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Cube and Axis");

	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	cube = new Cube();
	axis = new Axis();
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}