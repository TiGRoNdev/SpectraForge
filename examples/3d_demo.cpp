/**
 * @file 3d_demo.cpp
 * @brief Демонстрация возможностей 3D движка HyperEngine
 * 
 * Этот пример показывает основные возможности 3D движка:
 * - Инициализация рендерера
 * - Создание и управление игровыми объектами
 * - Система камер и навигация
 * - Основы физики и рендеринга
 */

#include "Engine3D/Core/GameObject3D.h"
#include "Engine3D/Rendering/Renderer3D.h"
#include "Engine3D/Rendering/Camera3D.h"
#include "Engine3D/Rendering/Mesh3D.h"
#include "Engine3D/Rendering/Shader3D.h"
#include "Engine3D/Input/Input3D.h"
#include "Engine3D/Physics/Physics3D.h"
#include "Engine3D/Math/Math.h"
#include "Engine3D/Math/Quaternion.h"
#include "Engine3D/Math/MathConstants.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

using namespace Engine3D;
using namespace Engine3D::Core;
using namespace Engine3D::Rendering;
using namespace Engine3D::Input;
using namespace Engine3D::Physics;
using namespace Engine3D::Math;

class Demo3D {
public:
    Demo3D() : window(nullptr), running(false) {}
    
    bool initialize() {
        std::cout << "=== HyperEngine 3D Demo ===" << std::endl;
        
        // Инициализация GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        // Настройка GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        // Создание окна
        window = glfwCreateWindow(1280, 720, "HyperEngine 3D Demo", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        
        // Callback для закрытия окна
        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            Demo3D* demo = static_cast<Demo3D*>(glfwGetWindowUserPointer(window));
            demo->running = false;
        });
        
        // Инициализация систем движка
        if (!initializeEngine()) {
            return false;
        }
        
        // Создание сцены
        createScene();
        
