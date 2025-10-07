#version 450

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec4 vColor;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    // простое ламбертово освещение от фиксированного направления
    vec3 L = normalize(vec3(0.3, 0.7, 0.6));
    float diff = max(dot(N, L), 0.0);
    vec3 color = vColor.rgb * (0.1 + 0.9 * diff);
    FragColor = vec4(color, vColor.a);
}


