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
GLfloat rotationX = 10.0f;
GLfloat rotationY = 10.0f;
bool cullFaceEnabled = false;
bool yRotationEnabled = false;


GLfloat rotationSpeed = 1.0f;
GLfloat frontFaceAngle = 0.0f; // �ո� ȸ�� ����
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);


//�ո� ȸ��
bool frontFaceMovingEnabled = false;  // �ո� �̵� Ȱ��ȭ ����
float slideDistance = 0.0f;          // ���� �����̵� �Ÿ�
float maxSlideDistance = 1.0f;       // �ִ� �����̵� �Ÿ�
float slideSpeed = 0.05f;            // �����̵� �ӵ�


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
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, colorVBO;
	GLuint shaderProgramID;  // ���̴� ���α׷� ID�� ������ ���� �߰�

	Cube(GLuint shaderProgram) {
		shaderProgramID = shaderProgram;  // ���̴� ���α׷� ID ����
		LoadOBJ("cube.obj");
		InitBuffer();
	}

	bool LoadOBJ(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Failed to open OBJ file: " << filename << std::endl;
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			std::istringstream ss(line);
			std::string prefix;
			ss >> prefix;

			if (prefix == "v") {
				Vertex vertex;
				ss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);
			}
			else if (prefix == "f") {
				GLuint index;
				for (int i = 0; i < 3; i++) {
					ss >> index;
					indices.push_back(index - 1);
				}
			}
		}

		file.close();
		return true;
	}


	void InitBuffer() {
    // ������ �ε��� �и�
    std::vector<Vertex> frontVertices;
    std::vector<GLuint> frontIndices;

    for (size_t i = 0; i < indices.size(); i += 3) {
        if (IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
            for (size_t j = 0; j < 3; ++j) {
                frontVertices.push_back(vertices[indices[i + j]]);
                frontIndices.push_back(frontIndices.size());
            }
        }
    }

    // �ո� VAO, VBO, EBO ����
    glGenVertexArrays(1, &frontVAO);
    glGenBuffers(1, &frontVBO);
    glGenBuffers(1, &frontEBO);

    glBindVertexArray(frontVAO);

    glBindBuffer(GL_ARRAY_BUFFER, frontVBO);
    glBufferData(GL_ARRAY_BUFFER, frontVertices.size() * sizeof(Vertex), frontVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frontEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, frontIndices.size() * sizeof(GLuint), frontIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // ������ �� ó�� (����ó��)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}


	void Render() {
		glUseProgram(shaderProgramID); // ���̴� ���α׷� ���

		glBindVertexArray(VAO);

		// 1. �ո鸸 ������ �����̵� �����Ͽ� ������
		if (frontFaceMovingEnabled) {
			// �����̵� ��ȯ ��� ����
			glm::mat4 leftSlideModel = glm::translate(glm::mat4(1.0f), glm::vec3(slideDistance, 0.0f, 0.0f));
			glm::mat4 rightSlideModel = glm::translate(glm::mat4(1.0f), glm::vec3(-slideDistance, 0.0f, 0.0f));

			unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");

			for (size_t i = 0; i < indices.size(); i += 3) {
				if (IsLeftFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(leftSlideModel));
					glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
				}
				else if (IsRightFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(rightSlideModel));
					glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
				}
			}
		}

		// 2. ������ �� ������ (�⺻ ��ȯ ����)
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

		unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		for (size_t i = 0; i < indices.size(); i += 3) {
			if (!IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
				glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
			}
		}

		glBindVertexArray(0);
	}



	bool IsFrontFace(GLuint idx1, GLuint idx2, GLuint idx3) {
		// �ո��� ���� 1, 2, 3�� 1, 3, 4�� ������ (����)
		return (idx1 == 0 && idx2 == 1 && idx3 == 2) ||
			(idx1 == 0 && idx2 == 2 && idx3 == 3);
	}

	bool IsLeftFrontFace(GLuint idx1, GLuint idx2, GLuint idx3) {
		return (idx1 == 0 && idx2 == 1 && idx3 == 2);
	}

	bool IsRightFrontFace(GLuint idx1, GLuint idx2, GLuint idx3) {
		return (idx1 == 0 && idx2 == 2 && idx3 == 3);
	}

	void SlideFrontFace(float distance) {
		for (size_t i = 0; i < indices.size(); i += 3) {
			// �ε��� 0, 1, 2�� ������ �ﰢ���� �������� �̵�
			if (IsLeftFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
				for (size_t j = 0; j < 3; ++j) {
					GLuint index = indices[i + j];
					vertices[index].x += distance;  // �������� �̵�
				}
			}
			// �ε��� 0, 2, 3���� ������ �ﰢ���� ���������� �̵�
			else if (IsRightFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
				for (size_t j = 0; j < 3; ++j) {
					GLuint index = indices[i + j];
					vertices[index].x -= distance;  // ���������� �̵�
				}
			}
		}

		// ���� �����͸� VBO�� ������Ʈ
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), 
			vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

};  // ť�� �ҷ����� Ŭ���� ��




