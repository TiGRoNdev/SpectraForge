#pragma once

#include <memory>
#include "IUpscalingStrategy.h"

namespace HyperEngine::Rendering::Upscaling {

/**
 * @brief NVIDIA DLSS реализация стратегии upscaling
 *
 * Конкретная реализация Strategy Pattern для NVIDIA DLSS.
 * Поддерживает DLSS 2.0 и 3.0 с Frame Generation.
 */
class DLSSUpscalingStrategy : public IUpscalingStrategy {
  private:
    struct Impl;  // PIMPL для скрытия DLSS SDK деталей
    std::unique_ptr<Impl> pImpl_;

    UpscalingConfig config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    bool initialized_ = false;
    mutable UpscalingStats stats_;

  public:
    DLSSUpscalingStrategy();
    ~DLSSUpscalingStrategy() override;

    // Запретить копирование
    DLSSUpscalingStrategy(const DLSSUpscalingStrategy&) = delete;
    DLSSUpscalingStrategy& operator=(const DLSSUpscalingStrategy&) = delete;

    // Разрешить перемещение
    DLSSUpscalingStrategy(DLSSUpscalingStrategy&&) noexcept;
    DLSSUpscalingStrategy& operator=(DLSSUpscalingStrategy&&) noexcept;

    // Реализация IUpscalingStrategy
    bool initialize(const UpscalingConfig& config,
                    std::shared_ptr<IResourceManager> resourceManager) override;

    UpscalingResult upscale(const UpscalingParams& params) override;

    void shutdown() override;

    UpscalingType getType() const override {
        return UpscalingType::DLSS3;  // Автоматически выбираем лучшую доступную версию
    }

    std::string getDisplayName() const override;

    bool isSupported() const override;

    std::vector<UpscalingQuality> getSupportedQualities() const override;

    std::pair<int, int> getRecommendedRenderResolution(int targetWidth,
                                                       int targetHeight,
                                                       UpscalingQuality quality) const override;

    bool supportsFeature(UpscalingFeature feature) const override;

    size_t getMemoryUsage() const override;

    UpscalingStats getStats() const override { return stats_; }

    void reset() override;

    // DLSS специфичные методы

    /**
     * @brief Проверить поддержку DLSS 3.0 Frame Generation
     * @return true если DLSS 3.0 поддерживается
     */
    bool supportsDLSS3() const;

    /**
     * @brief Получить версию DLSS SDK
     * @return Строка с версией DLSS
     */
    std::string getDLSSVersion() const;

    /**
     * @brief Настроить автоматическую экспозицию
     * @param enable Включить автоэкспозицию
     */
    void setAutoExposure(bool enable);

    /**
     * @brief Настроить интеграцию с NVIDIA Reflex
     * @param enable Включить Reflex
     */
    void setReflexIntegration(bool enable);

    /**
     * @brief Получить рекомендованные настройки jitter для TAA
     * @param frameIndex Индекс кадра
     * @return Offset для jittering
     */
    std::pair<float, float> getRecommendedJitter(uint64_t frameIndex) const;

  private:
    /**
     * @brief Инициализация DLSS SDK
     * @return true если успешно
     */
    bool initializeDLSS();

    /**
     * @brief Создание DLSS ресурсов
     * @return true если успешно
     */
    bool createDLSSResources();

    /**
     * @brief Обновление статистики
     * @param executionTime Время выполнения последнего кадра
     */
    void updateStats(float executionTime, bool success);

    /**
     * @brief Валидация входных параметров
     * @param params Параметры для проверки
     * @return true если параметры валидны
     */
    bool validateParams(const UpscalingParams& params) const;

    /**
     * @brief Конвертация качества в DLSS настройки
     * @param quality Режим качества HyperEngine
     * @return Внутренний режим DLSS
     */
    int convertQualityToDLSS(UpscalingQuality quality) const;
};

/**
 * @brief AMD FSR реализация стратегии upscaling
 *
 * Конкретная реализация Strategy Pattern для AMD FidelityFX Super Resolution.
 * Поддерживает FSR 1.0, 2.0 и 3.0.
 */
class FSRUpscalingStrategy : public IUpscalingStrategy {
  private:
    struct Impl;  // PIMPL для скрытия FSR SDK деталей
    std::unique_ptr<Impl> pImpl_;

    UpscalingConfig config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    bool initialized_ = false;
    mutable UpscalingStats stats_;

  public:
    FSRUpscalingStrategy();
    ~FSRUpscalingStrategy() override;

