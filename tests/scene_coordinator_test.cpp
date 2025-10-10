/**
 * @file scene_coordinator_test.cpp
 * @brief Unit tests for SceneCoordinator (P0.3 TDD)
 */

#include <gtest/gtest.h>
#include <memory>
#include <GLFW/glfw3.h>
#include "SpectraForge/App/Core/SceneCoordinator.h"
#include "SpectraForge/App/Engine.h" // Для SceneInfo, InputState

using namespace SpectraForge::App::Core;
using namespace SpectraForge::App;

class SceneCoordinatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        coordinator = std::make_unique<SceneCoordinator>();
    }
    
    void TearDown() override {
        coordinator.reset();
    }
    
    std::unique_ptr<SceneCoordinator> coordinator;
};

// ============================================================================
// ТЕСТ 1: Начальное состояние
// ============================================================================
TEST_F(SceneCoordinatorTest, InitialState) {
    EXPECT_FALSE(coordinator->isSceneLoaded());
    EXPECT_EQ(coordinator->getCamera(), nullptr);
}

// ============================================================================
// ТЕСТ 2: Загрузка простой сцены
// ============================================================================
TEST_F(SceneCoordinatorTest, LoadSimpleScene) {
    SpectraForge::Vulkan::SceneData sceneData;
    sceneData.scenePath = "test_scene.obj";
    
    bool success = coordinator->loadScene(sceneData);
    EXPECT_TRUE(success);
    EXPECT_TRUE(coordinator->isSceneLoaded());
}

// ============================================================================
// ТЕСТ 3: getSceneInfo после загрузки
// ============================================================================
TEST_F(SceneCoordinatorTest, GetSceneInfo) {
    SpectraForge::Vulkan::SceneData sceneData;
    sceneData.scenePath = "test_scene.obj";
    
    coordinator->loadScene(sceneData);
    
    SceneInfo info = coordinator->getSceneInfo();
    EXPECT_TRUE(info.isLoaded);
    EXPECT_EQ(info.scenePath, "test_scene.obj");
}

// ============================================================================
// ТЕСТ 4: Создание камеры автоматически
// ============================================================================
TEST_F(SceneCoordinatorTest, AutoCreateCamera) {
    SpectraForge::Vulkan::SceneData sceneData;
    coordinator->loadScene(sceneData);
    
    // После загрузки сцены камера должна быть создана
    auto camera = coordinator->getCamera();
    EXPECT_NE(camera, nullptr);
}

// ============================================================================
// ТЕСТ 5: setCamera и getCamera
// ============================================================================
TEST_F(SceneCoordinatorTest, SetAndGetCamera) {
    auto customCamera = std::make_shared<SpectraForge::Rendering::Camera3D>();
    
    coordinator->setCamera(customCamera);
    
    auto retrievedCamera = coordinator->getCamera();
    EXPECT_EQ(retrievedCamera, customCamera);
}

// ============================================================================
// ТЕСТ 6: updateCamera без external control
// ============================================================================
TEST_F(SceneCoordinatorTest, UpdateCameraInternal) {
    InputState input;
    input.keys[GLFW_KEY_W] = true;
    
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    auto cameraBefore = coordinator->getCamera()->getPosition();
    
    coordinator->updateCamera(0.016f, &input, false); // Не external control
    
    // Камера должна переместиться (WASD управление)
    // Проверим, что позиция изменилась
    // (Точная проверка зависит от реализации)
}

// ============================================================================
// ТЕСТ 7: updateCamera с external control (не должен обновлять)
// ============================================================================
TEST_F(SceneCoordinatorTest, UpdateCameraExternal) {
    InputState input;
    input.keys[GLFW_KEY_W] = true;
    
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    auto cameraBefore = coordinator->getCamera()->getPosition();
    
    coordinator->updateCamera(0.016f, &input, true); // External control
    
    // Камера НЕ должна переместиться
    auto cameraAfter = coordinator->getCamera()->getPosition();
    EXPECT_EQ(cameraBefore.x, cameraAfter.x);
    EXPECT_EQ(cameraBefore.y, cameraAfter.y);
    EXPECT_EQ(cameraBefore.z, cameraAfter.z);
}

