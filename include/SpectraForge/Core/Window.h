
#pragma once

#include <functional>
#include <memory>
#include <string>
#include "../Math/Vector2.h"

// Forward declarations
struct GLFWwindow;

namespace SpectraForge {
namespace Core {

/**
 * @brief Класс управления окном приложения
 *
 * Window предоставляет кроссплатформенный интерфейс для создания
 * и управления окном приложения, обработки событий и контекста рендеринга.
 */
class Window {
  public:
    /**
     * @brief Режимы окна
     */
    enum WindowMode {
        WINDOWED,          ///< Оконный режим
        FULLSCREEN,        ///< Полноэкранный режим
        BORDERLESS_WINDOW  ///< Безрамочное окно
    };

    /**
     * @brief Конфигурация окна
     */
    struct Config {
        std::string title = "HyperEngine Application";  ///< Заголовок окна
        int width = 1280;                               ///< Ширина окна
        int height = 720;                               ///< Высота окна
        WindowMode mode = WINDOWED;                     ///< Режим окна
        bool resizable = true;   ///< Возможность изменения размера
        bool vSync = true;       ///< Вертикальная синхронизация
        bool decorated = true;   ///< Рамка окна
        bool focused = true;     ///< Фокус при создании
        bool floating = false;   ///< Поверх всех окон
        bool maximized = false;  ///< Максимизированное при создании
        int samples = 0;         ///< MSAA сэмплы (0 = отключено)

        // OpenGL контекст
        int openglMajor = 3;     ///< Основная версия OpenGL
        int openglMinor = 3;     ///< Младшая версия OpenGL
        bool openglCore = true;  ///< Core профиль OpenGL
    };

    /**
     * @brief Callback функции для событий
     */
    using ResizeCallback = std::function<void(int width, int height)>;
    using CloseCallback = std::function<void()>;
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using MouseMoveCallback = std::function<void(double x, double y)>;
    using ScrollCallback = std::function<void(double xOffset, double yOffset)>;
    using FocusCallback = std::function<void(bool focused)>;

  public:
    /**
     * @brief Конструктор окна
     * @param config Конфигурация окна
     */
    explicit Window(const Config& config);

    /**
     * @brief Деструктор
     */
    ~Window();

    // Запрещаем копирование
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Разрешаем перемещение
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    // ============================================================
    // Управление жизненным циклом
    // ============================================================

    /**
     * @brief Инициализация окна
     * @return true если успешно
     */
    bool initialize();

    /**
     * @brief Закрытие окна
     */
    void close();

    /**
     * @brief Проверка, должно ли окно закрыться
     */
    bool shouldClose() const;

    /**
     * @brief Обработка событий
     */
    void pollEvents();

    /**
     * @brief Обмен буферов (swap buffers)
     */
    void swapBuffers();

    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const { return window != nullptr; }

    // ============================================================
    // Размер и позиция
    // ============================================================

    /**
     * @brief Получить размер окна
     */
    Math::Vector2 getSize() const;

    /**
     * @brief Установить размер окна
     */
    void setSize(int width, int height);

    /**
     * @brief Получить позицию окна
     */
    Math::Vector2 getPosition() const;

    /**
     * @brief Установить позицию окна
     */
    void setPosition(int x, int y);

    /**
     * @brief Получить размер framebuffer
     */
    Math::Vector2 getFramebufferSize() const;

    /**
     * @brief Центрировать окно на экране
     */
    void center();

    // ============================================================
    // Режимы окна
    // ============================================================

    /**
     * @brief Получить текущий режим окна
     */
    WindowMode getMode() const { return currentMode; }

    /**
     * @brief Установить оконный режим
     */
    void setWindowed(int width, int height);

    /**
     * @brief Установить полноэкранный режим
     */
    void setFullscreen();

    /**
     * @brief Переключить между оконным и полноэкранным режимами
     */
    void toggleFullscreen();

    /**
     * @brief Минимизировать окно
     */
    void minimize();

    /**
     * @brief Максимизировать окно
     */
    void maximize();

    /**
     * @brief Восстановить окно
     */
    void restore();

    // ============================================================
    // Свойства окна
    // ============================================================

    /**
     * @brief Получить заголовок окна
     */
    const std::string& getTitle() const { return config.title; }

    /**
     * @brief Установить заголовок окна
     */
    void setTitle(const std::string& title);

    /**
     * @brief Установить иконку окна
     */
    void setIcon(const std::string& iconPath);

