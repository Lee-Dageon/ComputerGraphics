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

void Keyboard(unsigned char key, int x, int y);
void DrawSpiral();

std::vector<glm::vec3> spiralPath;
int spiralIndex = 0;       // 현재 도형이 위치할 스파이럴의 인덱스
bool followSpiral = false; // 도형이 나선형을 따라 이동하는지 여부



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
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, colorVBO;

	Cube() {
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
	 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 앞면 빨강
	 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 뒷면 초록
	 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // 윗면 파랑
	 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // 아랫면 노랑
	 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // 왼쪽 면 자홍색
	 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f  // 오른쪽 면 청록색
		};

		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
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

class Horn {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// 기본 생성자: 기본 파일 이름 "horn.obj"로 로드
	Horn() {
		if (LoadOBJ("horn.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load horn OBJ file." << std::endl;
		}
	}

	// 색상과 인덱스 버퍼를 함께 설정하는 초기화 함수
	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// 정점 데이터 버퍼
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// 색상 데이터 버퍼
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(1);

		// 인덱스 데이터 버퍼
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Load .obj 파일 및 색상 설정
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
				// 정점 데이터
				Vertex vertex;
				ss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);

				// 기본 색상 추가 (다섯 개의 면에 각각 다른 색상을 적용)
				if (indices.size() == 0) { // 앞면 (빨강)
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // 정점 1
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // 정점 2
					colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f }); // 정점 5
				}
				else if (indices.size() == 3) { // 오른쪽 면 (초록)
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // 정점 2
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // 정점 3
					colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f }); // 정점 5
				}
				else if (indices.size() == 6) { // 뒷면 (파랑)
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // 정점 3
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // 정점 4
					colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f }); // 정점 5
				}
				else if (indices.size() == 9) { // 왼쪽 면 (노랑)
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // 정점 4
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // 정점 1
					colors.insert(colors.end(), { 1.0f, 1.0f, 0.0f }); // 정점 5
				}
				else if (indices.size() == 12) { // 밑면 (자홍)
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // 정점 1
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // 정점 2
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // 정점 3
					colors.insert(colors.end(), { 1.0f, 0.0f, 1.0f }); // 정점 4
				}

			}
			else if (prefix == "f") {
				// 면 데이터 (1-based 인덱스, 삼각형 면)
				GLuint index;
				for (int i = 0; i < 3; i++) {
					ss >> index;
					indices.push_back(index - 1); // 1-based를 0-based 인덱스로 변환
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

class Corn {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// 기본 생성자: 기본 파일 이름 "corn.obj"로 로드
	Corn() {
		if (LoadOBJ("corn.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load corn OBJ file." << std::endl;
		}
	}

	// 색상과 인덱스 버퍼를 함께 설정하는 초기화 함수
	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// 정점 데이터 버퍼
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		// 인덱스 데이터 버퍼
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Load .obj 파일
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
					indices.push_back(index - 1); // 1-based to 0-based indexing
				}

				// 각 면에 대해 랜덤 색상 추가 (예: RGB 각 요소가 0.0f ~ 1.0f 사이의 랜덤값)
				GLfloat r = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat g = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat b = static_cast<GLfloat>(rand()) / RAND_MAX;

				// 각 면의 3개의 정점에 동일한 색상 적용
				for (int i = 0; i < 3; i++) {
					colors.push_back(r);
					colors.push_back(g);
					colors.push_back(b);
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

	~Corn() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO);
		if (EBO) glDeleteBuffers(1, &EBO);
		if (colorVBO) glDeleteBuffers(1, &colorVBO);
	}
};

class Axis {        // 축 불러오는 클래스
public:
	GLuint VAO, VBO;

	Axis() {
		InitBuffer();
	}

	void InitBuffer() {
		GLfloat axisVertices[] = {
			// X축
			-1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 0.0f,

			 // Y축
			  0.0f, -1.0f, 0.0f,
			  0.0f, 0.0f, 0.0f,

			  0.0f, 1.0f, 0.0f,
			  0.0f, 0.0f, 0.0f,

			  // Z축
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
		glDrawArrays(GL_LINES, 0, 6);  // 총 6개의 라인 (각 축 2개의 포인트)
		glBindVertexArray(0);
	}
};        // 축 불러오는 클래스

class Sphere {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// 기본 생성자: 기본 파일 이름 "sphere.obj"로 로드
	Sphere() {
		if (LoadOBJ("sphere.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load sphere OBJ file." << std::endl;
		}
	}

	// 버퍼를 초기화하여 OpenGL에서 렌더링 가능하게 설정
	void InitBuffer() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &colorVBO);

		glBindVertexArray(VAO);

		// 정점 데이터 버퍼
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);

		// 색상 데이터 버퍼 - 각 면에 고유한 색상 적용
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		// 인덱스 데이터 버퍼
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Load .obj 파일 및 색상 설정
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
				// 정점 데이터
				Vertex vertex;
				ss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);
			}
			else if (prefix == "f") {
				// 면 데이터
				GLuint index;
				for (int i = 0; i < 3; i++) {
					ss >> index;
					indices.push_back(index - 1); // 1-based를 0-based 인덱스로 변환
				}

				// 각 면에 대해 랜덤 색상 추가 (예: RGB 각 요소가 0.0f ~ 1.0f 사이의 랜덤값)
				GLfloat r = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat g = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat b = static_cast<GLfloat>(rand()) / RAND_MAX;

				// 각 면의 3개의 정점에 동일한 색상 적용
				for (int i = 0; i < 3; i++) {
					colors.push_back(r);
					colors.push_back(g);
					colors.push_back(b);
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

	~Sphere() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO);
		if (EBO) glDeleteBuffers(1, &EBO);
		if (colorVBO) glDeleteBuffers(1, &colorVBO);
	}
};

