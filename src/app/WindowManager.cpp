/**
 * @file WindowManager.cpp
 * @brief Implementation of WindowManager (P0.3 - SRP compliant)
 */

#include "SpectraForge/App/Core/WindowManager.h"
#include "SpectraForge/Core/Logger.h"
#include <GLFW/glfw3.h>

namespace SpectraForge {
namespace App {
namespace Core {

WindowManager::WindowManager() 
    : window_(nullptr),
      initialized_(false),
      systemInitialized_(false) {
}

WindowManager::~WindowManager() {
    shutdown();
}

bool WindowManager::initializeSystem() {
    if (systemInitialized_) {
        return true; // Already initialized
    }
    
    if (!glfwInit()) {
        // Error logging (requires logger instance)
        return false;
    }
    
    systemInitialized_ = true;
    return true;
}

bool WindowManager::createWindow(const std::string& title, uint32_t width, uint32_t height) {
    if (!systemInitialized_) {
        return false;
    }
    
    // Destroy existing window if any
    if (window_) {
        window_.reset();
    }
    
    // Create window config
    SpectraForge::Core::Window::Config config;
    config.width = width;
    config.height = height;
    config.title = title;
    config.resizable = true;
    config.vSync = true;
    
    // Create new window
    window_ = std::make_unique<SpectraForge::Core::Window>(config);
    
    if (!window_ || !window_->getNativeWindow()) {
        window_.reset();
        initialized_ = false;
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool WindowManager::shouldClose() const {
    if (!window_) {
        return true;
    }
    return window_->shouldClose();
}

void WindowManager::pollEvents() {
    if (systemInitialized_) {
        glfwPollEvents();
    }
}

void WindowManager::swapBuffers() {
    if (window_) {
        window_->swapBuffers();
    }
}

void WindowManager::shutdown() {
    if (window_) {
        window_.reset();
        initialized_ = false;
    }
    
    if (systemInitialized_) {
        glfwTerminate();
        systemInitialized_ = false;
    }
}

}  // namespace Core
}  // namespace App
}  // namespace SpectraForge

