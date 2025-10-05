/**
 * @file IWindow.h
 * @brief Абстрактный интерфейс для оконной системы
 *
 * Следуя принципам SOLID:
 * - SRP: Интерфейс отвечает только за управление окном
 * - DIP: Зависит от абстракций, а не от конкретных реализаций
 * - OCP: Открыт для расширения через наследование
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <cstdint>

namespace SpectraForge {
namespace Core {

/**
 * @brief Перечисление событий окна
 */
enum class WindowEvent {
    Closed,           ///< Окно закрыто
    Resized,          ///< Окно изменено в размере
    FocusGained,      ///< Окно получило фокус
    FocusLost,        ///< Окно потеряло фокус
    Minimized,        ///< Окно свернуто
    Restored         ///< Окно восстановлено
};

/**
 * @brief Перечисление кнопок мыши
 */
enum class MouseButton {
    Left = 1,
    Middle = 2,
    Right = 3,
    Extra1 = 4,
    Extra2 = 5
};

/**
 * @brief Перечисление клавиш клавиатуры
 */
enum class KeyCode {
    Unknown = 0,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
    // Цифровые клавиши
    Key0 = 48, Key1 = 49, Key2 = 50, Key3 = 51, Key4 = 52,
    Key5 = 53, Key6 = 54, Key7 = 55, Key8 = 56, Key9 = 57,
    A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71,
    H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78,
    O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85,
    V = 86, W = 87, X = 88, Y = 89, Z = 90
};

/**
 * @brief Структура данных о событии мыши
 */
struct MouseEvent {
    int x, y;                    ///< Координаты курсора
    MouseButton button;          ///< Нажатая кнопка мыши
    bool pressed;                ///< Состояние кнопки (true = нажата)
    int wheelDelta;              ///< Прокрутка колеса (положительное = вверх)
};

/**
 * @brief Структура данных о событии клавиатуры
 */
struct KeyEvent {
    KeyCode key;                 ///< Код клавиши
    bool pressed;                ///< Состояние клавиши (true = нажата)
    bool repeat;                 ///< Повтор нажатия (автоповтор)
    int modifiers;               ///< Модификаторы (Shift, Ctrl, Alt)
};

/**
 * @brief Структура данных о событии окна
 */
struct WindowEventData {
    WindowEvent type;            ///< Тип события
    int width, height;           ///< Размеры окна (для события изменения размера)
};

/**
 * @brief Callback-функции для обработки событий
 */
using WindowEventCallback = std::function<void(const WindowEventData&)>;
using MouseEventCallback = std::function<void(const MouseEvent&)>;
using KeyEventCallback = std::function<void(const KeyEvent&)>;

/**
 * @brief Абстрактный интерфейс для оконной системы
 *
 * Определяет контракт для работы с окнами независимо от платформы.
 * Позволяет рендереру работать с любым окном без знания деталей реализации.
 */
class IWindow {
public:
    virtual ~IWindow() = default;

    /**
     * @brief Создание окна
     * @param width Ширина окна
     * @param height Высота окна
     * @param title Заголовок окна
     * @return true если окно создано успешно
     */
    virtual bool create(uint32_t width, uint32_t height, const std::string& title) = 0;

    /**
     * @brief Проверка, активно ли окно
     * @return true если окно активно и не закрыто
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Обновление окна (обработка событий)
     * @return true если окно должно продолжать работать
     */
    virtual bool update() = 0;

    /**
     * @brief Получение размеров окна
     * @param width Выходной параметр для ширины
     * @param height Выходной параметр для высоты
     */
    virtual void getSize(uint32_t& width, uint32_t& height) const = 0;

    /**
     * @brief Установка размеров окна
     * @param width Новая ширина
     * @param height Новая высота
     */
    virtual void setSize(uint32_t width, uint32_t height) = 0;

    /**
     * @brief Получение заголовка окна
     * @return Заголовок окна
     */
    virtual std::string getTitle() const = 0;

    /**
     * @brief Установка заголовка окна
     * @param title Новый заголовок
     */
    virtual void setTitle(const std::string& title) = 0;

    /**
     * @brief Получение платформенно-зависимого указателя на окно
     * @return Указатель на нативное окно (для интеграции с Vulkan)
     */
    virtual void* getNativeHandle() const = 0;

    /**
     * @brief Получение платформенно-зависимого указателя на дисплей/контекст оконной системы
     * @return Указатель на нативный дисплей (для X11: Display*)
     */
    virtual void* getNativeDisplay() const = 0;

    /**
     * @brief Регистрация callback-функции для событий окна
     * @param callback Функция обратного вызова
     */
    virtual void setWindowEventCallback(WindowEventCallback callback) = 0;

    /**
     * @brief Регистрация callback-функции для событий мыши
     * @param callback Функция обратного вызова
     */
    virtual void setMouseEventCallback(MouseEventCallback callback) = 0;

    /**
     * @brief Регистрация callback-функции для событий клавиатуры
     * @param callback Функция обратного вызова
     */
    virtual void setKeyEventCallback(KeyEventCallback callback) = 0;

    /**
     * @brief Закрытие окна
     */
    virtual void close() = 0;

    /**
     * @brief Проверка поддержки полноэкранного режима
     * @return true если полноэкранный режим поддерживается
     */
    virtual bool isFullscreenSupported() const = 0;

    /**
     * @brief Включение/выключение полноэкранного режима
     * @param fullscreen true для полноэкранного режима
     */
    virtual void setFullscreen(bool fullscreen) = 0;

    /**
     * @brief Проверка, в полноэкранном ли режиме окно
     * @return true если окно в полноэкранном режиме
     */
    virtual bool isFullscreen() const = 0;
};

} // namespace Core
} // namespace SpectraForge
