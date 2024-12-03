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
GLfloat movementOffset = 0.0f; // 0번과 1번 면의 이동 거리
bool opening = false;          // 무대 열림 여부

struct Vertex {
	float x, y, z;
};

class Shader {		//셰이더 불러오는 클래스
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
	std::vector<GLuint> faceIndices[7];  // 여섯 개의 면을 위한 인덱스

	Cube() {
		DefineVertices();
		DefineFaceIndices();
		InitBuffer();
	}

	void DefineVertices() {
		// 큐브의 정점을 정의합니다.
		vertices = {
			{ -0.5f, -0.5f, -0.5f },  // 0: 왼쪽 아래 뒤
			{ 0.5f, -0.5f, -0.5f },   // 1: 오른쪽 아래 뒤
			{ 0.5f,  0.5f, -0.5f },   // 2: 오른쪽 위 뒤
			{ -0.5f,  0.5f, -0.5f },  // 3: 왼쪽 위 뒤
			{ -0.5f, -0.5f,  0.5f },  // 4: 왼쪽 아래 앞
			{ 0.5f, -0.5f,  0.5f },   // 5: 오른쪽 아래 앞
			{ 0.5f,  0.5f,  0.5f },   // 6: 오른쪽 위 앞
			{ -0.5f,  0.5f,  0.5f }   // 7: 왼쪽 위 앞
		};
	}

	void DefineFaceIndices() {
		// 앞면의 첫 번째 삼각형
		faceIndices[0] = { 4, 5, 6 };

		// 앞면의 두 번째 삼각형
		faceIndices[1] = { 4, 6, 7 };

		// 나머지 면
		faceIndices[2] = { 0, 1, 2, 0, 2, 3 };  // 뒷면
		faceIndices[3] = { 3, 2, 6, 3, 6, 7 };  // 윗면
		faceIndices[4] = { 0, 1, 5, 5, 4, 0 };  // 아랫면
		faceIndices[5] = { 0, 3, 7, 7, 4, 0 };  // 왼쪽 면
		faceIndices[6] = { 1, 2, 6, 6, 5, 1 };  // 오른쪽 면
	}


	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(7, EBO);  // 여섯 개의 EBO 생성
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// VBO 설정
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// 색상 VBO 설정
		GLfloat colors[] = {
			1.0f, 0.0f, 0.0f, // 빨강
			1.0f, 0.0f, 0.0f, // 빨강
			0.0f, 1.0f, 0.0f, // 초록
			0.0f, 0.0f, 1.0f, // 파랑
			1.0f, 1.0f, 0.0f, // 노랑
			1.0f, 0.0f, 1.0f, // 자홍
			0.0f, 1.0f, 1.0f  // 청록
		};
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		// 각 면에 대한 EBO 설정
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

class Axis {		//축 불러오는 클래스
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
};		//축 불러오는 클래스

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
};		//큐브 불러오는 클래스

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
// RobotCube 객체를 관리
RobotCube* robotCube;

glm::vec3 robotPosition(0.0f, -0.3f, 0.0f); // 로봇의 초기 위치
float robotSpeed = 0.01f; // 이동 속도
float robotDirection = 1.0f; // 이동 방향 (1: 오른쪽, -1: 왼쪽)
float movementLimit = 0.45f; // 이동 가능한 최대 범위

float armSwingAngle = 0.0f;      // 팔과 다리의 스윙 각도
float armSwingSpeed = 2.0f;      // 스윙 속도
bool armSwingDirection = true;	// true: 증가 방향, false: 감소 방향
float bodyRotationY = 0.0f;      // 몸의 Y축 회전

float armSwingMaxAngle = 60.0f;  // 다리 스윙 각도의 최대치 (속도에 비례)
float speedIncrement = 0.005f;   // 속도 변화량

float maxRobotSpeed = 0.1f; // 로봇의 최대 속도


// SmallCube를 관리하는 객체 배열
std::vector<SmallCube*> smallCubes;

