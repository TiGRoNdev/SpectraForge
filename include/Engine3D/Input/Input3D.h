#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"

// Forward declaration for GLFW
struct GLFWwindow;

namespace Engine3D {
namespace Input {

/**
 * @brief Коды клавиш для 3D движка
 */
enum class Key3D {
    // Буквы
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,

    // Цифры
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,

    // Специальные клавиши
    Space = 32,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Delete = 261,

    // Стрелки
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,

    // Функциональные клавиши
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

    // Модификаторы
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,

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
 * @brief Состояние ввода
 */
struct InputState3D {
    // Клавиатура
    std::unordered_map<Key3D, bool> keyStates;
    std::unordered_map<Key3D, bool> keyJustPressed;
    std::unordered_map<Key3D, bool> keyJustReleased;

    // Мышь
    std::unordered_map<MouseButton3D, bool> mouseStates;
    std::unordered_map<MouseButton3D, bool> mouseJustPressed;
    std::unordered_map<MouseButton3D, bool> mouseJustReleased;

    Math::Vector3 mousePosition;
    Math::Vector3 mouseDelta;
    Math::Vector3 scrollDelta;

    void clear() {
        keyJustPressed.clear();
        keyJustReleased.clear();
        mouseJustPressed.clear();
        mouseJustReleased.clear();
        mouseDelta = Math::Vector3::zero();
        scrollDelta = Math::Vector3::zero();
    }
};

/**
 * @brief Действие ввода
 */
class InputAction3D {
  public:
    InputAction3D() : name(""), enabled(true) {}
    InputAction3D(const std::string& name);
    ~InputAction3D() = default;

    // Настройка действия
    void addKey(Key3D key);
    void addMouseButton(MouseButton3D button);
    void setOnPressed(std::function<void()> callback);
    void setOnReleased(std::function<void()> callback);
    void setOnHeld(std::function<void()> callback);

    // Проверка состояния
    bool isTriggered(const InputState3D& inputState) const;
    bool wasJustPressed(const InputState3D& inputState) const;
    bool wasJustReleased(const InputState3D& inputState) const;

    // Выполнение действий
    void executePressed();
    void executeReleased();
    void executeHeld();

    // Геттеры
    const std::string& getName() const { return name; }
    bool isEnabled() const { return enabled; }
    void setEnabled(bool isEnabled) { this->enabled = isEnabled; }

  private:
    std::string name;
    bool enabled;

    std::vector<Key3D> keys;
    std::vector<MouseButton3D> mouseButtons;

    std::function<void()> onPressed;
    std::function<void()> onReleased;
    std::function<void()> onHeld;
};

/**
 * @brief Менеджер ввода для 3D движка
 */
class InputManager3D {
  public:
    // Получение singleton экземпляра
    static InputManager3D& getInstance();

    // Инициализация и очистка
    bool initialize(GLFWwindow* windowPtr);
    void cleanup();
    void update();

    // Управление действиями
    void addAction(const InputAction3D& action);
    void removeAction(const std::string& name);
    InputAction3D* getAction(const std::string& name);

    // Проверка состояния клавиш
    bool isKeyPressed(Key3D key) const;
    bool isKeyJustPressed(Key3D key) const;
    bool isKeyJustReleased(Key3D key) const;

    // Проверка состояния мыши
    bool isMousePressed(MouseButton3D button) const;
    bool isMouseJustPressed(MouseButton3D button) const;
    bool isMouseJustReleased(MouseButton3D button) const;

    // Данные мыши
    Math::Vector3 getMousePosition() const;
    Math::Vector3 getMouseDelta() const;
    Math::Vector3 getScrollDelta() const;

    // Управление курсором
    void setCursorVisible(bool visible);
    void setCursorLocked(bool locked);
    void setCursorPosition(double x, double y);

    // Получение состояния ввода
    const InputState3D& getInputState() const { return inputState; }

  protected:
    // Singleton
    InputManager3D() = default;
    ~InputManager3D() = default;
    InputManager3D(const InputManager3D&) = delete;
    InputManager3D& operator=(const InputManager3D&) = delete;

    // GLFW callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Внутренние методы
    void processKeyInput(int key, int action);
    void processMouseButtonInput(int button, int action);
    void processMouseMovement(double xpos, double ypos);
    void processScroll(double xoffset, double yoffset);
    void processActions();

