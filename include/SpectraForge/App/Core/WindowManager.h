/**
 * @file WindowManager.h
 * @brief Window management component (P0.3 - SRP compliant)
 * 
 * SOLID COMPLIANCE:
 * - SRP ✅: Only window lifecycle management
 * - DIP ✅: Abstracted from Engine
 * - RAII ✅: Automatic cleanup
 */

#pragma once

#include <memory>
#include <string>
#include "SpectraForge/App/Core/Interfaces/IWindowManager.h"
#include "SpectraForge/Core/Window.h"

namespace SpectraForge {
namespace App {
namespace Core {

/**
 * @brief Manages GLFW window lifecycle
 * 
 * Single Responsibility: Window creation, configuration, and cleanup
 */
class WindowManager : public IWindowManager {
public:
    WindowManager();
    ~WindowManager();
    
    // Delete copy
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    // Allow move
    WindowManager(WindowManager&&) = default;
    WindowManager& operator=(WindowManager&&) = default;
    
    /**
     * @brief Initialize GLFW system
     */
    bool initializeSystem() override;
    
    /**
     * @brief Create window with specified parameters
     */
    bool createWindow(const std::string& title, uint32_t width, uint32_t height) override;
    
    /**
     * @brief Get window instance
     */
    SpectraForge::Core::Window* getWindow() const override { return window_.get(); }
    
    /**
     * @brief Check if window should close
     */
    bool shouldClose() const override;
    
    /**
     * @brief Poll window events
     */
    void pollEvents() override;
    
    /**
     * @brief Swap buffers
     */
    void swapBuffers() override;
    
    /**
     * @brief Shutdown window system
     */
    void shutdown() override;
    
    /**
     * @brief Check if initialized
     */
    bool isInitialized() const override { return initialized_; }
    
private:
    std::unique_ptr<SpectraForge::Core::Window> window_;
    bool initialized_ = false;
    bool systemInitialized_ = false;
};

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

