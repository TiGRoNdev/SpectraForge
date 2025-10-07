/**
 * @file HybridFreGSRenderer.h
 * @brief Реализация IRenderer для Hybrid DWT + FreGS пайплайна
 */

#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"
#include "SpectraForge/Rendering/RenderPass/FreGSPass.h"
#include "SpectraForge/Rendering/Mesh3D.h"
#include "SpectraForge/Rendering/Camera3D.h"
// #include "SpectraForge/rendering/WaveletPass.h"
// #include "SpectraForge/core/VulkanContext.h"

namespace SpectraForge {
namespace Rendering {

/**
 * @brief Рендерер, объединяющий WaveletPass, FreGSPass и TriangleSplattingPass
 * 
 * Dual-path rendering:
 * - Gaussian Splatting (FreGS) → Point clouds (.ply)
 * - Triangle Splatting → Triangle meshes (.obj)
 */
class HybridFreGSRenderer final : public IRenderer {
  public:
    /**
     * @brief Rendering mode selector
     */
    enum class RenderMode {
        GaussianSplatting,  ///< For point clouds (FreGS)
        TriangleSplatting   ///< For triangle meshes (Triangle Splatting)
    };

    HybridFreGSRenderer();
    ~HybridFreGSRenderer() override;

    // IRenderer
    bool initialize() override;
    /**
     * @brief Привязать окно рендереру (создание поверхности/свапчейна)
     */
    bool attachWindow(void* x11Display, void* x11Window, uint32_t width, uint32_t height);
    void renderFrame(const FrameData& frameData) override;
    void shutdown() override;
    RendererType getType() const override { return RendererType::Vulkan; }
    bool supportsFeature(RenderingFeature feature) const override;
    std::string getName() const override { return "HybridFreGSRenderer"; }
    std::string getApiVersion() const override { return "Vulkan 1.3 (compute)"; }
    bool isReady() const override { return initialized_; }
    bool isInitialized() const override { return initialized_; }
    void beginFrame() override;
    void endFrame() override;
    RenderingStats getStats() const override { return stats_; }

    /**
     * @brief Загрузка гауссианов для FreGS пасса (point clouds)
     */
    void uploadGaussians(const std::vector<spectraforge::rendering::GaussianSplat>& gaussians);

    /**
     * @brief Загрузка треугольников для Triangle Splatting (meshes)
     */
    void uploadTriangles(const std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle>& triangles);

    /**
     * @brief Установить режим рендеринга
     */
    void setRenderMode(RenderMode mode) { renderMode_ = mode; }

    /**
     * @brief Признак потери устройства (VK_ERROR_DEVICE_LOST)
     * @return true если устройство потеряно и рендер следует остановить
     */
    bool isDeviceLost() const { return device_lost_; }

    /**
     * @brief Загрузить меш для рендеринга через Triangle Splatting
     */
    void uploadMesh(const std::shared_ptr<Mesh3D>& mesh, const Math::Matrix4& transform = Math::Matrix4::identity());

    /**
     * @brief Установить камеру для рендеринга
     */
    void setCamera(Camera3D* camera) { camera_ = camera; }

    // НОВЫЕ DEBUG API МЕТОДЫ (реализуют расширенный IRenderer интерфейс):
    
    /**
     * @brief Установить режим отладки
     * @param mode 0=normal, 1=SDF, 2=barycentric, 3=depth, 4=wireframe
     */
     void setDebugMode(int mode) override;
    
     /**
      * @brief Получить текущий режим отладки
      */
     int getDebugMode() const override;
     
     /**
      * @brief Включить/выключить wireframe режим
      */
     void enableWireframe(bool enable) override;
     
     /**
      * @brief Включить/выключить backface culling
      */
     void enableBackfaceCulling(bool enable) override;
     
     /**
      * @brief Включить/выключить depth test
      */
     void enableDepthTest(bool enable) override;
     
     /**
      * @brief Установить цвет фона
      */
     void setBackgroundColor(float r, float g, float b, float a = 1.0f) override;
     
     /**
      * @brief Получить цвет фона
      */
     glm::vec4 getBackgroundColor() const override;
     
     /**
      * @brief Установить viewport
      */
     void setViewport(int x, int y, int width, int height) override;
     
     /**
      * @brief Включить/выключить alpha blending
      */
     void enableAlphaBlending(bool enable) override;
     
     /**
      * @brief Установить triangle budget для performance tuning
      */
     void setTriangleBudget(uint32_t maxTriangles) override;
     
