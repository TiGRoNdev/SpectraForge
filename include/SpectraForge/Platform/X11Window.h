/**
 * @file X11Window.h
 * @brief Платформо-зависимая реализация окна для Linux (X11)
 */

#pragma once

#include "SpectraForge/Core/IWindow.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <string>
#include <memory>

namespace SpectraForge {
namespace Platform {

/**
 * @brief Реализация окна для Linux с использованием X11
 *
 * Эта реализация следует принципам:
 * - SRP: Отвечает только за управление X11 окном
 * - LSP: Может заменять любой IWindow
 * - DIP: Зависит от абстракции IWindow, а не наоборот
 */
class X11Window : public Core::IWindow {
public:
    X11Window();
    ~X11Window() override;

    // IWindow interface
    bool create(uint32_t width, uint32_t height, const std::string& title) override;
    bool isOpen() const override;
    bool update() override;
    void getSize(uint32_t& width, uint32_t& height) const override;
    void setSize(uint32_t width, uint32_t height) override;
    std::string getTitle() const override;
    void setTitle(const std::string& title) override;
    void* getNativeHandle() const override;
    void* getNativeDisplay() const override { return reinterpret_cast<void*>(display_); }
    void setWindowEventCallback(SpectraForge::Core::WindowEventCallback callback) override;
    void setMouseEventCallback(SpectraForge::Core::MouseEventCallback callback) override;
    void setKeyEventCallback(SpectraForge::Core::KeyEventCallback callback) override;
    void close() override;
    bool isFullscreenSupported() const override;
    void setFullscreen(bool fullscreen) override;
    bool isFullscreen() const override;

private:
    /**
     * @brief Обработка X11 события
     * @param event X11 событие
     */
    void processEvent(const XEvent& event);

    /**
     * @brief Конвертация X11 клавиши в наш KeyCode
     * @param keysym X11 код клавиши
     * @return Наш код клавиши
     */
    Core::KeyCode convertKeyCode(KeySym keysym) const;

    /**
     * @brief Конвертация X11 кнопки мыши в наш MouseButton
     * @param button X11 код кнопки
     * @return Наша кнопка мыши
     */
    Core::MouseButton convertMouseButton(int button) const;

    /**
     * @brief Отправка события окна
     * @param event Событие окна
     */
    void sendWindowEvent(Core::WindowEvent event);

    /**
     * @brief Отправка события мыши
     * @param mouseEvent Событие мыши
     */
    void sendMouseEvent(const Core::MouseEvent& mouseEvent);

    /**
     * @brief Отправка события клавиатуры
     * @param keyEvent Событие клавиатуры
     */
    void sendKeyEvent(const Core::KeyEvent& keyEvent);

private:
    Display* display_;                    ///< X11 дисплей
    Window window_;                       ///< X11 окно
    int screen_;                          ///< Номер экрана
    Atom wmDeleteWindow_;                 ///< Атом для обработки закрытия окна
    bool isOpen_;                         ///< Флаг активности окна
    bool isFullscreen_;                   ///< Флаг полноэкранного режима
    uint32_t width_;                      ///< Ширина окна
    uint32_t height_;                     ///< Высота окна
    std::string title_;                   ///< Заголовок окна

    // Callback функции
    SpectraForge::Core::WindowEventCallback windowEventCallback_;
    SpectraForge::Core::MouseEventCallback mouseEventCallback_;
    SpectraForge::Core::KeyEventCallback keyEventCallback_;
};

} // namespace Platform
} // namespace SpectraForge
