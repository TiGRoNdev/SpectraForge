#include "HyperEngine/Input/Input3D.h"
#include <cmath>
#include <iostream>
#include "HyperEngine/Math/MathConstants.h"

// GLFW headers
#ifdef _WIN32
#include <GLFW/glfw3.h>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#else
#include <GLFW/glfw3.h>
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#endif

using namespace HyperEngine::Math;
using namespace HyperEngine::Input;
using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Input {

// InputAction3D implementation
InputAction3D::InputAction3D(const std::string& name) : name(name), enabled(true) {}

void InputAction3D::addKey(Key3D key) {
    keys.push_back(key);
}

void InputAction3D::addMouseButton(MouseButton3D button) {
    mouseButtons.push_back(button);
}

void InputAction3D::setOnPressed(std::function<void()> callback) {
    onPressed = callback;
}

void InputAction3D::setOnReleased(std::function<void()> callback) {
    onReleased = callback;
}

void InputAction3D::setOnHeld(std::function<void()> callback) {
    onHeld = callback;
}

bool InputAction3D::isTriggered(const InputState3D& inputState) const {
    if (!enabled)
        return false;

    // Проверяем клавиши
    for (Key3D key : keys) {
        auto it = inputState.keyStates.find(key);
        if (it != inputState.keyStates.end() && it->second) {
            return true;
        }
    }

    // Проверяем кнопки мыши
    for (MouseButton3D button : mouseButtons) {
        auto it = inputState.mouseStates.find(button);
        if (it != inputState.mouseStates.end() && it->second) {
            return true;
        }
    }

    return false;
}

bool InputAction3D::wasJustPressed(const InputState3D& inputState) const {
    if (!enabled)
        return false;

    // Проверяем клавиши
    for (Key3D key : keys) {
        auto it = inputState.keyJustPressed.find(key);
        if (it != inputState.keyJustPressed.end() && it->second) {
            return true;
        }
    }

    // Проверяем кнопки мыши
    for (MouseButton3D button : mouseButtons) {
        auto it = inputState.mouseJustPressed.find(button);
        if (it != inputState.mouseJustPressed.end() && it->second) {
            return true;
        }
    }

    return false;
}

bool InputAction3D::wasJustReleased(const InputState3D& inputState) const {
    if (!enabled)
        return false;

    // Проверяем клавиши
    for (Key3D key : keys) {
        auto it = inputState.keyJustReleased.find(key);
        if (it != inputState.keyJustReleased.end() && it->second) {
            return true;
        }
    }

    // Проверяем кнопки мыши
    for (MouseButton3D button : mouseButtons) {
        auto it = inputState.mouseJustReleased.find(button);
        if (it != inputState.mouseJustReleased.end() && it->second) {
            return true;
        }
    }

    return false;
}

void InputAction3D::executePressed() {
    if (onPressed && enabled) {
        onPressed();
    }
}

void InputAction3D::executeReleased() {
    if (onReleased && enabled) {
        onReleased();
    }
}

void InputAction3D::executeHeld() {
    if (onHeld && enabled) {
        onHeld();
    }
}

// InputManager3D implementation
InputManager3D& InputManager3D::getInstance() {
    static InputManager3D instance;
    return instance;
}

bool InputManager3D::initialize(GLFWwindow* windowPtr) {
    if (initialized) {
        SAFE_PRINT_LINE("InputManager3D already initialized");
        return true;
    }

    this->window = windowPtr;
    firstMouse = true;
    lastMouseX = 0.0;
    lastMouseY = 0.0;
    cursorLocked = false;

    // Устанавливаем пользовательский указатель для callbacks
    glfwSetWindowUserPointer(window, this);

    // Устанавливаем GLFW callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetScrollCallback(window, scrollCallback);

    initialized = true;
    SAFE_PRINT_LINE("InputManager3D initialized successfully");
    return true;
}

void InputManager3D::cleanup() {
    if (!initialized) {
        return;
    }

    // Сброс callbacks
    if (window) {
        glfwSetKeyCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
        glfwSetWindowUserPointer(window, nullptr);
    }

    actions.clear();
    inputState = InputState3D();
    previousInputState = InputState3D();

    initialized = false;
    SAFE_PRINT_LINE("InputManager3D cleaned up");
}

void InputManager3D::update() {
    if (!initialized) {
        return;
    }

    // Сохраняем предыдущее состояние
    previousInputState = inputState;

    // Очищаем события текущего кадра
    inputState.keyJustPressed.clear();
    inputState.keyJustReleased.clear();
    inputState.mouseJustPressed.clear();
    inputState.mouseJustReleased.clear();
    inputState.mouseDelta = Vector3::zero();
    inputState.scrollDelta = Vector3::zero();

    // Обрабатываем GLFW события
    glfwPollEvents();

    // Обрабатываем действия
    processActions();
}

void InputManager3D::addAction(const InputAction3D& action) {
    actions[action.getName()] = action;
    SAFE_PRINT_LINE("Added input action: " + SAFE_TO_STRING(action.getName()));
}

void InputManager3D::removeAction(const std::string& name) {
    actions.erase(name);
    SAFE_PRINT_LINE("Removed input action: " + SAFE_TO_STRING(name));
}

InputAction3D* InputManager3D::getAction(const std::string& name) {
    auto it = actions.find(name);
    return (it != actions.end()) ? &it->second : nullptr;
}

bool InputManager3D::isKeyPressed(Key3D key) const {
    auto it = inputState.keyStates.find(key);
    return (it != inputState.keyStates.end()) ? it->second : false;
}

bool InputManager3D::isKeyJustPressed(Key3D key) const {
    auto it = inputState.keyJustPressed.find(key);
    return (it != inputState.keyJustPressed.end()) ? it->second : false;
}

bool InputManager3D::isKeyJustReleased(Key3D key) const {
    auto it = inputState.keyJustReleased.find(key);
    return (it != inputState.keyJustReleased.end()) ? it->second : false;
}

bool InputManager3D::isMousePressed(MouseButton3D button) const {
    auto it = inputState.mouseStates.find(button);
    return (it != inputState.mouseStates.end()) ? it->second : false;
}

bool InputManager3D::isMouseJustPressed(MouseButton3D button) const {
    auto it = inputState.mouseJustPressed.find(button);
    return (it != inputState.mouseJustPressed.end()) ? it->second : false;
}

bool InputManager3D::isMouseJustReleased(MouseButton3D button) const {
    auto it = inputState.mouseJustReleased.find(button);
    return (it != inputState.mouseJustReleased.end()) ? it->second : false;
}

Vector3 InputManager3D::getMousePosition() const {
    return inputState.mousePosition;
}

Vector3 InputManager3D::getMouseDelta() const {
    return inputState.mouseDelta;
}

Vector3 InputManager3D::getScrollDelta() const {
    return inputState.scrollDelta;
}

void InputManager3D::setCursorVisible(bool visible) {
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}

void InputManager3D::setCursorLocked(bool locked) {
    cursorLocked = locked;
    if (window) {
        glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (locked) {
            firstMouse = true;  // Сбрасываем для предотвращения скачка
        }
    }
}

void InputManager3D::setCursorPosition(double x, double y) {
    if (window) {
        glfwSetCursorPos(window, x, y);
        lastMouseX = x;
        lastMouseY = y;
    }
}

// GLFW Callbacks
void InputManager3D::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager3D* manager = static_cast<InputManager3D*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->processKeyInput(key, action);
    }
}

