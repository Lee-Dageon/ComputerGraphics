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
float orbitAngle = 0.0f; // ���� ����
float orbitAngle2 = 0.0f; // �� ��° ���� ����
float orbitAngle3 = 0.0f; // �� ��° ���� ����

float orbitAngle4 = 0.0f; // ù ��° ���� �༺�� �߽����� �����ϴ� �༺�� ����
float orbitAngle5 = 0.0f; // ù ��° ���� �༺�� �߽����� �����ϴ� �༺�� ����
float orbitAngle6 = 0.0f; // �� ��° ���� �༺�� �߽����� �����ϴ� �༺�� ����

bool usePerspective = true; // ���� ����(true) / ���� ����(false)
bool useWireframe = false;  // �ָ���(false) / ���̾�������(true)

glm::vec3 translation = glm::vec3(0.0f); // x, y, z �̵�
float secondaryRotationZ = 0.0f;         // z�� ȸ�� (�߽� �� ����)

void Keyboard(unsigned char key, int x, int y);


struct Vertex {
	float x, y, z;
};

class Shader {        // ���̴� �ҷ����� Ŭ����
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

class Sphere {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO, colorVBO;

	Sphere() {
		LoadOBJ("sphere.obj");
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

		// ��� ������ ������ �Ķ������� ����
		std::vector<GLfloat> colors(vertices.size() * 3, 0.0f);
		for (size_t i = 0; i < vertices.size(); ++i) {
			colors[i * 3 + 2] = 1.0f; // �Ķ��� (B ���� 1.0f�� ����)
		}

		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
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
};        // �� �ҷ����� Ŭ����

class Orbit {
public:
	GLuint VAO, VBO;
	static const int segments = 100;
	float radius;

	Orbit(float radius) : radius(radius) {
		InitBuffer();
	}

	void InitBuffer() {
		std::vector<GLfloat> vertices;
		for (int i = 0; i < segments; ++i) {
			float angle = 2.0f * glm::pi<float>() * i / segments;
			vertices.push_back(radius * cos(angle));
			vertices.push_back(0.0f);
			vertices.push_back(radius * sin(angle));
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
	}

	void Render() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_LOOP, 0, segments);
		glBindVertexArray(0);
	}
};  //���� �˵� Ŭ����

class Axis {        // �� �ҷ����� Ŭ����
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
};        // �� �ҷ����� Ŭ����

Sphere* sphere;
Sphere* orbitingSphere;
Sphere* orbitingSphere2;
Sphere* orbitingSphere3;
Axis* axis;
Shader* shader;
Orbit* orbit;
Orbit* orbit2;
Orbit* orbit3;

Orbit* orbit4; // ù ��° ���� �༺�� �����ϴ� �༺�� �˵�
Orbit* orbit5; // �� ��° ���� �༺�� �����ϴ� �༺�� �˵�
Orbit* orbit6; // �� ��° ���� �༺�� �����ϴ� �༺�� �˵�

glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);   // ī�޶� ��ġ
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // ī�޶� �ٶ󺸴� ����
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);    // '����' ����
float cameraSpeed = 0.1f;                            // ī�޶� �̵� �ӵ�

