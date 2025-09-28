#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

// Forward declaration for GLFW
struct GLFWwindow;

namespace HyperEngine {
namespace Input {

/**
 * @brief Коды клавиш для 3D движка
 */
enum class Key3D {
    // Буквы
    A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71, H = 72, I = 73, J = 74,
    K = 75, L = 76, M = 77, N = 78, O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84,
    U = 85, V = 86, W = 87, X = 88, Y = 89, Z = 90,

    // Цифры
    Num0 = 48, Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
    Num5 = 53, Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,

    // Специальные клавиши
    Space = 32, Enter = 257, Tab = 258, Backspace = 259, Delete = 261,

    // Стрелки
    Right = 262, Left = 263, Down = 264, Up = 265,

    // Функциональные клавиши
    F1 = 290, F2 = 291, F3 = 292, F4 = 293, F5 = 294, F6 = 295,
    F7 = 296, F8 = 297, F9 = 298, F10 = 299, F11 = 300, F12 = 301,

    // Модификаторы
    LeftShift = 340, LeftControl = 341, LeftAlt = 342,
    RightShift = 344, RightControl = 345, RightAlt = 346,

    // Escape
    Escape = 256
};

/**
 * @brief Кнопки мыши
 */
enum class MouseButton3D {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7
};

/**
 * @brief Состояния клавиш/кнопок
 */
enum class KeyState3D {
    Released = 0,
    Pressed = 1,
    Held = 2
};

/**
 * @brief Система ввода для 3D движка
 */
class Input3D {
  public:
    /**
     * @brief Инициализация системы ввода
     * @param window GLFW окно
     * @return true если инициализация успешна
     */
    static bool initialize(GLFWwindow* window);

    /**
     * @brief Завершение работы системы ввода
     */
    static void shutdown();

    /**
     * @brief Обновление состояния ввода (вызывать каждый кадр)
     */
    static void update();

    // ============================================================
    // Клавиатура
    // ============================================================

    /**
     * @brief Проверка нажатия клавиши (один раз за нажатие)
     */
    static bool isKeyPressed(Key3D key);

    /**
     * @brief Проверка удержания клавиши
     */
    static bool isKeyHeld(Key3D key);

    /**
     * @brief Проверка отпускания клавиши (один раз за отпускание)
     */
    static bool isKeyReleased(Key3D key);

    /**
     * @brief Получение состояния клавиши
     */
    static KeyState3D getKeyState(Key3D key);

    // ============================================================
    // Мышь
    // ============================================================

    /**
     * @brief Проверка нажатия кнопки мыши (один раз за нажатие)
     */
    static bool isMouseButtonPressed(MouseButton3D button);

    /**
     * @brief Проверка удержания кнопки мыши
     */
    static bool isMouseButtonHeld(MouseButton3D button);

    /**
     * @brief Проверка отпускания кнопки мыши (один раз за отпускание)
     */
    static bool isMouseButtonReleased(MouseButton3D button);

    /**
     * @brief Получение позиции мыши
     */
    static Math::Vector2 getMousePosition();

    /**
     * @brief Получение смещения мыши с прошлого кадра
     */
    static Math::Vector2 getMouseDelta();

    /**
     * @brief Получение прокрутки колеса мыши
     */
    static Math::Vector2 getScrollDelta();

    /**
     * @brief Установка позиции мыши
     */
    static void setMousePosition(const Math::Vector2& position);

    /**
     * @brief Показать/скрыть курсор
     */
    static void setCursorVisible(bool visible);

    /**
     * @brief Заблокировать курсор в центре окна
     */
    static void setCursorLocked(bool locked);

    // ============================================================
    // Callback функции
    // ============================================================

    /**
     * @brief Установить callback для клавиатуры
     */
    static void setKeyCallback(std::function<void(Key3D, KeyState3D)> callback);

    /**
     * @brief Установить callback для мыши
     */
    static void setMouseButtonCallback(std::function<void(MouseButton3D, KeyState3D)> callback);

    /**
     * @brief Установить callback для движения мыши
     */
    static void setMouseMoveCallback(std::function<void(Math::Vector2)> callback);

    /**
     * @brief Установить callback для прокрутки
     */
    static void setScrollCallback(std::function<void(Math::Vector2)> callback);

    // ============================================================
    // Утилиты
    // ============================================================

    /**
     * @brief Получить строковое представление клавиши
     */
    static std::string getKeyName(Key3D key);

    /**
     * @brief Получить строковое представление кнопки мыши
     */
    static std::string getMouseButtonName(MouseButton3D button);

    /**
     * @brief Проверка инициализации
     */
    static bool isInitialized() { return initialized; }

  private:
    static GLFWwindow* window;
    static bool initialized;

    // Состояния клавиш и кнопок
    static std::unordered_map<int, KeyState3D> keyStates;
    static std::unordered_map<int, KeyState3D> mouseButtonStates;
    static std::unordered_map<int, KeyState3D> previousKeyStates;
    static std::unordered_map<int, KeyState3D> previousMouseButtonStates;

    // Позиция и движение мыши
    static Math::Vector2 mousePosition;
    static Math::Vector2 previousMousePosition;
    static Math::Vector2 mouseDelta;
    static Math::Vector2 scrollDelta;

    // Callback функции
    static std::function<void(Key3D, KeyState3D)> keyCallback;
    static std::function<void(MouseButton3D, KeyState3D)> mouseButtonCallback;
    static std::function<void(Math::Vector2)> mouseMoveCallback;
    static std::function<void(Math::Vector2)> scrollCallback;

    // GLFW callback функции
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Вспомогательные методы
    static KeyState3D actionToKeyState(int action);
    static void updateKeyStates();
    static void updateMouseStates();
};

}  // namespace Input
}  // namespace HyperEngine
