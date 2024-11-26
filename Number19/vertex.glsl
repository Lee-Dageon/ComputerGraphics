#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 fragColor;

uniform mat4 trans; // 모델 행렬
uniform mat4 view;  // 뷰 행렬
uniform mat4 projection; // 프로젝션 행렬

void main() {
    gl_Position = projection * view * trans * vec4(aPos, 1.0); // 최종 변환
    fragColor = aColor;
}
