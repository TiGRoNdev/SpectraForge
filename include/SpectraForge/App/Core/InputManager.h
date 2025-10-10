/**
 * @file InputManager.h
 * @brief Input handling and event processing (P0.3 - SRP compliant)
 * 
 * SOLID COMPLIANCE:
 * - SRP ✅: Only input state management
 * - DIP ✅: Abstracted from GLFW details
 * - ISP ✅: Clean input query interface
 */

#pragma once

#include <functional>
#include <GLFW/glfw3.h>

namespace SpectraForge {
namespace App {
namespace Core {

/**
 * @brief Input state tracking
 */
struct InputState {
    bool keys[512] = {false};
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float deltaMouseX = 0.0f;
    float deltaMouseY = 0.0f;
    bool mouseButtons[8] = {false};
    bool firstMouse = true;
};

/**
 * @brief Manages keyboard and mouse input
 * 
 * Single Responsibility: Track input state and provide query interface
 */
class InputManager {
public:
    InputManager();
    ~InputManager() = default;
    
    /**
     * @brief Setup input callbacks for window
     */
    void setupCallbacks(GLFWwindow* window);
    
    /**
     * @brief Update input state (call before processing)
     */
    void update();
    
    /**
     * @brief Check if key is pressed
     */
    bool isKeyPressed(int key) const;
    
    /**
     * @brief Check if key was just pressed (single frame)
     */
    bool isKeyJustPressed(int key) const;
    
    /**
     * @brief Check if mouse button is pressed
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * @brief Get mouse position
     */
    void getMousePosition(float& x, float& y) const;
    
    /**
     * @brief Get mouse delta (movement since last frame)
     */
    void getMouseDelta(float& dx, float& dy) const;
    
    /**
     * @brief Get raw input state (for compatibility)
     */
    const InputState* getState() const { return &state_; }
    
    /**
     * @brief Reset input state
     */
    void reset();
    
    /**
     * @brief Set mouse capture mode
     */
    void setMouseCaptured(bool captured);
    
private:
    InputState state_;
    InputState previousState_;  // For "just pressed" detection
    bool mouseCaptured_ = false;
    
    // GLFW callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
};

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

