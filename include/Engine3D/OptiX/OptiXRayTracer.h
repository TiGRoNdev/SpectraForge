#pragma once

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT

#include <optix.h>
#include <optix_stubs.h>
#include <cuda_runtime.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Engine3D::OptiX {

/**
 * @brief Геометрия сцены для построения acceleration structures
 */
struct SceneGeometry {
    float* vertices;         // Массив вершин
    uint32_t* indices;       // Массив индексов
    uint32_t vertexCount;    // Количество вершин
    uint32_t triangleCount;  // Количество треугольников
    uint32_t vertexStride;   // Шаг между вершинами
};

/**
 * @brief Параметры запуска ray tracing
 */
struct LaunchParams {
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::vec3 cameraPos;
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    float lightIntensity;
    uint32_t width;
    uint32_t height;
    uint32_t maxDepth;
    OptixTraversableHandle handle;
};

/**
 * @brief Подсказки для Shader Execution Reordering
 */
struct CoherencyHints {
    float rayCoherence;      // Когерентность лучей [0-1]
    float materialCoherence; // Когерентность материалов [0-1]
    float geometryCoherence; // Когерентность геометрии [0-1]
};

/**
 * @brief Сырые эффекты от ray tracing
 */
struct RawEffects {
    float* reflections;      // Буфер отражений
    float* shadows;          // Буфер теней
    float* globalIllumination; // Буфер глобального освещения
    float* motionVectors;    // Векторы движения
    float* albedo;           // Альбедо для деноизинга
    float* normals;          // Нормали для деноизинга
    uint32_t width;
    uint32_t height;
    cudaStream_t stream;
};

/**
 * @brief Shader Binding Table для OptiX
 */
class ShaderBindingTable {
public:
    ShaderBindingTable();
    ~ShaderBindingTable();
    
    bool init(OptixPipeline pipeline);
    void shutdown();
    
    OptixShaderBindingTable getSBT() const { return sbt; }

private:
    OptixShaderBindingTable sbt = {};
    CUdeviceptr raygenRecord = 0;
    CUdeviceptr missRecords = 0;
    CUdeviceptr hitgroupRecords = 0;
    
    bool initialized = false;
};

/**
 * @brief Acceleration Structure для OptiX
 */
class AccelerationStructure {
public:
    AccelerationStructure();
    ~AccelerationStructure();
    
    bool buildBLAS(const SceneGeometry& geometry);
    bool buildTLAS(const std::vector<OptixInstance>& instances);
    void rebuild();
    
    OptixTraversableHandle getHandle() const { return traversableHandle; }

private:
    OptixTraversableHandle traversableHandle = 0;
    CUdeviceptr blasBuffer = 0;
    CUdeviceptr tlasBuffer = 0;
    size_t blasBufferSize = 0;
    size_t tlasBufferSize = 0;
    
    bool initialized = false;
};

/**
 * @brief OptiX Ray Tracer
 * 
 * Реализует ray tracing для вторичных эффектов с использованием NVIDIA OptiX.
 * Поддерживает Shader Execution Reordering для повышения производительности.
 */
class OptiXRayTracer {
public:
    /**
     * @brief Конструктор
     */
    OptiXRayTracer();
    
    /**
     * @brief Деструктор
     */
    ~OptiXRayTracer();
    
    /**
     * @brief Инициализация OptiX ray tracer
     * @param cudaContext CUDA контекст
     * @return true если инициализация успешна
     */
    bool init(CUcontext cudaContext);
    
    /**
     * @brief Завершение работы
     */
    void shutdown();
    
    /**
     * @brief Построение acceleration structures
     * @param geo Геометрия сцены
     */
    void buildAccelerationStructures(const SceneGeometry& geo);
    
    /**
     * @brief Трассировка лучей
     * @param params Параметры запуска
     * @return Сырые эффекты
     */
    RawEffects traceRays(const LaunchParams& params);
    
    /**
     * @brief Применение Shader Execution Reordering
     * @param hints Подсказки когерентности
     */
    void applySER(const CoherencyHints& hints);
    
    /**
     * @brief Установка максимальной глубины трассировки
     * @param depth Максимальная глубина
     */
    void setMaxTraceDepth(uint32_t depth) { maxTraceDepth = depth; }
    
    /**
     * @brief Получение OptiX контекста
     * @return OptiX device context
     */
    OptixDeviceContext getContext() const { return context; }
    
    /**
     * @brief Получение pipeline
     * @return OptiX pipeline
     */
    OptixPipeline getPipeline() const { return pipeline; }

private:
    OptixDeviceContext context = nullptr;
    OptixPipeline pipeline = nullptr;
    OptixModule module = nullptr;
    
    std::unique_ptr<ShaderBindingTable> sbt;
    std::unique_ptr<AccelerationStructure> as;
    
    // CUDA буферы для результатов
    CUdeviceptr d_reflections = 0;
    CUdeviceptr d_shadows = 0;
    CUdeviceptr d_globalIllumination = 0;
    CUdeviceptr d_motionVectors = 0;
    CUdeviceptr d_albedo = 0;
    CUdeviceptr d_normals = 0;
    
    // Параметры
    uint32_t maxTraceDepth = 10;
    uint32_t imageWidth = 1920;
    uint32_t imageHeight = 1080;
    
    bool initialized = false;
    
    /**
     * @brief Создание OptiX модуля из PTX
     * @return true если создание успешно
     */
    bool createModule();
    
    /**
     * @brief Создание OptiX pipeline
     * @return true если создание успешно
     */
    bool createPipeline();
    
    /**
     * @brief Выделение CUDA буферов
     * @return true если выделение успешно
     */
    bool allocateBuffers();
    
    /**
     * @brief Освобождение CUDA буферов
     */
    void freeBuffers();
    
    /**
     * @brief Callback для логирования OptiX
     */
    static void logCallback(unsigned int level, const char* tag, const char* message, void* cbdata);
    
    // Запрет копирования
    OptiXRayTracer(const OptiXRayTracer&) = delete;
    OptiXRayTracer& operator=(const OptiXRayTracer&) = delete;
};

} // namespace Engine3D::OptiX

#else // !VULKAN_RENDERER_OPTIX_SUPPORT

// Заглушка для случая, когда OptiX не поддерживается
namespace Engine3D::OptiX {

// Forward declarations для заглушек
struct SceneGeometry {};
struct LaunchParams {};
struct CoherencyHints {};
struct RawEffects {};

/**
 * @brief Заглушка OptiX Ray Tracer для случая без поддержки OptiX
 */
class OptiXRayTracer {
public:
    OptiXRayTracer() = default;
    ~OptiXRayTracer() = default;
    
    bool init() { return false; }
    void shutdown() {}
    
    void buildAccelerationStructures(const SceneGeometry&) {}
    RawEffects traceRays(const LaunchParams&) { return RawEffects{}; }
    void applySER(const CoherencyHints&) {}
    
    bool isInitialized() const { return false; }
    
private:
    OptiXRayTracer(const OptiXRayTracer&) = delete;
    OptiXRayTracer& operator=(const OptiXRayTracer&) = delete;
};

} // namespace Engine3D::OptiX

#endif // VULKAN_RENDERER_OPTIX_SUPPORT
