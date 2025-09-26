#version 450

// Входные данные вершин
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inTexCoord;
layout(location = 3) in vec4 inColor;

// Push constants для depth prepass
layout(push_constant) uniform DepthPrepassPushConstants {
    mat4 model;
    mat4 mvp;
} pc;

// Uniform buffer для общих данных камеры
layout(set = 0, binding = 0) uniform ForwardPlusUniforms {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 viewPosition;
    vec4 screenDimensions;
    uint tileCountX;
    uint tileCountY;
    uint totalTileCount;
    uint lightCount;
    float nearPlane;
    float farPlane;
    float crossSectionW;
    uint useCrossSection;
} uniforms;

void main() {
    vec4 worldPos = pc.model * inPosition;
    
    // Применяем сечение в 4D пространстве если включено
    if (uniforms.useCrossSection != 0) {
        // Если точка слишком далеко от сечения, отбрасываем её
        float wDistance = abs(worldPos.w - uniforms.crossSectionW);
        if (wDistance > 0.1) {
            gl_Position = vec4(0.0, 0.0, -1.0, 1.0); // За пределами clip space
            return;
        }
        
        // Применяем сечение, проецируя 4D в 3D
        worldPos.w = 0.0;
    }
    
    // Проецируем 4D координаты в 3D для рендеринга
    vec3 pos3D = worldPos.xyz;
    if (!bool(uniforms.useCrossSection)) {
        // Перспективная проекция 4D->3D
        float wDistance = 5.0; // Расстояние для проекции
        if (worldPos.w != 0.0) {
            float scale = wDistance / (wDistance + worldPos.w);
            pos3D *= scale;
        }
    }
    
    // Применяем view и projection матрицы
    vec4 viewPos = uniforms.view * vec4(pos3D, 1.0);
    gl_Position = uniforms.projection * viewPos;
}