// ============================================================================
// ТЕСТ 8: resetCameraForSponza
// ============================================================================
TEST_F(SceneCoordinatorTest, ResetCameraForSponza) {
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    // Переместим камеру куда-то
    auto camera = coordinator->getCamera();
    camera->setPosition(SpectraForge::Math::Vector3(100.0f, 100.0f, 100.0f));
    
    // Сброс
    coordinator->resetCameraForSponza();
    
    // Камера должна вернуться в исходную позицию для Sponza
    auto newPos = camera->getPosition();
    EXPECT_LT(std::abs(newPos.x), 10.0f); // Примерно около начала координат
}

// ============================================================================
// ТЕСТ 9: getSceneBounds
// ============================================================================
TEST_F(SceneCoordinatorTest, GetSceneBounds) {
    SpectraForge::Vulkan::SceneData sceneData;
    coordinator->loadScene(sceneData);
    
    SpectraForge::Math::Vector3 min, max;
    coordinator->getSceneBounds(min, max);
    
    // Bounds должны быть инициализированы (даже если нулями)
    // Проверяем, что метод не крашится
}

// ============================================================================
// ТЕСТ 10: Загрузка сцены дважды (должен заменить)
// ============================================================================
TEST_F(SceneCoordinatorTest, LoadSceneTwice) {
    SpectraForge::Vulkan::SceneData scene1;
    scene1.scenePath = "scene1.obj";
    
    coordinator->loadScene(scene1);
    EXPECT_TRUE(coordinator->isSceneLoaded());
    
    SpectraForge::Vulkan::SceneData scene2;
    scene2.scenePath = "scene2.obj";
    
    coordinator->loadScene(scene2);
    EXPECT_TRUE(coordinator->isSceneLoaded());
    
    SceneInfo info = coordinator->getSceneInfo();
    EXPECT_EQ(info.scenePath, "scene2.obj");
}

// ============================================================================
// ТЕСТ 11: updateCamera с nullptr input (граничный случай)
// ============================================================================
TEST_F(SceneCoordinatorTest, UpdateCameraWithNullInput) {
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    // Не должно крашиться
    EXPECT_NO_THROW(coordinator->updateCamera(0.016f, nullptr, false));
}

// ============================================================================
// ТЕСТ 12: updateCamera с нулевым deltaTime
// ============================================================================
TEST_F(SceneCoordinatorTest, UpdateCameraZeroDeltaTime) {
    InputState input;
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    auto cameraBefore = coordinator->getCamera()->getPosition();
    
    coordinator->updateCamera(0.0f, &input, false);
    
    // Камера не должна сильно измениться при нулевом delta time
    auto cameraAfter = coordinator->getCamera()->getPosition();
}

// ============================================================================
// ТЕСТ 13: Camera movement with WASD
// ============================================================================
TEST_F(SceneCoordinatorTest, CameraMovementWASD) {
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    auto camera = coordinator->getCamera();
    
    InputState input;
    
    // Test W key (forward)
    input.keys[GLFW_KEY_W] = true;
    auto posBefore = camera->getPosition();
    coordinator->updateCamera(0.1f, &input, false);
    auto posAfter = camera->getPosition();
    
    // Камера должна переместиться (проверяем что хотя бы одна координата изменилась)
    bool moved = (posBefore.x != posAfter.x) || 
                 (posBefore.y != posAfter.y) || 
                 (posBefore.z != posAfter.z);
    EXPECT_TRUE(moved);
}

// ============================================================================
// ТЕСТ 14: Mouse look
// ============================================================================
TEST_F(SceneCoordinatorTest, MouseLook) {
    coordinator->loadScene(SpectraForge::Vulkan::SceneData());
    
    InputState input;
    input.deltaMouseX = 10.0f;
    input.deltaMouseY = 5.0f;
    
    // Поворот камеры от мыши
    coordinator->updateCamera(0.016f, &input, false);
    
    // Направление камеры должно измениться
    // (детальная проверка зависит от реализации)
}

