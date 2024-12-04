#version 330 core

layout (location = 0) in vec3 vPos;     // 정점 위치 (로컬 공간)
layout (location = 1) in vec3 vNormal;  // 정점 법선 (로컬 공간)

out vec3 FragPos;  // 프래그먼트 셰이더로 전달할 월드 좌표계의 정점 위치
out vec3 Normal;   // 프래그먼트 셰이더로 전달할 월드 좌표계의 법선 벡터

uniform mat4 model;      // 모델 변환 행렬
uniform mat4 view;       // 뷰 변환 행렬
uniform mat4 projection; // 투영 변환 행렬

void main() {
    // 정점의 월드 좌표 계산
    FragPos = vec3(model * vec4(vPos, 1.0)); 

    // 월드 좌표계에서의 법선 벡터 계산
    Normal = mat3(transpose(inverse(model))) * vNormal; 

    // 화면 좌표로 변환
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
