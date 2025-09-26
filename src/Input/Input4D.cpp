#include "Engine4D/Input/Input4D.h"
#include <algorithm>
#include <iostream>

namespace Engine4D {
namespace Input {

// InputAction4D
InputAction4D::InputAction4D(const std::string& name) : name(name) {}

void InputAction4D::addKey(Key4D key) {
    keys.push_back(key);
}

void InputAction4D::addMouseButton(MouseButton4D button) {
    mouseButtons.push_back(button);
}

void InputAction4D::setOnPressed(std::function<void()> callback) {
    onPressed = callback;
}

void InputAction4D::setOnReleased(std::function<void()> callback) {
    onReleased = callback;
}

void InputAction4D::setOnHeld(std::function<void()> callback) {
    onHeld = callback;
}

// Controller4D
Controller4D::Controller4D() 
    : position(Vector4::zero())
    , rotation(Vector4::zero())
    , velocity(Vector4::zero())
    , angularVelocity(Vector4::zero())
    , moveSpeed(5.0f)
    , rotationSpeed(2.0f)
    , wSpeed(3.0f)
    , sensitivity(0.1f)
    , enableMouseLook(true)
    , enableWMovement(true)
    , enableRotation(true) {}

void Controller4D::update(float deltaTime) {
    // Обновляем позицию
    position += velocity * deltaTime;
    
    // Обновляем поворот
    rotation += angularVelocity * deltaTime;
    
    // Применяем затухание скорости
    velocity *= 0.9f;
    angularVelocity *= 0.9f;
}

void Controller4D::handleInput(const InputState4D& inputState) {
    // Движение в 3D пространстве
    Vector4 moveDirection = Vector4::zero();
    
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveForward)]) {
        moveDirection += Vector4::unitZ();
    }
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveBackward)]) {
        moveDirection -= Vector4::unitZ();
    }
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveLeft)]) {
        moveDirection -= Vector4::unitX();
    }
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveRight)]) {
        moveDirection += Vector4::unitX();
    }
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveUp)]) {
        moveDirection += Vector4::unitY();
    }
    if (inputState.keyPressed[static_cast<int>(Key4D::MoveDown)]) {
        moveDirection -= Vector4::unitY();
    }
    
    // Движение в W-измерении
    if (enableWMovement) {
        if (inputState.keyPressed[static_cast<int>(Key4D::MoveWPositive)]) {
            moveDirection += Vector4::unitW();
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::MoveWNegative)]) {
            moveDirection -= Vector4::unitW();
        }
    }
    
    // Применяем движение
    if (moveDirection.magnitude() > 0.0f) {
        moveDirection.normalize();
        velocity += moveDirection * moveSpeed;
    }
    
    // Повороты мышью
    if (enableMouseLook && enableRotation) {
        Vector4 mouseDelta(inputState.mouseDeltaX, inputState.mouseDeltaY, 0.0f, 0.0f);
        mouseDelta *= sensitivity;
        
        // Поворот по X (вверх/вниз)
        rotation.x -= mouseDelta.y;
        rotation.x = std::clamp(rotation.x, -M_PI/2, M_PI/2);
        
        // Поворот по Y (влево/вправо)
        rotation.y += mouseDelta.x;
    }
    
    // Повороты клавишами
    if (enableRotation) {
        float rotationAmount = rotationSpeed * 0.016f; // Примерно 60 FPS
        
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateXY)]) {
            rotateXY(rotationAmount);
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateXZ)]) {
            rotateXZ(rotationAmount);
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateXW)]) {
            rotateXW(rotationAmount);
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateYZ)]) {
            rotateYZ(rotationAmount);
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateYW)]) {
            rotateYW(rotationAmount);
        }
        if (inputState.keyPressed[static_cast<int>(Key4D::RotateZW)]) {
            rotateZW(rotationAmount);
        }
    }
}

void Controller4D::move(const Vector4& direction) {
    velocity += direction * moveSpeed;
}

void Controller4D::moveForward(float amount) {
    move(Vector4::unitZ() * amount);
}

void Controller4D::moveRight(float amount) {
    move(Vector4::unitX() * amount);
}

void Controller4D::moveUp(float amount) {
    move(Vector4::unitY() * amount);
}

void Controller4D::moveW(float amount) {
    if (enableWMovement) {
        move(Vector4::unitW() * amount);
    }
}

void Controller4D::rotateXY(float angle) {
    angularVelocity.x += angle;
    angularVelocity.y += angle;
}

