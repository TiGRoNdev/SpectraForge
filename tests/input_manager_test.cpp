/**
 * @file input_manager_test.cpp
 * @brief Unit tests for InputManager (P0.3 TDD)
 */

#include <gtest/gtest.h>
#include "SpectraForge/App/Core/InputManager.h"

using namespace SpectraForge::App::Core;

class InputManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<InputManager>();
    }
    
    void TearDown() override {
        manager.reset();
    }
    
    std::unique_ptr<InputManager> manager;
};

// ============================================================================
// ТЕСТ 1: Начальное состояние
// ============================================================================
TEST_F(InputManagerTest, InitialState) {
    // Все клавиши должны быть не нажаты
    for (int key = 0; key < 512; ++key) {
        EXPECT_FALSE(manager->isKeyPressed(key));
    }
    
    // Все кнопки мыши должны быть не нажаты
    for (int button = 0; button < 8; ++button) {
        EXPECT_FALSE(manager->isMouseButtonPressed(button));
    }
    
    // Позиция мыши должна быть (0, 0)
    float x, y;
    manager->getMousePosition(x, y);
    EXPECT_EQ(x, 0.0f);
    EXPECT_EQ(y, 0.0f);
}

// ============================================================================
// ТЕСТ 2: update() не крашится
// ============================================================================
TEST_F(InputManagerTest, UpdateDoesNotCrash) {
    EXPECT_NO_THROW(manager->update());
}

// ============================================================================
// ТЕСТ 3: reset() сбрасывает состояние
// ============================================================================
TEST_F(InputManagerTest, ResetState) {
    // Симулируем нажатие клавиши через прямое обращение к state
    auto* state = const_cast<InputState*>(manager->getState());
    state->keys[GLFW_KEY_W] = true;
    state->mouseButtons[0] = true;
    state->mouseX = 100.0f;
    state->mouseY = 200.0f;
    
    manager->reset();
    
    // После reset всё должно быть сброшено
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_W));
    EXPECT_FALSE(manager->isMouseButtonPressed(0));
    
    float x, y;
    manager->getMousePosition(x, y);
    EXPECT_EQ(x, 0.0f);
    EXPECT_EQ(y, 0.0f);
}

// ============================================================================
// ТЕСТ 4: isKeyPressed
// ============================================================================
TEST_F(InputManagerTest, IsKeyPressed) {
    // Начально не нажата
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_W));
    
    // Симулируем нажатие
    auto* state = const_cast<InputState*>(manager->getState());
    state->keys[GLFW_KEY_W] = true;
    
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_W));
}

// ============================================================================
// ТЕСТ 5: isKeyJustPressed (single frame detection)
// ============================================================================
TEST_F(InputManagerTest, IsKeyJustPressed) {
    auto* state = const_cast<InputState*>(manager->getState());
    
    // Первый кадр: клавиша нажата
    state->keys[GLFW_KEY_SPACE] = true;
    EXPECT_TRUE(manager->isKeyJustPressed(GLFW_KEY_SPACE));
    
    // Update (сохраняет предыдущее состояние)
    manager->update();
    
    // Второй кадр: клавиша всё ещё нажата
    // Но isKeyJustPressed должен вернуть false (не "just pressed")
    EXPECT_FALSE(manager->isKeyJustPressed(GLFW_KEY_SPACE));
}

// ============================================================================
// ТЕСТ 6: isMouseButtonPressed
// ============================================================================
TEST_F(InputManagerTest, IsMouseButtonPressed) {
    EXPECT_FALSE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
    
    auto* state = const_cast<InputState*>(manager->getState());
    state->mouseButtons[GLFW_MOUSE_BUTTON_LEFT] = true;
    
    EXPECT_TRUE(manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT));
}

// ============================================================================
// ТЕСТ 7: getMousePosition
// ============================================================================
TEST_F(InputManagerTest, GetMousePosition) {
    auto* state = const_cast<InputState*>(manager->getState());
    state->mouseX = 123.45f;
    state->mouseY = 678.90f;
    
    float x, y;
    manager->getMousePosition(x, y);
    
    EXPECT_FLOAT_EQ(x, 123.45f);
    EXPECT_FLOAT_EQ(y, 678.90f);
}

// ============================================================================
// ТЕСТ 8: getMouseDelta
// ============================================================================
TEST_F(InputManagerTest, GetMouseDelta) {
    auto* state = const_cast<InputState*>(manager->getState());
    state->deltaMouseX = 10.5f;
    state->deltaMouseY = -5.2f;
    
    float dx, dy;
    manager->getMouseDelta(dx, dy);
    
    EXPECT_FLOAT_EQ(dx, 10.5f);
    EXPECT_FLOAT_EQ(dy, -5.2f);
}

// ============================================================================
// ТЕСТ 9: getState возвращает корректный указатель
// ============================================================================
TEST_F(InputManagerTest, GetState) {
    const InputState* state = manager->getState();
    EXPECT_NE(state, nullptr);
    
    // Проверяем, что это действительно актуальное состояние
    auto* mutableState = const_cast<InputState*>(state);
    mutableState->keys[GLFW_KEY_A] = true;
    
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_A));
}

// ============================================================================
// ТЕСТ 10: setMouseCaptured
// ============================================================================
TEST_F(InputManagerTest, SetMouseCaptured) {
    // Без окна это не должно крашиться
    EXPECT_NO_THROW(manager->setMouseCaptured(true));
    EXPECT_NO_THROW(manager->setMouseCaptured(false));
}

// ============================================================================
// ТЕСТ 11: Множественные клавиши одновременно
// ============================================================================
TEST_F(InputManagerTest, MultipleKeysPressed) {
    auto* state = const_cast<InputState*>(manager->getState());
    state->keys[GLFW_KEY_W] = true;
    state->keys[GLFW_KEY_A] = true;
    state->keys[GLFW_KEY_S] = true;
    state->keys[GLFW_KEY_D] = true;
    
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_W));
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_A));
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_S));
    EXPECT_TRUE(manager->isKeyPressed(GLFW_KEY_D));
    EXPECT_FALSE(manager->isKeyPressed(GLFW_KEY_Q));
}

// ============================================================================
// ТЕСТ 12: Граничные значения клавиш
// ============================================================================
TEST_F(InputManagerTest, KeyBoundaryValues) {
    EXPECT_NO_THROW(manager->isKeyPressed(0));
    EXPECT_NO_THROW(manager->isKeyPressed(511));
    
    // Вне диапазона (должен обрабатываться безопасно)
    // Это зависит от реализации, но не должно крашиться
}

// ============================================================================
// ТЕСТ 13: Граничные значения кнопок мыши
// ============================================================================
TEST_F(InputManagerTest, MouseButtonBoundaryValues) {
    EXPECT_NO_THROW(manager->isMouseButtonPressed(0));
    EXPECT_NO_THROW(manager->isMouseButtonPressed(7));
}

// ============================================================================
// ТЕСТ 14: firstMouse flag
// ============================================================================
TEST_F(InputManagerTest, FirstMouseFlag) {
    const InputState* state = manager->getState();
    EXPECT_TRUE(state->firstMouse);
    
    // После первого движения мыши флаг должен сброситься
    // (это проверится после интеграции с callbacks)
}

