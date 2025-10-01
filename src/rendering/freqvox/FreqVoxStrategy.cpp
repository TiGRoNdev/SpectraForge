/**
 * @file FreqVoxStrategy.cpp
 */

#include "HyperEngine/Rendering/FreqVox/FreqVoxStrategy.h"
#include "HyperEngine/Core/SafeConsole.h"

namespace HyperEngine::Rendering::FreqVox {

FreqVoxStrategy::FreqVoxStrategy() = default;
FreqVoxStrategy::~FreqVoxStrategy() { shutdown(); }

bool FreqVoxStrategy::initialize() {
    // Получение ResourceManager из контекста рендерера обычно через DI; заглушка
    stage_ = std::make_unique<FreqVoxRenderStage>();
    initialized_ = true;
    return true;
}

void FreqVoxStrategy::render(const FrameData& frameData) {
    (void)frameData;  // заглушка
    if (!initialized_) return;

    // Создаём временный RenderContext; в реальном коде он должен приходить из рендерера
    RenderContext ctx{};
    if (stage_) {
        if (!stage_->isReady()) {
            stage_->initialize(resource_manager_);
        }
        stage_->execute(ctx);
    }
}

void FreqVoxStrategy::shutdown() {
    if (stage_) {
        stage_->shutdown();
        stage_.reset();
    }
    initialized_ = false;
}

std::string FreqVoxStrategy::getName() const { return "FreqVoxStrategy"; }

bool FreqVoxStrategy::supportsFeature(RenderingFeature feature) const {
    (void)feature;
    return true;
}

}  // namespace HyperEngine::Rendering::FreqVox


