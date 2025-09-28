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

#include "HyperEngine/Core/GameObject3D.h"
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Rendering/Renderer3D.h"
#include "HyperEngine/Rendering/Camera3D.h"
#include "HyperEngine/Rendering/Mesh3D.h"
#include "HyperEngine/Rendering/Shader3D.h"
#include "HyperEngine/Input/Input3D.h"
#include "HyperEngine/Physics/Physics3D.h"
#include "HyperEngine/Math/Math.h"
#include "HyperEngine/Math/Quaternion.h"
#include "HyperEngine/Math/MathConstants.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

using namespace HyperEngine;
using namespace HyperEngine::Core;
using namespace HyperEngine::Rendering;
using namespace HyperEngine::Input;
using namespace HyperEngine::Physics;
using namespace HyperEngine::Math;

class Demo3D {
public:
    Demo3D() : window(nullptr), running(false) {}
    
    bool initialize() {
        // Инициализация UTF-8 консоли
        Console::initialize();
        Console::setTitle("🚀 HyperEngine 3D Demo");
        
        SAFE_PRINT_LINE("🌟═══════════════════════════════════════════🌟");
        SAFE_PRINT_LINE("          🚀 HYPERENGINE 3D ДЕМО 🚀");
        SAFE_PRINT_LINE("🌟═══════════════════════════════════════════🌟");
        
        // Инициализация GLFW
        if (!glfwInit()) {
            Console::error("❌ Не удалось инициализировать GLFW");
            return false;
        }
        
        // Настройка GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        // Создание окна
        window = glfwCreateWindow(1280, 720, "🚀 HyperEngine 3D Demo", nullptr, nullptr);
        if (!window) {
            Console::error("❌ Не удалось создать окно GLFW");
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        
        // Инициализация GLEW
        if (glewInit() != GLEW_OK) {
            Console::error("❌ Не удалось инициализировать GLEW");
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }
        
        Console::info("✅ GLEW инициализирован успешно");
        Console::info("🔧 OpenGL версия: " + std::string((char*)glGetString(GL_VERSION)));
        Console::info("🎮 Графический адаптер: " + std::string((char*)glGetString(GL_RENDERER)));
        
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
        Console::info("✅ Демо инициализировано успешно!");
        return true;
    }
    
    void run() {
        Console::info("🔄 Запуск основного цикла рендеринга...");
        
        auto lastTime = glfwGetTime();
        int frameCount = 0;
        
        while (running && !glfwWindowShouldClose(window)) {
            auto currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;
            
            update(deltaTime);
            render();
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            
            frameCount++;
            if (frameCount % 300 == 0) { // Каждые 5 секунд при 60 FPS
                Console::info("🖼️ Кадров отрендерено: " + SAFE_TO_STRING(frameCount));
            }
        }
        
        Console::info("🏁 Демо завершено (всего кадров: " + SAFE_TO_STRING(frameCount) + ")");
    }
    
    void cleanup() {
        Console::info("🧹 Очистка ресурсов демо...");
        
        // Очистка игровых объектов
        GameObject3D::clearAllObjects();
        
        // Очистка систем движка
        // physicsWorld.reset();
        InputManager3D::getInstance().cleanup();
        Renderer3D::getInstance().cleanup();
        
        // Очистка GLFW
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        
        Console::info("✅ Демо очищено успешно");
    }

private:
    GLFWwindow* window;
    bool running;
    
    // Системы движка
    std::shared_ptr<Camera3D> camera;
    std::shared_ptr<FirstPersonController> controller;
    // std::shared_ptr<PhysicsWorld3D> physicsWorld;
    
    // Игровые объекты
    GameObject3D* cubeObject;
    GameObject3D* planeObject;
    GameObject3D* cameraObject;
    
    bool initializeEngine() {
        Console::info("⚙️ Инициализация систем движка...");
        
        // Инициализация рендерера
        if (!Renderer3D::getInstance().initialize(1280, 720)) {
            Console::error("❌ Не удалось инициализировать Renderer3D");
            return false;
        }
        
        // Инициализация системы ввода
        if (!InputManager3D::getInstance().initialize(window)) {
            Console::error("❌ Не удалось инициализировать InputManager3D");
            return false;
        }
        
        // Создание физического мира (упрощенная версия)
        // physicsWorld = std::make_shared<PhysicsWorld3D>();
        // physicsWorld->setGravity(Vector3(0, -9.81f, 0));
        Console::info("ℹ️ Физический мир пропущен для упрощения демо");
        
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
        controller->setMouseSensitivity(0.1f);
        
        // Настройка действий ввода
        setupInputActions();
        
        Console::info("✅ Системы движка инициализированы успешно");
        return true;
    }
    
    void setupInputActions() {
        auto& inputManager = InputManager3D::getInstance();
        
        // Действие для выхода
        InputAction3D exitAction("exit");
        exitAction.addKey(Key3D::Escape);
        exitAction.setOnPressed([this]() {
            Console::info("🚪 Нажата клавиша выхода");
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
            Console::info(mouseLocked ? "🔒 Мышь заблокирована" : "🔓 Мышь разблокирована");
        });
        inputManager.addAction(mouseModeAction);
    }
    
    void createScene() {
        Console::info("🎬 Создание демонстрационной сцены...");
        
        // Создание простых объектов без сложных компонентов
        cubeObject = GameObject3D::create("Cube");
        cubeObject->getTransform()->setPosition(Vector3(0, 1, 0));
        
        planeObject = GameObject3D::create("Floor");
        planeObject->getTransform()->setPosition(Vector3(0, -1, 0));
        planeObject->getTransform()->setScale(Vector3(10, 1, 10));
        
        cameraObject = GameObject3D::create("MainCamera");
        
        // Добавление освещения
        setupLighting();
        
        Console::info("✅ Сцена создана с " + SAFE_TO_STRING(GameObject3D::getAllObjects().size()) + " объектами");
        Console::info("ℹ️ Используется упрощенная сцена без сложных компонентов");
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
        
        Console::info("💡 Настройка освещения завершена");
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
        
        // Обновление физики (пропущено для упрощения)
        // physicsWorld->update(deltaTime);
        
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
        Console::error("❌ Не удалось инициализировать демо");
        return -1;
    }
    
    demo.run();
    demo.cleanup();
    
    return 0;
}

