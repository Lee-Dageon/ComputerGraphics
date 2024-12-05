#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightDirection; 
uniform float cutoff;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float shininess; // �ݻ籤 ���� ����
uniform float ambientStrength = 0.5; // �ֺ��� ����
uniform int lightEnabled;    // ���� ���� (1: ����, 0: ����)

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // ����Ʈ����Ʈ ���� ���
    float theta = dot(lightDir, normalize(-lightDirection));
    vec3 result = vec3(0.0);

    if (theta > cutoff) {
        // Diffuse Lighting
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular Lighting
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * lightColor;

        result += diffuse + specular;
    }

    // �ֺ��� ���
    vec3 ambient = ambientStrength * lightColor;
    result += ambient;

    // ���� ���� ���
    result *= objectColor;
    FragColor = vec4(result, 1.0);
}
