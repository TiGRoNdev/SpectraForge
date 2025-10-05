
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

// Forward declarations
namespace SpectraForge::Vulkan {
struct Gaussians;
}

namespace SpectraForge::Vulkan {

/**
 * @brief Данные сцены для загрузки
 */
struct SceneData {
    std::string scenePath;
    std::vector<std::string> meshPaths;
    std::vector<std::string> texturePaths;

    // ВРЕМЕННО: Параметр для уменьшения количества треугольников для стабилизации производительности
    // step = 1: все треугольники, step = 5000: каждый 5000-й треугольник
    size_t triangleStep = 1;

    // TODO: Добавить больше данных сцены
};

/**
 * @brief Мульти-вид изображения для Gaussian Splatting
 */
struct MultiViewImages {
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;
    uint32_t viewCount = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

/**
 * @brief Динамические элементы сцены
 */
struct DynamicElements {
    std::vector<uint32_t> movingObjects;
    std::vector<uint32_t> animatedObjects;
    std::vector<uint32_t> particleSystems;
    float deltaTime = 0.0f;
};

/**
 * @brief Менеджер сцены для 4D движка
 *
 * Управляет загрузкой сцены, динамическими объектами и предоставляет
 * данные для рендеринга согласно UML архитектуре.
 */
class SceneManager {
  public:
    /**
     * @brief Конструктор
     */
    SceneManager();

    /**
     * @brief Деструктор
     */
    ~SceneManager();

    /**
     * @brief Инициализация менеджера сцены
     * @return true если инициализация успешна
     */
    bool init();

    /**
     * @brief Завершение работы менеджера
     */
    void shutdown();

    /**
     * @brief Загрузка сцены
     * @param data Данные сцены
     * @return true если загрузка успешна
     */
    bool loadScene(const SceneData& data);

    /**
     * @brief Обновление динамических элементов
     * @param deltaTime Время с последнего обновления
     */
    void updateDynamics(float deltaTime = 0.016f);

    /**
     * @brief Получение гауссианов для рендеринга
     * @return Гауссианы сцены
     */
    Gaussians getGaussians() const;

    /**
     * @brief Получение мульти-вид изображений
     * @return Изображения для Gaussian Splatting
     */
    const MultiViewImages& getMultiViewImages() const { return images; }

    /**
     * @brief Получение динамических элементов
     * @return Динамические элементы сцены
     */
    const DynamicElements& getDynamicElements() const { return elements; }

    /**
     * @brief Проверка загрузки сцены
     * @return true если сцена загружена
     */
    bool isSceneLoaded() const { return sceneLoaded; }

    /**
     * @brief Очистка сцены
     */
    void clearScene();

    /**
     * @brief Добавление объекта в сцену
     * @param objectPath Путь к объекту
     * @return ID добавленного объекта
     */
    uint32_t addObject(const std::string& objectPath);

    /**
     * @brief Удаление объекта из сцены
     * @param objectId ID объекта
     */
    void removeObject(uint32_t objectId);

    /**
     * @brief Получение количества объектов в сцене
     * @return Количество объектов
     */
    uint32_t getObjectCount() const { return objectCount; }

  private:
    MultiViewImages images;
    DynamicElements elements;

    bool initialized = false;
    bool sceneLoaded = false;
    uint32_t objectCount = 0;
    uint32_t nextObjectId = 1;

    // Внутренние структуры данных для объектов сцены
    struct SceneObject {
        uint32_t id;
        std::string path;
        bool isDynamic;
        // TODO: Добавить трансформации, материалы и т.д.
    };

    std::vector<SceneObject> sceneObjects;

    /**
     * @brief Загрузка мешей из файлов
     * @param meshPaths Пути к мешам
     * @return true если загрузка успешна
     */
    bool loadMeshes(const std::vector<std::string>& meshPaths);

    /**
     * @brief Загрузка текстур из файлов
     * @param texturePaths Пути к текстурам
     * @return true если загрузка успешна
     */
    bool loadTextures(const std::vector<std::string>& texturePaths);

    /**
     * @brief Генерация гауссианов из загруженных данных
     */
    void generateGaussians();

    /**
     * @brief Обновление анимаций
     * @param deltaTime Время с последнего обновления
     */
    void updateAnimations(float deltaTime);

    /**
     * @brief Обновление физики
     * @param deltaTime Время с последнего обновления
     */
    void updatePhysics(float deltaTime);

    // Запрет копирования
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
};

}  // namespace SpectraForge::Vulkan