void Controller4D::rotateXZ(float angle) {
    angularVelocity.x += angle;
    angularVelocity.z += angle;
}

void Controller4D::rotateXW(float angle) {
    angularVelocity.x += angle;
    angularVelocity.w += angle;
}

void Controller4D::rotateYZ(float angle) {
    angularVelocity.y += angle;
    angularVelocity.z += angle;
}

void Controller4D::rotateYW(float angle) {
    angularVelocity.y += angle;
    angularVelocity.w += angle;
}

void Controller4D::rotateZW(float angle) {
    angularVelocity.z += angle;
    angularVelocity.w += angle;
}

void Controller4D::setMoveSpeed(float speed) {
    moveSpeed = speed;
}

void Controller4D::setRotationSpeed(float speed) {
    rotationSpeed = speed;
}

void Controller4D::setWSpeed(float speed) {
    wSpeed = speed;
}

void Controller4D::setSensitivity(float sensitivity) {
    this->sensitivity = sensitivity;
}

void Controller4D::setMouseLook(bool enabled) {
    enableMouseLook = enabled;
}

void Controller4D::setWMovement(bool enabled) {
    enableWMovement = enabled;
}

void Controller4D::setRotation(bool enabled) {
    enableRotation = enabled;
}

Matrix4 Controller4D::getTransformMatrix() const {
    Matrix4 translationMatrix = Matrix4::translation(position);
    
    // Создаем матрицы поворота для каждой плоскости
    Matrix4 rotationXY = Matrix4::rotationXY(rotation.x + rotation.y);
    Matrix4 rotationXZ = Matrix4::rotationXZ(rotation.x + rotation.z);
    Matrix4 rotationXW = Matrix4::rotationXW(rotation.x + rotation.w);
    Matrix4 rotationYZ = Matrix4::rotationYZ(rotation.y + rotation.z);
    Matrix4 rotationYW = Matrix4::rotationYW(rotation.y + rotation.w);
    Matrix4 rotationZW = Matrix4::rotationZW(rotation.z + rotation.w);
    
    // Комбинируем все повороты
    Matrix4 combinedRotation = rotationXY * rotationXZ * rotationXW * 
                              rotationYZ * rotationYW * rotationZW;
    
    return translationMatrix * combinedRotation;
}

// InputManager4D
InputManager4D& InputManager4D::getInstance() {
    static InputManager4D instance;
    return instance;
}

bool InputManager4D::initialize(GLFWwindow* window) {
    this->window = window;
    initialized = false;
    
    // Инициализируем состояние ввода
    std::fill(std::begin(inputState.keyPressed), std::end(inputState.keyPressed), false);
    std::fill(std::begin(inputState.keyJustPressed), std::end(inputState.keyJustPressed), false);
    std::fill(std::begin(inputState.keyJustReleased), std::end(inputState.keyJustReleased), false);
    std::fill(std::begin(inputState.mousePressed), std::end(inputState.mousePressed), false);
    std::fill(std::begin(inputState.mouseJustPressed), std::end(inputState.mouseJustPressed), false);
    std::fill(std::begin(inputState.mouseJustReleased), std::end(inputState.mouseJustReleased), false);
    
    inputState.mouseX = inputState.mouseY = 0.0;
    inputState.mouseDeltaX = inputState.mouseDeltaY = 0.0;
    inputState.scrollX = inputState.scrollY = 0.0;
    inputState.lastMouseX = inputState.lastMouseY = 0.0;
    
    // Устанавливаем callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    initialized = true;
    return true;
}

void InputManager4D::cleanup() {
    actions.clear();
    initialized = false;
}

void InputManager4D::update() {
    if (!initialized) return;
    
    // Сбрасываем флаги "только что нажато/отпущено"
    std::fill(std::begin(inputState.keyJustPressed), std::end(inputState.keyJustPressed), false);
    std::fill(std::begin(inputState.keyJustReleased), std::end(inputState.keyJustReleased), false);
    std::fill(std::begin(inputState.mouseJustPressed), std::end(inputState.mouseJustPressed), false);
    std::fill(std::begin(inputState.mouseJustReleased), std::end(inputState.mouseJustReleased), false);
    
    // Сбрасываем дельту мыши
    inputState.mouseDeltaX = 0.0;
    inputState.mouseDeltaY = 0.0;
    inputState.scrollX = 0.0;
    inputState.scrollY = 0.0;
    
    // Обрабатываем действия
    processActions();
}

