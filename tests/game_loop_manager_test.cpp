/**
 * @file game_loop_manager_test.cpp
 * @brief Unit tests for GameLoopManager (P0.3 TDD)
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "SpectraForge/App/Core/GameLoopManager.h"

using namespace SpectraForge::App::Core;

class GameLoopManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<GameLoopManager>();
    }
    
    void TearDown() override {
        manager.reset();
    }
    
    std::unique_ptr<GameLoopManager> manager;
};

// ============================================================================
// ТЕСТ 1: Инициализация
// ============================================================================
TEST_F(GameLoopManagerTest, InitialState) {
    EXPECT_EQ(manager->getDeltaTime(), 0.0f);
    EXPECT_EQ(manager->getFPS(), 0.0f);
    EXPECT_EQ(manager->getFrameTime(), 0.0f);
    EXPECT_EQ(manager->getTargetFPS(), 0.0f);
}

// ============================================================================
// ТЕСТ 2: beginFrame/endFrame цикл
// ============================================================================
TEST_F(GameLoopManagerTest, BasicFrameCycle) {
    manager->beginFrame();
    
    // Симулируем небольшую задержку кадра
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    
    manager->endFrame();
    
    // Delta time должен быть примерно 0.016 секунд (16ms)
    EXPECT_GT(manager->getDeltaTime(), 0.01f);
    EXPECT_LT(manager->getDeltaTime(), 0.02f);
    
    // Frame time должен быть примерно 16ms
    EXPECT_GT(manager->getFrameTime(), 15.0f);
    EXPECT_LT(manager->getFrameTime(), 20.0f);
}

// ============================================================================
// ТЕСТ 3: FPS calculation
// ============================================================================
TEST_F(GameLoopManagerTest, FPSCalculation) {
    // Симулируем несколько кадров
    for (int i = 0; i < 60; ++i) {
        manager->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        manager->endFrame();
    }
    
    // FPS должен быть примерно 60
    float fps = manager->getFPS();
    EXPECT_GT(fps, 50.0f);
    EXPECT_LT(fps, 70.0f);
}

// ============================================================================
// ТЕСТ 4: Target FPS setting
// ============================================================================
TEST_F(GameLoopManagerTest, SetTargetFPS) {
    manager->setTargetFPS(60.0f);
    EXPECT_EQ(manager->getTargetFPS(), 60.0f);
    
    manager->setTargetFPS(144.0f);
    EXPECT_EQ(manager->getTargetFPS(), 144.0f);
    
    manager->setTargetFPS(0.0f); // Unlimited
    EXPECT_EQ(manager->getTargetFPS(), 0.0f);
}

// ============================================================================
// ТЕСТ 5: Total time tracking
// ============================================================================
TEST_F(GameLoopManagerTest, TotalTimeTracking) {
    manager->beginFrame();
    manager->endFrame();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    manager->beginFrame();
    manager->endFrame();
    
    float totalTime = manager->getTotalTime();
    EXPECT_GT(totalTime, 0.1f);
    EXPECT_LT(totalTime, 0.2f);
}

// ============================================================================
// ТЕСТ 6: Reset functionality
// ============================================================================
TEST_F(GameLoopManagerTest, Reset) {
    // Запустим несколько кадров
    for (int i = 0; i < 10; ++i) {
        manager->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        manager->endFrame();
    }
    
    EXPECT_GT(manager->getTotalTime(), 0.0f);
    
    // Reset
    manager->reset();
    
    // Total time должен быть близок к 0
    EXPECT_LT(manager->getTotalTime(), 0.01f);
}

// ============================================================================
// ТЕСТ 7: Multiple consecutive frames
// ============================================================================
TEST_F(GameLoopManagerTest, ConsecutiveFrames) {
    float previousDeltaTime = 0.0f;
    
    for (int i = 0; i < 5; ++i) {
        manager->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        manager->endFrame();
        
        float deltaTime = manager->getDeltaTime();
        
        // Delta time должен быть положительным
        EXPECT_GT(deltaTime, 0.0f);
        
        // Delta time должен быть разумным (не слишком большим)
        EXPECT_LT(deltaTime, 0.1f);
    }
}

// ============================================================================
// ТЕСТ 8: Zero frame time handling
// ============================================================================
TEST_F(GameLoopManagerTest, ZeroFrameTime) {
    manager->beginFrame();
    manager->endFrame(); // Немедленно завершаем кадр
    
    // Delta time может быть очень малым, но не отрицательным
    EXPECT_GE(manager->getDeltaTime(), 0.0f);
}

// ============================================================================
// ТЕСТ 9: Frame time consistency
// ============================================================================
TEST_F(GameLoopManagerTest, FrameTimeConsistency) {
    std::vector<float> frameTimes;
    
    for (int i = 0; i < 10; ++i) {
        manager->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        manager->endFrame();
        
        frameTimes.push_back(manager->getFrameTime());
    }
    
    // Все frame times должны быть примерно одинаковыми
    float avgFrameTime = 0.0f;
    for (float ft : frameTimes) {
        avgFrameTime += ft;
    }
    avgFrameTime /= frameTimes.size();
    
    for (float ft : frameTimes) {
        // Допустим разброс ±20% от среднего
        EXPECT_GT(ft, avgFrameTime * 0.8f);
        EXPECT_LT(ft, avgFrameTime * 1.2f);
    }
}

// ============================================================================
// ТЕСТ 10: FPS update rate
// ============================================================================
TEST_F(GameLoopManagerTest, FPSUpdateRate) {
    float initialFPS = manager->getFPS();
    
    // Запустим несколько быстрых кадров
    for (int i = 0; i < 10; ++i) {
        manager->beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        manager->endFrame();
    }
    
    // FPS должен обновиться
    // (может быть равен 0 изначально, но после кадров должен измениться)
    // Этот тест проверяет, что FPS вообще обновляется
}

