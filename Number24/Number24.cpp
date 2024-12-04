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
Horn* horn;
Shader* shader;
GLfloat rotationX = 0.0f;
GLfloat rotationY = 0.0f;
// �̵� ���� �ʱ�ȭ
float translationX = 0.0f;
float translationY = 0.0f;
float moveSpeed = 0.1f; // �̵� �ӵ�

bool drawCube = false; // ť��� �簢���� ������ �׸� ���� ����
bool drawHorn = false; // ť��� �簢���� ������ �׸� ���� ����
bool cullEnabled = false;     // ���� ���� ���� ����
bool rotateXEnabled = false;     // ���� ���� ���� ����
bool rotateYEnabled = false;     // ���� ���� ���� ����
void Keyboard(unsigned char key, int x, int y);
void SpecialKeys(int key, int x, int y);

// Ÿ�̸� �ݹ� �Լ�

void Timer(int value) {
	if (rotateXEnabled) {
		rotationX += 0.8f; // rotationX�� �����ϰ� ����
		if (rotationX >= 360.0f) {
			rotationX -= 360.0f; // ���� 360���� ������ �ʱ�ȭ
		}
		glutPostRedisplay();     // ȭ���� �ٽ� �׸��� ��û
		glutTimerFunc(16, Timer, 0); // 16ms �Ŀ� Ÿ�̸� �Լ� ��ȣ�� (�� 60 FPS)
	}

	if (rotateYEnabled) {
		rotationY += 0.8f; // rotationX�� �����ϰ� ����
		if (rotationY >= 360.0f) {
			rotationY -= 360.0f; // ���� 360���� ������ �ʱ�ȭ
		}
		glutPostRedisplay();     // ȭ���� �ٽ� �׸��� ��û
		glutTimerFunc(16, Timer, 0); // 16ms �Ŀ� Ÿ�̸� �Լ� ��ȣ�� (�� 60 FPS)
	}
}


void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// 1. ��, ��, ���� ��� ����
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(translationX, translationY, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(shader->programID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader->programID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader->programID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// 2. ���� ���� uniform ���� ����
	glm::vec3 lightPos(10.0f, 1.0f, 2.0f); // ������ ��ġ
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // ������ ���� (���)
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f); // ��ü�� ���� (��Ȳ��)
	glm::vec3 viewPos(0.0f, 0.0f, 3.0f); // ī�޶� ��ġ

	glUniform3fv(glGetUniformLocation(shader->programID, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shader->programID, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(shader->programID, "objectColor"), 1, glm::value_ptr(objectColor));
	glUniform3fv(glGetUniformLocation(shader->programID, "viewPos"), 1, glm::value_ptr(viewPos));

	// 3. ��ü ������
	if (drawCube) cube->Render();
	if (drawHorn) horn->Render();
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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	cube = new Cube();
	axis = new Axis();
	// �簢�� ��ü ����
	horn = new Horn();
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // ����Ű �Է� �Լ� ���
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}


void Keyboard(unsigned char key, int x, int y) {

	if (key == 'c') {
		drawCube = !drawCube; // 'c' Ű�� ������ ���¸� ���
		glutPostRedisplay(); // ȭ���� �ٽ� �׸��� ��û
	}

	else if (key == 'p') {
		drawHorn = !drawHorn; // 'c' Ű�� ������ ���¸� ���
		glutPostRedisplay(); // ȭ���� �ٽ� �׸��� ��û
	}

	else if (key == 'h') {
		cullEnabled = !cullEnabled; // 'h' Ű�� ������ ���� ���� ���
		glutPostRedisplay();        // ȭ���� �ٽ� �׸��� ��û
	}

	else if (key == 'x') {
		rotateXEnabled = !rotateXEnabled; // 'x' Ű�� ������ rotationX Ÿ�̸� ���
		if (rotateXEnabled) {
			glutTimerFunc(0, Timer, 0);   // Ÿ�̸� ����
		}
	}

	else if (key == 'y') {
		rotateYEnabled = !rotateYEnabled; // 'x' Ű�� ������ rotationX Ÿ�̸� ���
		if (rotateYEnabled) {
			glutTimerFunc(0, Timer, 0);   // Ÿ�̸� ����
		}
	}

	else if (key == 'w') {// ���̾������� ���
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
	}


	else if (key == 'W') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glutPostRedisplay();
	}

	else if (key == 's') {
		translationX = 0.0f;
		translationY = 0.0f;

		// rotateYEnabled�� rotateXEnabled�� false�� ����
		rotateYEnabled = false;
		rotateXEnabled = false;


		glutPostRedisplay();

	}
}

void SpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		translationY += moveSpeed;  // ���� �̵�
		break;
	case GLUT_KEY_DOWN:
		translationY -= moveSpeed;  // �Ʒ��� �̵�
		break;
	case GLUT_KEY_LEFT:
		translationX -= moveSpeed;  // ���� �̵�
		break;
	case GLUT_KEY_RIGHT:
		translationX += moveSpeed;  // ������ �̵�
		break;
	}
	glutPostRedisplay(); // ȭ���� �ٽ� �׸��� ��û
}



