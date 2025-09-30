
#pragma once

#include <memory>
#include "HyperEngine/Rendering/Upscaling/IUpscalingStrategy.h"
#include "IRenderStage.h"

namespace HyperEngine::Rendering {

/**
 * @brief Этап upscaling
 *
 * Применяет Single Responsibility Principle -
 * отвечает ТОЛЬКО за увеличение разрешения.
 *
 * Использует Strategy Pattern для переключения между
 * различными алгоритмами upscaling (DLSS, FSR, etc.)
 */
class UpscalingStage : public IRenderStage {
  private:
    std::unique_ptr<Upscaling::IUpscalingStrategy> strategy_;
    std::shared_ptr<IResourceManager> resourceManager_;

    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;

    // Статистика
    mutable float lastExecutionTime_ = 0.0f;
    mutable Upscaling::UpscalingStats upscalingStats_;

    // Настройки
    Upscaling::UpscalingConfig config_;
    Upscaling::UpscalingType preferredType_ = Upscaling::UpscalingType::DLSS3;

    // Fallback стратегии
    std::vector<Upscaling::UpscalingType> fallbackTypes_ = {Upscaling::UpscalingType::DLSS3,
                                                            Upscaling::UpscalingType::DLSS2,
                                                            Upscaling::UpscalingType::FSR3,
                                                            Upscaling::UpscalingType::FSR2,
                                                            Upscaling::UpscalingType::Bicubic};

  public:
    UpscalingStage();
    ~UpscalingStage() override;

    // Запретить копирование
    UpscalingStage(const UpscalingStage&) = delete;
    UpscalingStage& operator=(const UpscalingStage&) = delete;

    // Разрешить перемещение
    UpscalingStage(UpscalingStage&&) noexcept;
    UpscalingStage& operator=(UpscalingStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "Upscaling"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 4000; }  // После деноизинга
    std::vector<std::string> getDependencies() const override { return {"AIDenoising"}; }
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;

    // Специфичные методы этапа

    /**
     * @brief Установить стратегию upscaling
     * @param strategy Новая стратегия
     * @return true если стратегия установлена успешно
     */
    bool setStrategy(std::unique_ptr<Upscaling::IUpscalingStrategy> strategy);

    /**
     * @brief Установить тип upscaling (создаст стратегию автоматически)
     * @param type Тип upscaling
     * @return true если тип установлен успешно
     */
    bool setUpscalingType(Upscaling::UpscalingType type);

    /**
     * @brief Получить текущую стратегию
     * @return Указатель на стратегию или nullptr
     */
    Upscaling::IUpscalingStrategy* getStrategy() const { return strategy_.get(); }

    /**
     * @brief Получить текущий тип upscaling
     * @return Тип upscaling или None если стратегия не установлена
     */
    Upscaling::UpscalingType getCurrentType() const;

    /**
     * @brief Установить конфигурацию upscaling
     * @param config Новая конфигурация
     * @return true если конфигурация применена успешно
     */
    bool setConfig(const Upscaling::UpscalingConfig& config);

    /**
     * @brief Получить текущую конфигурацию
     * @return Копия текущей конфигурации
     */
    Upscaling::UpscalingConfig getConfig() const { return config_; }

    /**
     * @brief Установить предпочитаемый тип upscaling
     * @param type Предпочитаемый тип
     */
    void setPreferredType(Upscaling::UpscalingType type) { preferredType_ = type; }

    /**
     * @brief Получить предпочитаемый тип
     * @return Предпочитаемый тип upscaling
     */
    Upscaling::UpscalingType getPreferredType() const { return preferredType_; }

    /**
     * @brief Установить список fallback типов
     * @param types Список типов в порядке приоритета
     */
    void setFallbackTypes(const std::vector<Upscaling::UpscalingType>& types) {
        fallbackTypes_ = types;
    }

    /**
     * @brief Получить список fallback типов
     * @return Вектор типов
     */
    const std::vector<Upscaling::UpscalingType>& getFallbackTypes() const { return fallbackTypes_; }

    /**
     * @brief Получить статистику upscaling
     * @return Статистика производительности
     */
    Upscaling::UpscalingStats getUpscalingStats() const { return upscalingStats_; }

    /**
     * @brief Получить список поддерживаемых типов на текущем оборудовании
     * @return Вектор поддерживаемых типов
     */
    std::vector<Upscaling::UpscalingType> getSupportedTypes() const;

    /**
     * @brief Автоматически выбрать лучший доступный upscaling
     * @param preferPerformance Предпочесть производительность качеству
     * @return true если выбор прошел успешно
     */
    bool autoSelectBestUpscaling(bool preferPerformance = true);

    /**
     * @brief Проверить, включен ли upscaling
     * @return true если upscaling активен
     */
    bool isUpscalingEnabled() const;

    /**
     * @brief Включить/выключить upscaling
     * @param enable Включить upscaling
     */
    void setUpscalingEnabled(bool enable);

    /**
     * @brief Получить фактический коэффициент upscaling
     * @return Коэффициент увеличения
     */
    float getActualUpscaleFactor() const;

    /**
     * @brief Получить рекомендованное разрешение рендеринга
     * @param targetWidth Целевая ширина
     * @param targetHeight Целевая высота
     * @return Пара (ширина, высота) для рендеринга
     */
    std::pair<int, int> getRecommendedRenderResolution(int targetWidth, int targetHeight) const;

    /**
     * @brief Сбросить внутреннее состояние upscaling
     * Полезно при изменении сцены или камеры
     */
    void resetUpscalingState();

  private:
    /**
     * @brief Создать стратегию по типу
     * @param type Тип upscaling
     * @return Умный указатель на стратегию или nullptr
     */
    std::unique_ptr<Upscaling::IUpscalingStrategy> createStrategy(Upscaling::UpscalingType type);

    /**
     * @brief Попытаться инициализировать стратегию с fallback
     * @return true если какая-то стратегия была инициализирована
     */
    bool initializeWithFallback();

    /**
     * @brief Подготовить параметры upscaling
     * @param context Контекст рендеринга
     * @return Параметры для upscaling
     */
    Upscaling::UpscalingParams prepareUpscalingParams(const RenderContext& context);

    /**
     * @brief Валидация входных данных
     * @param context Контекст рендеринга
     * @return true если данные валидны
     */
    bool validateInputs(const RenderContext& context) const;

    /**
     * @brief Обновление статистики
     * @param result Результат upscaling
     */
    void updateStats(const Upscaling::UpscalingResult& result);

    /**
     * @brief Логирование переключения стратегии
     * @param oldType Старый тип
     * @param newType Новый тип
     */
    void logStrategySwitch(Upscaling::UpscalingType oldType,
                           Upscaling::UpscalingType newType) const;

    /**
     * @brief Проверка совместимости стратегии с текущей конфигурацией
     * @param strategy Стратегия для проверки
     * @return true если совместима
     */
    bool isStrategyCompatible(const Upscaling::IUpscalingStrategy* strategy) const;
};

}  // namespace HyperEngine::Rendering
