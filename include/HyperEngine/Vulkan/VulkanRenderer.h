#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

// Forward declarations
namespace HyperEngine {
struct CameraParams;

namespace Vulkan {
class ResourceManager;
}

namespace CUDA {
class FlashGSSplatter;
}

namespace OptiX {
class DenoiseModule;
class OptiXRayTracer;
}  // namespace OptiX

namespace Upscaling {
class Upscaler;
}
}  // namespace HyperEngine

namespace HyperEngine::Vulkan {

/**
 * @brief Структуры данных для рендеринга
 */
struct Gaussians {
    // TODO: Определить структуру гауссианов
    uint32_t count = 0;
};

struct PrimaryImage {
    vk::Image image;
    vk::ImageView imageView;
    uint32_t width;
    uint32_t height;
};

struct RawEffects {
    vk::Image reflections;
    vk::Image shadows;
    vk::Image globalIllumination;
};

struct DenoisedImage {
    vk::Image image;
    vk::ImageView imageView;
};

struct ResolutionTarget {
    uint32_t width;
    uint32_t height;
    float scaleFactor;
};

struct FinalImage {
    vk::Image image;
    vk::ImageView imageView;
};

/**
 * @brief Основной класс рендерера Vulkan
 *
 * Реализует гибридный рендеринг согласно UML архитектуре:
 * - Primary rasterization через FlashGS
 * - Secondary ray tracing через OptiX
 * - AI denoising
 * - Upscaling (DLSS/FSR)
 */
class VulkanRenderer {
  public:
    /**
     * @brief Конструктор
     */
    VulkanRenderer();

    /**
     * @brief Деструктор
     */
    ~VulkanRenderer();

    /**
     * @brief Инициализация рендерера
     * @param device Vulkan устройство
     * @param resourceManager Менеджер ресурсов
     * @return true если инициализация успешна
     */
    bool init(vk::Device device, ResourceManager* resourceManager);

    /**
     * @brief Завершение работы рендерера
     */
    void shutdown();

    /**
     * @brief Первичная растеризация гауссианов
     * @param gaussians Гауссианы для рендеринга
     * @return Первичное изображение
     */
    PrimaryImage rasterizePrimary(const Gaussians& gaussians);

    /**
     * @brief Вторичный ray tracing для эффектов
     * @param image Первичное изображение
     * @return Сырые эффекты
     */
    RawEffects rayTraceSecondary(const PrimaryImage& image);

    /**
     * @brief AI деноизинг
     * @param effects Сырые эффекты
     * @return Деноизированное изображение
     */
    DenoisedImage denoiseAI(const RawEffects& effects);

    /**
     * @brief Upscaling изображения
     * @param image Деноизированное изображение
     * @param target Целевое разрешение
     * @return Финальное изображение
     */
    FinalImage upscale(const DenoisedImage& image, const ResolutionTarget& target);

    /**
     * @brief Презентация финального изображения
     */
    void presentFinalImage();

    /**
     * @brief Проверка инициализации
     * @return true если рендерер инициализирован
     */
    bool isInitialized() const { return initialized; }

  private:
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::CommandPool commandPool;
    vk::SwapchainKHR swapchain;

    ResourceManager* resourceManager = nullptr;

    // Компоненты рендеринга (будут созданы на следующих этапах)
    std::unique_ptr<HyperEngine::CUDA::FlashGSSplatter> splatter;
    std::unique_ptr<HyperEngine::OptiX::OptiXRayTracer> rayTracer;
    std::unique_ptr<HyperEngine::OptiX::DenoiseModule> denoiseModule;
    std::unique_ptr<HyperEngine::Upscaling::Upscaler> upscaler;

    bool initialized = false;

    // Запрет копирования
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
};

}  // namespace HyperEngine::Vulkan

