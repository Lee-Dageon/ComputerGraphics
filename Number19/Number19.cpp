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


glm::vec3 cameraPos = glm::vec3(0.5f, 1.0f, 3.0f); // 초기 카메라 위치
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 카메라가 바라보는 점
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // 카메라 상단 방향

glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, 0.0f); // 로봇의 중심점


bool animateRotation = false; // 애니메이션 상태
float cameraAngle = 0.0f;   // 카메라 자체 회전 각도

bool isMovingX = false; // 아래 몸체의 이동 상태 (true: 이동, false: 정지)
bool isMovingminusX = false;
float bodyXPosition = 0.0f; // 아래 몸체의 현재 X축 위치
float moveSpeed = 0.01f; // 이동 속도
int moveDirection = 0; // 1: 양 방향, -1: 음 방향, 0: 정지

bool isRotatingY = false; // 중앙 몸체의 회전 상태 (true: 회전 중, false: 정지)
float centralRotationAngle = 0.0f; // 중앙 몸체의 현재 회전 각도
float rotationSpeed = 5.0f; // 회전 속도 (1초당 5도)
int centerRotationDirection = 0; // 회전 방향 (1: 양, -1: 음, 0: 정지)

bool isRotatingGunBarrel = false; // 포신 회전 여부
float gunBarrelRotationAngle1 = 0.0f; // 포신 1의 회전 각도
float gunBarrelRotationAngle2 = 0.0f; // 포신 2의 회전 각도
float gunBarrelRotationSpeed = 5.0f;  // 포신 회전 속도
int gunBarrelRotationDirection = 0;

bool isMerging = false;     // 포신 중앙으로 이동 중 상태
bool isReturning = false;   // 포신 원래 자리로 이동 중 상태
float gunBarrelMergeOffset = 0.0f; // 포신 이동 거리
float mergeSpeed = 0.01f;   // 이동 속도

bool isCraneRotating = false;    // 크레인 팔 회전 상태
float craneArmAngle1 = 0.0f;     // 팔 1의 현재 각도
float craneArmAngle2 = 0.0f;     // 팔 2의 현재 각도
float craneRotationSpeed = 0.5f; // 팔 회전 속도
int craneRotationDirection = 0;  // 회전 방향 (1: 양, -1: 음, 0: 정지)

void Keyboard(unsigned char key, int x, int y);
void Update(int value);

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

Cube* cube;
Axis* axis;
Shader* shader;


void drawRobot(Shader* shader) { 
	glm::mat4 model = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");

	// 1. 아래 몸체 (기본 큐브 크기 그대로 렌더링)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X축 이동 반영
	model = glm::scale(model, glm::vec3(0.5f, 0.3f, 0.5f)); // 크기 조정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 2. 중앙 몸체 (몸통 위에 위치한 작은 큐브)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X축 이동 반영
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 회전 반영
	model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f)); // Y축으로 올림
	model = glm::scale(model, glm::vec3(0.3f, 0.1f, 0.3f)); // 크기 축소
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 3. 팔 1 (왼쪽)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X축 이동 반영
	model = glm::rotate(model, glm::radians(craneArmAngle1), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 회전 반영
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // 중앙 몸체와 동일한 회전
	model = glm::translate(model, glm::vec3(-0.1f, 0.3f, 0.0f)); // 왼쪽으로 약간 이동 후 위로 올림
	model = glm::scale(model, glm::vec3(0.05f, 0.2f, 0.05f)); // 더듬이 크기 축소
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 4. 팔 2 (오른쪽)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bodyXPosition, 0.0f, 0.0f)); // X축 이동 반영
	model = glm::rotate(model, glm::radians(craneArmAngle2), glm::vec3(0.0f, 0.0f, 1.0f)); // Z축 회전 반영
	model = glm::rotate(model, centralRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 회전 반영
	model = glm::translate(model, glm::vec3(0.1f, 0.3f, 0.0f)); // 오른쪽으로 약간 이동 후 위로 올림
	model = glm::scale(model, glm::vec3(0.05f, 0.2f, 0.05f)); // 더듬이 크기 축소
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 5. 포신 1 (왼쪽)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.15f + gunBarrelMergeOffset, -0.05f, 0.3f)); // 왼쪽 아래에서 앞쪽으로 뻗음
	model = glm::rotate(model, gunBarrelRotationAngle1, glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 자전 반영
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.2f)); // 더듬이 크기 조정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

	// 6. 포신 2 (오른쪽)
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.15f - gunBarrelMergeOffset, -0.05f, 0.3f)); // 오른쪽 아래에서 앞쪽으로 뻗음
	model = glm::rotate(model, gunBarrelRotationAngle2, glm::vec3(0.0f, 1.0f, 0.0f)); // Y축 반대 방향 자전 반영
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.2f)); // 더듬이 크기 조정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	cube->Render();

}



