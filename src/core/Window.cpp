/**
 * @file Window.cpp
 * @brief Минимальная реализация окна на GLFW для App::Engine
 */

#include "SpectraForge/Core/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace SpectraForge {
namespace Core {

namespace {
    static bool g_glfw_initialized = false;
    static GLFWwindow* g_last_window = nullptr;
}

Window::Window(const Config& cfg)
    : config(cfg), window(nullptr), currentMode(cfg.mode),
      windowedWidth(cfg.width), windowedHeight(cfg.height),
      windowedPosX(100), windowedPosY(100) {}

Window::~Window() {
    cleanup();
}

bool Window::initialize() {
    if (!g_glfw_initialized) {
        if (!glfwInit()) {
            return false;
        }
        g_glfw_initialized = true;
    }

    // Если используется Vulkan presentation, не создаём GL контекст
    const char* useVulkan = std::getenv("SPECTRAFORGE_PRESENT_VULKAN");
    if (!useVulkan || std::string(useVulkan) != "1") {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.openglMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.openglMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, config.openglCore ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_ANY_PROFILE);
    } else {
        // Создаём окно без текущего GL контекста (Vulkan path сам создаёт VkSurfaceKHR)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        
        // КРИТИЧЕСКИ ВАЖНО: проверяем поддержку Vulkan в GLFW
        if (!glfwVulkanSupported()) {
            return false;
        }
    }
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);

    window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    if (!window) {
        return false;
    }

    g_last_window = window;  // Сохраняем для доступа из рендерера

    if (!useVulkan || std::string(useVulkan) != "1") {
        makeContextCurrent();
        setVSync(config.vSync);
    }
    setupCallbacks();
    return true;
}

void Window::close() {
    if (window) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

bool Window::shouldClose() const {
    return window ? glfwWindowShouldClose(window) == GLFW_TRUE : true;
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    if (window) {
        glfwSwapBuffers(window);
    }
}

Math::Vector2 Window::getFramebufferSize() const {
    if (!window) {
        return { static_cast<float>(config.width), static_cast<float>(config.height) };
    }
    int w = 0, h = 0;
    glfwGetFramebufferSize(window, &w, &h);
    return { static_cast<float>(w), static_cast<float>(h) };
}

void Window::makeContextCurrent() {
    if (window) {
        glfwMakeContextCurrent(window);
    }
}

void Window::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    config.vSync = enabled;
}

void Window::setupCallbacks() {
    // Базовые callback'и — заполняются при необходимости в будущем
}

void Window::cleanup() {
    if (window) {
        if (window == g_last_window) {
            g_last_window = nullptr;
        }
        glfwDestroyWindow(window);
        window = nullptr;
    }
}

GLFWwindow* Window::getLastCreatedWindow() {
    return g_last_window;
}

bool Window::initializeSystem() {
    if (!g_glfw_initialized) {
        g_glfw_initialized = glfwInit();
    }
    return g_glfw_initialized;
}

void Window::terminateSystem() {
    if (g_glfw_initialized) {
        glfwTerminate();
        g_glfw_initialized = false;
    }
}

} // namespace Core
} // namespace SpectraForge


