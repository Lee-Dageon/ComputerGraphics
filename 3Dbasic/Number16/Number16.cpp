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
int spiralIndex = 0;       // ���� ������ ��ġ�� �����̷��� �ε���
bool followSpiral = false; // ������ �������� ���� �̵��ϴ��� ����



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
	 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // �ո� ����
	 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // �޸� �ʷ�
	 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // ���� �Ķ�
	 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, // �Ʒ��� ���
	 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // ���� �� ��ȫ��
	 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f  // ������ �� û�ϻ�
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
};		//ť�� �ҷ����� Ŭ����

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

class Corn {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// �⺻ ������: �⺻ ���� �̸� "corn.obj"�� �ε�
	Corn() {
		if (LoadOBJ("corn.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load corn OBJ file." << std::endl;
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

		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		// �ε��� ������ ����
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	// Load .obj ����
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

				// �� �鿡 ���� ���� ���� �߰� (��: RGB �� ��Ұ� 0.0f ~ 1.0f ������ ������)
				GLfloat r = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat g = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat b = static_cast<GLfloat>(rand()) / RAND_MAX;

				// �� ���� 3���� ������ ������ ���� ����
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

class Sphere {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLfloat> colors;
	GLuint VAO = 0, VBO = 0, EBO = 0, colorVBO = 0;

	// �⺻ ������: �⺻ ���� �̸� "sphere.obj"�� �ε�
	Sphere() {
		if (LoadOBJ("sphere.obj")) {
			InitBuffer();
		}
		else {
			std::cerr << "Failed to load sphere OBJ file." << std::endl;
		}
	}

	// ���۸� �ʱ�ȭ�Ͽ� OpenGL���� ������ �����ϰ� ����
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

		// ���� ������ ���� - �� �鿡 ������ ���� ����
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
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
			}
			else if (prefix == "f") {
				// �� ������
				GLuint index;
				for (int i = 0; i < 3; i++) {
					ss >> index;
					indices.push_back(index - 1); // 1-based�� 0-based �ε����� ��ȯ
				}

				// �� �鿡 ���� ���� ���� �߰� (��: RGB �� ��Ұ� 0.0f ~ 1.0f ������ ������)
				GLfloat r = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat g = static_cast<GLfloat>(rand()) / RAND_MAX;
				GLfloat b = static_cast<GLfloat>(rand()) / RAND_MAX;

				// �� ���� 3���� ������ ������ ���� ����
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
	const float spiralRadius = 0.04f;  // �������� ���ݰ� ũ�⸦ �����ϴ� ����
	const float spiralHeight = 0.00f; // y������ �������� ����ϴ� ���� ����
	const int numTurns = 5;       // �������� ȸ�� ��
	const int numPointsPerTurn = 100; // �� ȸ���� ���� ����

	std::vector<glm::vec3> spiralVertices;

	for (int i = 0; i < numTurns * numPointsPerTurn; ++i) {
		float angle = glm::radians(i * (360.0f / numPointsPerTurn));  // ���� ���
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

	// �޸� ����
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
bool MaxingEnabled = false; // Ȯ�� ����
bool SmallingEnabled = false;

bool updownMove = false;

float Scale1 = 0.12f;          // ť���� �ʱ� ũ��
float Scale2 = 0.2f;


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	//-------------------------------------------------------------------------------------------------------------------
	glm::mat4 model = glm::mat4(1.0f);
	
	if (rotateYEnabled) {
		// ������ �� y ���� �߽����� x, z ��ǥ�� ���� �˵��� �׸����� ����
		float x = orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	else if (zigZagEnabled) {
		// ������ �� y ���� �߽����� x, z ��ǥ�� ���� �˵��� �׸����� ����
		float x = orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	// ������ �����̷� ��θ� ���� �̵��� ���
	else if (followSpiral && spiralIndex < spiralPath.size()) {
		glm::vec3 spiralPos = -spiralPath[spiralIndex];
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
		model = glm::translate(model, spiralPos); // �����̷� ��ġ�� �̵�
	}

	else {
		// ������ ��Ȱ��ȭ�Ǿ��� �� �⺻ ��ġ
		model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
	}

	model = glm::scale(model, glm::vec3(Scale1, Scale1, Scale1));
	model = glm::rotate(model, glm::radians(rotation1X), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	// renderHorn �÷��׿� ���� cube �Ǵ� horn ������
	if (renderHorn) {
		horn->Render();
	}
	else {
		cube->Render();
	}
	//-----------------------------------------------------------------------------------------------

	// �� ������ (ȸ�� ����)
	model = glm::mat4(1.0f);
	// �� ȸ���� ���� 3D �� ��Ʈ���� ����
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	//-----------------------------------------------------------------------------------------------------
	// 
	// ����(Corn) ������ (ũ�� 0.5�� ���, X/Y�� ȸ�� ����)
	model = glm::mat4(1.0f);
	if (rotateYEnabled) {
		// ������ �� y ���� �߽����� x, z ��ǥ�� ���� �˵��� �׸����� ����
		float x = orbityRadius * cos(glm::radians(orbityAngle));
		float z = orbityRadius * sin(glm::radians(orbityAngle));
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	// ������ �����̷� ��θ� ���� �̵��� ���
	else if (followSpiral && spiralIndex < spiralPath.size()) {
		glm::vec3 spiralPos = spiralPath[spiralIndex];
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
		model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
		model = glm::translate(model, spiralPos); // �����̷� ��ġ�� �̵�
	}

	else if (zigZagEnabled) {
		// ������ �� y ���� �߽����� x, z ��ǥ�� ���� �˵��� �׸����� ����
		float x = -orbityRadiusCube * cos(glm::radians(orbityAngle));
		float z = orbityRadiusCube * sin(glm::radians(orbityAngle));
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
	}

	else {
		// ������ ��Ȱ��ȭ�Ǿ��� �� �⺻ ��ġ
		model = glm::translate(model, glm::vec3(-0.5f, 0.0f, 0.0f));
	}


	model = glm::scale(model, glm::vec3(Scale2, Scale2, Scale2));
	model = glm::rotate(model, glm::radians(rotation2X), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	// renderHorn �÷��׿� ���� cube �Ǵ� horn ������
	if (renderSphere) {
		sphere->Render();
	}
	else {
		corn->Render();
	}


	model = glm::mat4(1.0f);
	// �� ȸ���� ���� 3D �� ��Ʈ���� ���� 
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(1.0f, 0.0f, 0.0f)); // X������ 45�� ȸ��
	model = glm::rotate(model, glm::radians(howmuchrotate), glm::vec3(0.0f, 1.0f, 0.0f)); // Y������ 45�� ȸ��
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE,glm::value_ptr(model));
	DrawSpiral();


	glutSwapBuffers();
}

// Ÿ�̸� �ݹ� �Լ�


bool isTimerRunning = false;  // Ÿ�̸Ӱ� ���� ������ Ȯ���ϴ� �÷���

void Timer(int value) {

	if (MaxingEnabled) {
		if (Scale1 <= 0.3f) {
			Scale1 += 0.001f; // �������� ���������� ����
		}

		else {
			Scale1 = 0.12f;
		}
	}

	if (SmallingEnabled) {
		if (Scale2 >= 0.0f) {
			Scale2 -= 0.001f; // �������� ���������� ����		
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
		// spiralPath�� ����� ��ġ�� �̵�
		spiralIndex = (spiralIndex + 1) % spiralPath.size(); // �ε����� ��ȯ�ϸ� ����
	}


	if (MaxingEnabled || SmallingEnabled || followSpiral || rotate1XEnabled || rotate2XEnabled || rotateYEnabled || zigZagEnabled) {
		glutTimerFunc(16, Timer, 0);  // Ÿ�̸Ӹ� �ٽ� ȣ���Ͽ� �ִϸ��̼��� ����
	}
	else {
		isTimerRunning = false;  // ȸ���� ��� ��Ȱ��ȭ�Ǹ� Ÿ�̸� ����
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

	// �簢�� ��ü ����
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


// �۷ι� �÷���
bool xKeyPressed = false;

// �ʱ�ȭ ���� ����
const GLfloat initialRotation1X = 30.0f;
const GLfloat initialRotation2X = 30.0f;
const GLfloat initialRotationY = 30.0f;
const float initialOrbityAngle = 0.0f;

void Reset() {
	// ������ �ʱ� ���·� �ǵ���
	rotation1X = initialRotation1X;
	rotation2X = initialRotation2X;
	rotationY = initialRotationY;
	orbityAngle = initialOrbityAngle;

	// ��� ȸ�� �÷��׸� ��Ȱ��ȭ
	rotate1XEnabled = false;
	rotate2XEnabled = false;
	rotateYEnabled = false;

	// Ÿ�̸� �ߺ� ȣ�� ���� �÷��� ����
	isTimerRunning = false;
	renderHorn = false;
	renderSphere = false;

	Scale1 = 0.12f;
	Scale2 = 0.2f;

	// ȭ�� ����
	glutPostRedisplay();
}


void Keyboard(unsigned char key, int x, int y) {
	//if (key == 'x') {
	//	xkeypressed = true;
	//}
	if (key == '1') {
		followSpiral = !followSpiral; // 1 Ű�� �����̷� ���� �̵� Ȱ��ȭ/��Ȱ��ȭ
		if (followSpiral && spiralPath.empty()) {
			CalculateSpiralPath(); // ��ΰ� ������ ���
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
		MaxingEnabled = !MaxingEnabled; // 5 Ű�� Ȯ�� ����/����
		SmallingEnabled = !SmallingEnabled;
	}


	// 'c' Ű�� ������ �� renderHorn �÷��׸� ���
	if (key == 'c') {
		renderHorn = !renderHorn;
		renderSphere = !renderSphere;
	}

	else if (key == 's') {
		Reset();  // ���¸� �ʱ�ȭ�ϰ� ���� ���·� ���ư�
	}


	if ((MaxingEnabled || SmallingEnabled || followSpiral || rotate1XEnabled || rotate2XEnabled || rotateYEnabled || zigZagEnabled) && !isTimerRunning) {
		isTimerRunning = true;
		glutTimerFunc(0, Timer, 0);
	}
}



