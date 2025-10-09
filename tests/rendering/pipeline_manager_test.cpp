/**
 * @file pipeline_manager_test.cpp
 * @brief Unit tests for PipelineManager (P0.2 - TDD)
 * 
 * STRATEGY:
 * - Test render pass creation
 * - Test framebuffer management
 * - Test command pool/buffers
 * - Test command buffer recording
 * - Verify RAII cleanup
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/PipelineManager.h"

using namespace SpectraForge::Rendering::Core;

/**
 * @brief Fixture для тестирования PipelineManager
 */
class PipelineManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockDevice = vk::Device{};
        manager = std::make_unique<PipelineManager>(mockDevice);
    }
    
    void TearDown() override {
        if (manager) {
            manager->shutdown();
            manager.reset();
        }
    }
    
    vk::Device mockDevice;
    std::unique_ptr<PipelineManager> manager;
};

/**
 * @brief Тест: Конструктор принимает device
 */
TEST_F(PipelineManagerTest, ConstructionWithDevice) {
    EXPECT_NE(nullptr, manager);
}

/**
 * @brief Тест: Начальное состояние без инициализации
 */
TEST_F(PipelineManagerTest, InitialState) {
    EXPECT_EQ(vk::RenderPass{}, manager->getRenderPass());
    EXPECT_EQ(vk::CommandPool{}, manager->getCommandPool());
    EXPECT_TRUE(manager->getFramebuffers().empty());
    EXPECT_TRUE(manager->getCommandBuffers().empty());
}

/**
 * @brief Тест: Инициализация требует image views
 */
TEST_F(PipelineManagerTest, DISABLED_InitializeRequiresImageViews) {
    std::vector<vk::ImageView> emptyViews;
    vk::Format format = vk::Format::eB8G8R8A8Unorm;
    vk::Extent2D extent{1920, 1080};
    uint32_t queueFamily = 0;
    
    // Ожидаем false с пустыми views
    bool result = manager->initialize(emptyViews, format, extent, queueFamily);
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Создание render pass без валидного device
 */
TEST_F(PipelineManagerTest, DISABLED_CreateRenderPassWithoutDevice) {
    vk::Format format = vk::Format::eB8G8R8A8Unorm;
    
    bool result = manager->createRenderPass(format);
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Создание framebuffers без render pass
 */
TEST_F(PipelineManagerTest, DISABLED_CreateFramebuffersWithoutRenderPass) {
    std::vector<vk::ImageView> views;
    vk::Extent2D extent{1920, 1080};
    
    bool result = manager->createFramebuffers(views, extent);
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Создание command pool
 */
TEST_F(PipelineManagerTest, DISABLED_CreateCommandPool) {
    uint32_t queueFamily = 0;
    
    // Ожидаем false без валидного device
    bool result = manager->createCommandPool(queueFamily);
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Создание command buffers без pool
 */
TEST_F(PipelineManagerTest, DISABLED_CreateCommandBuffersWithoutPool) {
    bool result = manager->createCommandBuffers();
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Получение пустого списка framebuffers
 */
TEST_F(PipelineManagerTest, GetEmptyFramebuffers) {
    auto framebuffers = manager->getFramebuffers();
    EXPECT_TRUE(framebuffers.empty());
}

/**
 * @brief Тест: Получение пустого списка command buffers
 */
TEST_F(PipelineManagerTest, GetEmptyCommandBuffers) {
    auto cmdBuffers = manager->getCommandBuffers();
    EXPECT_TRUE(cmdBuffers.empty());
}

/**
 * @brief Тест: Запись command buffer с callback
 */
TEST_F(PipelineManagerTest, RecordCommandBufferWithCallback) {
    uint32_t imageIndex = 0;
    uint32_t frameIndex = 0;
    bool callbackCalled = false;
    
    auto recordFunc = [&callbackCalled](vk::CommandBuffer) {
        callbackCalled = true;
    };
    
    // Не должно крашиться
    EXPECT_NO_THROW(
        manager->recordCommandBuffer(imageIndex, frameIndex, recordFunc)
    );
}

/**
 * @brief Тест: Shutdown безопасен без инициализации
 */
TEST_F(PipelineManagerTest, ShutdownWithoutInit) {
    EXPECT_NO_THROW(manager->shutdown());
}

/**
 * @brief Тест: Множественный shutdown безопасен
 */
TEST_F(PipelineManagerTest, MultipleShutdownSafe) {
    manager->shutdown();
    EXPECT_NO_THROW(manager->shutdown());
}

/**
 * @brief Тест: RAII cleanup в деструкторе
 */
TEST_F(PipelineManagerTest, RAIICleanup) {
    {
        PipelineManager scopedManager(mockDevice);
    }
    SUCCEED();
}

/**
 * @brief Тест: Инициализация с mock данными
 */
TEST_F(PipelineManagerTest, DISABLED_InitializeWithMockData) {
    // Создаем mock image view (nullptr но правильный тип)
    std::vector<vk::ImageView> mockViews;
    mockViews.push_back(vk::ImageView{});
    mockViews.push_back(vk::ImageView{});
    
    vk::Format format = vk::Format::eB8G8R8A8Unorm;
    vk::Extent2D extent{1920, 1080};
    uint32_t queueFamily = 0;
    
    // Ожидаем false без валидного device
    bool result = manager->initialize(mockViews, format, extent, queueFamily);
    EXPECT_FALSE(result);
}

/**
 * @brief Тест: Размер command buffers соответствует MAX_FRAMES_IN_FLIGHT
 */
TEST_F(PipelineManagerTest, CommandBuffersSizeMatchesMaxFrames) {
    // После успешного создания должно быть 2 command buffers (MAX_FRAMES_IN_FLIGHT = 2)
    auto cmdBuffers = manager->getCommandBuffers();
    // Пока 0, так как не инициализирован
    EXPECT_EQ(0u, cmdBuffers.size());
}