     /**
      * @brief Включить/выключить early termination в alpha blending
      */
     void enableEarlyTermination(bool enable) override;
     
     /**
      * @brief Получить подробную статистику производительности
      */
     DetailedRenderingStats getDetailedStats() const override;
     
     /**
      * @brief Сохранить скриншот в файл
      */
     bool saveScreenshot(const std::string& filename) const override;
     
     /**
      * @brief Получить данные framebuffer для анализа
      */
     std::vector<uint8_t> getFramebufferData() const override;
     
     /**
      * @brief Установить debug callback для логирования
      */
     void setDebugCallback(std::function<void(const std::string&)> callback) override;
     
     /**
      * @brief Принудительно обновить все uniform буферы
      */
     void flushUniforms() override;
     
     /**
      * @brief Получить информацию о GPU
      */
     GPUInfo getGPUInfo() const override;

  private:
    bool initialized_ = false;
    RenderingStats stats_{};
    RenderMode renderMode_ = RenderMode::TriangleSplatting;  // Default to Triangle Splatting

    // Vulkan core objects
    vk::Instance instance_{};
    vk::PhysicalDevice physicalDevice_{};
    vk::Device device_{};
    vk::Queue graphicsQueue_{};
    vk::Queue presentQueue_{};
    vk::CommandPool commandPool_{};
    
    uint32_t graphicsQueueFamily_ = 0;
    uint32_t presentQueueFamily_ = 0;
    uint32_t frameIndex_ = 0;

    // === Vulkan presentation (surface + swapchain) ===
    vk::SurfaceKHR surface_{};
    vk::SwapchainKHR swapchain_{};
    std::vector<vk::Image> swapchainImages_{};
    std::vector<vk::ImageView> swapchainImageViews_{};
    std::vector<vk::Framebuffer> swapchainFramebuffers_{};
    vk::Format swapchainFormat_ = vk::Format::eB8G8R8A8Unorm;
    vk::Extent2D swapchainExtent_{1920, 1080};
    vk::RenderPass renderPass_{};

    // Debug state
    int currentDebugMode_ = 0;
    glm::vec4 backgroundColor_{0.1f, 0.2f, 0.3f, 1.0f};
    bool wireframeEnabled_ = false;
    bool depthTestEnabled_ = true;
    bool backfaceCullingEnabled_ = true;
    bool alphaBlendingEnabled_ = true;
    uint32_t triangleBudget_ = 100000;
    bool earlyTerminationEnabled_ = true;
    vk::Viewport currentViewport_;
    
    // Command buffers
    std::vector<vk::CommandBuffer> commandBuffers_{};

    // Sync primitives (per frame in flight)
    std::vector<vk::Semaphore> imageAvailableSemaphores_{};
    std::vector<vk::Semaphore> renderFinishedSemaphores_{};
    std::vector<vk::Fence> inFlightFences_{};
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    size_t currentFrame_ = 0;
    
    uint32_t currentImageIndex_ = 0;

    // Device lost handling
    bool device_lost_ = false;

    // === VMA Allocator ===
    VmaAllocator allocator_ = VK_NULL_HANDLE;

    // === Rendering Passes ===
    std::unique_ptr<spectraforge::rendering::TriangleSplattingPass> triangleSplattingPass_;
    std::unique_ptr<spectraforge::rendering::FreGSPass> fregsPass_;

    // === Camera ===
    Camera3D* camera_ = nullptr;

    // Helpers
    std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> convertMeshToTriangles(
        const Mesh3D& mesh, const Math::Matrix4& transform);
    bool createInstance();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createAllocator();
    bool createSurfaceX11(void* x11Display, void* x11Window);
    bool createSwapchainAndViews(uint32_t width, uint32_t height);
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();
    bool initializeTriangleSplatting();
    void destroySwapchainAndViews();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    // Debug callback
    std::function<void(const std::string&)> debugCallback_;
    
    // Debug logging helper
    void debugLog(const std::string& message) const {
        if (debugCallback_) {
            debugCallback_(message);
        }
        std::cout << "[HybridFreGSRenderer DEBUG] " << message << std::endl;
    }

    // Corrected View-Projection for Vulkan (proj * view) with Matrix4 -> glm::mat4 conversion
    glm::mat4 getCorrectedViewProjMatrix() const;
};

}  // namespace Rendering
}  // namespace SpectraForge


