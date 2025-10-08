#pragma once

#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingTypes.h>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace spectraforge {
namespace rendering {

// Forward declarations
class TriangleSplattingCore;
class TriangleBufferManager;
class FrustumCullingPass;
class DepthSortingPass;
class TriangleRasterizationPass;
class TriangleSplattingDebugger;
class TriangleSplattingStatistics;

/**
 * @brief Orchestrator для Triangle Splatting pipeline
 * 
 * Ответственность: Координация всех подсистем Triangle Splatting.
 * НЕ выполняет рендеринг напрямую, а делегирует задачи компонентам.
 * 
 * SOLID Compliance:
 * - SRP: Только координация (не рендеринг, не culling, не сортировка)
 * - OCP: Можно заменить любой компонент через DI
 * - LSP: Реализует RenderPass интерфейс
 * - ISP: Зависит только от нужных интерфейсов
 * - DIP: Все зависимости через constructor injection
 * 
 * Pipeline sequence:
 * 1. Core initialization
 * 2. Upload triangles → BufferManager
 * 3. Frustum culling → FrustumCullingPass
 * 4. Depth sorting → DepthSortingPass
 * 5. Rasterization → TriangleRasterizationPass
 * 6. Statistics collection → TriangleSplattingStatistics
 * 
 * Architecture: Facade + Strategy pattern
 */
class TriangleSplattingPass {
public:
    /**
     * @brief Конфигурация Triangle Splatting pipeline
     */
    struct Config {
        uint32_t outputWidth = 1920;
        uint32_t outputHeight = 1080;
        uint32_t maxTriangles = 100000;
        
        bool enableFrustumCulling = true;
        bool enableDepthSorting = true;
        bool enableEarlyTermination = true;
        float alphaThreshold = 0.99f;
        bool enableTwoPassRendering = true;
    };
    
    /**
     * @brief Конструктор с конфигурацией
     */
    explicit TriangleSplattingPass(const Config& config);
    
    /**
     * @brief Деструктор
     */
    ~TriangleSplattingPass();
    
    /**
     * @brief Инициализация всех подсистем
     */
    bool initialize(vk::Device device,
                   VmaAllocator allocator,
                   vk::Queue computeQueue,
                   vk::Queue graphicsQueue,
                   vk::CommandPool commandPool);
    
    /**
     * @brief Выполнение полного Triangle Splatting pipeline
     */
    void execute(vk::CommandBuffer cmd, uint32_t frameIndex);
    
    /**
     * @brief Очистка всех ресурсов
     */
    void cleanup();
    
    // === API Methods (для совместимости с HybridFreGSRenderer) ===
    
    void uploadTriangles(const std::vector<Triangle>& triangles);
    void setViewProjection(const glm::mat4& viewProj);
    void setCameraPosition(const glm::vec3& cameraPos);
    void setFrustumCullingEnabled(bool enabled);
    void setDebugMode(uint32_t mode);
    
    // === Getters ===
    
    bool isInitialized() const { return initialized_; }
    uint32_t getTriangleCount() const;
    uint32_t getVisibleTriangleCount() const;
    vk::Image getOutputImage() const;
    vk::ImageView getOutputImageView() const;
    
    const TriangleSplattingStatistics& getStatistics() const { return *statistics_; }
    const Config& getConfig() const { return config_; }

private:
    Config config_;
    bool initialized_ = false;
    
    // Vulkan context
    vk::Device device_;
    VmaAllocator allocator_;
    vk::Queue computeQueue_;
    vk::Queue graphicsQueue_;
    vk::CommandPool commandPool_;
    
    // Subsystems (Dependency Injection)
    std::unique_ptr<TriangleSplattingCore> core_;
    std::unique_ptr<TriangleBufferManager> bufferManager_;
    std::unique_ptr<FrustumCullingPass> cullingPass_;
    std::unique_ptr<DepthSortingPass> sortingPass_;
    std::unique_ptr<TriangleRasterizationPass> rasterPass_;
    std::unique_ptr<TriangleSplattingDebugger> debugger_;
    std::unique_ptr<TriangleSplattingStatistics> statistics_;
    
    // State
    glm::mat4 viewProj_ = glm::mat4(1.0f);
    glm::vec3 cameraPos_ = glm::vec3(0.0f);
    bool frustumCullingEnabled_ = true;
    uint32_t visibleTriangleCount_ = 0;
    
    // Helper для создания subsystems
    bool initializeSubsystems();
};

} // namespace rendering
} // namespace spectraforge
