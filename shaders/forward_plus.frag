#version 450

// Входные данные из вершинного шейдера
layout(location = 0) in vec4 fragWorldPos;
layout(location = 1) in vec4 fragViewPos;
layout(location = 2) in vec4 fragNormal;
layout(location = 3) in vec4 fragTexCoord;
layout(location = 4) in vec4 fragColor;

// Выходной цвет
layout(location = 0) out vec4 outColor;

// Структуры данных
struct GPULight {
    vec4 position;      // Позиция света (w=0 для направленного)
    vec4 color;         // Цвет и интенсивность (w=интенсивность)
    vec4 direction;     // Направление (для направленного/прожектора)
    float radius;       // Радиус влияния
    float innerCone;    // Внутренний угол конуса
    float outerCone;    // Внешний угол конуса
    uint type;          // Тип света: 0=точечный, 1=направленный, 2=прожектор
};

struct TileData {
    uint lightCount;
    uint lightIndices[256]; // MAX_LIGHTS_PER_TILE
};

struct ForwardPlusMaterial {
    vec4 albedo;         // Базовый цвет (RGBA)
    vec4 emission;       // Эмиссия
    float metallic;      // Металличность
    float roughness;     // Шероховатость
    float normalScale;   // Масштаб normal map
    uint hasAlbedoTexture;
    uint hasNormalTexture;
    uint hasMetallicRoughnessTexture;
    uint hasEmissionTexture;
    uint padding;
};

// Push constants
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

// Storage buffers
layout(set = 1, binding = 0, std430) restrict readonly buffer LightBuffer {
    GPULight lights[];
};

layout(set = 1, binding = 1, std430) restrict readonly buffer TileDataBuffer {
    TileData tileData[];
};

// Материалы (пока упрощенно)
layout(set = 2, binding = 0, std430) restrict readonly buffer MaterialBuffer {
    ForwardPlusMaterial materials[];
};

// Текстуры материалов (если есть)
layout(set = 3, binding = 0) uniform sampler2D albedoTexture;
layout(set = 3, binding = 1) uniform sampler2D normalTexture;
layout(set = 3, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 3, binding = 3) uniform sampler2D emissionTexture;

// Константы
const float PI = 3.14159265359;
const float AMBIENT_STRENGTH = 0.1;

// PBR функции
vec3 getNormalFromMap() {
    if (pc.materialID < materials.length() && 
        materials[pc.materialID].hasNormalTexture != 0 &&
        pc.enableTextures != 0) {
        
        vec3 tangentNormal = texture(normalTexture, fragTexCoord.xy).xyz * 2.0 - 1.0;
        tangentNormal *= materials[pc.materialID].normalScale;
        
        // Упрощенное применение normal map без tangent space
        // В полной реализации нужен tangent space
        return normalize(fragNormal.xyz + tangentNormal * 0.1);
    }
    return normalize(fragNormal.xyz);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculatePointLight(GPULight light, vec3 fragPos, vec3 normal, vec3 viewDir, 
                        vec3 albedo, float metallic, float roughness) {
    // Позиция света в view space
    vec3 lightPos = (uniforms.view * light.position).xyz;
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Расстояние и затухание
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    // Учитываем радиус света
    if (distance > light.radius) {
        attenuation = 0.0;
    }
    
    // PBR вычисления
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    float NDF = distributionGGX(normal, halfwayDir, roughness);
    float G = geometrySmith(normal, viewDir, lightDir, roughness);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 radiance = light.color.rgb * light.color.w * attenuation;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculateDirectionalLight(GPULight light, vec3 normal, vec3 viewDir, 
                              vec3 albedo, float metallic, float roughness) {
    // Направление света в view space
    vec3 lightDir = normalize(-(uniforms.view * light.direction).xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // PBR вычисления (аналогично точечному свету, но без затухания)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    float NDF = distributionGGX(normal, halfwayDir, roughness);
    float G = geometrySmith(normal, viewDir, lightDir, roughness);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 radiance = light.color.rgb * light.color.w;
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
    // Определяем тайл для текущего фрагмента
    ivec2 tileID = ivec2(gl_FragCoord.xy) / 16; // TILE_SIZE
    uint tileIndex = tileID.y * uniforms.tileCountX + tileID.x;
    
    // Получаем материал
    vec3 albedo = fragColor.rgb;
    float metallic = 0.0;
    float roughness = 0.5;
    vec3 emission = vec3(0.0);
    
    if (pc.materialID < materials.length()) {
        ForwardPlusMaterial material = materials[pc.materialID];
        albedo = material.albedo.rgb;
        metallic = material.metallic;
        roughness = material.roughness;
        emission = material.emission.rgb;
        
        // Применяем текстуры если есть
        if (pc.enableTextures != 0) {
            if (material.hasAlbedoTexture != 0) {
                albedo *= texture(albedoTexture, fragTexCoord.xy).rgb;
            }
            
            if (material.hasMetallicRoughnessTexture != 0) {
                vec3 mr = texture(metallicRoughnessTexture, fragTexCoord.xy).rgb;
                metallic *= mr.b;  // Blue channel = metallic
                roughness *= mr.g; // Green channel = roughness
            }
            
            if (material.hasEmissionTexture != 0) {
                emission += texture(emissionTexture, fragTexCoord.xy).rgb;
            }
        }
    }
    
    // Получаем нормаль
    vec3 normal = getNormalFromMap();
    
    // View direction
    vec3 viewDir = normalize(-fragViewPos.xyz);
    
    vec3 finalColor = vec3(0.0);
    
    // Если освещение включено
    if (pc.enableLighting != 0 && tileIndex < uniforms.totalTileCount) {
        // Ambient освещение
        vec3 ambient = AMBIENT_STRENGTH * albedo;
        finalColor += ambient;
        
        // Получаем список света для этого тайла
        uint lightCount = tileData[tileIndex].lightCount;
        
        // Применяем каждый свет из тайла
        for (uint i = 0; i < lightCount && i < 256; ++i) {
            uint lightIndex = tileData[tileIndex].lightIndices[i];
            if (lightIndex < uniforms.lightCount) {
                GPULight light = lights[lightIndex];
                
                if (light.type == 0) { // Точечный свет
                    finalColor += calculatePointLight(light, fragViewPos.xyz, normal, 
                                                    viewDir, albedo, metallic, roughness);
                }
                else if (light.type == 1) { // Направленный свет
                    finalColor += calculateDirectionalLight(light, normal, viewDir, 
                                                          albedo, metallic, roughness);
                }
                // TODO: Добавить поддержку прожекторов (type == 2)
            }
        }
    } else {
        // Без освещения - просто базовый цвет
        finalColor = albedo;
    }
    
    // Добавляем эмиссию
    finalColor += emission;
    
    // Простой tone mapping
    finalColor = finalColor / (finalColor + vec3(1.0));
    
    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0/2.2));
    
    outColor = vec4(finalColor, fragColor.a);
}
