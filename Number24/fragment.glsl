#version 330 core

in vec3 FragPos;      // ���� ��ġ (���� ��ǥ��)
in vec3 Normal;       // ���� ���� ���� (���� ��ǥ��)

out vec4 FragColor;   // ���� ���� ���

uniform vec3 lightPos;         // ������ ��ġ
uniform vec3 lightDirection;   // ����Ʈ����Ʈ ����
uniform float cutoff;          // ���� �ƿ��� ���� (�ڻ��� ��)
uniform float outerCutoff;     // �ܺ� �ƿ��� ���� (�ڻ��� ��)
uniform vec3 lightColor;       // ������ ����
uniform vec3 objectColor;      // ��ü�� �⺻ ����
uniform vec3 viewPos;          // ī�޶� ��ġ
uniform float shininess;       // �ݻ籤 ����
uniform float ambientStrength = 0.3f; // �ֺ��� ����
uniform int lightEnabled;      // ���� ���� (1: ����, 0: ����)

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // �⺻ ����: �ֺ��� (�׻� ����)
    vec3 ambient = ambientStrength * lightColor;
    vec3 lighting = ambient; // ������ ���� ambient�� �׻� ����

    if (lightEnabled == 1) {
        // ����Ʈ����Ʈ ���� ���
        float theta = dot(lightDir, normalize(-lightDirection));

        // Soft Spotlight ���
        float intensity = 0.0;
        if (theta > outerCutoff) {  // ���� ���� ���� ���� ���� ���
            if (theta > cutoff) {
                intensity = 1.0; // ���� �ƿ��� ���� �������� Ǯ ����
            } else {
                // ��迡�� �ε巯�� ����
                float smoothFactor = (theta - outerCutoff) / (cutoff - outerCutoff);
                intensity = clamp(smoothFactor, 0.0, 1.0);
            }
        }

        // Diffuse Lighting (�����)
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * intensity;

        // Specular Lighting (�ݻ籤)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * lightColor * intensity;

        // ���� �߰� (�ֺ��� + ����� + �ݻ籤)
        lighting += diffuse + specular;
    }

    // ���� ����
    vec3 result = lighting * objectColor;
    FragColor = vec4(result, 1.0);
}
