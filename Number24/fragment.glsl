#version 330 core

in vec3 FragPos;       // ���� ���̴����� ���޵� ���� ��ǥ
in vec3 Normal;        // ���� ���̴����� ���޵� ���� ����

out vec4 FragColor;    // ���� ���� ���

uniform vec3 lightPos; // ������ ��ġ
uniform vec3 lightColor; // ������ ����
uniform vec3 objectColor; // ��ü�� �⺻ ����

void main() {

    // �ֺ��� (Ambient Light)
    float ambientStrength = 0.3;  // �ֺ��� ���
    vec3 ambient = ambientStrength * lightColor;

    // ����� (Diffuse Light)
    vec3 normalVector = normalize(Normal); // ���� ���� ����ȭ
    vec3 lightDir = normalize(lightPos - FragPos); // ���� ���� ���� ���
    float diffuseStrength = max(dot(normalVector, lightDir), 0.0); // ������ ���� ������ ���� ��
    vec3 diffuse = diffuseStrength * lightColor;

    // ���� ���� ���
    vec3 result = (ambient + diffuse) * objectColor; // ����� ��ü ���� ����
    FragColor = vec4(result, 1.0); // ���� ���� ���
}
