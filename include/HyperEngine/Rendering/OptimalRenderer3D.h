#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Gaussian3D.h"
#include "HybridRenderer3D.h"

namespace HyperEngine {
namespace Rendering {

/**
 * @brief Оптимальный рендерер 3D на основе алгоритма из 3DRenderer_Whitelist.md
 *
 * Реализует псевдо-алгоритм Optimal3DRendering с интеграцией:
 * - 3D Gaussian Splatting для эффективного представления сцены
 * - Гибридный пайплайн растеризации и трассировки лучей
 * - AI деноизинг и нейронное масштабирование
 * - Адаптивные оптимизации для различного железа
 */
class OptimalRenderer3D {
  public:
    /**
     * @brief Конфигурация аппаратного обеспечения
     */
    struct HardwareConfig {
        bool supportsRayTracing = false;   // Поддержка аппаратной трассировки лучей
        bool supportsNeural = false;       // Поддержка нейронных операций
        bool supportsMeshShaders = false;  // Поддержка mesh шейдеров
        bool supportsVRS = false;          // Variable Rate Shading
        int gpuMemoryMB = 4096;            // Память GPU в MB
        int computeUnits = 32;             // Количество compute units
        float performanceRating = 1.0f;    // Относительная производительность (0.1-3.0)

        // Автоматическое определение возможностей
        void autoDetect();
    };

    /**
     * @brief Данные сцены для рендеринга
     */
    struct SceneData {
        std::vector<std::shared_ptr<GaussianField3D>> gaussianFields;
        std::vector<std::shared_ptr<Mesh3D>> meshes;
        std::vector<Math::Matrix4> transforms;
        std::vector<Math::Vector3> multiViewImages;  // Точки обзора для оптимизации

        // Метаданные сцены
        Math::Vector3 sceneCenter;
        float sceneRadius;
        int complexityLevel = 1;  // 1-5, влияет на выбор алгоритмов

        void calculateBounds();
        void optimizeForHardware(const HardwareConfig& config);
    };

    /**
     * @brief Параметры камеры для рендеринга
     */
    struct CameraParams {
        Math::Vector3 position;
        Math::Vector3 target;
        Math::Vector3 up;
        float fov;
        float aspectRatio;
        float nearPlane;
        float farPlane;

        // Дополнительные параметры для оптимизации
        float viewDistance;      // Расстояние до центра сцены
        Math::Vector3 velocity;  // Скорость движения камеры
        bool isMoving;           // Флаг движения для временной стабилизации

        CameraParams()
            : position(0, 0, 5),
              target(0, 0, 0),
              up(0, 1, 0),
              fov(60.0f),
              aspectRatio(16.0f / 9.0f),
              nearPlane(0.1f),
              farPlane(1000.0f),
              viewDistance(5.0f),
              velocity(0, 0, 0),
              isMoving(false) {}
    };

    OptimalRenderer3D();
    virtual ~OptimalRenderer3D();

    // Основная инициализация по псевдо-алгоритму
    bool initialize(int width, int height, const HardwareConfig& hwConfig);
    void cleanup();

    /**
     * @brief Главный метод рендеринга согласно псевдо-алгоритму
     *
     * PROCEDURE Optimal3DRendering(SceneData, CameraParams, HardwareConfig)
     */
    void renderOptimal3D(const SceneData& sceneData,
                         const CameraParams& cameraParams,
                         const HardwareConfig& hardwareConfig);

    // Настройки качества и производительности
    void setQualityLevel(int level);          // 1-5 (Low, Medium, High, Ultra, Extreme)
    void setPerformanceTarget(float fps);     // Целевой FPS
    void enableAdaptiveQuality(bool enable);  // Автоматическая адаптация качества

    // Управление эффектами
    void enableGlobalIllumination(bool enable);
    void enableReflections(bool enable);
    void enableShadows(bool enable);
    void enableDenoising(bool enable);
    void enableUpscaling(bool enable, float factor = 2.0f);

