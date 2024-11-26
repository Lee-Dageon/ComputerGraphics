#version 330 core

layout (location = 0) in vec3 aPos;     // 정점 위치
layout (location = 1) in vec3 aColor;   // 정점 색상

out vec3 vertexColor;   // 프래그먼트 셰이더로 전달할 색상

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;   // 프래그먼트 셰이더로 색상 전달
}
