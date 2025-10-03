/**
 * @file HybridFreGSRenderer.cpp
 */

#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/core/VMAMemoryManager.h"
#include "SpectraForge/Core/Window.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace SpectraForge::Rendering;

namespace SpectraForge {
namespace Rendering {

HybridFreGSRenderer::HybridFreGSRenderer() = default;
HybridFreGSRenderer::~HybridFreGSRenderer() { shutdown(); }

bool HybridFreGSRenderer::initialize() {
    if (initialized_) return true;

    // Создаём Vulkan контекст
    vkContext_ = spectraforge::createVulkanContext(true);
    if (!vkContext_) {
        std::cerr << "HybridFreGSRenderer: не удалось создать VulkanContext" << std::endl;
        return false;
    }
    try {
    // Конфиг для волнового пасса
    spectraforge::rendering::WaveletPassConfig wcfg{};
    wcfg.inputWidth = 1920;
    wcfg.inputHeight = 1080;
    wavelet_ = std::make_unique<spectraforge::rendering::WaveletPass>(wcfg);
        if (!wavelet_->initialize(*vkContext_)) {
            std::cerr << "HybridFreGSRenderer: WaveletPass initialize failed" << std::endl;
            return false;
        }

    // Конфиг для FreGS пасса
    spectraforge::rendering::FreGSPassConfig fcfg{};
    fcfg.outputWidth = 1920;
    fcfg.outputHeight = 1080;
    fregs_ = std::make_unique<spectraforge::rendering::FreGSPass>(fcfg);
        // Передать саббенды до initialize
        fregs_->setInputSubbands(wavelet_->getSubbands());
        if (!fregs_->initialize(*vkContext_)) {
            std::cerr << "HybridFreGSRenderer: FreGSPass initialize failed" << std::endl;
            return false;
        }
        // === Presentation: поверхность и swapchain ===
        if (!createSurfaceFromCurrentGLFW()) {
            std::cerr << "HybridFreGSRenderer: не удалось создать VkSurfaceKHR (GLFW)" << std::endl;
            return false;
        }

        int fbw = 1920, fbh = 1080;
        if (GLFWwindow* win = glfwGetCurrentContext()) {
            glfwGetFramebufferSize(win, &fbw, &fbh);
        }
        swapchainExtent_ = vk::Extent2D{ static_cast<uint32_t>(fbw), static_cast<uint32_t>(fbh) };
        if (!createSwapchainAndViews(swapchainExtent_.width, swapchainExtent_.height)) {
            std::cerr << "HybridFreGSRenderer: не удалось создать swapchain" << std::endl;
            return false;
        }

        // Sync primitives
        vk::Device dev = vkContext_->getDevice();
        vk::SemaphoreCreateInfo sci{};
        imageAvailableSemaphore_ = dev.createSemaphore(sci);
        renderFinishedSemaphore_ = dev.createSemaphore(sci);
        vk::FenceCreateInfo fci{ vk::FenceCreateFlagBits::eSignaled };
        inFlightFence_ = dev.createFence(fci);

    } catch (const std::exception& e) {
        std::cerr << "HybridFreGSRenderer: исключение при инициализации пассов: " << e.what() << std::endl;
        return false;
    }

    initialized_ = true;
    return true;
}

void HybridFreGSRenderer::shutdown() {
    if (!initialized_) return;
    vk::Device dev = vkContext_ ? vkContext_->getDevice() : vk::Device{};

    if (dev) { (void)dev.waitIdle(); }
    if (fregs_) { fregs_->cleanup(); fregs_.reset(); }
    if (wavelet_) { wavelet_->cleanup(); wavelet_.reset(); }

    if (dev) {
        if (inFlightFence_) { dev.destroyFence(inFlightFence_); inFlightFence_ = nullptr; }
        if (renderFinishedSemaphore_) { dev.destroySemaphore(renderFinishedSemaphore_); renderFinishedSemaphore_ = nullptr; }
        if (imageAvailableSemaphore_) { dev.destroySemaphore(imageAvailableSemaphore_); imageAvailableSemaphore_ = nullptr; }
    }

    destroySwapchainAndViews();
    if (vkContext_ && surface_) { vkContext_->getInstance().destroySurfaceKHR(surface_); surface_ = nullptr; }
    vkContext_.reset();
    initialized_ = false;
}

bool HybridFreGSRenderer::supportsFeature(RenderingFeature feature) const {
    switch (feature) {
        case RenderingFeature::ComputeShaders:
        case RenderingFeature::GaussianSplatting:
        case RenderingFeature::TemporalEffects:
            return true;
        default:
            return false;
    }
}

void HybridFreGSRenderer::beginFrame() {
    // noop: реальный командный буфер создаётся в контексте VulkanRenderer
}

void HybridFreGSRenderer::endFrame() {
    // noop
}

void HybridFreGSRenderer::renderFrame(const FrameData&) {
    if (!initialized_) return;

    // Ожидаем fence прошлого кадра и получаем изображение
    vk::Device device = vkContext_->getDevice();
    if (device.waitForFences(1, &inFlightFence_, VK_TRUE, UINT64_C(1000000000)) != vk::Result::eSuccess) {
        return;
    }
    (void)device.resetFences(1, &inFlightFence_);
    uint32_t imageIndex = 0;
    vk::Result acq = device.acquireNextImageKHR(swapchain_, UINT64_MAX, imageAvailableSemaphore_, nullptr, &imageIndex);
    if (acq == vk::Result::eErrorOutOfDateKHR || acq == vk::Result::eSuboptimalKHR) {
        destroySwapchainAndViews();
        createSwapchainAndViews(swapchainExtent_.width, swapchainExtent_.height);
        return;
    }
    if (acq != vk::Result::eSuccess) {
        return;
    }

    // Одноразовый командный буфер
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = vkContext_->getCommandPool();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    auto cmdBufs = device.allocateCommandBuffers(allocInfo);
    vk::CommandBuffer cmd = cmdBufs[0];

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);

