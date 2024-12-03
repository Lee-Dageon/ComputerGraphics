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
GLfloat movementOffset = 0.0f; // 0���� 1�� ���� �̵� �Ÿ�
bool opening = false;          // ���� ���� ����

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

class SmallCube {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, colorVBO;

	SmallCube() {
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

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void Render() {
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};		//ť�� �ҷ����� Ŭ����

class RobotCube {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, colorVBO;

	RobotCube() {
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
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

Cube* cube;
Axis* axis;
Shader* shader;
SmallCube* smallcube;
// RobotCube ��ü�� ����
RobotCube* robotCube;

glm::vec3 robotPosition(0.0f, -0.3f, 0.0f); // �κ��� �ʱ� ��ġ
float robotSpeed = 0.01f; // �̵� �ӵ�
float robotDirection = 1.0f; // �̵� ���� (1: ������, -1: ����)
float movementLimit = 0.45f; // �̵� ������ �ִ� ����

float armSwingAngle = 0.0f;      // �Ȱ� �ٸ��� ���� ����
float armSwingSpeed = 2.0f;      // ���� �ӵ�
bool armSwingDirection = true;	// true: ���� ����, false: ���� ����
float bodyRotationY = 0.0f;      // ���� Y�� ȸ��

float armSwingMaxAngle = 60.0f;  // �ٸ� ���� ������ �ִ�ġ (�ӵ��� ���)
float speedIncrement = 0.005f;   // �ӵ� ��ȭ��

float maxRobotSpeed = 0.1f; // �κ��� �ִ� �ӵ�


// SmallCube�� �����ϴ� ��ü �迭
std::vector<SmallCube*> smallCubes;

// SmallCube ũ��� ��ġ ����
float smallCubeSize = 0.1f; // SmallCube ũ��
float robotscale = 0.08f;

float robotDirectionZ = 0.0f; // Z�� �̵� ���� (1: ����, -1: ����, 0: ����)
float movementLimitZ = 0.75f;  // Z�� �̵� ������ �ִ� ����

bool isJumping = false;        // �κ��� ���� ������ ����
float jumpVelocity = 0.03f;    // �ʱ� ���� �ӵ� (�������� ���� ����)
float gravity = 0.001f;        // �߷� �� ���� (���� �ϰ� �ӵ�)

float groundHeight = -0.3f;    // �κ��� �ʱ� y�� ��ġ
float obstacleHeight = 0.1f;   // ��ֹ��� ���� (��: 0.2f)


// SmallCube ��ġ�� �����ϴ� �迭
glm::vec3 positions[3];

void RenderRobotCube() {
	// ��ü
	glm::mat4 model = glm::translate(glm::mat4(1.0f), robotPosition); // RobotCube�� ��ġ
	model = glm::scale(model, glm::vec3(robotscale)); // ��ü ũ�� ����
	model = glm::rotate(model, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ȸ��
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	unsigned int faceColorLocation = glGetUniformLocation(shader->programID, "faceColor");

	// ��ü ����
	glm::vec3 robotBodyColor(0.2f, 0.5f, 0.8f); // �Ķ��� �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(robotBodyColor));
	robotCube->Render();

	// �Ӹ�
	glm::mat4 headModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(0.0f, 0.08f, 0.0f)); // ��ü ���� �̵�
	headModel = glm::rotate(headModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ȸ��
	headModel = glm::scale(headModel, glm::vec3(robotscale * 0.75f)); // �Ӹ� ũ�� ����
	glm::vec3 robotHeadColor(0.2f, 0.5f, 0.8f); // �Ķ��� �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(headModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(robotHeadColor));
	robotCube->Render();

	// ���� ��
	glm::mat4 leftArmModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(-0.05f, 0.0f, 0.0f)); // ��ü ����
	leftArmModel = glm::translate(leftArmModel, glm::vec3(0.05f, 0.0f, 0.0f));  // ���� ������ ��ü �߽����� �̵�
	leftArmModel = glm::rotate(leftArmModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // ��ü �߽� ���� ȸ��
	leftArmModel = glm::translate(leftArmModel, glm::vec3(-0.05f, 0.0f, 0.0f)); // �ٽ� ���� �� ��ġ�� �̵�
	leftArmModel = glm::rotate(leftArmModel, glm::radians(-armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // ����
	leftArmModel = glm::rotate(leftArmModel, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z�� ���� �ݽð� �������� ȸ��
	leftArmModel = glm::scale(leftArmModel, glm::vec3(robotscale * 0.2f, robotscale * 0.8f, robotscale * 0.2f)); // �� ũ�� ����
	glm::vec3 leftArmColor(0.5f, 0.2f, 0.2f); // ������ �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(leftArmModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(leftArmColor));
	robotCube->Render();

	// ������ ��
	glm::mat4 rightArmModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(+0.05f, 0.0f, 0.0f)); // ��ü ������
	rightArmModel = glm::translate(rightArmModel, glm::vec3(-0.05f, 0.0f, 0.0f));  // ���� ������ ��ü �߽����� �̵�
	rightArmModel = glm::rotate(rightArmModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // ��ü �߽� ���� ȸ��
	rightArmModel = glm::translate(rightArmModel, glm::vec3(0.05f, 0.0f, 0.0f)); // �ٽ� ���� �� ��ġ�� �̵�
	rightArmModel = glm::rotate(rightArmModel, glm::radians(armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // ����
	rightArmModel = glm::rotate(rightArmModel, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z�� ���� �ð� �������� ȸ��
	rightArmModel = glm::scale(rightArmModel, glm::vec3(robotscale * 0.2f, robotscale * 0.8f, robotscale * 0.2f)); // �� ũ�� ����
	glm::vec3 rightArmColor(0.5f, 0.2f, 0.2f); // ������ �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(rightArmModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(rightArmColor));
	robotCube->Render();


	// ���� �ٸ�
	glm::mat4 leftLegModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(-robotscale * 0.25f, - 0.06f, 0.0f)); // ��ü �Ʒ� ����
	leftLegModel = glm::translate(leftLegModel, glm::vec3(robotscale * 0.25f, 0.0f, 0.0f));  // ���� ������ ��ü �߽����� �̵�
	leftLegModel = glm::rotate(leftLegModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // ��ü �߽� ���� ȸ��
	leftLegModel = glm::translate(leftLegModel, glm::vec3(-robotscale * 0.25f, 0.0f, 0.0f)); // �ٽ� ���� �� ��ġ�� �̵�
	leftLegModel = glm::rotate(leftLegModel, glm::radians(armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // ����
	leftLegModel = glm::scale(leftLegModel, glm::vec3(robotscale * 0.2f, robotscale, robotscale * 0.2f)); // �ٸ� ũ�� ����
	glm::vec3 leftLegColor(0.2f, 0.5f, 0.2f); // �ʷϻ� �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(leftLegModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(leftLegColor));
	robotCube->Render();

	// ������ �ٸ�
	glm::mat4 rightLegModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(robotscale * 0.25f, - 0.06f, 0.0f)); // ��ü �Ʒ� ������
	rightLegModel = glm::translate(rightLegModel, glm::vec3(-robotscale * 0.25f, 0.0f, 0.0f));  // ���� ������ ��ü �߽����� �̵�
	rightLegModel = glm::rotate(rightLegModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // ��ü �߽� ���� ȸ��
	rightLegModel = glm::translate(rightLegModel, glm::vec3(robotscale * 0.25f, 0.0f, 0.0f)); // �ٽ� ���� �� ��ġ�� �̵�
	rightLegModel = glm::rotate(rightLegModel, glm::radians(-armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // ����
	rightLegModel = glm::scale(rightLegModel, glm::vec3(robotscale * 0.2f, robotscale, robotscale * 0.2f)); // �ٸ� ũ�� ����
	glm::vec3 rightLegColor(0.2f, 0.5f, 0.2f); // �ʷϻ� �迭
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(rightLegModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(rightLegColor));
	robotCube->Render();

	// ��
	glm::mat4 noseModel = glm::translate(headModel, glm::vec3(0.0f, -0.05f, 0.5f)); // �Ӹ� �߽ɿ��� �ణ ������
	noseModel = glm::scale(noseModel, glm::vec3(robotscale*2)); // �� ũ�� ���� (�۰�)
	glm::vec3 noseColor(0.0f, 0.0f, 0.0f); // ���
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(noseModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(noseColor));
	robotCube->Render();


}



// SmallCube ��ġ�� �ʱ�ȭ�ϴ� �Լ�
void InitializeSmallCubes() {
	// ���� �ʱ�ȭ�� ���� �õ� ����
	std::srand(static_cast<unsigned int>(std::time(0)));

	// Cube �Ʒ����� ����: (-0.5, 0.5) x (-0.5, 0.5)���� SmallCube�� ��ġ
	for (int i = 0; i < 3; ++i) {
		float x = -0.4f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (0.8f))); // -0.5 ~ 0.5
		float z = -0.4f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (0.8f))); // -0.5 ~ 0.5
		positions[i] = glm::vec3(x, -0.5f + smallCubeSize / 2, z); // ���� ��ġ ����

		// SmallCube ����
		SmallCube* cube = new SmallCube();
		smallCubes.push_back(cube);
	}
}



void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// ���� ��� ���� (���� ����)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	unsigned int projectionLocation = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// �� ��� ���� (ī�޶� ��ġ)
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 1.8f),  // ī�޶� ��ġ
		glm::vec3(0.0f, 0.0f, 0.0f),  // ī�޶� �ٶ󺸴� ����
		glm::vec3(0.0f, 1.0f, 0.0f)   // ����� ���� (y��)
	);

	// ������ ������ �����մϴ�.
	glm::vec3 faceColors[7] = {
		glm::vec3(1.0f, 0.0f, 0.0f), // 0�� ��: ����
		glm::vec3(1.0f, 0.0f, 0.0f), // 1�� ��: �ʷ�
		glm::vec3(0.529f, 0.808f, 0.922f), // 2�� ��: �Ķ�
		glm::vec3(1.0f, 1.0f, 0.0f), // 3�� ��: ���
		glm::vec3(1.0f, 0.0f, 1.0f), // 4�� ��: ��ȫ
		glm::vec3(0.0f, 1.0f, 1.0f), // 5�� ��: û��
		glm::vec3(0.7f, 0.6f, 1.0f)  // 6�� ��: ����
	};
	
	unsigned int viewLocation = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	// �� ��� �� ���� ����
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	unsigned int faceColorLocation = glGetUniformLocation(shader->programID, "faceColor");

	// 0���� 1�� ���� ������ ó�� (�̵� ����)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(+movementOffset, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[0])); // 0�� �� ����
	//cube->RenderFace(0);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-movementOffset, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[1])); // 1�� �� ����
	//cube->RenderFace(1);

	// ������ �� ������ (2�� ~ 6�� ��)
	for (int i = 2; i < 7; ++i) {
		model = glm::mat4(1.0f); // �⺻ ��ġ
		model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		// �� ���� ����
		glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[i]));

		// �� ������
		cube->RenderFace(i);
	}

	// �� ������
	model = glm::mat4(1.0f); // ���� �̵�/ȸ�� ���� �⺻ ����
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// SmallCube ������ - ȸ������ ����
	glm::vec3 grayColor(0.5f, 0.5f, 0.5f); // ȸ��
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(grayColor)); // SmallCube ���� ����

	// SmallCube ������
	for (size_t i = 0; i < smallCubes.size(); ++i) {
		model = glm::translate(glm::mat4(1.0f), positions[i]); // �� SmallCube ��ġ�� �̵�
		model = glm::scale(model, glm::vec3(smallCubeSize));  // ũ�� ����
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		smallCubes[i]->Render();
	}

	RenderRobotCube(); // RobotCube ������

	glutSwapBuffers();
}



void Keyboard(unsigned char key, int x, int y) {

	if (key == 'o' || key == 'O') {
		opening = !opening; // ���� ����/�ݱ� ���
	}
	else if (key == 's' || key == 'S') {
		robotDirection = 0.0f;      // X�� �̵� ����
		robotDirectionZ = 1.0f;     // Z�� ���� �������� �̵�
		bodyRotationY = 0.0f;       // ���� �������� ȸ��
	}
	else if (key == 'w' || key == 'W') {
		robotDirection = 0.0f;      // X�� �̵� ����
		robotDirectionZ = -1.0f;    // Z�� ���� �������� �̵�
		bodyRotationY = 180.0f;     // ���� �������� ȸ��
	}
	else if (key == 'a' || key == 'A') {
		robotDirection = -1.0f;     // X�� ���� �������� �̵�
		robotDirectionZ = 0.0f;     // Z�� �̵� ����
		bodyRotationY = -90.0f;     // ���� �������� ȸ��
	}
	else if (key == 'd' || key == 'D') {
		robotDirection = 1.0f;      // X�� ���� �������� �̵�
		robotDirectionZ = 0.0f;     // Z�� �̵� ����
		bodyRotationY = 90.0f;      // ���� ���������� ȸ��
	}

	else if (key == '+') {
		// �ӵ� ����
		if (robotSpeed + speedIncrement <= maxRobotSpeed) {  // �ӵ��� �������� ���� �ִ� �ӵ� �������� Ȯ��
			robotSpeed += speedIncrement;
			armSwingMaxAngle = std::min(60.0f, armSwingMaxAngle + 6.0f); // ���� ���� ����
		}
		else {
			robotSpeed = maxRobotSpeed; // �ִ� �ӵ��� ����
		}
	}

	else if (key == '-') {
		// �ӵ� ����
		if (robotSpeed - speedIncrement >= 0.01f) {  // �ӵ��� ���ҷ��� ���� �ּ� �ӵ� �̻����� Ȯ��
			robotSpeed -= speedIncrement;
			armSwingMaxAngle = std::max(10.0f, armSwingMaxAngle - 6.0f); // ���� ���� ����
		}
		else {
			robotSpeed = 0.01f; // �ּ� �ӵ��� ����
		}
	}

	else if (key == 'j' || key == 'J') {
		if (!isJumping) { // ���� ���� �ƴ� ���� ���� ����
			isJumping = true;
			jumpVelocity = 0.05f; // �ʱ� ���� �ӵ�
		}
	}


}


// �ִϸ��̼� ������Ʈ �Լ�
void Update(int value) {

	// ���� ���� �ִϸ��̼�
	if (opening && movementOffset < 2.0f) {
		movementOffset += 0.01f;
	}
	else if (!opening && movementOffset > 2.0f) {
		movementOffset -= 0.01f;
	}

	// �κ��� �¿� �̵� ������Ʈ
	robotPosition.x += robotSpeed * robotDirection;

	// Z�� �̵�
	robotPosition.z += robotSpeed * robotDirectionZ;


	// X�� ������ ����� ���� ����
	if (robotPosition.x > movementLimit || robotPosition.x < -movementLimit) {
		robotDirection *= -1.0f;
		if (robotDirection > 0) {
			bodyRotationY = 90.0f; // ������ ����
		}
		else {
			bodyRotationY = -90.0f; // ���� ����
		}
	}

	// Z�� ������ ����� ���� ����
	if (robotPosition.z > movementLimitZ || robotPosition.z < -movementLimitZ) {
		robotDirectionZ *= -1.0f;
		if (robotDirectionZ > 0) {
			bodyRotationY = 0.0f; // ���� ����
		}
		else {
			bodyRotationY = 180.0f; // ���� ����
		}
	}

	if (isJumping) {
		robotPosition.y += jumpVelocity;    // ���� �ӵ���ŭ y�� ��ġ ����
		jumpVelocity -= gravity;           // �߷� ����

		// ���� �� ��ֹ� ���� �ö󰡰ų� �������� ����
		if (robotPosition.y <= groundHeight) {
			robotPosition.y = groundHeight; // ���� ����
			isJumping = false;              // ���� ����
		}
		else if (robotPosition.y >= obstacleHeight) {
			robotPosition.y = obstacleHeight; // ��ֹ� ���̿� ����
			jumpVelocity = -0.03f;            // �������� ����
		}
	}


	// �Ȱ� �ٸ��� ���� ���� ����
	if (armSwingDirection) {
		armSwingAngle += armSwingSpeed;
		if (armSwingAngle > 60.0f) armSwingDirection = false; // �ִ� ������ �����ϸ� ����
	}
	else {
		armSwingAngle -= armSwingSpeed;
		if (armSwingAngle < -60.0f) armSwingDirection = true; // �ּ� ������ �����ϸ� ����
	}

	// �Ȱ� �ٸ��� ���� ���� ����
	if (armSwingDirection) {
		armSwingAngle += armSwingSpeed;
		if (armSwingAngle > armSwingMaxAngle) {
			armSwingDirection = false; // �ִ� ������ �����ϸ� ����
		}
	}
	else {
		armSwingAngle -= armSwingSpeed;
		if (armSwingAngle < -armSwingMaxAngle) {
			armSwingDirection = true; // �ּ� ������ �����ϸ� ����
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Update, 0); // �� 60fps
}





int main(int argc, char** argv) {
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Cube and Axis");

	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // R, G, B ���� 0���� ������ ���������� ����

	cube = new Cube();
	axis = new Axis();
	shader = new Shader("vertex.glsl", "fragment.glsl");
	robotCube = new RobotCube(); // RobotCube ��ü ����

	InitializeSmallCubes();
	smallcube = new SmallCube();
	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Update, 0); // 60fps�� ������Ʈ �Լ� ȣ��
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}

