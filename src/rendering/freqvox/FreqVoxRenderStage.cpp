/**
 * @file FreqVoxRenderStage.cpp
 */

#include "SpectraForge/Rendering/FreqVox/FreqVoxRenderStage.h"
#include "SpectraForge/Rendering/FreqVox/BackendFactory.h"
#include <chrono>
#include "SpectraForge/Core/SafeConsole.h"

namespace SpectraForge::Rendering::FreqVox {

FreqVoxRenderStage::FreqVoxRenderStage() = default;
FreqVoxRenderStage::~FreqVoxRenderStage() { shutdown(); }

bool FreqVoxRenderStage::initialize(std::shared_ptr<IResourceManager> resourceManager) {
    resource_manager_ = std::move(resourceManager);

    // Конфигурация DCT блоков (8×8 согласно типичным DCT системам)
    DctBlockConfig dct_config;
    dct_config.blockSize = 8;    // 8×8 блоки
    dct_config.batchCount = 1;   // Будет обновлено при рендеринге

    // Автоматический выбор лучшего доступного бэкенда
    auto backend = BackendFactory::create(BackendFactory::BackendType::Auto);
    if (!backend) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось создать FFT бэкенд");
        return false;
    }
    
    freq_pipeline_ = std::make_unique<FrequencyShadingPipeline>(std::move(backend));
    if (!freq_pipeline_->initialize(dct_config)) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось инициализировать частотный пайплайн");
        return false;
    }

    // Precompute default material BRDF в частотной области
    // Для примера: Lambert diffuse материал (константа 1/π)
    const uint32_t blockElements = dct_config.blockSize * dct_config.blockSize;
    std::vector<float> default_material_spatial(blockElements, 1.0f / 3.14159f);
    default_material_freq_.resize(blockElements);
    
    if (!freq_pipeline_->precompute_material(default_material_spatial, default_material_freq_)) {
        SAFE_ERROR("[FreqVoxRenderStage] Не удалось precompute material");
        return false;
    }
    
    SAFE_PRINT_LINE("[FreqVoxRenderStage] Material BRDF precomputed в частотной области");

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

    // 2) Frequency-Domain Shading (согласно Math.md раздел 2)
    // Для демонстрации: создаем тестовый lighting block (8×8)
    const uint32_t blockSize = 8;
    const uint32_t blockElements = blockSize * blockSize;
    
    // Генерируем synthetic lighting data (например, gradient)
    std::vector<float> lighting_blocks(blockElements);
    for (uint32_t i = 0; i < blockElements; ++i) {
        // Простой gradient: от 0 до 1
        lighting_blocks[i] = static_cast<float>(i) / blockElements;
    }

    // Выполняем frequency-domain shading: S̃ = L̃ ⊙ M̃
    if (freq_pipeline_->shade_blocks(lighting_blocks, default_material_freq_)) {
        // Результат в lighting_blocks теперь содержит shaded result
        // В реальной системе это передается дальше по pipeline
        
        // Для статистики: считаем среднюю яркость
        float avg_brightness = 0.0f;
        for (float val : lighting_blocks) {
            avg_brightness += val;
        }
        avg_brightness /= lighting_blocks.size();
        
        // TODO: вывести в debug context когда появится custom field
        (void)avg_brightness; // suppress unused warning
    }

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

}  // namespace SpectraForge::Rendering::FreqVox


