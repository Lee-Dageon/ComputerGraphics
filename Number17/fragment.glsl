#version 330 core

out vec4 FragColor;  // ���� ��� ����

in vec3 ourColor;    // vertex shader���� ���޹��� ����

void main()
{
    // ����� ������ ���� (ourColor ���� ���İ� 1.0�� �߰��Ͽ� ���)
    FragColor = vec4(ourColor, 1.0);
}


