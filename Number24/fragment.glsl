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
uniform float shininess; // 반사광 강도 조정
uniform float ambientStrength = 0.5; // 주변광 강도
uniform int lightEnabled;    // 조명 상태 (1: 켜짐, 0: 꺼짐)

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 스포트라이트 각도 계산
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

    // 주변광 계산
    vec3 ambient = ambientStrength * lightColor;
    result += ambient;

    // 최종 색상 계산
    result *= objectColor;
    FragColor = vec4(result, 1.0);
}
