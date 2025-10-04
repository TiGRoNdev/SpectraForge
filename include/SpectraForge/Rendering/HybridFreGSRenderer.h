/**
 * @file HybridFreGSRenderer.h
 * @brief Реализация IRenderer для Hybrid DWT + FreGS пайплайна
 */

#pragma once

#include <memory>
#include <string>
#include "SpectraForge/Rendering/Common/IRenderer.h"
#include "SpectraForge/rendering/WaveletPass.h"
#include "SpectraForge/rendering/FreGSPass.h"
#include "SpectraForge/rendering/TriangleSplattingPass.h"
#include "SpectraForge/core/VulkanContext.h"
#include <vk_mem_alloc.h>

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

  private:
    bool initialized_ = false;
    RenderingStats stats_{};
    RenderMode renderMode_ = RenderMode::TriangleSplatting;  // Default to Triangle Splatting

    // Vulkan context (DIP abstraction)
    std::unique_ptr<spectraforge::VulkanContext> vkContext_;
    
    // VMA allocator for Triangle Splatting (owned by renderer)
    VmaAllocator triangleSplattingVmaAllocator_ = nullptr;

    // Passes
    std::unique_ptr<spectraforge::rendering::WaveletPass> wavelet_;
    std::unique_ptr<spectraforge::rendering::FreGSPass> fregs_;
    std::unique_ptr<spectraforge::rendering::TriangleSplattingPass> triangleSplatting_;

    // Simple command buffer lifecycle
    vk::CommandBuffer lastSubmittedCmd_{};  // освобождаем на следующем кадре после fence
    uint32_t frameIndex_ = 0;

    // === Vulkan presentation (surface + swapchain) ===
    vk::SurfaceKHR surface_{};
    vk::SwapchainKHR swapchain_{};
    std::vector<vk::Image> swapchainImages_{};
    std::vector<vk::ImageView> swapchainImageViews_{};
    vk::Format swapchainFormat_ = vk::Format::eB8G8R8A8Unorm;
    vk::Extent2D swapchainExtent_{1920, 1080};

    // Sync primitives
    vk::Semaphore imageAvailableSemaphore_{};
    vk::Semaphore renderFinishedSemaphore_{};
    vk::Fence inFlightFence_{};

    // Frame capture for debugging
    bool frameCaptured_ = false;

    // Helpers
    bool createSurfaceFromCurrentGLFW();
    bool createSwapchainAndViews(uint32_t width, uint32_t height);
    void destroySwapchainAndViews();
    void saveFrameToPNG(vk::Image srcImage, uint32_t width, uint32_t height, const std::string& filename);
};

}  // namespace Rendering
}  // namespace SpectraForge


