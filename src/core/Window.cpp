/**
 * @file Window.cpp
 * @brief Минимальная реализация окна на GLFW для App::Engine
 */

#include "SpectraForge/Core/Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <string>

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
            std::cerr << "[Window] ❌ GLFW инициализация не удалась!" << std::endl;
            return false;
        }
        g_glfw_initialized = true;
        std::cout << "[Window] ✅ GLFW инициализирован" << std::endl;
    }

    // Если используется Vulkan presentation, не создаём GL контекст
    const char* useVulkan = std::getenv("SPECTRAFORGE_PRESENT_VULKAN");
    if (!useVulkan || std::string(useVulkan) != "1") {
        std::cout << "[Window] 📋 Режим: OpenGL" << std::endl;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.openglMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.openglMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, config.openglCore ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_ANY_PROFILE);
    } else {
        std::cout << "[Window] 📋 Режим: Vulkan" << std::endl;
        
        // КРИТИЧЕСКИ ВАЖНО: проверяем поддержку Vulkan в GLFW ДО установки hints
        std::cout << "[Window] 🔍 Проверка поддержки Vulkan в GLFW..." << std::endl;
        if (!glfwVulkanSupported()) {
            std::cerr << "[Window] ❌ GLFW не поддерживает Vulkan!" << std::endl;
            std::cerr << "  Возможные причины:" << std::endl;
            std::cerr << "  1) GLFW скомпилирован без поддержки Vulkan" << std::endl;
            std::cerr << "  2) Vulkan loader не установлен (vulkan-loader, mesa-vulkan-drivers)" << std::endl;
            std::cerr << "  3) Драйверы GPU не поддерживают Vulkan" << std::endl;
            std::cerr << std::endl;
            std::cerr << "  Установите: sudo apt install libvulkan1 mesa-vulkan-drivers vulkan-tools" << std::endl;
            std::cerr << "  Проверка: vulkaninfo" << std::endl;
            return false;
        }
        std::cout << "[Window] ✅ GLFW поддерживает Vulkan" << std::endl;
        
        // Создаём окно без текущего GL контекста (Vulkan path сам создаёт VkSurfaceKHR)
        std::cout << "[Window] 🔧 Устанавливаем GLFW_CLIENT_API = GLFW_NO_API" << std::endl;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);

    std::cout << "[Window] 🪟 Создание окна " << config.width << "x" << config.height << "..." << std::endl;
    window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "[Window] ❌ Не удалось создать окно GLFW!" << std::endl;
        return false;
    }
    std::cout << "[Window] ✅ Окно создано успешно" << std::endl;
    
    // Проверяем расширения Vulkan ПОСЛЕ создания окна (только для Vulkan режима)
    if (useVulkan && std::string(useVulkan) == "1") {
        std::cout << "[Window] 🔍 Проверка Vulkan расширений после создания окна..." << std::endl;
        uint32_t testExtCount = 0;
        const char** testExt = glfwGetRequiredInstanceExtensions(&testExtCount);
        if (!testExt || testExtCount == 0) {
            std::cerr << "[Window] ❌ glfwGetRequiredInstanceExtensions() вернул NULL!" << std::endl;
            std::cerr << "  Это критическая ошибка GLFW-Vulkan интеграции." << std::endl;
            std::cerr << "  Проверьте: sudo apt install libvulkan1 mesa-vulkan-drivers" << std::endl;
            return false;
        }
        std::cout << "[Window] ✅ GLFW предоставляет " << testExtCount << " Vulkan расширений:" << std::endl;
        for (uint32_t i = 0; i < testExtCount; ++i) {
            std::cout << "    - " << testExt[i] << std::endl;
        }
    }

    g_last_window = window;  // Сохраняем для доступа из рендерера

    // Устанавливаем user pointer для доступа в callback'ах
    glfwSetWindowUserPointer(window, this);

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
    // Не блокирующий опрос — поддерживает реакцию ESC и других событий
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
    // Минимальная обработка ESC: закрыть окно
    if (window) {
        glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            (void)scancode; (void)mods;
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
        });
    }
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


