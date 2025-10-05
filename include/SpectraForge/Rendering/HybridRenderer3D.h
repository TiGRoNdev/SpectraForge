
#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Gaussian3D.h"
#include "Renderer3D.h"

namespace SpectraForge {
namespace Rendering {

// Forward declarations
class Shader3D;
class Camera3D;
class Mesh3D;

/**
 * @brief Гибридный рендерер, объединяющий растеризацию и трассировку лучей
 *
 * Реализует современный подход 2025 года:
 * 1. Быстрая растеризация для первичной геометрии и видимости
 * 2. Селективная трассировка лучей для глобального освещения, отражений и теней
 * 3. Интеграция с 3D Gaussian Splatting для оптимального представления сцены
 */
class HybridRenderer3D : public Renderer3D {
  public:
    /**
     * @brief Типы рендеринга для различных этапов пайплайна
     */
    enum class RenderPass {
        DEPTH_PREPASS,           // Предварительный проход глубины
        GAUSSIAN_RASTERIZATION,  // Растеризация гауссианов
        RAY_TRACED_GI,  // Трассировка лучей для глобального освещения
        RAY_TRACED_REFLECTIONS,  // Трассировка лучей для отражений
        RAY_TRACED_SHADOWS,      // Трассировка лучей для теней
        COMPOSITING,             // Композитинг результатов
        POST_PROCESSING          // Пост-обработка
    };

    /**
     * @brief Настройки качества рендеринга
     */
    struct RenderSettings {
        // Настройки растеризации
        bool enableGaussianSplatting = true;
        float gaussianQuality = 1.0f;
        bool enableDepthPrepass = true;

        // Настройки трассировки лучей
        bool enableRayTracedGI = true;
        bool enableRayTracedReflections = true;
        bool enableRayTracedShadows = true;
        int rayTracingSamples = 4;
        float rayTracingIntensity = 1.0f;

        // Настройки производительности
        bool enableAdaptiveSampling = true;
        bool enableTemporalAccumulation = true;
        bool enableSpatialUpsampling = true;
        float performanceTarget = 60.0f;  // FPS

        // Настройки качества
        bool enableDenoising = true;
        bool enableNeuralUpscaling = true;
        float denoisingStrength = 0.8f;
        float upscalingFactor = 1.0f;
    };

    HybridRenderer3D();
    virtual ~HybridRenderer3D();

    // Инициализация (переопределение базового класса)
    bool initialize(int width, int height);
    void cleanup();

    // Основной рендеринг
    void beginFrame();
    void endFrame();

    // Гибридный рендеринг сцены
    void renderScene(const std::vector<std::shared_ptr<GaussianField3D>>& gaussianFields,
                     const std::vector<std::shared_ptr<Mesh3D>>& meshes,
                     const std::vector<Math::Matrix4>& transforms);

    // Настройки рендеринга
    void setRenderSettings(const RenderSettings& settings);
    const RenderSettings& getRenderSettings() const { return renderSettings; }

    // Управление пассами рендеринга
    void enableRenderPass(RenderPass pass, bool enable);
    bool isRenderPassEnabled(RenderPass pass) const;

    // Адаптивная производительность
    void setTargetFrameRate(float fps);
    void enableDynamicQuality(bool enable);
    float getCurrentFrameRate() const { return currentFrameRate; }

    // Статистика гибридного рендеринга
    struct HybridRenderStats {
        // Статистика растеризации
        float rasterizationTime;
        size_t gaussiansRendered;
        size_t trianglesRendered;

        // Статистика трассировки лучей
        float rayTracingTime;
        size_t raysTraced;
        size_t rayHits;

        // Статистика композитинга
        float compositingTime;
        float denoisingTime;
        float upscalingTime;

        // Общая статистика
        float totalFrameTime;
        float adaptiveQualityFactor;

        void reset() {
            rasterizationTime = rayTracingTime = compositingTime = 0.0f;
            denoisingTime = upscalingTime = totalFrameTime = 0.0f;
            gaussiansRendered = trianglesRendered = raysTraced = rayHits = 0;
            adaptiveQualityFactor = 1.0f;
        }
    };
    const HybridRenderStats& getHybridRenderStats() const { return hybridStats; }

  protected:
    // Рендерер компоненты
    std::unique_ptr<GaussianRenderer3D> gaussianRenderer;