void InputManager4D::addAction(const InputAction4D& action) {
    actions[action.name] = action;
}

void InputManager4D::removeAction(const std::string& name) {
    actions.erase(name);
}

InputAction4D* InputManager4D::getAction(const std::string& name) {
    auto it = actions.find(name);
    return (it != actions.end()) ? &it->second : nullptr;
}

bool InputManager4D::isKeyPressed(Key4D key) const {
    return inputState.keyPressed[static_cast<int>(key)];
}

bool InputManager4D::isKeyJustPressed(Key4D key) const {
    return inputState.keyJustPressed[static_cast<int>(key)];
}

bool InputManager4D::isKeyJustReleased(Key4D key) const {
    return inputState.keyJustReleased[static_cast<int>(key)];
}

bool InputManager4D::isMousePressed(MouseButton4D button) const {
    return inputState.mousePressed[static_cast<int>(button)];
}

bool InputManager4D::isMouseJustPressed(MouseButton4D button) const {
    return inputState.mouseJustPressed[static_cast<int>(button)];
}

bool InputManager4D::isMouseJustReleased(MouseButton4D button) const {
    return inputState.mouseJustReleased[static_cast<int>(button)];
}

Vector4 InputManager4D::getMousePosition() const {
    return Vector4(inputState.mouseX, inputState.mouseY, 0.0f, 0.0f);
}

Vector4 InputManager4D::getMouseDelta() const {
    return Vector4(inputState.mouseDeltaX, inputState.mouseDeltaY, 0.0f, 0.0f);
}

Vector4 InputManager4D::getScrollDelta() const {
    return Vector4(inputState.scrollX, inputState.scrollY, 0.0f, 0.0f);
}

void InputManager4D::setCursorVisible(bool visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void InputManager4D::setCursorLocked(bool locked) {
    glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void InputManager4D::setCursorPosition(double x, double y) {
    glfwSetCursorPos(window, x, y);
    inputState.mouseX = x;
    inputState.mouseY = y;
    inputState.lastMouseX = x;
    inputState.lastMouseY = y;
}

void InputManager4D::processActions() {
    for (auto& pair : actions) {
        InputAction4D& action = pair.second;
        
        bool isPressed = false;
        
        // Проверяем клавиши
        for (Key4D key : action.keys) {
            if (inputState.keyPressed[static_cast<int>(key)]) {
                isPressed = true;
                break;
            }
        }
        
        // Проверяем кнопки мыши
        for (MouseButton4D button : action.mouseButtons) {
            if (inputState.mousePressed[static_cast<int>(button)]) {
                isPressed = true;
                break;
            }
        }
        
        // Выполняем действия
        if (isPressed) {
            if (action.onHeld) {
                action.onHeld();
            }
        }
    }
}

// Callbacks
void InputManager4D::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager4D& manager = getInstance();
    
    if (key >= 0 && key < 512) {
        if (action == GLFW_PRESS) {
            manager.inputState.keyPressed[key] = true;
            manager.inputState.keyJustPressed[key] = true;
        } else if (action == GLFW_RELEASE) {
            manager.inputState.keyPressed[key] = false;
            manager.inputState.keyJustReleased[key] = true;
        }
    }
}

void InputManager4D::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    InputManager4D& manager = getInstance();
    
    if (button >= 0 && button < 8) {
        if (action == GLFW_PRESS) {
            manager.inputState.mousePressed[button] = true;
            manager.inputState.mouseJustPressed[button] = true;
        } else if (action == GLFW_RELEASE) {
            manager.inputState.mousePressed[button] = false;
            manager.inputState.mouseJustReleased[button] = true;
        }
    }
}

void InputManager4D::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    InputManager4D& manager = getInstance();
    
    manager.inputState.mouseDeltaX = xpos - manager.inputState.lastMouseX;
    manager.inputState.mouseDeltaY = ypos - manager.inputState.lastMouseY;
    
    manager.inputState.mouseX = xpos;
    manager.inputState.mouseY = ypos;
    manager.inputState.lastMouseX = xpos;
    manager.inputState.lastMouseY = ypos;
}

void InputManager4D::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager4D& manager = getInstance();
    
    manager.inputState.scrollX = xoffset;
    manager.inputState.scrollY = yoffset;
}

