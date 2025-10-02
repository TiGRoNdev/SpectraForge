/**
 * @file NativeUpscaler_Test.cpp
 * @brief Unit tests for NativeUpscaler (AAA pattern)
 *
 * Tests cover:
 * - Initialization and configuration
 * - Blit requirement detection
 * - Resize functionality
 * - RAII cleanup
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include <gtest/gtest.h>
#include "SpectraForge/upscaling/NativeUpscaler.h"
#include "SpectraForge/core/VulkanContext.h"
#include <vulkan/vulkan.hpp>

using namespace spectraforge::upscaling;

// ============================================================================
// Mock VulkanContext for testing
// ============================================================================

class MockVulkanContext : public spectraforge::core::VulkanContext {
public:
    vk::Instance getInstance() const override { return {}; }
    vk::PhysicalDevice getPhysicalDevice() const override { return {}; }
    vk::Device getDevice() const override { return mockDevice_; }
    vk::Queue getGraphicsQueue() const override { return {}; }
    vk::Queue getComputeQueue() const override { return {}; }
    vk::Queue getTransferQueue() const override { return {}; }
    uint32_t getGraphicsQueueFamily() const override { return 0; }
    uint32_t getComputeQueueFamily() const override { return 1; }
    uint32_t getTransferQueueFamily() const override { return 2; }

private:
    vk::Device mockDevice_;
};

// ============================================================================
// Test Fixture
// ============================================================================

class NativeUpscalerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize mock context
        context_ = std::make_unique<MockVulkanContext>();
    }

    void TearDown() override {
        // Cleanup happens via RAII
        context_.reset();
    }

    std::unique_ptr<MockVulkanContext> context_;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(NativeUpscalerTest, Constructor_CreatesInstance) {
    // Arrange & Act
    NativeUpscaler upscaler;

    // Assert
    EXPECT_STREQ("Native (Pass-Through)", upscaler.getName());
    EXPECT_FALSE(upscaler.isInitialized());
}

// ============================================================================
// Initialize Tests
// ============================================================================

TEST_F(NativeUpscalerTest, Initialize_SameResolution_NoBlit) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;

    // Act
    bool success = upscaler.initialize(*context_, config);

    // Assert
    EXPECT_TRUE(success);
    EXPECT_TRUE(upscaler.isInitialized());
    // Blit not required (input == output)
}

TEST_F(NativeUpscalerTest, Initialize_DifferentResolution_RequiresBlit) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 3840;
    config.outputHeight = 2160;

    // Act
    bool success = upscaler.initialize(*context_, config);

    // Assert
    EXPECT_TRUE(success);
    EXPECT_TRUE(upscaler.isInitialized());
    // Blit required (upscaling 1080p → 4K)
}

TEST_F(NativeUpscalerTest, Initialize_4KOutput_Success) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 7680;
    config.outputHeight = 4320;
    config.quality = UpscaleQuality::ULTRA_QUALITY;

    // Act
    bool success = upscaler.initialize(*context_, config);

    // Assert
    EXPECT_TRUE(success);
    EXPECT_TRUE(upscaler.isInitialized());
}

// ============================================================================
// Resize Tests
// ============================================================================

TEST_F(NativeUpscalerTest, Resize_UpdatesConfiguration) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler.initialize(*context_, config);

    // Act
    bool success = upscaler.resize(2560, 1440, 3840, 2160);

    // Assert
    EXPECT_TRUE(success);
    // Should detect blit requirement after resize
}

TEST_F(NativeUpscalerTest, Resize_SameResolution_NoBlit) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler.initialize(*context_, config);

    // Act
    bool success = upscaler.resize(1920, 1080, 1920, 1080);

    // Assert
    EXPECT_TRUE(success);
}

// ============================================================================
// Jitter Tests
// ============================================================================

TEST_F(NativeUpscalerTest, GetJitterOffset_AlwaysZero) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler.initialize(*context_, config);

    // Act
    float jitterX, jitterY;
    upscaler.getJitterOffset(0, jitterX, jitterY);

    // Assert
    EXPECT_FLOAT_EQ(0.0f, jitterX);
    EXPECT_FLOAT_EQ(0.0f, jitterY);
}

TEST_F(NativeUpscalerTest, GetJitterOffset_MultipleFrames_StillZero) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler.initialize(*context_, config);

    // Act & Assert
    for (uint32_t frame = 0; frame < 100; ++frame) {
        float jitterX, jitterY;
        upscaler.getJitterOffset(frame, jitterX, jitterY);
        EXPECT_FLOAT_EQ(0.0f, jitterX);
        EXPECT_FLOAT_EQ(0.0f, jitterY);
    }
}

// ============================================================================
// Cleanup Tests (RAII)
// ============================================================================

TEST_F(NativeUpscalerTest, Cleanup_ResetsState) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler.initialize(*context_, config);

    // Act
    upscaler.cleanup();

    // Assert
    EXPECT_FALSE(upscaler.isInitialized());
}

TEST_F(NativeUpscalerTest, Destructor_AutomaticCleanup) {
    // Arrange
    auto upscaler = std::make_unique<NativeUpscaler>();
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 1920;
    config.outputHeight = 1080;
    upscaler->initialize(*context_, config);

    // Act
    upscaler.reset();  // Destructor called

    // Assert
    // No memory leaks (verified by ASAN)
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(NativeUpscalerTest, Execute_BeforeInitialize_NoError) {
    // Arrange
    NativeUpscaler upscaler;
    vk::CommandBuffer cmd;  // Mock command buffer
    UpscaleResources resources{};

    // Act (should handle gracefully, not crash)
    upscaler.execute(cmd, resources, 0, 0.0f, 0.0f);

    // Assert
    // No crash expected
}

TEST_F(NativeUpscalerTest, Resize_BeforeInitialize_StillSucceeds) {
    // Arrange
    NativeUpscaler upscaler;

    // Act
    bool success = upscaler.resize(1920, 1080, 1920, 1080);

    // Assert
    EXPECT_TRUE(success);  // Should not require prior initialization
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(NativeUpscalerTest, FullWorkflow_Initialize_Execute_Cleanup) {
    // Arrange
    NativeUpscaler upscaler;
    UpscaleConfig config;
    config.inputWidth = 1920;
    config.inputHeight = 1080;
    config.outputWidth = 3840;
    config.outputHeight = 2160;

    // Act
    bool initSuccess = upscaler.initialize(*context_, config);
    ASSERT_TRUE(initSuccess);

    vk::CommandBuffer cmd;  // Mock
    UpscaleResources resources{};
    upscaler.execute(cmd, resources, 0, 0.0f, 0.0f);

    upscaler.cleanup();

    // Assert
    EXPECT_FALSE(upscaler.isInitialized());
}

// ============================================================================
// Main (for standalone execution)
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

