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
GLfloat howmuchrotationX = -30.0f; 
GLfloat howmuchrotationY = 30.0f;


glm::vec3 cameraPos = glm::vec3(0.5f, 1.0f, 3.0f); // �ʱ� ī�޶� ��ġ
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // ī�޶� �ٶ󺸴� ��
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // ī�޶� ��� ����

glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f); // �κ��� �߽���


bool animateRotation = false; // �ִϸ��̼� ����
float cameraAngle = 0.0f;   // ī�޶� ��ü ȸ�� ����

bool isMovingX = false; // �Ʒ� ��ü�� �̵� ���� (true: �̵�, false: ����)
bool isMovingminusX = false;
float bodyXPosition = 0.0f; // �Ʒ� ��ü�� ���� X�� ��ġ
float moveSpeed = 0.01f; // �̵� �ӵ�
int moveDirection = 0; // 1: �� ����, -1: �� ����, 0: ����

bool isRotatingY = false; // �߾� ��ü�� ȸ�� ���� (true: ȸ�� ��, false: ����)
float centralRotationAngle = 0.0f; // �߾� ��ü�� ���� ȸ�� ����
float rotationSpeed = 5.0f; // ȸ�� �ӵ� (1�ʴ� 5��)
int centerRotationDirection = 0; // ȸ�� ���� (1: ��, -1: ��, 0: ����)

bool isRotatingGunBarrel = false; // ���� ȸ�� ����
float gunBarrelRotationAngle1 = 0.0f; // ���� 1�� ȸ�� ����
float gunBarrelRotationAngle2 = 0.0f; // ���� 2�� ȸ�� ����
float gunBarrelRotationSpeed = 5.0f;  // ���� ȸ�� �ӵ�
int gunBarrelRotationDirection = 0;

bool isMerging = false;     // ���� �߾����� �̵� �� ����
bool isReturning = false;   // ���� ���� �ڸ��� �̵� �� ����
float gunBarrelMergeOffset = 0.0f; // ���� �̵� �Ÿ�
float mergeSpeed = 0.01f;   // �̵� �ӵ�

bool isCraneRotating = false;    // ũ���� �� ȸ�� ����
float craneArmAngle1 = 0.0f;     // �� 1�� ���� ����
float craneArmAngle2 = 0.0f;     // �� 2�� ���� ����
float craneRotationSpeed = 0.5f; // �� ȸ�� �ӵ�
int craneRotationDirection = 0;  // ȸ�� ���� (1: ��, -1: ��, 0: ����)

void Keyboard(unsigned char key, int x, int y);
void Update(int value);

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


void drawRobot(Shader* shader) { 
	glm::mat4 model = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");

	// 1. �Ʒ� ��ü (�⺻ ť�� ũ�� �״�� ������)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X�� �̵� �ݿ�
	model = glm::scale(model, glm::vec3(0.5f, 0.3f, 0.5f)); // ũ�� ����
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 2. �߾� ��ü (���� ���� ��ġ�� ���� ť��)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X�� �̵� �ݿ�
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ȸ�� �ݿ�
	model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f)); // Y������ �ø�
	model = glm::scale(model, glm::vec3(0.3f, 0.1f, 0.3f)); // ũ�� ���
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 3. �� 1 (����)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X�� �̵� �ݿ�
	model = glm::rotate(model, glm::radians(craneArmAngle1), glm::vec3(0.0f, 0.0f, 1.0f)); // Z�� ȸ�� �ݿ�
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // �߾� ��ü�� ������ ȸ��
	model = glm::translate(model, glm::vec3(-0.1f, 0.3f, 0.0f)); // �������� �ణ �̵� �� ���� �ø�
	model = glm::scale(model, glm::vec3(0.05f, 0.2f, 0.05f)); // ������ ũ�� ���
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 4. �� 2 (������)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X�� �̵� �ݿ�
	model = glm::rotate(model, glm::radians(craneArmAngle2), glm::vec3(0.0f, 0.0f, 1.0f)); // Z�� ȸ�� �ݿ�
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ȸ�� �ݿ�
	model = glm::translate(model, glm::vec3(0.1f, 0.3f, 0.0f)); // ���������� �ణ �̵� �� ���� �ø�
	model = glm::scale(model, glm::vec3(0.05f, 0.2f, 0.05f)); // ������ ũ�� ���
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 5. ���� 1 (����)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.15f + gunBarrelMergeOffset, -0.05f, 0.3f)); // ���� �Ʒ����� �������� ����
	model = glm::rotate(model, gunBarrelRotationAngle1, glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� ���� �ݿ�
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.2f)); // ������ ũ�� ����
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 6. ���� 2 (������)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.15f - gunBarrelMergeOffset, -0.05f, 0.3f)); // ������ �Ʒ����� �������� ����
	model = glm::rotate(model, gunBarrelRotationAngle2, glm::vec3(0.0f, 1.0f, 0.0f)); // Y�� �ݴ� ���� ���� �ݿ�
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.2f)); // ������ ũ�� ����
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

}



