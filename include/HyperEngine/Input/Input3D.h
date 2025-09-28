#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
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
 * @brief Состояния клавиш/кнопок
 */
enum class KeyState3D { Released = 0, Pressed = 1, Held = 2 };

/**
 * @brief Состояние ввода
 */
struct InputState3D {
    std::unordered_map<Key3D, bool> keyStates;
    std::unordered_map<Key3D, bool> keyJustPressed;
    std::unordered_map<Key3D, bool> keyJustReleased;

    std::unordered_map<MouseButton3D, bool> mouseStates;
    std::unordered_map<MouseButton3D, bool> mouseJustPressed;
    std::unordered_map<MouseButton3D, bool> mouseJustReleased;

    Math::Vector3 mousePosition;
    Math::Vector3 mouseDelta;
    Math::Vector3 scrollDelta;
};

/**
 * @brief Действие ввода
 */
class InputAction3D {
  public:
    InputAction3D() = default;  // Конструктор по умолчанию
    explicit InputAction3D(const std::string& name);

    const std::string& getName() const { return name; }

    void addKey(Key3D key);
    void addMouseButton(MouseButton3D button);

    void setOnPressed(std::function<void()> callback);
    void setOnReleased(std::function<void()> callback);
    void setOnHeld(std::function<void()> callback);

    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }

    bool isTriggered(const InputState3D& inputState) const;
    bool wasJustPressed(const InputState3D& inputState) const;
    bool wasJustReleased(const InputState3D& inputState) const;

    void executePressed();
    void executeReleased();
    void executeHeld();

  private:
    std::string name;
    std::vector<Key3D> keys;
    std::vector<MouseButton3D> mouseButtons;
    std::function<void()> onPressed;
    std::function<void()> onReleased;
    std::function<void()> onHeld;
    bool enabled = true;
};

/**
 * @brief Менеджер ввода (Singleton)
 */
class InputManager3D {
  public:
    static InputManager3D& getInstance();

    bool initialize(GLFWwindow* window);
    void cleanup();
    void update();

    // Управление действиями
    void addAction(const InputAction3D& action);
    void removeAction(const std::string& name);
    InputAction3D* getAction(const std::string& name);

    // Прямой доступ к состоянию
    bool isKeyPressed(Key3D key) const;
    bool isKeyJustPressed(Key3D key) const;
    bool isKeyJustReleased(Key3D key) const;

    bool isMousePressed(MouseButton3D button) const;
    bool isMouseJustPressed(MouseButton3D button) const;
    bool isMouseJustReleased(MouseButton3D button) const;

    Math::Vector3 getMousePosition() const;
    Math::Vector3 getMouseDelta() const;
    Math::Vector3 getScrollDelta() const;

    // Получение состояния ввода
    const InputState3D& getInputState() const { return inputState; }

    // Управление курсором
    void setCursorVisible(bool visible);
    void setCursorLocked(bool locked);
    void setCursorPosition(double x, double y);

    bool isInitialized() const { return initialized; }

  private:
    InputManager3D() = default;
    ~InputManager3D() = default;
    InputManager3D(const InputManager3D&) = delete;
    InputManager3D& operator=(const InputManager3D&) = delete;

    GLFWwindow* window = nullptr;
    bool initialized = false;
    bool firstMouse = true;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool cursorLocked = false;

    std::unordered_map<std::string, InputAction3D> actions;
    InputState3D inputState;
    InputState3D previousInputState;

    // GLFW callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Обработка ввода
    void processKeyInput(int key, int action);
    void processMouseButtonInput(int button, int action);
    void processMouseMovement(double xpos, double ypos);
    void processScroll(double xoffset, double yoffset);
    void processActions();

    // Конвертация GLFW -> Engine
    Key3D glfwKeyToKey3D(int glfwKey);
    MouseButton3D glfwButtonToMouseButton3D(int glfwButton);
};

/**
 * @brief Базовый контроллер для 3D навигации
 */