void InputManager3D::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    InputManager3D* manager = static_cast<InputManager3D*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->processMouseButtonInput(button, action);
    }
}

void InputManager3D::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    InputManager3D* manager = static_cast<InputManager3D*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->processMouseMovement(xpos, ypos);
    }
}

void InputManager3D::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager3D* manager = static_cast<InputManager3D*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->processScroll(xoffset, yoffset);
    }
}

void InputManager3D::processKeyInput(int key, int action) {
    Key3D key3D = glfwKeyToKey3D(key);

    if (action == GLFW_PRESS) {
        inputState.keyStates[key3D] = true;
        inputState.keyJustPressed[key3D] = true;
    } else if (action == GLFW_RELEASE) {
        inputState.keyStates[key3D] = false;
        inputState.keyJustReleased[key3D] = true;
    }
}

void InputManager3D::processMouseButtonInput(int button, int action) {
    MouseButton3D button3D = glfwButtonToMouseButton3D(button);

    if (action == GLFW_PRESS) {
        inputState.mouseStates[button3D] = true;
        inputState.mouseJustPressed[button3D] = true;
    } else if (action == GLFW_RELEASE) {
        inputState.mouseStates[button3D] = false;
        inputState.mouseJustReleased[button3D] = true;
    }
}

void InputManager3D::processMouseMovement(double xpos, double ypos) {
    inputState.mousePosition = Vector3(static_cast<float>(xpos), static_cast<float>(ypos), 0.0f);

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    double deltaX = xpos - lastMouseX;
    double deltaY = lastMouseY - ypos;  // Инвертируем Y для стандартной 3D навигации

    inputState.mouseDelta = Vector3(static_cast<float>(deltaX), static_cast<float>(deltaY), 0.0f);

    lastMouseX = xpos;
    lastMouseY = ypos;
}

