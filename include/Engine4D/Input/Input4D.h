#pragma once

#include "../Math/Vector4.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Engine4D {
namespace Input {

/**
 * @brief Коды клавиш для 4D навигации
 */
enum class Key4D {
    // Движение в 3D пространстве
    MoveForward = GLFW_KEY_W,
    MoveBackward = GLFW_KEY_S,
    MoveLeft = GLFW_KEY_A,
    MoveRight = GLFW_KEY_D,
    MoveUp = GLFW_KEY_SPACE,
    MoveDown = GLFW_KEY_LEFT_SHIFT,
    
    // Движение в 4D пространстве (W-измерение)
    MoveWPositive = GLFW_KEY_Q,
    MoveWNegative = GLFW_KEY_E,
    
    // Повороты в различных плоскостях
    RotateXY = GLFW_KEY_1,
    RotateXZ = GLFW_KEY_2,
    RotateXW = GLFW_KEY_3,
    RotateYZ = GLFW_KEY_4,
    RotateYW = GLFW_KEY_5,
    RotateZW = GLFW_KEY_6,
    
    // Специальные функции
    ToggleCrossSection = GLFW_KEY_TAB,
    ResetView = GLFW_KEY_R,
    ToggleWireframe = GLFW_KEY_F1,
    ToggleFullscreen = GLFW_KEY_F11,
    
    // Дополнительные клавиши
    SpeedBoost = GLFW_KEY_LEFT_CONTROL,
    SlowMotion = GLFW_KEY_LEFT_ALT,
    Pause = GLFW_KEY_P,
    Exit = GLFW_KEY_ESCAPE
};

/**
 * @brief Коды кнопок мыши
 */
enum class MouseButton4D {
    Left = GLFW_MOUSE_BUTTON_LEFT,
    Right = GLFW_MOUSE_BUTTON_RIGHT,
    Middle = GLFW_MOUSE_BUTTON_MIDDLE,
    Button4 = GLFW_MOUSE_BUTTON_4,
    Button5 = GLFW_MOUSE_BUTTON_5
};

/**
 * @brief Состояние ввода
 */
struct InputState4D {
    bool keyPressed[512];      // Состояние клавиш
    bool keyJustPressed[512];  // Клавиши, нажатые в этом кадре
    bool keyJustReleased[512]; // Клавиши, отпущенные в этом кадре
    
    bool mousePressed[8];      // Состояние кнопок мыши
    bool mouseJustPressed[8];  // Кнопки мыши, нажатые в этом кадре
    bool mouseJustReleased[8]; // Кнопки мыши, отпущенные в этом кадре
    
    double mouseX, mouseY;     // Позиция мыши
    double mouseDeltaX, mouseDeltaY; // Изменение позиции мыши
    double scrollX, scrollY;    // Скролл мыши
    
    double lastMouseX, lastMouseY; // Предыдущая позиция мыши
};

/**
 * @brief Действие ввода
 */
class InputAction4D {
public:
    std::string name;
    std::vector<Key4D> keys;
    std::vector<MouseButton4D> mouseButtons;
    std::function<void()> onPressed;
    std::function<void()> onReleased;
    std::function<void()> onHeld;
    
    InputAction4D(const std::string& name);
    void addKey(Key4D key);
    void addMouseButton(MouseButton4D button);
    void setOnPressed(std::function<void()> callback);
    void setOnReleased(std::function<void()> callback);
    void setOnHeld(std::function<void()> callback);
};

/**
 * @brief 4D контроллер для навигации
 */
class Controller4D {
public:
    Vector4 position;           // Позиция в 4D пространстве
    Vector4 rotation;           // Поворот (углы для 6 плоскостей)
    Vector4 velocity;           // Скорость движения
    Vector4 angularVelocity;    // Угловая скорость
    
    float moveSpeed;            // Скорость движения
    float rotationSpeed;        // Скорость поворота
    float wSpeed;               // Скорость движения в W-измерении
    float sensitivity;          // Чувствительность мыши
    
