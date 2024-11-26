#version 330 core

layout (location = 0) in vec3 aPos;  // ���� ��ġ (x, y, z)
layout (location = 1) in vec3 aColor; // ���� ���� (r, g, b)

out vec3 ourColor; // fragment shader�� ���޵� ����

uniform mat4 trans; // ��ȯ ��� (��, ��, ���� ���)

void main()
{
    // �Է� ���� ��ġ�� ��ȯ ��Ŀ� �����ؼ� gl_Position�� ����
    gl_Position = trans * vec4(aPos, 1.0);

    // ���� ������ fragment shader�� ����
    ourColor = aColor;
}