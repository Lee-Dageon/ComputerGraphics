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



Cube* cube;
Axis* axis;
Shader* shader;


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
		glm::vec3(0.5f, 0.5f, 0.5f)  // 6번 면: 회색
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
	cube->RenderFace(0);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-movementOffset, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(faceColorLocation, 1, glm::value_ptr(faceColors[1])); // 1번 면 색상
	cube->RenderFace(1);

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

	glutSwapBuffers();
}



// 키 입력 처리 함수
void Keyboard(unsigned char key, int x, int y) {
	if (key == 'o' || key == 'O') {
		opening = !opening; // 무대 열기/닫기 토글
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

	glutDisplayFunc(Render);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Update, 0); // 60fps로 업데이트 함수 호출
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}

