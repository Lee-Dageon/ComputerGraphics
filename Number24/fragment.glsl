#version 330 core

in vec3 FragPos;       // 정점 셰이더에서 전달된 월드 좌표
in vec3 Normal;        // 정점 셰이더에서 전달된 법선 벡터

out vec4 FragColor;    // 최종 색상 출력

uniform vec3 lightPos; // 광원의 위치
uniform vec3 lightColor; // 광원의 색상
uniform vec3 objectColor; // 객체의 기본 색상

void main() {

    // 주변광 (Ambient Light)
    float ambientStrength = 0.3;  // 주변광 계수
    vec3 ambient = ambientStrength * lightColor;

    // 산란광 (Diffuse Light)
    vec3 normalVector = normalize(Normal); // 법선 벡터 정규화
    vec3 lightDir = normalize(lightPos - FragPos); // 광원 방향 벡터 계산
    float diffuseStrength = max(dot(normalVector, lightDir), 0.0); // 법선과 광원 방향의 내적 값
    vec3 diffuse = diffuseStrength * lightColor;

    // 최종 색상 계산
    vec3 result = (ambient + diffuse) * objectColor; // 조명과 객체 색상 결합
    FragColor = vec4(result, 1.0); // 최종 색상 출력
}
