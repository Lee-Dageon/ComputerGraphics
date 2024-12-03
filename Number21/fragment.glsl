#version 330 core

uniform vec3 faceColor; // 면별 색상

out vec4 FragColor;

void main() {
    FragColor = vec4(faceColor, 1.0); // 단색 출력
}
