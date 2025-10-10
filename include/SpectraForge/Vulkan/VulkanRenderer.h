
#pragma once

#include <optional>

#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
#include <vulkan/vulkan.hpp>
#endif

// Forward declarations
namespace SpectraForge {
struct CameraParams;

namespace Vulkan {
class ResourceManager;
}

}  // namespace SpectraForge

namespace SpectraForge::Vulkan {

// Не используем using namespace во избежание загрязнения пространства имен

/**
 * @brief Структуры данных для рендеринга
 */
struct Gaussians {
    uint32_t count = 0;
};

struct PrimaryImage {
#ifdef HyperEngine_ENABLE_VULKAN
    vk::Image image;
    vk::ImageView imageView;
#else
    void* image = nullptr;
    void* imageView = nullptr;
#endif
    uint32_t width = 0;
    uint32_t height = 0;
};

struct RenderSettings {
    uint32_t width = 0;
    uint32_t height = 0;
};

/**
 * @brief Упрощенный VulkanRenderer, отвечающий только за первичную растеризацию.
 *
 * Класс содержит минимальный набор зависимостей и избегает преждевременных заглушек
 * для будущих стадий конвейера. Дополнительные функции будут добавляться по мере
 * появления реальных требований.
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
    bool init(
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
        vk::Device device,
#else
        void* device,
#endif
        ResourceManager* resourceManager);

    /**
     * @brief Завершение работы рендерера
     */
    void shutdown();

    /**
     * @brief Первичная растеризация гауссианов
     * @param gaussians Гауссианы для рендеринга
     * @return Первичное изображение
     * @throws std::invalid_argument если данные гауссианов невалидны
     */
    PrimaryImage rasterizePrimary(const Gaussians& gaussians);

    /**
     * @brief Настройка параметров рендеринга
     * @param settings Желаемые параметры вывода
     */
    void setRenderSettings(const RenderSettings& settings);

    /**
     * @brief Проверка инициализации
     * @return true если рендерер инициализирован
     */
    bool isInitialized() const { return initialized; }

  private:

#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::Device device{};
#else
    void* device = nullptr;
#endif

    ResourceManager* resourceManager = nullptr;

    std::optional<RenderSettings> renderSettings_{};

    bool initialized = false;

    // Запрет копирования
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
};

}  // namespace SpectraForge::Vulkan
