/**
 * @file X11Window.cpp
 * @brief Реализация X11 окна для Linux
 */

#include "SpectraForge/Platform/X11Window.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace SpectraForge {
namespace Platform {

X11Window::X11Window()
    : display_(nullptr)
    , window_(0)
    , screen_(0)
    , wmDeleteWindow_(0)
    , isOpen_(false)
    , isFullscreen_(false)
    , width_(0)
    , height_(0)
    , title_("")
{
}

X11Window::~X11Window() {
    close();
}

bool X11Window::create(uint32_t width, uint32_t height, const std::string& title) {
    if (isOpen_) {
        return false; // Окно уже создано
    }

    // Открываем дисплей
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        std::cerr << "❌ Не удалось открыть X11 дисплей" << std::endl;
        return false;
    }

    screen_ = DefaultScreen(display_);

    // Создаем окно
    Window root = RootWindow(display_, screen_);
    window_ = XCreateSimpleWindow(display_, root,
                                 0, 0, width, height, 1,
                                 BlackPixel(display_, screen_),
                                 WhitePixel(display_, screen_));

    if (!window_) {
        std::cerr << "❌ Не удалось создать X11 окно" << std::endl;
        XCloseDisplay(display_);
        display_ = nullptr;
        return false;
    }

    // Устанавливаем заголовок
    setTitle(title);

    // Устанавливаем протокол закрытия окна
    wmDeleteWindow_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDeleteWindow_, 1);

    // Показываем окно
    XMapWindow(display_, window_);
    XFlush(display_);

    // Сохраняем параметры
    width_ = width;
    height_ = height;
    title_ = title;
    isOpen_ = true;

    std::cout << "✅ X11 окно создано успешно: " << width << "x" << height
              << " \"" << title << "\"" << std::endl;

    return true;
}

bool X11Window::isOpen() const {
    return isOpen_;
}

bool X11Window::update() {
    if (!isOpen_) {
        return false;
    }

    // Обрабатываем все доступные события
    while (XPending(display_)) {
        XEvent event;
        XNextEvent(display_, &event);
        processEvent(event);
    }

    return isOpen_;
}

void X11Window::getSize(uint32_t& width, uint32_t& height) const {
    width = width_;
    height = height_;
}

void X11Window::setSize(uint32_t width, uint32_t height) {
    if (!isOpen_) return;

    // Изменяем размер окна
    XResizeWindow(display_, window_, width, height);
    XFlush(display_);

    // Обновляем сохраненные размеры
    width_ = width;
    height_ = height;

    // Отправляем событие изменения размера
    sendWindowEvent(Core::WindowEvent::Resized);
}

std::string X11Window::getTitle() const {
    return title_;
}

void X11Window::setTitle(const std::string& title) {
    if (!isOpen_) return;

    title_ = title;
    XStoreName(display_, window_, title_.c_str());
    XFlush(display_);
}

void* X11Window::getNativeHandle() const {
    return reinterpret_cast<void*>(window_);
}

void X11Window::setWindowEventCallback(SpectraForge::Core::WindowEventCallback callback) {
    windowEventCallback_ = std::move(callback);
}

void X11Window::setMouseEventCallback(SpectraForge::Core::MouseEventCallback callback) {
    mouseEventCallback_ = std::move(callback);
}

void X11Window::setKeyEventCallback(SpectraForge::Core::KeyEventCallback callback) {
    keyEventCallback_ = std::move(callback);
}

void X11Window::close() {
    if (!isOpen_) return;

    isOpen_ = false;

    if (window_) {
        XDestroyWindow(display_, window_);
        window_ = 0;
    }

    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }

    std::cout << "✅ X11 окно закрыто" << std::endl;
}

bool X11Window::isFullscreenSupported() const {
    return true; // X11 поддерживает полноэкранный режим
}

