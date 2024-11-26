#version 330 core

out vec4 FragColor;  // 최종 출력 색상

in vec3 ourColor;    // vertex shader에서 전달받은 색상

void main()
{
    // 출력할 색상을 설정 (ourColor 값에 알파값 1.0을 추가하여 출력)
    FragColor = vec4(ourColor, 1.0);
}


