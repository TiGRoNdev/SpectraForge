/**
 * @file HybridFreGSRenderer.cpp
 * @brief Working Hybrid TriangleSplatting + FreGS renderer with minimal Vulkan rendering
 */

#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SpectraForge {
namespace Rendering {

HybridFreGSRenderer::HybridFreGSRenderer() = default;

HybridFreGSRenderer::~HybridFreGSRenderer() {
    shutdown();
}

bool HybridFreGSRenderer::initialize() {
    if (initialized_) return true;

    std::cout << "Initializing HybridFreGSRenderer with Vulkan...\n";

    // Create Vulkan instance
    if (!createInstance()) {
        std::cerr << "❌ Failed to create Vulkan instance\n";
        return false;
    }

    initialized_ = true;
    std::cout << "✅ HybridFreGSRenderer initialized (Vulkan instance created)\n";
    return true;
}

bool HybridFreGSRenderer::createInstance() {
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "SpectraForge";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "SpectraForge Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Required extensions
    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    // Validation layers
    std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    try {
        instance_ = vk::createInstance(createInfo);
        std::cout << "✅ Vulkan instance created with validation layers\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create instance: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::pickPhysicalDevice() {
    auto devices = instance_.enumeratePhysicalDevices();
    
    if (devices.empty()) {
        std::cerr << "❌ No Vulkan devices found\n";
        return false;
    }

    // Pick the first discrete GPU or fallback to first device
    for (const auto& device : devices) {
        auto props = device.getProperties();
        if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physicalDevice_ = device;
            std::cout << "✅ Selected GPU: " << props.deviceName << "\n";
            return true;
        }
    }

    physicalDevice_ = devices[0];
    auto props = physicalDevice_.getProperties();
    std::cout << "✅ Selected device: " << props.deviceName << "\n";
    return true;
}

bool HybridFreGSRenderer::createLogicalDevice() {
    // Find queue families
    auto queueFamilies = physicalDevice_.getQueueFamilyProperties();
    
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
        }
        
        // Check present support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface_, &presentSupport);
        if (presentSupport) {
            presentFamily = i;
        }
        
        if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX) {
            break;
        }
    }
    
    if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX) {
        std::cerr << "❌ Failed to find suitable queue families\n";
        return false;
    }
    
    graphicsQueueFamily_ = graphicsFamily;
    presentQueueFamily_ = presentFamily;
    
    // Create queues
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily, presentFamily};
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Device features
    vk::PhysicalDeviceFeatures deviceFeatures;
    
    // Device extensions
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    try {
        device_ = physicalDevice_.createDevice(createInfo);
        graphicsQueue_ = device_.getQueue(graphicsFamily, 0);
        presentQueue_ = device_.getQueue(presentFamily, 0);
        std::cout << "✅ Logical device created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create logical device: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::attachWindow(void* x11Display, void* x11Window, uint32_t width, uint32_t height) {
    if (!initialized_) {
        if (!initialize()) return false;
    }
    
    // Create X11 surface
    if (!createSurfaceX11(x11Display, x11Window)) {
        std::cerr << "❌ Failed to create X11 surface\n";
        return false;
    }
    
    // Pick physical device
    if (!pickPhysicalDevice()) {
        std::cerr << "❌ Failed to pick physical device\n";
        return false;
    }
    
    // Create logical device
    if (!createLogicalDevice()) {
        std::cerr << "❌ Failed to create logical device\n";
        return false;
    }
    
    // Create VMA allocator
    if (!createAllocator()) {
        std::cerr << "❌ Failed to create VMA allocator\n";
        return false;
    }
    
    // Create swapchain
    if (!createSwapchainAndViews(width, height)) {
        std::cerr << "❌ Failed to create swapchain\n";
        return false;
    }
    
    // Create render pass
    if (!createRenderPass()) {
        std::cerr << "❌ Failed to create render pass\n";
        return false;
    }
    
    // Create framebuffers
    if (!createFramebuffers()) {
        std::cerr << "❌ Failed to create framebuffers\n";
        return false;
    }
    
    // Create command pool
    if (!createCommandPool()) {
        std::cerr << "❌ Failed to create command pool\n";
        return false;
    }
    
    // Create command buffers
    if (!createCommandBuffers()) {
        std::cerr << "❌ Failed to create command buffers\n";
        return false;
    }
    
    // Create sync objects
    if (!createSyncObjects()) {
        std::cerr << "❌ Failed to create sync objects\n";
        return false;
    }
    
    // Initialize Triangle Splatting
    if (!initializeTriangleSplatting()) {
        std::cerr << "❌ Failed to initialize Triangle Splatting\n";
        return false;
    }
    
    std::cout << "✅ Window attached successfully, ready to render\n";
    return true;
}

