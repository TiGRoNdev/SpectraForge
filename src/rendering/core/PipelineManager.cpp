/**
 * @file PipelineManager.cpp
 * @brief Implementation of PipelineManager (P0.2)
 */

#include "SpectraForge/Rendering/Core/PipelineManager.h"
#include <iostream>

namespace SpectraForge {
namespace Rendering {
namespace Core {

PipelineManager::PipelineManager(vk::Device device)
    : device_(device) {
}

PipelineManager::~PipelineManager() {
    shutdown();
}

bool PipelineManager::initialize(const std::vector<vk::ImageView>& swapchainImageViews,
                                 vk::Format swapchainFormat, vk::Extent2D swapchainExtent,
                                 uint32_t graphicsQueueFamily) {
    if (initialized_) {
        std::cout << "[PipelineManager] Already initialized\n";
        return true;
    }
    
    if (!createRenderPass(swapchainFormat)) {
        return false;
    }
    
    if (!createFramebuffers(swapchainImageViews, swapchainExtent)) {
        return false;
    }
    
    if (!createCommandPool(graphicsQueueFamily)) {
        return false;
    }
    
    if (!createCommandBuffers()) {
        return false;
    }
    
    initialized_ = true;
    std::cout << "[PipelineManager] ✅ Initialized\n";
    return true;
}

void PipelineManager::shutdown() {
    if (!initialized_) {
        return;
    }
    
    if (device_) {
        // Free command buffers
        if (commandPool_ && !commandBuffers_.empty()) {
            device_.freeCommandBuffers(commandPool_, commandBuffers_);
            commandBuffers_.clear();
        }
        
        // Destroy command pool
        if (commandPool_) {
            device_.destroyCommandPool(commandPool_);
            commandPool_ = vk::CommandPool{};
        }
        
        // Destroy framebuffers
        for (auto framebuffer : framebuffers_) {
            device_.destroyFramebuffer(framebuffer);
        }
        framebuffers_.clear();
        
        // Destroy render pass
        if (renderPass_) {
            device_.destroyRenderPass(renderPass_);
            renderPass_ = vk::RenderPass{};
        }
    }
    
    initialized_ = false;
    std::cout << "[PipelineManager] ✅ Shutdown complete\n";
}

bool PipelineManager::createRenderPass(vk::Format swapchainFormat) {
    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    
    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
    
    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlags();
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    try {
        renderPass_ = device_.createRenderPass(renderPassInfo);
        std::cout << "[PipelineManager] ✅ Render pass created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PipelineManager] ❌ Failed to create render pass: " << e.what() << "\n";
        return false;
    }
}

bool PipelineManager::createFramebuffers(const std::vector<vk::ImageView>& swapchainImageViews,
                                        vk::Extent2D swapchainExtent) {
    framebuffers_.resize(swapchainImageViews.size());
    
    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        vk::ImageView attachments[] = {swapchainImageViews[i]};
        
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;
        
        try {
            framebuffers_[i] = device_.createFramebuffer(framebufferInfo);
        } catch (const std::exception& e) {
            std::cerr << "[PipelineManager] ❌ Failed to create framebuffer: " << e.what() << "\n";
            return false;
        }
    }
    
    std::cout << "[PipelineManager] ✅ Framebuffers created (" << framebuffers_.size() << ")\n";
    return true;
}

bool PipelineManager::createCommandPool(uint32_t graphicsQueueFamily) {
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = graphicsQueueFamily;
    
    try {
        commandPool_ = device_.createCommandPool(poolInfo);
        std::cout << "[PipelineManager] ✅ Command pool created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PipelineManager] ❌ Failed to create command pool: " << e.what() << "\n";
        return false;
    }
}

bool PipelineManager::createCommandBuffers() {
    commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());
    
    try {
        commandBuffers_ = device_.allocateCommandBuffers(allocInfo);
        std::cout << "[PipelineManager] ✅ Command buffers created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PipelineManager] ❌ Failed to create command buffers: " << e.what() << "\n";
        return false;
    }
}

void PipelineManager::recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex,
                                          const std::function<void(vk::CommandBuffer)>& recordFunc) {
    if (frameIndex >= commandBuffers_.size()) {
        std::cerr << "[PipelineManager] ❌ Invalid frame index\n";
        return;
    }
    
    vk::CommandBuffer commandBuffer = commandBuffers_[frameIndex];
    
    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);
    
    // Execute user-provided recording function
    if (recordFunc) {
        recordFunc(commandBuffer);
    }
    
    commandBuffer.end();
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

