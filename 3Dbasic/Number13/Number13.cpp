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
GLfloat rotationSpeed = 5.0f;  // ȸ�� �ӵ�
int a1 = 0, a2 = 0;
int b1 = 0, b2 = 0;
// �ݹ� �Լ�
GLvoid Render(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
bool two_cube = false;
bool two_tetra = false;
// VAO, VBO
GLuint VAO, VBO[2], EBO;
int currentFace = -1;  // ���� ���õ� �� (-1�� ��� ���� �׸��� �⺻ ����)
int currentFace2 = -1;

void DrawCube();
void Drawtetramesh();
GLchar* vertexSource, * fragmentSource; // �ҽ��ڵ� ���� ����
GLuint vertexShader, fragmentShader;    // ���̴� ��ü
GLuint shaderProgramID;                 // ���̴� ���α׷�
GLfloat dx_th;                          // ���� ��ȭ�� x
GLfloat dy_th;                          // ���� ��ȭ�� y
GLint CreateShader(const char* file, int type);
GLvoid CreateShaderProgram();
GLvoid InitBuffer();

GLvoid SpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        rotationX -= rotationSpeed;  // ���� ȭ��ǥ�� X�� ȸ��
        break;
    case GLUT_KEY_DOWN:
        rotationX += rotationSpeed;  // �Ʒ��� ȭ��ǥ�� X�� ȸ��
        break;
    case GLUT_KEY_LEFT:
        rotationY -= rotationSpeed;  // ���� ȭ��ǥ�� Y�� ȸ��
        break;
    case GLUT_KEY_RIGHT:
        rotationY += rotationSpeed;  // ������ ȭ��ǥ�� Y�� ȸ��
        break;
    }
    glutPostRedisplay();  // �ٽ� �׸���
}

void SelectRandomFaces() {
    int firstFace = rand() % 6; // 0���� 5 ������ ������ �� ����
    int secondFace;
    do {
        secondFace = rand() % 6; // �� �ٸ� ������ �� ����
    } while (secondFace == firstFace); // ù ��° ��� �ٸ����� ����

    currentFace = -1; // ��� ���� �׸��� ���� ����

    // ���õ� �� ���� �׸���
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(firstFace * 6 * sizeof(GLuint)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(secondFace * 6 * sizeof(GLuint)));
}
void main(int argc, char** argv)
{
    // ������ ����
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("�ǽ�13");

    // GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    glewInit();

    InitBuffer();
    CreateShaderProgram();

    glutDisplayFunc(Render);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);  // Ű���� �Լ� ���
    glutSpecialFunc(SpecialKeys);  // ȭ��ǥ Ű �Է� ó�� �Լ� ���

    glEnable(GL_DEPTH_TEST);
    glutMainLoop();
}