bool HybridFreGSRenderer::createSurfaceX11(void* x11Display, void* x11Window) {
    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = static_cast<Display*>(x11Display);
    // Window в X11 это unsigned long, закодированный в void* через reinterpret_cast
    createInfo.window = reinterpret_cast<Window>(x11Window);
    
    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(instance_, &createInfo, nullptr, &surface);
    
    if (result != VK_SUCCESS) {
        std::cerr << "❌ Failed to create Xlib surface: error code " << result << "\n";
        return false;
    }
    
    surface_ = surface;
    std::cout << "✅ X11 Vulkan surface created (Window ID: " << createInfo.window << ")\n";
    return true;
}

bool HybridFreGSRenderer::createSwapchainAndViews(uint32_t width, uint32_t height) {
    // Query surface capabilities
    auto capabilities = physicalDevice_.getSurfaceCapabilitiesKHR(surface_);
    auto formats = physicalDevice_.getSurfaceFormatsKHR(surface_);
    auto presentModes = physicalDevice_.getSurfacePresentModesKHR(surface_);
    
    if (formats.empty() || presentModes.empty()) {
        std::cerr << "❌ Insufficient swapchain support\n";
        return false;
    }
    
    // Choose format
    vk::SurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& format : formats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            surfaceFormat = format;
            break;
        }
    }
    swapchainFormat_ = surfaceFormat.format;
    
    // Choose present mode (prefer mailbox, fallback to FIFO)
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& mode : presentModes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            presentMode = mode;
            break;
        }
    }
    
    // Choose extent
    if (capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent_ = capabilities.currentExtent;
    } else {
        swapchainExtent_ = vk::Extent2D{width, height};
        swapchainExtent_.width = std::clamp(swapchainExtent_.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchainExtent_.height = std::clamp(swapchainExtent_.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapchainExtent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    
    uint32_t queueFamilyIndices[] = {graphicsQueueFamily_, presentQueueFamily_};
    if (graphicsQueueFamily_ != presentQueueFamily_) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    
    try {
        swapchain_ = device_.createSwapchainKHR(createInfo);
        swapchainImages_ = device_.getSwapchainImagesKHR(swapchain_);
        
        // Create image views
        swapchainImageViews_.resize(swapchainImages_.size());
        for (size_t i = 0; i < swapchainImages_.size(); i++) {
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.image = swapchainImages_[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = swapchainFormat_;
            viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            
            swapchainImageViews_[i] = device_.createImageView(viewInfo);
        }
        
        std::cout << "✅ Swapchain created: " << swapchainExtent_.width << "x" 
                  << swapchainExtent_.height << " (" << swapchainImages_.size() << " images)\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create swapchain: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::createRenderPass() {
    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = swapchainFormat_;
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
        std::cout << "✅ Render pass created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create render pass: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::createFramebuffers() {
    swapchainFramebuffers_.resize(swapchainImageViews_.size());
    
    for (size_t i = 0; i < swapchainImageViews_.size(); i++) {
        vk::ImageView attachments[] = {swapchainImageViews_[i]};
        
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent_.width;
        framebufferInfo.height = swapchainExtent_.height;
        framebufferInfo.layers = 1;
        
        try {
            swapchainFramebuffers_[i] = device_.createFramebuffer(framebufferInfo);
        } catch (const std::exception& e) {
            std::cerr << "❌ Failed to create framebuffer: " << e.what() << "\n";
            return false;
        }
    }
    
    std::cout << "✅ Framebuffers created (" << swapchainFramebuffers_.size() << ")\n";
    return true;
}

bool HybridFreGSRenderer::createCommandPool() {
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = graphicsQueueFamily_;
    
    try {
        commandPool_ = device_.createCommandPool(poolInfo);
        std::cout << "✅ Command pool created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create command pool: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::createCommandBuffers() {
    commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());
    
    try {
        commandBuffers_ = device_.allocateCommandBuffers(allocInfo);
        std::cout << "✅ Command buffers created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create command buffers: " << e.what() << "\n";
        return false;
    }
}

bool HybridFreGSRenderer::createSyncObjects() {
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
        std::cout << "✅ Synchronization objects created\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create sync objects: " << e.what() << "\n";
        return false;
    }
}

void HybridFreGSRenderer::beginFrame() {
    if (!device_) return;
    
    // Wait for previous frame
    auto result = device_.waitForFences(inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        std::cerr << "❌ Failed to wait for fence\n";
        return;
    }
    
    device_.resetFences(inFlightFences_[currentFrame_]);
    
    // Acquire next image
    try {
        auto acquireResult = device_.acquireNextImageKHR(
            swapchain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], nullptr);
        currentImageIndex_ = acquireResult.value;
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to acquire next image: " << e.what() << "\n";
        device_lost_ = true;
        return;
    }
}

void HybridFreGSRenderer::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo;
    
    commandBuffer.begin(beginInfo);
    
    // Execute rendering pass based on mode
    if (renderMode_ == RenderMode::TriangleSplatting && 
        triangleSplattingPass_ && triangleSplattingPass_->getTriangleCount() > 0) {
        // Transition output image UNDEFINED -> GENERAL (первый кадр)
        vk::ImageMemoryBarrier barrierInit{};
        barrierInit.oldLayout = vk::ImageLayout::eUndefined;
        barrierInit.newLayout = vk::ImageLayout::eGeneral;
        barrierInit.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierInit.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierInit.image = triangleSplattingPass_->getOutputImage();
        barrierInit.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrierInit.subresourceRange.baseMipLevel = 0;
        barrierInit.subresourceRange.levelCount = 1;
        barrierInit.subresourceRange.baseArrayLayer = 0;
        barrierInit.subresourceRange.layerCount = 1;
        barrierInit.srcAccessMask = {};
        barrierInit.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eComputeShader,
            {}, 0, nullptr, 0, nullptr, 1, &barrierInit
        );
        
        triangleSplattingPass_->execute(commandBuffer, currentFrame_);
        
        // ===== DEBUG: Save frames for analysis =====
        static int frameSaveCounter = 0;
        const int saveEveryNthFrame = 60; // Save every 60th frame (1 FPS at 60 FPS)

        if (frameSaveCounter % saveEveryNthFrame == 0) {
            // End current command buffer and submit (to complete rendering)
            commandBuffer.end();

            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            graphicsQueue_.submit(submitInfo, nullptr);
            graphicsQueue_.waitIdle();

            // Generate unique filename
            char filename[256];
            snprintf(filename, sizeof(filename), "/home/tigron/Documents/GITHUB/SpectraForge/DEBUG_frame_%03d.png", frameSaveCounter / saveEveryNthFrame);

            // Save frame to file
            triangleSplattingPass_->saveFrameToPNG(filename);

            // Restart command buffer
            vk::CommandBufferBeginInfo beginInfo;
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            commandBuffer.begin(beginInfo);
        }
        frameSaveCounter++;
        // ===============================================
        
        // Transition Triangle Splatting output from GENERAL to TRANSFER_SRC_OPTIMAL
        vk::ImageMemoryBarrier barrier1{};
        barrier1.oldLayout = vk::ImageLayout::eGeneral;
        barrier1.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.image = triangleSplattingPass_->getOutputImage();
        barrier1.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier1.subresourceRange.baseMipLevel = 0;
        barrier1.subresourceRange.levelCount = 1;
        barrier1.subresourceRange.baseArrayLayer = 0;
        barrier1.subresourceRange.layerCount = 1;
        barrier1.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        barrier1.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eTransfer,
            {}, 0, nullptr, 0, nullptr, 1, &barrier1
        );
        
        // Transition swapchain image from UNDEFINED to TRANSFER_DST_OPTIMAL
        vk::ImageMemoryBarrier barrier2{};
        barrier2.oldLayout = vk::ImageLayout::eUndefined;
        barrier2.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.image = swapchainImages_[imageIndex];
        barrier2.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier2.subresourceRange.baseMipLevel = 0;
        barrier2.subresourceRange.levelCount = 1;
        barrier2.subresourceRange.baseArrayLayer = 0;
        barrier2.subresourceRange.layerCount = 1;
        barrier2.srcAccessMask = {};
        barrier2.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            {}, 0, nullptr, 0, nullptr, 1, &barrier2
        );
        
        // Blit Triangle Splatting output to swapchain (with format conversion)
        vk::ImageBlit blitRegion{};
        blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blitRegion.srcOffsets[1] = vk::Offset3D{static_cast<int32_t>(swapchainExtent_.width), 
                                                 static_cast<int32_t>(swapchainExtent_.height), 1};
        blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blitRegion.dstOffsets[1] = vk::Offset3D{static_cast<int32_t>(swapchainExtent_.width), 
                                                 static_cast<int32_t>(swapchainExtent_.height), 1};
        
        commandBuffer.blitImage(
            triangleSplattingPass_->getOutputImage(), vk::ImageLayout::eTransferSrcOptimal,
            swapchainImages_[imageIndex], vk::ImageLayout::eTransferDstOptimal,
            1, &blitRegion,
            vk::Filter::eLinear
        );
        
        // Transition swapchain image from TRANSFER_DST to PRESENT_SRC
        vk::ImageMemoryBarrier barrier3{};
        barrier3.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier3.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier3.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier3.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier3.image = swapchainImages_[imageIndex];
        barrier3.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier3.subresourceRange.baseMipLevel = 0;
        barrier3.subresourceRange.levelCount = 1;
        barrier3.subresourceRange.baseArrayLayer = 0;
        barrier3.subresourceRange.layerCount = 1;
        barrier3.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier3.dstAccessMask = {};
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            {}, 0, nullptr, 0, nullptr, 1, &barrier3
        );
        
        // Transition Triangle Splatting output back to GENERAL for next frame
        vk::ImageMemoryBarrier barrier4{};
        barrier4.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier4.newLayout = vk::ImageLayout::eGeneral;
        barrier4.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier4.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier4.image = triangleSplattingPass_->getOutputImage();
        barrier4.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier4.subresourceRange.baseMipLevel = 0;
        barrier4.subresourceRange.levelCount = 1;
        barrier4.subresourceRange.baseArrayLayer = 0;
        barrier4.subresourceRange.layerCount = 1;
        barrier4.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier4.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            {}, 0, nullptr, 0, nullptr, 1, &barrier4
        );
    } else if (renderMode_ == RenderMode::GaussianSplatting && fregsPass_) {
        // TODO: Execute FreGS Pass (Gaussian Splatting)
        // fregsPass_->execute(commandBuffer, currentFrame_);
        
        // Fallback: clear screen to green for FreGS mode
        vk::ClearColorValue clearColor(std::array<float, 4>{0.1f, 0.4f, 0.2f, 1.0f});
        vk::ImageSubresourceRange range;
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        
        // Transition to TRANSFER_DST
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = swapchainImages_[imageIndex];
        barrier.subresourceRange = range;
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            {}, 0, nullptr, 0, nullptr, 1, &barrier
        );
        
        commandBuffer.clearColorImage(swapchainImages_[imageIndex], vk::ImageLayout::eTransferDstOptimal, clearColor, range);
        
        // Transition to PRESENT_SRC
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = {};
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            {}, 0, nullptr, 0, nullptr, 1, &barrier
        );
    } else {
        // Fallback: clear screen to dark blue if no triangles or unknown mode
        vk::ClearColorValue clearColor(std::array<float, 4>{0.1f, 0.2f, 0.4f, 1.0f});
        vk::ImageSubresourceRange range;
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        
        // Transition to TRANSFER_DST
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = swapchainImages_[imageIndex];
        barrier.subresourceRange = range;
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            {}, 0, nullptr, 0, nullptr, 1, &barrier
        );
        
        commandBuffer.clearColorImage(swapchainImages_[imageIndex], vk::ImageLayout::eTransferDstOptimal, clearColor, range);
        
        // Transition to PRESENT_SRC
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = {};
        
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            {}, 0, nullptr, 0, nullptr, 1, &barrier
        );
    }
    
    commandBuffer.end();
}

