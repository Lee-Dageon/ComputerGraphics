#version 330 core

in vec3 vertexColor; // 버텍스 셰이더로부터 전달된 색상

out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0); // 전달된 색상을 사용하여 출력
}
