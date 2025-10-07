/**
 * @file Upscaler.cpp
 * @brief Base implementation for upscalers
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/Upscaling/Upscaler.h"
#include "SpectraForge/Core/VulkanContext.h"
#include <cmath>

namespace spectraforge {
namespace upscaling {

UpscalerBase::UpscalerBase(std::string name)
    : name_(std::move(name))
{
}

void UpscalerBase::haltonSequence(uint32_t index, uint32_t base, float& out) {
    float result = 0.0f;
    float f = 1.0f / static_cast<float>(base);
    uint32_t i = index;
    
    while (i > 0) {
        result += f * static_cast<float>(i % base);
        i = i / base;
        f = f / static_cast<float>(base);
    }
    
    out = result;
}

// ============================================================================
// NullUpscaler (Deprecated - use NativeUpscaler instead)
// ============================================================================

NullUpscaler::NullUpscaler()
    : UpscalerBase("Null (Deprecated)")
{
}

bool NullUpscaler::initialize(
    const VulkanContext& context,
    const UpscaleConfig& config)
{
    (void)context;
    
    config_ = config;
    initialized_ = true;
    
    return true;
}

void NullUpscaler::execute(
    vk::CommandBuffer cmd,
    const UpscaleResources& resources,
    uint32_t frameIndex,
    float jitterX,
    float jitterY)
{
    // No-op
    (void)cmd;
    (void)resources;
    (void)frameIndex;
    (void)jitterX;
    (void)jitterY;
}

void NullUpscaler::cleanup() {
    initialized_ = false;
}

void NullUpscaler::getJitterOffset(uint32_t frameIndex, float& outX, float& outY) const {
    (void)frameIndex;
    outX = 0.0f;
    outY = 0.0f;
}

bool NullUpscaler::resize(
    uint32_t newInputWidth,
    uint32_t newInputHeight,
    uint32_t newOutputWidth,
    uint32_t newOutputHeight)
{
    (void)newInputWidth;
    (void)newInputHeight;
    (void)newOutputWidth;
    (void)newOutputHeight;
    return true;
}

} // namespace upscaling
} // namespace spectraforge