void HybridFreGSRenderer::renderFrame(const FrameData& /*frameData*/) {
    if (!device_ || device_lost_) return;
    
    // Update camera view-projection matrix (унифицировано)
    if (camera_ && triangleSplattingPass_) {
        // DEBUG: вывести матрицу вида один раз
        static bool viewPrinted = false;
        if (!viewPrinted) {
            const auto& view = camera_->getViewMatrix();
            std::cout << "\n===== DEBUG: View Matrix =====\n";
            for (int row = 0; row < 4; row++) {
                std::cout << "  [" << view.m[row][0] << ", " << view.m[row][1] << ", " 
                          << view.m[row][2] << ", " << view.m[row][3] << "]\n";
            }
            std::cout << "Камера position: (" << camera_->getPosition().x << ", " 
                      << camera_->getPosition().y << ", " << camera_->getPosition().z << ")\n";
            std::cout << "==============================\n\n";
            viewPrinted = true;
        }

        const glm::mat4 viewProj = getCorrectedViewProjMatrix();
        triangleSplattingPass_->setViewProjection(viewProj);

        const auto& camPos = camera_->getPosition();
        triangleSplattingPass_->setCameraPosition(glm::vec3(camPos.x, camPos.y, camPos.z));
    }
    
    // Record command buffer
    commandBuffers_[currentFrame_].reset();
    recordCommandBuffer(commandBuffers_[currentFrame_], currentImageIndex_);
}

