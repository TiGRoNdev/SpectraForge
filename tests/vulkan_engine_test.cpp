/**
 * @file vulkan_engine_test.cpp
 * @brief Комплексное тестирование VulkanEngine
 * 
 * Покрывает все методы VulkanEngine для достижения 100% coverage
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/VulkanEngine.h"
#include "SpectraForge/Vulkan/VulkanRenderer.h"
#include "SpectraForge/Vulkan/SceneManager.h"
#include "SpectraForge/Vulkan/ResourceManager.h"
#include "SpectraForge/Vulkan/HardwareDetector.h"
#include <vulkan/vulkan.hpp>
#include <memory>

using namespace SpectraForge;
using namespace SpectraForge::Vulkan;

// ============================================================================
// Mock Vulkan Instance Helper
// ============================================================================

class MockVulkanInstanceForEngine {
public:
    static vk::Instance createMockInstance() {
        try {
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "VulkanEngine Test";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SpectraForge Test";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;

            return vk::createInstance(createInfo);
        } catch (const std::exception& e) {
            std::cerr << "[MockVulkanInstanceForEngine] Failed to create instance: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static void destroyMockInstance(vk::Instance instance) {
        if (instance) {
            instance.destroy();
        }
    }
};

// ============================================================================
// Test Fixture
// ============================================================================

class VulkanEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        instance = MockVulkanInstanceForEngine::createMockInstance();
        if (!instance) {
            GTEST_SKIP() << "Vulkan не доступен на этой системе";
        }

        engine = std::make_unique<VulkanEngine>();
    }

    void TearDown() override {
        engine.reset();
        if (instance) {
            MockVulkanInstanceForEngine::destroyMockInstance(instance);
        }
    }

    vk::Instance instance;
    std::unique_ptr<VulkanEngine> engine;
};

// ============================================================================
// Constructor/Destructor Tests
// ============================================================================

TEST(VulkanEngineBasicTest, ConstructorTest) {
    // Arrange & Act
    VulkanEngine engine;

    // Assert
    EXPECT_NO_THROW({
        VulkanEngine temp;
    });
}

TEST(VulkanEngineBasicTest, DestructorTest) {
    // Arrange
    auto engine = std::make_unique<VulkanEngine>();

    // Act & Assert
    EXPECT_NO_THROW({
        engine.reset();
    });
}

TEST(VulkanEngineBasicTest, DestructorAfterInitTest) {
    // Arrange
    vk::Instance instance = MockVulkanInstanceForEngine::createMockInstance();
    if (!instance) {
        GTEST_SKIP() << "Vulkan не доступен";
    }

    auto engine = std::make_unique<VulkanEngine>();
    engine->init(instance);

    // Act & Assert
    EXPECT_NO_THROW({
        engine.reset();
    });

    MockVulkanInstanceForEngine::destroyMockInstance(instance);
}

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(VulkanEngineTest, InitSuccessTest) {
    // Arrange: instance создан в SetUp

    // Act
    bool result = engine->init(instance);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(engine->isInitialized());
}

TEST(VulkanEngineBasicTest, InitWithNullInstanceTest) {
    // Arrange
    VulkanEngine engine;
    vk::Instance nullInstance;

    // Act
    bool result = engine.init(nullInstance);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(VulkanEngineTest, InitMultipleTimesTest) {
    // Arrange
    engine->init(instance);

    // Act: Повторная инициализация
    bool secondInit = engine->init(instance);

    // Assert
    EXPECT_TRUE(secondInit);
}

TEST_F(VulkanEngineTest, IsInitializedTest) {
    // Arrange
    EXPECT_FALSE(engine->isInitialized());

    // Act
    engine->init(instance);

    // Assert
    EXPECT_TRUE(engine->isInitialized());
}

TEST_F(VulkanEngineTest, IsInitializedAfterShutdownTest) {
    // Arrange
    engine->init(instance);
    EXPECT_TRUE(engine->isInitialized());

    // Act
    engine->shutdown();

    // Assert
    EXPECT_FALSE(engine->isInitialized());
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST_F(VulkanEngineTest, ShutdownTest) {
    // Arrange
    engine->init(instance);

    // Act & Assert
    EXPECT_NO_THROW({
        engine->shutdown();
    });
}

TEST_F(VulkanEngineTest, ShutdownWithoutInitTest) {
    // Arrange: engine не инициализирован

    // Act & Assert
    EXPECT_NO_THROW({
        engine->shutdown();
    });
}

TEST_F(VulkanEngineTest, MultipleShutdownsTest) {
    // Arrange
    engine->init(instance);
    engine->shutdown();

    // Act & Assert
    EXPECT_NO_THROW({
        engine->shutdown();
    });
}

// ============================================================================
// Component Access Tests
// ============================================================================

TEST_F(VulkanEngineTest, GetRendererTest) {
    // Arrange
    engine->init(instance);

    // Act
    VulkanRenderer* renderer = engine->getRenderer();

    // Assert
    EXPECT_NE(renderer, nullptr);
}

TEST_F(VulkanEngineTest, GetRendererBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    VulkanRenderer* renderer = engine->getRenderer();

    // Assert
    EXPECT_EQ(renderer, nullptr);
}

TEST_F(VulkanEngineTest, GetSceneManagerTest) {
    // Arrange
    engine->init(instance);

    // Act
    SceneManager* sceneManager = engine->getSceneManager();

    // Assert
    EXPECT_NE(sceneManager, nullptr);
}

TEST_F(VulkanEngineTest, GetSceneManagerBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    SceneManager* sceneManager = engine->getSceneManager();

    // Assert
    EXPECT_EQ(sceneManager, nullptr);
}

TEST_F(VulkanEngineTest, GetResourceManagerTest) {
    // Arrange
    engine->init(instance);

    // Act
    ResourceManager* resourceManager = engine->getResourceManager();

    // Assert
    EXPECT_NE(resourceManager, nullptr);
}

TEST_F(VulkanEngineTest, GetResourceManagerBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    ResourceManager* resourceManager = engine->getResourceManager();

    // Assert
    EXPECT_EQ(resourceManager, nullptr);
}

TEST_F(VulkanEngineTest, GetHardwareDetectorTest) {
    // Arrange
    engine->init(instance);

    // Act
    HardwareDetector* hardwareDetector = engine->getHardwareDetector();

    // Assert
    EXPECT_NE(hardwareDetector, nullptr);
}

TEST_F(VulkanEngineTest, GetHardwareDetectorBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    HardwareDetector* hardwareDetector = engine->getHardwareDetector();

    // Assert
    EXPECT_EQ(hardwareDetector, nullptr);
}

// ============================================================================
// Vulkan Object Access Tests
// ============================================================================

TEST_F(VulkanEngineTest, GetInstanceTest) {
    // Arrange
    engine->init(instance);

    // Act
    vk::Instance retrievedInstance = engine->getInstance();

    // Assert
    EXPECT_EQ(retrievedInstance, instance);
}

TEST_F(VulkanEngineTest, GetInstanceBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    vk::Instance retrievedInstance = engine->getInstance();

    // Assert
    EXPECT_FALSE(retrievedInstance);
}

TEST_F(VulkanEngineTest, GetPhysicalDeviceTest) {
    // Arrange
    engine->init(instance);

    // Act
    vk::PhysicalDevice physicalDevice = engine->getPhysicalDevice();

    // Assert
    EXPECT_TRUE(physicalDevice);
}

TEST_F(VulkanEngineTest, GetPhysicalDeviceBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    vk::PhysicalDevice physicalDevice = engine->getPhysicalDevice();

    // Assert
    EXPECT_FALSE(physicalDevice);
}

TEST_F(VulkanEngineTest, GetDeviceTest) {
    // Arrange
    engine->init(instance);

    // Act
    vk::Device device = engine->getDevice();

    // Assert
    EXPECT_TRUE(device);
}

TEST_F(VulkanEngineTest, GetDeviceBeforeInitTest) {
    // Arrange: engine не инициализирован

    // Act
    vk::Device device = engine->getDevice();

    // Assert
    EXPECT_FALSE(device);
}

// ============================================================================
// Render Frame Tests
// ============================================================================

TEST_F(VulkanEngineTest, RenderFrameTest) {
    // Arrange
    engine->init(instance);

    CameraParams params;
    params.position = glm::vec3(0.0f, 0.0f, 5.0f);
    params.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    params.fov = 45.0f;
    params.nearPlane = 0.1f;
    params.farPlane = 100.0f;

    // Act & Assert
    EXPECT_NO_THROW({
        engine->renderFrame(params);
    });
}

TEST_F(VulkanEngineTest, RenderFrameBeforeInitTest) {
    // Arrange: engine не инициализирован
    CameraParams params;

    // Act & Assert
    EXPECT_THROW({
        engine->renderFrame(params);
    }, std::exception);
}

TEST_F(VulkanEngineTest, RenderFrameMultipleTimesTest) {
    // Arrange
    engine->init(instance);

    CameraParams params;
    params.position = glm::vec3(0.0f, 0.0f, 5.0f);
    params.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    params.fov = 45.0f;
    params.nearPlane = 0.1f;
    params.farPlane = 100.0f;

    // Act: Рендерим несколько кадров
    for (int i = 0; i < 10; i++) {
        EXPECT_NO_THROW({
            engine->renderFrame(params);
        });
    }
}

TEST_F(VulkanEngineTest, RenderFrameDifferentCameraParametersTest) {
    // Arrange
    engine->init(instance);

    // Act & Assert: Тестируем разные параметры камеры
    std::vector<CameraParams> cameraConfigs;
    
    // Wide FOV
    CameraParams wideFov;
    wideFov.fov = 90.0f;
    wideFov.nearPlane = 0.1f;
    wideFov.farPlane = 100.0f;
    cameraConfigs.push_back(wideFov);

    // Narrow FOV
    CameraParams narrowFov;
    narrowFov.fov = 30.0f;
    narrowFov.nearPlane = 0.1f;
    narrowFov.farPlane = 100.0f;
    cameraConfigs.push_back(narrowFov);

    // Far plane
    CameraParams farPlane;
    farPlane.fov = 45.0f;
    farPlane.nearPlane = 0.1f;
    farPlane.farPlane = 10000.0f;
    cameraConfigs.push_back(farPlane);

    for (const auto& params : cameraConfigs) {
        EXPECT_NO_THROW({
            engine->renderFrame(params);
        });
    }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(VulkanEngineTest, FullEngineWorkflowTest) {
    // Arrange & Act & Assert: Полный workflow
    
    // 1. Инициализация
    EXPECT_TRUE(engine->init(instance));
    EXPECT_TRUE(engine->isInitialized());

    // 2. Проверка компонентов
    EXPECT_NE(engine->getRenderer(), nullptr);
    EXPECT_NE(engine->getSceneManager(), nullptr);
    EXPECT_NE(engine->getResourceManager(), nullptr);
    EXPECT_NE(engine->getHardwareDetector(), nullptr);

    // 3. Рендеринг нескольких кадров
    CameraParams params;
    params.position = glm::vec3(0.0f, 0.0f, 5.0f);
    params.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    params.fov = 45.0f;
    params.nearPlane = 0.1f;
    params.farPlane = 100.0f;

    for (int i = 0; i < 5; i++) {
        engine->renderFrame(params);
    }

    // 4. Shutdown
    engine->shutdown();
    EXPECT_FALSE(engine->isInitialized());

    // Все должно пройти успешно
    SUCCEED();
}

TEST_F(VulkanEngineTest, ComponentInteractionTest) {
    // Arrange
    engine->init(instance);

    // Act: Проверяем взаимодействие компонентов
    HardwareDetector* hwDetector = engine->getHardwareDetector();
    ResourceManager* resMgr = engine->getResourceManager();
    SceneManager* sceneMgr = engine->getSceneManager();
    VulkanRenderer* renderer = engine->getRenderer();

    // Assert: Все компоненты должны быть валидны
    EXPECT_NE(hwDetector, nullptr);
    EXPECT_NE(resMgr, nullptr);
    EXPECT_NE(sceneMgr, nullptr);
    EXPECT_NE(renderer, nullptr);

    // Проверяем что HardwareDetector работает
    std::string deviceName = hwDetector->getDeviceName();
    EXPECT_FALSE(deviceName.empty());
}

TEST_F(VulkanEngineTest, MultipleReinitializationsTest) {
    // Arrange & Act: Несколько циклов init/shutdown
    for (int i = 0; i < 3; i++) {
        EXPECT_TRUE(engine->init(instance));
        EXPECT_TRUE(engine->isInitialized());

        // Рендерим кадр
        CameraParams params;
        params.fov = 45.0f;
        params.nearPlane = 0.1f;
        params.farPlane = 100.0f;
        engine->renderFrame(params);

        engine->shutdown();
        EXPECT_FALSE(engine->isInitialized());
    }

    SUCCEED();
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(VulkanEngineTest, InitPerformanceTest) {
    // Arrange
    auto start = std::chrono::high_resolution_clock::now();

    // Act
    engine->init(instance);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Инициализация должна занимать меньше 2 секунд
    EXPECT_LT(duration.count(), 2000);
}

TEST_F(VulkanEngineTest, RenderFramePerformanceTest) {
    // Arrange
    engine->init(instance);

    CameraParams params;
    params.fov = 45.0f;
    params.nearPlane = 0.1f;
    params.farPlane = 100.0f;

    auto start = std::chrono::high_resolution_clock::now();

    // Act: Рендерим 60 кадров
    for (int i = 0; i < 60; i++) {
        engine->renderFrame(params);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: 60 кадров должны отрендериться меньше чем за 5 секунд
    EXPECT_LT(duration.count(), 5000);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(VulkanEngineTest, InvalidCameraParametersTest) {
    // Arrange
    engine->init(instance);

    CameraParams invalidParams;
    invalidParams.fov = 0.0f;  // Невалидный FOV
    invalidParams.nearPlane = 0.0f;
    invalidParams.farPlane = 0.0f;

    // Act & Assert: Должен обработать невалидные параметры
    EXPECT_NO_THROW({
        engine->renderFrame(invalidParams);
    });
}

TEST_F(VulkanEngineTest, NegativeClipPlanesTest) {
    // Arrange
    engine->init(instance);

    CameraParams params;
    params.fov = 45.0f;
    params.nearPlane = -1.0f;  // Отрицательный near plane
    params.farPlane = -100.0f;  // Отрицательный far plane

    // Act & Assert
    EXPECT_NO_THROW({
        engine->renderFrame(params);
    });
}

TEST_F(VulkanEngineTest, ExtremeFOVTest) {
    // Arrange
    engine->init(instance);

    // Act & Assert: Очень маленький FOV
    CameraParams smallFov;
    smallFov.fov = 1.0f;
    smallFov.nearPlane = 0.1f;
    smallFov.farPlane = 100.0f;

    EXPECT_NO_THROW({
        engine->renderFrame(smallFov);
    });

    // Очень большой FOV
    CameraParams largeFov;
    largeFov.fov = 179.0f;
    largeFov.nearPlane = 0.1f;
    largeFov.farPlane = 100.0f;

    EXPECT_NO_THROW({
        engine->renderFrame(largeFov);
    });
}

// ============================================================================
// State Tests
// ============================================================================

TEST_F(VulkanEngineTest, EngineStateConsistencyTest) {
    // Arrange
    EXPECT_FALSE(engine->isInitialized());

    // Act & Assert: Проверяем состояние на каждом этапе
    engine->init(instance);
    EXPECT_TRUE(engine->isInitialized());

    // Компоненты должны быть доступны
    EXPECT_NE(engine->getRenderer(), nullptr);
    EXPECT_NE(engine->getSceneManager(), nullptr);
    EXPECT_NE(engine->getResourceManager(), nullptr);
    EXPECT_NE(engine->getHardwareDetector(), nullptr);

    engine->shutdown();
    EXPECT_FALSE(engine->isInitialized());
}

TEST_F(VulkanEngineTest, VulkanObjectsConsistencyTest) {
    // Arrange
    engine->init(instance);

    // Act
    vk::Instance inst1 = engine->getInstance();
    vk::Instance inst2 = engine->getInstance();

    vk::PhysicalDevice physDev1 = engine->getPhysicalDevice();
    vk::PhysicalDevice physDev2 = engine->getPhysicalDevice();

    vk::Device dev1 = engine->getDevice();
    vk::Device dev2 = engine->getDevice();

    // Assert: Объекты должны быть одинаковыми при повторных вызовах
    EXPECT_EQ(inst1, inst2);
    EXPECT_EQ(physDev1, physDev2);
    EXPECT_EQ(dev1, dev2);
}

// ============================================================================
// Copy Protection Tests
// ============================================================================

TEST(VulkanEngineCopyTest, CopyConstructorDeletedTest) {
    // Assert: Проверяем что конструктор копирования удален
    EXPECT_FALSE(std::is_copy_constructible<VulkanEngine>::value);
}

TEST(VulkanEngineCopyTest, CopyAssignmentDeletedTest) {
    // Assert: Проверяем что оператор присваивания удален
    EXPECT_FALSE(std::is_copy_assignable<VulkanEngine>::value);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(VulkanEngineTest, GetComponentsAfterShutdownTest) {
    // Arrange
    engine->init(instance);
    engine->shutdown();

    // Act & Assert: Доступ к компонентам после shutdown
    EXPECT_NO_THROW({
        engine->getRenderer();
        engine->getSceneManager();
        engine->getResourceManager();
        engine->getHardwareDetector();
    });
}

TEST_F(VulkanEngineTest, RenderAfterShutdownTest) {
    // Arrange
    engine->init(instance);
    engine->shutdown();

    CameraParams params;
    params.fov = 45.0f;
    params.nearPlane = 0.1f;
    params.farPlane = 100.0f;

    // Act & Assert: Рендеринг после shutdown должен выбросить исключение
    EXPECT_THROW({
        engine->renderFrame(params);
    }, std::exception);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