void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// �� ���: ī�޶� ��ġ�� ���� ����
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// �������� ���: ���� ����
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);


	glm::mat4 model = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");

	unsigned int viewLoc = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	unsigned int projLoc = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// ������ �� ������
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// �κ� ������ (���� + �Ӹ�)
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // ũ�� ����
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	drawRobot(shader);

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
	// �簢�� ��ü ����
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard); // Ű���� �Է� ���
	glutTimerFunc(16, Update, 0); // �ִϸ��̼� Ÿ�̸� ���
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}

void Keyboard(unsigned char key, int x, int y) {
	const float moveSpeed = 0.1f;
	const float rotateSpeed = 1.0f;

	switch (key) {
	case 'z': // z�� �� ���� �̵�
		cameraPos.z -= moveSpeed;
		break;
	case 'Z': // z�� �� ���� �̵�
		cameraPos.z += moveSpeed;
		break;
	case 'x': // x�� �� ���� �̵�
		cameraPos.x += moveSpeed;
		break;
	case 'X': // x�� �� ���� �̵�
		cameraPos.x -= moveSpeed;
		break;

	case 'y': // ī�޶� �ڱ� �ڽ��� �������� �ð� ���� ȸ��

		cameraAngle += glm::radians(rotateSpeed); // ȸ�� ������ ����
		cameraTarget = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)
		) + cameraPos; // ���ο� �ٶ󺸴� ���� ���

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;


	case 'Y': // ī�޶� �ڱ� �ڽ��� �������� �ݽð� ���� ȸ��
		cameraAngle -= glm::radians(rotateSpeed); // ȸ�� ������ ����
		cameraTarget = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)
		) + cameraPos; // ���ο� �ٶ󺸴� ���� ���

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'r': // y�� ���� ���� (ȭ�� �߽� ���� ȸ��)
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // ī�޶� �ٶ󺸴� ��
		cameraAngle += glm::radians(rotateSpeed);
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'R': // y�� ���� ���� �ݴ� ����
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // ī�޶� �ٶ󺸴� ��
		cameraAngle -= glm::radians(rotateSpeed);
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'a': // ���� �ִϸ��̼� ����/����
		animateRotation = true;
		break;

	case 'A':
		animateRotation = false;
		break;

	case '+': // z�� �� ���� �̵�
		cameraPos.y += moveSpeed;
		break;
	case '-': // z�� �� ���� �̵�
		cameraPos.y -= moveSpeed;
		break;

	case 'b': // �̵� ���� ��/������ ���
		if (moveDirection == 1) moveDirection = 0; // ����
		else moveDirection = 1; // �� ���� �̵�
		break;

	case 'B':
		if (moveDirection == -1) moveDirection = 0; // ����
		else moveDirection = -1; // �� ���� �̵�
		break;

	case 'm': // �߾� ��ü ���� ���� ȸ�� ���
		if (isRotatingY && centerRotationDirection == 1) {
			isRotatingY = false;  // ȸ�� ����
			centerRotationDirection = 0;
		}
		else {
			isRotatingY = true;   // ȸ�� ����
			centerRotationDirection = 1; // ���� ����
		}
		break;

	case 'M': // �߾� ��ü ���� ���� ȸ�� ���
		if (isRotatingY && centerRotationDirection == -1) {
			isRotatingY = false;  // ȸ�� ����
			centerRotationDirection = 0;
		}
		else {
			isRotatingY = true;   // ȸ�� ����
			centerRotationDirection = -1; // ���� ����
		}
		break;


	case 'f': // ���� ���� ���� ȸ�� ���
		if (isRotatingGunBarrel && gunBarrelRotationDirection == 1) {
			isRotatingGunBarrel = false;  // ����
			gunBarrelRotationDirection = 0;
		}
		else {
			isRotatingGunBarrel = true;   // ����
			gunBarrelRotationDirection = 1;
		}
		break;


	case 'F': // ���� ���� ���� ȸ�� ���
		if (isRotatingGunBarrel && gunBarrelRotationDirection == -1) {
			isRotatingGunBarrel = false;  // ����
			gunBarrelRotationDirection = 0;
		}
		else {
			isRotatingGunBarrel = true;   // ����
			gunBarrelRotationDirection = -1;
		}
		break;


	case 'e': // ���� �߾����� �̵� ����
		if (!isMerging && !isReturning) {
			isMerging = true;  // �߾����� �̵� ����
		}
		break;

	case 'E': // ���� ���� �ڸ��� �̵� ����
		if (!isMerging && !isReturning) {
			isReturning = true; // ���� �ڸ��� �̵� ����
		}
		break;

	case 't': // ���� ���� ȸ�� ���
		if (isCraneRotating && craneRotationDirection == 1) {
			isCraneRotating = false;  // ����
			craneRotationDirection = 0;
		}
		else {
			isCraneRotating = true;   // ����
			craneRotationDirection = 1; // ���� ����
		}
		break;

	case 'T': // ���� ���� ȸ�� ���
		if (isCraneRotating && craneRotationDirection == -1) {
			isCraneRotating = false;  // ����
			craneRotationDirection = 0;
		}
		else {
			isCraneRotating = true;   // ����
			craneRotationDirection = -1; // ���� ����
		}
		break;

	case 's': // ��� ������ ���߱�
	case 'S':
		animateRotation = false; // ���� �ִϸ��̼� ����
		isMovingX = false;       // X�� �̵� ����
		isRotatingY = false;     // �߾� ��ü ȸ�� ����
		isRotatingGunBarrel = false; // ���� ȸ�� ����
		isMerging = false;       // ���� ���� ����
		isReturning = false;     // ���� ���� ����
		isCraneRotating = false; // ũ���� �� ȸ�� ����
		moveDirection = 0;       // �̵� ���� �ʱ�ȭ
		centerRotationDirection = 0; // �߾� ȸ�� ���� �ʱ�ȭ
		gunBarrelRotationDirection = 0; // ���� ȸ�� ���� �ʱ�ȭ
		craneRotationDirection = 0; // ũ���� ȸ�� ���� �ʱ�ȭ
		break;

	case 'c': // ��� ������ �ʱ�ȭ
	case 'C':
		animateRotation = false;
		isMovingX = false;
		isRotatingY = false;
		isRotatingGunBarrel = false;
		isMerging = false;
		isReturning = false;
		isCraneRotating = false;

		// ���� �ʱ�ȭ
		cameraPos = glm::vec3(0.5f, 1.0f, 3.0f);
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		bodyXPosition = 0.0f;
		centralRotationAngle = 0.0f;
		gunBarrelRotationAngle1 = 0.0f;
		gunBarrelRotationAngle2 = 0.0f;
		gunBarrelMergeOffset = 0.0f;
		craneArmAngle1 = 0.0f;
		craneArmAngle2 = 0.0f;

		moveDirection = 0;
		centerRotationDirection = 0;
		gunBarrelRotationDirection = 0;
		craneRotationDirection = 0;

		std::cout << "All movements reset to initial state." << std::endl;
		break;

	case 'q': // ���α׷� ����
	case 'Q':
		exit(0); // ���α׷� ����
		break;



	default:
		break;
	}

	glutPostRedisplay(); // ȭ�� ���� ��û
}


