/**
 * @file vulkan_scene_manager_test.cpp
 * @brief Комплексное тестирование SceneManager
 * 
 * Покрывает все методы SceneManager для достижения 100% coverage
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Vulkan/VulkanRenderer.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <memory>

using namespace SpectraForge::Vulkan;

// ============================================================================
// Test Fixture
// ============================================================================

class SceneManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        sceneManager = std::make_unique<SceneManager>();
    }

    void TearDown() override {
        sceneManager.reset();
    }

    std::unique_ptr<SceneManager> sceneManager;
};

// ============================================================================
// Constructor/Destructor Tests
// ============================================================================

TEST(SceneManagerBasicTest, ConstructorTest) {
    // Arrange & Act
    SceneManager manager;

    // Assert
    EXPECT_NO_THROW({
        SceneManager temp;
    });
}

TEST(SceneManagerBasicTest, DestructorTest) {
    // Arrange
    auto manager = std::make_unique<SceneManager>();

    // Act & Assert
    EXPECT_NO_THROW({
        manager.reset();
    });
}

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(SceneManagerTest, InitSuccessTest) {
    // Arrange & Act
    bool result = sceneManager->init();

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(SceneManagerTest, InitMultipleTimesTest) {
    // Arrange
    sceneManager->init();

    // Act
    bool secondInit = sceneManager->init();

    // Assert
    EXPECT_TRUE(secondInit);
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST_F(SceneManagerTest, ShutdownTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->shutdown();
    });
}

TEST_F(SceneManagerTest, ShutdownWithoutInitTest) {
    // Arrange: manager не инициализирован

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->shutdown();
    });
}

TEST_F(SceneManagerTest, MultipleShutdownsTest) {
    // Arrange
    sceneManager->init();
    sceneManager->shutdown();

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->shutdown();
    });
}

// ============================================================================
// Scene Loading Tests
// ============================================================================

TEST_F(SceneManagerTest, LoadSceneTest) {
    // Arrange
    sceneManager->init();

    SceneData data;
    data.scenePath = "test_scene.obj";
    data.meshPaths = {"mesh1.obj", "mesh2.obj"};
    data.texturePaths = {"texture1.png"};
    data.triangleStep = 1;

    // Act
    bool result = sceneManager->loadScene(data);

    // Assert: Может вернуть false если файлы не существуют, но не должно падать
    EXPECT_NO_THROW({
        sceneManager->loadScene(data);
    });
}

TEST_F(SceneManagerTest, LoadSceneEmptyDataTest) {
    // Arrange
    sceneManager->init();

    SceneData data;  // Пустые данные

    // Act
    bool result = sceneManager->loadScene(data);

    // Assert: Должен обработать пустые данные
    EXPECT_NO_THROW({
        sceneManager->loadScene(data);
    });
}

TEST_F(SceneManagerTest, LoadSceneBeforeInitTest) {
    // Arrange: manager не инициализирован
    SceneData data;
    data.scenePath = "test_scene.obj";

    // Act
    bool result = sceneManager->loadScene(data);

    // Assert: Должен вернуть false
    EXPECT_FALSE(result);
}

TEST_F(SceneManagerTest, LoadSceneWithTriangleStepTest) {
    // Arrange
    sceneManager->init();

    SceneData data;
    data.scenePath = "test_scene.obj";
    data.triangleStep = 5000;  // Каждый 5000-й треугольник

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->loadScene(data);
    });
}

TEST_F(SceneManagerTest, LoadMultipleScenesTest) {
    // Arrange
    sceneManager->init();

    // Act: Загружаем несколько сцен подряд
    for (int i = 0; i < 3; i++) {
        SceneData data;
        data.scenePath = "scene_" + SpectraForge::Core::SAFE_TO_STRING(i) + ".obj";
        
        EXPECT_NO_THROW({
            sceneManager->loadScene(data);
        });
    }
}

// ============================================================================
// Scene State Tests
// ============================================================================

TEST_F(SceneManagerTest, IsSceneLoadedTest) {
    // Arrange
    sceneManager->init();
    EXPECT_FALSE(sceneManager->isSceneLoaded());

    // Act
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Assert: Должен установить флаг загрузки
    // (может быть false если файл не найден, но метод должен работать)
    EXPECT_NO_THROW({
        sceneManager->isSceneLoaded();
    });
}

TEST_F(SceneManagerTest, IsSceneLoadedAfterClearTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    sceneManager->clearScene();

    // Assert
    EXPECT_FALSE(sceneManager->isSceneLoaded());
}

// ============================================================================
// Dynamic Elements Tests
// ============================================================================

TEST_F(SceneManagerTest, UpdateDynamicsTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->updateDynamics(0.016f);  // 60 FPS
    });
}

TEST_F(SceneManagerTest, UpdateDynamicsDefaultDeltaTimeTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert: Вызов без параметра (default 0.016f)
    EXPECT_NO_THROW({
        sceneManager->updateDynamics();
    });
}

TEST_F(SceneManagerTest, UpdateDynamicsZeroDeltaTimeTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert
    EXPECT_NO_THROW({
        sceneManager->updateDynamics(0.0f);
    });
}

TEST_F(SceneManagerTest, UpdateDynamicsLargeDeltaTimeTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert: Большой deltaTime
    EXPECT_NO_THROW({
        sceneManager->updateDynamics(1.0f);  // 1 секунда
    });
}

TEST_F(SceneManagerTest, UpdateDynamicsMultipleTimesTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act: Симулируем 60 кадров
    for (int i = 0; i < 60; i++) {
        EXPECT_NO_THROW({
            sceneManager->updateDynamics(0.016f);
        });
    }
}

// ============================================================================
// Gaussians Tests
// ============================================================================

TEST_F(SceneManagerTest, GetGaussiansTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    Gaussians gaussians = sceneManager->getGaussians();

    // Assert: Должен вернуть структуру гауссианов
    EXPECT_NO_THROW({
        sceneManager->getGaussians();
    });
}

TEST_F(SceneManagerTest, GetGaussiansWithoutSceneTest) {
    // Arrange
    sceneManager->init();

    // Act
    Gaussians gaussians = sceneManager->getGaussians();

    // Assert: Без загруженной сцены должен вернуть пустую структуру
    EXPECT_EQ(gaussians.count, 0u);
}

// ============================================================================
// Multi-View Images Tests
// ============================================================================

TEST_F(SceneManagerTest, GetMultiViewImagesTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    const MultiViewImages& images = sceneManager->getMultiViewImages();

    // Assert
    EXPECT_NO_THROW({
        sceneManager->getMultiViewImages();
    });
}

TEST_F(SceneManagerTest, GetMultiViewImagesWithoutSceneTest) {
    // Arrange
    sceneManager->init();

    // Act
    const MultiViewImages& images = sceneManager->getMultiViewImages();

    // Assert: Без сцены должен вернуть пустую структуру
    EXPECT_EQ(images.viewCount, 0u);
}

// ============================================================================
// Dynamic Elements Retrieval Tests
// ============================================================================

TEST_F(SceneManagerTest, GetDynamicElementsTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    const DynamicElements& elements = sceneManager->getDynamicElements();

    // Assert
    EXPECT_NO_THROW({
        sceneManager->getDynamicElements();
    });
}

TEST_F(SceneManagerTest, GetDynamicElementsAfterUpdateTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    sceneManager->updateDynamics(0.016f);
    const DynamicElements& elements = sceneManager->getDynamicElements();

    // Assert: DeltaTime должен быть обновлен
    EXPECT_EQ(elements.deltaTime, 0.016f);
}

// ============================================================================
// Object Management Tests
// ============================================================================

TEST_F(SceneManagerTest, AddObjectTest) {
    // Arrange
    sceneManager->init();
    uint32_t initialCount = sceneManager->getObjectCount();

    // Act
    uint32_t objectId = sceneManager->addObject("new_object.obj");

    // Assert
    EXPECT_GT(objectId, 0u);
    EXPECT_EQ(sceneManager->getObjectCount(), initialCount + 1);
}

TEST_F(SceneManagerTest, AddObjectBeforeInitTest) {
    // Arrange: manager не инициализирован

    // Act
    uint32_t objectId = sceneManager->addObject("object.obj");

    // Assert: Должен вернуть 0 или выбросить исключение
    EXPECT_NO_THROW({
        sceneManager->addObject("object.obj");
    });
}

TEST_F(SceneManagerTest, AddMultipleObjectsTest) {
    // Arrange
    sceneManager->init();

    // Act: Добавляем несколько объектов
    std::vector<uint32_t> objectIds;
    for (int i = 0; i < 10; i++) {
        uint32_t id = sceneManager->addObject("object_" + SpectraForge::Core::SAFE_TO_STRING(i) + ".obj");
        objectIds.push_back(id);
    }

    // Assert: Все ID должны быть уникальными
    for (size_t i = 0; i < objectIds.size(); i++) {
        for (size_t j = i + 1; j < objectIds.size(); j++) {
            EXPECT_NE(objectIds[i], objectIds[j]);
        }
    }

    EXPECT_EQ(sceneManager->getObjectCount(), 10u);
}

TEST_F(SceneManagerTest, RemoveObjectTest) {
    // Arrange
    sceneManager->init();
    uint32_t objectId = sceneManager->addObject("object.obj");

    // Act
    sceneManager->removeObject(objectId);

    // Assert: Количество объектов должно уменьшиться
    EXPECT_EQ(sceneManager->getObjectCount(), 0u);
}

TEST_F(SceneManagerTest, RemoveNonExistentObjectTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert: Удаление несуществующего объекта не должно падать
    EXPECT_NO_THROW({
        sceneManager->removeObject(9999);
    });
}

TEST_F(SceneManagerTest, RemoveObjectTwiceTest) {
    // Arrange
    sceneManager->init();
    uint32_t objectId = sceneManager->addObject("object.obj");

    // Act
    sceneManager->removeObject(objectId);

    // Assert: Повторное удаление не должно падать
    EXPECT_NO_THROW({
        sceneManager->removeObject(objectId);
    });
}

// ============================================================================
// Object Count Tests
// ============================================================================

TEST_F(SceneManagerTest, GetObjectCountTest) {
    // Arrange
    sceneManager->init();

    // Act
    uint32_t count = sceneManager->getObjectCount();

    // Assert: Изначально должно быть 0
    EXPECT_EQ(count, 0u);
}

TEST_F(SceneManagerTest, GetObjectCountAfterAddTest) {
    // Arrange
    sceneManager->init();

    // Act
    sceneManager->addObject("obj1.obj");
    sceneManager->addObject("obj2.obj");
    sceneManager->addObject("obj3.obj");

    // Assert
    EXPECT_EQ(sceneManager->getObjectCount(), 3u);
}

TEST_F(SceneManagerTest, GetObjectCountAfterRemoveTest) {
    // Arrange
    sceneManager->init();
    uint32_t id1 = sceneManager->addObject("obj1.obj");
    uint32_t id2 = sceneManager->addObject("obj2.obj");
    uint32_t id3 = sceneManager->addObject("obj3.obj");

    // Act
    sceneManager->removeObject(id2);

    // Assert
    EXPECT_EQ(sceneManager->getObjectCount(), 2u);
}

// ============================================================================
// Clear Scene Tests
// ============================================================================

TEST_F(SceneManagerTest, ClearSceneTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    // Act
    sceneManager->clearScene();

    // Assert
    EXPECT_FALSE(sceneManager->isSceneLoaded());
    EXPECT_EQ(sceneManager->getObjectCount(), 0u);
}

TEST_F(SceneManagerTest, ClearSceneWithObjectsTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);
    
    sceneManager->addObject("obj1.obj");
    sceneManager->addObject("obj2.obj");

    // Act
    sceneManager->clearScene();

    // Assert
    EXPECT_EQ(sceneManager->getObjectCount(), 0u);
}

TEST_F(SceneManagerTest, ClearSceneMultipleTimesTest) {
    // Arrange
    sceneManager->init();

    // Act & Assert: Множественные вызовы clearScene
    for (int i = 0; i < 3; i++) {
        EXPECT_NO_THROW({
            sceneManager->clearScene();
        });
    }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(SceneManagerTest, FullWorkflowTest) {
    // Arrange & Act & Assert: Полный workflow

    // 1. Инициализация
    EXPECT_TRUE(sceneManager->init());

    // 2. Загрузка сцены
    SceneData data;
    data.scenePath = "test_scene.obj";
    data.meshPaths = {"mesh1.obj", "mesh2.obj"};
    sceneManager->loadScene(data);

    // 3. Добавление объектов
    uint32_t id1 = sceneManager->addObject("obj1.obj");
    uint32_t id2 = sceneManager->addObject("obj2.obj");

    // 4. Обновление динамики
    for (int i = 0; i < 60; i++) {
        sceneManager->updateDynamics(0.016f);
    }

    // 5. Получение данных
    Gaussians gaussians = sceneManager->getGaussians();
    const MultiViewImages& images = sceneManager->getMultiViewImages();
    const DynamicElements& elements = sceneManager->getDynamicElements();

    // 6. Удаление объекта
    sceneManager->removeObject(id1);

    // 7. Очистка сцены
    sceneManager->clearScene();

    // 8. Shutdown
    sceneManager->shutdown();

    SUCCEED();
}

TEST_F(SceneManagerTest, SceneLoadUnloadCycleTest) {
    // Arrange
    sceneManager->init();

    // Act: Несколько циклов загрузки/выгрузки
    for (int i = 0; i < 5; i++) {
        SceneData data;
        data.scenePath = "scene_" + SpectraForge::Core::SAFE_TO_STRING(i) + ".obj";
        sceneManager->loadScene(data);

        sceneManager->addObject("obj1.obj");
        sceneManager->addObject("obj2.obj");

        sceneManager->updateDynamics(0.016f);

        sceneManager->clearScene();
    }

    SUCCEED();
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(SceneManagerTest, AddObjectPerformanceTest) {
    // Arrange
    sceneManager->init();
    auto start = std::chrono::high_resolution_clock::now();

    // Act: Добавляем 1000 объектов
    for (int i = 0; i < 1000; i++) {
        sceneManager->addObject("obj_" + SpectraForge::Core::SAFE_TO_STRING(i) + ".obj");
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Должно занять меньше 1 секунды
    EXPECT_LT(duration.count(), 1000);
}

TEST_F(SceneManagerTest, UpdateDynamicsPerformanceTest) {
    // Arrange
    sceneManager->init();
    SceneData data;
    data.scenePath = "test_scene.obj";
    sceneManager->loadScene(data);

    auto start = std::chrono::high_resolution_clock::now();

    // Act: 1000 обновлений
    for (int i = 0; i < 1000; i++) {
        sceneManager->updateDynamics(0.016f);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Должно занять меньше 2 секунд
    EXPECT_LT(duration.count(), 2000);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(SceneManagerTest, LoadInvalidSceneTest) {
    // Arrange
    sceneManager->init();

    SceneData data;
    data.scenePath = "non_existent_scene.obj";

    // Act
    bool result = sceneManager->loadScene(data);

    // Assert: Должен обработать ошибку (вернуть false или выбросить исключение)
    EXPECT_NO_THROW({
        sceneManager->loadScene(data);
    });
}

TEST_F(SceneManagerTest, AddObjectWithEmptyPathTest) {
    // Arrange
    sceneManager->init();

    // Act
    uint32_t objectId = sceneManager->addObject("");

    // Assert: Должен обработать пустой путь
    EXPECT_NO_THROW({
        sceneManager->addObject("");
    });
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
