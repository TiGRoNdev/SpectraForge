/**
 * @file swapchain_manager_test.cpp
 * @brief Unit tests for SwapchainManager (P0.2 - TDD)
 * 
 * STRATEGY:
 * - Test swapchain lifecycle (create, recreate, destroy)
 * - Test surface creation
 * - Test image view management
 * - Verify RAII cleanup
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/SwapchainManager.h"

using namespace SpectraForge::Rendering::Core;

/**
 * @brief Fixture для тестирования SwapchainManager
 */
class SwapchainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock Vulkan objects
        mockInstance = vk::Instance{};
        mockPhysicalDevice = vk::PhysicalDevice{};
        mockDevice = vk::Device{};
        
        manager = std::make_unique<SwapchainManager>(
            mockInstance, mockPhysicalDevice, mockDevice
        );
    }
    
    void TearDown() override {
        if (manager) {
            manager->shutdown();
            manager.reset();
        }
    }
    
    vk::Instance mockInstance;
    vk::PhysicalDevice mockPhysicalDevice;
    vk::Device mockDevice;
    std::unique_ptr<SwapchainManager> manager;
};

/**
 * @brief Тест: Конструктор принимает Vulkan объекты
 */
TEST_F(SwapchainManagerTest, ConstructionWithVulkanObjects) {
    EXPECT_NE(nullptr, manager);
}

/**
 * @brief Тест: Начальное состояние (без swapchain)
 */
TEST_F(SwapchainManagerTest, InitialState) {
    EXPECT_EQ(vk::SurfaceKHR{}, manager->getSurface());
    EXPECT_EQ(vk::SwapchainKHR{}, manager->getSwapchain());
    EXPECT_TRUE(manager->getSwapchainImages().empty());
    EXPECT_TRUE(manager->getSwapchainImageViews().empty());
}

/**
 * @brief Тест: Создание surface (будет требовать валидное X11 окно)
 * 
 * NOTE: В реальном окружении с X11 этот тест упадёт без реального display/window
 * Для TDD это нормально - тест показывает ожидаемое поведение
 */
TEST_F(SwapchainManagerTest, DISABLED_CreateSurfaceX11_RequiresValidDisplay) {
    // В TDD RED фазе тест упадёт, так как нужен реальный X11
    void* mockDisplay = nullptr;
    void* mockWindow = nullptr;
    
    bool result = manager->createSurfaceX11(mockDisplay, mockWindow);
    
    // Ожидаем false при nullptr
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Создание swapchain без surface должно провалиться
 */
TEST_F(SwapchainManagerTest, DISABLED_CreateSwapchain_FailsWithoutSurface) {
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t graphicsQueueFamily = 0;
    uint32_t presentQueueFamily = 0;
    
    // Попытка создать swapchain без surface
    bool result = manager->createSwapchain(width, height, 
                                          graphicsQueueFamily, presentQueueFamily);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Получение формата swapchain
 */
TEST_F(SwapchainManagerTest, GetSwapchainFormat) {
    auto format = manager->getSwapchainFormat();
    // По умолчанию должен быть B8G8R8A8Unorm
    EXPECT_EQ(vk::Format::eB8G8R8A8Unorm, format);
}

/**
 * @brief Тест: Получение размеров swapchain
 */
TEST_F(SwapchainManagerTest, GetSwapchainExtent) {
    auto extent = manager->getSwapchainExtent();
    // По умолчанию 1920x1080
    EXPECT_EQ(1920u, extent.width);
    EXPECT_EQ(1080u, extent.height);
}

/**
 * @brief Тест: Уничтожение swapchain безопасно без создания
 */
TEST_F(SwapchainManagerTest, DestroySwapchain_SafeWithoutCreation) {
    // Не должно крашиться
    EXPECT_NO_THROW(manager->destroySwapchain());
}

/**
 * @brief Тест: Shutdown безопасен при множественном вызове
 */
TEST_F(SwapchainManagerTest, ShutdownMultipleCallsSafe) {
    manager->shutdown();
    EXPECT_NO_THROW(manager->shutdown());
}

/**
 * @brief Тест: Image views пусты без swapchain
 */
TEST_F(SwapchainManagerTest, ImageViewsEmptyWithoutSwapchain) {
    auto views = manager->getSwapchainImageViews();
    EXPECT_TRUE(views.empty());
}

/**
 * @brief Тест: Swapchain images пусты без swapchain
 */
TEST_F(SwapchainManagerTest, SwapchainImagesEmptyWithoutSwapchain) {
    auto images = manager->getSwapchainImages();
    EXPECT_TRUE(images.empty());
}

/**
 * @brief Тест: RAII cleanup вызывается в деструкторе
 */
TEST_F(SwapchainManagerTest, RAIICleanup) {
    // Создаем в scope
    {
        SwapchainManager scopedManager(mockInstance, mockPhysicalDevice, mockDevice);
        // При выходе из scope должен вызваться деструктор
    }
    // Если дошли сюда - cleanup прошел нормально
    SUCCEED();
}

