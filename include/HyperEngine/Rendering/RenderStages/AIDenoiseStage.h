#pragma once

#include <memory>
#include "IRenderStage.h"

namespace HyperEngine::Rendering {

/**
 * @brief Этап AI деноизинга
 *
 * Применяет Single Responsibility Principle -
 * отвечает ТОЛЬКО за деноизинг эффектов.
 *
 * Использует OptiX AI Denoiser для очистки ray-traced эффектов.
 */
class AIDenoiseStage : public IRenderStage {
  private:
    std::unique_ptr<class DenoiseModule> denoiser_;
    std::shared_ptr<IResourceManager> resourceManager_;

    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;

    // Статистика
    mutable float lastExecutionTime_ = 0.0f;

    // Настройки деноизинга
    struct DenoiseSettings {
        float temporalAlpha = 0.95f;  ///< Temporal accumulation factor
        bool enableSpatialFilter = true;  ///< Включить пространственную фильтрацию
        bool enableTemporalFilter = true;  ///< Включить временную фильтрацию
        uint32_t spatialIterations = 3;  ///< Итерации пространственной фильтрации
        float varianceThreshold = 0.1f;  ///< Порог variance для адаптивности
        bool enableAOV = true;           ///< Использовать auxiliary buffers
    } settings_;

  public:
    AIDenoiseStage();
    ~AIDenoiseStage() override;

    // Запретить копирование
    AIDenoiseStage(const AIDenoiseStage&) = delete;
    AIDenoiseStage& operator=(const AIDenoiseStage&) = delete;

    // Разрешить перемещение
    AIDenoiseStage(AIDenoiseStage&&) noexcept;
    AIDenoiseStage& operator=(AIDenoiseStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "AIDenoising"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 3000; }  // После ray tracing
    std::vector<std::string> getDependencies() const override { return {"SecondaryRayTracing"}; }
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;

    // Специфичные методы этапа

    /**
     * @brief Получить настройки деноизинга
     * @return Ссылка на настройки
     */
    DenoiseSettings& getSettings() { return settings_; }
    const DenoiseSettings& getSettings() const { return settings_; }

    /**
     * @brief Сбросить temporal данные
     */
    void resetTemporalData();

  private:
    /**
     * @brief Деноизинг отражений
     * @param context Контекст рендеринга
     */
    void denoiseReflections(RenderContext& context);

    /**
     * @brief Деноизинг теней
     * @param context Контекст рендеринга
     */
    void denoiseShadows(RenderContext& context);

    /**
     * @brief Деноизинг global illumination
     * @param context Контекст рендеринга
     */
    void denoiseGlobalIllumination(RenderContext& context);
};

}  // namespace HyperEngine::Rendering
