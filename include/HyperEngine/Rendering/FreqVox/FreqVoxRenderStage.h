/**
 * @file FreqVoxRenderStage.h
 * @brief Этап рендера FreqVox для интеграции в ModernRenderer3D
 */

#pragma once

#include <memory>
#include <vector>
#include "HyperEngine/Rendering/RenderStages/IRenderStage.h"
#include "HyperEngine/Rendering/FreqVox/FreqVoxTypes.h"
#include "HyperEngine/Rendering/FreqVox/FoveatedSelector.h"
#include "HyperEngine/Rendering/FreqVox/FrequencyShading.h"
#include "HyperEngine/Rendering/FreqVox/TemporalReprojection.h"
#include "HyperEngine/Rendering/FreqVox/NeuralUpscaling.h"

namespace HyperEngine::Rendering::FreqVox {

class FreqVoxRenderStage : public IRenderStage {
  public:
    FreqVoxRenderStage();
    ~FreqVoxRenderStage() override;

    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override;
    bool isReady() const override;
    int getPriority() const override;
    std::vector<std::string> getDependencies() const override;
    float getExecutionTime() const override;
    bool supportsFeature(const std::string& feature) const override;

  private:
    std::shared_ptr<IResourceManager> resource_manager_;
    FoveatedSelector selector_;
    std::unique_ptr<FrequencyShadingPipeline> freq_pipeline_;
    std::unique_ptr<TemporalReprojection> temporal_reprojection_;
    std::unique_ptr<NeuralUpscaler> neural_upscaler_;
    
    std::vector<Voxel> voxels_;
    DctBlockConfig block_cfg_{};
    bool initialized_ = false;
    mutable float last_time_ms_ = 0.0f;
};

}  // namespace HyperEngine::Rendering::FreqVox