        running = true;
        std::cout << "Demo initialized successfully!" << std::endl;
        return true;
    }
    
    void run() {
        std::cout << "Starting main loop..." << std::endl;
        
        auto lastTime = glfwGetTime();
        
        while (running && !glfwWindowShouldClose(window)) {
            auto currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            update(deltaTime);
            render();
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        std::cout << "Demo finished" << std::endl;
    }
    
    void cleanup() {
        std::cout << "Cleaning up demo..." << std::endl;
        
        // Очистка игровых объектов
        GameObject3D::clearAllObjects();
        
        // Очистка систем движка
        physicsWorld.reset();
        InputManager3D::getInstance().cleanup();
        Renderer3D::getInstance().cleanup();
        
        // Очистка GLFW
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        
        std::cout << "Demo cleaned up" << std::endl;
    }

private:
    GLFWwindow* window;
    bool running;
    
    // Системы движка
    std::shared_ptr<Camera3D> camera;
    std::shared_ptr<FirstPersonController> controller;
    std::shared_ptr<PhysicsWorld3D> physicsWorld;
    
    // Игровые объекты
    GameObject3D* cubeObject;
    GameObject3D* planeObject;
    GameObject3D* cameraObject;
    
    bool initializeEngine() {
        std::cout << "Initializing engine systems..." << std::endl;
        
        // Инициализация рендерера
        if (!Renderer3D::getInstance().initialize(1280, 720)) {
            std::cerr << "Failed to initialize Renderer3D" << std::endl;
            return false;
        }
        
        // Инициализация системы ввода
        if (!InputManager3D::getInstance().initialize(window)) {
            std::cerr << "Failed to initialize InputManager3D" << std::endl;
            return false;
        }
        
        // Создание физического мира
        physicsWorld = std::make_shared<PhysicsWorld3D>();
        physicsWorld->setGravity(Vector3(0, -9.81f, 0));
        
        // Создание камеры
        camera = std::make_shared<Camera3D>();
        camera->setPerspective(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        camera->setPosition(Vector3(0, 2, 5));
        camera->lookAt(Vector3(0, 2, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
        
        // Установка главной камеры
        Renderer3D::getInstance().setMainCamera(camera);
        
        // Создание контроллера
        controller = std::make_shared<FirstPersonController>();
        controller->setPosition(Vector3(0, 2, 5));
        controller->setMoveSpeed(5.0f);
        controller->setSensitivity(0.1f);
        
        // Настройка действий ввода
        setupInputActions();
        
        std::cout << "Engine systems initialized" << std::endl;
        return true;
    }
    
    void setupInputActions() {
        auto& inputManager = InputManager3D::getInstance();
        
        // Действие для выхода
        InputAction3D exitAction("exit");
        exitAction.addKey(Key3D::Escape);
        exitAction.setOnPressed([this]() {
            std::cout << "Exit key pressed" << std::endl;
            running = false;
        });
        inputManager.addAction(exitAction);
        
        // Действие для переключения режима мыши
        InputAction3D mouseModeAction("toggle_mouse");
        mouseModeAction.addKey(Key3D::Tab);
        mouseModeAction.setOnPressed([this]() {
            static bool mouseLocked = true;
            mouseLocked = !mouseLocked;
            InputManager3D::getInstance().setCursorLocked(mouseLocked);
            std::cout << "Mouse " << (mouseLocked ? "locked" : "unlocked") << std::endl;
        });
        inputManager.addAction(mouseModeAction);
    }
    
    void createScene() {
        std::cout << "Creating demo scene..." << std::endl;
        
        // Создание объекта камеры
        cameraObject = GameObject3D::create("MainCamera");
        auto* cameraComponent = cameraObject->addComponent<Camera3DComponent>();
        cameraComponent->setMainCamera(true);
        cameraComponent->setFieldOfView(60.0f);
        
        // Создание куба
        cubeObject = GameObject3D::create("Cube");
        cubeObject->getTransform()->setPosition(Vector3(0, 1, 0));
        
        auto* meshRenderer = cubeObject->addComponent<MeshRenderer3D>();
        auto cubeMesh = Mesh3D::createCube(2.0f);
        auto basicShader = Shader3D::createBasicShader();
        
        meshRenderer->setMesh(cubeMesh);
        meshRenderer->setShader(basicShader);
        meshRenderer->setColor(Vector3(0.8f, 0.2f, 0.2f)); // Красный куб
        
        // Добавление физики к кубу
        auto* rigidBody = cubeObject->addComponent<RigidBody3DComponent>();
        auto cubeRigidBody = std::make_shared<RigidBody3D>();
        cubeRigidBody->setPosition(Vector3(0, 1, 0));
        cubeRigidBody->setMass(1.0f);
        
        auto cubeCollider = std::make_shared<BoxCollider3D>(Vector3(2, 2, 2));
        cubeRigidBody->setCollider(cubeCollider);
        
        rigidBody->setRigidBody(cubeRigidBody);
        physicsWorld->addRigidBody(cubeRigidBody);
        
        // Создание плоскости (пол)
        planeObject = GameObject3D::create("Floor");
        planeObject->getTransform()->setPosition(Vector3(0, -1, 0));
        planeObject->getTransform()->setScale(Vector3(10, 1, 10));
        
        auto* planeMeshRenderer = planeObject->addComponent<MeshRenderer3D>();
        auto planeMesh = Mesh3D::createPlane(10.0f, 10.0f);
        
        planeMeshRenderer->setMesh(planeMesh);
        planeMeshRenderer->setShader(basicShader);
        planeMeshRenderer->setColor(Vector3(0.2f, 0.8f, 0.2f)); // Зеленый пол
        
        // Добавление статического коллайдера для пола
        auto planeCollider = std::make_shared<PlaneCollider3D>(Vector3(0, 1, 0), 1.0f);
        physicsWorld->addCollider(planeCollider);
        
        // Добавление освещения
        setupLighting();
        
        std::cout << "Scene created with " << GameObject3D::getAllObjects().size() << " objects" << std::endl;
    }
    
    void setupLighting() {
        auto& renderer = Renderer3D::getInstance();
        
        // Добавление направленного света (солнце)
        Renderer3D::Light sunLight;
        sunLight.type = Renderer3D::Light::DIRECTIONAL;
        sunLight.direction = Vector3(-0.5f, -1.0f, -0.3f).normalized();
        sunLight.color = Vector3(1.0f, 0.95f, 0.8f);
        sunLight.intensity = 1.0f;
        renderer.addLight(sunLight);
        
        // Добавление точечного света
        Renderer3D::Light pointLight;
        pointLight.type = Renderer3D::Light::POINT;
        pointLight.position = Vector3(2, 3, 2);
        pointLight.color = Vector3(0.8f, 0.8f, 1.0f);
        pointLight.intensity = 2.0f;
        pointLight.range = 10.0f;
        renderer.addLight(pointLight);
        
        // Установка ambient освещения
        renderer.setAmbientLight(Vector3(0.2f, 0.2f, 0.3f));
        
        std::cout << "Lighting setup complete" << std::endl;
    }
    
    void update(float deltaTime) {
        // Обновление системы ввода
        InputManager3D::getInstance().update();
        
        // Обновление контроллера
        controller->handleInput(InputManager3D::getInstance().getInputState());
        controller->update(deltaTime);
        
        // Обновление позиции камеры
        camera->setPosition(controller->getPosition());
        camera->setRotation(controller->getRotation());
        
        // Обновление физики
        physicsWorld->update(deltaTime);
        
        // Обновление игровых объектов
        for (auto* obj : GameObject3D::getAllObjects()) {
            if (obj && obj->isActive()) {
                obj->update(deltaTime);
            }
        }
        
        // Простая анимация куба (вращение)
        if (cubeObject) {
            auto* transform = cubeObject->getTransform();
            Quaternion currentRotation = transform->getRotation();
            Quaternion deltaRotation = Quaternion::fromAxisAngle(Vector3::up(), 45.0f * deltaTime * Constants::DEG_TO_RAD);
            transform->setRotation(deltaRotation * currentRotation);
        }
    }
    
    void render() {
        auto& renderer = Renderer3D::getInstance();
        
        // Начало кадра
        renderer.beginFrame();
        
        // Рендеринг всех игровых объектов
        for (auto* obj : GameObject3D::getAllObjects()) {
            if (obj && obj->isActive()) {
                obj->render();
            }
        }
        
        // Завершение кадра
        renderer.endFrame();
    }
};

int main() {
    Demo3D demo;
    
    if (!demo.initialize()) {
        std::cerr << "Failed to initialize demo" << std::endl;
        return -1;
    }
    
    demo.run();
    demo.cleanup();
    
    return 0;
}