// DefaultActions4D
void DefaultActions4D::setupDefaultActions(InputManager4D& inputManager) {
    inputManager.addAction(createMoveForwardAction());
    inputManager.addAction(createMoveBackwardAction());
    inputManager.addAction(createMoveLeftAction());
    inputManager.addAction(createMoveRightAction());
    inputManager.addAction(createMoveUpAction());
    inputManager.addAction(createMoveDownAction());
    inputManager.addAction(createMoveWPositiveAction());
    inputManager.addAction(createMoveWNegativeAction());
    
    inputManager.addAction(createRotateXYAction());
    inputManager.addAction(createRotateXZAction());
    inputManager.addAction(createRotateXWAction());
    inputManager.addAction(createRotateYZAction());
    inputManager.addAction(createRotateYWAction());
    inputManager.addAction(createRotateZWAction());
    
    inputManager.addAction(createToggleCrossSectionAction());
    inputManager.addAction(createResetViewAction());
    inputManager.addAction(createToggleWireframeAction());
    inputManager.addAction(createToggleFullscreenAction());
    inputManager.addAction(createPauseAction());
    inputManager.addAction(createExitAction());
}

InputAction4D DefaultActions4D::createMoveForwardAction() {
    InputAction4D action("MoveForward");
    action.addKey(Key4D::MoveForward);
    return action;
}

InputAction4D DefaultActions4D::createMoveBackwardAction() {
    InputAction4D action("MoveBackward");
    action.addKey(Key4D::MoveBackward);
    return action;
}

InputAction4D DefaultActions4D::createMoveLeftAction() {
    InputAction4D action("MoveLeft");
    action.addKey(Key4D::MoveLeft);
    return action;
}

InputAction4D DefaultActions4D::createMoveRightAction() {
    InputAction4D action("MoveRight");
    action.addKey(Key4D::MoveRight);
    return action;
}

InputAction4D DefaultActions4D::createMoveUpAction() {
    InputAction4D action("MoveUp");
    action.addKey(Key4D::MoveUp);
    return action;
}

InputAction4D DefaultActions4D::createMoveDownAction() {
    InputAction4D action("MoveDown");
    action.addKey(Key4D::MoveDown);
    return action;
}

InputAction4D DefaultActions4D::createMoveWPositiveAction() {
    InputAction4D action("MoveWPositive");
    action.addKey(Key4D::MoveWPositive);
    return action;
}

InputAction4D DefaultActions4D::createMoveWNegativeAction() {
    InputAction4D action("MoveWNegative");
    action.addKey(Key4D::MoveWNegative);
    return action;
}

InputAction4D DefaultActions4D::createRotateXYAction() {
    InputAction4D action("RotateXY");
    action.addKey(Key4D::RotateXY);
    return action;
}

InputAction4D DefaultActions4D::createRotateXZAction() {
    InputAction4D action("RotateXZ");
    action.addKey(Key4D::RotateXZ);
    return action;
}

InputAction4D DefaultActions4D::createRotateXWAction() {
    InputAction4D action("RotateXW");
    action.addKey(Key4D::RotateXW);
    return action;
}

InputAction4D DefaultActions4D::createRotateYZAction() {
    InputAction4D action("RotateYZ");
    action.addKey(Key4D::RotateYZ);
    return action;
}

InputAction4D DefaultActions4D::createRotateYWAction() {
    InputAction4D action("RotateYW");
    action.addKey(Key4D::RotateYW);
    return action;
}

InputAction4D DefaultActions4D::createRotateZWAction() {
    InputAction4D action("RotateZW");
    action.addKey(Key4D::RotateZW);
    return action;
}

InputAction4D DefaultActions4D::createToggleCrossSectionAction() {
    InputAction4D action("ToggleCrossSection");
    action.addKey(Key4D::ToggleCrossSection);
    return action;
}

InputAction4D DefaultActions4D::createResetViewAction() {
    InputAction4D action("ResetView");
    action.addKey(Key4D::ResetView);
    return action;
}

InputAction4D DefaultActions4D::createToggleWireframeAction() {
    InputAction4D action("ToggleWireframe");
    action.addKey(Key4D::ToggleWireframe);
    return action;
}

InputAction4D DefaultActions4D::createToggleFullscreenAction() {
    InputAction4D action("ToggleFullscreen");
    action.addKey(Key4D::ToggleFullscreen);
    return action;
}

InputAction4D DefaultActions4D::createPauseAction() {
    InputAction4D action("Pause");
    action.addKey(Key4D::Pause);
    return action;
}

InputAction4D DefaultActions4D::createExitAction() {
    InputAction4D action("Exit");
    action.addKey(Key4D::Exit);
    return action;
}

} // namespace Input
} // namespace Engine4D