    // 1) Wavelet pass
    // Перед первым использованием переведём саббенды в GENERAL
    {
        const auto& subs = wavelet_->getSubbands();
        std::array<vk::ImageMemoryBarrier, 4> barriers{};
        const vk::Image images[4] = { subs.imageLL, subs.imageLH, subs.imageHL, subs.imageHH };
        for (int i = 0; i < 4; ++i) {
            barriers[i].srcAccessMask = {};
            barriers[i].dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            barriers[i].oldLayout = vk::ImageLayout::eUndefined;
            barriers[i].newLayout = vk::ImageLayout::eGeneral;
            barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].image = images[i];
            barriers[i].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barriers[i].subresourceRange.baseMipLevel = 0;
            barriers[i].subresourceRange.levelCount = 1;
            barriers[i].subresourceRange.baseArrayLayer = 0;
            barriers[i].subresourceRange.layerCount = 1;
        }
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }
    wavelet_->execute(cmd, /*frameIndex*/ 0);

    // Барьер между пассами для всех 4 субдиапазонов: shaderWrite->shaderRead GENERAL->GENERAL
    {
        std::array<vk::ImageMemoryBarrier, 4> barriers{};
        const auto& subs = wavelet_->getSubbands();
        const vk::Image images[4] = { subs.imageLL, subs.imageLH, subs.imageHL, subs.imageHH };
        for (int i = 0; i < 4; ++i) {
            barriers[i].srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            barriers[i].dstAccessMask = vk::AccessFlagBits::eShaderRead;
            barriers[i].oldLayout = vk::ImageLayout::eGeneral;
            barriers[i].newLayout = vk::ImageLayout::eGeneral;
            barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].image = images[i];
            barriers[i].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barriers[i].subresourceRange.baseMipLevel = 0;
            barriers[i].subresourceRange.levelCount = 1;
            barriers[i].subresourceRange.baseArrayLayer = 0;
            barriers[i].subresourceRange.layerCount = 1;
        }
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }

    // 2) FreGS pass
    fregs_->execute(cmd, /*frameIndex*/ 0);

    // Blit to swapchain and prepare for present
    {
        vk::Image srcImage = fregs_->getOutputImage();
        // src GENERAL -> TRANSFER_SRC
        vk::ImageMemoryBarrier b0{};
        b0.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        b0.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        b0.oldLayout = vk::ImageLayout::eGeneral;
        b0.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        b0.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b0.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b0.image = srcImage;
        b0.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        b0.subresourceRange.baseMipLevel = 0;
        b0.subresourceRange.levelCount = 1;
        b0.subresourceRange.baseArrayLayer = 0;
        b0.subresourceRange.layerCount = 1;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &b0);

        // dst UNDEFINED -> TRANSFER_DST
        vk::Image dstImage = swapchainImages_[imageIndex];
        vk::ImageMemoryBarrier b1{};
        b1.srcAccessMask = {};
        b1.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        b1.oldLayout = vk::ImageLayout::eUndefined;
        b1.newLayout = vk::ImageLayout::eTransferDstOptimal;
        b1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b1.image = dstImage;
        b1.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        b1.subresourceRange.baseMipLevel = 0;
        b1.subresourceRange.levelCount = 1;
        b1.subresourceRange.baseArrayLayer = 0;
        b1.subresourceRange.layerCount = 1;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &b1);

        // Blit (linear)
        vk::ImageBlit blit{};
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = 0;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D{ static_cast<int32_t>(swapchainExtent_.width), static_cast<int32_t>(swapchainExtent_.height), 1 };
        blit.dstSubresource = blit.srcSubresource;
        blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D{ static_cast<int32_t>(swapchainExtent_.width), static_cast<int32_t>(swapchainExtent_.height), 1 };
        cmd.blitImage(srcImage, vk::ImageLayout::eTransferSrcOptimal,
                      dstImage, vk::ImageLayout::eTransferDstOptimal,
                      1, &blit, vk::Filter::eLinear);

        // dst -> PRESENT
        vk::ImageMemoryBarrier b2{};
        b2.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        b2.dstAccessMask = {};
        b2.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        b2.newLayout = vk::ImageLayout::ePresentSrcKHR;
        b2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b2.image = dstImage;
        b2.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        b2.subresourceRange.baseMipLevel = 0;
        b2.subresourceRange.levelCount = 1;
        b2.subresourceRange.baseArrayLayer = 0;
        b2.subresourceRange.layerCount = 1;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &b2);

        // src -> GENERAL back
        vk::ImageMemoryBarrier b3{};
        b3.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        b3.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
        b3.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        b3.newLayout = vk::ImageLayout::eGeneral;
        b3.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b3.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b3.image = srcImage;
        b3.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        b3.subresourceRange.baseMipLevel = 0;
        b3.subresourceRange.levelCount = 1;
        b3.subresourceRange.baseArrayLayer = 0;
        b3.subresourceRange.layerCount = 1;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &b3);
    }
    // Submit + present
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eTransfer;
    vk::SubmitInfo submit{};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &imageAvailableSemaphore_;
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinishedSemaphore_;
    [[maybe_unused]] auto submitResB = vkContext_->getGraphicsQueue().submit(1, &submit, inFlightFence_);

    vk::PresentInfoKHR present{};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinishedSemaphore_;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &imageIndex;
    vk::Result pres = vkContext_->getGraphicsQueue().presentKHR(present);
    if (pres == vk::Result::eErrorOutOfDateKHR || pres == vk::Result::eSuboptimalKHR) {
        destroySwapchainAndViews();
        createSwapchainAndViews(swapchainExtent_.width, swapchainExtent_.height);
    }
    if (pres != vk::Result::eSuccess) {
        // Пропускаем кадр при ошибке презентации
        return;
    }

    device.freeCommandBuffers(vkContext_->getCommandPool(), 1, &cmd);
}