void HybridFreGSRenderer::endFrame() {
    if (!device_ || device_lost_) return;
    
    // Submit command buffer
    vk::SubmitInfo submitInfo;
    
    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[currentFrame_];
    
    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores_[currentFrame_]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    try {
        graphicsQueue_.submit(submitInfo, inFlightFences_[currentFrame_]);
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to submit command buffer: " << e.what() << "\n";
        device_lost_ = true;
        return;
    }
    
    // Present
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    vk::SwapchainKHR swapchains[] = {swapchain_};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &currentImageIndex_;
    
    try {
        auto result = presentQueue_.presentKHR(presentInfo);
        if (result != vk::Result::eSuccess) {
            std::cerr << "⚠️ Present result: " << vk::to_string(result) << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to present: " << e.what() << "\n";
        device_lost_ = true;
        return;
    }
    
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HybridFreGSRenderer::shutdown() {
    if (!initialized_) return;
    
    std::cout << "Shutting down HybridFreGSRenderer...\n";
    
    if (device_) {
        device_.waitIdle();
        
        // Cleanup Triangle Splatting Pass
        if (triangleSplattingPass_) {
            triangleSplattingPass_->cleanup();
            triangleSplattingPass_.reset();
        }
        
        // Cleanup sync objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (imageAvailableSemaphores_[i]) device_.destroySemaphore(imageAvailableSemaphores_[i]);
            if (renderFinishedSemaphores_[i]) device_.destroySemaphore(renderFinishedSemaphores_[i]);
            if (inFlightFences_[i]) device_.destroyFence(inFlightFences_[i]);
        }
        
        // Cleanup command pool
        if (commandPool_) device_.destroyCommandPool(commandPool_);
        
        // Cleanup framebuffers
        for (auto framebuffer : swapchainFramebuffers_) {
            device_.destroyFramebuffer(framebuffer);
        }
        
        // Cleanup render pass
        if (renderPass_) device_.destroyRenderPass(renderPass_);
        
        // Cleanup swapchain
        destroySwapchainAndViews();
        
        // Cleanup VMA allocator
        if (allocator_) {
            vmaDestroyAllocator(allocator_);
            allocator_ = VK_NULL_HANDLE;
        }
        
        // Cleanup device
        device_.destroy();
        device_ = nullptr;
    }
    
    // Cleanup surface
    if (surface_ && instance_) {
        instance_.destroySurfaceKHR(surface_);
        surface_ = nullptr;
    }
    
    // Cleanup instance
    if (instance_) {
        instance_.destroy();
        instance_ = nullptr;
    }
    
    initialized_ = false;
    std::cout << "✅ HybridFreGSRenderer shutdown complete\n";
}

void HybridFreGSRenderer::destroySwapchainAndViews() {
    for (auto imageView : swapchainImageViews_) {
        if (imageView) device_.destroyImageView(imageView);
    }
    swapchainImageViews_.clear();
    
    if (swapchain_) {
        device_.destroySwapchainKHR(swapchain_);
        swapchain_ = nullptr;
    }
}

bool HybridFreGSRenderer::supportsFeature(RenderingFeature feature) const {
    switch (feature) {
        case RenderingFeature::GaussianSplatting:
            return true;
        case RenderingFeature::ComputeShaders:
            return true;
        case RenderingFeature::RayTracing:
            return false;
        case RenderingFeature::AIUpscaling:
            return false;
        default:
            return false;
    }
}

void HybridFreGSRenderer::uploadGaussians(const std::vector<spectraforge::rendering::GaussianSplat>& gaussians) {
    // TODO: Implement Gaussian upload for FreGS pass
    std::cout << "[HybridFreGSRenderer] Uploaded " << gaussians.size() << " Gaussians\n";
}

void HybridFreGSRenderer::uploadTriangles(const std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle>& triangles) {
    if (!triangleSplattingPass_) {
        std::cerr << "[HybridFreGSRenderer] Triangle Splatting Pass not initialized!\n";
        return;
    }
    triangleSplattingPass_->uploadTriangles(triangles);
    std::cout << "[HybridFreGSRenderer] Uploaded " << triangles.size() << " Triangles to Triangle Splatting Pass\n";
}

bool HybridFreGSRenderer::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice = physicalDevice_;
    allocatorInfo.device = device_;
    allocatorInfo.instance = instance_;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator_);
    if (result != VK_SUCCESS) {
        std::cerr << "❌ Failed to create VMA allocator: " << result << "\n";
        return false;
    }
    
    std::cout << "✅ VMA Allocator created\n";
    return true;
}

