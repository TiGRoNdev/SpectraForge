#pragma once

#include "IRenderStage.h"
#include "HyperEngine/Rendering/Upscaling/IUpscalingStrategy.h"
#include <memory>

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
    std::vector<Upscaling::UpscalingType> fallbackTypes_ = {
        Upscaling::UpscalingType::DLSS3,
        Upscaling::UpscalingType::DLSS2,
        Upscaling::UpscalingType::FSR3,
        Upscaling::UpscalingType::FSR2,
        Upscaling::UpscalingType::Bicubic
    };

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
    int getPriority() const override { return 4000; } // После деноизинга
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
    const std::vector<Upscaling::UpscalingType>& getFallbackTypes() const { 
        return fallbackTypes_; 
    }
    
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
    void logStrategySwitch(Upscaling::UpscalingType oldType, Upscaling::UpscalingType newType) const;
    
    /**
     * @brief Проверка совместимости стратегии с текущей конфигурацией
     * @param strategy Стратегия для проверки
     * @return true если совместима
     */
    bool isStrategyCompatible(const Upscaling::IUpscalingStrategy* strategy) const;
};

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
        bool enableToneMapping = true;      ///< Включить tone mapping
        bool enableColorGrading = true;     ///< Включить color grading
        bool enableBloom = true;            ///< Включить bloom эффект
        bool enableVignette = false;        ///< Включить виньетирование
        bool enableFXAA = true;             ///< Включить FXAA
        bool enableDebugOverlay = false;    ///< Показать отладочную информацию
        float exposure = 1.0f;              ///< Экспозиция
        float gamma = 2.2f;                 ///< Гамма коррекция
        float saturation = 1.0f;            ///< Насыщенность
        float contrast = 1.0f;              ///< Контрастность
    } settings_;

public:
    FinalCompositeStage();
    ~FinalCompositeStage() override;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "FinalComposite"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 5000; } // Последний этап
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

} // namespace HyperEngine::Rendering
