#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Engine4D/Math/Vector4.h>
#include <Engine4D/Math/Matrix4.h>
#include <Engine4D/Math/Quaternion4D.h>
#include <Engine4D/Rendering/Renderer.h>
#include <Engine4D/Input/Input4D.h>
#include <Engine4D/Core/GameObject4D.h>
#include <chrono>

using namespace Engine4D;
using namespace Engine4D::Math;
using namespace Engine4D::Rendering;
using namespace Engine4D::Input;
using namespace Engine4D::Core;

// Глобальные переменные
GLFWwindow* window;
Renderer* renderer;
InputManager4D* inputManager;
Controller4D* controller;
GameObject4D* tesseractObject;
Camera4DComponent* camera;
bool wireframeMode = false;
bool crossSectionMode = false;
float crossSectionW = 0.0f;
float rotationSpeed = 1.0f;

// Функции
bool initializeGLFW();
bool initializeGLEW();
bool initializeEngine();
void cleanup();
void update(float deltaTime);
void render();
void handleInput(float deltaTime);
void setupScene();
void createTesseract();

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main() {
    std::cout << "=== 4D Game Engine Demo ===" << std::endl;
    std::cout << "Управление:" << std::endl;
    std::cout << "WASD - движение в 3D пространстве" << std::endl;
    std::cout << "QE - движение в W-измерении" << std::endl;
    std::cout << "Space/Shift - движение вверх/вниз" << std::endl;
    std::cout << "Мышь - поворот камеры" << std::endl;
    std::cout << "1-6 - повороты в различных плоскостях" << std::endl;
    std::cout << "Tab - переключение сечения" << std::endl;
    std::cout << "F1 - переключение каркасного режима" << std::endl;
    std::cout << "R - сброс вида" << std::endl;
    std::cout << "ESC - выход" << std::endl;
    std::cout << "=========================" << std::endl;

    // Инициализация
    if (!initializeGLFW()) {
        std::cerr << "Ошибка инициализации GLFW" << std::endl;
        return -1;
    }

    if (!initializeGLEW()) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        cleanup();
        return -1;
    }

    if (!initializeEngine()) {
        std::cerr << "Ошибка инициализации движка" << std::endl;
        cleanup();
        return -1;
    }

    setupScene();

    // Основной цикл
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running && !glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Обработка событий
        glfwPollEvents();

        // Обновление
        update(deltaTime);

        // Рендеринг
        render();

        // Проверка на выход
        if (inputManager->isKeyPressed(Key4D::Exit)) {
            running = false;
        }
    }

    cleanup();
    std::cout << "Демо завершено" << std::endl;
    return 0;
}