void InputManager3D::processScroll(double xoffset, double yoffset) {
    inputState.scrollDelta =
        Vector3(static_cast<float>(xoffset), static_cast<float>(yoffset), 0.0f);
}

void InputManager3D::processActions() {
    for (auto& pair : actions) {
        InputAction3D& action = pair.second;

        if (action.wasJustPressed(inputState)) {
            action.executePressed();
        }

        if (action.wasJustReleased(inputState)) {
            action.executeReleased();
        }

        if (action.isTriggered(inputState)) {
            action.executeHeld();
        }
    }
}

Key3D InputManager3D::glfwKeyToKey3D(int glfwKey) {
    // Преобразование GLFW клавиш в Key3D
    return static_cast<Key3D>(glfwKey);
}

MouseButton3D InputManager3D::glfwButtonToMouseButton3D(int glfwButton) {
    // Преобразование GLFW кнопок мыши в MouseButton3D
    return static_cast<MouseButton3D>(glfwButton);
}

// Controller3D implementation
Controller3D::Controller3D()
    : position(0, 0, 0),
      rotation(Quaternion::identity()),
      velocity(0, 0, 0),
      moveSpeed(5.0f),
      rotationSpeed(90.0f),
      mouseSensitivity(0.1f),
      mouseLookEnabled(true),
      movementEnabled(true),
      rotationEnabled(true),
      inputVector(0, 0, 0),
      rotationInput(0, 0, 0) {}

void Controller3D::update(float deltaTime) {
    updateMovement(deltaTime);
    updateRotation(deltaTime);
}

void Controller3D::handleInput(const InputState3D& inputState) {
    processMovementInput(inputState);
    processRotationInput(inputState);
}

void Controller3D::move(const Vector3& direction) {
    position += direction;
}

void Controller3D::moveForward(float amount) {
    move(getForward() * amount);
}

void Controller3D::moveRight(float amount) {
    move(getRight() * amount);
}

void Controller3D::moveUp(float amount) {
    move(getUp() * amount);
}

void Controller3D::rotateX(float angle) {
    Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3::right(), angle);
    rotation = deltaRotation * rotation;
    rotation.normalize();
}

void Controller3D::rotateY(float angle) {
    Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3::up(), angle);
    rotation = deltaRotation * rotation;
    rotation.normalize();
}

void Controller3D::rotateZ(float angle) {
    Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3::forward(), angle);
    rotation = deltaRotation * rotation;
    rotation.normalize();
}

void Controller3D::rotate(float pitch, float yaw, float roll) {
    Quaternion pitchQuat = Quaternion::fromAxisAngle(Vector3::right(), pitch);
    Quaternion yawQuat = Quaternion::fromAxisAngle(Vector3::up(), yaw);
    Quaternion rollQuat = Quaternion::fromAxisAngle(Vector3::forward(), roll);

    rotation = yawQuat * rotation * pitchQuat * rollQuat;
    rotation.normalize();
}

