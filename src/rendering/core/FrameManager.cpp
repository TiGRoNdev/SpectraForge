/**
 * @file FrameManager.cpp
 * @brief Implementation of FrameManager (P0.2)
 */

#include "SpectraForge/Rendering/Core/FrameManager.h"
#include <iostream>

namespace SpectraForge {
namespace Rendering {
namespace Core {

FrameManager::FrameManager(vk::Device device)
    : device_(device) {
}

FrameManager::~FrameManager() {
    shutdown();
}

bool FrameManager::initialize() {
    if (initialized_) {
        std::cout << "[FrameManager] Already initialized\n";
        return true;
    }
    
    if (!createSyncObjects()) {
        std::cerr << "[FrameManager] ❌ Failed to create sync objects\n";
        return false;
    }
    
    initialized_ = true;
    std::cout << "[FrameManager] ✅ Initialized\n";
    return true;
}

void FrameManager::shutdown() {
    if (!initialized_) {
        return;
    }
    
    if (device_) {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (imageAvailableSemaphores_[i]) {
                device_.destroySemaphore(imageAvailableSemaphores_[i]);
            }
            if (renderFinishedSemaphores_[i]) {
                device_.destroySemaphore(renderFinishedSemaphores_[i]);
            }
            if (inFlightFences_[i]) {
                device_.destroyFence(inFlightFences_[i]);
            }
        }
    }
    
    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();
    inFlightFences_.clear();
    
    initialized_ = false;
    deviceLost_ = false;
    
    std::cout << "[FrameManager] ✅ Shutdown complete\n";
}

bool FrameManager::beginFrame(vk::SwapchainKHR swapchain) {
    if (!initialized_) {
        std::cerr << "[FrameManager] ❌ Not initialized\n";
        return false;
    }
    
    if (deviceLost_) {
        return false;
    }
    
    // Wait for previous frame
    auto result = device_.waitForFences(inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        std::cerr << "[FrameManager] ❌ Failed to wait for fence\n";
        return false;
    }
    
    device_.resetFences(inFlightFences_[currentFrame_]);
    
    // Acquire next image
    try {
        auto acquireResult = device_.acquireNextImageKHR(
            swapchain, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], nullptr);
        currentImageIndex_ = acquireResult.value;
        return true;
    } catch (const vk::OutOfDateKHRError&) {
        std::cerr << "[FrameManager] ⚠️ Swapchain out of date\n";
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[FrameManager] ❌ Failed to acquire next image: " << e.what() << "\n";
        deviceLost_ = true;
        return false;
    }
}

bool FrameManager::endFrame(vk::SwapchainKHR swapchain,
                           vk::Queue presentQueue,
                           vk::CommandBuffer commandBuffer) {
    if (!initialized_ || deviceLost_) {
        return false;
    }
    
    // Submit command buffer
    vk::SubmitInfo submitInfo;
    
    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    try {
        // Submit to graphics queue with fence
        vk::Queue graphicsQueue = presentQueue;  // Assume same queue for now
        graphicsQueue.submit(submitInfo, inFlightFences_[currentFrame_]);
    } catch (const std::exception& e) {
        std::cerr << "[FrameManager] ❌ Failed to submit command buffer: " << e.what() << "\n";
        return false;
    }
    
    // Present
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    vk::SwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &currentImageIndex_;
    
    try {
        auto result = presentQueue.presentKHR(presentInfo);
        if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) {
            std::cerr << "[FrameManager] ⚠️ Swapchain out of date or suboptimal\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[FrameManager] ❌ Failed to present: " << e.what() << "\n";
        return false;
    }
    
    // Advance frame
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

bool FrameManager::createSyncObjects() {
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);
    
    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    
    try {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            imageAvailableSemaphores_[i] = device_.createSemaphore(semaphoreInfo);
            renderFinishedSemaphores_[i] = device_.createSemaphore(semaphoreInfo);
            inFlightFences_[i] = device_.createFence(fenceInfo);
        }
        std::cout << "[FrameManager] ✅ Synchronization objects created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[FrameManager] ❌ Failed to create sync objects: " << e.what() << "\n";
        return false;
    }
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

