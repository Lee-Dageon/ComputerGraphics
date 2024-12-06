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

class Axis {
public:
	GLuint VAO, VBO;

	Axis() {
		InitBuffer();
	}

	void InitBuffer() {
		GLfloat axisVertices[] = {
			// 위치             // 색상 (하얀색)
			-1.0f,  0.0f,  0.0f,   1.0f, 1.0f, 1.0f, // X축 음의 방향
			 1.0f,  0.0f,  0.0f,   1.0f, 1.0f, 1.0f, // X축 양의 방향

			 0.0f, -1.0f,  0.0f,   1.0f, 1.0f, 1.0f, // Y축 음의 방향
			 0.0f,  1.0f,  0.0f,   1.0f, 1.0f, 1.0f, // Y축 양의 방향

			 0.0f,  0.0f, -1.0f,   1.0f, 1.0f, 1.0f, // Z축 음의 방향
			 0.0f,  0.0f,  1.0f,   1.0f, 1.0f, 1.0f  // Z축 양의 방향
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

		// 위치 데이터
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);

		// 색상 데이터
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void Render() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 6); // 6개의 점(X, Y, Z 축)
		glBindVertexArray(0);
	}

	~Axis() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
};

class LightCube {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO, VBO, EBO;

	LightCube() {
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

	~LightCube() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO);
		if (EBO) glDeleteBuffers(1, &EBO);
	}
};

Cube* cube;
Axis* axis;
Horn* horn;
Shader* shader;
LightCube* lightCube;

GLfloat rotationX = 0.0f;
GLfloat rotationY = 0.0f;
// 이동 변수 초기화
float translationX = 0.0f;
float translationY = 0.0f;
float translationZ = 0.0f;
float moveSpeed = 0.1f; // 이동 속도

bool drawCube = false; // 큐브와 사각뿔을 번갈아 그릴 상태 변수
bool drawHorn = false; // 큐브와 사각뿔을 번갈아 그릴 상태 변수
bool cullEnabled = false;     // 은면 제거 상태 변수
bool rotateXEnabled = false;     // 은면 제거 상태 변수
bool rotateYEnabled = false;     // 은면 제거 상태 변수
bool lightEnabled = true; // true: 조명 켜짐, false: 조명 꺼짐
void Keyboard(unsigned char key, int x, int y);
void SpecialKeys(int key, int x, int y);


// 타이머 콜백 함수

void Timer(int value) {
	if (rotateXEnabled) {
		rotationX += 0.8f; // rotationX를 일정하게 증가
		if (rotationX >= 360.0f) {
			rotationX -= 360.0f; // 값이 360도를 넘으면 초기화
		}
		glutPostRedisplay();     // 화면을 다시 그리기 요청
		glutTimerFunc(16, Timer, 0); // 16ms 후에 타이머 함수 재호출 (약 60 FPS)
	}
	
	if (rotateYEnabled) {
		rotationY += 0.8f; // rotationX를 일정하게 증가
		if (rotationY >= 360.0f) {
			rotationY -= 360.0f; // 값이 360도를 넘으면 초기화
		}
		glutPostRedisplay();     // 화면을 다시 그리기 요청
		glutTimerFunc(16, Timer, 0); // 16ms 후에 타이머 함수 재호출 (약 60 FPS)
	}
}


glm::vec3 cameraPos(-1.3, 0.9, 3.7); // 초기 카메라 위치
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f); // 카메라가 바라보는 지점
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f); // 월드 공간의 "위쪽 방향"