    Key3D glfwKeyToKey3D(int glfwKey);
    MouseButton3D glfwButtonToMouseButton3D(int glfwButton);

  private:
    GLFWwindow* window;
    bool initialized;

    InputState3D inputState;
    std::unordered_map<std::string, InputAction3D> actions;

    // Для отслеживания состояния предыдущего кадра
    InputState3D previousInputState;

    // Мышь
    bool firstMouse;
    double lastMouseX, lastMouseY;
    bool cursorLocked;
};

/**
 * @brief 3D контроллер для навигации
 */
class Controller3D {
  public:
    Controller3D();
    virtual ~Controller3D() = default;

    // Обновление
    void update(float deltaTime);
    virtual void handleInput(const InputState3D& inputState);

    // Движение
    void move(const Math::Vector3& direction);
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    // Повороты
    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    void rotate(float pitch, float yaw, float roll = 0.0f);
    void lookAt(const Math::Vector3& target);

    // Настройки движения
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    void setSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
    void setMouseLook(bool enabled) { mouseLookEnabled = enabled; }
    void setMovementEnabled(bool enabled) { movementEnabled = enabled; }
    void setRotationEnabled(bool enabled) { rotationEnabled = enabled; }

    // Геттеры
    const Math::Vector3& getPosition() const { return position; }
    const Math::Quaternion& getRotation() const { return rotation; }
    const Math::Vector3& getVelocity() const { return velocity; }
    Math::Matrix4 getTransformMatrix() const;

    float getMoveSpeed() const { return moveSpeed; }
    float getRotationSpeed() const { return rotationSpeed; }
    float getSensitivity() const { return mouseSensitivity; }

    // Направления
    Math::Vector3 getForward() const;
    Math::Vector3 getRight() const;
    Math::Vector3 getUp() const;

    // Настройка позиции
    void setPosition(const Math::Vector3& pos);
    void setRotation(const Math::Quaternion& rot);
    void setVelocity(const Math::Vector3& vel);

  protected:
    // Позиция и ориентация
    Math::Vector3 position;
    Math::Quaternion rotation;
    Math::Vector3 velocity;

    // Настройки
    float moveSpeed;
    float rotationSpeed;
    float mouseSensitivity;
    bool mouseLookEnabled;
    bool movementEnabled;
    bool rotationEnabled;

    // Состояние движения
    Math::Vector3 inputVector;
    Math::Vector3 rotationInput;

    void updateMovement(float deltaTime);
    void updateRotation(float deltaTime);
    void processMovementInput(const InputState3D& inputState);
    void processRotationInput(const InputState3D& inputState);
};

/**
 * @brief Первопёрсонный контроллер
 */
class FirstPersonController : public Controller3D {
  public:
    FirstPersonController();

    void handleInput(const InputState3D& inputState) override;

    // Специфичные настройки FPS
    void setPitchLimit(float minPitchValue, float maxPitchValue);
    void setJumpHeight(float height) { jumpHeight = height; }
    void setGravity(float gravityValue) { this->gravity = gravityValue; }
    void setGrounded(bool isGrounded) { this->grounded = isGrounded; }

    bool isGrounded() const { return grounded; }
    void jump();

  private:
    float minPitch, maxPitch;
    float currentPitch, currentYaw;
    float jumpHeight;
    float gravity;
    bool grounded;

    void updatePitchYaw();
};

/**
 * @brief Орбитальный контроллер (для просмотра объектов)
 */
class OrbitController : public Controller3D {
  public:
    OrbitController();

    void handleInput(const InputState3D& inputState) override;

    // Настройки орбиты
    void setTarget(const Math::Vector3& target);
    void setDistance(float distance);
    void setDistanceRange(float minDistance, float maxDistance);
    void setPitchRange(float minPitch, float maxPitch);
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    void setOrbitSpeed(float speed) { orbitSpeed = speed; }

    const Math::Vector3& getTarget() const { return target; }
    float getDistance() const { return distance; }

  private:
    Math::Vector3 target;
    float distance;
    float minDistance, maxDistance;
    float minPitch, maxPitch;
    float currentPitch, currentYaw;
    float zoomSpeed;
    float orbitSpeed;

    void updateOrbitPosition();
};

}  // namespace Input
}  // namespace Engine3D
