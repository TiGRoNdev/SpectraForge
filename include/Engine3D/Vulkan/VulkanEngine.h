#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Forward declarations
namespace Engine3D {
    namespace Vulkan {
        class VulkanRenderer;
        class SceneManager;
        class ResourceManager;
        class HardwareDetector;
    }
    
    struct CameraParams {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec3 position;
        glm::vec3 direction;
        float fov;
        float nearPlane;
        float farPlane;
    };
}

namespace Engine3D::Vulkan {

/**
 * @brief Главный класс Vulkan-based движка
 * 
 * Координирует работу всех подсистем согласно UML архитектуре:
 * - Renderer для рендеринга
 * - SceneManager для управления сценой
 * - ResourceManager для управления ресурсами
 * - HardwareDetector для определения возможностей железа
 */
class VulkanEngine {
public:
    /**
     * @brief Конструктор
     */
    VulkanEngine();
    
    /**
     * @brief Деструктор
     */
    ~VulkanEngine();
    
    /**
     * @brief Инициализация движка
     * @param instance Vulkan instance
     * @return true если инициализация успешна
     */
    bool init(vk::Instance instance);
    
    /**
     * @brief Рендеринг кадра
     * @param params Параметры камеры
     */
    void renderFrame(const CameraParams& params);
    
    /**
     * @brief Завершение работы движка
     */
    void shutdown();
    
    /**
     * @brief Получить рендерер
     * @return Указатель на рендерер
     */
    VulkanRenderer* getRenderer() const { return renderer.get(); }
    
    /**
     * @brief Получить менеджер сцены
     * @return Указатель на менеджер сцены
     */
    SceneManager* getSceneManager() const { return sceneManager.get(); }
    
    /**
     * @brief Получить менеджер ресурсов
     * @return Указатель на менеджер ресурсов
     */
    ResourceManager* getResourceManager() const { return resourceManager.get(); }
    
    /**
     * @brief Получить детектор железа
     * @return Указатель на детектор железа
     */
    HardwareDetector* getHardwareDetector() const { return hardwareDetector.get(); }
    
    /**
     * @brief Проверка инициализации
     * @return true если движок инициализирован
     */
    bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<HardwareDetector> hardwareDetector;
    
    bool initialized = false;
    
    // Запрет копирования
    VulkanEngine(const VulkanEngine&) = delete;
    VulkanEngine& operator=(const VulkanEngine&) = delete;
};

} // namespace Engine3D::Vulkan