    // Статистика и мониторинг
    struct PerformanceMetrics {
        // Время выполнения этапов (в мс)
        float sceneOptimizationTime;
        float rasterizationTime;
        float rayTracingTime;
        float denoisingTime;
        float upscalingTime;
        float postProcessingTime;
        float totalFrameTime;

        // Эффективность
        float renderingSpeedup;  // Ускорение по сравнению с базовым рендерингом
        float memoryEfficiency;  // Эффективность использования памяти
        int gaussiansProcessed;
        int raysTraced;
        int trianglesRendered;

        // Качество
        float visualQuality;      // Субъективная оценка качества
        float temporalStability;  // Стабильность между кадрами

        void reset();
        float getTotalTime() const;
        void print() const;
    };
    const PerformanceMetrics& getPerformanceMetrics() const { return metrics; }

    // Экспорт/импорт настроек для различных конфигураций
    void saveProfile(const std::string& filename) const;
    bool loadProfile(const std::string& filename);

    // Бенчмаркинг и профилирование
    void runBenchmark(const SceneData& testScene, int frames = 100);
    void startProfiling();
    void endProfiling();

  protected:
    // Этапы псевдо-алгоритма

    /**
     * @brief Этап 1: Scene Representation Optimization
     * Gaussians = OptimizeGaussians(SceneData.MultiViewImages)
     */
    void optimizeSceneRepresentation(SceneData& sceneData, const HardwareConfig& hwConfig);

    /**
     * @brief Интеграция NeRF вариантов (опционально)
     * IF HardwareConfig.SupportsNeural THEN IntegrateNeRFVariants(Gaussians)
     */
    void integrateNeRFVariants(SceneData& sceneData);

    /**
     * @brief Этап 2: Geometry and Primary Visibility
     * PrimaryImage = RasterizeGaussians(Gaussians, CameraParams)
     */
    void executeGeometryAndVisibility(const SceneData& sceneData, const CameraParams& cameraParams);

    /**
     * @brief Этап 3: Advanced Lighting Computation
     * LightingEffects = RayTraceSelective(Gaussians, PrimaryImage)
     * ApplyRealTimeGI(LightingEffects)
     */
    void executeAdvancedLighting(const SceneData& sceneData);

    /**
     * @brief Этап 4: Denoising and Refinement
     * DenoisedImage = AIDenoise(LightingEffects + PrimaryImage)
     */
    void executeDenoisingAndRefinement();

    /**
     * @brief Этап 5: Post-Processing and Output
     * FinalImage = NeuralUpscale(DenoisedImage, ResolutionTarget)
     * ApplyPostEffects(FinalImage)
     */
    void executePostProcessingAndOutput(int outputWidth, int outputHeight);

    // Компоненты рендерера
    std::unique_ptr<HybridRenderer3D> hybridRenderer;
    std::unique_ptr<ReSTIRGlobalIllumination> globalIllumination;
    std::unique_ptr<AIDenoiser> denoiser;
    std::unique_ptr<NeuralUpscaler> upscaler;

    // Конфигурация и состояние
    HardwareConfig currentHardware;
    CameraParams lastCameraParams;
    bool initialized;
    int renderWidth, renderHeight;
    int targetWidth, targetHeight;

    // Настройки качества
    int qualityLevel;
    float targetFPS;
    bool adaptiveQualityEnabled;

    // Включенные эффекты
    bool globalIlluminationEnabled;
    bool reflectionsEnabled;
    bool shadowsEnabled;
    bool denoisingEnabled;
    bool upscalingEnabled;
    float upscalingFactor;

    // Метрики производительности
    PerformanceMetrics metrics;
    std::vector<float> frameTimeHistory;
    bool profilingEnabled;

    // Адаптивные алгоритмы
    void adaptToHardware();
    void optimizeForFrameRate();
    void balanceQualityAndPerformance();

    // Утилиты
    float measureExecutionTime(const std::function<void()>& func);
    void updateMetrics();