class Controller3D {
  public:
    Controller3D();
    virtual ~Controller3D() = default;

    virtual void update(float deltaTime);
    virtual void handleInput(const InputState3D& inputState);

    // Движение
    void move(const Math::Vector3& direction);
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    // Поворот
    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    void rotate(float pitch, float yaw, float roll);
    void lookAt(const Math::Vector3& target);

    // Геттеры/сеттеры
    const Math::Vector3& getPosition() const { return position; }
    void setPosition(const Math::Vector3& pos);

    const Math::Quaternion& getRotation() const { return rotation; }
    void setRotation(const Math::Quaternion& rot);

    const Math::Vector3& getVelocity() const { return velocity; }
    void setVelocity(const Math::Vector3& vel);

    Math::Matrix4 getTransformMatrix() const;

    Math::Vector3 getForward() const;
    Math::Vector3 getRight() const;
    Math::Vector3 getUp() const;

    // Настройки
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    float getMoveSpeed() const { return moveSpeed; }

    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationSpeed() const { return rotationSpeed; }

    void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
    float getMouseSensitivity() const { return mouseSensitivity; }

    void setMouseLookEnabled(bool enabled) { mouseLookEnabled = enabled; }
    bool isMouseLookEnabled() const { return mouseLookEnabled; }

    void setMovementEnabled(bool enabled) { movementEnabled = enabled; }
    bool isMovementEnabled() const { return movementEnabled; }

    void setRotationEnabled(bool enabled) { rotationEnabled = enabled; }
    bool isRotationEnabled() const { return rotationEnabled; }

  protected:
    Math::Vector3 position;
    Math::Quaternion rotation;
    Math::Vector3 velocity;

    float moveSpeed;
    float rotationSpeed;
    float mouseSensitivity;

    bool mouseLookEnabled;
    bool movementEnabled;
    bool rotationEnabled;

    Math::Vector3 inputVector;
    Math::Vector3 rotationInput;

    virtual void updateMovement(float deltaTime);
    virtual void updateRotation(float deltaTime);
    virtual void processMovementInput(const InputState3D& inputState);
    virtual void processRotationInput(const InputState3D& inputState);
};

/**
 * @brief Контроллер от первого лица
 */
class FirstPersonController : public Controller3D {
  public:
    FirstPersonController();

    void handleInput(const InputState3D& inputState) override;

    void setPitchLimit(float minPitch, float maxPitch);
    void jump();

    void setJumpHeight(float height) { jumpHeight = height; }
    float getJumpHeight() const { return jumpHeight; }

    void setGravity(float grav) { gravity = grav; }
    float getGravity() const { return gravity; }

    bool isGrounded() const { return grounded; }
    void setGrounded(bool isGrounded) { grounded = isGrounded; }

  private:
    float minPitch;
    float maxPitch;
    float currentPitch;
    float currentYaw;
    float jumpHeight;
    float gravity;
    bool grounded;

    void updatePitchYaw();
};

/**
 * @brief Орбитальный контроллер камеры
 */
class OrbitController : public Controller3D {
  public:
    OrbitController();

    void handleInput(const InputState3D& inputState) override;

    void setTarget(const Math::Vector3& target);
    const Math::Vector3& getTarget() const { return target; }

    void setDistance(float distance);
    float getDistance() const { return distance; }

    void setDistanceRange(float minDist, float maxDist);
    void setPitchRange(float minPitch, float maxPitch);

    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    float getZoomSpeed() const { return zoomSpeed; }

    void setOrbitSpeed(float speed) { orbitSpeed = speed; }
    float getOrbitSpeed() const { return orbitSpeed; }

  private:
    Math::Vector3 target;
    float distance;
    float minDistance;
    float maxDistance;
    float minPitch;
    float maxPitch;
    float currentPitch;
    float currentYaw;
    float zoomSpeed;
    float orbitSpeed;

    void updateOrbitPosition();
};

}  // namespace Input
}  // namespace HyperEngine
