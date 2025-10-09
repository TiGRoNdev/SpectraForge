/**
 * @file PipelineManager.h
 * @brief Render pass, framebuffers, command buffers management (P0.2 Refactoring)
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <functional>

namespace SpectraForge {
namespace Rendering {
namespace Core {

/**
 * @brief Управление render pass, framebuffers, command buffers
 * 
 * SOLID:
 * - SRP ✅: Только pipeline/command buffer management
 * - DIP ✅: Зависит от abstractions
 */
class PipelineManager {
public:
    explicit PipelineManager(vk::Device device);
    ~PipelineManager();
    
    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    
    bool initialize(const std::vector<vk::ImageView>& swapchainImageViews,
                    vk::Format swapchainFormat, vk::Extent2D swapchainExtent,
                    uint32_t graphicsQueueFamily);
    void shutdown();
    
    bool createRenderPass(vk::Format swapchainFormat);
    bool createFramebuffers(const std::vector<vk::ImageView>& swapchainImageViews,
                           vk::Extent2D swapchainExtent);
    bool createCommandPool(uint32_t graphicsQueueFamily);
    bool createCommandBuffers();
    
    vk::RenderPass getRenderPass() const { return renderPass_; }
    const std::vector<vk::Framebuffer>& getFramebuffers() const { return framebuffers_; }
    vk::CommandPool getCommandPool() const { return commandPool_; }
    const std::vector<vk::CommandBuffer>& getCommandBuffers() const { return commandBuffers_; }
    vk::CommandBuffer getCurrentCommandBuffer() const { return commandBuffers_[currentFrame_]; }
    
    void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex,
                            const std::function<void(vk::CommandBuffer)>& recordFunc);
    
    void setCurrentFrame(size_t frame) { currentFrame_ = frame; }
    
    bool isInitialized() const { return initialized_; }
    
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

private:
    vk::Device device_;
    vk::RenderPass renderPass_{};
    std::vector<vk::Framebuffer> framebuffers_{};
    vk::CommandPool commandPool_{};
    std::vector<vk::CommandBuffer> commandBuffers_{};
    size_t currentFrame_ = 0;
    bool initialized_ = false;
};

}}}

