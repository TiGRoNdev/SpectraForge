/**
 * @file renderer_core_test.cpp
 * @brief Unit tests for RendererCore (P0.2 Refactoring - TDD RED)
 * 
 * STRATEGY:
 * - Mock Vulkan API calls через test fixtures
 * - Проверка RAII (автоматическая очистка)
 * - Проверка error handling
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/RendererCore.h"

using namespace SpectraForge::Rendering::Core;

/**
 * @brief Fixture для тестирования RendererCore
 * 
 * NOTE: В реальности Vulkan требует валидного instance/device,
 * но для TDD мы сначала пишем тесты, которые упадут, а потом реализуем mock
 */
class RendererCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // В будущем здесь будет mock Vulkan instance
        core = std::make_unique<RendererCore>();
    }
    
    void TearDown() override {
        if (core) {
            core->shutdown();
            core.reset();
        }
    }
    
    std::unique_ptr<RendererCore> core;
};

/**
 * @brief Тест: Создание RendererCore без инициализации
 */
TEST_F(RendererCoreTest, ConstructionWithoutInitialization) {
    EXPECT_NE(nullptr, core);
    EXPECT_FALSE(core->isInitialized());
    
    // До инициализации должны быть null handles
    EXPECT_EQ(vk::Instance{}, core->getInstance());
    EXPECT_EQ(vk::Device{}, core->getDevice());
}

/**
 * @brief Тест: Инициализация Vulkan instance
 * 
 * RED: Этот тест упадёт, т.к. RendererCore ещё не реализован
 */
TEST_F(RendererCoreTest, InitializeSuccess) {
    // ACT
    bool result = core->initialize();
    
    // ASSERT
    EXPECT_TRUE(result);
    EXPECT_TRUE(core->isInitialized());
    EXPECT_NE(vk::Instance{}, core->getInstance());
}

/**
 * @brief Тест: Повторная инициализация должна быть idempotent
 */
TEST_F(RendererCoreTest, InitializeIdempotent) {
    // ARRANGE
    core->initialize();
    auto firstInstance = core->getInstance();
    
    // ACT - вторая инициализация
    bool result = core->initialize();
    
    // ASSERT - должна вернуть true и не пересоздать instance
    EXPECT_TRUE(result);
    EXPECT_EQ(firstInstance, core->getInstance());
}

/**
 * @brief Тест: Physical device должен быть выбран после инициализации
 */
TEST_F(RendererCoreTest, PhysicalDeviceSelectedAfterInit) {
    // ACT
    core->initialize();
    
    // ASSERT
    EXPECT_NE(vk::PhysicalDevice{}, core->getPhysicalDevice());
}

/**
 * @brief Тест: Создание logical device требует surface
 * 
 * NOTE: Logical device создаётся после surface, чтобы проверить present support
 */
TEST_F(RendererCoreTest, CreateLogicalDeviceWithSurfaceSuccess) {
    // ARRANGE
    core->initialize();
    
    // Mock surface (в реальности нужен X11 window)
    // Для TDD RED фазы просто передаём null handle
    vk::SurfaceKHR mockSurface{};
    
    // ACT
    bool result = core->createLogicalDeviceWithSurface(mockSurface);
    
    // ASSERT
    // RED: Упадёт, т.к. не реализовано
    EXPECT_TRUE(result);
    EXPECT_NE(vk::Device{}, core->getDevice());
    EXPECT_NE(vk::Queue{}, core->getGraphicsQueue());
    EXPECT_NE(vk::Queue{}, core->getPresentQueue());
}

/**
 * @brief Тест: Queue families должны быть найдены
 */
TEST_F(RendererCoreTest, QueueFamiliesFound) {
    // ARRANGE
    core->initialize();
    vk::SurfaceKHR mockSurface{};
    core->createLogicalDeviceWithSurface(mockSurface);
    
    // ACT
    uint32_t graphicsFamily = core->getGraphicsQueueFamily();
    uint32_t presentFamily = core->getPresentQueueFamily();
    
    // ASSERT
    // В большинстве GPU graphics и present совпадают (0)
    EXPECT_LT(graphicsFamily, 10u);  // Разумный лимит
    EXPECT_LT(presentFamily, 10u);
}

