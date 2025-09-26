# Примеры использования - 4D Game Engine

## Содержание

1. [Базовый пример](#базовый-пример)
2. [Создание 4D объектов](#создание-4d-объектов)
3. [Навигация в 4D пространстве](#навигация-в-4d-пространстве)
4. [Физика в 4D](#физика-в-4d)
5. [Рендеринг с сечениями](#рендеринг-с-сечениями)
6. [Система частиц](#система-частиц)
7. [Продвинутые примеры](#продвинутые-примеры)

## Базовый пример

### Простое окно с 4D объектом

```cpp
#include <Engine4D/Engine4D.h>
#include <GLFW/glfw3.h>

int main() {
    // Инициализация GLFW
    if (!glfwInit()) return -1;
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "4D Engine Demo", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Инициализация движка
    auto& renderer = Engine4D::Rendering::Renderer::getInstance();
    renderer.initialize(800, 600);
    
    // Создание тессеракта
    auto tesseract = Engine4D::Rendering::Mesh4D::createTesseract(2.0f);
    tesseract.uploadToGPU();
    
    // Создание шейдера
    auto shader = std::make_shared<Engine4D::Rendering::Shader4D>();
    shader->loadFromFiles("shaders/vertex_4d.glsl", "shaders/fragment_4d.glsl");
    
    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        renderer.beginFrame();
        renderer.clear();
        
        // Рендеринг тессеракта
        Engine4D::Math::Matrix4 transform = Engine4D::Math::Matrix4::identity();
        renderer.renderMesh(tesseract, transform, *shader);
        
        renderer.endFrame();
        glfwSwapBuffers(window);
    }
    
    // Очистка
    renderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
```

## Создание 4D объектов

### Создание тессеракта с компонентами

```cpp
#include <Engine4D/Core/GameObject4D.h>

void createTesseractScene() {
    // Создание объекта тессеракта
    auto* tesseract = Engine4D::Core::GameObject4D::createPrimitive("Tesseract");
    
    // Настройка позиции и масштаба
    tesseract->transform->setPosition(Engine4D::Math::Vector4(0, 0, 0, 0));
    tesseract->transform->setScale(Engine4D::Math::Vector4(2, 2, 2, 2));
    
    // Получение компонента рендеринга
    auto* renderer = tesseract->getComponent<Engine4D::Core::MeshRenderer4D>();
    if (renderer) {
        // Настройка цвета
        renderer->setColor(Engine4D::Math::Vector4(1, 0.5f, 0, 0.8f));
        
        // Создание и загрузка шейдера
        auto shader = std::make_shared<Engine4D::Rendering::Shader4D>();
        if (shader->loadFromFiles("shaders/vertex_4d.glsl", "shaders/fragment_4d.glsl")) {
            renderer->setShader(shader);
        }
    }
    
    // Добавление физического тела
    auto* rigidBody = tesseract->addComponent<Engine4D::Core::RigidBody4DComponent>();
    auto physicsBody = std::make_shared<Engine4D::Physics::RigidBody4D>();
    physicsBody->setMass(1.0f);
    rigidBody->setRigidBody(physicsBody);
    
    // Добавление коллайдера
    auto* collider = tesseract->addComponent<Engine4D::Core::Collider4DComponent>();
    auto boxCollider = std::make_shared<Engine4D::Physics::BoxCollider4D>(
        Engine4D::Math::Vector4(2, 2, 2, 2)
    );
    collider->setCollider(boxCollider);
}
```

### Создание 4D камеры

```cpp
void createCamera() {
    // Создание объекта камеры
    auto* cameraObj = Engine4D::Core::GameObject4D::create("MainCamera");
    
    // Добавление компонента камеры
    auto* camera = cameraObj->addComponent<Engine4D::Core::Camera4DComponent>();
    camera->setMainCamera(true);
    camera->setFieldOfView(45.0f);
    camera->setNearPlane(0.1f);
    camera->setFarPlane(100.0f);
    
    // Настройка позиции в 4D пространстве
    cameraObj->transform->setPosition(Engine4D::Math::Vector4(0, 0, -5, 0));
    cameraObj->transform->setRotation(Engine4D::Math::Quaternion4D::identity());
}
```

### Создание 4D источника света

```cpp
void createLight() {
    auto* lightObj = Engine4D::Core::GameObject4D::create("Light");
    
    // Позиция в 4D пространстве
    lightObj->transform->setPosition(Engine4D::Math::Vector4(5, 5, 5, 0));
    
    // Можно добавить компонент света (если реализован)
    // auto* light = lightObj->addComponent<Light4DComponent>();
}
```

## Навигация в 4D пространстве

### Базовый контроллер

```cpp
#include <Engine4D/Input/Input4D.h>

void setupInput() {
    // Получение менеджера ввода
    auto& inputManager = Engine4D::Input::InputManager4D::getInstance();
    
    // Настройка действий по умолчанию
    Engine4D::Input::DefaultActions4D::setupDefaultActions(inputManager);
    
    // Создание контроллера
    auto controller = std::make_unique<Engine4D::Input::Controller4D>();
    controller->setMoveSpeed(10.0f);
    controller->setWSpeed(5.0f);
    controller->setSensitivity(0.002f);
    controller->setWMovement(true);
    controller->setMouseLook(true);
}
```

### Обработка ввода в игровом цикле

```cpp
void updateInput(Engine4D::Input::Controller4D& controller, float deltaTime) {
    auto& inputManager = Engine4D::Input::InputManager4D::getInstance();
    
    // Обновление системы ввода
    inputManager.update();
    
    // Обработка ввода контроллером
    controller.handleInput(inputManager.getInputState());
    
    // Специальные действия
    if (inputManager.isKeyJustPressed(Engine4D::Input::Key4D::ToggleWireframe)) {
        // Переключение каркасного режима
        toggleWireframeMode();
    }
    
    if (inputManager.isKeyJustPressed(Engine4D::Input::Key4D::ToggleCrossSection)) {
        // Переключение режима сечения
        toggleCrossSectionMode();
    }
    
    // Управление сечением колесом мыши
    if (crossSectionMode) {
        auto scrollDelta = inputManager.getScrollDelta();
        crossSectionW += scrollDelta.y * 0.1f;
        crossSectionW = std::clamp(crossSectionW, -2.0f, 2.0f);
    }
}
```

### Создание пользовательских действий

```cpp
void createCustomActions() {
    auto& inputManager = Engine4D::Input::InputManager4D::getInstance();
    
    // Действие для быстрого движения
    Engine4D::Input::InputAction4D speedBoost("SpeedBoost");
    speedBoost.addKey(Engine4D::Input::Key4D::SpeedBoost);
    speedBoost.setOnHeld([]() {
        // Увеличить скорость движения
        controller->setMoveSpeed(20.0f);
    });
    inputManager.addAction(speedBoost);
    
    // Действие для медленного движения
    Engine4D::Input::InputAction4D slowMotion("SlowMotion");
    slowMotion.addKey(Engine4D::Input::Key4D::SlowMotion);
    slowMotion.setOnHeld([]() {
        // Уменьшить скорость движения
        controller->setMoveSpeed(2.0f);
    });
    inputManager.addAction(slowMotion);
}
```

## Физика в 4D

### Создание физического мира

```cpp
void setupPhysics() {
    // Создание физического мира
    auto physicsWorld = std::make_unique<Engine4D::Physics::PhysicsWorld4D>();
    
    // Настройка гравитации (вниз по Y)
    physicsWorld->setGravity(Engine4D::Math::Vector4(0, -9.81f, 0, 0));
    
    // Создание физических объектов
    createPhysicsObjects(*physicsWorld);
}

void createPhysicsObjects(Engine4D::Physics::PhysicsWorld4D& world) {
    // Создание 4D сферы
    auto sphereBody = std::make_shared<Engine4D::Physics::RigidBody4D>();
    sphereBody->setPosition(Engine4D::Math::Vector4(0, 5, 0, 0));
    sphereBody->setMass(1.0f);
    world.addBody(sphereBody);
    
    auto sphereCollider = std::make_shared<Engine4D::Physics::SphereCollider4D>(1.0f);
    sphereCollider->setCenter(sphereBody->getPosition());
    world.addCollider(sphereCollider);
    
    // Создание 4D плоскости (пол)
    auto planeCollider = std::make_shared<Engine4D::Physics::PlaneCollider4D>(
        Engine4D::Math::Vector4::unitY(), // Нормаль вверх
        0.0f // Расстояние от начала координат
    );
    world.addCollider(planeCollider);
    
    // Создание 4D бокса
    auto boxBody = std::make_shared<Engine4D::Physics::RigidBody4D>();
    boxBody->setPosition(Engine4D::Math::Vector4(3, 2, 0, 1));
    boxBody->setMass(2.0f);
    world.addBody(boxBody);
    
    auto boxCollider = std::make_shared<Engine4D::Physics::BoxCollider4D>(
        Engine4D::Math::Vector4(2, 2, 2, 2)
    );
    boxCollider->setCenter(boxBody->getPosition());
    world.addCollider(boxCollider);
}
```

### Raycasting в 4D

```cpp
void performRaycast() {
    auto physicsWorld = std::make_unique<Engine4D::Physics::PhysicsWorld4D>();
    
    // Начальная точка и направление луча
    Engine4D::Math::Vector4 origin(0, 0, 0, 0);
    Engine4D::Math::Vector4 direction(1, 0, 0, 0); // Вправо по X
    
    // Выполнение raycast
    auto hit = physicsWorld->raycast(origin, direction, 100.0f);
    
    if (hit.hasHit) {
        std::cout << "Попадание в точке: " << hit.point << std::endl;
        std::cout << "Нормаль: " << hit.normal << std::endl;
        std::cout << "Расстояние: " << hit.distance << std::endl;
    } else {
        std::cout << "Луч не попал ни в один объект" << std::endl;
    }
}
```

## Рендеринг с сечениями

### Настройка сечения

```cpp
void setupCrossSection() {
    auto* camera = findMainCamera();
    if (camera) {
        // Включение режима сечения
        camera->camera.setCrossSection(0.0f); // Сечение при w = 0
        
        // Настройка шейдера для сечения
        auto shader = std::make_shared<Engine4D::Rendering::Shader4D>();
        shader->loadFromFiles("shaders/vertex_4d.glsl", "shaders/fragment_4d.glsl");
        
        // Установка униформы сечения
        shader->setFloat("crossSectionW", 0.0f);
        shader->setBool("useCrossSection", true);
    }
}
```

### Динамическое изменение сечения

```cpp
void updateCrossSection(float wValue) {
    // Обновление всех объектов с учетом нового сечения
    auto objects = Engine4D::Core::GameObject4D::findAllWithTag("4DObject");
    
    for (auto* obj : objects) {
        auto* renderer = obj->getComponent<Engine4D::Core::MeshRenderer4D>();
        if (renderer && renderer->shader) {
            renderer->shader->setFloat("crossSectionW", wValue);
        }
    }
    
    // Обновление камеры
    auto* camera = findMainCamera();
    if (camera) {
        camera->camera.setCrossSection(wValue);
    }
}
```

### Создание эффекта "призрака" для далеких W-слоев

```cpp
void setupGhostEffect() {
    auto shader = std::make_shared<Engine4D::Rendering::Shader4D>();
    shader->loadFromSource(
        // Вершинный шейдер
        R"(
        #version 330 core
        layout (location = 0) in vec4 position;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out float WCoord;
        
        void main() {
            vec4 worldPos = model * position;
            WCoord = worldPos.w;
            
            vec4 viewPos = view * worldPos;
            vec4 clipPos = projection * viewPos;
            
            gl_Position = clipPos;
        }
        )",
        
        // Фрагментный шейдер
        R"(
        #version 330 core
        in float WCoord;
        out vec4 FragColor;
        
        void main() {
            vec4 color = vec4(1.0, 0.5, 0.0, 1.0);
            
            // Эффект прозрачности для далеких W-слоев
            float wDistance = abs(WCoord);
            float alpha = 1.0 - smoothstep(0.0, 2.0, wDistance);
            color.a *= alpha;
            
            // Приглушение цвета для далеких слоев
            if (wDistance > 0.5) {
                color.rgb *= 0.7;
            }
            
            FragColor = color;
        }
        )"
    );
}
```

## Система частиц

### Создание 4D системы частиц

```cpp
void createParticleSystem() {
    // Создание объекта для системы частиц
    auto* particleObj = Engine4D::Core::GameObject4D::create("ParticleSystem");
    
    // Добавление компонента системы частиц
    auto* particleComponent = particleObj->addComponent<Engine4D::Core::ParticleSystem4DComponent>();
    
    // Создание системы частиц
    auto particleSystem = std::make_shared<Engine4D::Physics::ParticleSystem4D>();
    
    // Настройка эмиттера
    particleSystem->setEmitter(
        Engine4D::Math::Vector4(0, 0, 0, 0), // Позиция
        Engine4D::Math::Vector4(0, 1, 0, 0)  // Начальная скорость
    );
    
    // Настройка гравитации
    particleSystem->setGravity(Engine4D::Math::Vector4(0, -2, 0, 0));
    
    // Настройка параметров частиц
    particleSystem->setEmissionRate(50.0f);
    particleSystem->setParticleLife(3.0f);
    particleSystem->setParticleSize(0.1f);
    
    // Настройка цветового градиента
    particleSystem->setColorRange(
        Engine4D::Math::Vector4(1, 1, 0, 1), // Желтый в начале
        Engine4D::Math::Vector4(1, 0, 0, 0)  // Красный в конце
    );
    
    particleComponent->setParticleSystem(particleSystem);
    particleComponent->setAutoPlay(true);
}
```

### Обновление системы частиц

```cpp
void updateParticleSystem(float deltaTime) {
    auto objects = Engine4D::Core::GameObject4D::findAllWithTag("ParticleSystem");
    
    for (auto* obj : objects) {
        auto* particleComponent = obj->getComponent<Engine4D::Core::ParticleSystem4DComponent>();
        if (particleComponent) {
            particleComponent->update(deltaTime);
        }
    }
}
```

## Продвинутые примеры

### Создание 4D лабиринта

```cpp
void create4DMaze() {
    // Создание стен лабиринта в 4D пространстве
    for (int x = -5; x <= 5; x += 2) {
        for (int y = -5; y <= 5; y += 2) {
            for (int z = -5; z <= 5; z += 2) {
                for (int w = -5; w <= 5; w += 2) {
                    // Случайно создаем стены
                    if (rand() % 3 == 0) {
                        createWall(Engine4D::Math::Vector4(x, y, z, w));
                    }
                }
            }
        }
    }
}

void createWall(const Engine4D::Math::Vector4& position) {
    auto* wall = Engine4D::Core::GameObject4D::createPrimitive("Cube");
    wall->transform->setPosition(position);
    wall->transform->setScale(Engine4D::Math::Vector4(1.8f, 1.8f, 1.8f, 1.8f));
    
    // Добавление коллайдера
    auto* collider = wall->addComponent<Engine4D::Core::Collider4DComponent>();
    auto boxCollider = std::make_shared<Engine4D::Physics::BoxCollider4D>(
        Engine4D::Math::Vector4(1.8f, 1.8f, 1.8f, 1.8f)
    );
    collider->setCollider(boxCollider);
    
    // Настройка рендеринга
    auto* renderer = wall->getComponent<Engine4D::Core::MeshRenderer4D>();
    if (renderer) {
        renderer->setColor(Engine4D::Math::Vector4(0.5f, 0.5f, 0.5f, 1.0f));
    }
}
```

### Создание 4D портала

```cpp
void create4DPortal() {
    auto* portal = Engine4D::Core::GameObject4D::create("4DPortal");
    
    // Создание визуального эффекта портала
    auto* renderer = portal->addComponent<Engine4D::Core::MeshRenderer4D>();
    auto portalMesh = Engine4D::Rendering::Mesh4D::createTesseract(1.0f);
    portalMesh.uploadToGPU();
    renderer->setMesh(std::make_shared<Engine4D::Rendering::Mesh4D>(portalMesh));
    
    // Создание специального шейдера для портала
    auto portalShader = std::make_shared<Engine4D::Rendering::Shader4D>();
    portalShader->loadFromSource(
        // Вершинный шейдер с эффектом искажения
        R"(
        #version 330 core
        layout (location = 0) in vec4 position;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform float time;
        
        out vec4 FragPos;
        out float WCoord;
        
        void main() {
            vec4 worldPos = model * position;
            WCoord = worldPos.w;
            
            // Эффект искажения времени
            float distortion = sin(time + worldPos.w * 2.0) * 0.1;
            worldPos.xyz += distortion;
            
            vec4 viewPos = view * worldPos;
            vec4 clipPos = projection * viewPos;
            
            gl_Position = clipPos;
            FragPos = worldPos;
        }
        )",
        
        // Фрагментный шейдер с эффектом портала
        R"(
        #version 330 core
        in vec4 FragPos;
        in float WCoord;
        uniform float time;
        
        out vec4 FragColor;
        
        void main() {
            // Создание эффекта "портала" с изменяющимися цветами
            vec3 color = vec3(
                sin(time + WCoord * 3.0) * 0.5 + 0.5,
                cos(time + WCoord * 2.0) * 0.5 + 0.5,
                sin(time + WCoord * 4.0) * 0.5 + 0.5
            );
            
            // Эффект прозрачности
            float alpha = 0.7 + sin(time + WCoord * 5.0) * 0.3;
            
            FragColor = vec4(color, alpha);
        }
        )"
    );
    
    renderer->setShader(portalShader);
    
    // Добавление триггера для телепортации
    auto* collider = portal->addComponent<Engine4D::Core::Collider4DComponent>();
    auto sphereCollider = std::make_shared<Engine4D::Physics::SphereCollider4D>(1.0f);
    collider->setCollider(sphereCollider);
    collider->setTrigger(true);
}
```

### Создание 4D анимации

```cpp
void create4DAnimation() {
    auto* animatedObject = Engine4D::Core::GameObject4D::create("Animated4DObject");
    
    // Создание тессеракта
    auto* renderer = animatedObject->addComponent<Engine4D::Core::MeshRenderer4D>();
    auto mesh = Engine4D::Rendering::Mesh4D::createTesseract(1.0f);
    mesh.uploadToGPU();
    renderer->setMesh(std::make_shared<Engine4D::Rendering::Mesh4D>(mesh));
    
    // Анимация в игровом цикле
    static float time = 0.0f;
    time += 0.016f; // Примерно 60 FPS
    
    // Поворот в различных плоскостях
    auto rotationXY = Engine4D::Math::Matrix4::rotationXY(time * 0.5f);
    auto rotationXW = Engine4D::Math::Matrix4::rotationXW(time * 0.3f);
    auto rotationZW = Engine4D::Math::Matrix4::rotationZW(time * 0.7f);
    
    // Комбинированный поворот
    auto combinedRotation = rotationXY * rotationXW * rotationZW;
    
    // Применение поворота
    animatedObject->transform->setRotation(
        Engine4D::Math::Quaternion4D::fromMatrix(combinedRotation)
    );
    
    // Пульсация масштаба
    float scale = 1.0f + sin(time * 2.0f) * 0.2f;
    animatedObject->transform->setScale(
        Engine4D::Math::Vector4(scale, scale, scale, scale)
    );
    
    // Движение в W-измерении
    float wOffset = sin(time * 0.8f) * 2.0f;
    auto position = animatedObject->transform->getPosition();
    position.w = wOffset;
    animatedObject->transform->setPosition(position);
}
```

### Создание 4D пользовательского интерфейса

```cpp
void create4DUI() {
    // Создание 4D панели интерфейса
    auto* uiPanel = Engine4D::Core::GameObject4D::create("4DUI");
    
    // Позиционирование в 4D пространстве
    uiPanel->transform->setPosition(Engine4D::Math::Vector4(0, 0, -2, 0));
    
    // Создание 4D кнопки
    auto* button = Engine4D::Core::GameObject4D::create("4DButton");
    button->transform->setPosition(Engine4D::Math::Vector4(0, 0, 0, 1));
    
    // Добавление коллайдера для взаимодействия
    auto* collider = button->addComponent<Engine4D::Core::Collider4DComponent>();
    auto boxCollider = std::make_shared<Engine4D::Physics::BoxCollider4D>(
        Engine4D::Math::Vector4(1, 0.5f, 0.1f, 0.1f)
    );
    collider->setCollider(boxCollider);
    collider->setTrigger(true);
    
    // Обработка клика по 4D кнопке
    auto& inputManager = Engine4D::Input::InputManager4D::getInstance();
    if (inputManager.isMouseJustPressed(Engine4D::Input::MouseButton4D::Left)) {
        // Raycast для определения попадания в 4D UI
        auto camera = findMainCamera();
        if (camera) {
            auto rayOrigin = camera->camera.position;
            auto rayDirection = camera->camera.target - camera->camera.position;
            
            auto hit = physicsWorld->raycast(rayOrigin, rayDirection, 100.0f);
            if (hit.hasHit && hit.collider == collider->getCollider()) {
                // Обработка клика по 4D кнопке
                handle4DButtonClick();
            }
        }
    }
}

void handle4DButtonClick() {
    std::cout << "4D кнопка нажата!" << std::endl;
    // Логика обработки клика
}
```

Эти примеры демонстрируют различные возможности 4D Game Engine и показывают, как создавать интерактивные 4D приложения с использованием движка.
