/**
 * @file WindowFactory.cpp
 * @brief Реализация фабрики окон
 */

#include "SpectraForge/Core/WindowFactory.h"
#include "SpectraForge/Platform/X11Window.h"
#include <iostream>

namespace SpectraForge {
namespace Core {

std::unique_ptr<IWindow> WindowFactory::createWindow(uint32_t width, uint32_t height, const std::string& title) {
    // Определяем платформу и создаем соответствующее окно
    #if defined(__linux__) && !defined(__ANDROID__)
        // Linux платформа - используем X11
        auto window = std::make_unique<Platform::X11Window>();
        if (window->create(width, height, title)) {
            return window;
        } else {
            std::cerr << "❌ Не удалось создать X11 окно" << std::endl;
            return nullptr;
        }
    #else
        #error "Платформа не поддерживается. Добавьте реализацию для вашей платформы."
        return nullptr;
    #endif
}

bool WindowFactory::isWindowSystemSupported() {
    #if defined(__linux__) && !defined(__ANDROID__)
        // Проверяем доступность X11 дисплея
        Display* display = XOpenDisplay(nullptr);
        if (display) {
            XCloseDisplay(display);
            return true;
        }
        return false;
    #else
        return false;
    #endif
}

} // namespace Core
} // namespace SpectraForge
