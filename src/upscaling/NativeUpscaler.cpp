/**
 * @file NativeUpscaler.cpp
 * @brief Implementation of native pass-through upscaler
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/Upscaling/NativeUpscaler.h"
#include "SpectraForge/Core/VulkanContext.h"
#include <iostream>

namespace spectraforge {
namespace upscaling {

NativeUpscaler::NativeUpscaler()
    : UpscalerBase("Native (Pass-Through)")
{
}

bool NativeUpscaler::initialize(
    const VulkanContext& context,
    const UpscaleConfig& config)
{
    device_ = context.getDevice();
    config_ = config;
    
    // Check if we need blitting (resolutions differ)
    needsBlit_ = (config.inputWidth != config.outputWidth) ||
                 (config.inputHeight != config.outputHeight);
    
    initialized_ = true;
    
    std::cout << "NativeUpscaler initialized\n";
    std::cout << "  Input: " << config.inputWidth << "x" << config.inputHeight << "\n";
    std::cout << "  Output: " << config.outputWidth << "x" << config.outputHeight << "\n";
    std::cout << "  Blit required: " << (needsBlit_ ? "YES" : "NO") << "\n";
    
    return true;
}

void NativeUpscaler::execute(
    vk::CommandBuffer cmd,
    const UpscaleResources& resources,
    uint32_t frameIndex,
    float jitterX,
    float jitterY)
{
    if (!initialized_) {
        std::cerr << "NativeUpscaler::execute() called before initialize()\n";
        return;
    }
    
    (void)frameIndex;
    (void)jitterX;
    (void)jitterY;
    
    // If resolutions match, assume input is already in output image
    // (or application handles copy externally)
    if (!needsBlit_) {
        return;
    }
    
    // Otherwise, perform blit from input to output
    blitImage(
        cmd,
        resources.inputColor,
        resources.outputColor,
        config_.inputWidth,
        config_.inputHeight,
        config_.outputWidth,
        config_.outputHeight
    );
}

void NativeUpscaler::cleanup() {
    initialized_ = false;
    device_ = nullptr;
    
    std::cout << "NativeUpscaler cleaned up\n";
}

void NativeUpscaler::getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const {
    // No jitter for native rendering
    (void)frameIndex;
    outX = 0.0f;
    outY = 0.0f;
}

bool NativeUpscaler::resize(
    uint32_t newInputWidth,
    uint32_t newInputHeight,
    uint32_t newOutputWidth,
    uint32_t newOutputHeight)
{
    config_.inputWidth = newInputWidth;
    config_.inputHeight = newInputHeight;
    config_.outputWidth = newOutputWidth;
    config_.outputHeight = newOutputHeight;
    
    // Update blit requirement
    needsBlit_ = (newInputWidth != newOutputWidth) ||
                 (newInputHeight != newOutputHeight);
    
    std::cout << "NativeUpscaler resized to " << newInputWidth << "x" << newInputHeight
              << " → " << newOutputWidth << "x" << newOutputHeight
              << " (blit: " << (needsBlit_ ? "YES" : "NO") << ")\n";
    
    return true;
}

void NativeUpscaler::blitImage(
    vk::CommandBuffer cmd,
    vk::Image src,
    vk::Image dst,
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t dstWidth,
    uint32_t dstHeight)
{
    // Transition src to TRANSFER_SRC_OPTIMAL
    vk::ImageMemoryBarrier srcBarrier;
    srcBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    srcBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    srcBarrier.oldLayout = vk::ImageLayout::eGeneral;
    srcBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    srcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    srcBarrier.image = src;
    srcBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    srcBarrier.subresourceRange.baseMipLevel = 0;
    srcBarrier.subresourceRange.levelCount = 1;
    srcBarrier.subresourceRange.baseArrayLayer = 0;
    srcBarrier.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags{},
        0, nullptr,
        0, nullptr,
        1, &srcBarrier
    );
    
    // Transition dst to TRANSFER_DST_OPTIMAL
    vk::ImageMemoryBarrier dstBarrier;
    dstBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
    dstBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    dstBarrier.oldLayout = vk::ImageLayout::eUndefined;
    dstBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    dstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    dstBarrier.image = dst;
    dstBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    dstBarrier.subresourceRange.baseMipLevel = 0;
    dstBarrier.subresourceRange.levelCount = 1;
    dstBarrier.subresourceRange.baseArrayLayer = 0;
    dstBarrier.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags{},
        0, nullptr,
        0, nullptr,
        1, &dstBarrier
    );
    
    // Perform blit (with linear filter if upscaling)
    vk::ImageBlit blitRegion;
    blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcOffsets[0] = vk::Offset3D{0, 0, 0};
    blitRegion.srcOffsets[1] = vk::Offset3D{static_cast<int32_t>(srcWidth),
                                            static_cast<int32_t>(srcHeight), 1};
    
    blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstOffsets[0] = vk::Offset3D{0, 0, 0};
    blitRegion.dstOffsets[1] = vk::Offset3D{static_cast<int32_t>(dstWidth),
                                            static_cast<int32_t>(dstHeight), 1};
    
    cmd.blitImage(
        src, vk::ImageLayout::eTransferSrcOptimal,
        dst, vk::ImageLayout::eTransferDstOptimal,
        1, &blitRegion,
        vk::Filter::eLinear  // Bilinear filter for upscaling
    );
    
    // Transition dst to GENERAL for further use
    vk::ImageMemoryBarrier finalBarrier;
    finalBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    finalBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    finalBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    finalBarrier.newLayout = vk::ImageLayout::eGeneral;
    finalBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    finalBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    finalBarrier.image = dst;
    finalBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    finalBarrier.subresourceRange.baseMipLevel = 0;
    finalBarrier.subresourceRange.levelCount = 1;
    finalBarrier.subresourceRange.baseArrayLayer = 0;
    finalBarrier.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags{},
        0, nullptr,
        0, nullptr,
        1, &finalBarrier
    );
}

} // namespace upscaling
} // namespace spectraforge