void X11Window::setFullscreen(bool fullscreen) {
    if (!isOpen_) return;

    if (fullscreen == isFullscreen_) return; // Уже в нужном режиме

    if (fullscreen) {
        // Включаем полноэкранный режим
        Atom wmState = XInternAtom(display_, "_NET_WM_STATE", False);
        Atom fullscreenAtom = XInternAtom(display_, "_NET_WM_STATE_FULLSCREEN", False);

        XEvent xev;
        xev.type = ClientMessage;
        xev.xclient.window = window_;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
        xev.xclient.data.l[1] = fullscreenAtom;
        xev.xclient.data.l[2] = 0;

        XSendEvent(display_, DefaultRootWindow(display_), False,
                  SubstructureRedirectMask | SubstructureNotifyMask, &xev);

        isFullscreen_ = true;
    } else {
        // Выключаем полноэкранный режим
        Atom wmState = XInternAtom(display_, "_NET_WM_STATE", False);
        Atom fullscreenAtom = XInternAtom(display_, "_NET_WM_STATE_FULLSCREEN", False);

        XEvent xev;
        xev.type = ClientMessage;
        xev.xclient.window = window_;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
        xev.xclient.data.l[1] = fullscreenAtom;
        xev.xclient.data.l[2] = 0;

        XSendEvent(display_, DefaultRootWindow(display_), False,
                  SubstructureRedirectMask | SubstructureNotifyMask, &xev);

        isFullscreen_ = false;
    }

    XFlush(display_);
}

bool X11Window::isFullscreen() const {
    return isFullscreen_;
}

void X11Window::processEvent(const XEvent& event) {
    switch (event.type) {
        case ClientMessage:
            if (event.xclient.data.l[0] == static_cast<long>(wmDeleteWindow_)) {
                sendWindowEvent(Core::WindowEvent::Closed);
                isOpen_ = false;
            }
            break;

        case ConfigureNotify: {
            // Событие изменения размера окна
            XConfigureEvent* configureEvent = const_cast<XConfigureEvent*>(&event.xconfigure);
            if (configureEvent->width != static_cast<int>(width_) ||
                configureEvent->height != static_cast<int>(height_)) {
                width_ = configureEvent->width;
                height_ = configureEvent->height;
                sendWindowEvent(Core::WindowEvent::Resized);
            }
            break;
        }

        case FocusIn:
            sendWindowEvent(Core::WindowEvent::FocusGained);
            break;

        case FocusOut:
            sendWindowEvent(Core::WindowEvent::FocusLost);
            break;

        case ButtonPress:
        case ButtonRelease: {
            // События мыши
            Core::MouseEvent mouseEvent;
            mouseEvent.x = event.xbutton.x;
            mouseEvent.y = event.xbutton.y;
            mouseEvent.button = convertMouseButton(event.xbutton.button);
            mouseEvent.pressed = (event.type == ButtonPress);
            mouseEvent.wheelDelta = 0; // X11 не предоставляет информацию о колесе

            sendMouseEvent(mouseEvent);
            break;
        }

        case KeyPress:
        case KeyRelease: {
            // События клавиатуры
            KeySym keysym = XLookupKeysym(const_cast<XKeyEvent*>(&event.xkey), 0);

            Core::KeyEvent keyEvent;
            keyEvent.key = convertKeyCode(keysym);
            keyEvent.pressed = (event.type == KeyPress);

            // Проверяем автоповтор
            if (event.type == KeyPress) {
                // Для простоты считаем автоповтор если клавиша уже была нажата
                // В реальности нужно отслеживать состояние клавиш
                keyEvent.repeat = false;
            } else {
                keyEvent.repeat = false;
            }

            keyEvent.modifiers = 0; // Для простоты игнорируем модификаторы

            sendKeyEvent(keyEvent);
            break;
        }

        default:
            break;
    }
}