bool initializeGLFW() {
    if (!glfwInit()) {
        return false;
    }

    // Настройки OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Создание окна
    window = glfwCreateWindow(1200, 800, "4D Game Engine Demo", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Настройки OpenGL
    glfwSwapInterval(1); // VSync

    return true;
}

bool initializeGLEW() {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Ошибка GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    std::cout << "OpenGL версия: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW версия: " << glewGetString(GLEW_VERSION) << std::endl;

    return true;
}

bool initializeEngine() {
    // Инициализация рендерера
    renderer = &Renderer::getInstance();
    if (!renderer->initialize(1200, 800)) {
        return false;
    }

    // Инициализация системы ввода
    inputManager = &InputManager4D::getInstance();
    if (!inputManager->initialize(window)) {
        return false;
    }

    // Настройка действий по умолчанию
    DefaultActions4D::setupDefaultActions(*inputManager);

    // Создание контроллера
    controller = new Controller4D();
    controller->setMoveSpeed(10.0f);
    controller->setRotationSpeed(3.0f);
    controller->setWSpeed(5.0f);
    controller->setSensitivity(0.002f);

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    return true;
}

void cleanup() {
    if (tesseractObject) {
        delete tesseractObject;
    }
    if (controller) {
        delete controller;
    }
    if (renderer) {
        renderer->cleanup();
    }
    if (inputManager) {
        inputManager->cleanup();
    }
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void update(float deltaTime) {
    // Обновление системы ввода
    inputManager->update();

    // Обработка ввода
    handleInput(deltaTime);

    // Обновление контроллера
    controller->update(deltaTime);

    // Обновление объектов
    if (tesseractObject) {
        tesseractObject->update(deltaTime);
    }
}

void render() {
    renderer->beginFrame();
    renderer->clear();

    // Настройка камеры
    if (camera) {
        Matrix4 viewMatrix = camera->getViewMatrix();
        Matrix4 projMatrix = camera->getProjectionMatrix();
        
        // Применяем сечение, если включено
        if (crossSectionMode) {
            camera->camera.setCrossSection(crossSectionW);
        }
    }

    // Рендеринг объектов
    if (tesseractObject) {
        tesseractObject->render();
    }

    renderer->endFrame();
    glfwSwapBuffers(window);
}

void handleInput(float deltaTime) {
    // Обработка ввода контроллером
    controller->handleInput(inputManager->getInputState());

    // Специальные действия
    if (inputManager->isKeyJustPressed(Key4D::ToggleWireframe)) {
        wireframeMode = !wireframeMode;
        std::cout << "Каркасный режим: " << (wireframeMode ? "ВКЛ" : "ВЫКЛ") << std::endl;
    }

    if (inputManager->isKeyJustPressed(Key4D::ToggleCrossSection)) {
        crossSectionMode = !crossSectionMode;
        std::cout << "Режим сечения: " << (crossSectionMode ? "ВКЛ" : "ВЫКЛ") << std::endl;
    }

    if (inputManager->isKeyJustPressed(Key4D::ResetView)) {
        controller->position = Vector4::zero();
        controller->rotation = Vector4::zero();
        crossSectionW = 0.0f;
        std::cout << "Вид сброшен" << std::endl;
    }

    // Управление сечением
    if (crossSectionMode) {
        Vector4 scrollDelta = inputManager->getScrollDelta();
        crossSectionW += scrollDelta.y * 0.1f;
        crossSectionW = (crossSectionW < -2.0f) ? -2.0f : (crossSectionW > 2.0f) ? 2.0f : crossSectionW;
        
        if (camera) {
            camera->getCamera().setCrossSection(crossSectionW);
        }
    }

    // Обновление позиции камеры
    if (camera) {
        camera->getCamera().setPosition(controller->getPosition());
        camera->getCamera().setTarget(controller->getPosition() + controller->forward());
    }
}

void setupScene() {
    // Создание камеры
    GameObject4D* cameraObject = GameObject4D::create("MainCamera");
    camera = cameraObject->addComponent<Camera4DComponent>();
    camera->setMainCamera(true);
    camera->setFieldOfView(45.0f);
    camera->setNearPlane(0.1f);
    camera->setFarPlane(100.0f);

    // Создание тессеракта
    createTesseract();

    // Создание источника света
    GameObject4D* lightObject = GameObject4D::create("Light");
    lightObject->getTransform()->setPosition(Vector4(5.0f, 5.0f, 5.0f, 0.0f));

    std::cout << "Сцена создана" << std::endl;
}

void createTesseract() {
    // Создание объекта тессеракта
    tesseractObject = GameObject4D::createPrimitive("Tesseract");
    tesseractObject->transform->setPosition(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    tesseractObject->transform->setScale(Vector4(2.0f, 2.0f, 2.0f, 2.0f));

    // Получаем компонент рендеринга
    auto* renderer = tesseractObject->getComponent<MeshRenderer4D>();
    if (renderer) {
        // Создаем шейдер
        auto shader = std::make_shared<Shader4D>();
        if (shader->loadFromFiles("shaders/vertex_4d.glsl", "shaders/fragment_4d.glsl")) {
            renderer->setShader(shader);
            std::cout << "Шейдер загружен успешно" << std::endl;
        } else {
            std::cout << "Ошибка загрузки шейдера" << std::endl;
        }

        // Настраиваем цвет
        renderer->setColor(Vector4(0.8f, 0.2f, 0.2f, 0.8f));
    }

    // Добавляем физическое тело
    auto* rigidBody = tesseractObject->addComponent<RigidBody4DComponent>();
    auto physicsBody = std::make_shared<Physics::RigidBody4D>();
    physicsBody->setMass(1.0f);
    rigidBody->setRigidBody(physicsBody);

    std::cout << "Тессеракт создан" << std::endl;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    renderer->setViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
