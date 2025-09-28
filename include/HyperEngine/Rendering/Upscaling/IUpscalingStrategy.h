#pragma once

#include <memory>
#include <string>
#include "HyperEngine/Rendering/Common/IResourceManager.h"

namespace HyperEngine::Rendering::Upscaling {

// Forward declarations
struct UpscalingConfig;
struct UpscalingParams;
struct UpscalingResult;

/**
 * @brief Стратегия для upscaling изображений
 * 
 * Применяет Strategy Pattern для переключения между
 * различными алгоритмами upscaling (DLSS, FSR, XeSS, etc.)
 * 
 * Это позволяет добавлять новые алгоритмы upscaling
 * без изменения существующего кода (Open/Closed Principle).
 */
class IUpscalingStrategy {
public:
    virtual ~IUpscalingStrategy() = default;
    
    /**
     * @brief Инициализация стратегии
     * @param config Конфигурация upscaling
     * @param resourceManager Менеджер ресурсов
     * @return true если инициализация прошла успешно
     */
    virtual bool initialize(const UpscalingConfig& config, 
                          std::shared_ptr<IResourceManager> resourceManager) = 0;
    
    /**
     * @brief Применить upscaling к изображению
     * @param params Параметры upscaling для текущего кадра
     * @return Результат upscaling с метриками
     */
    virtual UpscalingResult upscale(const UpscalingParams& params) = 0;
    
    /**
     * @brief Завершение работы стратегии
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief Получить тип стратегии
     * @return Тип upscaling алгоритма
     */
    virtual UpscalingType getType() const = 0;
    
    /**
     * @brief Получить имя стратегии для отображения пользователю
     * @return Человекочитаемое имя (например, "NVIDIA DLSS 3.0")
     */
    virtual std::string getDisplayName() const = 0;
    
    /**
     * @brief Проверить поддержку на текущем оборудовании
     * @return true если алгоритм поддерживается
     */
    virtual bool isSupported() const = 0;
    
    /**
     * @brief Получить поддерживаемые качественные режимы
     * @return Список доступных режимов качества
     */
    virtual std::vector<UpscalingQuality> getSupportedQualities() const = 0;
    
    /**
     * @brief Получить рекомендованное разрешение рендеринга
     * @param targetWidth Целевая ширина вывода
     * @param targetHeight Целевая высота вывода
     * @param quality Режим качества
     * @return Рекомендованное разрешение для рендеринга
     */
    virtual std::pair<int, int> getRecommendedRenderResolution(
        int targetWidth, int targetHeight, UpscalingQuality quality) const = 0;
    
    /**
     * @brief Проверить поддержку функции
     * @param feature Функция для проверки
     * @return true если функция поддерживается
     */
    virtual bool supportsFeature(UpscalingFeature feature) const = 0;
    
    /**
     * @brief Получить использование памяти
     * @return Использованная память в байтах
     */
    virtual size_t getMemoryUsage() const = 0;
    
    /**
     * @brief Получить статистику производительности
     * @return Структура с метриками производительности
     */
    virtual UpscalingStats getStats() const = 0;
    
    /**
     * @brief Сброс внутреннего состояния (например, temporal data)
     * Используется при переключении сцен или значительном изменении камеры
     */
    virtual void reset() = 0;
};

/**
 * @brief Типы upscaling алгоритмов
 */
enum class UpscalingType {
    None,          ///< Без upscaling (native разрешение)
    Bilinear,      ///< Простая билинейная фильтрация
    Bicubic,       ///< Бикубическая фильтрация
    Lanczos,       ///< Lanczos фильтрация
    FSR1,          ///< AMD FidelityFX Super Resolution 1.0
    FSR2,          ///< AMD FidelityFX Super Resolution 2.0
    FSR3,          ///< AMD FidelityFX Super Resolution 3.0
    DLSS2,         ///< NVIDIA Deep Learning Super Sampling 2.0
    DLSS3,         ///< NVIDIA Deep Learning Super Sampling 3.0
    XeSS,          ///< Intel Xe Super Sampling
    MetalFX,       ///< Apple MetalFX Upscaling
    Custom         ///< Пользовательская реализация
};

/**
 * @brief Режимы качества upscaling
 */
enum class UpscalingQuality {
    Performance,   ///< Максимальная производительность (низкое качество)
    Balanced,      ///< Баланс между качеством и производительностью
    Quality,       ///< Высокое качество (средняя производительность)
    UltraQuality,  ///< Максимальное качество (низкая производительность)
    Native         ///< Нативное разрешение (без upscaling)
};

/**
 * @brief Функции upscaling
 */
enum class UpscalingFeature {
    TemporalAccumulation,  ///< Temporal accumulation для качества
    MotionVectors,         ///< Использование motion vectors
    DepthBuffer,           ///< Использование depth buffer
    RayTracing,            ///< Интеграция с ray tracing
    FrameGeneration,       ///< Генерация промежуточных кадров
    ReflexIntegration,     ///< Интеграция с NVIDIA Reflex
    VariableRateShading,   ///< Поддержка VRS
    HDR,                   ///< Поддержка HDR
    AutoExposure           ///< Автоматическая экспозиция
};

/**
 * @brief Конфигурация upscaling
 */
struct UpscalingConfig {
    int outputWidth = 1920;                    ///< Целевая ширина
    int outputHeight = 1080;                   ///< Целевая высота  
    UpscalingQuality quality = UpscalingQuality::Quality; ///< Режим качества
    bool enableFrameGeneration = false;       ///< Включить генерацию кадров (DLSS 3)
    bool enableSharpening = true;              ///< Включить повышение резкости
    float sharpeningStrength = 0.5f;          ///< Сила повышения резкости [0, 1]
    bool enableHDR = false;                    ///< Поддержка HDR
    bool enableDebugOverlay = false;           ///< Отладочная информация
    std::string modelPath;                     ///< Путь к модели (для алгоритмов с ML)
};

/**
 * @brief Параметры upscaling для кадра
 */
struct UpscalingParams {
    TextureHandle inputColor = INVALID_HANDLE;       ///< Входное цветное изображение
    TextureHandle inputDepth = INVALID_HANDLE;       ///< Входной depth buffer
    TextureHandle motionVectors = INVALID_HANDLE;    ///< Motion vectors
    TextureHandle outputTexture = INVALID_HANDLE;    ///< Выходная текстура
    
