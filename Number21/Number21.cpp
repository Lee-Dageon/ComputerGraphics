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
GLfloat rotationY = -30.0f;
bool cullFaceEnabled = false;
bool yRotationEnabled = false;
bool frontFaceRotationEnabled = false; // �ո� ȸ�� Ȱ��ȭ ����
// ���� �ӵ�
GLfloat rotationSpeed = 1.0f;
GLfloat frontFaceAngle = -30.0f; // �ո� ȸ�� ����
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);

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
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		GLfloat colors[] = {
			1.0f, 0.0f, 0.0f,  // �ո� ����
			0.0f, 1.0f, 0.0f,  // �޸� �ʷ�
			0.0f, 0.0f, 1.0f,  // ���� �Ķ�
			1.0f, 1.0f, 0.0f,  // �Ʒ��� ���
			1.0f, 0.0f, 1.0f,  // ���� �� ��ȫ��
			0.0f, 1.0f, 1.0f   // ������ �� û�ϻ�
		};

		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void Render() {
		glUseProgram(shaderProgramID);  // ���̴� ���α׷� ���

		glBindVertexArray(VAO);

		// �ո鸸 ȸ����Ű�� ����
		if (frontFaceRotationEnabled) {
			// �ո��� �������� �߽��� ���
			glm::vec3 center(0.0f);
			for (size_t i = 0; i < indices.size(); i += 3) {
				if (IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					center += glm::vec3(vertices[indices[i]].x, vertices[indices[i]].y, vertices[indices[i]].z);
					center += glm::vec3(vertices[indices[i + 1]].x, vertices[indices[i + 1]].y, vertices[indices[i + 1]].z);
					center += glm::vec3(vertices[indices[i + 2]].x, vertices[indices[i + 2]].y, vertices[indices[i + 2]].z);
					center /= 3.0f;  // �ո��� �� ������ ����� �߽����� ����
					break;
				}
			}

			// �ո鸸 ȸ���ϴ� ��ȯ ��� ���� (���� �� ȸ�� �߰�)
			glm::mat4 frontFaceTransform = glm::mat4(1.0f);

			// ������ ť��� ������ ȸ�� ���� (��ü ȸ��)
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

			// �ո� �߽� ���� ȸ�� �߰� (x�� �������� ȸ�� - �Ѳ�ó�� ȸ��)
			frontFaceTransform = glm::translate(frontFaceTransform, center); // �߽����� �̵�
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(frontFaceAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // �߽� ���� ȸ�� (x�� �������� �Ѳ�ó�� ȸ��)
			frontFaceTransform = glm::translate(frontFaceTransform, -center); // ���� ��ġ�� �̵�

			unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(frontFaceTransform));

			// �ո鸸 �׸���
			for (size_t i = 0; i < indices.size(); i += 3) {
				if (IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					// �ո� ������ ȸ�� ���� �� ������
					glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
				}
			}
		}

		// ������ ť�� �κ��� �⺻ ȸ�� �����Ͽ� ������
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

		unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		for (size_t i = 0; i < indices.size(); i += 3) {
			if (!IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
				// �ո��� �ƴ� ������ �� ������
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
};  // ť�� �ҷ����� Ŭ���� ��


class Horn {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// �⺻ ������: �⺻ ���� �̸� "horn.obj"�� �ε�
	Horn() {
		if (LoadOBJ("horn.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load horn OBJ file." << std::endl;
		}
	}

	// ����� �ε��� ���۸� �Բ� �����ϴ� �ʱ�ȭ �Լ�
	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// ���� ������ ����
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// ���� ������ ����
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		// �ε��� ������ ����
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Load .obj ���� �� ���� ����
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
				// ���� ������
				Vertex vertex;
				ss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);

				// �⺻ ���� �߰� (�ټ� ���� �鿡 ���� �ٸ� ������ ����)
				if (indices.size() == 0) { // �ո� (����)
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // ���� 1
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // ���� 2
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // ���� 5
				}
				else if (indices.size() == 3) { // ������ �� (�ʷ�)
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // ���� 2
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // ���� 3
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // ���� 5
				}
				else if (indices.size() == 6) { // �޸� (�Ķ�)
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // ���� 3
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // ���� 4
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // ���� 5
				}
				else if (indices.size() == 9) { // ���� �� (���)
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // ���� 4
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // ���� 1
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // ���� 5
				}
				else if (indices.size() == 12) { // �ظ� (��ȫ)
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // ���� 1
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // ���� 2
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // ���� 3
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // ���� 4
				}

			}
			else if (prefix == "f") {
				// �� ������ (1-based �ε���, �ﰢ�� ��)
				GLuint index;
				for (int i = 0; i < 3; i++) {
					ss >> index;
					indices.push_back(index - 1); // 1-based�� 0-based �ε����� ��ȯ
				}
			}
		}

		file.close();
		return true;
	}

	void Render() const {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	~Horn() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO);
		if (EBO) glDeleteBuffers(1, &EBO);
		if (colorVBO) glDeleteBuffers(1, &colorVBO);
	}
};


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
Horn* horn;
Shader* shader;





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

	// y�� ������ Ȱ��ȭ�� ��� ȸ�� ������ ������Ʈ
	if (yRotationEnabled) {
		rotationY += rotationSpeed;
		if (rotationY > 360.0f) {
			rotationY -= 360.0f;
		}
	}


	glm::mat4 model = glm::mat4(1.0f);

	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	cube->Render();

	// ������ �� ������ (���� ũ��� ��ġ�� ������)
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// �簢��(Horn) ������ (ũ�� 0.5�� ���, X/Y�� ȸ�� ����)
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	//horn->Render();

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
	// �簢�� ��ü ����
	horn = new Horn();


	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);  // Ű���� �ݹ� �Լ� ���
	glEnable(GL_DEPTH_TEST);
	glutTimerFunc(0, Timer, 0);  // Ÿ�̸� �Լ� ���
	glutMainLoop();
	return 0;
}

// Ÿ�̸� �Լ�
void Timer(int value) {

	if (frontFaceRotationEnabled) {
		frontFaceAngle += rotationSpeed;
		if (frontFaceAngle > 360.0f) {
			frontFaceAngle -= 360.0f;
		}
	}

	glutPostRedisplay();  // ����� �����Ͽ� ������ ���̵��� ��
	glutTimerFunc(16, Timer, 0);  // �� 60 FPS�� Ÿ�̸� ȣ�� (16ms����)
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

	case 'y':
	case 'Y':
		yRotationEnabled = !yRotationEnabled;
		std::cout << "Y-axis rotation " << (yRotationEnabled ? "Enabled" : "Disabled") << std::endl;
		break;

	case 't':
	case 'T':
		frontFaceRotationEnabled = !frontFaceRotationEnabled;
		std::cout << "Front Face Rotation " << (frontFaceRotationEnabled ? "Enabled" : "Disabled") << std::endl;
		break;

	default:
		break;
	}
}

