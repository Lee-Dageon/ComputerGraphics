#version 330 core

layout (location = 0) in vec3 vPos;     // ���� ��ġ (���� ����)
layout (location = 1) in vec3 vNormal;  // ���� ���� (���� ����)

out vec3 FragPos;  // �����׸�Ʈ ���̴��� ������ ���� ��ǥ���� ���� ��ġ
out vec3 Normal;   // �����׸�Ʈ ���̴��� ������ ���� ��ǥ���� ���� ����

uniform mat4 model;      // �� ��ȯ ���
uniform mat4 view;       // �� ��ȯ ���
uniform mat4 projection; // ���� ��ȯ ���

void main() {
    // ������ ���� ��ǥ ���
    FragPos = vec3(model * vec4(vPos, 1.0)); 

    // ���� ��ǥ�迡���� ���� ���� ���
    Normal = mat3(transpose(inverse(model))) * vNormal; 

    // ȭ�� ��ǥ�� ��ȯ
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
