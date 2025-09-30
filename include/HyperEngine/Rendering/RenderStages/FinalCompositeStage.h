
#pragma once

#include <memory>
#include "IRenderStage.h"

namespace HyperEngine::Rendering {

/**
 * @brief Этап финального композитинга
 *
 * Применяет Single Responsibility Principle -
 * отвечает ТОЛЬКО за финальное объединение всех эффектов.
 *
 * Комбинирует результаты всех предыдущих этапов в итоговое изображение.
 */
class FinalCompositeStage : public IRenderStage {
  private:
    std::shared_ptr<IResourceManager> resourceManager_;

    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;

    // Статистика
    mutable float lastExecutionTime_ = 0.0f;

    // Настройки композитинга
    struct CompositeSettings {
        bool enableToneMapping = true;   ///< Включить tone mapping
        bool enableColorGrading = true;  ///< Включить color grading
        bool enableBloom = true;         ///< Включить bloom эффект
        bool enableVignette = false;     ///< Включить виньетирование
        bool enableFXAA = true;          ///< Включить FXAA
        bool enableDebugOverlay = false;  ///< Показать отладочную информацию
        float exposure = 1.0f;            ///< Экспозиция
        float gamma = 2.2f;               ///< Гамма коррекция
        float saturation = 1.0f;          ///< Насыщенность
        float contrast = 1.0f;            ///< Контрастность
    } settings_;

  public:
    FinalCompositeStage();
    ~FinalCompositeStage() override;

    // Запретить копирование
    FinalCompositeStage(const FinalCompositeStage&) = delete;
    FinalCompositeStage& operator=(const FinalCompositeStage&) = delete;

    // Разрешить перемещение
    FinalCompositeStage(FinalCompositeStage&&) noexcept;
    FinalCompositeStage& operator=(FinalCompositeStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "FinalComposite"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 5000; }  // Последний этап
    std::vector<std::string> getDependencies() const override { return {"Upscaling"}; }
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;

    // Специфичные методы этапа

    /**
     * @brief Получить настройки композитинга
     * @return Ссылка на настройки
     */
    CompositeSettings& getSettings() { return settings_; }
    const CompositeSettings& getSettings() const { return settings_; }

    /**
     * @brief Применить LUT для color grading
     * @param lutTexture Handle текстуры LUT
     */
    void setColorGradingLUT(TextureHandle lutTexture);

    /**
     * @brief Установить параметры экспозиции
     * @param exposure Новая экспозиция
     */
    void setExposure(float exposure) { settings_.exposure = exposure; }

    /**
     * @brief Включить/выключить HDR tone mapping
     * @param enable Включить tone mapping
     */
    void setToneMappingEnabled(bool enable) { settings_.enableToneMapping = enable; }

  private:
    /**
     * @brief Применить tone mapping
     * @param context Контекст рендеринга
     */
    void applyToneMapping(RenderContext& context);

    /**
     * @brief Применить color grading
     * @param context Контекст рендеринга
     */
    void applyColorGrading(RenderContext& context);

    /**
     * @brief Применить post-processing эффекты
     * @param context Контекст рендеринга
     */
    void applyPostProcessing(RenderContext& context);

    /**
     * @brief Отрисовать отладочную информацию
     * @param context Контекст рендеринга
     */
    void renderDebugOverlay(RenderContext& context);
};

}  // namespace HyperEngine::Rendering
