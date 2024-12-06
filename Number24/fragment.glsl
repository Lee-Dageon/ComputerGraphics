#version 330 core

in vec3 FragPos;      // 정점 위치 (월드 좌표계)
in vec3 Normal;       // 정점 법선 벡터 (월드 좌표계)

out vec4 FragColor;   // 최종 색상 출력

uniform vec3 lightPos;         // 광원의 위치
uniform vec3 lightDirection;   // 스포트라이트 방향
uniform float cutoff;          // 내부 컷오프 각도 (코사인 값)
uniform float outerCutoff;     // 외부 컷오프 각도 (코사인 값)
uniform vec3 lightColor;       // 광원의 색상
uniform vec3 objectColor;      // 물체의 기본 색상
uniform vec3 viewPos;          // 카메라 위치
uniform float shininess;       // 반사광 강도
uniform float ambientStrength = 0.3f; // 주변광 강도
uniform int lightEnabled;      // 조명 상태 (1: 켜짐, 0: 꺼짐)

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 기본 조명: 주변광 (항상 유지)
    vec3 ambient = ambientStrength * lightColor;
    vec3 lighting = ambient; // 조명을 꺼도 ambient는 항상 유지

    if (lightEnabled == 1) {
        // 스포트라이트 각도 계산
        float theta = dot(lightDir, normalize(-lightDirection));

        // Soft Spotlight 계산
        float intensity = 0.0;
        if (theta > outerCutoff) {  // 광원 영향 영역 내에 있을 경우
            if (theta > cutoff) {
                intensity = 1.0; // 내부 컷오프 각도 내에서는 풀 강도
            } else {
                // 경계에서 부드러운 감쇠
                float smoothFactor = (theta - outerCutoff) / (cutoff - outerCutoff);
                intensity = clamp(smoothFactor, 0.0, 1.0);
            }
        }

        // Diffuse Lighting (산란광)
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * intensity;

        // Specular Lighting (반사광)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spec * lightColor * intensity;

        // 조명 추가 (주변광 + 산란광 + 반사광)
        lighting += diffuse + specular;
    }

    // 최종 색상
    vec3 result = lighting * objectColor;
    FragColor = vec4(result, 1.0);
}
