/**
 * @file FreqVoxStrategy.h
 * @brief Стратегия рендеринга для FreqVox (AFS-NVR) на базе IRenderStrategy
 */

#pragma once

#include <memory>
#include <string>
#include "HyperEngine/Rendering/ModernRenderer3D.h"
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"
#include "HyperEngine/Rendering/FreqVox/FreqVoxRenderStage.h"

namespace HyperEngine::Rendering::FreqVox {

class FreqVoxStrategy : public IRenderStrategy {
  public:
    FreqVoxStrategy();
    ~FreqVoxStrategy() override;

    bool initialize() override;
    void render(const FrameData& frameData) override;
    void shutdown() override;
    std::string getName() const override;
    bool supportsFeature(RenderingFeature feature) const override;

  private:
    std::unique_ptr<IRenderStage> stage_;
    std::shared_ptr<IResourceManager> resource_manager_;
    bool initialized_ = false;
};

}  // namespace HyperEngine::Rendering::FreqVox