void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// 뷰 행렬: 카메라 위치와 방향 정의
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

	// 프로젝션 행렬: 원근 투영
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);


	glm::mat4 model = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(shader->programID, "trans");

	unsigned int viewLoc = glGetUniformLocation(shader->programID, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	unsigned int projLoc = glGetUniformLocation(shader->programID, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// 고정된 축 렌더링
	model = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// 로봇 렌더링 (몸통 + 머리)
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // 크기 조정
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
	// 사각뿔 객체 생성
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard); // 키보드 입력 등록
	glutTimerFunc(16, Update, 0); // 애니메이션 타이머 등록
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}

void Keyboard(unsigned char key, int x, int y) {
	const float moveSpeed = 0.1f;
	const float rotateSpeed = 1.0f;

	switch (key) {
	case 'z': // z축 양 방향 이동
		cameraPos.z -= moveSpeed;
		break;
	case 'Z': // z축 음 방향 이동
		cameraPos.z += moveSpeed;
		break;
	case 'x': // x축 양 방향 이동
		cameraPos.x += moveSpeed;
		break;
	case 'X': // x축 음 방향 이동
		cameraPos.x -= moveSpeed;
		break;

	case 'y': // 카메라가 자기 자신을 기준으로 시계 방향 회전

		cameraAngle += glm::radians(rotateSpeed); // 회전 각도를 증가
		cameraTarget = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)
		) + cameraPos; // 새로운 바라보는 지점 계산

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;


	case 'Y': // 카메라가 자기 자신을 기준으로 반시계 방향 회전
		cameraAngle -= glm::radians(rotateSpeed); // 회전 각도를 감소
		cameraTarget = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)
		) + cameraPos; // 새로운 바라보는 지점 계산

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'r': // y축 기준 공전 (화면 중심 기준 회전)
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 카메라가 바라보는 점
		cameraAngle += glm::radians(rotateSpeed);
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'R': // y축 기준 공전 반대 방향
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // 카메라가 바라보는 점
		cameraAngle -= glm::radians(rotateSpeed);
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), cameraAngle, cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;

		std::cout << "cameraPos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
		std::cout << "cameraTarget: (" << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << ")" << std::endl;
		break;

	case 'a': // 공전 애니메이션 시작/정지
		animateRotation = true;
		break;

	case 'A':
		animateRotation = false;
		break;

	case '+': // z축 양 방향 이동
		cameraPos.y += moveSpeed;
		break;
	case '-': // z축 음 방향 이동
		cameraPos.y -= moveSpeed;
		break;

	case 'b': // 이동 방향 양/음으로 토글
		if (moveDirection == 1) moveDirection = 0; // 멈춤
		else moveDirection = 1; // 양 방향 이동
		break;

	case 'B':
		if (moveDirection == -1) moveDirection = 0; // 멈춤
		else moveDirection = -1; // 음 방향 이동
		break;

	case 'm': // 중앙 몸체 양의 방향 회전 토글
		if (isRotatingY && centerRotationDirection == 1) {
			isRotatingY = false;  // 회전 멈춤
			centerRotationDirection = 0;
		}
		else {
			isRotatingY = true;   // 회전 시작
			centerRotationDirection = 1; // 양의 방향
		}
		break;

	case 'M': // 중앙 몸체 음의 방향 회전 토글
		if (isRotatingY && centerRotationDirection == -1) {
			isRotatingY = false;  // 회전 멈춤
			centerRotationDirection = 0;
		}
		else {
			isRotatingY = true;   // 회전 시작
			centerRotationDirection = -1; // 음의 방향
		}
		break;


	case 'f': // 포신 양의 방향 회전 토글
		if (isRotatingGunBarrel && gunBarrelRotationDirection == 1) {
			isRotatingGunBarrel = false;  // 정지
			gunBarrelRotationDirection = 0;
		}
		else {
			isRotatingGunBarrel = true;   // 시작
			gunBarrelRotationDirection = 1;
		}
		break;


	case 'F': // 포신 음의 방향 회전 토글
		if (isRotatingGunBarrel && gunBarrelRotationDirection == -1) {
			isRotatingGunBarrel = false;  // 정지
			gunBarrelRotationDirection = 0;
		}
		else {
			isRotatingGunBarrel = true;   // 시작
			gunBarrelRotationDirection = -1;
		}
		break;


	case 'e': // 포신 중앙으로 이동 시작
		if (!isMerging && !isReturning) {
			isMerging = true;  // 중앙으로 이동 시작
		}
		break;

	case 'E': // 포신 원래 자리로 이동 시작
		if (!isMerging && !isReturning) {
			isReturning = true; // 원래 자리로 이동 시작
		}
		break;

	case 't': // 양의 방향 회전 토글
		if (isCraneRotating && craneRotationDirection == 1) {
			isCraneRotating = false;  // 정지
			craneRotationDirection = 0;
		}
		else {
			isCraneRotating = true;   // 시작
			craneRotationDirection = 1; // 양의 방향
		}
		break;

	case 'T': // 음의 방향 회전 토글
		if (isCraneRotating && craneRotationDirection == -1) {
			isCraneRotating = false;  // 정지
			craneRotationDirection = 0;
		}
		else {
			isCraneRotating = true;   // 시작
			craneRotationDirection = -1; // 음의 방향
		}
		break;

	case 's': // 모든 움직임 멈추기
	case 'S':
		animateRotation = false; // 공전 애니메이션 정지
		isMovingX = false;       // X축 이동 정지
		isRotatingY = false;     // 중앙 몸체 회전 정지
		isRotatingGunBarrel = false; // 포신 회전 정지
		isMerging = false;       // 포신 병합 정지
		isReturning = false;     // 포신 복귀 정지
		isCraneRotating = false; // 크레인 팔 회전 정지
		moveDirection = 0;       // 이동 방향 초기화
		centerRotationDirection = 0; // 중앙 회전 방향 초기화
		gunBarrelRotationDirection = 0; // 포신 회전 방향 초기화
		craneRotationDirection = 0; // 크레인 회전 방향 초기화
		break;

	case 'c': // 모든 움직임 초기화
	case 'C':
		animateRotation = false;
		isMovingX = false;
		isRotatingY = false;
		isRotatingGunBarrel = false;
		isMerging = false;
		isReturning = false;
		isCraneRotating = false;

		// 상태 초기화
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

	case 'q': // 프로그램 종료
	case 'Q':
		exit(0); // 프로그램 종료
		break;



	default:
		break;
	}

	glutPostRedisplay(); // 화면 갱신 요청
}


