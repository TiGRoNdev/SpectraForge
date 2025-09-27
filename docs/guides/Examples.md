# Руководство по использованию HyperEngine

## Содержание

1. [Быстрый старт](#быстрый-старт)
2. [Создание 3D сцены](#создание-3d-сцены)
3. [Работа с компонентами](#работа-с-компонентами)
4. [Физическая симуляция](#физическая-симуляция)
5. [Система ввода](#система-ввода)
6. [Продвинутый рендеринг](#продвинутый-рендеринг)
7. [Оптимизация производительности](#оптимизация-производительности)
8. [UTF-8 консоль](#utf-8-консоль)
9. [Vulkan рендеринг](#vulkan-рендеринг)

---

## Быстрый старт

### Минимальное приложение

```cpp
#include <Engine3D/Engine3D.h>

int main() {
    using namespace Engine3D;
    
    // Инициализация UTF-8 консоли
    Core::Console::initialize();
    Core::Console::info("🚀 Запуск HyperEngine...");
    
    // Инициализация рендерера
    auto& renderer = Rendering::Renderer3D::getInstance();
    if (!renderer.initialize(800, 600)) {
        Core::Console::error("❌ Ошибка инициализации рендерера");
        return -1;
    }
    
    Core::Console::success("✅ Рендерер инициализирован успешно!");
    
    // Основной игровой цикл
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        renderer.beginFrame();
        renderer.clear();
        
        // Здесь ваш код рендеринга
        
        renderer.endFrame();
        glfwPollEvents();
    }
    
    renderer.cleanup();
    Core::Console::cleanup();
    return 0;
}
```

### Компиляция

```bash
# CMake
mkdir build && cd build
cmake .. -DBUILD_ENGINE_3D=ON -DBUILD_EXAMPLES=ON
make -j4

# Запуск
./Engine3D_Demo
```

---

## Создание 3D сцены

### Базовая сцена с объектами

```cpp
#include <Engine3D/Engine3D.h>

class BasicScene {
public:
    void initialize() {
        using namespace Engine3D;
        
        // Создание камеры
        setupCamera();
        
        // Создание источника света
        setupLighting();
        
        // Создание объектов сцены
        createCube();
        createSphere();
        createPlane();
        
        Core::Console::info("🎨 Сцена создана с " + std::to_string(gameObjects.size()) + " объектами");
    }
    
    void update(float deltaTime) {
        // Вращение куба
        if (cube) {
            auto transform = cube->getComponent<Core::Transform3D>();
            Math::Quaternion rotation = Math::Quaternion::fromEulerAngles(0, deltaTime * 50, 0);
            transform->rotate(rotation);
        }
        
        // Обновление всех объектов
        for (auto& obj : gameObjects) {
            obj->update(deltaTime);
        }
    }
    
    void render() {
        for (auto& obj : gameObjects) {
            obj->render();
        }
    }

private:
    std::shared_ptr<Core::GameObject3D> camera;
    std::shared_ptr<Core::GameObject3D> cube;
    std::shared_ptr<Core::GameObject3D> sphere;
    std::shared_ptr<Core::GameObject3D> plane;
    std::vector<std::shared_ptr<Core::GameObject3D>> gameObjects;
    
    void setupCamera() {
        camera = Core::GameObject3D::create("MainCamera");
        auto cameraComponent = camera->addComponent<Rendering::Camera3D>();
        
        // Настройка позиции камеры
        auto transform = camera->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(0, 2, -8));
        transform->lookAt(Math::Vector3(0, 0, 0), Math::Vector3(0, 1, 0));
        
        // Настройка проекции
        cameraComponent->setPerspective(45.0f, 800.0f/600.0f, 0.1f, 100.0f);
        
        // Установка как основную камеру
        auto& renderer = Rendering::Renderer3D::getInstance();
        renderer.setMainCamera(cameraComponent);
        
        gameObjects.push_back(camera);
    }
    
    void setupLighting() {
        auto& renderer = Rendering::Renderer3D::getInstance();
        
        // Направленный свет (солнце)
        Rendering::Renderer3D::Light sunLight;
        sunLight.type = Rendering::Renderer3D::Light::DIRECTIONAL;
        sunLight.direction = Math::Vector3(-0.5f, -1.0f, -0.3f).normalized();
        sunLight.color = Math::Vector3(1.0f, 0.95f, 0.8f);
        sunLight.intensity = 1.0f;
        renderer.addLight(sunLight);
        
        // Точечный свет
        Rendering::Renderer3D::Light pointLight;
        pointLight.type = Rendering::Renderer3D::Light::POINT;
        pointLight.position = Math::Vector3(2, 3, -2);
        pointLight.color = Math::Vector3(0.8f, 0.8f, 1.0f);
        pointLight.intensity = 0.5f;
        pointLight.range = 10.0f;
        renderer.addLight(pointLight);
    }
    
    void createCube() {
        cube = Core::GameObject3D::create("RotatingCube");
        
        // Добавление рендерера
        auto meshRenderer = cube->addComponent<Rendering::MeshRenderer3D>();
        auto mesh = Rendering::Mesh3D::createCube(1.0f);
        meshRenderer->setMesh(mesh);
        meshRenderer->setColor(Math::Vector3(0.8f, 0.3f, 0.2f));
        
        // Позиционирование
        auto transform = cube->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(-2, 1, 0));
        
        gameObjects.push_back(cube);
    }
    
    void createSphere() {
        sphere = Core::GameObject3D::create("Sphere");
        
        auto meshRenderer = sphere->addComponent<Rendering::MeshRenderer3D>();
        auto mesh = Rendering::Mesh3D::createSphere(1.0f, 32);
        meshRenderer->setMesh(mesh);
        meshRenderer->setColor(Math::Vector3(0.2f, 0.8f, 0.3f));
        
        auto transform = sphere->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(2, 1, 0));
        
        gameObjects.push_back(sphere);
    }
    
    void createPlane() {
        plane = Core::GameObject3D::create("Ground");
        
        auto meshRenderer = plane->addComponent<Rendering::MeshRenderer3D>();
        auto mesh = Rendering::Mesh3D::createPlane(10.0f, 10.0f);
        meshRenderer->setMesh(mesh);
        meshRenderer->setColor(Math::Vector3(0.7f, 0.7f, 0.7f));
        
        auto transform = plane->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(0, 0, 0));
        
        gameObjects.push_back(plane);
    }
};
```

---

## Работа с компонентами

### Создание пользовательского компонента

```cpp
class RotatorComponent : public Engine3D::Core::Component {
public:
    RotatorComponent(const Math::Vector3& axis = Math::Vector3(0, 1, 0), float speed = 90.0f)
        : rotationAxis(axis.normalized()), rotationSpeed(speed) {}
    
    void start() override {
        transform = getGameObject()->getComponent<Core::Transform3D>();
        if (!transform) {
            Core::Console::warning("⚠️ RotatorComponent: GameObject не имеет Transform3D");
        }
    }
    
    void update(float deltaTime) override {
        if (transform && isEnabled()) {
            float angle = rotationSpeed * deltaTime;
            Math::Quaternion rotation = Math::Quaternion::fromAxisAngle(rotationAxis, Math::radians(angle));
            transform->rotate(rotation);
        }
    }
    
    // Настройки
    void setRotationAxis(const Math::Vector3& axis) { rotationAxis = axis.normalized(); }
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    
private:
    Math::Vector3 rotationAxis;
    float rotationSpeed;
    Core::Transform3D* transform = nullptr;
};

// Использование
auto gameObject = Core::GameObject3D::create("RotatingObject");
auto rotator = gameObject->addComponent<RotatorComponent>();
rotator->setRotationAxis(Math::Vector3(1, 1, 0));
rotator->setRotationSpeed(120.0f);
```

### Компонент осциллятора

```cpp
class OscillatorComponent : public Engine3D::Core::Component {
public:
    OscillatorComponent(const Math::Vector3& amplitude = Math::Vector3(0, 1, 0), float frequency = 1.0f)
        : amplitude(amplitude), frequency(frequency), time(0.0f) {}
    
    void start() override {
        transform = getGameObject()->getComponent<Core::Transform3D>();
        startPosition = transform ? transform->getPosition() : Math::Vector3::zero();
    }
    
    void update(float deltaTime) override {
        if (transform && isEnabled()) {
            time += deltaTime;
            Math::Vector3 offset = amplitude * std::sin(time * frequency * 2.0f * Math::PI);
            transform->setPosition(startPosition + offset);
        }
    }

private:
    Math::Vector3 amplitude;
    float frequency;
    float time;
    Math::Vector3 startPosition;
    Core::Transform3D* transform = nullptr;
};
```

---

## Физическая симуляция

### Падающие объекты

```cpp
class PhysicsDemo {
public:
    void initialize() {
        // Создание физического мира
        physicsWorld = std::make_unique<Physics::PhysicsWorld3D>();
        physicsWorld->setGravity(Math::Vector3(0, -9.81f, 0));
        
        // Создание земли
        createGround();
        
        // Создание падающих объектов
        createFallingObjects();
    }
    
    void update(float deltaTime) {
        // Обновление физики
        physicsWorld->update(deltaTime);
        
        // Создание новых объектов через интервалы
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnFallingObject();
            spawnTimer = 0.0f;
        }
        
        // Обновление объектов
        for (auto& obj : gameObjects) {
            obj->update(deltaTime);
        }
    }

private:
    std::unique_ptr<Physics::PhysicsWorld3D> physicsWorld;
    std::vector<std::shared_ptr<Core::GameObject3D>> gameObjects;
    float spawnTimer = 0.0f;
    float spawnInterval = 2.0f;
    
    void createGround() {
        auto ground = Core::GameObject3D::create("Ground");
        
        // Визуальная часть
        auto meshRenderer = ground->addComponent<Rendering::MeshRenderer3D>();
        auto mesh = Rendering::Mesh3D::createPlane(20.0f, 20.0f);
        meshRenderer->setMesh(mesh);
        meshRenderer->setColor(Math::Vector3(0.5f, 0.8f, 0.3f));
        
        // Физическая часть
        auto rigidBody = ground->addComponent<Physics::RigidBody3D>();
        rigidBody->setMass(0.0f); // Статическое тело
        
        auto collider = ground->addComponent<Physics::BoxCollider3D>();
        collider->setSize(Math::Vector3(20, 0.1f, 20));
        
        // Позиционирование
        auto transform = ground->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(0, -0.5f, 0));
        
        // Добавление в физический мир
        physicsWorld->addBody(rigidBody);
        physicsWorld->addCollider(collider);
        
        gameObjects.push_back(ground);
    }
    
    void createFallingObjects() {
        for (int i = 0; i < 5; ++i) {
            spawnFallingObject();
        }
    }
    
    void spawnFallingObject() {
        auto object = Core::GameObject3D::create("FallingObject");
        
        // Случайная позиция
        float x = (rand() % 200 - 100) / 10.0f; // -10 до 10
        float z = (rand() % 200 - 100) / 10.0f;
        
        // Визуальная часть
        auto meshRenderer = object->addComponent<Rendering::MeshRenderer3D>();
        
        // Случайная форма
        if (rand() % 2 == 0) {
            auto mesh = Rendering::Mesh3D::createCube(1.0f);
            meshRenderer->setMesh(mesh);
            meshRenderer->setColor(Math::Vector3(0.8f, 0.2f, 0.2f));
        } else {
            auto mesh = Rendering::Mesh3D::createSphere(0.5f, 16);
            meshRenderer->setMesh(mesh);
            meshRenderer->setColor(Math::Vector3(0.2f, 0.2f, 0.8f));
        }
        
        // Физическая часть
        auto rigidBody = object->addComponent<Physics::RigidBody3D>();
        rigidBody->setMass(1.0f);
        rigidBody->setRestitution(0.6f); // Упругость
        rigidBody->setFriction(0.5f);
        
        auto collider = object->addComponent<Physics::BoxCollider3D>();
        collider->setSize(Math::Vector3(1, 1, 1));
        
        // Позиционирование
        auto transform = object->getComponent<Core::Transform3D>();
        transform->setPosition(Math::Vector3(x, 10, z));
        
        // Случайный импульс
        Math::Vector3 impulse(
            (rand() % 100 - 50) / 25.0f,
            0,
            (rand() % 100 - 50) / 25.0f
        );
        rigidBody->applyImpulse(impulse);
        
        // Добавление в физический мир
        physicsWorld->addBody(rigidBody);
        physicsWorld->addCollider(collider);
        
        gameObjects.push_back(object);
    }
};
```

---

## Система ввода

### Контроллер от первого лица

```cpp
class FirstPersonController : public Engine3D::Core::Component {
public:
    FirstPersonController(float moveSpeed = 5.0f, float mouseSensitivity = 0.002f)
        : moveSpeed(moveSpeed), mouseSensitivity(mouseSensitivity), yaw(0), pitch(0) {}
    
    void start() override {
        transform = getGameObject()->getComponent<Core::Transform3D>();
        camera = getGameObject()->getComponent<Rendering::Camera3D>();
        
        // Блокировка курсора
        auto& input = Input::Input3D::getInstance();
        input.setCursorLocked(true);
        input.setCursorVisible(false);
    }
    
    void update(float deltaTime) override {
        if (!isEnabled()) return;
        
        handleMouseLook();
        handleMovement(deltaTime);
        handleActions();
    }

private:
    Core::Transform3D* transform = nullptr;
    Rendering::Camera3D* camera = nullptr;
    float moveSpeed;
    float mouseSensitivity;
    float yaw, pitch;
    
    void handleMouseLook() {
        auto& input = Input::Input3D::getInstance();
        Math::Vector3 mouseDelta = input.getMouseDelta();
        
        yaw += mouseDelta.x * mouseSensitivity;
        pitch -= mouseDelta.y * mouseSensitivity;
        
        // Ограничение поворота по вертикали
        pitch = Math::clamp(pitch, -Math::PI/2 + 0.01f, Math::PI/2 - 0.01f);
        
        // Применение вращения
        Math::Quaternion rotation = Math::Quaternion::fromEulerAngles(pitch, yaw, 0);
        transform->setRotation(rotation);
    }
    
    void handleMovement(float deltaTime) {
        auto& input = Input::Input3D::getInstance();
        Math::Vector3 movement(0);
        
        // WASD движение
        if (input.isKeyPressed(Input::KeyCode::W)) movement += transform->forward();
        if (input.isKeyPressed(Input::KeyCode::S)) movement -= transform->forward();
        if (input.isKeyPressed(Input::KeyCode::A)) movement -= transform->right();
        if (input.isKeyPressed(Input::KeyCode::D)) movement += transform->right();
        
        // Движение вверх/вниз
        if (input.isKeyPressed(Input::KeyCode::Space)) movement += Math::Vector3::up();
        if (input.isKeyPressed(Input::KeyCode::LeftShift)) movement -= Math::Vector3::up();
        
        // Нормализация и применение скорости
        if (movement.magnitude() > 0) {
            movement = movement.normalized() * moveSpeed * deltaTime;
            transform->translate(movement);
        }
    }
    
    void handleActions() {
        auto& input = Input::Input3D::getInstance();
        
        // Выход из режима мыши
        if (input.isKeyJustPressed(Input::KeyCode::Escape)) {
            input.setCursorLocked(false);
            input.setCursorVisible(true);
        }
        
        // Блокировка мыши при клике
        if (input.isMouseJustPressed(Input::MouseButton::Left)) {
            input.setCursorLocked(true);
            input.setCursorVisible(false);
        }
    }
};

// Использование
auto playerCamera = Core::GameObject3D::create("PlayerCamera");
auto camera = playerCamera->addComponent<Rendering::Camera3D>();
auto controller = playerCamera->addComponent<FirstPersonController>();
controller->setMoveSpeed(8.0f);
controller->setMouseSensitivity(0.003f);
```

---

## Продвинутый рендеринг

### Использование OptimalRenderer3D

```cpp
class AdvancedRenderingDemo {
public:
    void initialize() {
        // Создание оптимального рендерера
        setupOptimalRenderer();
        
        // Создание сложной сцены
        setupComplexScene();
        
        Core::Console::info("🎨 Продвинутый рендеринг инициализирован");
    }
    
    void render() {
        // Подготовка данных сцены
        SceneData sceneData = prepareSceneData();
        CameraParams cameraParams = prepareCameraParams();
        HardwareConfig hardwareConfig = detectHardwareConfig();
        
        // Рендеринг с оптимальным алгоритмом
        optimalRenderer->renderOptimal3D(sceneData, cameraParams, hardwareConfig);
        
        // Показ метрик производительности
        displayPerformanceMetrics();
    }

private:
    std::unique_ptr<Rendering::OptimalRenderer3D> optimalRenderer;
    std::vector<std::shared_ptr<Core::GameObject3D>> complexObjects;
    
    void setupOptimalRenderer() {
        optimalRenderer = std::make_unique<Rendering::OptimalRenderer3D>();
        
        // Настройка качества
        Rendering::QualitySettings quality;
        quality.gaussianSplatting = true;
        quality.rayTracedGI = true;
        quality.rayTracedReflections = true;
        quality.aiDenoising = true;
        quality.neuralUpscaling = true;
        optimalRenderer->setQualitySettings(quality);
        
        // Включение профилирования
        optimalRenderer->enableProfiling(true);
        
        if (!optimalRenderer->initialize(1920, 1080, 2560, 1440)) {
            Core::Console::error("❌ Ошибка инициализации OptimalRenderer3D");
        }
    }
    
    void setupComplexScene() {
        // Создание множества объектов для стресс-теста
        for (int i = 0; i < 100; ++i) {
            auto object = createComplexObject(i);
            complexObjects.push_back(object);
        }
    }
    
    std::shared_ptr<Core::GameObject3D> createComplexObject(int index) {
        auto object = Core::GameObject3D::create("ComplexObject_" + std::to_string(index));
        
        // Добавление различных компонентов
        auto meshRenderer = object->addComponent<Rendering::MeshRenderer3D>();
        
        // Случайная геометрия
        std::shared_ptr<Rendering::Mesh3D> mesh;
        switch (index % 3) {
            case 0: mesh = Rendering::Mesh3D::createCube(1.0f); break;
            case 1: mesh = Rendering::Mesh3D::createSphere(0.8f, 32); break;
            case 2: mesh = Rendering::Mesh3D::createCylinder(0.5f, 2.0f, 16); break;
        }
        
        meshRenderer->setMesh(mesh);
        
        // Случайный цвет
        Math::Vector3 color(
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f
        );
        meshRenderer->setColor(color);
        
        // Случайная позиция
        auto transform = object->getComponent<Core::Transform3D>();
        Math::Vector3 position(
            (rand() % 200 - 100) / 10.0f,
            (rand() % 100) / 10.0f,
            (rand() % 200 - 100) / 10.0f
        );
        transform->setPosition(position);
        
        // Добавление аниматора
        auto rotator = object->addComponent<RotatorComponent>();
        rotator->setRotationSpeed((rand() % 180) + 10);
        
        return object;
    }
    
    SceneData prepareSceneData() {
        SceneData data;
        
        // Конвертация GameObject в данные сцены
        for (const auto& obj : complexObjects) {
            // Добавление мешей и трансформаций
            auto meshRenderer = obj->getComponent<Rendering::MeshRenderer3D>();
            auto transform = obj->getComponent<Core::Transform3D>();
            
            if (meshRenderer && transform) {
                data.meshes.push_back(meshRenderer->getMesh());
                data.transforms.push_back(transform->getWorldMatrix());
                data.materials.push_back(meshRenderer->getMaterial());
            }
        }
        
        return data;
    }
    
    void displayPerformanceMetrics() {
        if (optimalRenderer->isProfilingEnabled()) {
            auto metrics = optimalRenderer->getPerformanceMetrics();
            
            Core::Console::info("📊 Метрики производительности:");
            Core::Console::info("  🎯 Scene Optimization: " + std::to_string(metrics.sceneOptimizationTime) + "ms");
            Core::Console::info("  🖼️ Rasterization: " + std::to_string(metrics.rasterizationTime) + "ms");
            Core::Console::info("  ⚡ Ray Tracing: " + std::to_string(metrics.rayTracingTime) + "ms");
            Core::Console::info("  🧠 Denoising: " + std::to_string(metrics.denoisingTime) + "ms");
            Core::Console::info("  📈 Upscaling: " + std::to_string(metrics.upscalingTime) + "ms");
            Core::Console::info("  🕒 Total: " + std::to_string(metrics.totalFrameTime) + "ms");
        }
    }
};
```

---

## Оптимизация производительности

### Frustum Culling

```cpp
class PerformanceOptimizer {
public:
    static void enableOptimizations(Rendering::Renderer3D& renderer) {
        // Включение отсечения по видимости
        renderer.enableFrustumCulling(true);
        
        // Включение batch rendering
        renderer.enableBatchRendering(true);
        
        // Настройка LOD (Level of Detail)
        renderer.setLODDistances(10.0f, 50.0f, 100.0f);
        
        Core::Console::info("🚀 Оптимизации производительности включены");
    }
    
    static void optimizeScene(std::vector<std::shared_ptr<Core::GameObject3D>>& objects) {
        // Сортировка по материалам для batch rendering
        std::sort(objects.begin(), objects.end(), [](const auto& a, const auto& b) {
            auto rendererA = a->getComponent<Rendering::MeshRenderer3D>();
            auto rendererB = b->getComponent<Rendering::MeshRenderer3D>();
            
            if (!rendererA || !rendererB) return false;
            
            return rendererA->getMaterial()->getId() < rendererB->getMaterial()->getId();
        });
        
        Core::Console::info("✨ Сцена оптимизирована для batch rendering");
    }
};

// Профайлер производительности
class PerformanceProfiler {
public:
    void beginFrame() {
        frameStartTime = std::chrono::high_resolution_clock::now();
    }
    
    void endFrame() {
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(
            frameEndTime - frameStartTime).count() / 1000.0f;
        
        frameTimes.push_back(frameDuration);
        
        // Сохраняем только последние 60 кадров
        if (frameTimes.size() > 60) {
            frameTimes.erase(frameTimes.begin());
        }
        
        // Обновление статистики каждую секунду
        frameCount++;
        if (frameCount % 60 == 0) {
            calculateStatistics();
            displayStatistics();
        }
    }

private:
    std::chrono::high_resolution_clock::time_point frameStartTime;
    std::vector<float> frameTimes;
    int frameCount = 0;
    float avgFrameTime = 0;
    float minFrameTime = 0;
    float maxFrameTime = 0;
    
    void calculateStatistics() {
        if (frameTimes.empty()) return;
        
        float sum = 0;
        minFrameTime = frameTimes[0];
        maxFrameTime = frameTimes[0];
        
        for (float time : frameTimes) {
            sum += time;
            minFrameTime = std::min(minFrameTime, time);
            maxFrameTime = std::max(maxFrameTime, time);
        }
        
        avgFrameTime = sum / frameTimes.size();
    }
    
    void displayStatistics() {
        float avgFPS = 1000.0f / avgFrameTime;
        float minFPS = 1000.0f / maxFrameTime;
        float maxFPS = 1000.0f / minFrameTime;
        
        Core::Console::info("📊 FPS: " + std::to_string(avgFPS) + 
                           " (min: " + std::to_string(minFPS) + 
                           ", max: " + std::to_string(maxFPS) + ")");
    }
};
```

---

## UTF-8 консоль

### Продвинутое использование консоли

```cpp
class ConsoleDemo {
public:
    static void demonstrateConsole() {
        using namespace Engine3D::Core;
        
        // Инициализация
        Console::initialize();
        Console::setTitle("🎮 HyperEngine - UTF-8 Console Demo");
        
        // Различные типы сообщений
        Console::info("ℹ️ Информационное сообщение");
        Console::success("✅ Операция выполнена успешно!");
        Console::warning("⚠️ Предупреждение о низкой производительности");
        Console::error("❌ Критическая ошибка в системе");
        Console::debug("🐛 Отладочная информация");
        
        // Прогресс бар
        showProgressBar();
        
        // Таблица с данными
        showDataTable();
        
        // Интерактивное меню
        showInteractiveMenu();
        
        Console::cleanup();
    }

private:
    static void showProgressBar() {
        Console::info("📊 Загрузка ресурсов...");
        
        for (int i = 0; i <= 100; i += 10) {
            Console::setCursorPosition(0, Console::getCursorY());
            
            std::string bar = "▓";
            int filled = i / 10;
            int empty = 10 - filled;
            
            std::string progress = "🔄 [";
            progress += std::string(filled, '█');
            progress += std::string(empty, '░');
            progress += "] " + std::to_string(i) + "%";
            
            Console::print(progress, ConsoleColor::Cyan);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        Console::println("");
        Console::success("✅ Ресурсы загружены!");
    }
    
    static void showDataTable() {
        Console::info("📋 Статистика производительности:");
        Console::println("┌─────────────────┬──────────┬──────────┐", ConsoleColor::White);
        Console::println("│ Компонент       │ Время    │ Статус   │", ConsoleColor::White);
        Console::println("├─────────────────┼──────────┼──────────┤", ConsoleColor::White);
        Console::println("│ 🎨 Рендеринг    │ 16.7ms   │ ✅ OK    │", ConsoleColor::Green);
        Console::println("│ ⚡ Физика       │ 2.3ms    │ ✅ OK    │", ConsoleColor::Green);
        Console::println("│ 🎮 Ввод         │ 0.5ms    │ ✅ OK    │", ConsoleColor::Green);
        Console::println("│ 🔊 Звук         │ 1.2ms    │ ⚠️ Slow  │", ConsoleColor::Yellow);
        Console::println("└─────────────────┴──────────┴──────────┘", ConsoleColor::White);
    }
    
    static void showInteractiveMenu() {
        Console::info("🎛️ Интерактивное меню:");
        Console::println("1️⃣ Запустить 3D демо");
        Console::println("2️⃣ Тест производительности");
        Console::println("3️⃣ Настройки графики");
        Console::println("4️⃣ Выход");
        
        Console::print("Выберите опцию (1-4): ", ConsoleColor::Cyan);
        
        // Здесь можно добавить чтение ввода пользователя
        // int choice = Console::readInt();
        // handleMenuChoice(choice);
    }
};
```

---

## Vulkan рендеринг

### Использование VulkanRenderer

```cpp
class VulkanDemo {
public:
    void initialize() {
        // Обнаружение аппаратных возможностей
        detectHardware();
        
        // Инициализация Vulkan рендерера
        setupVulkanRenderer();
        
        // Создание сцены для Vulkan
        setupVulkanScene();
    }
    
    void render() {
        if (!vulkanRenderer) return;
        
        // Подготовка данных
        SceneData sceneData = prepareVulkanSceneData();
        CameraParams cameraParams = prepareVulkanCameraParams();
        
        // Рендеринг с Vulkan
        vulkanRenderer->renderFrame(sceneData, cameraParams);
    }

private:
    std::unique_ptr<VulkanRenderer> vulkanRenderer;
    HardwareConfig hardwareConfig;
    std::vector<std::shared_ptr<Rendering::Mesh3D>> vulkanMeshes;
    
    void detectHardware() {
        using namespace Engine3D::Vulkan;
        
        auto vendor = HardwareDetector::detectGPUVendor();
        bool rayTracingSupport = HardwareDetector::supportsRayTracing();
        bool dlssSupport = HardwareDetector::supportsDLSS();
        bool fsrSupport = HardwareDetector::supportsFSR();
        
        Core::Console::info("🖥️ Обнаружено оборудование:");
        Core::Console::info("  GPU Vendor: " + vendorToString(vendor));
        Core::Console::info("  Ray Tracing: " + (rayTracingSupport ? "✅" : "❌"));
        Core::Console::info("  DLSS: " + (dlssSupport ? "✅" : "❌"));
        Core::Console::info("  FSR: " + (fsrSupport ? "✅" : "❌"));
        
        // Настройка конфигурации
        hardwareConfig.vendor = vendor;
        hardwareConfig.supportsRayTracing = rayTracingSupport;
        hardwareConfig.supportsDLSS = dlssSupport;
        hardwareConfig.supportsFSR = fsrSupport;
        hardwareConfig.supportsNeural = dlssSupport || fsrSupport;
    }
    
    void setupVulkanRenderer() {
        vulkanRenderer = std::make_unique<VulkanRenderer>();
        
        if (!vulkanRenderer->initialize(1920, 1080)) {
            Core::Console::error("❌ Ошибка инициализации Vulkan рендерера");
            return;
        }
        
        // Настройка аппаратной конфигурации
        vulkanRenderer->setHardwareConfig(hardwareConfig);
        
        // Включение дополнительных возможностей
        if (hardwareConfig.supportsRayTracing) {
            vulkanRenderer->enableRayTracing(true);
            Core::Console::success("✅ Ray Tracing включен");
        }
        
        if (hardwareConfig.supportsDLSS) {
            vulkanRenderer->enableDLSS(true);
            Core::Console::success("✅ DLSS включен");
        } else if (hardwareConfig.supportsFSR) {
            vulkanRenderer->enableFSR(true);
            Core::Console::success("✅ FSR включен");
        }
        
        Core::Console::success("🚀 Vulkan рендерер инициализирован");
    }
    
    void setupVulkanScene() {
        // Создание высокополигональных мешей для демонстрации Vulkan
        auto highDetailSphere = Rendering::Mesh3D::createSphere(2.0f, 128);
        auto complexGeometry = createComplexGeometry();
        
        vulkanMeshes.push_back(highDetailSphere);
        vulkanMeshes.push_back(complexGeometry);
        
        Core::Console::info("🎨 Vulkan сцена создана с " + 
                           std::to_string(vulkanMeshes.size()) + " высокодетализированными мешами");
    }
    
    std::shared_ptr<Rendering::Mesh3D> createComplexGeometry() {
        // Создание сложной геометрии для тестирования производительности Vulkan
        auto mesh = std::make_shared<Rendering::Mesh3D>();
        
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        
        // Генерация сложной геометрии (например, фрактальной поверхности)
        generateFractalSurface(vertices, indices, 5); // 5 уровней детализации
        
        mesh->setVertices(vertices);
        mesh->setIndices(indices);
        mesh->uploadToGPU();
        
        return mesh;
    }
    
    void generateFractalSurface(std::vector<Vertex>& vertices, 
                               std::vector<unsigned int>& indices, 
                               int detail) {
        // Реализация генерации фрактальной поверхности
        // Это демонстрационный код - реальная реализация будет более сложной
        
        float scale = 1.0f;
        int resolution = std::pow(2, detail);
        
        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                float fx = (x / float(resolution - 1)) * 2.0f - 1.0f;
                float fy = (y / float(resolution - 1)) * 2.0f - 1.0f;
                
                // Простая фрактальная функция
                float height = 0;
                float amplitude = 1.0f;
                float frequency = 1.0f;
                
                for (int i = 0; i < detail; ++i) {
                    height += amplitude * std::sin(fx * frequency) * std::cos(fy * frequency);
                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }
                
                Vertex vertex;
                vertex.position = Math::Vector3(fx * scale, height * 0.5f, fy * scale);
                vertex.normal = Math::Vector3(0, 1, 0); // Упрощено
                vertex.texCoord = Math::Vector2((x / float(resolution - 1)), (y / float(resolution - 1)));
                
                vertices.push_back(vertex);
            }
        }
        
        // Генерация индексов для треугольников
        for (int y = 0; y < resolution - 1; ++y) {
            for (int x = 0; x < resolution - 1; ++x) {
                int i = y * resolution + x;
                
                // Первый треугольник
                indices.push_back(i);
                indices.push_back(i + 1);
                indices.push_back(i + resolution);
                
                // Второй треугольник
                indices.push_back(i + 1);
                indices.push_back(i + resolution + 1);
                indices.push_back(i + resolution);
            }
        }
    }
    
    std::string vendorToString(VendorType vendor) {
        switch (vendor) {
            case VendorType::NVIDIA: return "NVIDIA";
            case VendorType::AMD: return "AMD";
            case VendorType::INTEL: return "Intel";
            default: return "Unknown";
        }
    }
};
```

---

## Заключение

Это руководство демонстрирует основные возможности HyperEngine и показывает, как эффективно использовать его компоненты для создания современных 3D приложений. 

### Ключевые принципы:

1. **Модульность**: Используйте компонентную архитектуру для гибкости
2. **Производительность**: Применяйте оптимизации и профилирование
3. **Современность**: Используйте продвинутые техники рендеринга
4. **Удобство**: Используйте UTF-8 консоль для лучшего опыта разработки

### Дальнейшее изучение:

- [API Reference](../api/API_Reference.md) - Полная справочная документация
- [Architecture](../architecture/ARCHITECTURE.md) - Детальное описание архитектуры
- [Examples](../../examples/) - Дополнительные примеры кода

**HyperEngine** предоставляет все инструменты для создания современных игр и интерактивных 3D приложений с высокой производительностью и качеством.
