/**
 * @file triangle_splatting_core_test.cpp
 * @brief Unit tests для TriangleSplattingCore
 * 
 * TDD RED Phase - Day 1
 * Тесты написаны ДО реализации класса (должны fail)
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingCore.h>
#include <vulkan/vulkan.hpp>

using namespace spectraforge::rendering;

/**
 * @brief Mock VulkanContext для тестирования
 */
struct MockVulkanContext {
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
    VmaAllocator allocator = nullptr;
    vk::Queue computeQueue;
    vk::CommandPool commandPool;
};

/**
 * @brief Fixture для TriangleSplattingCore тестов
 */
class TriangleSplattingCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TODO: Создать mock Vulkan context
        // context_ = createMockVulkanContext();
        
        config_.outputWidth = 800;
        config_.outputHeight = 600;
    }
    
    void TearDown() override {
        // TODO: Cleanup mock resources
    }
    
    MockVulkanContext context_;
    TriangleSplattingCore::Config config_;
};

// ============================================================================
// TEST 1: Initialize Success
// ============================================================================
TEST_F(TriangleSplattingCoreTest, InitializeSuccess) {
    TriangleSplattingCore core;
    
    // ACT
    bool result = core.initialize(context_, config_);
    
    // ASSERT
    EXPECT_TRUE(result) << "Initialization should succeed with valid context";
    EXPECT_TRUE(core.isInitialized()) << "isInitialized() should return true";
}

// ============================================================================
// TEST 2: Initialize Fails With Invalid Device
// ============================================================================
TEST_F(TriangleSplattingCoreTest, InitializeFailsWithInvalidDevice) {
    TriangleSplattingCore core;
    
    // ARRANGE - Invalid context
    MockVulkanContext invalidContext;
    invalidContext.device = vk::Device(nullptr);
    
    // ACT
    bool result = core.initialize(invalidContext, config_);
    
    // ASSERT
    EXPECT_FALSE(result) << "Initialization should fail with null device";
    EXPECT_FALSE(core.isInitialized());
}

// ============================================================================
// TEST 3: Create Output Image Success
// ============================================================================
TEST_F(TriangleSplattingCoreTest, CreateOutputImageSuccess) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT - createOutputImage() вызывается внутри initialize()
    vk::Image image = core.getOutputImage();
    vk::ImageView imageView = core.getOutputImageView();
    
    // ASSERT
    EXPECT_NE(image, vk::Image(nullptr)) << "Output image should be created";
    EXPECT_NE(imageView, vk::ImageView(nullptr)) << "Image view should be created";
}

// ============================================================================
// TEST 4: Create Shader Modules Success
// ============================================================================
TEST_F(TriangleSplattingCoreTest, CreateShaderModulesSuccess) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT
    bool hasShaders = core.hasShaderModules();
    
    // ASSERT
    EXPECT_TRUE(hasShaders) << "Shader modules should be loaded";
}

// ============================================================================
// TEST 5: Create Pipelines Success
// ============================================================================
TEST_F(TriangleSplattingCoreTest, CreatePipelinesSuccess) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT
    bool hasPipelines = core.hasPipelines();
    
    // ASSERT
    EXPECT_TRUE(hasPipelines) << "Pipelines should be created";
}

// ============================================================================
// TEST 6: Create Descriptor Sets Success
// ============================================================================
TEST_F(TriangleSplattingCoreTest, CreateDescriptorSetsSuccess) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT
    bool hasDescriptorSets = core.hasDescriptorSets();
    
    // ASSERT
    EXPECT_TRUE(hasDescriptorSets) << "Descriptor sets should be created";
}

// ============================================================================
// TEST 7: Shutdown Cleans Up Resources
// ============================================================================
TEST_F(TriangleSplattingCoreTest, ShutdownCleansUpResources) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT
    core.shutdown();
    
    // ASSERT
    EXPECT_FALSE(core.isInitialized()) << "After shutdown, should not be initialized";
    EXPECT_EQ(core.getOutputImage(), vk::Image(nullptr)) << "Output image should be null";
}

// ============================================================================
// TEST 8: Getters Return Valid Handles
// ============================================================================
TEST_F(TriangleSplattingCoreTest, GettersReturnValidHandles) {
    TriangleSplattingCore core;
    ASSERT_TRUE(core.initialize(context_, config_));
    
    // ACT & ASSERT
    EXPECT_NE(core.getDevice(), vk::Device(nullptr));
    EXPECT_NE(core.getAllocator(), nullptr);
    EXPECT_NE(core.getOutputImage(), vk::Image(nullptr));
    EXPECT_NE(core.getOutputImageView(), vk::ImageView(nullptr));
}

// ============================================================================
// TEST 9: Multiple Initialize Calls Are Safe
// ============================================================================
TEST_F(TriangleSplattingCoreTest, MultipleInitializeCallsAreSafe) {
    TriangleSplattingCore core;
    
    // ACT
    ASSERT_TRUE(core.initialize(context_, config_));
    bool secondInit = core.initialize(context_, config_);
    
    // ASSERT
    EXPECT_TRUE(secondInit) << "Second initialize should return true (idempotent)";
    EXPECT_TRUE(core.isInitialized());
}

// ============================================================================
// TEST 10: Config Parameters Are Respected
// ============================================================================
TEST_F(TriangleSplattingCoreTest, ConfigParametersAreRespected) {
    TriangleSplattingCore core;
    
    // ARRANGE
    config_.outputWidth = 1920;
    config_.outputHeight = 1080;
    
    // ACT
    ASSERT_TRUE(core.initialize(context_, config_));
    auto imageExtent = core.getOutputImageExtent();
    
    // ASSERT
    EXPECT_EQ(imageExtent.width, 1920u);
    EXPECT_EQ(imageExtent.height, 1080u);
}

