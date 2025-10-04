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
#include <cstring>
#include <cmath>
#include <limits>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// stb_image_write для сохранения PNG
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <stb_image_write.h> // Временно отключено - библиотека не установлена

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
    int fbw_init = 1920, fbh_init = 1080;
    if (GLFWwindow* win0 = Core::Window::getLastCreatedWindow()) {
        glfwGetFramebufferSize(win0, &fbw_init, &fbh_init);
    }
    wcfg.inputWidth = static_cast<uint32_t>(fbw_init);
    wcfg.inputHeight = static_cast<uint32_t>(fbh_init);
    wavelet_ = std::make_unique<spectraforge::rendering::WaveletPass>(wcfg);
        if (!wavelet_->initialize(*vkContext_)) {
            std::cerr << "HybridFreGSRenderer: WaveletPass initialize failed" << std::endl;
            return false;
        }

    // Конфиг для FreGS пасса
    spectraforge::rendering::FreGSPassConfig fcfg{};
    fcfg.outputWidth = static_cast<uint32_t>(fbw_init);
    fcfg.outputHeight = static_cast<uint32_t>(fbh_init);
    fregs_ = std::make_unique<spectraforge::rendering::FreGSPass>(fcfg);
        // Передать саббенды до initialize
        fregs_->setInputSubbands(wavelet_->getSubbands());
        if (!fregs_->initialize(*vkContext_)) {
            std::cerr << "HybridFreGSRenderer: FreGSPass initialize failed" << std::endl;
            return false;
        }

    // Конфиг для Triangle Splatting пасса
    // Создаём VMA allocator для Triangle Splatting
    // Используем Vulkan-Hpp dispatcher для получения функций
    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    
    VmaAllocatorCreateInfo vmaInfo{};
    vmaInfo.flags = 0;
    vmaInfo.physicalDevice = static_cast<VkPhysicalDevice>(vkContext_->getPhysicalDevice());
    vmaInfo.device = static_cast<VkDevice>(vkContext_->getDevice());
    vmaInfo.instance = static_cast<VkInstance>(vkContext_->getInstance());
    vmaInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    vmaInfo.pVulkanFunctions = &vulkanFunctions;
    
    VkResult vmaResult = vmaCreateAllocator(&vmaInfo, &triangleSplattingVmaAllocator_);
    if (vmaResult != VK_SUCCESS) {
        std::cerr << "HybridFreGSRenderer: Failed to create VMA allocator for Triangle Splatting" << std::endl;
        return false;
    }
    
    spectraforge::rendering::TriangleSplattingPass::Config tscfg{};
    tscfg.outputWidth = static_cast<uint32_t>(fbw_init);
    tscfg.outputHeight = static_cast<uint32_t>(fbh_init);
    tscfg.enableDepthSort = true;
    tscfg.enableEarlyTermination = true;
    triangleSplatting_ = std::make_unique<spectraforge::rendering::TriangleSplattingPass>(tscfg);
        if (!triangleSplatting_->initialize(
            vkContext_->getDevice(),
            triangleSplattingVmaAllocator_,
            vkContext_->getComputeQueue(),
            vkContext_->getGraphicsQueue(),
            vkContext_->getCommandPool()
        )) {
            std::cerr << "HybridFreGSRenderer: TriangleSplattingPass initialize failed" << std::endl;
            vmaDestroyAllocator(triangleSplattingVmaAllocator_);
            triangleSplattingVmaAllocator_ = nullptr;
            return false;
        }
        // === Presentation: поверхность и swapchain ===
        if (!createSurfaceFromCurrentGLFW()) {
            std::cerr << "HybridFreGSRenderer: не удалось создать VkSurfaceKHR (GLFW)" << std::endl;
            return false;
        }

        int fbw = 1920, fbh = 1080;
    if (GLFWwindow* win = Core::Window::getLastCreatedWindow()) {
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
    if (triangleSplatting_) { triangleSplatting_->cleanup(); triangleSplatting_.reset(); }
    if (triangleSplattingVmaAllocator_) { vmaDestroyAllocator(triangleSplattingVmaAllocator_); triangleSplattingVmaAllocator_ = nullptr; }
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

void HybridFreGSRenderer::renderFrame(const FrameData& frameData) {
    if (!initialized_) return;
    
    // Обновляем VP матрицу в активном рендерере
    glm::mat4 VP = frameData.camera.projectionMatrix * frameData.camera.viewMatrix;
    
    if (renderMode_ == RenderMode::GaussianSplatting && fregs_) {
        fregs_->updateViewProjection(VP);
    } else if (renderMode_ == RenderMode::TriangleSplatting && triangleSplatting_) {
        triangleSplatting_->setViewProjection(VP);
    }

    // Ожидаем fence прошлого кадра с TIMEOUT (5 секунд)
    vk::Device device = vkContext_->getDevice();
    // Используем разумный timeout вместо UINT64_MAX для предотвращения зависания
    constexpr uint64_t FENCE_TIMEOUT_NS = 5'000'000'000; // 5 секунд
    vk::Result fenceWait = device.waitForFences(1, &inFlightFence_, VK_TRUE, FENCE_TIMEOUT_NS);
    
    if (fenceWait == vk::Result::eTimeout) {
        std::cerr << "[HybridFreGSRenderer] ⚠️ TIMEOUT: Fence ожидание превысило 5 секунд!" << std::endl;
        std::cerr << "  GPU может быть перегружен или deadlock в pipeline." << std::endl;
        return; // Пропускаем кадр вместо зависания системы
    }
    
    if (fenceWait != vk::Result::eSuccess) {
        std::cerr << "[HybridFreGSRenderer] ❌ Ошибка waitForFences: " << vk::to_string(fenceWait) << std::endl;
        return;
    }
    
    // Безопасно освободим командный буфер прошлого кадра
    if (lastSubmittedCmd_) {
        device.freeCommandBuffers(vkContext_->getCommandPool(), 1, &lastSubmittedCmd_);
        lastSubmittedCmd_ = nullptr;
    }
    (void)device.resetFences(1, &inFlightFence_);
    
    // Acquire swapchain image с TIMEOUT (1 секунда)
    uint32_t imageIndex = 0;
    constexpr uint64_t ACQUIRE_TIMEOUT_NS = 1'000'000'000; // 1 секунда
    vk::Result acq = device.acquireNextImageKHR(swapchain_, ACQUIRE_TIMEOUT_NS, imageAvailableSemaphore_, nullptr, &imageIndex);
    
    if (acq == vk::Result::eTimeout) {
        std::cerr << "[HybridFreGSRenderer] ⚠️ TIMEOUT: acquireNextImageKHR превысило 1 секунду!" << std::endl;
        std::cerr << "  Возможно окно минимизировано или compositor не отвечает." << std::endl;
        return; // Пропускаем кадр
    }
    
    if (acq == vk::Result::eErrorOutOfDateKHR || acq == vk::Result::eSuboptimalKHR) {
        std::cout << "[HybridFreGSRenderer] 🔄 Пересоздание swapchain (out of date/suboptimal)" << std::endl;
        destroySwapchainAndViews();
        createSwapchainAndViews(swapchainExtent_.width, swapchainExtent_.height);
        return;
    }
    
    if (acq != vk::Result::eSuccess) {
        std::cerr << "[HybridFreGSRenderer] ❌ Ошибка acquireNextImageKHR: " << vk::to_string(acq) << std::endl;
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

    vk::Image srcImage{};
    
    // === DUAL-PATH RENDERING ===
    if (renderMode_ == RenderMode::TriangleSplatting && triangleSplatting_) {
        // Triangle Splatting path (прямой рендеринг без wavelets)
        
        // ===== ИСПРАВЛЕНИЕ СИНХРОНИЗАЦИИ: Transition Triangle Splatting output image =====
        // Первый кадр: UNDEFINED -> GENERAL
        // Последующие кадры: GENERAL -> GENERAL (уже в правильном layout)
        vk::Image tsOutputImage = triangleSplatting_->getOutputImage();
        
        vk::ImageMemoryBarrier tsBarrier;
        tsBarrier.srcAccessMask = frameIndex_ == 0 ? vk::AccessFlagBits::eNone : vk::AccessFlagBits::eShaderRead;
        tsBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
        tsBarrier.oldLayout = frameIndex_ == 0 ? vk::ImageLayout::eUndefined : vk::ImageLayout::eGeneral;
        tsBarrier.newLayout = vk::ImageLayout::eGeneral;
        tsBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        tsBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        tsBarrier.image = tsOutputImage;
        tsBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        tsBarrier.subresourceRange.baseMipLevel = 0;
        tsBarrier.subresourceRange.levelCount = 1;
        tsBarrier.subresourceRange.baseArrayLayer = 0;
        tsBarrier.subresourceRange.layerCount = 1;
        
        cmd.pipelineBarrier(
            frameIndex_ == 0 ? vk::PipelineStageFlagBits::eTopOfPipe : vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            1, &tsBarrier
        );
        
        // Логирование для первого кадра
        if (frameIndex_ == 0) {
            std::cout << "[HybridFreGSRenderer] 🔄 Triangle Splatting Image: UNDEFINED → GENERAL\n";
        }
        
        // Set camera position for depth sorting
        glm::vec3 cameraPos(
            static_cast<float>(frameData.camera.position.x),
            static_cast<float>(frameData.camera.position.y),
            static_cast<float>(frameData.camera.position.z)
        );
        triangleSplatting_->setCameraPosition(cameraPos);
        
        triangleSplatting_->execute(cmd, frameIndex_);
        srcImage = tsOutputImage;
        
        // Log culling statistics (once after a few frames)
        static bool cullingStatsLogged = false;
        if (!cullingStatsLogged && frameIndex_ >= 2) {
            uint32_t totalTriangles = triangleSplatting_->getTriangleCount();
            uint32_t visibleTriangles = triangleSplatting_->getVisibleTriangleCount();
            float culledPercentage = 100.0f * (1.0f - float(visibleTriangles) / float(totalTriangles));
            
            std::cout << "\n[Frustum Culling Stats]\n";
            std::cout << "  Total triangles:   " << totalTriangles << "\n";
            std::cout << "  Visible triangles: " << visibleTriangles << "\n";
            std::cout << "  Culled:            " << (totalTriangles - visibleTriangles) 
                      << " (" << culledPercentage << "%)\n\n";
            
            cullingStatsLogged = true;
        }
        
    } else if (renderMode_ == RenderMode::GaussianSplatting && fregs_ && wavelet_) {
        // Gaussian Splatting path (с wavelets)
        
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

        // Барьер между пассами
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
        srcImage = fregs_->getOutputImage();
    } else {
        // Fallback: skip frame if no valid mode
        cmd.end();
        device.freeCommandBuffers(vkContext_->getCommandPool(), 1, &cmd);
        return;
    }

    // ===== ИСПРАВЛЕНИЕ СИНХРОНИЗАЦИИ: Blit to swapchain and prepare for present =====
    {
        // src GENERAL -> TRANSFER_SRC (с полным барьером для записи шейдера)
        vk::ImageMemoryBarrier b0{};
        b0.srcAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
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

        // Blit выходного изображения FreGS на swapchain
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
    // Завершаем запись командного буфера
    cmd.end();

    // Submit + present
    // Мы выполняем transfer на swapchain image — ждём на TRANSFER стадии
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
    // На некоторых драйверах требуется явное указание пустых списков
    present.pResults = nullptr;
    vk::Result pres = vkContext_->getGraphicsQueue().presentKHR(present);
    if (pres == vk::Result::eErrorOutOfDateKHR || pres == vk::Result::eSuboptimalKHR) {
        destroySwapchainAndViews();
        createSwapchainAndViews(swapchainExtent_.width, swapchainExtent_.height);
    }
    if (pres != vk::Result::eSuccess) {
        // Пропускаем кадр при ошибке презентации
        // Освободим командный буфер, так как fence не будет сигнализирован
        device.freeCommandBuffers(vkContext_->getCommandPool(), 1, &cmd);
        return;
    }
    
    // === DEBUG: Сохраняем кадр в PNG для анализа (первый отрендеренный кадр) ===
    if (!frameCaptured_) {
        std::cout << "[HybridFreGSRenderer] 📸 Capturing frame #" << frameIndex_ << " to PNG...\n";
        saveFrameToPNG(srcImage, swapchainExtent_.width, swapchainExtent_.height, "triangle_splatting_frame.png");
        frameCaptured_ = true;
        std::cout << "[HybridFreGSRenderer] ✅ Frame saved successfully!\n";
    }
    
    // Освобождать командный буфер будем на следующем кадре после ожидания fence
    lastSubmittedCmd_ = cmd;
    ++frameIndex_;
}

void HybridFreGSRenderer::uploadGaussians(const std::vector<spectraforge::rendering::GaussianSplat>& gaussians) {
    if (!fregs_) return;
    fregs_->uploadGaussians(gaussians);
}

void HybridFreGSRenderer::uploadTriangles(const std::vector<spectraforge::rendering::TriangleSplattingPass::Triangle>& triangles) {
    if (!triangleSplatting_) return;
    triangleSplatting_->uploadTriangles(triangles);
    // Автоматически переключаемся на Triangle Splatting режим
    renderMode_ = RenderMode::TriangleSplatting;
    std::cout << "[HybridFreGSRenderer] Uploaded " << triangles.size() 
              << " triangles, switched to TriangleSplatting mode\n";
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

void HybridFreGSRenderer::saveFrameToPNG(vk::Image srcImage, uint32_t width, uint32_t height, const std::string& filename) {
    if (!vkContext_) return;
    
    vk::Device device = vkContext_->getDevice();
    vk::Queue queue = vkContext_->getGraphicsQueue();
    vk::CommandPool pool = vkContext_->getCommandPool();
    
    // Создаём staging buffer для копирования данных с GPU
    // Image формат RGBA16F = 8 bytes/pixel (4 components × 2 bytes each)
    vk::DeviceSize imageSize = width * height * 8; // RGBA16F
    
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = imageSize;
    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    vk::Buffer stagingBuffer = device.createBuffer(bufferInfo);
    
    vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(stagingBuffer);
    
    vk::PhysicalDeviceMemoryProperties memProps = vkContext_->getPhysicalDevice().getMemoryProperties();
    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)) {
            memTypeIndex = i;
            break;
        }
    }
    
    if (memTypeIndex == UINT32_MAX) {
        std::cerr << "[HybridFreGSRenderer] Failed to find suitable memory type for staging buffer\n";
        device.destroyBuffer(stagingBuffer);
        return;
    }
    
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memTypeIndex;
    
    vk::DeviceMemory stagingMemory = device.allocateMemory(allocInfo);
    device.bindBufferMemory(stagingBuffer, stagingMemory, 0);
    
    // Создаём командный буфер для копирования
    vk::CommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.commandPool = pool;
    cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdAllocInfo.commandBufferCount = 1;
    
    auto cmdBuffers = device.allocateCommandBuffers(cmdAllocInfo);
    vk::CommandBuffer cmd = cmdBuffers[0];
    
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);
    
    // Transition image to TRANSFER_SRC
    vk::ImageMemoryBarrier barrier1;
    barrier1.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier1.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier1.oldLayout = vk::ImageLayout::eGeneral;
    barrier1.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image = srcImage;
    barrier1.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier1.subresourceRange.baseMipLevel = 0;
    barrier1.subresourceRange.levelCount = 1;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags{},
        0, nullptr,
        0, nullptr,
        1, &barrier1
    );
    
    // Copy image to buffer
    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{width, height, 1};
    
    cmd.copyImageToBuffer(srcImage, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, 1, &region);
    
    // Transition image back to GENERAL
    vk::ImageMemoryBarrier barrier2;
    barrier2.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    barrier2.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier2.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
    barrier2.newLayout = vk::ImageLayout::eGeneral;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.image = srcImage;
    barrier2.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier2.subresourceRange.baseMipLevel = 0;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.baseArrayLayer = 0;
    barrier2.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        0, nullptr,
        1, &barrier2
    );
    
    cmd.end();
    
    // Submit
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    queue.submit(submitInfo, nullptr);
    queue.waitIdle();
    
    // Map и читаем данные
    void* data = device.mapMemory(stagingMemory, 0, imageSize);
    
    // Создаём RGBA8 буфер для PNG (предполагаем что image формат RGBA16F)
    std::vector<uint8_t> rgba8Data(width * height * 4);
    
    // RGBA16F → RGBA8: простейшая конвертация (clamp to [0, 1])
    // Формат RGBA16F: каждый компонент - 16-bit half float
    const uint16_t* rgba16 = static_cast<const uint16_t*>(data);
    
    for (size_t px = 0; px < width * height; ++px) {
        for (int c = 0; c < 4; ++c) {
            uint16_t halfFloat = rgba16[px * 4 + c];
            
            // Простая конвертация half → float → uint8
            // Half float format: sign(1) | exponent(5) | mantissa(10)
            uint32_t sign = (halfFloat >> 15) & 0x1;
            uint32_t exponent = (halfFloat >> 10) & 0x1F;
            uint32_t mantissa = halfFloat & 0x3FF;
            
            float floatVal = 0.0f;
            if (exponent == 0) {
                // Subnormal or zero
                floatVal = (sign ? -1.0f : 1.0f) * std::pow(2.0f, -14.0f) * (mantissa / 1024.0f);
            } else if (exponent == 31) {
                // Infinity or NaN
                floatVal = (sign ? -1.0f : 1.0f) * std::numeric_limits<float>::infinity();
            } else {
                // Normalized
                floatVal = (sign ? -1.0f : 1.0f) * std::pow(2.0f, int(exponent) - 15) * (1.0f + mantissa / 1024.0f);
            }
            
            // Clamp to [0, 1] and convert to uint8
            floatVal = std::min(std::max(floatVal, 0.0f), 1.0f);
            rgba8Data[px * 4 + c] = static_cast<uint8_t>(floatVal * 255.0f);
        }
    }
    
    device.unmapMemory(stagingMemory);
    
    // Сохраняем PNG (временно отключено - библиотека не установлена)
    // int result = stbi_write_png(filename.c_str(), width, height, 4, rgba8Data.data(), width * 4);

    std::cout << "[HybridFreGSRenderer] ✅ Frame ready for save to: " << filename << " (PNG save disabled)" << "\n";

    // Fallback: сохраняем в простой бинарный PPM (P6), чтобы можно было анализировать кадр без PNG-библиотеки
    {
        const std::string ppmName = "triangle_splatting_frame.ppm";
        std::ofstream ppm(ppmName, std::ios::binary);
        if (ppm) {
            // Заголовок PPM
            ppm << "P6\n" << width << " " << height << "\n255\n";
            // Записываем только RGB (пропускаем альфа)
            for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
                ppm.put(static_cast<char>(rgba8Data[i * 4 + 0]));
                ppm.put(static_cast<char>(rgba8Data[i * 4 + 1]));
                ppm.put(static_cast<char>(rgba8Data[i * 4 + 2]));
            }
            std::cout << "[HybridFreGSRenderer] 💾 Frame saved as PPM: " << ppmName << "\n";
        } else {
            std::cerr << "[HybridFreGSRenderer] ❌ Failed to write PPM fallback" << "\n";
        }
    }
    // if (result) {
    //     std::cout << "[HybridFreGSRenderer] ✅ Frame saved to: " << filename << "\n";
    // } else {
    //     std::cerr << "[HybridFreGSRenderer] ❌ Failed to save PNG: " << filename << "\n";
    // }
    
    // Cleanup
    device.freeCommandBuffers(pool, cmdBuffers);
    device.freeMemory(stagingMemory);
    device.destroyBuffer(stagingBuffer);
}

}  // namespace Rendering
}  // namespace SpectraForge