// === Swapchain helpers ===
bool HybridFreGSRenderer::createSurfaceFromCurrentGLFW() {
    if (!vkContext_) return false;
    
    // Получаем окно через статический метод Window (для Vulkan окон с GLFW_NO_API)
    GLFWwindow* win = Core::Window::getLastCreatedWindow();
    if (!win) {
        std::cerr << "HybridFreGSRenderer: нет доступного GLFW окна для создания surface" << std::endl;
        return false;
    }

    VkSurfaceKHR rawSurface = VK_NULL_HANDLE;
    VkResult res = glfwCreateWindowSurface(static_cast<VkInstance>(vkContext_->getInstance()), win, nullptr, &rawSurface);
    if (res != VK_SUCCESS) {
        std::cerr << "HybridFreGSRenderer: ошибка создания VkSurfaceKHR, код: " << res << std::endl;
        return false;
    }

    surface_ = vk::SurfaceKHR(rawSurface);
    return true;
}

bool HybridFreGSRenderer::createSwapchainAndViews(uint32_t width, uint32_t height) {
    if (!vkContext_ || !surface_) return false;

    vk::Device dev = vkContext_->getDevice();
    vk::PhysicalDevice physDev = vkContext_->getPhysicalDevice();

    // Query surface capabilities
    vk::SurfaceCapabilitiesKHR surfCaps = physDev.getSurfaceCapabilitiesKHR(surface_);
    auto formats = physDev.getSurfaceFormatsKHR(surface_);
    auto presentModes = physDev.getSurfacePresentModesKHR(surface_);

    // Pick format
    swapchainFormat_ = vk::Format::eB8G8R8A8Unorm;
    for (auto& fmt : formats) {
        if (fmt.format == vk::Format::eB8G8R8A8Unorm || fmt.format == vk::Format::eR8G8B8A8Unorm) {
            swapchainFormat_ = fmt.format;
            break;
        }
    }

    // Clamp extent
    if (width < surfCaps.minImageExtent.width) width = surfCaps.minImageExtent.width;
    if (height < surfCaps.minImageExtent.height) height = surfCaps.minImageExtent.height;
    if (width > surfCaps.maxImageExtent.width) width = surfCaps.maxImageExtent.width;
    if (height > surfCaps.maxImageExtent.height) height = surfCaps.maxImageExtent.height;
    swapchainExtent_ = vk::Extent2D{width, height};

    // Create swapchain
    vk::SwapchainCreateInfoKHR sci{};
    sci.surface = surface_;
    sci.minImageCount = std::max(2u, surfCaps.minImageCount);
    if (surfCaps.maxImageCount > 0 && sci.minImageCount > surfCaps.maxImageCount) {
        sci.minImageCount = surfCaps.maxImageCount;
    }
    sci.imageFormat = swapchainFormat_;
    sci.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    sci.imageExtent = swapchainExtent_;
    sci.imageArrayLayers = 1;
    sci.imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment;
    sci.imageSharingMode = vk::SharingMode::eExclusive;
    sci.preTransform = surfCaps.currentTransform;
    sci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    sci.presentMode = vk::PresentModeKHR::eFifo;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = nullptr;

    swapchain_ = dev.createSwapchainKHR(sci);
    swapchainImages_ = dev.getSwapchainImagesKHR(swapchain_);

    // Create image views
    swapchainImageViews_.resize(swapchainImages_.size());
    for (size_t i = 0; i < swapchainImages_.size(); ++i) {
        vk::ImageViewCreateInfo ivci{};
        ivci.image = swapchainImages_[i];
        ivci.viewType = vk::ImageViewType::e2D;
        ivci.format = swapchainFormat_;
        ivci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount = 1;
        swapchainImageViews_[i] = dev.createImageView(ivci);
    }

    return true;
}

void HybridFreGSRenderer::destroySwapchainAndViews() {
    if (!vkContext_) return;
    vk::Device dev = vkContext_->getDevice();
    if (!dev) return;

    for (auto& view : swapchainImageViews_) {
        if (view) dev.destroyImageView(view);
    }
    swapchainImageViews_.clear();
    swapchainImages_.clear();

    if (swapchain_) {
        dev.destroySwapchainKHR(swapchain_);
        swapchain_ = nullptr;
    }
}

}  // namespace Rendering
}  // namespace SpectraForge