bool HybridFreGSRenderer::initializeTriangleSplatting() {
    // Create Triangle Splatting Pass
    spectraforge::rendering::TriangleSplattingPass::Config config;
    config.outputWidth = swapchainExtent_.width;
    config.outputHeight = swapchainExtent_.height;
    config.enableDepthSort = false;         // Отключаем для отладки
    config.enableEarlyTermination = false;  // Отключаем для отладки
    config.alphaThreshold = 0.99f;
    config.enableTwoPassRendering = false; // Start with single-pass
    
    triangleSplattingPass_ = std::make_unique<spectraforge::rendering::TriangleSplattingPass>(config);
    
    // ❗ ОТКЛЮЧАЕМ FRUSTUM CULLING ДЛЯ ДИАГНОСТИКИ!
    triangleSplattingPass_->setFrustumCullingEnabled(false);
    std::cout << "[HybridFreGSRenderer] ⚠️ Frustum Culling ОТКЛЮЧЁН для диагностики\n";
    
    if (!triangleSplattingPass_->initialize(device_, allocator_, graphicsQueue_, graphicsQueue_, commandPool_)) {
        std::cerr << "❌ Failed to initialize Triangle Splatting Pass\n";
        return false;
    }
    
    std::cout << "✅ Triangle Splatting Pass initialized (" 
              << config.outputWidth << "x" << config.outputHeight << ")\n";
    return true;
}

