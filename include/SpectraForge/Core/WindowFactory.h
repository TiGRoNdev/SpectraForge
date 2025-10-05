/**
 * @file WindowFactory.h
 * @brief Фабрика для создания окон в зависимости от платформы
 */

#pragma once

#include "SpectraForge/Core/IWindow.h"
#include <memory>

namespace SpectraForge {
namespace Core {

/**
 * @brief Фабрика для создания окон
 *
 * Следуя принципам:
 * - SRP: Отвечает только за создание окон
 * - OCP: Легко расширяется для новых платформ
 * - DIP: Возвращает абстракцию IWindow
 */
class WindowFactory {
public:
    /**
     * @brief Создание окна для текущей платформы
     * @param width Ширина окна
     * @param height Высота окна
     * @param title Заголовок окна
     * @return Умный указатель на созданное окно или nullptr при ошибке
     */
    static std::unique_ptr<IWindow> createWindow(uint32_t width, uint32_t height, const std::string& title);

    /**
     * @brief Проверка поддержки оконной системы на текущей платформе
     * @return true если оконная система поддерживается
     */
    static bool isWindowSystemSupported();

private:
    // Запрещаем создание экземпляров фабрики
    WindowFactory() = delete;
    ~WindowFactory() = delete;
    WindowFactory(const WindowFactory&) = delete;
    WindowFactory& operator=(const WindowFactory&) = delete;
};

} // namespace Core
} // namespace SpectraForge
