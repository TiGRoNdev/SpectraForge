#pragma once

#include "IRenderStage.h"
#include <memory>
#include <chrono>

namespace HyperEngine::Rendering {

// Forward declarations
class FlashGSSplatter;

/**
 * @brief Этап первичной растеризации
 * 
 * Применяет Single Responsibility Principle - 
 * отвечает ТОЛЬКО за 3D Gaussian Splatting.
 * 
 * Этот этап рендерит основную геометрию сцены используя
 * алгоритм 3D Gaussian Splatting для максимальной производительности.
 */
class PrimaryRasterizationStage : public IRenderStage {
private:
    std::unique_ptr<FlashGSSplatter> splatter_;
    std::shared_ptr<IResourceManager> resourceManager_;
    
    // Состояние этапа
    bool initialized_ = false;
    bool ready_ = false;
    
    // Статистика производительности
    mutable float lastExecutionTime_ = 0.0f;
    mutable uint32_t gaussiansRendered_ = 0;
    mutable std::vector<float> executionTimeHistory_;
    static constexpr size_t HISTORY_SIZE = 60;
    
    // Настройки Gaussian Splatting
    struct GaussianSettings {
        float lodBias = 0.0f;               ///< LOD bias для дальних Gaussian'ов
        float cullingThreshold = 0.01f;     ///< Порог отсечения по размеру
        bool enableOcclusion = true;        ///< Включить occlusion culling
        bool enableFrustumCulling = true;   ///< Включить frustum culling
        uint32_t maxGaussians = 1000000;    ///< Максимальное количество Gaussian'ов
        bool enableLOD = true;              ///< Включить level-of-detail
        float lodDistance = 100.0f;         ///< Расстояние начала LOD
    } settings_;

public:
    PrimaryRasterizationStage();
    ~PrimaryRasterizationStage() override;
    
    // Запретить копирование
    PrimaryRasterizationStage(const PrimaryRasterizationStage&) = delete;
    PrimaryRasterizationStage& operator=(const PrimaryRasterizationStage&) = delete;
    
    // Разрешить перемещение
    PrimaryRasterizationStage(PrimaryRasterizationStage&&) noexcept;
    PrimaryRasterizationStage& operator=(PrimaryRasterizationStage&&) noexcept;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "PrimaryRasterization"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 1000; } // Высокий приоритет - выполняется первым
    std::vector<std::string> getDependencies() const override { return {}; } // Нет зависимостей
    float getExecutionTime() const override { return lastExecutionTime_; }
    bool supportsFeature(const std::string& feature) const override;
    
    // Специфичные методы этапа
    
    /**
     * @brief Загрузить Gaussian данные из файла
     * @param filePath Путь к файлу с Gaussian данными
     * @return true если загрузка прошла успешно
     */
    bool loadGaussianData(const std::string& filePath);
    
    /**
     * @brief Установить Gaussian данные напрямую
     * @param gaussians Указатель на данные Gaussian'ов
     * @param count Количество Gaussian'ов
     * @return true если данные установлены успешно
     */
    bool setGaussianData(const void* gaussians, uint32_t count);
    
    /**
     * @brief Получить настройки Gaussian Splatting
     * @return Ссылка на настройки
     */
    GaussianSettings& getSettings() { return settings_; }
    const GaussianSettings& getSettings() const { return settings_; }
    
    /**
     * @brief Получить количество отрендеренных Gaussian'ов в последнем кадре
     * @return Количество Gaussian'ов
     */
    uint32_t getRenderedGaussianCount() const { return gaussiansRendered_; }
    
    /**
     * @brief Получить среднее время выполнения
     * @return Время в миллисекундах
     */
    float getAverageExecutionTime() const;
    
    /**
     * @brief Включить/выключить отладочную визуализацию
     * @param enable Включить отладку
     */
    void setDebugVisualization(bool enable);
    
    /**
     * @brief Сбросить статистику производительности
     */
    void resetPerformanceStats();

private:
    /**
     * @brief Обновление статистики
     * @param executionTime Время выполнения
     * @param gaussianCount Количество Gaussian'ов
     */
    void updateStats(float executionTime, uint32_t gaussianCount);
    
    /**
     * @brief Валидация входных данных
     * @param context Контекст рендеринга
     * @return true если данные валидны
     */
    bool validateInputs(const RenderContext& context) const;
    
    /**
     * @brief Подготовка Gaussian данных для рендеринга
     * @param context Контекст рендеринга
     * @return true если подготовка прошла успешно
     */
    bool prepareGaussianData(RenderContext& context);
    
    /**
     * @brief Выполнение culling операций
     * @param context Контекст рендеринга
     * @return Количество видимых Gaussian'ов
     */
    uint32_t performCulling(const RenderContext& context);
    
    /**
     * @brief Сортировка Gaussian'ов по глубине
     * @param context Контекст рендеринга
     */
    void sortGaussiansByDepth(const RenderContext& context);
};

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
        bool enableReflections = true;      ///< Включить отражения
        bool enableShadows = true;          ///< Включить тени
        bool enableGlobalIllumination = true; ///< Включить GI
        bool enableAmbientOcclusion = true; ///< Включить AO
        uint32_t maxBounces = 3;            ///< Максимальное количество отскоков
        uint32_t samplesPerPixel = 1;       ///< Количество лучей на пиксель
        float rayTMin = 0.001f;             ///< Минимальное расстояние луча
        float rayTMax = 1000.0f;            ///< Максимальное расстояние луча
        bool enableSER = true;              ///< Shader Execution Reordering
        bool enableAdaptiveSampling = true; ///< Адаптивный сэмплинг
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
    int getPriority() const override { return 2000; } // После первичной растеризации
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
        float temporalAlpha = 0.95f;        ///< Temporal accumulation factor
        bool enableSpatialFilter = true;    ///< Включить пространственную фильтрацию
        bool enableTemporalFilter = true;   ///< Включить временную фильтрацию
        uint32_t spatialIterations = 3;     ///< Итерации пространственной фильтрации
        float varianceThreshold = 0.1f;     ///< Порог variance для адаптивности
        bool enableAOV = true;              ///< Использовать auxiliary buffers
    } settings_;

public:
    AIDenoiseStage();
    ~AIDenoiseStage() override;

    // Реализация IRenderStage
    bool initialize(std::shared_ptr<IResourceManager> resourceManager) override;
    void execute(RenderContext& context) override;
    void shutdown() override;
    std::string getName() const override { return "AIDenoising"; }
    bool isReady() const override { return ready_; }
    int getPriority() const override { return 3000; } // После ray tracing
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

} // namespace HyperEngine::Rendering
