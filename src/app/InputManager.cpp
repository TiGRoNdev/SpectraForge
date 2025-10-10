/**
 * @file InputManager.cpp
 * @brief Implementation of InputManager (P0.3 - SRP compliant)
 */

#include "SpectraForge/App/Core/InputManager.h"
#include <cstring>

namespace SpectraForge {
namespace App {
namespace Core {

// Global pointer for callbacks (GLFW требует C-style callbacks)
static InputManager* g_inputManager = nullptr;

InputManager::InputManager() 
    : state_(),
      previousState_(),
      mouseCaptured_(false) {
    // Initialize state
    std::memset(state_.keys, 0, sizeof(state_.keys));
    std::memset(state_.mouseButtons, 0, sizeof(state_.mouseButtons));
    std::memset(previousState_.keys, 0, sizeof(previousState_.keys));
    std::memset(previousState_.mouseButtons, 0, sizeof(previousState_.mouseButtons));
}

void InputManager::setupCallbacks(GLFWwindow* window) {
    if (!window) return;
    
    // Set this as the global instance for callbacks
    g_inputManager = this;
    
    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window, this);
    
    // Register callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
}

void InputManager::update() {
    // Save current state as previous
    previousState_ = state_;
    
    // Reset deltas
    state_.deltaMouseX = 0.0f;
    state_.deltaMouseY = 0.0f;
}

bool InputManager::isKeyPressed(int key) const {
    if (key < 0 || key >= 512) {
        return false;
    }
    return state_.keys[key];
}

bool InputManager::isKeyJustPressed(int key) const {
    if (key < 0 || key >= 512) {
        return false;
    }
    // Just pressed = pressed now, but not pressed in previous frame
    return state_.keys[key] && !previousState_.keys[key];
}

bool InputManager::isMouseButtonPressed(int button) const {
    if (button < 0 || button >= 8) {
        return false;
    }
    return state_.mouseButtons[button];
}

void InputManager::getMousePosition(float& x, float& y) const {
    x = state_.mouseX;
    y = state_.mouseY;
}

void InputManager::getMouseDelta(float& dx, float& dy) const {
    dx = state_.deltaMouseX;
    dy = state_.deltaMouseY;
}

void InputManager::reset() {
    std::memset(state_.keys, 0, sizeof(state_.keys));
    std::memset(state_.mouseButtons, 0, sizeof(state_.mouseButtons));
    std::memset(previousState_.keys, 0, sizeof(previousState_.keys));
    std::memset(previousState_.mouseButtons, 0, sizeof(previousState_.mouseButtons));
    
    state_.mouseX = 0.0f;
    state_.mouseY = 0.0f;
    state_.deltaMouseX = 0.0f;
    state_.deltaMouseY = 0.0f;
    state_.firstMouse = true;
}

void InputManager::setMouseCaptured(bool captured) {
    mouseCaptured_ = captured;
    // Note: Actual cursor mode setting requires GLFWwindow*
    // This would be handled by the caller with glfwSetInputMode
}

// ============================================================================
// GLFW Callbacks (Static)
// ============================================================================

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!manager || key < 0 || key >= 512) {
        return;
    }
    
    if (action == GLFW_PRESS) {
        manager->state_.keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        manager->state_.keys[key] = false;
    }
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!manager || button < 0 || button >= 8) {
        return;
    }
    
    if (action == GLFW_PRESS) {
        manager->state_.mouseButtons[button] = true;
    } else if (action == GLFW_RELEASE) {
        manager->state_.mouseButtons[button] = false;
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!manager) {
        return;
    }
    
    if (manager->state_.firstMouse) {
        manager->state_.mouseX = static_cast<float>(xpos);
        manager->state_.mouseY = static_cast<float>(ypos);
        manager->state_.firstMouse = false;
        return;
    }
    
    // Calculate delta
    manager->state_.deltaMouseX = static_cast<float>(xpos) - manager->state_.mouseX;
    manager->state_.deltaMouseY = static_cast<float>(ypos) - manager->state_.mouseY;
    
    // Update position
    manager->state_.mouseX = static_cast<float>(xpos);
    manager->state_.mouseY = static_cast<float>(ypos);
}

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

