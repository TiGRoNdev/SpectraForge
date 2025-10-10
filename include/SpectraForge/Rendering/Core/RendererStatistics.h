/**
 * @file RendererStatistics.h
 * @brief Statistics collection and reporting (P0.2 Refactoring)
 */

#pragma once

#include <memory>
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
class IRendererStatistics {
  public:
    virtual ~IRendererStatistics() = default;

    virtual void updateStats(const RenderingStats& stats) = 0;
    virtual RenderingStats getStats() const = 0;
    virtual DetailedRenderingStats getDetailedStats() const = 0;
    virtual GPUInfo getGPUInfo() const = 0;
    virtual void setTriangleBudget(uint32_t maxTriangles) = 0;
    virtual uint32_t getTriangleBudget() const = 0;
};

class IRendererStatisticsFactory {
  public:
    virtual ~IRendererStatisticsFactory() = default;
    virtual std::shared_ptr<IRendererStatistics> create(vk::PhysicalDevice physicalDevice) = 0;
};

class RendererStatistics : public IRendererStatistics {
public:
    explicit RendererStatistics(vk::PhysicalDevice physicalDevice);
    ~RendererStatistics() = default;

    void updateStats(const RenderingStats& stats) override;

    RenderingStats getStats() const override { return stats_; }
    DetailedRenderingStats getDetailedStats() const override;
    GPUInfo getGPUInfo() const override;

    void setTriangleBudget(uint32_t maxTriangles) override { triangleBudget_ = maxTriangles; }
    uint32_t getTriangleBudget() const override { return triangleBudget_; }

private:
    vk::PhysicalDevice physicalDevice_;
    RenderingStats stats_{};
    uint32_t triangleBudget_ = 100000;
};

class RendererStatisticsFactory : public IRendererStatisticsFactory {
  public:
    std::shared_ptr<IRendererStatistics> create(vk::PhysicalDevice physicalDevice) override {
        return std::make_shared<RendererStatistics>(physicalDevice);
    }
};

}}}

