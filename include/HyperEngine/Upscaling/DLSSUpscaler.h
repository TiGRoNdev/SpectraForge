#pragma once

#ifdef VULKAN_RENDERER_DLSS_SUPPORT

#include <glm/glm.hpp>
#include <memory>
#include "Upscaler.h"

// Forward declarations для DLSS/Streamline
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace HyperEngine::Upscaling {

/**
 * @brief Векторы движения для temporal upscaling
 */
struct MotionVectors {
    float* motionBuffer;  // Буфер векторов движения
    uint32_t width;
    uint32_t height;
    glm::mat4 prevViewMatrix;  // Предыдущая матрица вида
    glm::mat4 currViewMatrix;  // Текущая матрица вида
};

/**
 * @brief Tensor Core ускоритель
 */
class TensorCoreAccelerator {
  public:
    TensorCoreAccelerator();
    ~TensorCoreAccelerator();

    bool init();
    void shutdown();

    void accelerateInference(const float* input,
                             float* output,
                             uint32_t inputWidth,
                             uint32_t inputHeight,
                             uint32_t outputWidth,
                             uint32_t outputHeight);

  private:
    bool initialized = false;
    void* tensorCoreContext = nullptr;
};

/**
 * @brief Streamline Framework интеграция
 */
class StreamlineFramework {
  public:
    StreamlineFramework();
    ~StreamlineFramework();

    bool init();
    void shutdown();

    bool isDLSSAvailable() const;
    bool isDLSSGAvailable() const;

    void setDLSSOptions(uint32_t outputWidth,
                        uint32_t outputHeight,
                        uint32_t renderWidth,
                        uint32_t renderHeight);

  private:
    bool initialized = false;
    void* streamlineContext = nullptr;
};

/**
 * @brief NVIDIA DLSS Upscaler
 *
 * Реализует NVIDIA DLSS (Deep Learning Super Sampling) с поддержкой:
 * - Super Resolution
 * - Ray Reconstruction
 * - Multi-Frame Generation (DLSS 3)
 */
class DLSSUpscaler : public Upscaler {
  public:
    /**
     * @brief Конструктор
     */
    DLSSUpscaler();

    /**
     * @brief Деструктор
     */
    ~DLSSUpscaler() override;

    /**
     * @brief Инициализация DLSS
     * @param config Конфигурация железа
     * @return true если инициализация успешна
     */
    bool init(const HardwareConfig& config) override;

    /**
     * @brief Завершение работы DLSS
     */
    void shutdown() override;

    /**
     * @brief DLSS upscaling
     * @param image Входное изображение
     * @param target Целевое разрешение
     * @return Финальное изображение
     */
    FinalImage upscaleImage(const DenoisedImage& image, const ResolutionTarget& target) override;

    /**
     * @brief Проверка поддержки DLSS
     * @param config Конфигурация железа
     * @return true если DLSS поддерживается
     */
    bool isSupported(const HardwareConfig& config) const override;

    /**
     * @brief Получение имени
     * @return "DLSS"
     */
    const char* getName() const override { return "DLSS"; }

    /**
     * @brief Проверка инициализации
     * @return true если инициализирован
     */
    bool isInitialized() const override { return initialized; }

    /**
     * @brief Super Resolution с temporal данными
     * @param vectors Векторы движения
     */
    void superResolution(const MotionVectors& vectors);

    /**
     * @brief Ray Reconstruction для ray traced эффектов
     */
    void rayReconstruction();

    /**
     * @brief Multi-Frame Generation (DLSS 3)
     */
    void multiFrameGeneration();

    /**
     * @brief Установка качества DLSS
     * @param quality Качество (Performance, Balanced, Quality, Ultra Performance)
     */
    void setQuality(const std::string& quality);

    /**
     * @brief Получение рекомендуемого render разрешения
     * @param targetWidth Целевая ширина
     * @param targetHeight Целевая высота
     * @param quality Качество DLSS
     * @return Рекомендуемое render разрешение
     */
    static std::pair<uint32_t, uint32_t> getOptimalRenderResolution(uint32_t targetWidth,
                                                                    uint32_t targetHeight,
                                                                    const std::string& quality);

  private:
    std::unique_ptr<StreamlineFramework> streamline;
    std::unique_ptr<TensorCoreAccelerator> accelerator;

    // DLSS параметры
    uint32_t renderWidth = 0;
    uint32_t renderHeight = 0;
    uint32_t outputWidth = 0;
    uint32_t outputHeight = 0;

    std::string currentQuality = "Quality";
    bool dlssGEnabled = false;
    bool rayReconstructionEnabled = false;

    bool initialized = false;

    /**
     * @brief Проверка поддержки Tensor Cores
     * @return true если Tensor Cores поддерживаются
     */
    bool checkTensorCoreSupport() const;

    /**
     * @brief Настройка DLSS параметров
     * @param target Целевое разрешение
     * @return true если настройка успешна
     */
    bool configureDLSS(const ResolutionTarget& target);

    /**
     * @brief Выполнение DLSS inference
     * @param input Входные данные
     * @param output Выходные данные
     */
    void executeDLSSInference(const DenoisedImage& input, FinalImage& output);

    // Запрет копирования
    DLSSUpscaler(const DLSSUpscaler&) = delete;
    DLSSUpscaler& operator=(const DLSSUpscaler&) = delete;
};

}  // namespace HyperEngine::Upscaling

#endif  // VULKAN_RENDERER_DLSS_SUPPORT
