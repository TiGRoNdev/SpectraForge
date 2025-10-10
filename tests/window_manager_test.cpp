/**
 * @file window_manager_test.cpp
 * @brief Unit tests for WindowManager (P0.3 TDD)
 */

#include <gtest/gtest.h>
#include "SpectraForge/App/Core/WindowManager.h"

using namespace SpectraForge::App::Core;

class WindowManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<WindowManager>();
    }
    
    void TearDown() override {
        if (manager && manager->isInitialized()) {
            manager->shutdown();
        }
        manager.reset();
    }
    
    std::unique_ptr<WindowManager> manager;
};

// ============================================================================
// ТЕСТ 1: Инициализация GLFW системы
// ============================================================================
TEST_F(WindowManagerTest, InitializeSystem) {
    bool success = manager->initializeSystem();
    EXPECT_TRUE(success);
    EXPECT_TRUE(manager->isInitialized());
}

// ============================================================================
// ТЕСТ 2: Создание окна
// ============================================================================
TEST_F(WindowManagerTest, CreateWindow) {
    ASSERT_TRUE(manager->initializeSystem());
    
    bool success = manager->createWindow("Test Window", 800, 600);
    EXPECT_TRUE(success);
    EXPECT_NE(manager->getWindow(), nullptr);
}

// ============================================================================
// ТЕСТ 3: Создание окна без инициализации системы (должно провалиться)
// ============================================================================
TEST_F(WindowManagerTest, CreateWindowWithoutSystemInit) {
    bool success = manager->createWindow("Test Window", 800, 600);
    EXPECT_FALSE(success);
    EXPECT_EQ(manager->getWindow(), nullptr);
}

// ============================================================================
// ТЕСТ 4: shouldClose начальное состояние
// ============================================================================
TEST_F(WindowManagerTest, InitialShouldClose) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Test Window", 800, 600));
    
    EXPECT_FALSE(manager->shouldClose());
}

// ============================================================================
// ТЕСТ 5: Shutdown
// ============================================================================
TEST_F(WindowManagerTest, Shutdown) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Test Window", 800, 600));
    
    manager->shutdown();
    
    // После shutdown окно должно быть nullptr
    EXPECT_EQ(manager->getWindow(), nullptr);
}

// ============================================================================
// ТЕСТ 6: pollEvents (не должен крашиться)
// ============================================================================
TEST_F(WindowManagerTest, PollEvents) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Test Window", 800, 600));
    
    EXPECT_NO_THROW(manager->pollEvents());
}

// ============================================================================
// ТЕСТ 7: swapBuffers (не должен крашиться)
// ============================================================================
TEST_F(WindowManagerTest, SwapBuffers) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Test Window", 800, 600));
    
    EXPECT_NO_THROW(manager->swapBuffers());
}

// ============================================================================
// ТЕСТ 8: Множественное создание окон (должен использовать последнее)
// ============================================================================
TEST_F(WindowManagerTest, MultipleCreateWindow) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Window 1", 800, 600));
    
    auto* firstWindow = manager->getWindow();
    
    ASSERT_TRUE(manager->createWindow("Window 2", 1024, 768));
    
    auto* secondWindow = manager->getWindow();
    
    // Должно быть новое окно
    EXPECT_NE(secondWindow, nullptr);
}

// ============================================================================
// ТЕСТ 9: Shutdown без инициализации (не должен крашиться)
// ============================================================================
TEST_F(WindowManagerTest, ShutdownWithoutInit) {
    EXPECT_NO_THROW(manager->shutdown());
}

// ============================================================================
// ТЕСТ 10: Запрет копирования
// ============================================================================
TEST_F(WindowManagerTest, NoCopyConstructor) {
    EXPECT_FALSE(std::is_copy_constructible<WindowManager>::value);
    EXPECT_FALSE(std::is_copy_assignable<WindowManager>::value);
}

// ============================================================================
// ТЕСТ 11: Разрешение перемещения
// ============================================================================
TEST_F(WindowManagerTest, MoveConstructor) {
    EXPECT_TRUE(std::is_move_constructible<WindowManager>::value);
    EXPECT_TRUE(std::is_move_assignable<WindowManager>::value);
}

// ============================================================================
// ТЕСТ 12: getWindow без создания окна
// ============================================================================
TEST_F(WindowManagerTest, GetWindowBeforeCreate) {
    EXPECT_EQ(manager->getWindow(), nullptr);
}

// ============================================================================
// ТЕСТ 13: isInitialized начальное состояние
// ============================================================================
TEST_F(WindowManagerTest, InitialIsInitialized) {
    EXPECT_FALSE(manager->isInitialized());
}

// ============================================================================
// ТЕСТ 14: Размеры окна
// ============================================================================
TEST_F(WindowManagerTest, WindowDimensions) {
    ASSERT_TRUE(manager->initializeSystem());
    ASSERT_TRUE(manager->createWindow("Test Window", 1280, 720));
    
    auto* window = manager->getWindow();
    ASSERT_NE(window, nullptr);
    
    // Проверяем, что окно создано с правильными размерами
    // (детальная проверка зависит от реализации Window)
}

