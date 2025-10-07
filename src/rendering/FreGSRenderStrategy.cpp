/**
 * @file FreGSRenderStrategy.cpp
 * @brief Реализация стратегии рендеринга FreGS (Week 2)
 */

#include "SpectraForge/Rendering/RenderStrategy/FreGSRenderStrategy.h"
#include "SpectraForge/Rendering/RenderStrategy/RenderContextView.h"
#include "SpectraForge/Rendering/RenderStrategy/IPassScheduler.h"
#include "SpectraForge/Core/SafeConsole.h"

namespace SpectraForge::Rendering {

bool FreGSRenderStrategy::initialize(std::shared_ptr<IRenderContextView> context_view) {
    context_view_ = std::move(context_view);
    initialized_ = (context_view_ != nullptr);

    if (!initialized_) {
        SAFE_ERROR("[FreGSRenderStrategy] Ошибка: context_view == nullptr");
        return false;
    }

    uint32_t w = 0, h = 0;
    context_view_->get_framebuffer_size(w, h);
    SAFE_PRINT_LINE(
        std::string("[FreGSRenderStrategy] Инициализация для backend: ") +
        context_view_->backend_name() +
        ", size=" + SpectraForge::Core::SAFE_TO_STRING(w) + "x" + SpectraForge::Core::SAFE_TO_STRING(h)
    );
    return true;
}

void FreGSRenderStrategy::shutdown() {
    prepared_ = false;
    initialized_ = false;
    context_view_.reset();
}

bool FreGSRenderStrategy::prepare_pipelines(IPassScheduler& scheduler) {
    if (!initialized_) return false;

    // Регистрация упрощённых проходов: wavelet -> fregs
    const bool ok_wavelet = scheduler.register_pass(
        "wavelet_decompose", 0, [this]() { wavelet_pass_executed_ = true; });

    const bool dep_ok = scheduler.add_dependency("fregs_splat", "wavelet_decompose");

    const bool ok_fregs = scheduler.register_pass(
        "fregs_splat", 1, [this]() { fregs_pass_executed_ = true; });

    prepared_ = ok_wavelet && ok_fregs && dep_ok;
    return prepared_;
}

bool FreGSRenderStrategy::update_for_frame(uint64_t frame_index) {
    if (!initialized_ || !prepared_) return false;
    last_frame_index_ = frame_index;
    return true;
}

void FreGSRenderStrategy::record_commands() {
    // Week 2: заглушка без конкретных backend-команд
}

void FreGSRenderStrategy::render_frame() {
    // Week 2: заглушка
}

std::string FreGSRenderStrategy::strategy_name() const { return "FreGS"; }

}  // namespace SpectraForge::Rendering


