#version 330 core

layout (location = 0) in vec3 aPos;  // 정점 위치 (x, y, z)
layout (location = 1) in vec3 aColor; // 정점 색상 (r, g, b)

out vec3 ourColor; // fragment shader로 전달될 변수

uniform mat4 trans; // 변환 행렬 (모델, 뷰, 투영 행렬)

void main()
{
    // 입력 받은 위치를 변환 행렬에 적용해서 gl_Position에 전달
    gl_Position = trans * vec4(aPos, 1.0);

    // 정점 색상을 fragment shader로 전달
    ourColor = aColor;
}