// SmallCube 크기와 위치 설정
float smallCubeSize = 0.1f; // SmallCube 크기
float robotscale = 0.08f;

float robotDirectionZ = 0.0f; // Z축 이동 방향 (1: 앞쪽, -1: 뒤쪽, 0: 멈춤)
float movementLimitZ = 0.75f;  // Z축 이동 가능한 최대 범위

bool isJumping = false;        // 로봇이 점프 중인지 여부
float jumpVelocity = 0.03f;    // 초기 점프 속도 (기존보다 낮게 설정)
float gravity = 0.001f;        // 중력 값 감소 (느린 하강 속도)

float groundHeight = -0.3f;    // 로봇의 초기 y축 위치
float obstacleHeight = 0.1f;   // 장애물의 높이 (예: 0.2f)


// SmallCube 위치를 저장하는 배열
glm::vec3 positions[3];

void RenderRobotCube() {
	// 본체
	glm::mat4 model = glm::translate(glm::mat4(1.0f), robotPosition); // RobotCube의 위치
	model = glm::scale(model, glm::vec3(robotscale)); // 본체 크기 조정
	model = glm::rotate(model, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 회전
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	unsigned int faceColorLocation = glGetUniformLocation(shader->programID, "faceColor");

	// 본체 색상
	glm::vec3 robotBodyColor(0.2f, 0.5f, 0.8f); // 파란색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(robotBodyColor));
	robotCube->Render();

	// 머리
	glm::mat4 headModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(0.0f, 0.08f, 0.0f)); // 본체 위로 이동
	headModel = glm::rotate(headModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 회전
	headModel = glm::scale(headModel, glm::vec3(robotscale * 0.75f)); // 머리 크기 조정
	glm::vec3 robotHeadColor(0.2f, 0.5f, 0.8f); // 파란색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(headModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(robotHeadColor));
	robotCube->Render();

	// 왼쪽 팔
	glm::mat4 leftArmModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(-0.05f, 0.0f, 0.0f)); // 본체 왼쪽
	leftArmModel = glm::translate(leftArmModel, glm::vec3(0.05f, 0.0f, 0.0f));  // 팔의 기준을 몸체 중심으로 이동
	leftArmModel = glm::rotate(leftArmModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // 몸체 중심 기준 회전
	leftArmModel = glm::translate(leftArmModel, glm::vec3(-0.05f, 0.0f, 0.0f)); // 다시 원래 팔 위치로 이동
	leftArmModel = glm::rotate(leftArmModel, glm::radians(-armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // 스윙
	leftArmModel = glm::rotate(leftArmModel, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 기준 반시계 방향으로 회전
	leftArmModel = glm::scale(leftArmModel, glm::vec3(robotscale * 0.2f, robotscale * 0.8f, robotscale * 0.2f)); // 팔 크기 조정
	glm::vec3 leftArmColor(0.5f, 0.2f, 0.2f); // 붉은색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(leftArmModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(leftArmColor));
	robotCube->Render();

	// 오른쪽 팔
	glm::mat4 rightArmModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(+0.05f, 0.0f, 0.0f)); // 본체 오른쪽
	rightArmModel = glm::translate(rightArmModel, glm::vec3(-0.05f, 0.0f, 0.0f));  // 팔의 기준을 몸체 중심으로 이동
	rightArmModel = glm::rotate(rightArmModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // 몸체 중심 기준 회전
	rightArmModel = glm::translate(rightArmModel, glm::vec3(0.05f, 0.0f, 0.0f)); // 다시 원래 팔 위치로 이동
	rightArmModel = glm::rotate(rightArmModel, glm::radians(armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // 스윙
	rightArmModel = glm::rotate(rightArmModel, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 기준 시계 방향으로 회전
	rightArmModel = glm::scale(rightArmModel, glm::vec3(robotscale * 0.2f, robotscale * 0.8f, robotscale * 0.2f)); // 팔 크기 조정
	glm::vec3 rightArmColor(0.5f, 0.2f, 0.2f); // 붉은색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(rightArmModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(rightArmColor));
	robotCube->Render();


	// 왼쪽 다리
	glm::mat4 leftLegModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(-robotscale * 0.25f, - 0.06f, 0.0f)); // 본체 아래 왼쪽
	leftLegModel = glm::translate(leftLegModel, glm::vec3(robotscale * 0.25f, 0.0f, 0.0f));  // 팔의 기준을 몸체 중심으로 이동
	leftLegModel = glm::rotate(leftLegModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // 몸체 중심 기준 회전
	leftLegModel = glm::translate(leftLegModel, glm::vec3(-robotscale * 0.25f, 0.0f, 0.0f)); // 다시 원래 팔 위치로 이동
	leftLegModel = glm::rotate(leftLegModel, glm::radians(armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // 스윙
	leftLegModel = glm::scale(leftLegModel, glm::vec3(robotscale * 0.2f, robotscale, robotscale * 0.2f)); // 다리 크기 조정
	glm::vec3 leftLegColor(0.2f, 0.5f, 0.2f); // 초록색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(leftLegModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(leftLegColor));
	robotCube->Render();

	// 오른쪽 다리
	glm::mat4 rightLegModel = glm::translate(glm::mat4(1.0f), robotPosition + glm::vec3(robotscale * 0.25f, - 0.06f, 0.0f)); // 본체 아래 오른쪽
	rightLegModel = glm::translate(rightLegModel, glm::vec3(-robotscale * 0.25f, 0.0f, 0.0f));  // 팔의 기준을 몸체 중심으로 이동
	rightLegModel = glm::rotate(rightLegModel, glm::radians(bodyRotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // 몸체 중심 기준 회전
	rightLegModel = glm::translate(rightLegModel, glm::vec3(robotscale * 0.25f, 0.0f, 0.0f)); // 다시 원래 팔 위치로 이동
	rightLegModel = glm::rotate(rightLegModel, glm::radians(-armSwingAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // 스윙
	rightLegModel = glm::scale(rightLegModel, glm::vec3(robotscale * 0.2f, robotscale, robotscale * 0.2f)); // 다리 크기 조정
	glm::vec3 rightLegColor(0.2f, 0.5f, 0.2f); // 초록색 계열
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(rightLegModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(rightLegColor));
	robotCube->Render();

	// 코
	glm::mat4 noseModel = glm::translate(headModel, glm::vec3(0.0f, -0.05f, 0.5f)); // 머리 중심에서 약간 앞으로
	noseModel = glm::scale(noseModel, glm::vec3(robotscale*2)); // 코 크기 조정 (작게)
	glm::vec3 noseColor(0.0f, 0.0f, 0.0f); // 까만색
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(noseModel));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(noseColor));
	robotCube->Render();


}



// SmallCube 위치를 초기화하는 함수
void InitializeSmallCubes() {
	// 랜덤 초기화를 위해 시드 설정
	std::srand(static_cast<unsigned int>(std::time(0)));

	// Cube 아랫면의 범위: (-0.5, 0.5) x (-0.5, 0.5)에서 SmallCube를 배치
	for (int i = 0; i < 3; ++i) {
		float x = -0.4f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (0.8f))); // -0.5 ~ 0.5
		float z = -0.4f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (0.8f))); // -0.5 ~ 0.5
		positions[i] = glm::vec3(x, -0.5f + smallCubeSize / 2, z); // 랜덤 위치 설정

		// SmallCube 생성
		SmallCube* cube = new SmallCube();
		smallCubes.push_back(cube);
	}
}



void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// 투영 행렬 설정 (원근 투영)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	unsigned int projectionLocation = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// 뷰 행렬 설정 (카메라 위치)
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 1.8f),  // 카메라 위치
		glm::vec3(0.0f, 0.0f, 0.0f),  // 카메라가 바라보는 지점
		glm::vec3(0.0f, 1.0f, 0.0f)   // 월드업 벡터 (y축)
	);

	// 육면의 색상을 정의합니다.
	glm::vec3 faceColors[7] = {
		glm::vec3(1.0f, 0.0f, 0.0f), // 0번 면: 빨강
		glm::vec3(1.0f, 0.0f, 0.0f), // 1번 면: 초록
		glm::vec3(0.529f, 0.808f, 0.922f), // 2번 면: 파랑
		glm::vec3(1.0f, 1.0f, 0.0f), // 3번 면: 노랑
		glm::vec3(1.0f, 0.0f, 1.0f), // 4번 면: 자홍
		glm::vec3(0.0f, 1.0f, 1.0f), // 5번 면: 청록
		glm::vec3(0.7f, 0.6f, 1.0f)  // 6번 면: 보라
	};
	
	unsigned int viewLocation = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	// 모델 행렬 및 색상 설정
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	unsigned int faceColorLocation = glGetUniformLocation(shader->programID, "faceColor");

	// 0번과 1번 면은 별도로 처리 (이동 적용)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(+movementOffset, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[0])); // 0번 면 색상
	//cube->RenderFace(0);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-movementOffset, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[1])); // 1번 면 색상
	//cube->RenderFace(1);

	// 나머지 면 렌더링 (2번 ~ 6번 면)
	for (int i = 2; i < 7; ++i) {
		model = glm::mat4(1.0f); // 기본 위치
		model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		// 면 색상 전달
		glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[i]));

		// 면 렌더링
		cube->RenderFace(i);
	}

	// 축 렌더링
	model = glm::mat4(1.0f); // 축은 이동/회전 없이 기본 상태
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// SmallCube 렌더링 - 회색으로 설정
	glm::vec3 grayColor(0.5f, 0.5f, 0.5f); // 회색
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(grayColor)); // SmallCube 색상 설정

	// SmallCube 렌더링
	for (size_t i = 0; i < smallCubes.size(); ++i) {
		model = glm::translate(glm::mat4(1.0f), positions[i]); // 각 SmallCube 위치로 이동
		model = glm::scale(model, glm::vec3(smallCubeSize));  // 크기 조정
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		smallCubes[i]->Render();
	}

	RenderRobotCube(); // RobotCube 렌더링

	glutSwapBuffers();
}



void Keyboard(unsigned char key, int x, int y) {

	if (key == 'o' || key == 'O') {
		opening = !opening; // 무대 열기/닫기 토글
	}
	else if (key == 's' || key == 'S') {
		robotDirection = 0.0f;      // X축 이동 멈춤
		robotDirectionZ = 1.0f;     // Z축 양의 방향으로 이동
		bodyRotationY = 0.0f;       // 몸을 앞쪽으로 회전
	}
	else if (key == 'w' || key == 'W') {
		robotDirection = 0.0f;      // X축 이동 멈춤
		robotDirectionZ = -1.0f;    // Z축 음의 방향으로 이동
		bodyRotationY = 180.0f;     // 몸을 뒤쪽으로 회전
	}
	else if (key == 'a' || key == 'A') {
		robotDirection = -1.0f;     // X축 음의 방향으로 이동
		robotDirectionZ = 0.0f;     // Z축 이동 멈춤
		bodyRotationY = -90.0f;     // 몸을 왼쪽으로 회전
	}
	else if (key == 'd' || key == 'D') {
		robotDirection = 1.0f;      // X축 양의 방향으로 이동
		robotDirectionZ = 0.0f;     // Z축 이동 멈춤
		bodyRotationY = 90.0f;      // 몸을 오른쪽으로 회전
	}

	else if (key == '+') {
		// 속도 증가
		if (robotSpeed + speedIncrement <= maxRobotSpeed) {  // 속도와 증감량의 합이 최대 속도 이하인지 확인
			robotSpeed += speedIncrement;
			armSwingMaxAngle = std::min(60.0f, armSwingMaxAngle + 6.0f); // 스윙 각도 제한
		}
		else {
			robotSpeed = maxRobotSpeed; // 최대 속도에 고정
		}
	}

	else if (key == '-') {
		// 속도 감소
		if (robotSpeed - speedIncrement >= 0.01f) {  // 속도와 감소량의 차가 최소 속도 이상인지 확인
			robotSpeed -= speedIncrement;
			armSwingMaxAngle = std::max(10.0f, armSwingMaxAngle - 6.0f); // 스윙 각도 제한
		}
		else {
			robotSpeed = 0.01f; // 최소 속도로 고정
		}
	}

	else if (key == 'j' || key == 'J') {
		if (!isJumping) { // 점프 중이 아닐 때만 점프 시작
			isJumping = true;
			jumpVelocity = 0.05f; // 초기 점프 속도
		}
	}


}


// 애니메이션 업데이트 함수
void Update(int value) {

	// 무대 열림 애니메이션
	if (opening && movementOffset < 2.0f) {
		movementOffset += 0.01f;
	}
	else if (!opening && movementOffset > 2.0f) {
		movementOffset -= 0.01f;
	}

	// 로봇의 좌우 이동 업데이트
	robotPosition.x += robotSpeed * robotDirection;

	// Z축 이동
	robotPosition.z += robotSpeed * robotDirectionZ;


	// X축 범위를 벗어나면 방향 반전
	if (robotPosition.x > movementLimit || robotPosition.x < -movementLimit) {
		robotDirection *= -1.0f;
		if (robotDirection > 0) {
			bodyRotationY = 90.0f; // 오른쪽 방향
		}
		else {
			bodyRotationY = -90.0f; // 왼쪽 방향
		}
	}

	// Z축 범위를 벗어나면 방향 반전
	if (robotPosition.z > movementLimitZ || robotPosition.z < -movementLimitZ) {
		robotDirectionZ *= -1.0f;
		if (robotDirectionZ > 0) {
			bodyRotationY = 0.0f; // 앞쪽 방향
		}
		else {
			bodyRotationY = 180.0f; // 뒤쪽 방향
		}
	}

	if (isJumping) {
		robotPosition.y += jumpVelocity;    // 점프 속도만큼 y축 위치 증가
		jumpVelocity -= gravity;           // 중력 적용

		// 점프 중 장애물 위로 올라가거나 내려오는 로직
		if (robotPosition.y <= groundHeight) {
			robotPosition.y = groundHeight; // 땅에 도달
			isJumping = false;              // 점프 종료
		}
		else if (robotPosition.y >= obstacleHeight) {
			robotPosition.y = obstacleHeight; // 장애물 높이에 도달
			jumpVelocity = -0.03f;            // 내려가기 시작
		}
	}


	// 팔과 다리의 스윙 각도 조정
	if (armSwingDirection) {
		armSwingAngle += armSwingSpeed;
		if (armSwingAngle > 60.0f) armSwingDirection = false; // 최대 각도에 도달하면 반전
	}
	else {
		armSwingAngle -= armSwingSpeed;
		if (armSwingAngle < -60.0f) armSwingDirection = true; // 최소 각도에 도달하면 반전
	}

	// 팔과 다리의 스윙 각도 조정
	if (armSwingDirection) {
		armSwingAngle += armSwingSpeed;
		if (armSwingAngle > armSwingMaxAngle) {
			armSwingDirection = false; // 최대 각도에 도달하면 반전
		}
	}
	else {
		armSwingAngle -= armSwingSpeed;
		if (armSwingAngle < -armSwingMaxAngle) {
			armSwingDirection = true; // 최소 각도에 도달하면 반전
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Update, 0); // 약 60fps
}





int main(int argc, char** argv) {
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Cube and Axis");

	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // R, G, B 값을 0으로 설정해 검은색으로 설정

	cube = new Cube();
	axis = new Axis();
	shader = new Shader("vertex.glsl", "fragment.glsl");
	robotCube = new RobotCube(); // RobotCube 객체 생성

	InitializeSmallCubes();
	smallcube = new SmallCube();
	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Update, 0); // 60fps로 업데이트 함수 호출
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}