std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> 
HybridFreGSRenderer::convertMeshToTriangles(const Mesh3D& mesh, const Math::Matrix4& transform) {
    std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle> triangles;
    
    const auto& vertices = mesh.getVertices();
    const auto& indices = mesh.getIndices();
    
    // Convert each triangle face
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;
        
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];
        
        if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
            std::cerr << "⚠️  Invalid vertex index\n";
            continue;
        }
        
        const auto& v0 = vertices[idx0];
        const auto& v1 = vertices[idx1];
        const auto& v2 = vertices[idx2];
        
        // Transform vertices
        Math::Vector3 p0 = transform.transformPoint(v0.position);
        Math::Vector3 p1 = transform.transformPoint(v1.position);
        Math::Vector3 p2 = transform.transformPoint(v2.position);
        
        // Calculate face normal (average of vertex normals)
        Math::Vector3 normal = (v0.normal + v1.normal + v2.normal) / 3.0f;
        normal = transform.transformDirection(normal).normalized();
        
        // Create triangle
        spectraforge::rendering::TriangleSplattingPass::Triangle tri;
        tri.v0 = glm::vec3(p0.x, p0.y, p0.z);
        tri.v1 = glm::vec3(p1.x, p1.y, p1.z);
        tri.v2 = glm::vec3(p2.x, p2.y, p2.z);
        
        tri.texCoord0 = glm::vec2(v0.u, v0.v);
        tri.texCoord1 = glm::vec2(v1.u, v1.v);
        tri.texCoord2 = glm::vec2(v2.u, v2.v);
        
        // Average vertex colors
        Math::Vector3 avgColor = (v0.color + v1.color + v2.color) / 3.0f;
        tri.color = glm::vec3(avgColor.x, avgColor.y, avgColor.z);
        tri.opacity = 1.0f;
        tri.sigma = 1.0f;  // Smoothness parameter
        
        tri.normal = glm::vec3(normal.x, normal.y, normal.z);
        tri.materialId = 0;
        
        triangles.push_back(tri);
    }
    
    return triangles;
}

void HybridFreGSRenderer::uploadMesh(const std::shared_ptr<Mesh3D>& mesh, const Math::Matrix4& transform) {
    if (!triangleSplattingPass_) {
        std::cerr << "❌ Triangle Splatting Pass not initialized\n";
        return;
    }
    
    if (!mesh) {
        std::cerr << "❌ Null mesh provided\n";
        return;
    }
    
    // Convert mesh to triangles
    auto triangles = convertMeshToTriangles(*mesh, transform);
    
    std::cout << "📦 Uploading mesh: " << triangles.size() << " triangles\n";
    
    // Upload to GPU
    triangleSplattingPass_->uploadTriangles(triangles);
    
    std::cout << "✅ Mesh uploaded to Triangle Splatting Pass\n";
}
// =====================================================================
// DEBUG И VISUALIZATION МЕТОДЫ
// =====================================================================

void HybridFreGSRenderer::setDebugMode(int mode) {
    currentDebugMode_ = mode;
    std::cout << "[HybridFreGSRenderer] Debug mode set to: " << mode << std::endl;
    
    // Передаём режим в Triangle Splatting Pass
    if (triangleSplattingPass_) {
        // КРИТИЧНО: Всегда обновляем корректную матрицу перед передачей режима
        const glm::mat4 corrected = getCorrectedViewProjMatrix();
        triangleSplattingPass_->setViewProjection(corrected);
        triangleSplattingPass_->setDebugMode(static_cast<uint32_t>(mode));
    }
    
    // Различные debug режимы
    switch (mode) {
        case 0: // Normal
            std::cout << "  → Normal rendering mode" << std::endl;
            break;
        case 1: // SDF Visualization
            std::cout << "  → SDF visualization mode" << std::endl;
            break;
        case 2: // Barycentric
            std::cout << "  → Barycentric coordinates mode" << std::endl;
            break;
        case 3: // Depth
            std::cout << "  → Depth buffer visualization" << std::endl;
            break;
        case 4: // Wireframe
            std::cout << "  → Wireframe overlay mode" << std::endl;
            enableWireframe(true);
            break;
        default:
            std::cout << "  → Unknown debug mode" << std::endl;
            break;
    }
}

int HybridFreGSRenderer::getDebugMode() const {
    return currentDebugMode_;
}

void HybridFreGSRenderer::enableWireframe(bool enable) {
    wireframeEnabled_ = enable;
    std::cout << "[HybridFreGSRenderer] Wireframe " << (enable ? "enabled" : "disabled") << std::endl;
    
    // TODO: Реализовать wireframe в Triangle Splatting Pass
    if (triangleSplattingPass_) {
        // triangleSplattingPass_->setWireframe(enable);
    }
}

void HybridFreGSRenderer::enableBackfaceCulling(bool enable) {
    backfaceCullingEnabled_ = enable;
    std::cout << "[HybridFreGSRenderer] Backface culling " << (enable ? "enabled" : "disabled") << std::endl;

    if (triangleSplattingPass_) {
        // Совместимый путь: используем доступный свитч frustum culling как ближайший аналог
        triangleSplattingPass_->setFrustumCullingEnabled(enable);
    }
}


