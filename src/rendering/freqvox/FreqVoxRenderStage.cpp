/**
 * @file FreqVoxRenderStage.cpp
 */

#include "HyperEngine/Rendering/FreqVox/FreqVoxRenderStage.h"
#include "HyperEngine/Rendering/FreqVox/BackendFactory.h"
#include <chrono>
#include "HyperEngine/Core/SafeConsole.h"

namespace HyperEngine::Rendering::FreqVox {

FreqVoxRenderStage::FreqVoxRenderStage() = default;
FreqVoxRenderStage::~FreqVoxRenderStage() { shutdown(); }

bool FreqVoxRenderStage::initialize(std::shared_ptr<IResourceManager> resourceManager) {
    resource_manager_ = std::move(resourceManager);

    block_cfg_.width = 8;
    block_cfg_.height = 8;
    block_cfg_.batch_size = 0;  // будет установлен при подготовке данных

    // Автоматический выбор лучшего доступного бэкенда
    auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    if (!backend) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось создать FFT бэкенд");
        return false;
    }
    
    freq_pipeline_ = std::make_unique<FrequencyShadingPipeline>(std::move(backend));
    if (!freq_pipeline_->initialize(block_cfg_)) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось инициализировать частотный пайплайн");
        return false;
    }

    // Инициализация temporal reprojection (базовое разрешение 1920x1080)
    temporal_reprojection_ = std::make_unique<TemporalReprojection>();
    TemporalReprojectionParams temp_params;
    temp_params.blendFactor = 0.2f;
    temp_params.motionVectorThreshold = 2.0f;
    temp_params.depthChangeThreshold = 0.01f;
    if (!temporal_reprojection_->initialize(1920, 1080, temp_params)) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось инициализировать temporal reprojection");
        return false;
    }

    // Инициализация neural upscaler (upscale factor 2.0)
    neural_upscaler_ = std::make_unique<NeuralUpscaler>();
    NeuralUpscalingParams upscale_params;
    upscale_params.upscaleFactor = 2.0f;
    if (!neural_upscaler_->initialize(UpscalerType::Auto, 960, 540, upscale_params)) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось инициализировать neural upscaler");
        return false;
    }

    initialized_ = true;
    SAFE_PRINT_LINE("[FreqVoxRenderStage] Полностью инициализирован (FFT + Temporal + Upscaling)");
    return true;
}

void FreqVoxRenderStage::execute(RenderContext& context) {
    if (!initialized_) return;

    const auto t0 = std::chrono::high_resolution_clock::now();

    // 1) Выбор вокселей (пока без камеры)
    std::vector<Voxel> selected;
    float v_eff = 0.0f;
    selector_.select(voxels_, FoveatedParams{}, selected, v_eff);

    // 2) Частотное шейдинг ядро: заглушка без реального бэкенда
    // В дальнейшем сюда подадим блоки данных и вызовем freq_pipeline_.

    const auto t1 = std::chrono::high_resolution_clock::now();
    last_time_ms_ = std::chrono::duration<float, std::milli>(t1 - t0).count();
    context.debug.stageTimes[getName()] = last_time_ms_;
}

void FreqVoxRenderStage::shutdown() {
    if (freq_pipeline_) {
        freq_pipeline_->shutdown();
        freq_pipeline_.reset();
    }
    if (temporal_reprojection_) {
        temporal_reprojection_->shutdown();
        temporal_reprojection_.reset();
    }
    if (neural_upscaler_) {
        neural_upscaler_->shutdown();
        neural_upscaler_.reset();
    }
    voxels_.clear();
    initialized_ = false;
    SAFE_PRINT_LINE("[FreqVoxRenderStage] Shutdown завершен");
}

std::string FreqVoxRenderStage::getName() const { return "FreqVoxRenderStage"; }
bool FreqVoxRenderStage::isReady() const { return initialized_; }
int FreqVoxRenderStage::getPriority() const { return 50; }
std::vector<std::string> FreqVoxRenderStage::getDependencies() const { return {}; }
float FreqVoxRenderStage::getExecutionTime() const { return last_time_ms_; }
bool FreqVoxRenderStage::supportsFeature(const std::string& feature) const {
    (void)feature;
    return true;
}

}  // namespace HyperEngine::Rendering::FreqVox


