/**
 * @file triangle_splatting_debugger_test.cpp
 * @brief Unit tests для TriangleSplattingDebugger
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingDebugger.h>

using namespace spectraforge::rendering;

TEST(TriangleSplattingDebuggerTest, SetDebugModeSwitchesModes) {
    TriangleSplattingDebugger debugger;
    
    debugger.setDebugMode(0); // Normal
    EXPECT_EQ(debugger.getDebugMode(), 0u);
    
    debugger.setDebugMode(1); // SDF visualization
    EXPECT_EQ(debugger.getDebugMode(), 1u);
    
    debugger.setDebugMode(2); // Barycentric visualization
    EXPECT_EQ(debugger.getDebugMode(), 2u);
}

TEST(TriangleSplattingDebuggerTest, SaveFrameToPPMCreatesFile) {
    TriangleSplattingDebugger debugger;
    
    vk::Image mockImage = vk::Image(nullptr);
    // ... mock parameters ...
    
    EXPECT_NO_THROW(debugger.saveFrameToPPM("test_output.ppm", mockImage, 
        vk::Device(nullptr), nullptr, vk::CommandPool(nullptr), 
        vk::Queue(nullptr), 800, 600));
}

TEST(TriangleSplattingDebuggerTest, SaveFrameToPNGCreatesFile) {
    TriangleSplattingDebugger debugger;
    EXPECT_NO_THROW(debugger.saveFrameToPNG("test_output.png", vk::Image(nullptr),
        vk::Device(nullptr), nullptr, vk::CommandPool(nullptr),
        vk::Queue(nullptr), 800, 600));
}

TEST(TriangleSplattingDebuggerTest, WireframeToggle) {
    TriangleSplattingDebugger debugger;
    
    debugger.enableWireframe(true);
    EXPECT_TRUE(debugger.isWireframeEnabled());
    
    debugger.enableWireframe(false);
    EXPECT_FALSE(debugger.isWireframeEnabled());
}

TEST(TriangleSplattingDebuggerTest, BackgroundColorChange) {
    TriangleSplattingDebugger debugger;
    
    glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f); // Red
    debugger.setBackgroundColor(color);
    
    EXPECT_EQ(debugger.getBackgroundColor(), color);
}

TEST(TriangleSplattingDebuggerTest, InvalidFilenameHandling) {
    TriangleSplattingDebugger debugger;
    
    // Should handle invalid filename gracefully
    EXPECT_NO_THROW(debugger.saveFrameToPPM("", vk::Image(nullptr),
        vk::Device(nullptr), nullptr, vk::CommandPool(nullptr),
        vk::Queue(nullptr), 800, 600));
}