    // Шейдеры для различных пассов
    std::shared_ptr<Shader3D> depthPrepassShader;
    std::shared_ptr<Shader3D> rayTracingShader;
    std::shared_ptr<Shader3D> compositingShader;
    std::shared_ptr<Shader3D> denoisingShader;
    std::shared_ptr<Shader3D> upscalingShader;

    // Буферы рендеринга
    struct RenderBuffers {
        unsigned int depthBuffer;
        unsigned int colorBuffer;
        unsigned int normalBuffer;
        unsigned int motionBuffer;
        unsigned int rayTracedGI;
        unsigned int rayTracedReflections;
        unsigned int rayTracedShadows;
        unsigned int denoisedBuffer;
        unsigned int finalBuffer;

        unsigned int framebuffer;
        int width, height;

        void resize(int w, int h);
        void cleanup();
    } renderBuffers;

    // Настройки и состояние
    RenderSettings renderSettings;
    std::vector<bool> enabledPasses;

    // Адаптивная производительность
    bool dynamicQualityEnabled;
    float targetFrameRate;
    float currentFrameRate;
    float qualityAdjustmentFactor;
    std::vector<float> frameTimeHistory;

    // Статистика
    HybridRenderStats hybridStats;

    // Внутренние методы рендеринга
    void executeDepthPrepass(const std::vector<std::shared_ptr<Mesh3D>>& meshes,
                             const std::vector<Math::Matrix4>& transforms);

    void executeGaussianRasterization(
        const std::vector<std::shared_ptr<GaussianField3D>>& gaussianFields);

    void executeRayTracedGI();
    void executeRayTracedReflections();
    void executeRayTracedShadows();

    void executeCompositing();
    void executeDenoising();
    void executeNeuralUpscaling();

    // Адаптивное управление качеством
    void updateAdaptiveQuality();
    void adjustRenderSettings();

    // Управление буферами
    void setupRenderBuffers();
    void resizeRenderBuffers(int width, int height);

    // Утилиты
    void bindRenderTarget(RenderPass pass);
    void setupShaderForPass(RenderPass pass);
    float measurePassTime(const std::function<void()>& renderFunc);
};

/**
 * @brief Система глобального освещения на основе ReSTIR
 *
 * Реализует современные алгоритмы стохастического освещения
 * для real-time глобального освещения.
 */
class ReSTIRGlobalIllumination {
  public:
    struct ReSTIRSettings {
        int candidateSamples = 8;  // Количество кандидатов для выборки
        int spatialSamples = 4;  // Пространственная повторная выборка
        int temporalSamples = 4;      // Временная повторная выборка
        float spatialRadius = 32.0f;  // Радиус пространственной выборки
        bool enableTemporalReuse = true;
        bool enableSpatialReuse = true;
        float biasCorrection = 0.1f;
    };

    ReSTIRGlobalIllumination();
    ~ReSTIRGlobalIllumination();

    bool initialize(int width, int height);
    void cleanup();

    void computeGlobalIllumination(const Math::Matrix4& viewMatrix,
                                   const Math::Matrix4& projMatrix,
                                   unsigned int depthBuffer,
                                   unsigned int normalBuffer,
                                   unsigned int outputBuffer);

    void setSettings(const ReSTIRSettings& settings);
    const ReSTIRSettings& getSettings() const { return settings; }

    // Статистика
    struct ReSTIRStats {
        float computeTime;
        size_t lightSamples;
        size_t spatialResamples;
        size_t temporalResamples;

        void reset() {
            computeTime = 0.0f;
            lightSamples = spatialResamples = temporalResamples = 0;
        }
    };
    const ReSTIRStats& getStats() const { return stats; }

  private:
    bool initialized;
    ReSTIRSettings settings;
    ReSTIRStats stats;

    // GPU ресурсы
    unsigned int reservoirBuffer;
    unsigned int lightBuffer;
    unsigned int computeShader;

    // Временные буферы для многопроходного алгоритма
    unsigned int temporalBuffer;
    unsigned int spatialBuffer;

    void setupReservoirs();
    void candidateGeneration();
    void temporalResampling();
    void spatialResampling();
    void finalShading();
};

/**
 * @brief Система адаптивного деноизинга с ИИ
 *
 * Использует нейронные сети для удаления шума из ray-traced изображений
 * с минимальной потерей деталей.
 */
class AIDenoiser {
  public:
    enum class DenoiserType {
        FAST_BILATERAL,  // Быстрый билатеральный фильтр
        EDGE_AWARE,      // Edge-aware деноизинг
        NEURAL_NETWORK,  // Нейронная сеть (если поддерживается)
        TEMPORAL_ACCUMULATION  // Временное накопление
    };