void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Use();

    // 1. 모델, 뷰, 투영 행렬 설정
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(translationX, translationY, translationZ));
    model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
    model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

	// 뷰 행렬 갱신
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader->programID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader->programID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->programID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// 조명 Uniform 변수 설정
	glUniform1i(glGetUniformLocation(shader->programID, "lightEnabled"), lightEnabled ? 1 : 0);

	// 2. 스포트라이트 설정
	glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 1.5f); // 빛의 위치
	glm::vec3 lightDirection = glm::vec3(0.0f, 0.0f, -1.0f); // 빛의 방향
	float cutoff = glm::cos(glm::radians(5.0f)); // 스포트라이트 각도 (12.5도)
	float outerCutoff = glm::cos(glm::radians(20.0f)); // 외부 컷오프 (17.5도)

	glUniform1f(glGetUniformLocation(shader->programID, "outerCutoff"), outerCutoff);

	glUniform3fv(glGetUniformLocation(shader->programID, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shader->programID, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform1f(glGetUniformLocation(shader->programID, "cutoff"), cutoff);

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // 흰색 광원
	glm::vec3 objectColor(0.0f, 1.0f, 0.0f); // 주황색 물체
	glm::vec3 viewPos = cameraPos;

	glUniform3fv(glGetUniformLocation(shader->programID, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(shader->programID, "objectColor"), 1, glm::value_ptr(objectColor));
	glUniform3fv(glGetUniformLocation(shader->programID, "viewPos"), 1, glm::value_ptr(viewPos));

    // 3. 객체 렌더링
    if (drawCube) cube->Render();
    if (drawHorn) horn->Render();
	
	// 광원 위치에 큐브 렌더링
	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // 작은 크기의 큐브
	glUniformMatrix4fv(glGetUniformLocation(shader->programID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lightCube->Render();

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
	lightCube = new LightCube();
	cube = new Cube();
	axis = new Axis();
	// 사각뿔 객체 생성
	horn = new Horn();
	shader = new Shader("vertex.glsl", "fragment.glsl");

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys); // 방향키 입력 함수 등록
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}


void Keyboard(unsigned char key, int x, int y) {

	if (key == 'c') {
		drawCube = !drawCube; // 'c' 키가 눌리면 상태를 토글
		glutPostRedisplay(); // 화면을 다시 그리기 요청
	}

	else if (key == 'p') {
		drawHorn = !drawHorn; // 'c' 키가 눌리면 상태를 토글
		glutPostRedisplay(); // 화면을 다시 그리기 요청
	}

	else if (key == 'h') {
		cullEnabled = !cullEnabled; // 'h' 키가 눌리면 은면 제거 토글
		glutPostRedisplay();        // 화면을 다시 그리기 요청
	}

	else if (key == 'x') {
		rotateXEnabled = !rotateXEnabled; // 'x' 키가 눌리면 rotationX 타이머 토글
		if (rotateXEnabled) {
			glutTimerFunc(0, Timer, 0);   // 타이머 시작
		}
	}

	else if (key == 'y') {
		rotateYEnabled = !rotateYEnabled; // 'x' 키가 눌리면 rotationX 타이머 토글
		if (rotateYEnabled) {
			glutTimerFunc(0, Timer, 0);   // 타이머 시작
		}
	}

	else if (key == 'z') {// 와이어프레임 모드
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
	}

	else if (key == 'Z') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glutPostRedisplay();
	}

	else if (key == 'a') { // x축 양의 방향으로 이동
		cameraPos.x += moveSpeed;
		glutPostRedisplay();
	}
	else if (key == 'A') { // x축 음의 방향으로 이동
		cameraPos.x -= moveSpeed;
		glutPostRedisplay();
	}
	else if (key == 'w') { // y축 양의 방향으로 이동
		cameraPos.y += moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'W') { // y축 음의 방향으로 이동
		cameraPos.y -= moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'd') { // z축 양의 방향으로 이동
		cameraPos.z += moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'D') { // z축 음의 방향으로 이동
		cameraPos.z -= moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'i') { // y축 양의 방향으로 이동
		translationZ += moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'k') { // y축 음의 방향으로 이동
		translationZ -= moveSpeed;
		glutPostRedisplay();
	}

	else if (key == 'm' || key == 'M') { // 'm' 키로 조명 켜기/끄기
		lightEnabled = !lightEnabled;
		glutPostRedisplay(); // 화면 다시 그리기 요청
	}

	// 카메라 위치 출력 (디버깅용)
	std::cout << "Camera Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
	std::cout << rotationX << ", " << rotationY << std::endl;

}

void SpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		translationY += moveSpeed;  // 위쪽 이동
		break;
	case GLUT_KEY_DOWN:
		translationY -= moveSpeed;  // 아래쪽 이동
		break;
	case GLUT_KEY_LEFT:
		translationX -= moveSpeed;  // 왼쪽 이동
		break;
	case GLUT_KEY_RIGHT:
		translationX += moveSpeed;  // 오른쪽 이동
		break;
	}
	glutPostRedisplay(); // 화면을 다시 그리기 요청
}



