#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <ctime>

#define _CRT_SECURE_NO_WARNINGS

#define WIDTH 600
#define HEIGHT 600

using namespace std;
GLfloat rotationX = 0.0f;
GLfloat rotationY = 0.0f;
GLfloat rotationSpeed = 5.0f;  // 회전 속도
int a1 = 0, a2 = 0;
int b1 = 0, b2 = 0;
// 콜백 함수
GLvoid Render(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
bool two_cube = false;
bool two_tetra = false;
// VAO, VBO
GLuint VAO, VBO[2], EBO;
int currentFace = -1;  // 현재 선택된 면 (-1은 모든 면을 그리는 기본 상태)
int currentFace2 = -1;

void DrawCube();
void Drawtetramesh();
GLchar* vertexSource, * fragmentSource; // 소스코드 저장 변수
GLuint vertexShader, fragmentShader;    // 셰이더 객체
GLuint shaderProgramID;                 // 셰이더 프로그램
GLfloat dx_th;                          // 각도 변화량 x
GLfloat dy_th;                          // 각도 변화량 y
GLint CreateShader(const char* file, int type);
GLvoid CreateShaderProgram();
GLvoid InitBuffer();

GLvoid SpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        rotationX -= rotationSpeed;  // 위쪽 화살표로 X축 회전
        break;
    case GLUT_KEY_DOWN:
        rotationX += rotationSpeed;  // 아래쪽 화살표로 X축 회전
        break;
    case GLUT_KEY_LEFT:
        rotationY -= rotationSpeed;  // 왼쪽 화살표로 Y축 회전
        break;
    case GLUT_KEY_RIGHT:
        rotationY += rotationSpeed;  // 오른쪽 화살표로 Y축 회전
        break;
    }
    glutPostRedisplay();  // 다시 그리기
}

void SelectRandomFaces() {
    int firstFace = rand() % 6; // 0부터 5 사이의 랜덤한 면 선택
    int secondFace;
    do {
        secondFace = rand() % 6; // 또 다른 랜덤한 면 선택
    } while (secondFace == firstFace); // 첫 번째 면과 다르도록 보장

    currentFace = -1; // 모든 면을 그리기 위해 리셋

    // 선택된 두 면을 그리기
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(firstFace * 6 * sizeof(GLuint)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(secondFace * 6 * sizeof(GLuint)));
}
void main(int argc, char** argv)
{
    // 윈도우 생성
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("실습13");

    // GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    InitBuffer();
    CreateShaderProgram();

    glutDisplayFunc(Render);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);  // 키보드 함수 등록
    glutSpecialFunc(SpecialKeys);  // 화살표 키 입력 처리 함수 등록

    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
}

GLvoid Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    // 회전 행렬 업데이트
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

    model = model * scale * rotateX * rotateY;

    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    DrawCube();
    Drawtetramesh();
    glutSwapBuffers();
}


GLvoid Reshape(int w, int h)
{
    // 뷰포트 기본 WIDTH HEIGHT로 설정
    glViewport(0, 0, w, h);
}

GLvoid InitBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);
}

char* GetBuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;

    fopen_s(&fptr, file, "rb");

    if (!fptr) return NULL;

    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);

    buf[length] = 0;
    return buf;
}

GLint CreateShader(const char* file, int type)
{
    GLchar* source = GetBuf(file);

    GLint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, 0);
    glCompileShader(shader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (!result)
    {
        glGetShaderInfoLog(shader, 512, NULL, errorLog);
        std::cerr << "ERROR: 컴파일 실패\n" << errorLog << std::endl;
        return 0;
    }

    return shader;
}

void CreateShaderProgram()
{
    vertexShader = CreateShader("vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = CreateShader("fragment.glsl", GL_FRAGMENT_SHADER);

    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgramID);
}

