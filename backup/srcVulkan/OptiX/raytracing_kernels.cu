/**
 * @file raytracing_kernels.cu
 * @brief OptiX Ray Tracing CUDA kernels для вторичных эффектов
 * 
 * Этот файл содержит OptiX шейдеры для трассировки лучей:
 * - Ray Generation shader для запуска лучей
 * - Miss shader для обработки промахов
 * - Closest Hit shader для обработки пересечений
 * - Any Hit shader для прозрачности
 */

#include <optix.h>
#include <cuda_runtime.h>
#include <glm/glm.hpp>

using namespace glm;

/**
 * @brief Параметры запуска OptiX
 */
struct LaunchParams {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 cameraPos;
    vec3 lightPos;
    vec3 lightColor;
    float lightIntensity;
    uint32_t width;
    uint32_t height;
    uint32_t maxDepth;
    OptixTraversableHandle handle;
    
    // Выходные буферы
    float* reflections;
    float* shadows;
    float* globalIllumination;
    float* motionVectors;
    float* albedo;
    float* normals;
};

/**
 * @brief Ray payload для передачи данных между шейдерами
 */
struct RayPayload {
    vec3 color;          // Цвет
    float distance;      // Расстояние
    uint32_t depth;      // Глубина рекурсии
    vec3 normal;         // Нормаль поверхности
    vec3 position;       // Позиция пересечения
    float roughness;     // Шероховатость материала
    float metallic;      // Металличность
    vec3 albedo;         // Альбедо материала
};

/**
 * @brief Hit group data для материалов
 */
struct HitGroupData {
    vec3 albedo;
    float roughness;
    float metallic;
    float transparency;
    vec3 emission;
};

// Глобальные параметры запуска
extern "C" __constant__ LaunchParams params;

/**
 * @brief Генерация первичных лучей для отражений
 */
extern "C" __global__ void __raygen__reflections() {
    const uint3 idx = optixGetLaunchIndex();
    const uint3 dim = optixGetLaunchDimensions();
    
    if (idx.x >= params.width || idx.y >= params.height) {
        return;
    }
    
    // Вычисляем координаты пикселя в NDC
    const float x = (float(idx.x) + 0.5f) / float(params.width);
    const float y = (float(idx.y) + 0.5f) / float(params.height);
    const float nx = x * 2.0f - 1.0f;
    const float ny = (1.0f - y) * 2.0f - 1.0f;
    
    // Создаем луч для отражений
    vec3 rayDir = normalize(vec3(nx, ny, -1.0f));
    
    // Трансформируем луч в мировые координаты
    mat4 invView = inverse(params.viewMatrix);
    vec4 worldDir = invView * vec4(rayDir, 0.0f);
    rayDir = normalize(vec3(worldDir));
    
    // Инициализируем payload
    RayPayload payload;
    payload.color = vec3(0.0f);
    payload.distance = 1e16f;
    payload.depth = 0;
    payload.normal = vec3(0.0f);
    payload.position = vec3(0.0f);
    payload.roughness = 0.0f;
    payload.metallic = 0.0f;
    payload.albedo = vec3(0.0f);
    
    // Трассируем луч
    uint32_t u0, u1;
    packPointer(&payload, u0, u1);
    
    optixTrace(
        params.handle,              // traversable handle
        params.cameraPos,           // ray origin
        rayDir,                     // ray direction
        0.001f,                     // tmin
        1e16f,                      // tmax
        0.0f,                       // ray time
        OptixVisibilityMask(255),   // visibility mask
        OPTIX_RAY_FLAG_NONE,        // ray flags
        0,                          // SBT offset
        1,                          // SBT stride
        0,                          // miss SBT index
        u0, u1                      // payload
    );
    
    // Записываем результат в буфер отражений
    const uint32_t pixelIdx = idx.y * params.width + idx.x;
    const uint32_t colorIdx = pixelIdx * 4;
    
    params.reflections[colorIdx + 0] = payload.color.r;
    params.reflections[colorIdx + 1] = payload.color.g;
    params.reflections[colorIdx + 2] = payload.color.b;
    params.reflections[colorIdx + 3] = 1.0f; // Alpha
}

/**
 * @brief Генерация лучей для теней
 */
extern "C" __global__ void __raygen__shadows() {
    const uint3 idx = optixGetLaunchIndex();
    const uint3 dim = optixGetLaunchDimensions();
    
    if (idx.x >= params.width || idx.y >= params.height) {
        return;
    }
    
    // Простой тест теней от точечного источника света
    vec3 lightDir = normalize(params.lightPos - params.cameraPos);
    float lightDistance = length(params.lightPos - params.cameraPos);
    
    RayPayload payload;
    payload.color = vec3(1.0f); // Начинаем с полного освещения
    payload.distance = lightDistance;
    
    uint32_t u0, u1;
    packPointer(&payload, u0, u1);
    
    // Трассируем луч к источнику света
    optixTrace(
        params.handle,
        params.cameraPos,
        lightDir,
        0.001f,
        lightDistance - 0.001f,
        0.0f,
        OptixVisibilityMask(255),
        OPTIX_RAY_FLAG_TERMINATE_ON_FIRST_HIT, // Для теней достаточно первого пересечения
        1, // Offset для shadow rays
        1,
        1, // Miss program для теней
        u0, u1
    );
    
    // Записываем результат тени
    const uint32_t pixelIdx = idx.y * params.width + idx.x;
    const uint32_t colorIdx = pixelIdx * 4;
    
    float shadowFactor = payload.color.r; // 0 = тень, 1 = освещено
    params.shadows[colorIdx + 0] = shadowFactor;
    params.shadows[colorIdx + 1] = shadowFactor;
    params.shadows[colorIdx + 2] = shadowFactor;
    params.shadows[colorIdx + 3] = 1.0f;
}

