#pragma once

#include <memory>
#include <vector>
#include "IRenderStage.h"

namespace HyperEngine::Rendering {

/**
 * @brief Этап вторичного ray tracing
 *
 * Применяет Single Responsibility Principle -
 * отвечает ТОЛЬКО за трассировку лучей для эффектов.
 *
 * Обрабатывает отражения, тени, global illumination
 * и ambient occlusion используя OptiX.
 */
class SecondaryRayTracingStage : public IRenderStage {
  private:
    std::unique_ptr<class OptiXRayTracer> rayTracer_;
    std::shared_ptr<IResourceManager> resourceManager_;

    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;

    // Статистика
    mutable float lastExecutionTime_ = 0.0f;
    mutable uint64_t raysTraced_ = 0;
    mutable std::vector<float> executionTimeHistory_;

    // Настройки ray tracing
    struct RayTracingSettings {
        bool enableReflections = true;         ///< Включить отражения
        bool enableShadows = true;             ///< Включить тени
        bool enableGlobalIllumination = true;  ///< Включить GI
        bool enableAmbientOcclusion = true;    ///< Включить AO
        uint32_t maxBounces = 3;  ///< Максимальное количество отскоков
        uint32_t samplesPerPixel = 1;  ///< Количество лучей на пиксель
        float rayTMin = 0.001f;        ///< Минимальное расстояние луча
        float rayTMax = 1000.0f;       ///< Максимальное расстояние луча
        bool enableSER = true;         ///< Shader Execution Reordering
        bool enableAdaptiveSampling = true;  ///< Адаптивный сэмплинг
    } settings_;

  public:
    SecondaryRayTracingStage();
    ~SecondaryRayTracingStage() override;

    // Запретить копирование
    SecondaryRayTracingStage(const SecondaryRayTracingStage&) = delete;
    SecondaryRayTracingStage& operator=(const SecondaryRayTracingStage&) = delete;

    // Разрешить перемещение
    SecondaryRayTracingStage(SecondaryRayTracingStage&&) noexcept;
    SecondaryRayTracingStage& operator=(SecondaryRayTracingStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "SecondaryRayTracing"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 2000; }  // После первичной растеризации
    std::vector<std::string> getDependencies() const override { return {"PrimaryRasterization"}; }
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;

    // Специфичные методы этапа

    /**
     * @brief Загрузить сцену для ray tracing
     * @param sceneData Данные сцены
     * @return true если загрузка прошла успешно
     */
    bool loadScene(const void* sceneData);

    /**
     * @brief Построить acceleration structures
     * @return true если построение прошло успешно
     */
    bool buildAccelerationStructures();

    /**
     * @brief Получить настройки ray tracing
     * @return Ссылка на настройки
     */
    RayTracingSettings& getSettings() { return settings_; }
    const RayTracingSettings& getSettings() const { return settings_; }

    /**
     * @brief Получить количество отслеженных лучей
     * @return Количество лучей
     */
    uint64_t getTracedRayCount() const { return raysTraced_; }

    /**
     * @brief Получить статистику OptiX
     * @return Структура со статистикой
     */
    struct OptiXStats {
        uint64_t totalRays = 0;
        uint64_t hitRays = 0;
        uint64_t missRays = 0;
        float hitRate = 0.0f;
        size_t accelerationStructureMemory = 0;
    };

    OptiXStats getOptiXStats() const;

  private:
    /**
     * @brief Обновление статистики
     * @param executionTime Время выполнения
     * @param rayCount Количество лучей
     */
    void updateStats(float executionTime, uint64_t rayCount);

    /**
     * @brief Трассировка отражений
     * @param context Контекст рендеринга
     */
    void traceReflections(RenderContext& context);

    /**
     * @brief Трассировка теней
     * @param context Контекст рендеринга
     */
    void traceShadows(RenderContext& context);

    /**
     * @brief Трассировка global illumination
     * @param context Контекст рендеринга
     */
    void traceGlobalIllumination(RenderContext& context);

    /**
     * @brief Трассировка ambient occlusion
     * @param context Контекст рендеринга
     */
    void traceAmbientOcclusion(RenderContext& context);
};

}  // namespace HyperEngine::Rendering
