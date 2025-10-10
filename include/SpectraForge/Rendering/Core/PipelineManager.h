/**
 * @file PipelineManager.h
 * @brief Render pass, framebuffers, command buffers management (P0.2 Refactoring)
 */

#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vector>

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
class IPipelineManager {
  public:
    virtual ~IPipelineManager() = default;

    virtual bool initialize(const std::vector<vk::ImageView>& swapchainImageViews,
                            vk::Format swapchainFormat, vk::Extent2D swapchainExtent,
                            uint32_t graphicsQueueFamily) = 0;
    virtual void shutdown() = 0;
    virtual bool createRenderPass(vk::Format swapchainFormat) = 0;
    virtual bool createFramebuffers(const std::vector<vk::ImageView>& swapchainImageViews,
                                    vk::Extent2D swapchainExtent) = 0;
    virtual bool createCommandPool(uint32_t graphicsQueueFamily) = 0;
    virtual bool createCommandBuffers() = 0;
    virtual vk::RenderPass getRenderPass() const = 0;
    virtual const std::vector<vk::Framebuffer>& getFramebuffers() const = 0;
    virtual vk::CommandPool getCommandPool() const = 0;
    virtual const std::vector<vk::CommandBuffer>& getCommandBuffers() const = 0;
    virtual vk::CommandBuffer getCurrentCommandBuffer() const = 0;
    virtual void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex,
                                    const std::function<void(vk::CommandBuffer)>& recordFunc) = 0;
    virtual void setCurrentFrame(size_t frame) = 0;
    virtual bool isInitialized() const = 0;
};

class IPipelineManagerFactory {
  public:
    virtual ~IPipelineManagerFactory() = default;
    virtual std::shared_ptr<IPipelineManager> create(vk::Device device) = 0;
};

class PipelineManager : public IPipelineManager {
public:
    explicit PipelineManager(vk::Device device);
    ~PipelineManager();
    
    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;
    
    bool initialize(const std::vector<vk::ImageView>& swapchainImageViews,
                    vk::Format swapchainFormat, vk::Extent2D swapchainExtent,
                    uint32_t graphicsQueueFamily) override;
    void shutdown() override;

    bool createRenderPass(vk::Format swapchainFormat) override;
    bool createFramebuffers(const std::vector<vk::ImageView>& swapchainImageViews,
                           vk::Extent2D swapchainExtent) override;
    bool createCommandPool(uint32_t graphicsQueueFamily) override;
    bool createCommandBuffers() override;

    vk::RenderPass getRenderPass() const override { return renderPass_; }
    const std::vector<vk::Framebuffer>& getFramebuffers() const override { return framebuffers_; }
    vk::CommandPool getCommandPool() const override { return commandPool_; }
    const std::vector<vk::CommandBuffer>& getCommandBuffers() const override { return commandBuffers_; }
    vk::CommandBuffer getCurrentCommandBuffer() const override { return commandBuffers_[currentFrame_]; }

    void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex,
                            const std::function<void(vk::CommandBuffer)>& recordFunc) override;

    void setCurrentFrame(size_t frame) override { currentFrame_ = frame; }

    bool isInitialized() const override { return initialized_; }
    
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

class PipelineManagerFactory : public IPipelineManagerFactory {
  public:
    std::shared_ptr<IPipelineManager> create(vk::Device device) override {
        return std::make_shared<PipelineManager>(device);
    }
};

}}}