/**
 * @brief Miss shader для отражений
 */
extern "C" __global__ void __miss__reflections() {
    RayPayload* payload = getPayload();
    
    // Простой градиентный фон для отражений
    const vec3 rayDir = optixGetWorldRayDirection();
    const float t = 0.5f * (rayDir.y + 1.0f);
    payload->color = mix(vec3(1.0f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f), t);
    payload->distance = 1e16f;
}

/**
 * @brief Miss shader для теней
 */
extern "C" __global__ void __miss__shadows() {
    RayPayload* payload = getPayload();
    
    // Если луч не попал ни во что - полное освещение
    payload->color = vec3(1.0f);
}

/**
 * @brief Closest hit shader для отражений
 */
extern "C" __global__ void __closesthit__reflections() {
    RayPayload* payload = getPayload();
    
    // Получаем данные о пересечении
    const float2 barycentrics = optixGetTriangleBarycentrics();
    const uint32_t primitiveIndex = optixGetPrimitiveIndex();
    const vec3 worldPos = optixGetWorldRayOrigin() + optixGetRayTmax() * optixGetWorldRayDirection();
    
    // Вычисляем нормаль (упрощенная версия)
    vec3 normal = normalize(cross(
        vec3(1.0f, 0.0f, 0.0f), // Примерные касательные векторы
        vec3(0.0f, 1.0f, 0.0f)
    ));
    
    // Получаем данные материала
    const HitGroupData* hitData = (HitGroupData*)optixGetSbtDataPointer();
    
    payload->normal = normal;
    payload->position = worldPos;
    payload->distance = optixGetRayTmax();
    payload->albedo = hitData->albedo;
    payload->roughness = hitData->roughness;
    payload->metallic = hitData->metallic;
    
    // Простое освещение Фонга
    vec3 lightDir = normalize(params.lightPos - worldPos);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    
    // Базовый цвет с учетом освещения
    payload->color = hitData->albedo * NdotL * params.lightColor * params.lightIntensity;
    
    // Рекурсивные отражения
    if (payload->depth < params.maxDepth && hitData->metallic > 0.1f) {
        vec3 rayDir = optixGetWorldRayDirection();
        vec3 reflectedDir = reflect(rayDir, normal);
        
        RayPayload reflectionPayload;
        reflectionPayload.depth = payload->depth + 1;
        
        uint32_t u0, u1;
        packPointer(&reflectionPayload, u0, u1);
        
        optixTrace(
            params.handle,
            worldPos + normal * 0.001f, // Смещение от поверхности
            reflectedDir,
            0.001f,
            1e16f,
            0.0f,
            OptixVisibilityMask(255),
            OPTIX_RAY_FLAG_NONE,
            0,
            1,
            0,
            u0, u1
        );
        
        // Смешиваем с отражением
        float fresnel = pow(1.0f - max(dot(-rayDir, normal), 0.0f), 5.0f);
        fresnel = mix(0.04f, 1.0f, fresnel); // Базовая отражательная способность
        
        payload->color = mix(payload->color, reflectionPayload.color, fresnel * hitData->metallic);
    }
}

/**
 * @brief Closest hit shader для теней
 */
extern "C" __global__ void __closesthit__shadows() {
    RayPayload* payload = getPayload();
    
    // Для теней - просто затеняем
    payload->color = vec3(0.0f);
}

/**
 * @brief Any hit shader для прозрачности
 */
extern "C" __global__ void __anyhit__transparency() {
    const HitGroupData* hitData = (HitGroupData*)optixGetSbtDataPointer();
    
    // Если материал прозрачный, игнорируем пересечение
    if (hitData->transparency > 0.5f) {
        optixIgnoreIntersection();
    }
}

/**
 * @brief Вспомогательная функция для распаковки указателя из payload
 */
__forceinline__ __device__ RayPayload* getPayload() {
    const uint32_t u0 = optixGetPayload_0();
    const uint32_t u1 = optixGetPayload_1();
    return reinterpret_cast<RayPayload*>(unpackPointer(u0, u1));
}

/**
 * @brief Упаковка указателя в два uint32_t для payload
 */
__forceinline__ __device__ void packPointer(void* ptr, uint32_t& u0, uint32_t& u1) {
    const uint64_t uptr = reinterpret_cast<uint64_t>(ptr);
    u0 = uptr >> 32;
    u1 = uptr & 0xFFFFFFFF;
}

/**
 * @brief Распаковка указателя из двух uint32_t
 */
__forceinline__ __device__ void* unpackPointer(uint32_t u0, uint32_t u1) {
    const uint64_t uptr = (static_cast<uint64_t>(u0) << 32) | u1;
    return reinterpret_cast<void*>(uptr);
}