    /**
     * @brief Установить вертикальную синхронизацию
     */
    void setVSync(bool enabled);

    /**
     * @brief Проверить состояние VSync
     */
    bool isVSyncEnabled() const { return config.vSync; }

    /**
     * @brief Проверить фокус окна
     */
    bool isFocused() const;

    /**
     * @brief Проверить, минимизировано ли окно
     */
    bool isMinimized() const;

    /**
     * @brief Проверить, максимизировано ли окно
     */
    bool isMaximized() const;

    // ============================================================
    // Управление курсором
    // ============================================================

    /**
     * @brief Показать/скрыть курсор
     */
    void setCursorVisible(bool visible);

    /**
     * @brief Заблокировать курсор в окне
     */
    void setCursorLocked(bool locked);

    /**
     * @brief Установить позицию курсора
     */
    void setCursorPosition(double x, double y);

    /**
     * @brief Получить позицию курсора
     */
    Math::Vector2 getCursorPosition() const;

    // ============================================================
    // Callback функции
    // ============================================================

    /**
     * @brief Установить callback для изменения размера
     */
    void setResizeCallback(const ResizeCallback& callback) { resizeCallback = callback; }

    /**
     * @brief Установить callback для закрытия окна
     */
    void setCloseCallback(const CloseCallback& callback) { closeCallback = callback; }

    /**
     * @brief Установить callback для клавиатуры
     */
    void setKeyCallback(const KeyCallback& callback) { keyCallback = callback; }

    /**
     * @brief Установить callback для кнопок мыши
     */
    void setMouseButtonCallback(const MouseButtonCallback& callback) {
        mouseButtonCallback = callback;
    }

    /**
     * @brief Установить callback для движения мыши
     */
    void setMouseMoveCallback(const MouseMoveCallback& callback) { mouseMoveCallback = callback; }

    /**
     * @brief Установить callback для прокрутки
     */
    void setScrollCallback(const ScrollCallback& callback) { scrollCallback = callback; }

    /**
     * @brief Установить callback для фокуса
     */
    void setFocusCallback(const FocusCallback& callback) { focusCallback = callback; }

    // ============================================================
    // Утилиты
    // ============================================================

    /**
     * @brief Получить нативный указатель на окно (GLFWwindow*)
     */
    GLFWwindow* getNativeWindow() const { return window; }

    /**
     * @brief Получить конфигурацию окна
     */
    const Config& getConfig() const { return config; }

    /**
     * @brief Сделать контекст окна текущим
     */
    void makeContextCurrent();

    /**
     * @brief Сохранить скриншот окна
     */
    bool saveScreenshot(const std::string& filename) const;

    // ============================================================
    // Статические методы
    // ============================================================

    /**
     * @brief Инициализация системы окон (GLFW)
     */
    static bool initializeSystem();

    /**
     * @brief Завершение работы системы окон
     */
    static void terminateSystem();

    /**
     * @brief Получить список доступных мониторов
     */
    static std::vector<std::string> getAvailableMonitors();

    /**
     * @brief Получить размер основного монитора
     */
    static Math::Vector2 getPrimaryMonitorSize();

    /**
     * @brief Получить последнее созданное окно (для Vulkan surface)
     */
    static GLFWwindow* getLastCreatedWindow();

  private:
    Config config;           ///< Конфигурация окна
    GLFWwindow* window;      ///< Указатель на GLFW окно
    WindowMode currentMode;  ///< Текущий режим окна

    // Сохраненные параметры для переключения между режимами
    int windowedWidth, windowedHeight;
    int windowedPosX, windowedPosY;

    // Callback функции
    ResizeCallback resizeCallback;
    CloseCallback closeCallback;
    KeyCallback keyCallback;
    MouseButtonCallback mouseButtonCallback;
    MouseMoveCallback mouseMoveCallback;
    ScrollCallback scrollCallback;
    FocusCallback focusCallback;

    // Статические callback функции для GLFW
    static void glfwResizeCallback(GLFWwindow* window, int width, int height);
    static void glfwCloseCallback(GLFWwindow* window);
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwMouseMoveCallback(GLFWwindow* window, double x, double y);
    static void glfwScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void glfwFocusCallback(GLFWwindow* window, int focused);

    // Вспомогательные методы
    void setupCallbacks();
    void cleanup();
};

}  // namespace Core
}  // namespace SpectraForge