void DrawCube()
{
    GLfloat CubeVertexs[6][4][3] = {
        // front
        {{-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f}},

        // top
        {{-0.5f, 0.5f, -0.5f},
       {0.5f, 0.5f, -0.5f},
       {0.5f, 0.5f, 0.5f},
       {-0.5f, 0.5f, 0.5f}},

       // left
       {{-0.5f, -0.5f, -0.5f},
      {-0.5f, 0.5f, -0.5f},
      {-0.5f, 0.5f, 0.5f},
      {-0.5f, -0.5f, 0.5f}},

      // right
      {{0.5f, -0.5f, -0.5f},
     {0.5f, 0.5f, -0.5f},
     {0.5f, 0.5f, 0.5f},
     {0.5f, -0.5f, 0.5f}},

     // back
     {{-0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f}},

    // bottom
    {{-0.5f, -0.5f, 0.5f},
   {0.5f, -0.5f, 0.5f},
   {0.5f, -0.5f, -0.5f},
   {-0.5f, -0.5f, -0.5f}}
    };

    GLfloat CubeColors[6][12] = {
        1.0f, 0.0f, 0.0f,  // 윗면 빨간색
        1.0f, 0.0f, 0.0f,  // 윗면 빨간색
        1.0f, 0.0f, 0.0f,  // 윗면 빨간색
        1.0f, 0.0f, 0.0f,  // 윗면 빨간색

        0.0f, 0.0f, 1.0f,  // 윗면 빨간색
        0.0f, 0.0f, 1.0f,  // 윗면 빨간색
        0.0f, 0.0f, 1.0f,  // 윗면 빨간색
        0.0f, 0.0f, 1.0f,   // 윗면 빨간색

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f
    };

    GLint CubeIndexs[6][6] = {
        {0, 1, 2, 0, 2, 3},  // 윗면
        {4,5,6,6,7,4},  // 앞면
        {8,9,10,10,11,8},  // 우측면
        {14,13,12,12,15,14},  // 좌측면
        {18,17,16,16,19,18},  // 밑면
        {20,21,22,22,23,20}   // 뒷면
    };

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertexs), CubeVertexs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndexs), CubeIndexs, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeColors), CubeColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 move = glm::mat4(1.0f);

    scale = glm::scale(scale, glm::vec3(0.5f, 0.5f, 0.5f));
    move = glm::translate(move, glm::vec3(0.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    model = model * scale * move * rot;

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    // 선택된 면만 그리기
    if (currentFace >= 0 && currentFace < 6) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(currentFace * 6 * sizeof(GLuint)));
    }
    else if (two_cube)
    {
        int firstFace = rand() % 6; // 0부터 5 사이의 랜덤한 면 선택
        int secondFace;
        do {
            secondFace = rand() % 6; // 또 다른 랜덤한 면 선택
        } while (secondFace == firstFace); // 첫 번째 면과 다르도록 보장

        currentFace = -1; // 모든 면을 그리기 위해 리셋
        currentFace2 = -1; // 모든 면을 그리기 위해 리셋
        // 선택된 두 면을 그리기
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(firstFace * 6 * sizeof(GLuint)));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(secondFace * 6 * sizeof(GLuint)));
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
void Drawtetramesh()
{
    GLfloat tetraVertexs[4][3][3] = {
        {{0.5f, 0.5f, -0.5f},{0.5f, -0.5f, -0.5f},{-0.5f, -0.5f, -0.5f}},//1면
        {{ 0.5f, 0.5f, -0.5f },{0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}},//2면
        {{0.5f, 0.5f, -0.5f},{-0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}},//3면
        {{0.5f, -0.5f, -0.5f},{-0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}}//4면

    };

    GLfloat tetraColors[12][3] = {
        {1.0f, 0.0f, 0.0f},  // 빨간색
        {1.0f, 0.0f, 0.0f},  // 빨간색
        {1.0f, 0.0f, 0.0f},  // 빨간색

        {0.0f, 0.0f, 1.0f},  // 파란색
        {0.0f, 0.0f, 1.0f},  // 파란색
        {0.0f, 0.0f, 1.0f},  // 파란색

        {0.0f, 1.0f, 0.0f},  // 초록색
        {0.0f, 1.0f, 0.0f},  // 초록색
        {0.0f, 1.0f, 0.0f},  // 초록색

        {1.0f, 1.0f, 0.0f} ,  // 노란색
        {1.0f, 1.0f, 0.0f} ,  // 노란색
        {1.0f, 1.0f, 0.0f} ,  // 노란색

    };

    GLint tetraIndexs[4][3] = {
        {0, 1, 2},   // 첫 번째 삼각형
        {3, 4, 5},   // 두 번째 삼각형
        {6, 7, 8},   // 세 번째 삼각형
        {9, 10, 11}    // 네 번째 삼각형
    };

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetraVertexs), tetraVertexs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetraIndexs), tetraIndexs, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tetraColors), tetraColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "trans");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 move = glm::mat4(1.0f);

    scale = glm::scale(scale, glm::vec3(0.5f, 0.5f, 0.5f));
    move = glm::translate(move, glm::vec3(0.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(10.0f + rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    rot = glm::rotate(rot, glm::radians(10.0f + rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

    model = model * scale * move * rot;

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    if (currentFace2 >= 0 && currentFace2 < 4) {
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(currentFace2 * 3 * sizeof(GLuint)));
    }
    else if (two_tetra)
    {
        int firstFace = rand() % 4; // 0부터 5 사이의 랜덤한 면 선택
        int secondFace;
        do {
            secondFace = rand() % 4; // 또 다른 랜덤한 면 선택
        } while (secondFace == firstFace); // 첫 번째 면과 다르도록 보장

        currentFace = -1; // 모든 면을 그리기 위해 리셋
        currentFace2 = -1; // 모든 면을 그리기 위해 리셋
        // 선택된 두 면을 그리기
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(firstFace * 3 * sizeof(GLuint)));
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(secondFace * 3 * sizeof(GLuint)));
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':
        currentFace = 0;  // 윗면
        currentFace2 = -1;  // 뒷면
        //two_cube = false;
        //two_tetra = false;
        break;
    case '2':
        currentFace = 1;  // 앞면
        currentFace2 = -1;
        break;
    case '3':
        currentFace = 2;  // 우측면
        currentFace2 = -1;
        break;
    case '4':
        currentFace = 3;  // 좌측면
        currentFace2 = -1;
        break;
    case '5':
        currentFace = 4;  // 밑면
        currentFace2 = -1;
        break;
    case '6':
        currentFace = 5;  // 뒷면
        currentFace2 = -1;
        break;

    case '7':
        currentFace = -1;
        currentFace2 = 0;  // 뒷면
        break;
    case '8':
        currentFace = -1;
        currentFace2 = 1;  // 뒷면
        break;
    case '9':
        currentFace = -1;
        currentFace2 = 2;  // 뒷면
        break;
    case '0':
        currentFace = -1;
        currentFace2 = 3;  // 모든 면
        break;
    case 'c':
        two_cube = true;
        two_tetra = false;
        break;

    case 't':
        two_tetra = true;
        two_cube = false;
        break;
        printf("%d  %d\n", b1, b2);
    }

    glutPostRedisplay();  // 다시 그리기
}
