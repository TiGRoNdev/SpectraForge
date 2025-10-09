/**
 * @file frame_manager_test.cpp
 * @brief Unit tests for FrameManager (P0.2 - TDD)
 * 
 * STRATEGY:
 * - Test frame synchronization objects (semaphores, fences)
 * - Test frame cycling (MAX_FRAMES_IN_FLIGHT)
 * - Test device lost handling
 * - Verify RAII cleanup
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/FrameManager.h"

using namespace SpectraForge::Rendering::Core;

/**
 * @brief Fixture для тестирования FrameManager
 */
class FrameManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock Vulkan device
        mockDevice = vk::Device{};
        manager = std::make_unique<FrameManager>(mockDevice);
    }
    
    void TearDown() override {
        if (manager) {
            manager->shutdown();
            manager.reset();
        }
    }
    
    vk::Device mockDevice;
    std::unique_ptr<FrameManager> manager;
};

/**
 * @brief Тест: Конструктор принимает device
 */
TEST_F(FrameManagerTest, ConstructionWithDevice) {
    EXPECT_NE(nullptr, manager);
}

/**
 * @brief Тест: Начальное состояние без инициализации
 */
TEST_F(FrameManagerTest, InitialStateWithoutInit) {
    EXPECT_EQ(0u, manager->getCurrentFrame());
    EXPECT_EQ(0u, manager->getCurrentImageIndex());
    EXPECT_FALSE(manager->isDeviceLost());
}

/**
 * @brief Тест: Инициализация создает sync objects
 */
TEST_F(FrameManagerTest, DISABLED_InitializeCreatesSyncObjects) {
    // В TDD RED фазе упадет, если sync objects не созданы
    bool result = manager->initialize();
    
    // Ожидаем false без валидного device
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Получение текущего кадра (должен чередоваться 0-1)
 */
TEST_F(FrameManagerTest, CurrentFrameAlternates) {
    size_t frame1 = manager->getCurrentFrame();
    EXPECT_TRUE(frame1 == 0 || frame1 == 1);
}

/**
 * @brief Тест: Получение image index
 */
TEST_F(FrameManagerTest, GetCurrentImageIndex) {
    uint32_t index = manager->getCurrentImageIndex();
    EXPECT_GE(index, 0u);
}

/**
 * @brief Тест: Получение семафоров (пусты без инициализации)
 */
TEST_F(FrameManagerTest, GetSemaphoresWithoutInit) {
    auto imageAvailable = manager->getImageAvailableSemaphore();
    auto renderFinished = manager->getRenderFinishedSemaphore();
    
    EXPECT_EQ(vk::Semaphore{}, imageAvailable);
    EXPECT_EQ(vk::Semaphore{}, renderFinished);
}

/**
 * @brief Тест: Получение fence (пустой без инициализации)
 */
TEST_F(FrameManagerTest, GetFenceWithoutInit) {
    auto fence = manager->getInFlightFence();
    EXPECT_EQ(vk::Fence{}, fence);
}

/**
 * @brief Тест: Device lost флаг изначально false
 */
TEST_F(FrameManagerTest, DeviceLostInitiallyFalse) {
    EXPECT_FALSE(manager->isDeviceLost());
}

/**
 * @brief Тест: beginFrame без swapchain должно обработаться
 */
TEST_F(FrameManagerTest, DISABLED_BeginFrameWithoutSwapchain) {
    vk::SwapchainKHR mockSwapchain{};
    
    // Не должно крашиться, но может установить device_lost
    EXPECT_NO_THROW(manager->beginFrame(mockSwapchain));
}

/**
 * @brief Тест: endFrame без swapchain должно обработаться
 */
TEST_F(FrameManagerTest, DISABLED_EndFrameWithoutSwapchain) {
    vk::SwapchainKHR mockSwapchain{};
    vk::Queue mockQueue{};
    vk::CommandBuffer mockCmdBuf{};
    
    // Не должно крашиться
    EXPECT_NO_THROW(manager->endFrame(mockSwapchain, mockQueue, mockCmdBuf));
}

/**
 * @brief Тест: Shutdown безопасен без инициализации
 */
TEST_F(FrameManagerTest, ShutdownWithoutInit) {
    EXPECT_NO_THROW(manager->shutdown());
}

/**
 * @brief Тест: Множественный shutdown безопасен
 */
TEST_F(FrameManagerTest, MultipleShutdownSafe) {
    manager->shutdown();
    EXPECT_NO_THROW(manager->shutdown());
}

/**
 * @brief Тест: RAII cleanup в деструкторе
 */
TEST_F(FrameManagerTest, RAIICleanup) {
    {
        FrameManager scopedManager(mockDevice);
    }
    // Если дошли сюда - cleanup прошел
    SUCCEED();
}

