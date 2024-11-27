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
bool frontFaceRotationEnabled = false; // 앞면 회전 활성화 여부
// 자전 속도
GLfloat rotationSpeed = 1.0f;
GLfloat frontFaceAngle = -30.0f; // 앞면 회전 각도
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);

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
	GLuint shaderProgramID;  // 셰이더 프로그램 ID를 저장할 변수 추가

	Cube(GLuint shaderProgram) {
		shaderProgramID = shaderProgram;  // 셰이더 프로그램 ID 저장
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
			1.0f, 0.0f, 0.0f,  // 앞면 빨강
			0.0f, 1.0f, 0.0f,  // 뒷면 초록
			0.0f, 0.0f, 1.0f,  // 윗면 파랑
			1.0f, 1.0f, 0.0f,  // 아랫면 노랑
			1.0f, 0.0f, 1.0f,  // 왼쪽 면 자홍색
			0.0f, 1.0f, 1.0f   // 오른쪽 면 청록색
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
		glUseProgram(shaderProgramID);  // 셰이더 프로그램 사용

		glBindVertexArray(VAO);

		// 앞면만 회전시키는 로직
		if (frontFaceRotationEnabled) {
			// 앞면의 정점들의 중심을 계산
			glm::vec3 center(0.0f);
			for (size_t i = 0; i < indices.size(); i += 3) {
				if (IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					center += glm::vec3(vertices[indices[i]].x, vertices[indices[i]].y, vertices[indices[i]].z);
					center += glm::vec3(vertices[indices[i + 1]].x, vertices[indices[i + 1]].y, vertices[indices[i + 1]].z);
					center += glm::vec3(vertices[indices[i + 2]].x, vertices[indices[i + 2]].y, vertices[indices[i + 2]].z);
					center /= 3.0f;  // 앞면의 세 정점의 평균을 중심으로 설정
					break;
				}
			}

			// 앞면만 회전하는 변환 행렬 설정 (기존 모델 회전 추가)
			glm::mat4 frontFaceTransform = glm::mat4(1.0f);

			// 나머지 큐브와 동일한 회전 적용 (전체 회전)
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

			// 앞면 중심 기준 회전 추가 (x축 기준으로 회전 - 뚜껑처럼 회전)
			frontFaceTransform = glm::translate(frontFaceTransform, center); // 중심으로 이동
			frontFaceTransform = glm::rotate(frontFaceTransform, glm::radians(frontFaceAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // 중심 기준 회전 (x축 기준으로 뚜껑처럼 회전)
			frontFaceTransform = glm::translate(frontFaceTransform, -center); // 원래 위치로 이동

			unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(frontFaceTransform));

			// 앞면만 그리기
			for (size_t i = 0; i < indices.size(); i += 3) {
				if (IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
					// 앞면 정점만 회전 적용 후 렌더링
					glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
				}
			}
		}

		// 나머지 큐브 부분은 기본 회전 적용하여 렌더링
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

		unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		for (size_t i = 0; i < indices.size(); i += 3) {
			if (!IsFrontFace(indices[i], indices[i + 1], indices[i + 2])) {
				// 앞면이 아닌 나머지 면 렌더링
				glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)), 0);
			}
		}

		glBindVertexArray(0);
	}



	bool IsFrontFace(GLuint idx1, GLuint idx2, GLuint idx3) {
		// 앞면은 정점 1, 2, 3과 1, 3, 4로 구성됨 (예시)
		return (idx1 == 0 && idx2 == 1 && idx3 == 2) ||
			(idx1 == 0 && idx2 == 2 && idx3 == 3);
	}
};  // 큐브 불러오는 클래스 끝


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
Horn* horn;
Shader* shader;





void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->Use();

	// 은면 제거 활성화 여부에 따른 설정
	if (cullFaceEnabled) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	// y축 자전이 활성화된 경우 회전 각도를 업데이트
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

	// 고정된 축 렌더링 (원래 크기와 위치로 렌더링)
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	axis->Render();

	// 사각뿔(Horn) 렌더링 (크기 0.5배 축소, X/Y축 회전 적용)
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
	// Cube 객체 생성 시 셰이더 프로그램 ID를 전달
	cube = new Cube(shader->programID);
	axis = new Axis();
	// 사각뿔 객체 생성
	horn = new Horn();


	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);  // 키보드 콜백 함수 등록
	glEnable(GL_DEPTH_TEST);
	glutTimerFunc(0, Timer, 0);  // 타이머 함수 등록
	glutMainLoop();
	return 0;
}

// 타이머 함수
void Timer(int value) {

	if (frontFaceRotationEnabled) {
		frontFaceAngle += rotationSpeed;
		if (frontFaceAngle > 360.0f) {
			frontFaceAngle -= 360.0f;
		}
	}

	glutPostRedisplay();  // 장면을 갱신하여 자전이 보이도록 함
	glutTimerFunc(16, Timer, 0);  // 약 60 FPS로 타이머 호출 (16ms마다)
}


// 키보드 입력 처리 함수
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

