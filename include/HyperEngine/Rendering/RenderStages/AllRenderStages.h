
#pragma once

/**
 * @brief Удобный заголовочный файл для включения всех этапов рендеринга
 *
 * Этот файл включает все базовые этапы рендеринга HyperEngine.
 * Используйте его когда нужно работать с полным pipeline рендеринга.
 */

// Базовый интерфейс
#include "IRenderStage.h"

// Все этапы рендеринга
#include "AIDenoiseStage.h"
#include "FinalCompositeStage.h"
#include "PrimaryRasterizationStage.h"
#include "SecondaryRayTracingStage.h"
#include "UpscalingStage.h"

namespace HyperEngine::Rendering {

/**
 * @brief Перечисление всех стандартных этапов рендеринга
 */
enum class StandardRenderStage {
    PrimaryRasterization,  ///< Первичная растеризация (Gaussian Splatting)
    SecondaryRayTracing,  ///< Вторичная трассировка лучей (отражения, тени, GI, AO)
    AIDenoising,    ///< AI деноизинг (OptiX Denoiser)
    Upscaling,      ///< Upscaling (DLSS/FSR/Basic)
    FinalComposite  ///< Финальный композитинг (tone mapping, post-effects)
};

/**
 * @brief Фабрика для создания стандартных этапов рендеринга
 */
class StandardRenderStageFactory {
  public:
    /**
     * @brief Создать стандартный этап рендеринга
     * @param stage Тип этапа
     * @return Умный указатель на созданный этап
     */
    static std::unique_ptr<IRenderStage> createStage(StandardRenderStage stage);

    /**
     * @brief Создать полный стандартный pipeline
     * @return Вектор всех стандартных этапов в правильном порядке
     */
    static std::vector<std::unique_ptr<IRenderStage>> createFullPipeline();

    /**
     * @brief Создать минимальный pipeline (без ray tracing и upscaling)
     * @return Вектор базовых этапов
     */
    static std::vector<std::unique_ptr<IRenderStage>> createMinimalPipeline();

    /**
     * @brief Создать pipeline для производительности (с upscaling, без ray tracing)
     * @return Вектор этапов для максимальной производительности
     */
    static std::vector<std::unique_ptr<IRenderStage>> createPerformancePipeline();

    /**
     * @brief Создать pipeline для качества (все этапы с максимальными настройками)
     * @return Вектор этапов для максимального качества
     */
    static std::vector<std::unique_ptr<IRenderStage>> createQualityPipeline();

    /**
     * @brief Получить название этапа
     * @param stage Тип этапа
     * @return Человекочитаемое название
     */
    static std::string getStageName(StandardRenderStage stage);

    /**
     * @brief Получить описание этапа
     * @param stage Тип этапа
     * @return Описание функциональности этапа
     */
    static std::string getStageDescription(StandardRenderStage stage);

  private:
    StandardRenderStageFactory() = default;  // Запретить создание экземпляров
};

}  // namespace HyperEngine::Rendering