class Axis {        // �� �ҷ����� Ŭ����
public:
	GLuint VAO, VBO;

	Axis() {
		InitBuffer();
	}

	void InitBuffer() {
		GLfloat axisVertices[] = {
			// X��
			-1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 // Y��
			  0.0f, -1.0f, 0.0f,
			  0.0f, 0.0f, 0.0f,

			  0.0f, 1.0f, 0.0f,
			  0.0f, 0.0f, 0.0f,

			  // Z��
			   0.0f, 0.0f, -1.0f,
			   0.0f, 0.0f, 0.0f,

			   0.0f, 0.0f, 1.0f,
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
		glDrawArrays(GL_LINES, 0, 6);  // �� 6���� ���� (�� �� 2���� ����Ʈ)
		glBindVertexArray(0);
	}
};        // �� �ҷ����� Ŭ����


Cube* cube;
Axis* axis;
Shader* shader;

glm::mat4 projection, view;


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// ���� ���� Ȱ��ȭ ���ο� ���� ����
	if (cullFaceEnabled) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	// ���� ���� ���
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

	// ī�޶� �� ���
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, -2.0f), // ī�޶� ��ġ (�ָ��� ť�긦 �ٶ�)
		glm::vec3(0.0f, 0.0f, 0.0f), // Ÿ�� ��ġ (ť�� �߽�)
		glm::vec3(0.0f, 1.0f, 0.0f)  // �� ���� (Y�� ����)
	);

	// ť�� �� ���
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

	// ���̴��� ��� ����
	unsigned int projLocation = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));

	unsigned int viewLocation = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));


	// ť�� ������
	cube->Render();

	// �� ������
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

	shader = new Shader("vertex.glsl", "fragment.glsl");
	// Cube ��ü ���� �� ���̴� ���α׷� ID�� ����
	cube = new Cube(shader->programID);
	axis = new Axis();

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);  // Ű���� �ݹ� �Լ� ���
	glEnable(GL_DEPTH_TEST);
	glutTimerFunc(0, Timer, 0);  // Ÿ�̸� �Լ� ���
	glutMainLoop();
	return 0;
}

void Timer(int value) {
	if (frontFaceMovingEnabled) {
		static float slideDistance = 0.0f;  // ���� �����̵� �Ÿ�
		float maxSlideDistance = 1.0f;     // �ִ� �����̵� �Ÿ�
		float slideSpeed = 0.02f;          // �����̵� �ӵ�

		if (slideDistance < maxSlideDistance) {
			cube->SlideFrontFace(slideSpeed);   // �����̵� ����
			slideDistance += slideSpeed; // ���� �Ÿ� ����
		}
		else {
			frontFaceMovingEnabled = false; // �����̵� ����
		}
	}

	glutPostRedisplay();           // ȭ�� ���� ��û
	glutTimerFunc(16, Timer, 0);   // �� 60 FPS�� Ÿ�̸� ȣ��
}


// Ű���� �Է� ó�� �Լ�
void Keyboard(unsigned char key, int x, int y) {

	switch (key) {

	case 'h':
	case 'H':
		cullFaceEnabled = !cullFaceEnabled;
		std::cout << "Cull Face " << (cullFaceEnabled ? "Enabled" : "Disabled") << std::endl;
		glutPostRedisplay();
		break;

	case 't':
	case 'T':
		frontFaceMovingEnabled = !frontFaceMovingEnabled;
		std::cout << "Front Face Rotation " << (frontFaceMovingEnabled ? "Enabled" : "Disabled") << std::endl;
		break;

	case 'o':  // �����̵� �ִϸ��̼� Ȱ��ȭ
	case 'O':
		if (!frontFaceMovingEnabled) {
			frontFaceMovingEnabled = true;  // �����̵� ����
		}
		break;


	default:
		break;
	}
}

