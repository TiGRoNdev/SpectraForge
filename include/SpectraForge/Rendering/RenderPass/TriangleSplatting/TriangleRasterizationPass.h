#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <memory>

namespace spectraforge {
namespace rendering {

// Forward declarations
class TriangleSplattingCore;
class TriangleBufferManager;

/**
 * @brief SDF-based triangle rasterization pass
 * 
 * Ответственность: Растеризация треугольников с использованием Signed Distance Field
 * и alpha blending для создания финального изображения.
 * 
 * Поддерживает 2 режима:
 * 1. Single-Pass - классический O(N×M), проще но медленнее
 * 2. Two-Pass (Visibility + Shading) - оптимизированный O(N+M), быстрее
 * 
 * SOLID Compliance:
 * - SRP: Только рендеринг треугольников (не culling, не сортировка)
 * - OCP: Можно добавить новые режимы рендеринга
 * - DIP: Зависит от абстракций Core и BufferManager
 * 
 * Алгоритм:
 * - Для каждого пикселя вычисляется SDF к каждому треугольнику
 * - Window function определяет contribution треугольника
 * - Alpha blending выполняется front-to-back с early termination
 */
class TriangleRasterizationPass {
public:
    /**
     * @brief Конфигурация rasterization pass
     */
    struct Config {
        uint32_t outputWidth = 1920;          ///< Ширина output image
        uint32_t outputHeight = 1080;         ///< Высота output image
        bool enableEarlyTermination = true;   ///< Ранняя остановка при alpha > threshold
        float alphaThreshold = 0.99f;         ///< Порог для early termination
        bool enableTwoPassRendering = true;   ///< Two-Pass optimization (O(N+M))
        uint32_t maxTrianglesPerPixel = 64;   ///< Лимит треугольников на пиксель (для Two-Pass)
        uint32_t debugMode = 0;               ///< 0=normal, 1=SDF, 2=barycentric
    };
    
    /**
     * @brief Инициализация rasterization pass
     * @param device Vulkan device
     * @param allocator VMA allocator
     * @param core Core для доступа к output image
     * @param bufferManager Buffer manager для triangle data
     * @param config Конфигурация
     * @return true если успешно
     */
    bool initialize(vk::Device device,
                   VmaAllocator allocator,
                   const TriangleSplattingCore& core,
                   const TriangleBufferManager& bufferManager,
                   const Config& config);
    
    /**
     * @brief Очистка ресурсов
     */
    void cleanup();
    
    /**
     * @brief Выполнение single-pass рендеринга
     * @param cmd Command buffer
     * @param viewProj View-Projection matrix
     * @param triangleCount Количество треугольников для рендеринга
     */
    void execute(vk::CommandBuffer cmd,
                const glm::mat4& viewProj,
                uint32_t triangleCount);
    
    /**
     * @brief Выполнение two-pass рендеринга (Visibility + Shading)
     * @param cmd Command buffer
     * @param viewProj View-Projection matrix
     * @param triangleCount Количество треугольников
     */
    void executeTwoPass(vk::CommandBuffer cmd,
                       const glm::mat4& viewProj,
                       uint32_t triangleCount);
    
    /**
     * @brief Установка debug режима
     * @param mode 0=normal, 1=SDF visualization, 2=barycentric coordinates
     */
    void setDebugMode(uint32_t mode);
    
    /**
     * @brief Обновление конфигурации
     */
    void updateConfig(const Config& config) { config_ = config; }
    
    const Config& getConfig() const { return config_; }
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    Config config_;
    
    vk::Device device_;
    VmaAllocator allocator_;
    
    // === Single-Pass Resources ===
    vk::ShaderModule triangleSplattingShader_;
    vk::Pipeline triangleSplattingPipeline_;
    vk::PipelineLayout triangleSplattingPipelineLayout_;
    vk::DescriptorSetLayout triangleSplattingDescriptorSetLayout_;
    vk::DescriptorPool triangleSplattingDescriptorPool_;
    vk::DescriptorSet triangleSplattingDescriptorSet_;
    
    // === Two-Pass Resources ===
    // Visibility Pass
    vk::ShaderModule visibilityPassShader_;
    vk::Pipeline visibilityPassPipeline_;
    vk::PipelineLayout visibilityPassPipelineLayout_;
    vk::DescriptorSetLayout visibilityPassDescriptorSetLayout_;
    vk::DescriptorPool visibilityPassDescriptorPool_;
    vk::DescriptorSet visibilityPassDescriptorSet_;
    
    // Shading Pass
    vk::ShaderModule shadingPassShader_;
    vk::Pipeline shadingPassPipeline_;
    vk::PipelineLayout shadingPassPipelineLayout_;
    vk::DescriptorSetLayout shadingPassDescriptorSetLayout_;
    vk::DescriptorPool shadingPassDescriptorPool_;
    vk::DescriptorSet shadingPassDescriptorSet_;
    
    // Visibility buffer (для Two-Pass)
    vk::Buffer visibilityBuffer_;
    VmaAllocation visibilityBufferAllocation_ = VK_NULL_HANDLE;
    
    // Helper functions
    bool loadShaders();
    bool createSinglePassPipeline();
    bool createTwoPassPipelines();
    bool createDescriptorSets(const TriangleSplattingCore& core,
                             const TriangleBufferManager& bufferManager);
    bool createVisibilityBuffer(uint32_t outputWidth, uint32_t outputHeight);
    
    void executeVisibilityPass(vk::CommandBuffer cmd, const glm::mat4& viewProj, uint32_t triangleCount);
    void executeShadingPass(vk::CommandBuffer cmd, const glm::mat4& viewProj, uint32_t triangleCount);
    
    // Push constants
    struct RasterizationPushConstants {
        glm::mat4 viewProj;
        uint32_t outputWidth;
        uint32_t outputHeight;
        uint32_t triangleCount;
        uint32_t enableEarlyTermination;
        float alphaThreshold;
        uint32_t debugMode;
    };
};

} // namespace rendering
} // namespace spectraforge