void Update(int value) {

	if (animateRotation) {
		cameraAngle += glm::radians(1.0f); // 애니메이션 속도
		cameraPos = glm::vec3(
			glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), cameraUp) * glm::vec4(cameraPos - cameraTarget, 1.0f)
		) + cameraTarget;
		glutPostRedisplay();
	}

	// 이동 방향에 따라 아래 몸체 위치 업데이트
	if (moveDirection != 0) {
		bodyXPosition += moveDirection * moveSpeed; // 이동 방향(1 또는 -1) 곱하기 속도
		glutPostRedisplay();
	}

	// 중앙 몸체 회전 업데이트
	if (isRotatingY && centerRotationDirection != 0) {
		centralRotationAngle += glm::radians(rotationSpeed) * centerRotationDirection; // 방향에 따라 각도 변경
		if (centralRotationAngle > glm::radians(360.0f)) {
			centralRotationAngle -= glm::radians(360.0f); // 360도 초과 시 초기화
		}
		if (centralRotationAngle < glm::radians(-360.0f)) {
			centralRotationAngle += glm::radians(360.0f); // -360도 초과 시 초기화
		}
		glutPostRedisplay();
	}

	// 포신 회전 업데이트
	if (isRotatingGunBarrel && gunBarrelRotationDirection != 0) {
		gunBarrelRotationAngle1 += glm::radians(gunBarrelRotationSpeed) * gunBarrelRotationDirection; // 포신 1 회전
		gunBarrelRotationAngle2 += glm::radians(gunBarrelRotationSpeed) * -gunBarrelRotationDirection; // 포신 2 회전

		// 각도를 360도로 제한
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


	// 포신 이동 애니메이션
	if (isMerging) {
		gunBarrelMergeOffset += mergeSpeed; // 중앙으로 이동
		if (gunBarrelMergeOffset >= 0.15f) { // 중앙 도달
			gunBarrelMergeOffset = 0.15f;   // 중앙에서 멈춤
			isMerging = false;
		}
		glutPostRedisplay();
	}

	if (isReturning) {
		gunBarrelMergeOffset -= mergeSpeed; // 원래 자리로 이동
		if (gunBarrelMergeOffset <= 0.0f) { // 원래 자리 도달
			gunBarrelMergeOffset = 0.0f;   // 이동 종료
			isReturning = false;
		}
		glutPostRedisplay();
	}

	// 크레인 팔 회전 업데이트
	if (isCraneRotating && craneRotationDirection != 0) {
		craneArmAngle1 += craneRotationSpeed * craneRotationDirection;  // 팔 1 회전
		craneArmAngle2 -= craneRotationSpeed * craneRotationDirection;  // 팔 2 반대 방향 회전

		// 각도를 -90 ~ 90도 사이로 제한
		if (craneArmAngle1 > 90.0f) craneArmAngle1 = 90.0f;
		if (craneArmAngle1 < -90.0f) craneArmAngle1 = -90.0f;
		if (craneArmAngle2 > 90.0f) craneArmAngle2 = 90.0f;
		if (craneArmAngle2 < -90.0f) craneArmAngle2 = -90.0f;

		glutPostRedisplay();
	}



	glutTimerFunc(16, Update, 0); // 60FPS 기준
}