void Timer(int value) {
	orbitAngle += 0.5f; // ���� ���� ����
	if (orbitAngle > 360.0f) {
		orbitAngle -= 360.0f;
	}

	orbitAngle2 += 1.0f; // �� ��° ���� ���� ����
	if (orbitAngle2 > 360.0f) {
		orbitAngle2 -= 360.0f;
	}

	orbitAngle3 += 0.3f; // �� ��° ���� ���� ����
	if (orbitAngle3 > 360.0f) {
		orbitAngle3 -= 360.0f;
	}

	orbitAngle4 += 2.8f; // �� ��° ���� ���� ����
	if (orbitAngle4 > 360.0f) {
		orbitAngle4 -= 360.0f;
	}

	orbitAngle5 += 2.8f; // �� ��° ���� ���� ����
	if (orbitAngle5 > 360.0f) {
		orbitAngle5 -= 360.0f;
	}

	orbitAngle6 += 2.8f; // �� ��° ���� ���� ����
	if (orbitAngle6 > 360.0f) {
		orbitAngle6 -= 360.0f;
	}
	glutPostRedisplay(); // ȭ�� ���� ��û
	glutTimerFunc(16, Timer, 0); // �� 60fps�� Ÿ�̸� ȣ��
}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, upVector);
	unsigned int viewLocation = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;
	if (usePerspective) {
		// ���� ����
		projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	}
	else {
		// ���� ����
		float orthoScale = 0.8f;
		projection = glm::ortho(-orthoScale, orthoScale, -orthoScale, orthoScale, 0.1f, 100.0f);
	}
	unsigned int projectionLocation = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// �� ��� ���� �� ������
	glm::mat4 model = glm::mat4(1.0f);

	// �߽� �� ������ (ũ�� 0.5�� ���, X/Y�� ȸ�� ����)
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "model");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	sphere->Render();

	 // ù ��° ���� �༺ ������
    float orbitRadius = 0.6f;
    glm::vec3 orbitingPlanetPos = glm::vec3(orbitRadius * cos(glm::radians(orbitAngle)), 0.0f, orbitRadius * sin(glm::radians(orbitAngle)));
    model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, orbitingPlanetPos);
    model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// �����ϴ� ���� ���� �����͸� �ʷϻ����� ����
	std::vector<GLfloat> orbitColors(orbitingSphere->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere->vertices.size(); ++i) {
		orbitColors[i * 3 + 1] = 1.0f; // �ʷϻ� (G ���� 1.0f�� ����)
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors.size() * sizeof(GLfloat), orbitColors.data(), GL_STATIC_DRAW);
	orbitingSphere->Render();


	// ���� �༺ �ֺ��� ������ �༺ ������
	float orbitRadius4 = 0.15f; // ���� ������
	glm::vec3 smallOrbitPos = glm::vec3(orbitRadius4 * cos(glm::radians(orbitAngle4)), 0.0f, orbitRadius4 * sin(glm::radians(orbitAngle4)));
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos + smallOrbitPos);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	

	// ������ ����
	std::vector<GLfloat> orbitColors4(orbitingSphere->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere->vertices.size(); ++i) {
		orbitColors4[i * 3] = 1.0f; // R ���� 1.0���� ����
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors4.size() * sizeof(GLfloat), orbitColors4.data(), GL_STATIC_DRAW);
	orbitingSphere->Render();


	// ù ��° ���� �༺�� �����ϴ� �༺�� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos); // ù ��° ���� �༺�� ��ġ�� �̵�
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit4->Render();


	
	// �� ��° ���� �༺ ������
	float orbitRadius2 = 0.6f; // �� ��° ���� �˵��� ������
	glm::mat4 tiltedOrbit = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 45�� ����
	glm::vec3 orbitingPlanetPos2 = glm::vec3(
		orbitRadius2 * cos(glm::radians(orbitAngle2)),
		0.0f,
		orbitRadius2 * sin(glm::radians(orbitAngle2))
	);
	orbitingPlanetPos2 = glm::vec3(tiltedOrbit * glm::vec4(orbitingPlanetPos2, 1.0f)); // 45�� ���� ����

	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos2);
	model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// �ʷϻ� ����
	std::vector<GLfloat> orbitColors2(orbitingSphere2->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere2->vertices.size(); ++i) {
		orbitColors2[i * 3 + 1] = 1.0f; // �ʷϻ�
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere2->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors.size() * sizeof(GLfloat), orbitColors.data(), GL_STATIC_DRAW);
	orbitingSphere2->Render();

	// �� ��° ���� �༺ �ֺ��� ������ �༺ ������
	float orbitRadius5 = 0.15f; // �� ��° ���� �༺ ������ �˵� ������
	glm::vec3 smallOrbitPos2 = glm::vec3(
		orbitRadius5 * cos(glm::radians(orbitAngle5)),
		0.0f,
		orbitRadius5 * sin(glm::radians(orbitAngle5))
	);
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos2 + smallOrbitPos2); // �� ��° ���� �༺ �������� ���� �༺ �̵�
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// ������ ����
	std::vector<GLfloat> orbitColors5(orbitingSphere2->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere2->vertices.size(); ++i) {
		orbitColors5[i * 3] = 1.0f; // ������
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere2->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors5.size() * sizeof(GLfloat), orbitColors5.data(), GL_STATIC_DRAW);
	orbitingSphere2->Render();

	// �� ��° ���� �༺�� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos2); // �� ��° ���� �༺�� ��ġ�� �̵�
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit5->Render();


	// �� ��° ���� �༺ ������
	float orbitRadius3 = 0.6f; // �� ��° ���� �༺�� �˵� ������
	glm::mat4 tiltedOrbit3 = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // -45�� ����
	glm::vec3 orbitingPlanetPos3 = glm::vec3(
		orbitRadius3 * cos(glm::radians(orbitAngle3)),
		0.0f,
		orbitRadius3 * sin(glm::radians(orbitAngle3))
	);
	orbitingPlanetPos3 = glm::vec3(tiltedOrbit3 * glm::vec4(orbitingPlanetPos3, 1.0f)); // -45�� ���� ����


	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos3);
	model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// �ʷϻ� ����
	std::vector<GLfloat> orbitColors3(orbitingSphere3->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere3->vertices.size(); ++i) {
		orbitColors3[i * 3 + 1] = 1.0f; // �ʷϻ�
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere3->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors3.size() * sizeof(GLfloat), orbitColors3.data(), GL_STATIC_DRAW);
	orbitingSphere3->Render();

	// �� ��° ���� �༺ �ֺ��� ������ �༺ ������
	float orbitRadius6 = 0.15f; // �� ��° ���� �༺ ������ �˵� ������
	glm::vec3 smallOrbitPos3 = glm::vec3(
		orbitRadius6 * cos(glm::radians(orbitAngle6)), // ���ο� ���� ����
		0.0f,
		orbitRadius6 * sin(glm::radians(orbitAngle6))
	);
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, orbitingPlanetPos3 + smallOrbitPos3); // �� ��° ���� �༺ �������� ���� �༺ �̵�
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// ������ ����
	std::vector<GLfloat> orbitColors6(orbitingSphere3->vertices.size() * 3, 0.0f);
	for (size_t i = 0; i < orbitingSphere3->vertices.size(); ++i) {
		orbitColors6[i * 3] = 1.0f; // ������
	}
	glBindBuffer(GL_ARRAY_BUFFER, orbitingSphere3->colorVBO);
	glBufferData(GL_ARRAY_BUFFER, orbitColors6.size() * sizeof(GLfloat), orbitColors6.data(), GL_STATIC_DRAW);
	orbitingSphere3->Render();

	// �� ��° ���� �༺�� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, orbitingPlanetPos3); // �� ��° ���� �༺�� ��ġ�� �̵�
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit6->Render();




	// ���� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit->Render();


	// �� ��° ���� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 45�� ����̱�
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit->Render();

	// �� ��° ���� �˵� ������
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(secondaryRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 45�� ����̱�
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	orbit->Render();





	// ������ �� ������
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	//axis->Render();

	if (useWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // ���̾�������
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // �ָ���
	}

	glutSwapBuffers();
}