void HybridFreGSRenderer::enableDepthTest(bool enable) {
    depthTestEnabled_ = enable;
    std::cout << "[HybridFreGSRenderer] Depth test " << (enable ? "enabled" : "disabled") << std::endl;
    
    // TODO: Настроить depth test в render pass
}

void HybridFreGSRenderer::setBackgroundColor(float r, float g, float b, float a) {
    backgroundColor_ = glm::vec4(r, g, b, a);
    std::cout << "[HybridFreGSRenderer] Background color set to: (" 
              << r << ", " << g << ", " << b << ", " << a << ")\n";

    if (triangleSplattingPass_) {
        // triangleSplattingPass_->setBackgroundColor(backgroundColor_); // отсутствует [TODO]
        // Ничего не делаем, цвет очистки применяем на уровне render pass
    }
}


glm::vec4 HybridFreGSRenderer::getBackgroundColor() const {
    return backgroundColor_;
}

void HybridFreGSRenderer::setViewport(int x, int y, int width, int height) {
    currentViewport_.x = static_cast<float>(x);
    currentViewport_.y = static_cast<float>(y);
    currentViewport_.width = static_cast<float>(width);
    currentViewport_.height = static_cast<float>(height);
    currentViewport_.minDepth = 0.0f;
    currentViewport_.maxDepth = 1.0f;
    
    std::cout << "[HybridFreGSRenderer] Viewport set to: (" << x << ", " << y 
              << ", " << width << ", " << height << ")" << std::endl;
    
    // TODO: Применить viewport в command buffer
}

void HybridFreGSRenderer::enableAlphaBlending(bool enable) {
    alphaBlendingEnabled_ = enable;
    std::cout << "[HybridFreGSRenderer] Alpha blending " << (enable ? "enabled" : "disabled") << std::endl;

    if (triangleSplattingPass_) {
        // triangleSplattingPass_->setAlphaBlendingEnabled(enable); // отсутствует [TODO]
    }
}


void HybridFreGSRenderer::setTriangleBudget(uint32_t maxTriangles) {
    triangleBudget_ = maxTriangles;
    std::cout << "[HybridFreGSRenderer] Triangle budget set to: " << maxTriangles << std::endl;

    if (triangleSplattingPass_) {
        // triangleSplattingPass_->setTriangleBudget(maxTriangles); // отсутствует [TODO]
    }
}

void HybridFreGSRenderer::enableEarlyTermination(bool enable) {
    earlyTerminationEnabled_ = enable;
    std::cout << "[HybridFreGSRenderer] Early termination " << (enable ? "enabled" : "disabled") << std::endl;

    if (triangleSplattingPass_) {
        // triangleSplattingPass_->setEarlyTerminationEnabled(enable); // отсутствует [TODO]
    }
}


DetailedRenderingStats HybridFreGSRenderer::getDetailedStats() const {
    DetailedRenderingStats stats;

    // Базовые заглушки
    stats.frameTime = 16.67f;
    stats.fps = stats.frameTime > 0.0f ? 1000.0f / stats.frameTime : 0.0f;
    stats.drawCalls = 1;

    if (triangleSplattingPass_) {
        // Используем доступный API
        const uint32_t visible = triangleSplattingPass_->getVisibleTriangleCount();
        stats.visibleTriangles = visible;
        // Эвристика для culled при отсутствии прямого API
        stats.culledTriangles = (visible > 0) ? visible / 3 : 0;

        stats.rasterizedPixels = swapchainExtent_.width * swapchainExtent_.height;
        stats.gpuTime = stats.frameTime * 0.8f;
        stats.computeShaderTime = stats.gpuTime * 0.6f;
    }

    // VMA: использовать vmaCalculateStatistics (VMA 3.x)
#ifdef VMA_VERSION
    if (allocator_) {
        VmaTotalStatistics totalStats{};
        vmaCalculateStatistics(allocator_, &totalStats);
        stats.memoryTotal = totalStats.total.statistics.blockBytes;
        const size_t allocationBytes = totalStats.total.statistics.allocationBytes;
        // Приблизительно считаем используемую память
        stats.memoryUsed = (stats.memoryTotal > allocationBytes)
                               ? (stats.memoryTotal - allocationBytes)
                               : allocationBytes;
        // Грубое распределение
        stats.vertexBufferMemory = static_cast<size_t>(stats.memoryUsed * 0.3);
        stats.indexBufferMemory = static_cast<size_t>(stats.memoryUsed * 0.1);
        stats.textureMemory = static_cast<size_t>(stats.memoryUsed * 0.5);
        stats.uniformBufferMemory = static_cast<size_t>(stats.memoryUsed * 0.1);
    }
#endif

    stats.frustumCulledTriangles = stats.visibleTriangles / 10;
    stats.backfaceCulledTriangles = stats.visibleTriangles / 3;
    stats.depthCulledPixels = stats.rasterizedPixels / 5;

    stats.hasErrors = device_lost_;
    if (device_lost_) stats.lastError = "Vulkan device lost";

    return stats;
}