    // Кэширование и оптимизация
    struct OptimizationCache {
        std::shared_ptr<GaussianField3D> optimizedGaussians;
        std::vector<Math::Vector3> lastViewPoints;
        bool cacheValid;
        float sceneComplexity;

        void invalidate() { cacheValid = false; }
    } optimizationCache;
};

/**
 * @brief Фабрика для создания оптимального рендерера
 *
 * Автоматически определяет конфигурацию аппаратного обеспечения
 * и создает наиболее подходящий рендерер.
 */
class OptimalRendererFactory {
  public:
    struct CreationParams {
        int width = 1920;
        int height = 1080;
        int qualityLevel = 3;  // 1-5
        float targetFPS = 60.0f;
        bool enableAllEffects = true;
        bool autoOptimize = true;
    };

    /**
     * @brief Создает оптимальный рендерер для текущей системы
     */
    static std::unique_ptr<OptimalRenderer3D> createOptimalRenderer(
        const CreationParams& params = CreationParams{});

    /**
     * @brief Определяет конфигурацию аппаратного обеспечения
     */
    static OptimalRenderer3D::HardwareConfig detectHardware();

    /**
     * @brief Рекомендует настройки для данного железа
     */
    static CreationParams recommendSettings(const OptimalRenderer3D::HardwareConfig& hw);

    /**
     * @brief Бенчмарк системы для определения оптимальных настроек
     */
    static void benchmarkSystem(OptimalRenderer3D::HardwareConfig& hw, CreationParams& params);
};

/**
 * @brief Система адаптивного LOD для гауссианов
 *
 * Автоматически управляет уровнем детализации в зависимости от:
 * - Расстояния до камеры
 * - Размера на экране
 * - Производительности системы
 * - Движения камеры
 */
class AdaptiveLOD {
  public:
    struct LODSettings {
        float maxDistance = 1000.0f;         // Максимальная дистанция рендеринга
        float highDetailDistance = 10.0f;    // Расстояние для максимальной детализации
        float mediumDetailDistance = 50.0f;  // Расстояние для средней детализации
        float lowDetailDistance = 200.0f;    // Расстояние для низкой детализации

        int maxGaussiansPerLevel[4] = {50000, 20000, 5000, 1000};  // LOD уровни
        float qualityMultipliers[4] = {1.0f, 0.7f, 0.4f, 0.2f};    // Множители качества

        bool enableTemporalStability = true;  // Стабилизация LOD во времени
        bool enableMotionBasedLOD = true;     // LOD на основе движения
    };

    AdaptiveLOD();

    void setSettings(const LODSettings& settings);
    const LODSettings& getSettings() const { return settings; }

    /**
     * @brief Вычисляет LOD для поля гауссианов
     */
    void computeLOD(GaussianField3D& field,
                    const Math::Vector3& cameraPos,
                    const Math::Vector3& cameraVelocity,
                    float targetFrameTime);

    /**
     * @brief Адаптивная настройка LOD на основе производительности
     */
    void adaptToPerformance(float currentFrameTime, float targetFrameTime);

    // Статистика LOD
    struct LODStats {
        int gaussiansPerLevel[4] = {0, 0, 0, 0};
        float averageDistance;
        int totalGaussians;
        float lodAdjustmentFactor;

        void reset() {
            for (int i = 0; i < 4; ++i)
                gaussiansPerLevel[i] = 0;
            averageDistance = 0.0f;
            totalGaussians = 0;
            lodAdjustmentFactor = 1.0f;
        }
    };
    const LODStats& getStats() const { return stats; }

  private:
    LODSettings settings;
    LODStats stats;
    float currentLODAdjustment;
    std::vector<float> frameTimeHistory;

    int computeLODLevel(float distance, float screenSize, float velocity) const;
    void applySpatialLOD(GaussianField3D& field, const Math::Vector3& cameraPos);
    void applyTemporalStabilization(GaussianField3D& field);
    void applyMotionBasedLOD(GaussianField3D& field, const Math::Vector3& velocity);
};

}  // namespace Rendering
}  // namespace HyperEngine
