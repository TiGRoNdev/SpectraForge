/**
 * @file RendererStatistics.cpp
 * @brief Implementation of RendererStatistics (P0.2)
 */

#include "SpectraForge/Rendering/Core/RendererStatistics.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <iostream>

namespace SpectraForge {
namespace Rendering {
namespace Core {

RendererStatistics::RendererStatistics(vk::PhysicalDevice physicalDevice)
    : physicalDevice_(physicalDevice) {
}

void RendererStatistics::updateStats(const RenderingStats& stats) {
    stats_ = stats;
}

DetailedRenderingStats RendererStatistics::getDetailedStats() const {
    DetailedRenderingStats detailed;
    // Base stats from RenderingStats
    detailed.frameTime = stats_.frameTime;
    detailed.fps = stats_.fps;
    detailed.drawCalls = stats_.drawCalls;
    detailed.primitives = stats_.primitives;
    detailed.memoryUsed = stats_.memoryUsed;
    detailed.memoryTotal = stats_.memoryTotal;
    detailed.gaussianCount = stats_.gaussianCount;
    detailed.upscaleFactor = stats_.upscaleFactor;
    
    // Detailed stats (defaults)
    detailed.visibleTriangles = stats_.primitives;
    detailed.culledTriangles = 0;
    detailed.rasterizedPixels = 0;
    detailed.shadedPixels = 0;
    detailed.gpuTime = stats_.frameTime;
    detailed.vertexShaderTime = 0.0f;
    detailed.fragmentShaderTime = 0.0f;
    
    return detailed;
}

GPUInfo RendererStatistics::getGPUInfo() const {
    GPUInfo info;
    
    if (physicalDevice_) {
        auto props = physicalDevice_.getProperties();
        info.deviceName = std::string(props.deviceName.data());
        info.driverVersion = SpectraForge::Core::SAFE_TO_STRING(props.driverVersion);
        info.apiVersion = SpectraForge::Core::SAFE_TO_STRING(VK_API_VERSION_MAJOR(props.apiVersion)) + "." +
                         SpectraForge::Core::SAFE_TO_STRING(VK_API_VERSION_MINOR(props.apiVersion)) + "." +
                         SpectraForge::Core::SAFE_TO_STRING(VK_API_VERSION_PATCH(props.apiVersion));
        
        // Memory
        auto memProps = physicalDevice_.getMemoryProperties();
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                info.totalMemory = memProps.memoryHeaps[i].size;
                break;
            }
        }
        
        // Additional caps (defaults for now)
        info.maxTextureSize = 16384; // Conservative default
        info.maxComputeWorkGroupSize = 1024;
        info.supportsRayTracing = false;
        info.supportsVariableRateShading = false;
        info.supportsMeshShaders = false;
    } else {
        info.deviceName = "Unknown (Mock Device)";
        info.driverVersion = "0.0.0";
        info.apiVersion = "1.3.0";
    }
    
    return info;
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