GLvoid Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    // ȸ�� ��� ������Ʈ
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
    // ����Ʈ �⺻ WIDTH HEIGHT�� ����
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
        std::cerr << "ERROR: ������ ����\n" << errorLog << std::endl;
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
        1.0f, 0.0f, 0.0f,  // ���� ������
        1.0f, 0.0f, 0.0f,  // ���� ������
        1.0f, 0.0f, 0.0f,  // ���� ������
        1.0f, 0.0f, 0.0f,  // ���� ������

        0.0f, 0.0f, 1.0f,  // ���� ������
        0.0f, 0.0f, 1.0f,  // ���� ������
        0.0f, 0.0f, 1.0f,  // ���� ������
        0.0f, 0.0f, 1.0f,   // ���� ������

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
        {0, 1, 2, 0, 2, 3},  // ����
        {4,5,6,6,7,4},  // �ո�
        {8,9,10,10,11,8},  // ������
        {14,13,12,12,15,14},  // ������
        {18,17,16,16,19,18},  // �ظ�
        {20,21,22,22,23,20}   // �޸�
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

    // ���õ� �鸸 �׸���
    if (currentFace >= 0 && currentFace < 6) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(currentFace * 6 * sizeof(GLuint)));
    }
    else if (two_cube)
    {
        int firstFace = rand() % 6; // 0���� 5 ������ ������ �� ����
        int secondFace;
        do {
            secondFace = rand() % 6; // �� �ٸ� ������ �� ����
        } while (secondFace == firstFace); // ù ��° ��� �ٸ����� ����

        currentFace = -1; // ��� ���� �׸��� ���� ����
        currentFace2 = -1; // ��� ���� �׸��� ���� ����
        // ���õ� �� ���� �׸���
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(firstFace * 6 * sizeof(GLuint)));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(secondFace * 6 * sizeof(GLuint)));
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
void Drawtetramesh()
{
    GLfloat tetraVertexs[4][3][3] = {
        {{0.5f, 0.5f, -0.5f},{0.5f, -0.5f, -0.5f},{-0.5f, -0.5f, -0.5f}},//1��
        {{ 0.5f, 0.5f, -0.5f },{0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}},//2��
        {{0.5f, 0.5f, -0.5f},{-0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}},//3��
        {{0.5f, -0.5f, -0.5f},{-0.5f, -0.5f, -0.5f},{0.5f, -0.5f, 0.5f}}//4��

    };

    GLfloat tetraColors[12][3] = {
        {1.0f, 0.0f, 0.0f},  // ������
        {1.0f, 0.0f, 0.0f},  // ������
        {1.0f, 0.0f, 0.0f},  // ������

        {0.0f, 0.0f, 1.0f},  // �Ķ���
        {0.0f, 0.0f, 1.0f},  // �Ķ���
        {0.0f, 0.0f, 1.0f},  // �Ķ���

        {0.0f, 1.0f, 0.0f},  // �ʷϻ�
        {0.0f, 1.0f, 0.0f},  // �ʷϻ�
        {0.0f, 1.0f, 0.0f},  // �ʷϻ�

        {1.0f, 1.0f, 0.0f} ,  // �����
        {1.0f, 1.0f, 0.0f} ,  // �����
        {1.0f, 1.0f, 0.0f} ,  // �����

    };

    GLint tetraIndexs[4][3] = {
        {0, 1, 2},   // ù ��° �ﰢ��
        {3, 4, 5},   // �� ��° �ﰢ��
        {6, 7, 8},   // �� ��° �ﰢ��
        {9, 10, 11}    // �� ��° �ﰢ��
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
        int firstFace = rand() % 4; // 0���� 5 ������ ������ �� ����
        int secondFace;
        do {
            secondFace = rand() % 4; // �� �ٸ� ������ �� ����
        } while (secondFace == firstFace); // ù ��° ��� �ٸ����� ����

        currentFace = -1; // ��� ���� �׸��� ���� ����
        currentFace2 = -1; // ��� ���� �׸��� ���� ����
        // ���õ� �� ���� �׸���
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
        currentFace = 0;  // ����
        currentFace2 = -1;  // �޸�
        //two_cube = false;
        //two_tetra = false;
        break;
    case '2':
        currentFace = 1;  // �ո�
        currentFace2 = -1;
        break;
    case '3':
        currentFace = 2;  // ������
        currentFace2 = -1;
        break;
    case '4':
        currentFace = 3;  // ������
        currentFace2 = -1;
        break;
    case '5':
        currentFace = 4;  // �ظ�
        currentFace2 = -1;
        break;
    case '6':
        currentFace = 5;  // �޸�
        currentFace2 = -1;
        break;

    case '7':
        currentFace = -1;
        currentFace2 = 0;  // �޸�
        break;
    case '8':
        currentFace = -1;
        currentFace2 = 1;  // �޸�
        break;
    case '9':
        currentFace = -1;
        currentFace2 = 2;  // �޸�
        break;
    case '0':
        currentFace = -1;
        currentFace2 = 3;  // ��� ��
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

    glutPostRedisplay();  // �ٽ� �׸���
}
