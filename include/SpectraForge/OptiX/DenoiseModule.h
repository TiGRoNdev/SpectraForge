
#pragma once

#include <memory>
#include <string>

// Forward declarations
namespace SpectraForge::Vulkan {
struct RawEffects;
struct DenoisedImage;
}  // namespace SpectraForge::Vulkan

namespace SpectraForge::OptiX {

/**
 * @brief Auxiliary buffers для деноизинга
 */
struct AuxBuffers {
    float* motionVectors;  // Motion vectors для temporal denoising
    float* albedo;         // Albedo buffer для better quality
    float* normals;        // Normal buffer для geometry-aware denoising
    uint32_t width;
    uint32_t height;
};

/**
 * @brief Модель автоэнкодера для деноизинга
 */
class AutoencoderModel {
  public:
    AutoencoderModel();
    ~AutoencoderModel();

    bool loadModel(const std::string& modelPath);
    void processImage(const float* input, float* output, uint32_t width, uint32_t height);

  private:
    bool initialized = false;
};

/**
 * @brief Модуль AI деноизинга на основе OptiX
 *
 * Использует OptiX AI Denoiser для удаления шума из ray traced эффектов
 * с поддержкой temporal accumulation и auxiliary buffers.
 */
class DenoiseModule {
  public:
    /**
     * @brief Конструктор
     */
    DenoiseModule();

    /**
     * @brief Деструктор
     */
    ~DenoiseModule();

    /**
     * @brief Инициализация модуля деноизинга
     * @return true если инициализация успешна
     */
    bool init();

    /**
     * @brief Завершение работы модуля
     */
    void shutdown();

    /**
     * @brief Деноизинг эффектов с auxiliary buffers
     * @param effects Сырые эффекты от ray tracing
     * @param buffers Auxiliary buffers для улучшения качества
     * @return Деноизированное изображение
     */
    Vulkan::DenoisedImage denoise(const Vulkan::RawEffects& effects, const AuxBuffers& buffers);

    /**
     * @brief Простой деноизинг без auxiliary buffers
     * @param effects Сырые эффекты от ray tracing
     * @return Деноизированное изображение
     */
    Vulkan::DenoisedImage denoise(const Vulkan::RawEffects& effects);

    /**
     * @brief Проверка инициализации
     * @return true если модуль инициализирован
     */
    bool isInitialized() const { return initialized; }

  private:
    // OptiX ресурсы (будут добавлены на этапе 5)
    void* denoiser = nullptr;  // OptixDenoiser
    std::unique_ptr<AutoencoderModel> model;

    bool initialized = false;

    /**
     * @brief Инициализация OptiX denoiser
     */
    bool initOptiXDenoiser();

    /**
     * @brief Освобождение OptiX ресурсов
     */
    void cleanupOptiX();

    // Запрет копирования
    DenoiseModule(const DenoiseModule&) = delete;
    DenoiseModule& operator=(const DenoiseModule&) = delete;
};

}  // namespace SpectraForge::OptiX