/**
 * @brief Тест: VMA allocator должен быть создан
 */
TEST_F(RendererCoreTest, AllocatorCreatedSuccess) {
    // ARRANGE
    core->initialize();
    vk::SurfaceKHR mockSurface{};
    core->createLogicalDeviceWithSurface(mockSurface);
    
    // ACT
    VmaAllocator allocator = core->getAllocator();
    
    // ASSERT
    // RED: Упадёт
    EXPECT_NE(VK_NULL_HANDLE, allocator);
}

/**
 * @brief Тест: Shutdown должен очистить все ресурсы
 */
TEST_F(RendererCoreTest, ShutdownCleansUpResources) {
    // ARRANGE
    core->initialize();
    vk::SurfaceKHR mockSurface{};
    core->createLogicalDeviceWithSurface(mockSurface);
    
    // ACT
    core->shutdown();
    
    // ASSERT
    EXPECT_FALSE(core->isInitialized());
    
    // После shutdown можно снова инициализировать
    bool reinitResult = core->initialize();
    EXPECT_TRUE(reinitResult);
}

/**
 * @brief Тест: RAII - деструктор должен вызывать shutdown
 */
TEST_F(RendererCoreTest, RAIIDestructorCallsShutdown) {
    // ARRANGE
    auto tempCore = std::make_unique<RendererCore>();
    tempCore->initialize();
    
    // ACT - деструктор должен вызваться автоматически
    tempCore.reset();
    
    // ASSERT - если не упало с Vulkan ошибкой, значит cleanup работает
    SUCCEED();  // RAII test
}

/**
 * @brief Тест: Нельзя создать device без инициализации instance
 */
TEST_F(RendererCoreTest, CreateDeviceWithoutInstanceFails) {
    // ACT - пытаемся создать device без initialize()
    vk::SurfaceKHR mockSurface{};
    bool result = core->createLogicalDeviceWithSurface(mockSurface);
    
    // ASSERT
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Нельзя копировать RendererCore (deleted copy constructor)
 */
TEST(RendererCoreCopyTest, CopyConstructorDeleted) {
    // Это тест времени компиляции - если скомпилируется, значит ОК
    EXPECT_FALSE(std::is_copy_constructible<RendererCore>::value);
    EXPECT_FALSE(std::is_copy_assignable<RendererCore>::value);
}

/**
 * @brief Benchmark: Скорость инициализации Vulkan
 * 
 * NOTE: Не часть TDD, но полезно для performance tracking
 */
TEST_F(RendererCoreTest, DISABLED_BenchmarkInitialization) {
    auto start = std::chrono::high_resolution_clock::now();
    
    core->initialize();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Инициализация Vulkan должна быть быстрой (<1000ms)
    EXPECT_LT(duration.count(), 1000);
    std::cout << "Vulkan initialization took: " << duration.count() << "ms\n";
}

/**
 * @brief Integration test: Полный цикл init->use->shutdown
 */
TEST_F(RendererCoreTest, FullLifecycleIntegration) {
    // ARRANGE & ACT - полный lifecycle
    EXPECT_TRUE(core->initialize());
    vk::SurfaceKHR mockSurface{};  // TODO: реальный surface
    EXPECT_TRUE(core->createLogicalDeviceWithSurface(mockSurface));
    
    // Проверка всех компонентов
    EXPECT_NE(vk::Instance{}, core->getInstance());
    EXPECT_NE(vk::PhysicalDevice{}, core->getPhysicalDevice());
    EXPECT_NE(vk::Device{}, core->getDevice());
    EXPECT_NE(VK_NULL_HANDLE, core->getAllocator());
    
    // CLEANUP
    core->shutdown();
    EXPECT_FALSE(core->isInitialized());
}

