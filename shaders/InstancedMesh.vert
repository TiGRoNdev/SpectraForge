#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// Push constants для глобальных матриц (128 байт)
layout(push_constant) uniform CameraPC {
    mat4 uView;
    mat4 uProj;
} cam;

struct InstanceData {
    mat4 model;
    vec4 color; // rgb + alpha
};

layout(set = 0, binding = 1, std430) readonly buffer InstanceBuffer {
    InstanceData instances[];
};

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec4 vColor;

void main() {
    uint idx = gl_InstanceIndex;
    InstanceData inst = instances[idx];

    mat4 model = inst.model;
    vec4 worldPos = model * vec4(aPos, 1.0);
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vUV = aUV;
    vColor = inst.color;

    gl_Position = cam.uProj * cam.uView * worldPos;
}


