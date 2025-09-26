#version 450

// Входные данные вершин
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inTexCoord;
layout(location = 3) in vec4 inColor;

// Выходные данные для фрагментного шейдера
layout(location = 0) out vec4 fragWorldPos;
layout(location = 1) out vec4 fragViewPos;
layout(location = 2) out vec4 fragNormal;
layout(location = 3) out vec4 fragTexCoord;
layout(location = 4) out vec4 fragColor;

// Push constants для forward+ shading
layout(push_constant) uniform ForwardPlusPushConstants {
    mat4 model;
    mat4 normalMatrix;
    uint materialID;
    uint enableLighting;
    uint enableTextures;
    uint padding;
} pc;

// Uniform buffer
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
    // Трансформируем позицию в мировые координаты
    vec4 worldPos = pc.model * inPosition;
    fragWorldPos = worldPos;
    
    // Применяем сечение в 4D пространстве если включено
    bool useProjection = true;
    if (uniforms.useCrossSection != 0) {
        // Если точка слишком далеко от сечения, отбрасываем её
        float wDistance = abs(worldPos.w - uniforms.crossSectionW);
        if (wDistance > 0.1) {
            gl_Position = vec4(0.0, 0.0, -1.0, 1.0); // За пределами clip space
            return;
        }
        
        // Применяем сечение, проецируя 4D в 3D
        worldPos.w = 0.0;
        useProjection = false;
    }
    
    // Проецируем 4D координаты в 3D для рендеринга
    vec3 pos3D = worldPos.xyz;
    if (useProjection) {
        // Перспективная проекция 4D->3D
        float wDistance = 5.0; // Расстояние для проекции
        if (worldPos.w != 0.0) {
            float scale = wDistance / (wDistance + worldPos.w);
            pos3D *= scale;
        }
    }
    
    // Трансформируем в view space
    vec4 viewPos = uniforms.view * vec4(pos3D, 1.0);
    fragViewPos = viewPos;
    
    // Трансформируем нормаль
    vec4 worldNormal = pc.normalMatrix * inNormal;
    
    // Проецируем нормаль так же, как и позицию
    vec3 normal3D = worldNormal.xyz;
    if (useProjection && worldPos.w != 0.0) {
        float wDistance = 5.0;
        float scale = wDistance / (wDistance + worldPos.w);
        normal3D *= scale;
    }
    
    // Нормализуем и трансформируем в view space
    fragNormal = vec4(mat3(uniforms.view) * normalize(normal3D), 0.0);
    
    // Передаем текстурные координаты и цвет
    fragTexCoord = inTexCoord;
    fragColor = inColor;
    
    // Финальная трансформация в clip space
    gl_Position = uniforms.projection * viewPos;
}
