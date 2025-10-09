/**
 * @file RendererStatistics.h
 * @brief Statistics collection and reporting (P0.2 Refactoring)
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include "SpectraForge/Rendering/Common/IRenderer.h"

namespace SpectraForge {
namespace Rendering {
namespace Core {

/**
 * @brief Сбор и агрегация статистики рендеринга
 * 
 * SOLID:
 * - SRP ✅: Только статистика и метрики
 */
class RendererStatistics {
public:
    explicit RendererStatistics(vk::PhysicalDevice physicalDevice);
    ~RendererStatistics() = default;
    
    void updateStats(const RenderingStats& stats);
    
    RenderingStats getStats() const { return stats_; }
    DetailedRenderingStats getDetailedStats() const;
    GPUInfo getGPUInfo() const;
    
    void setTriangleBudget(uint32_t maxTriangles) { triangleBudget_ = maxTriangles; }
    uint32_t getTriangleBudget() const { return triangleBudget_; }

private:
    vk::PhysicalDevice physicalDevice_;
    RenderingStats stats_{};
    uint32_t triangleBudget_ = 100000;
};

}}}

