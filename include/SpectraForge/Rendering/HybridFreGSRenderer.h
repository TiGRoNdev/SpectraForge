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
#include "SpectraForge/core/VulkanContext.h"

namespace SpectraForge {
namespace Rendering {

/**
 * @brief Рендерер, объединяющий WaveletPass и FreGSPass
 */
class HybridFreGSRenderer final : public IRenderer {
  public:
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

  private:
    bool initialized_ = false;
    RenderingStats stats_{};

    // Vulkan context (DIP abstraction)
    std::unique_ptr<spectraforge::VulkanContext> vkContext_;

    // Passes
    std::unique_ptr<spectraforge::rendering::WaveletPass> wavelet_;
    std::unique_ptr<spectraforge::rendering::FreGSPass> fregs_;

    // Simple command buffer lifecycle
    vk::CommandBuffer cmd_{};
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

    // Helpers
    bool createSurfaceFromCurrentGLFW();
    bool createSwapchainAndViews(uint32_t width, uint32_t height);
    void destroySwapchainAndViews();
};

}  // namespace Rendering
}  // namespace SpectraForge