void DrawSpiral() {
	const float spiralRadius = 0.04f;  // 나선형의 간격과 크기를 조정하는 변수
	const float spiralHeight = 0.00f; // y축으로 나선형이 상승하는 높이 조정
	const int numTurns = 5;       // 나선형의 회전 수
	const int numPointsPerTurn = 100; // 각 회전당 점의 개수

	std::vector<glm::vec3> spiralVertices;

	for (int i = 0; i < numTurns * numPointsPerTurn; ++i) {
		float angle = glm::radians(i * (360.0f / numPointsPerTurn));  // 각도 계산
		float x = spiralRadius * angle * cos(angle);
		float z = spiralRadius * angle * sin(angle);
		float y = spiralHeight * angle;

		spiralVertices.emplace_back(x, y, z);
	}

	GLuint spiralVAO, spiralVBO;
	glGenVertexArrays(1, &spiralVAO);
	glGenBuffers(1, &spiralVBO);

	glBindVertexArray(spiralVAO);
	glBindBuffer(GL_ARRAY_BUFFER, spiralVBO);
	glBufferData(GL_ARRAY_BUFFER, spiralVertices.size() * sizeof(glm::vec3), spiralVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(spiralVAO);
	glDrawArrays(GL_LINE_STRIP, 0, spiralVertices.size());
	glBindVertexArray(0);

	// 메모리 해제
	glDeleteBuffers(1, &spiralVBO);
	glDeleteVertexArrays(1, &spiralVAO);
}

void CalculateSpiralPath() {
	const float spiralRadius = 0.04f;
	const float spiralHeight = 0.00f;
	const int numTurns = 5;
	const int numPointsPerTurn = 100;

	spiralPath.clear();
	for (int i = 0; i < numTurns * numPointsPerTurn; ++i) {
		float angle = glm::radians(i * (360.0f / numPointsPerTurn));
		float x = spiralRadius * angle * cos(angle);
		float z = spiralRadius * angle * sin(angle);
		float y = spiralHeight * angle;

		spiralPath.emplace_back(x, y, z);
	}
}


Cube* cube;
Axis* axis;
Horn* horn;
Corn* corn;
Sphere* sphere;
Shader* shader;


GLfloat rotation1X = 30.0f;
GLfloat rotation2X = 30.0f;
GLfloat rotationX = 30.0f;
GLfloat rotationY = 30.0f;
float howmuchrotate = 30.0f;
float orbityAngle = 0.0f;
float orbityRadius = -0.6f;
float orbityRadiusCube = 0.6f;
bool rotate1XEnabled = false;
bool rotate2XEnabled = false;
bool rotateYEnabled = false;
bool zigZagEnabled = false;
bool renderHorn = false;
bool renderSphere = false;
bool MaxingEnabled = false; // 확대 여부
bool SmallingEnabled = false;

bool updownMove = false;

float Scale1 = 0.12f;          // 큐브의 초기 크기
float Scale2 = 0.2f;


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	//-------------------------------------------------------------------------------------------------------------------
	glm::mat4 model = glm::mat4(1.0f);
	
	if (rotateYEnabled) {
		// 공전할 때 y 축을 중심으로 x, z 좌표가 원형 궤도를 그리도록 설정
		float x = orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	else if (zigZagEnabled) {
		// 공전할 때 y 축을 중심으로 x, z 좌표가 원형 궤도를 그리도록 설정
		float x = orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	// 도형이 스파이럴 경로를 따라 이동할 경우
	else if (followSpiral && spiralIndex < spiralPath.size()) {
		glm::vec3 spiralPos = -spiralPath[spiralIndex];
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
		model = glm::translate(model, spiralPos); // 스파이럴 위치로 이동
	}

	else {
		// 공전이 비활성화되었을 때 기본 위치
		model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
	}

	model = glm::scale(model, glm::vec3(Scale1, Scale1, Scale1));
	model = glm::rotate(model, glm::radians(rotation1X), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	// renderHorn 플래그에 따라 cube 또는 horn 렌더링
	if (renderHorn) {
		horn->Render();
	}
	else {
		cube->Render();
	}
	//-----------------------------------------------------------------------------------------------

	// 축 렌더링 (회전 적용)
	model = glm::mat4(1.0f);
	// 축 회전을 위한 3D 뷰 매트릭스 설정
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	//-----------------------------------------------------------------------------------------------------
	// 
	// 원뿔(Corn) 렌더링 (크기 0.5배 축소, X/Y축 회전 적용)
	model = glm::mat4(1.0f);
	if (rotateYEnabled) {
		// 공전할 때 y 축을 중심으로 x, z 좌표가 원형 궤도를 그리도록 설정
		float x = orbityRadius * cos(glm::radians(orbityAngle));
		float z = orbityRadius * sin(glm::radians(orbityAngle));
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	// 도형이 스파이럴 경로를 따라 이동할 경우
	else if (followSpiral && spiralIndex < spiralPath.size()) {
		glm::vec3 spiralPos = spiralPath[spiralIndex];
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
		model = glm::translate(model, spiralPos); // 스파이럴 위치로 이동
	}

	else if (zigZagEnabled) {
		// 공전할 때 y 축을 중심으로 x, z 좌표가 원형 궤도를 그리도록 설정
		float x = -orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	else {
		// 공전이 비활성화되었을 때 기본 위치
		model = glm::translate(model, glm::vec3(-0.5f, 0.0f, 0.0f));
	}


	model = glm::scale(model, glm::vec3(Scale2, Scale2, Scale2));
	model = glm::rotate(model, glm::radians(rotation2X), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	// renderHorn 플래그에 따라 cube 또는 horn 렌더링
	if (renderSphere) {
		sphere->Render();
	}
	else {
		corn->Render();
	}


	model = glm::mat4(1.0f);
	// 축 회전을 위한 3D 뷰 매트릭스 설정 
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X축으로 45도 회전
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y축으로 45도 회전
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE,glm::value_ptr(model));
	DrawSpiral();


	glutSwapBuffers();
}

// 타이머 콜백 함수


bool isTimerRunning = false;  // 타이머가 실행 중인지 확인하는 플래그

void Timer(int value) {

	if (MaxingEnabled) {
		if (Scale1 <= 0.3f) {
			Scale1 += 0.001f; // 스케일을 점진적으로 증가
		}

		else {
			Scale1 = 0.12f;
		}
	}

	if (SmallingEnabled) {
		if (Scale2 >= 0.0f) {
			Scale2 -= 0.001f; // 스케일을 점진적으로 감소		
		}

		else {
			Scale2 = 0.2f;
		}
	}

	if (rotate1XEnabled) {
		rotation1X += 0.8f;
		if (rotation1X >= 360.0f) {
			rotation1X -= 360.0f;
		}
	}

	if (rotate2XEnabled) {
		rotation2X += 0.8f;
		if (rotation2X >= 360.0f) {
			rotation2X -= 360.0f;
		}
	}

	if (rotateYEnabled) {
		orbityAngle += 0.8f;
		if (orbityAngle >= 360.0f) {
			orbityAngle -= 360.0f;
		}
	}


	if (zigZagEnabled) {
		orbityAngle += 0.8f;
		if (orbityAngle >= 360.0f) {
			orbityAngle -= 360.0f;
		}
	}

	glutPostRedisplay();

	if (followSpiral && !spiralPath.empty()) {
		// spiralPath에 저장된 위치로 이동
		spiralIndex = (spiralIndex + 1) % spiralPath.size(); // 인덱스를 순환하며 증가
	}


	if (MaxingEnabled || SmallingEnabled || followSpiral || rotate1XEnabled || rotate2XEnabled || rotateYEnabled || zigZagEnabled) {
		glutTimerFunc(16, Timer, 0);  // 타이머를 다시 호출하여 애니메이션을 지속
	}
	else {
		isTimerRunning = false;  // 회전이 모두 비활성화되면 타이머 종료
	}
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

	// 사각뿔 객체 생성
	horn = new Horn();
	corn = new Corn();
	sphere = new Sphere();
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}


// 글로벌 플래그
bool xKeyPressed = false;

// 초기화 상태 저장
const GLfloat initialRotation1X = 30.0f;
const GLfloat initialRotation2X = 30.0f;
const GLfloat initialRotationY = 30.0f;
const float initialOrbityAngle = 0.0f;

void Reset() {
	// 각도를 초기 상태로 되돌림
	rotation1X = initialRotation1X;
	rotation2X = initialRotation2X;
	rotationY = initialRotationY;
	orbityAngle = initialOrbityAngle;

	// 모든 회전 플래그를 비활성화
	rotate1XEnabled = false;
	rotate2XEnabled = false;
	rotateYEnabled = false;

	// 타이머 중복 호출 방지 플래그 해제
	isTimerRunning = false;
	renderHorn = false;
	renderSphere = false;

	Scale1 = 0.12f;
	Scale2 = 0.2f;

	// 화면 갱신
	glutPostRedisplay();
}


void Keyboard(unsigned char key, int x, int y) {
	//if (key == 'x') {
	//	xkeypressed = true;
	//}
	if (key == '1') {
		followSpiral = !followSpiral; // 1 키로 스파이럴 따라 이동 활성화/비활성화
		if (followSpiral && spiralPath.empty()) {
			CalculateSpiralPath(); // 경로가 없으면 계산
		}
	}
	else if (key == '2') {
		rotateYEnabled = !rotateYEnabled;
	}

	else if (key == '3') {
		zigZagEnabled = !zigZagEnabled;
	}

	else if (key == '5') {
		rotate1XEnabled = !rotate1XEnabled;
		rotate2XEnabled = !rotate2XEnabled;
		rotateYEnabled = !rotateYEnabled;
		MaxingEnabled = !MaxingEnabled; // 5 키로 확대 시작/멈춤
		SmallingEnabled = !SmallingEnabled;
	}


	// 'c' 키를 눌렀을 때 renderHorn 플래그를 토글
	if (key == 'c') {
		renderHorn = !renderHorn;
		renderSphere = !renderSphere;
	}

	else if (key == 's') {
		Reset();  // 상태를 초기화하고 원래 상태로 돌아감
	}


	if ((MaxingEnabled || SmallingEnabled || followSpiral || rotate1XEnabled || rotate2XEnabled || rotateYEnabled || zigZagEnabled) && !isTimerRunning) {
		isTimerRunning = true;
		glutTimerFunc(0, Timer, 0);
	}
}