    struct DenoiserSettings {
        DenoiserType type = DenoiserType::EDGE_AWARE;
        float strength = 0.8f;
        float edgeThreshold = 0.1f;
        int filterRadius = 3;
        bool useMotionVectors = true;
        bool useNormalBuffer = true;
        bool useAlbedoBuffer = false;
    };

    AIDenoiser();
    ~AIDenoiser();

    bool initialize();
    void cleanup();

    void denoise(unsigned int noisyBuffer,
                 unsigned int outputBuffer,
                 unsigned int normalBuffer = 0,
                 unsigned int motionBuffer = 0,
                 unsigned int albedoBuffer = 0);

    void setSettings(const DenoiserSettings& settings);
    const DenoiserSettings& getSettings() const { return settings; }

    // Статистика
    struct DenoiserStats {
        float denoiseTime;
        float noiseReduction;      // 0.0 - 1.0
        float detailPreservation;  // 0.0 - 1.0

        void reset() { denoiseTime = noiseReduction = detailPreservation = 0.0f; }
    };
    const DenoiserStats& getStats() const { return stats; }

  private:
    bool initialized;
    DenoiserSettings settings;
    DenoiserStats stats;

    // Различные деноизеры
    std::shared_ptr<Shader3D> bilateralShader;
    std::shared_ptr<Shader3D> edgeAwareShader;
    std::shared_ptr<Shader3D> temporalShader;

    // Временные буферы
    unsigned int temporalHistoryBuffer;
    unsigned int intermediateBuffer;

    void applyBilateralFilter(unsigned int input, unsigned int output);
    void applyEdgeAwareFilter(unsigned int input, unsigned int output, unsigned int normalBuffer);
    void applyTemporalAccumulation(unsigned int input,
                                   unsigned int output,
                                   unsigned int motionBuffer);
};

/**
 * @brief Система нейронного масштабирования (AI Upscaling)
 *
 * Реализует алгоритмы масштабирования изображений с использованием
 * нейронных сетей для повышения разрешения без потери качества.
 */
class NeuralUpscaler {
  public:
    enum class UpscalerType {
        BILINEAR,  // Стандартное билинейное масштабирование
        LANCZOS,   // Lanczos фильтр
        EDGE_ENHANCED,  // Улучшение краев
        AI_ENHANCED  // ИИ масштабирование (если поддерживается)
    };

    struct UpscalerSettings {
        UpscalerType type = UpscalerType::EDGE_ENHANCED;
        float scaleFactor = 2.0f;
        float sharpness = 0.5f;
        bool preserveDetails = true;
        bool antiAliasing = true;
    };

    NeuralUpscaler();
    ~NeuralUpscaler();

    bool initialize();
    void cleanup();

    void upscale(unsigned int inputBuffer,
                 unsigned int outputBuffer,
                 int inputWidth,
                 int inputHeight,
                 int outputWidth,
                 int outputHeight);

    void setSettings(const UpscalerSettings& settings);
    const UpscalerSettings& getSettings() const { return settings; }

    // Статистика
    struct UpscalerStats {
        float upscaleTime;
        float qualityMetric;  // PSNR или аналогичная метрика

        void reset() { upscaleTime = qualityMetric = 0.0f; }
    };
    const UpscalerStats& getStats() const { return stats; }

  private:
    bool initialized;
    UpscalerSettings settings;
    UpscalerStats stats;

    // Шейдеры для различных типов масштабирования
    std::shared_ptr<Shader3D> bilinearShader;
    std::shared_ptr<Shader3D> lanczosShader;
    std::shared_ptr<Shader3D> edgeEnhancedShader;

    void applyBilinearUpscaling(unsigned int input,
                                unsigned int output,
                                int inW,
                                int inH,
                                int outW,
                                int outH);
    void applyLanczosUpscaling(unsigned int input,
                               unsigned int output,
                               int inW,
                               int inH,
                               int outW,
                               int outH);
    void applyEdgeEnhancedUpscaling(unsigned int input,
                                    unsigned int output,
                                    int inW,
                                    int inH,
                                    int outW,
                                    int outH);
};

}  // namespace Rendering
}  // namespace SpectraForge