    // Запретить копирование
    FSRUpscalingStrategy(const FSRUpscalingStrategy&) = delete;
    FSRUpscalingStrategy& operator=(const FSRUpscalingStrategy&) = delete;

    // Разрешить перемещение
    FSRUpscalingStrategy(FSRUpscalingStrategy&&) noexcept;
    FSRUpscalingStrategy& operator=(FSRUpscalingStrategy&&) noexcept;

    // Реализация IUpscalingStrategy
    bool initialize(const UpscalingConfig& config,
                    std::shared_ptr<IResourceManager> resourceManager) override;

    UpscalingResult upscale(const UpscalingParams& params) override;

    void shutdown() override;

    UpscalingType getType() const override {
        return UpscalingType::FSR3;  // Автоматически выбираем лучшую доступную версию
    }

    std::string getDisplayName() const override;

    bool isSupported() const override;

    std::vector<UpscalingQuality> getSupportedQualities() const override;

    std::pair<int, int> getRecommendedRenderResolution(int targetWidth,
                                                       int targetHeight,
                                                       UpscalingQuality quality) const override;

    bool supportsFeature(UpscalingFeature feature) const override;

    size_t getMemoryUsage() const override;

    UpscalingStats getStats() const override { return stats_; }

    void reset() override;

    // FSR специфичные методы

    /**
     * @brief Проверить поддержку FSR 3.0
     * @return true если FSR 3.0 поддерживается
     */
    bool supportsFSR3() const;

    /**
     * @brief Получить версию FSR SDK
     * @return Строка с версией FSR
     */
    std::string getFSRVersion() const;

    /**
     * @brief Настроить силу sharpening
     * @param strength Сила от 0.0 до 1.0
     */
    void setSharpeningStrength(float strength);

  private:
    /**
     * @brief Инициализация FSR SDK
     * @return true если успешно
     */
    bool initializeFSR();

    /**
     * @brief Создание FSR ресурсов
     * @return true если успешно
     */
    bool createFSRResources();

    /**
     * @brief Обновление статистики
     * @param executionTime Время выполнения последнего кадра
     */
    void updateStats(float executionTime, bool success);

    /**
     * @brief Валидация входных параметров
     * @param params Параметры для проверки
     * @return true если параметры валидны
     */
    bool validateParams(const UpscalingParams& params) const;

    /**
     * @brief Конвертация качества в FSR настройки
     * @param quality Режим качества HyperEngine
     * @return Коэффициент upscaling для FSR
     */
    float convertQualityToFSR(UpscalingQuality quality) const;
};

/**
 * @brief Базовая реализация upscaling без ML
 *
 * Простая реализация с традиционными алгоритмами фильтрации.
 * Используется как fallback когда специализированные алгоритмы недоступны.
 */
class BasicUpscalingStrategy : public IUpscalingStrategy {
  private:
    UpscalingType algorithm_;
    UpscalingConfig config_;
    std::shared_ptr<IResourceManager> resourceManager_;
    bool initialized_ = false;
    mutable UpscalingStats stats_;

  public:
    explicit BasicUpscalingStrategy(UpscalingType algorithm = UpscalingType::Bicubic);
    ~BasicUpscalingStrategy() override = default;

    // Реализация IUpscalingStrategy
    bool initialize(const UpscalingConfig& config,
                    std::shared_ptr<IResourceManager> resourceManager) override;

    UpscalingResult upscale(const UpscalingParams& params) override;

    void shutdown() override;

    UpscalingType getType() const override { return algorithm_; }

    std::string getDisplayName() const override;

    bool isSupported() const override { return true; }  // Всегда доступно

    std::vector<UpscalingQuality> getSupportedQualities() const override;

    std::pair<int, int> getRecommendedRenderResolution(int targetWidth,
                                                       int targetHeight,
                                                       UpscalingQuality quality) const override;

    bool supportsFeature(UpscalingFeature feature) const override;

    size_t getMemoryUsage() const override;

    UpscalingStats getStats() const override { return stats_; }

    void reset() override {}  // Нет temporal данных для сброса

  private:
    /**
     * @brief Применить билинейную фильтрацию
     */
    UpscalingResult applyBilinear(const UpscalingParams& params);

    /**
     * @brief Применить бикубическую фильтрацию
     */
    UpscalingResult applyBicubic(const UpscalingParams& params);

    /**
     * @brief Применить Lanczos фильтрацию
     */
    UpscalingResult applyLanczos(const UpscalingParams& params);

    /**
     * @brief Обновление статистики
     */
    void updateStats(float executionTime, bool success);
};

}  // namespace HyperEngine::Rendering::Upscaling
