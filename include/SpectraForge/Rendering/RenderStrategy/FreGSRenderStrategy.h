/**
 * @file FreGSRenderStrategy.h
 * @brief Стратегия рендеринга для Frequency-Encoded Gaussian Splatting (FreGS)
 *
 * Следует контракту IRenderStrategy: инициализация, подготовка проходов,
 * обновление на кадр, запись команд и выполнение кадра. На этапе Фазы 1/Неделя 2
 * реализация носит минимальный характер и готовит основу для переноса
 * Gaussian-специфичной логики из существующих путей (FreGSPass/WaveletPass).
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "SpectraForge/Rendering/IRenderStrategy.h"

namespace SpectraForge::Rendering {

/**
 * @brief Реализация стратегии рендеринга FreGS
 *
 * Обеспечивает регистрацию необходимых проходов и базовый жизненный цикл.
 * В будущих неделях будет расширена интеграцией с VulkanRenderer и реальными
 * рендер-пассами (WaveletPass, FreGSPass).
 */
class FreGSRenderStrategy final : public IRenderStrategy {
  public:
    /**
     * @brief Инициализация стратегии FreGS
     * @param context_view Read-only доступ к контексту
     * @return true если успешно
     */
    bool initialize(std::shared_ptr<IRenderContextView> context_view) override;

    /**
     * @brief Освобождение ресурсов
     */
    void shutdown() override;

    /**
     * @brief Регистрация проходов (волновое разложение + FreGS)
     * @param scheduler Планировщик проходов
     */
    bool prepare_pipelines(IPassScheduler& scheduler) override;

    /**
     * @brief Обновление состояния на кадр
     */
    bool update_for_frame(uint64_t frame_index) override;

    /**
     * @brief Запись команд рендеринга (placeholder на Week 2)
     */
    void record_commands() override;

    /**
     * @brief Выполнение кадра (placeholder на Week 2)
     */
    void render_frame() override;

    /**
     * @brief Идентификатор стратегии
     */
    std::string strategy_name() const override;

  private:
    std::shared_ptr<IRenderContextView> context_view_;
    bool initialized_ = false;
    bool prepared_ = false;
    uint64_t last_frame_index_ = 0;

    // Диагностические признаки выполнения проходов на Week 2
    bool wavelet_pass_executed_ = false;
    bool fregs_pass_executed_ = false;
};

}  // namespace SpectraForge::Rendering