int main(int argc, char** argv) {
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Sphere and Axis");

	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	sphere = new Sphere();

	orbit = new Orbit(0.6f); // ���� �˵� ��ü ����
	orbit2 = new Orbit(0.6f); // �� ��° ���� �˵� ��ü ����
	orbit3 = new Orbit(0.6f); // �� ��° ���� �˵� ��ü ����
	orbit4 = new Orbit(0.15f); // ù ��° ���� �༺�� �����ϴ� �˵�
	orbit5 = new Orbit(0.15f); // �� ��° ���� �༺�� �����ϴ� �˵�
	orbit6 = new Orbit(0.15f); // �� ��° ���� �༺�� �����ϴ� �˵�


	orbitingSphere = new Sphere(); // �����ϴ� �� ��ü ����
	orbitingSphere2 = new Sphere(); // �� ��° �����ϴ� �� ��ü ����
	orbitingSphere3 = new Sphere(); // �� ��° �����ϴ� �� ��ü ����

	axis = new Axis();
	// �� ��ü ����
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Timer, 0); // Ÿ�̸� �Լ� ����
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}


void Keyboard(unsigned char key, int x, int y) {
	// ī�޶� �̵� ���� ���
	glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 right = glm::normalize(glm::cross(cameraDirection, upVector));


	switch (key) {
	case 'p': // ���� ����
	case 'P':
		usePerspective = !usePerspective; // ���� ��� ��ȯ
		break;

	case 'm': // ���̾������� ��
	case 'M':
		useWireframe = !useWireframe; // ������ ��� ��ȯ
		break;

	case 's': // ���� �̵�
		cameraPos += cameraSpeed * upVector;      // ī�޶� ��ġ�� ���� �̵�
		cameraTarget += cameraSpeed * upVector;  // ī�޶� ���⵵ ���� �̵�
		break;
	case 'w': // �Ʒ��� �̵�
		cameraPos -= cameraSpeed * upVector;      // ī�޶� ��ġ�� �Ʒ��� �̵�
		cameraTarget -= cameraSpeed * upVector;  // ī�޶� ���⵵ �Ʒ��� �̵�
		break;
	case 'd': // �������� �̵�
		cameraPos -= glm::normalize(glm::cross(cameraDirection, upVector)) * cameraSpeed;
		cameraTarget -= glm::normalize(glm::cross(cameraDirection, upVector)) * cameraSpeed;
		break;
	case 'a': // ���������� �̵�
		cameraPos += glm::normalize(glm::cross(cameraDirection, upVector)) * cameraSpeed;
		cameraTarget += glm::normalize(glm::cross(cameraDirection, upVector)) * cameraSpeed;
		break;

	case '+': // ������ �̵�
		cameraPos += cameraSpeed * cameraDirection;
		cameraTarget += cameraSpeed * cameraDirection;
		break;
	case '-': // �ڷ� �̵�
		cameraPos -= cameraSpeed * cameraDirection;
		cameraTarget -= cameraSpeed * cameraDirection;
		break;

	case 'y': // ��ü ��ü�� y�� �������� �� ���� ȸ��
		rotationY += 2.0f;
		break;

	case 'Y': // ��ü ��ü�� y�� �������� �� ���� ȸ��
		rotationY -= 2.0f;
		break;

	case 'z': // z�� �������� �� ���� ȸ�� (�߽� �� ����)
		secondaryRotationZ -= 5.0f;
		break;

	case 'Z': // z�� �������� �� ���� ȸ�� (�߽� �� ����)
		secondaryRotationZ += 5.0f;
		break;

	default:
		break;
	}
	glutPostRedisplay(); // ȭ�� ����
}