    bool enableMouseLook;       // Включен ли режим "мышь-взгляд"
    bool enableWMovement;       // Включено ли движение в W-измерении
    bool enableRotation;        // Включены ли повороты
    
    Controller4D();
    virtual ~Controller4D() = default;
    
    void update(float deltaTime);
    void handleInput(const InputState4D& inputState);
    
    // Движение
    void move(const Vector4& direction);
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void moveW(float amount);
    
    // Повороты
    void rotateXY(float angle);
    void rotateXZ(float angle);
    void rotateXW(float angle);
    void rotateYZ(float angle);
    void rotateYW(float angle);
    void rotateZW(float angle);
    
    // Настройки
    void setMoveSpeed(float speed);
    void setRotationSpeed(float speed);
    void setWSpeed(float speed);
    void setSensitivity(float sensitivity);
    void setMouseLook(bool enabled);
    void setWMovement(bool enabled);
    void setRotation(bool enabled);
    
    // Геттеры
    Vector4 getPosition() const { return position; }
    Vector4 getRotation() const { return rotation; }
    Vector4 getVelocity() const { return velocity; }
    Matrix4 getTransformMatrix() const;
    
    // Дополнительные методы для совместимости
    Vector4 forward() const;
    Vector4 right() const;
    Vector4 up() const;
};

/**
 * @brief Система ввода для 4D движка
 */
class InputManager4D {
public:
    static InputManager4D& getInstance();
    
    bool initialize(GLFWwindow* window);
    void cleanup();
    void update();
    
    // Управление действиями
    void addAction(const InputAction4D& action);
    void removeAction(const std::string& name);
    InputAction4D* getAction(const std::string& name);
    
    // Проверка состояния
    bool isKeyPressed(Key4D key) const;
    bool isKeyJustPressed(Key4D key) const;
    bool isKeyJustReleased(Key4D key) const;
    bool isMousePressed(MouseButton4D button) const;
    bool isMouseJustPressed(MouseButton4D button) const;
    bool isMouseJustReleased(MouseButton4D button) const;
    
    // Получение данных мыши
    Vector4 getMousePosition() const;
    Vector4 getMouseDelta() const;
    Vector4 getScrollDelta() const;
    
    // Управление курсором
    void setCursorVisible(bool visible);
    void setCursorLocked(bool locked);
    void setCursorPosition(double x, double y);
    
    // Геттеры
    const InputState4D& getInputState() const { return inputState; }
    GLFWwindow* getWindow() const { return window; }
    
private:
    InputManager4D() = default;
    ~InputManager4D() = default;
    InputManager4D(const InputManager4D&) = delete;
    InputManager4D& operator=(const InputManager4D&) = delete;
    
    GLFWwindow* window;
    InputState4D inputState;
    std::unordered_map<std::string, InputAction4D> actions;
    bool initialized;
    
    // Callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    void processActions();
};

/**
 * @brief Предустановленные действия для 4D навигации
 */
class DefaultActions4D {
public:
    static void setupDefaultActions(InputManager4D& inputManager);
    
    // Действия движения
    static InputAction4D createMoveForwardAction();
    static InputAction4D createMoveBackwardAction();
    static InputAction4D createMoveLeftAction();
    static InputAction4D createMoveRightAction();
    static InputAction4D createMoveUpAction();
    static InputAction4D createMoveDownAction();
    static InputAction4D createMoveWPositiveAction();
    static InputAction4D createMoveWNegativeAction();
    
    // Действия поворота
    static InputAction4D createRotateXYAction();
    static InputAction4D createRotateXZAction();
    static InputAction4D createRotateXWAction();
    static InputAction4D createRotateYZAction();
    static InputAction4D createRotateYWAction();
    static InputAction4D createRotateZWAction();
    
    // Специальные действия
    static InputAction4D createToggleCrossSectionAction();
    static InputAction4D createResetViewAction();
    static InputAction4D createToggleWireframeAction();
    static InputAction4D createToggleFullscreenAction();
    static InputAction4D createPauseAction();
    static InputAction4D createExitAction();
};

} // namespace Input
} // namespace Engine4D