    struct {
        float x = 0.0f, y = 0.0f;    ///< Jittering offset для temporal AA
    } jitter;
    
    struct {
        float deltaTime = 0.016f;     ///< Время кадра
        bool reset = false;           ///< Сбросить temporal данные
    } temporal;
    
    struct {
        float exposure = 1.0f;        ///< Текущая экспозиция
        float preExposure = 1.0f;     ///< Предыдущая экспозиция
    } exposure;
};

/**
 * @brief Результат upscaling
 */
struct UpscalingResult {
    bool success = false;              ///< Успешность операции
    float executionTime = 0.0f;        ///< Время выполнения в мс
    float actualUpscaleFactor = 1.0f;  ///< Фактический коэффициент увеличения
    int outputWidth = 0;               ///< Фактическая ширина результата
    int outputHeight = 0;              ///< Фактическая высота результата
    std::string errorMessage;          ///< Сообщение об ошибке (если success = false)
};

/**
 * @brief Статистика производительности upscaling
 */
struct UpscalingStats {
    float averageExecutionTime = 0.0f;  ///< Среднее время выполнения в мс
    float minExecutionTime = 0.0f;      ///< Минимальное время выполнения в мс
    float maxExecutionTime = 0.0f;      ///< Максимальное время выполнения в мс
    uint64_t framesProcessed = 0;       ///< Количество обработанных кадров
    uint64_t errorsCount = 0;           ///< Количество ошибок
    size_t memoryUsage = 0;             ///< Использование памяти в байтах
    float qualityScore = 0.0f;          ///< Оценка качества (если доступна)
    
    /**
     * @brief Получить FPS boost от upscaling
     * @param nativeFrameTime Время рендеринга в нативном разрешении
     * @return Коэффициент ускорения FPS
     */
    float getFpsBoost(float nativeFrameTime) const {
        if (averageExecutionTime <= 0.0f || nativeFrameTime <= 0.0f) return 1.0f;
        return nativeFrameTime / (nativeFrameTime + averageExecutionTime);
    }
};

/**
 * @brief Фабрика для создания upscaling стратегий
 */
class UpscalingStrategyFactory {
public:
    /**
     * @brief Создать стратегию upscaling
     * @param type Тип алгоритма
     * @return Умный указатель на стратегию или nullptr если не поддерживается
     */
    static std::unique_ptr<IUpscalingStrategy> createStrategy(UpscalingType type);
    
    /**
     * @brief Получить список поддерживаемых алгоритмов на текущем оборудовании
     * @return Вектор поддерживаемых типов
     */
    static std::vector<UpscalingType> getSupportedTypes();
    
    /**
     * @brief Получить рекомендованную стратегию для текущего оборудования
     * @return Тип рекомендованного алгоритма
     */
    static UpscalingType getRecommendedType();
    
    /**
     * @brief Проверить поддержку алгоритма
     * @param type Тип алгоритма
     * @return true если поддерживается
     */
    static bool isSupported(UpscalingType type);
    
private:
    UpscalingStrategyFactory() = default; // Запретить создание экземпляров
};

} // namespace HyperEngine::Rendering::Upscaling