void Update(int value) {

	if (animateRotation) {
		cameraAngle += glm::radians(1.0f); // �ִϸ��̼� �ӵ�
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;
		glutPostRedisplay();
	}

	// �̵� ���⿡ ���� �Ʒ� ��ü ��ġ ������Ʈ
	if (moveDirection != 0) {
		bodyXPosition += moveDirection * moveSpeed; // �̵� ����(1 �Ǵ� -1) ���ϱ� �ӵ�
		glutPostRedisplay();
	}

	// �߾� ��ü ȸ�� ������Ʈ
	if (isRotatingY && centerRotationDirection != 0) {
		centralRotationAngle += glm::radians(rotationSpeed) * centerRotationDirection; // ���⿡ ���� ���� ����
		if (centralRotationAngle > glm::radians(360.0f)) {
			centralRotationAngle -= glm::radians(360.0f); // 360�� �ʰ� �� �ʱ�ȭ
		}
		if (centralRotationAngle < glm::radians(-360.0f)) {
			centralRotationAngle += glm::radians(360.0f); // -360�� �ʰ� �� �ʱ�ȭ
		}
		glutPostRedisplay();
	}

	// ���� ȸ�� ������Ʈ
	if (isRotatingGunBarrel && gunBarrelRotationDirection != 0) {
		gunBarrelRotationAngle1 += glm::radians(gunBarrelRotationSpeed) * gunBarrelRotationDirection; // ���� 1 ȸ��
		gunBarrelRotationAngle2 += glm::radians(gunBarrelRotationSpeed) * -gunBarrelRotationDirection; // ���� 2 ȸ��

		// ������ 360���� ����
		if (gunBarrelRotationAngle1 > glm::radians(360.0f)) {
			gunBarrelRotationAngle1 -= glm::radians(360.0f);
		}
		if (gunBarrelRotationAngle2 > glm::radians(360.0f)) {
			gunBarrelRotationAngle2 -= glm::radians(360.0f);
		}
		if (gunBarrelRotationAngle1 < glm::radians(-360.0f)) {
			gunBarrelRotationAngle1 += glm::radians(360.0f);
		}
		if (gunBarrelRotationAngle2 < glm::radians(-360.0f)) {
			gunBarrelRotationAngle2 += glm::radians(360.0f);
		}
		glutPostRedisplay();
	}


	// ���� �̵� �ִϸ��̼�
	if (isMerging) {
		gunBarrelMergeOffset += mergeSpeed; // �߾����� �̵�
		if (gunBarrelMergeOffset >= 0.15f) { // �߾� ����
			gunBarrelMergeOffset = 0.15f;   // �߾ӿ��� ����
			isMerging = false;
		}
		glutPostRedisplay();
	}

	if (isReturning) {
		gunBarrelMergeOffset -= mergeSpeed; // ���� �ڸ��� �̵�
		if (gunBarrelMergeOffset <= 0.0f) { // ���� �ڸ� ����
			gunBarrelMergeOffset = 0.0f;   // �̵� ����
			isReturning = false;
		}
		glutPostRedisplay();
	}

	// ũ���� �� ȸ�� ������Ʈ
	if (isCraneRotating && craneRotationDirection != 0) {
		craneArmAngle1 += craneRotationSpeed * craneRotationDirection;  // �� 1 ȸ��
		craneArmAngle2 -= craneRotationSpeed * craneRotationDirection;  // �� 2 �ݴ� ���� ȸ��

		// ������ -90 ~ 90�� ���̷� ����
		if (craneArmAngle1 > 90.0f) craneArmAngle1 = 90.0f;
		if (craneArmAngle1 < -90.0f) craneArmAngle1 = -90.0f;
		if (craneArmAngle2 > 90.0f) craneArmAngle2 = 90.0f;
		if (craneArmAngle2 < -90.0f) craneArmAngle2 = -90.0f;

		glutPostRedisplay();
	}



	glutTimerFunc(16, Update, 0); // 60FPS ����
}



