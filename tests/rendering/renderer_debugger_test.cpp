/**
 * @file renderer_debugger_test.cpp
 * @brief Unit tests for RendererDebugger (P0.2 - TDD RED)
 * 
 * Это самый простой класс для тестирования (нет Vulkan зависимостей)
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/RendererDebugger.h"

using namespace SpectraForge::Rendering::Core;

class RendererDebuggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        debugger = std::make_unique<RendererDebugger>();
    }
    
    std::unique_ptr<RendererDebugger> debugger;
};

TEST_F(RendererDebuggerTest, DefaultState) {
    EXPECT_EQ(0, debugger->getDebugMode());
    EXPECT_FALSE(debugger->isWireframeEnabled());
    EXPECT_TRUE(debugger->isDepthTestEnabled());
    EXPECT_TRUE(debugger->isBackfaceCullingEnabled());
    EXPECT_TRUE(debugger->isAlphaBlendingEnabled());
    EXPECT_TRUE(debugger->isEarlyTerminationEnabled());
}

TEST_F(RendererDebuggerTest, SetDebugMode) {
    debugger->setDebugMode(1);
    EXPECT_EQ(1, debugger->getDebugMode());
    
    debugger->setDebugMode(3);
    EXPECT_EQ(3, debugger->getDebugMode());
}

TEST_F(RendererDebuggerTest, EnableWireframe) {
    debugger->enableWireframe(true);
    EXPECT_TRUE(debugger->isWireframeEnabled());
    
    debugger->enableWireframe(false);
    EXPECT_FALSE(debugger->isWireframeEnabled());
}

TEST_F(RendererDebuggerTest, SetBackgroundColor) {
    debugger->setBackgroundColor(1.0f, 0.0f, 0.0f, 1.0f);
    auto color = debugger->getBackgroundColor();
    EXPECT_FLOAT_EQ(1.0f, color.r);
    EXPECT_FLOAT_EQ(0.0f, color.g);
    EXPECT_FLOAT_EQ(0.0f, color.b);
    EXPECT_FLOAT_EQ(1.0f, color.a);
}

TEST_F(RendererDebuggerTest, SetViewport) {
    debugger->setViewport(0, 0, 1920, 1080);
    auto viewport = debugger->getViewport();
    EXPECT_FLOAT_EQ(0.0f, viewport.x);
    EXPECT_FLOAT_EQ(0.0f, viewport.y);
    EXPECT_FLOAT_EQ(1920.0f, viewport.width);
    EXPECT_FLOAT_EQ(1080.0f, viewport.height);
}

TEST_F(RendererDebuggerTest, DebugCallback) {
    bool callbackCalled = false;
    std::string receivedMessage;
    
    debugger->setDebugCallback([&](const std::string& msg) {
        callbackCalled = true;
        receivedMessage = msg;
    });
    
    debugger->log("Test message");
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ("Test message", receivedMessage);
}

TEST_F(RendererDebuggerTest, ToggleAllFlags) {
    debugger->enableBackfaceCulling(false);
    debugger->enableDepthTest(false);
    debugger->enableAlphaBlending(false);
    debugger->enableEarlyTermination(false);
    
    EXPECT_FALSE(debugger->isBackfaceCullingEnabled());
    EXPECT_FALSE(debugger->isDepthTestEnabled());
    EXPECT_FALSE(debugger->isAlphaBlendingEnabled());
    EXPECT_FALSE(debugger->isEarlyTerminationEnabled());
}