Core::KeyCode X11Window::convertKeyCode(KeySym keysym) const {
    switch (keysym) {
        case XK_Escape: return Core::KeyCode::Escape;
        case XK_Return: return Core::KeyCode::Enter;
        case XK_Tab: return Core::KeyCode::Tab;
        case XK_BackSpace: return Core::KeyCode::Backspace;
        case XK_Insert: return Core::KeyCode::Insert;
        case XK_Delete: return Core::KeyCode::Delete;
        case XK_Right: return Core::KeyCode::Right;
        case XK_Left: return Core::KeyCode::Left;
        case XK_Down: return Core::KeyCode::Down;
        case XK_Up: return Core::KeyCode::Up;
        case XK_Page_Up: return Core::KeyCode::PageUp;
        case XK_Page_Down: return Core::KeyCode::PageDown;
        case XK_Home: return Core::KeyCode::Home;
        case XK_End: return Core::KeyCode::End;
        case XK_Caps_Lock: return Core::KeyCode::CapsLock;
        case XK_Scroll_Lock: return Core::KeyCode::ScrollLock;
        case XK_Num_Lock: return Core::KeyCode::NumLock;
        case XK_Print: return Core::KeyCode::PrintScreen;
        case XK_Pause: return Core::KeyCode::Pause;
        case XK_F1: return Core::KeyCode::F1;
        case XK_F2: return Core::KeyCode::F2;
        case XK_F3: return Core::KeyCode::F3;
        case XK_F4: return Core::KeyCode::F4;
        case XK_F5: return Core::KeyCode::F5;
        case XK_F6: return Core::KeyCode::F6;
        case XK_F7: return Core::KeyCode::F7;
        case XK_F8: return Core::KeyCode::F8;
        case XK_F9: return Core::KeyCode::F9;
        case XK_F10: return Core::KeyCode::F10;
        case XK_F11: return Core::KeyCode::F11;
        case XK_F12: return Core::KeyCode::F12;
        case XK_Shift_L: return Core::KeyCode::LeftShift;
        case XK_Control_L: return Core::KeyCode::LeftControl;
        case XK_Alt_L: return Core::KeyCode::LeftAlt;
        case XK_Super_L: return Core::KeyCode::LeftSuper;
        case XK_Shift_R: return Core::KeyCode::RightShift;
        case XK_Control_R: return Core::KeyCode::RightControl;
        case XK_Alt_R: return Core::KeyCode::RightAlt;
        case XK_Super_R: return Core::KeyCode::RightSuper;
        case XK_Menu: return Core::KeyCode::Menu;
        default:
            // Цифровые клавиши и буквы
            if (keysym >= XK_0 && keysym <= XK_9) {
                return static_cast<Core::KeyCode>(keysym);
            }
            if (keysym >= XK_A && keysym <= XK_Z) {
                return static_cast<Core::KeyCode>(keysym);
            }
            if (keysym >= XK_a && keysym <= XK_z) {
                // Маленькие буквы конвертируем в большие
                return static_cast<Core::KeyCode>(keysym - XK_a + XK_A);
            }
            return Core::KeyCode::Unknown;
    }
}

Core::MouseButton X11Window::convertMouseButton(int button) const {
    switch (button) {
        case 1: return Core::MouseButton::Left;
        case 2: return Core::MouseButton::Middle;
        case 3: return Core::MouseButton::Right;
        case 4: return Core::MouseButton::Extra1;
        case 5: return Core::MouseButton::Extra2;
        default: return Core::MouseButton::Left;
    }
}

void X11Window::sendWindowEvent(Core::WindowEvent event) {
    if (windowEventCallback_) {
        Core::WindowEventData eventData;
        eventData.type = event;
        eventData.width = width_;
        eventData.height = height_;

        try {
            windowEventCallback_(eventData);
        } catch (const std::exception& e) {
            std::cerr << "❌ Ошибка в callback окна: " << e.what() << std::endl;
        }
    }
}

void X11Window::sendMouseEvent(const Core::MouseEvent& mouseEvent) {
    if (mouseEventCallback_) {
        try {
            mouseEventCallback_(mouseEvent);
        } catch (const std::exception& e) {
            std::cerr << "❌ Ошибка в callback мыши: " << e.what() << std::endl;
        }
    }
}

void X11Window::sendKeyEvent(const Core::KeyEvent& keyEvent) {
    if (keyEventCallback_) {
        try {
            keyEventCallback_(keyEvent);
        } catch (const std::exception& e) {
            std::cerr << "❌ Ошибка в callback клавиатуры: " << e.what() << std::endl;
        }
    }
}

} // namespace Platform
} // namespace SpectraForge