void Controller3D::lookAt(const Vector3& target) {
    Vector3 forward = (target - position).normalized();
    rotation = Quaternion::lookRotation(forward, Vector3::up());
}

Matrix4 Controller3D::getTransformMatrix() const {
    Matrix4 translationMatrix = Matrix4::translation(position);
    Matrix4 rotationMatrix = rotation.toMatrix();
    return translationMatrix * rotationMatrix;
}

Vector3 Controller3D::getForward() const {
    return rotation.rotate(Vector3::forward());
}

Vector3 Controller3D::getRight() const {
    return rotation.rotate(Vector3::right());
}

Vector3 Controller3D::getUp() const {
    return rotation.rotate(Vector3::up());
}

void Controller3D::setPosition(const Vector3& pos) {
    position = pos;
}

void Controller3D::setRotation(const Quaternion& rot) {
    rotation = rot;
}

void Controller3D::setVelocity(const Vector3& vel) {
    velocity = vel;
}

void Controller3D::updateMovement(float deltaTime) {
    if (!movementEnabled) {
        return;
    }

    Vector3 movement = inputVector * moveSpeed * deltaTime;
    move(movement);

    // Применяем velocity
    position += velocity * deltaTime;

    // Затухание input vector
    inputVector *= 0.9f;
}

void Controller3D::updateRotation(float deltaTime) {
    if (!rotationEnabled) {
        return;
    }

    if (rotationInput.magnitudeSquared() > 0.0f) {
        float rotationAmount = rotationSpeed * deltaTime;
        rotate(rotationInput.x * rotationAmount,
               rotationInput.y * rotationAmount,
               rotationInput.z * rotationAmount);
    }

    // Затухание rotation input
    rotationInput *= 0.9f;
}

void Controller3D::processMovementInput(const InputState3D& inputState) {
    Vector3 movement = Vector3::zero();

    // WASD движение
    if (inputState.keyStates.count(Key3D::W) && inputState.keyStates.at(Key3D::W)) {
        movement += getForward();
    }
    if (inputState.keyStates.count(Key3D::S) && inputState.keyStates.at(Key3D::S)) {
        movement -= getForward();
    }
    if (inputState.keyStates.count(Key3D::A) && inputState.keyStates.at(Key3D::A)) {
        movement -= getRight();
    }
    if (inputState.keyStates.count(Key3D::D) && inputState.keyStates.at(Key3D::D)) {
        movement += getRight();
    }

    // Вертикальное движение
    if (inputState.keyStates.count(Key3D::Space) && inputState.keyStates.at(Key3D::Space)) {
        movement += Vector3::up();
    }
    if (inputState.keyStates.count(Key3D::LeftShift) && inputState.keyStates.at(Key3D::LeftShift)) {
        movement -= Vector3::up();
    }

    inputVector = movement.normalized();
}

void Controller3D::processRotationInput(const InputState3D& inputState) {
    if (mouseLookEnabled) {
        Vector3 mouseDelta = inputState.mouseDelta;
        rotationInput.x = -mouseDelta.y * mouseSensitivity;  // Pitch
        rotationInput.y = -mouseDelta.x * mouseSensitivity;  // Yaw
    }

    // Клавиатурное вращение стрелками
    if (inputState.keyStates.count(Key3D::Up) && inputState.keyStates.at(Key3D::Up)) {
        rotationInput.x += 1.0f;
    }
    if (inputState.keyStates.count(Key3D::Down) && inputState.keyStates.at(Key3D::Down)) {
        rotationInput.x -= 1.0f;
    }
    if (inputState.keyStates.count(Key3D::Left) && inputState.keyStates.at(Key3D::Left)) {
        rotationInput.y += 1.0f;
    }
    if (inputState.keyStates.count(Key3D::Right) && inputState.keyStates.at(Key3D::Right)) {
        rotationInput.y -= 1.0f;
    }
}