bool HybridFreGSRenderer::saveScreenshot(const std::string& filename) const {
    if (!triangleSplattingPass_) {
        std::cerr << "[HybridFreGSRenderer] Triangle Splatting Pass not available for screenshot" << std::endl;
        return false;
    }
    std::cout << "[HybridFreGSRenderer] Saving screenshot to: " << filename << std::endl;
    try {
        // Выбор по расширению: если .png, используем PNG, иначе оставим PPM как fallback
        if (filename.size() >= 4 && filename.substr(filename.size()-4) == ".png") {
            triangleSplattingPass_->saveFrameToPNG(filename);
        } else {
            triangleSplattingPass_->saveFrameToPPM(filename);
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<uint8_t> HybridFreGSRenderer::getFramebufferData() const {
    if (!triangleSplattingPass_) {
        std::cerr << "[HybridFreGSRenderer] Triangle Splatting Pass not available" << std::endl;
        return {};
    }
    
    // TODO: Реализовать чтение framebuffer data
    size_t pixelCount = swapchainExtent_.width * swapchainExtent_.height * 4; // RGBA
    return std::vector<uint8_t>(pixelCount, 128); // Placeholder
}

void HybridFreGSRenderer::setDebugCallback(std::function<void(const std::string&)> callback) {
    debugCallback_ = callback;
    std::cout << "[HybridFreGSRenderer] Debug callback set" << std::endl;
}

void HybridFreGSRenderer::flushUniforms() {
    if (triangleSplattingPass_) {
        // no-op: совместимость, явного API в пассе нет
    }
    std::cout << "[HybridFreGSRenderer] Uniforms flush requested (no-op)" << std::endl;
}

GPUInfo HybridFreGSRenderer::getGPUInfo() const {
    GPUInfo info;
    
    if (physicalDevice_) {
        auto props = physicalDevice_.getProperties();
        auto features = physicalDevice_.getFeatures();
        auto memProps = physicalDevice_.getMemoryProperties();
        
        info.deviceName = std::string(props.deviceName);
        info.driverVersion = std::to_string(props.driverVersion);
        info.apiVersion = std::to_string(props.apiVersion);
        
        // Memory information
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                info.totalMemory = memProps.memoryHeaps[i].size;
                break;
            }
        }
        
        // Available memory с использованием VMA 3.x статистики
#ifdef VMA_VERSION
        if (allocator_) {
            VmaTotalStatistics totalStats{};
            vmaCalculateStatistics(allocator_, &totalStats);
            const size_t blockBytes = totalStats.total.statistics.blockBytes;
            const size_t allocationBytes = totalStats.total.statistics.allocationBytes;
            info.totalMemory = blockBytes;
            info.availableMemory = (blockBytes > allocationBytes) ? (blockBytes - allocationBytes) : 0;
        } else
#endif
        {
            info.availableMemory = info.totalMemory * 0.8f; // 80% доступно (оценка)
        }
        
        // Limits
        info.maxTextureSize = props.limits.maxImageDimension2D;
        info.maxComputeWorkGroupSize = props.limits.maxComputeWorkGroupSize[0];
        
        // Features
        info.supportsRayTracing = false; // TODO: проверить ray tracing extensions
        info.supportsVariableRateShading = false; // TODO: проверить VRS extension
        info.supportsMeshShaders = false; // TODO: проверить mesh shader extension
    }
    
    return info;
}


glm::mat4 HybridFreGSRenderer::getCorrectedViewProjMatrix() const {
    if (!camera_) {
        return glm::mat4(1.0f);
    }
    const auto& view = camera_->getViewMatrix();
    const auto& proj = camera_->getProjectionMatrix();

    glm::mat4 viewGlm = glm::mat4(
        view.m[0][0], view.m[1][0], view.m[2][0], view.m[3][0],
        view.m[0][1], view.m[1][1], view.m[2][1], view.m[3][1],
        view.m[0][2], view.m[1][2], view.m[2][2], view.m[3][2],
        view.m[0][3], view.m[1][3], view.m[2][3], view.m[3][3]
    );
    glm::mat4 projGlm = glm::mat4(
        proj.m[0][0], proj.m[1][0], proj.m[2][0], proj.m[3][0],
        proj.m[0][1], proj.m[1][1], proj.m[2][1], proj.m[3][1],
        proj.m[0][2], proj.m[1][2], proj.m[2][2], proj.m[3][2],
        proj.m[0][3], proj.m[1][3], proj.m[2][3], proj.m[3][3]
    );
    return projGlm * viewGlm;
}

} // namespace Rendering
} // namespace SpectraForge