// FirstPersonController implementation
FirstPersonController::FirstPersonController()
    : Controller3D(),
      minPitch(-89.0f),
      maxPitch(89.0f),
      currentPitch(0.0f),
      currentYaw(0.0f),
      jumpHeight(5.0f),
      gravity(-9.81f),
      grounded(true) {}

void FirstPersonController::handleInput(const InputState3D& inputState) {
    Controller3D::handleInput(inputState);

    // Прыжок
    if (grounded && inputState.keyJustPressed.count(Key3D::Space)
        && inputState.keyJustPressed.at(Key3D::Space)) {
        jump();
    }
}

void FirstPersonController::setPitchLimit(float minPitchValue, float maxPitchValue) {
    this->minPitch = minPitchValue;
    this->maxPitch = maxPitchValue;
}

void FirstPersonController::jump() {
    if (grounded) {
        velocity.y = std::sqrt(2.0f * jumpHeight * -gravity);
        grounded = false;
    }
}

void FirstPersonController::updatePitchYaw() {
    // Ограничиваем pitch
    currentPitch = std::max(minPitch, std::min(maxPitch, currentPitch));

    // Создаем кватернион из углов Эйлера
    rotation =
        Quaternion::fromEulerAngles(currentPitch * M_PI / 180.0f, currentYaw * M_PI / 180.0f, 0.0f);
}

// OrbitController implementation
OrbitController::OrbitController()
    : Controller3D(),
      target(0, 0, 0),
      distance(5.0f),
      minDistance(1.0f),
      maxDistance(20.0f),
      minPitch(-89.0f),
      maxPitch(89.0f),
      currentPitch(0.0f),
      currentYaw(0.0f),
      zoomSpeed(2.0f),
      orbitSpeed(100.0f) {}

void OrbitController::handleInput(const InputState3D& inputState) {
    // Орбитальное вращение мышью
    if (inputState.mouseStates.count(MouseButton3D::Left)
        && inputState.mouseStates.at(MouseButton3D::Left)) {
        Vector3 mouseDelta = inputState.mouseDelta;
        currentYaw -= mouseDelta.x * mouseSensitivity;
        currentPitch -= mouseDelta.y * mouseSensitivity;

        // Ограничиваем pitch
        currentPitch = std::max(minPitch, std::min(maxPitch, currentPitch));

        updateOrbitPosition();
    }

    // Зум колесиком мыши
    Vector3 scrollDelta = inputState.scrollDelta;
    if (scrollDelta.y != 0.0f) {
        distance -= scrollDelta.y * zoomSpeed;
        distance = std::max(minDistance, std::min(maxDistance, distance));
        updateOrbitPosition();
    }
}

void OrbitController::setTarget(const Vector3& newTarget) {
    target = newTarget;
    updateOrbitPosition();
}

void OrbitController::setDistance(float newDistance) {
    distance = std::max(minDistance, std::min(maxDistance, newDistance));
    updateOrbitPosition();
}

void OrbitController::setDistanceRange(float minDist, float maxDist) {
    minDistance = minDist;
    maxDistance = maxDist;
    distance = std::max(minDistance, std::min(maxDistance, distance));
}

void OrbitController::setPitchRange(float minPitchDeg, float maxPitchDeg) {
    minPitch = minPitchDeg;
    maxPitch = maxPitchDeg;
    currentPitch = std::max(minPitch, std::min(maxPitch, currentPitch));
}

void OrbitController::updateOrbitPosition() {
    float pitchRad = currentPitch * M_PI / 180.0f;
    float yawRad = currentYaw * M_PI / 180.0f;

    // Вычисляем позицию камеры в сферических координатах
    float x = distance * std::cos(pitchRad) * std::cos(yawRad);
    float y = distance * std::sin(pitchRad);
    float z = distance * std::cos(pitchRad) * std::sin(yawRad);

    position = target + Vector3(x, y, z);

    // Направляем камеру на цель
    lookAt(target);
}

}  // namespace Input
}  // namespace HyperEngine
