# API Reference - SpectraForge v1.0.0

## Содержание
### SpectraForge::App фасад (v0.1.0)

Высокоуровневый слой для быстрого запуска приложения без знания внутренних подсистем.

- `SpectraForge::App::IApp`
  - `bool init()` — инициализация окна/ядра/ресурсов
  - `bool load_scene(const Vulkan::SceneData& data)` — загрузка сцены
  - `void update(float delta_time)` — обработка событий и обновление
  - `void render()` — рендеринг кадра и `swapBuffers`
  - `void shutdown()` — корректное завершение работы

- `SpectraForge::App::Engine`
  - Реализация `IApp` поверх `Core::EngineCore`, использует `Core::Window`, `Rendering::IRenderer`, `Rendering::IResourceManager`, `Vulkan::SceneManager`

- `SpectraForge::App::AppConfig`
  - `window_width`, `window_height`, `window_title`, `vsync`, `debug`

Пример использования (псевдокод):

```cpp
using namespace SpectraForge;
App::AppConfig cfg{1280, 720, "Demo", true, false};
auto logger = std::make_shared<Core::Logger>();
auto renderer = createRenderer();              // конкретная реализация
auto resources = createResourceManager();      // конкретная реализация

App::Engine app(cfg, logger, renderer, resources);
if (app.init()) {
  Vulkan::SceneData scene{}; scene.scenePath = "examples/scenes/sponza/sponza.obj";
  app.load_scene(scene);
  for (;;) { app.update(0.016f); app.render(); }
}
app.shutdown();
```


1. [Математическая библиотека](#математическая-библиотека)
2. [Система рендеринга](#система-рендеринга)
3. [Физическая система](#физическая-система)
4. [Система ввода](#система-ввода)
5. [Система объектов](#система-объектов)
6. [Консольная система](#консольная-система)
7. [Vulkan подсистема](#vulkan-подсистема)
8. [CUDA интеграция](#cuda-интеграция)
9. [Система ресурсов](#система-ресурсов)
10. [Профилирование](#профилирование)

---

## Математическая библиотека

### Vector3

3-мерный вектор для работы с трехмерным пространством.

#### Конструкторы

```cpp
Vector3();                                    // [0, 0, 0]
Vector3(float x, float y, float z);          // [x, y, z]
Vector3(const Vector3& other);               // Копирование
```

#### Операторы

```cpp
Vector3 operator+(const Vector3& other) const;
Vector3 operator-(const Vector3& other) const;
Vector3 operator*(float scalar) const;
Vector3 operator/(float scalar) const;
Vector3& operator+=(const Vector3& other);
Vector3& operator-=(const Vector3& other);
Vector3& operator*=(float scalar);
Vector3& operator/=(float scalar);
bool operator==(const Vector3& other) const;
bool operator!=(const Vector3& other) const;
float& operator[](int index);
const float& operator[](int index) const;
```

#### Математические операции

```cpp
float dot(const Vector3& other) const;           // Скалярное произведение
Vector3 cross(const Vector3& other) const;       // Векторное произведение
float magnitude() const;                         // Длина вектора
float magnitudeSquared() const;                  // Квадрат длины
Vector3 normalized() const;                      // Нормализованный вектор
void normalize();                                // Нормализация на месте
float distance(const Vector3& other) const;      // Расстояние до другого вектора
Vector3 lerp(const Vector3& other, float t) const; // Линейная интерполяция
```

#### Статические методы

```cpp
static Vector3 zero();      // [0, 0, 0]
static Vector3 one();       // [1, 1, 1]
static Vector3 up();        // [0, 1, 0]
static Vector3 down();      // [0, -1, 0]
static Vector3 left();      // [-1, 0, 0]
static Vector3 right();     // [1, 0, 0]
static Vector3 forward();   // [0, 0, 1]
static Vector3 back();      // [0, 0, -1]
```

### Matrix4

4x4 матрица для трансформаций в 3D пространстве.

#### Конструкторы

```cpp
Matrix4();                                    // Единичная матрица
Matrix4(const Matrix4& other);               // Копирование
explicit Matrix4(float diagonal);            // Диагональная матрица
```

#### Операторы

```cpp
Matrix4 operator+(const Matrix4& other) const;
Matrix4 operator-(const Matrix4& other) const;
Matrix4 operator*(const Matrix4& other) const;
Matrix4 operator*(float scalar) const;
Vector3 operator*(const Vector3& vec) const;
Matrix4& operator+=(const Matrix4& other);
Matrix4& operator-=(const Matrix4& other);
Matrix4& operator*=(const Matrix4& other);
Matrix4& operator*=(float scalar);
bool operator==(const Matrix4& other) const;
bool operator!=(const Matrix4& other) const;
```

#### Основные операции

```cpp
Matrix4 transpose() const;                       // Транспонирование
float determinant() const;                       // Определитель
Matrix4 inverse() const;                         // Обратная матрица
bool isInvertible() const;                       // Проверка обратимости
void setIdentity();                              // Установка единичной матрицы
```

#### Трансформации

```cpp
static Matrix4 translation(const Vector3& translation);
static Matrix4 translation(float x, float y, float z);
static Matrix4 scaling(const Vector3& scale);
static Matrix4 scaling(float x, float y, float z);
static Matrix4 scaling(float uniformScale);
static Matrix4 rotationX(float angle);
static Matrix4 rotationY(float angle);
static Matrix4 rotationZ(float angle);
static Matrix4 rotation(const Vector3& axis, float angle);
```

#### Проекции

```cpp
static Matrix4 perspective(float fovy, float aspect, float near, float far);
static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
```

### Quaternion

Кватернион для представления поворотов в 3D пространстве.

#### Конструкторы

```cpp
Quaternion();                                 // Единичный кватернион
Quaternion(float w, float x, float y, float z);
Quaternion(const Quaternion& other);         // Копирование
explicit Quaternion(const Vector3& axis, float angle);
```

#### Операторы

```cpp
Quaternion operator+(const Quaternion& other) const;
Quaternion operator-(const Quaternion& other) const;
Quaternion operator*(const Quaternion& other) const;
Quaternion operator*(float scalar) const;
Quaternion& operator+=(const Quaternion& other);
Quaternion& operator-=(const Quaternion& other);
Quaternion& operator*=(const Quaternion& other);
Quaternion& operator*=(float scalar);
bool operator==(const Quaternion& other) const;
bool operator!=(const Quaternion& other) const;
```

#### Основные операции

```cpp
float magnitude() const;                         // Длина кватерниона
float magnitudeSquared() const;                  // Квадрат длины
Quaternion normalized() const;                   // Нормализованный кватернион
void normalize();                                // Нормализация на месте
Quaternion conjugate() const;                    // Сопряженный кватернион
Quaternion inverse() const;                      // Обратный кватернион
float dot(const Quaternion& other) const;       // Скалярное произведение
```

#### Повороты

```cpp
Vector3 rotate(const Vector3& vec) const;        // Поворот вектора
Matrix4 toMatrix() const;                        // Преобразование в матрицу
static Quaternion fromMatrix(const Matrix4& mat); // Создание из матрицы
static Quaternion fromEulerAngles(float x, float y, float z);
static Quaternion fromAxisAngle(const Vector3& axis, float angle);
```

#### Интерполяция

```cpp
Quaternion slerp(const Quaternion& other, float t) const; // Сферическая интерполяция
Quaternion lerp(const Quaternion& other, float t) const;  // Линейная интерполяция
```

---

## Система рендеринга

### Renderer3D

Основной класс рендерера для 3D.

#### Инициализация

```cpp
static Renderer3D& getInstance();
bool initialize(int width, int height);
void cleanup();
bool isInitialized() const;
```

#### Управление кадром

```cpp
void beginFrame();
void endFrame();
void clear();
```

#### Настройки рендеринга

```cpp
void setClearColor(float r, float g, float b, float a = 1.0f);
void setViewport(int x, int y, int width, int height);
void enableDepthTest(bool enable);
void enableBlending(bool enable);
void enableWireframe(bool enable);
void enableBackfaceCulling(bool enable);
```

#### Управление камерой

```cpp
void setMainCamera(std::shared_ptr<Camera3D> camera);
std::shared_ptr<Camera3D> getMainCamera() const;
```

#### Рендеринг объектов

```cpp
void renderMesh(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader);
void renderMesh(std::shared_ptr<Mesh3D> mesh, const Matrix4& transform, std::shared_ptr<Shader3D> shader);
void renderWireframe(const Mesh3D& mesh, const Matrix4& transform, const Shader3D& shader);
```

### Camera3D

3D камера для рендеринга.

#### Конструкторы

```cpp
Camera3D();
```

#### Настройка позиции

```cpp
void setPosition(const Vector3& pos);
void setTarget(const Vector3& target);
void lookAt(const Vector3& pos, const Vector3& target, const Vector3& up);
```

#### Проекции

```cpp
void setPerspective(float fovy, float aspect, float near, float far);
void setOrthographic(float left, float right, float bottom, float top, float near, float far);
```

#### Получение матриц

```cpp
Matrix4 getViewMatrix() const;
Matrix4 getProjectionMatrix() const;
Matrix4 getViewProjectionMatrix() const;
```

#### Навигация

```cpp
void move(const Vector3& direction);
void rotate(float yaw, float pitch);
void zoom(float factor);
```

### Mesh3D

3D меш для хранения геометрии.

#### Конструкторы

```cpp
Mesh3D();
```

#### Управление вершинами

```cpp
void addVertex(const Vertex& vertex);
void addTriangle(unsigned int i1, unsigned int i2, unsigned int i3);
void setVertices(const std::vector<Vertex>& vertices);
void setIndices(const std::vector<unsigned int>& indices);
```

#### GPU операции

```cpp
void uploadToGPU();
void render() const;
void cleanup();
```

#### Создание примитивов

```cpp
static std::shared_ptr<Mesh3D> createCube(float size = 1.0f);
static std::shared_ptr<Mesh3D> createSphere(float radius = 1.0f, int segments = 16);
static std::shared_ptr<Mesh3D> createPlane(float width = 1.0f, float height = 1.0f);
static std::shared_ptr<Mesh3D> createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 16);
```

### Shader3D

Шейдер для рендеринга 3D объектов.

#### Конструкторы

```cpp
Shader3D();
```

#### Загрузка шейдеров

```cpp
bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
```

#### Использование

```cpp
void use() const;
void cleanup();
```

#### Униформы

```cpp
void setMatrix4(const std::string& name, const Matrix4& matrix) const;
void setVector3(const std::string& name, const Vector3& vector) const;
void setFloat(const std::string& name, float value) const;
void setInt(const std::string& name, int value) const;
void setBool(const std::string& name, bool value) const;
```

---

## Физическая система

### RigidBody3D

3D физическое тело.

#### Конструкторы

```cpp
RigidBody3D();
```

#### Настройка

```cpp
void setMass(float mass);
void setRestitution(float restitution);
void setFriction(float friction);
void setGravityScale(float scale);
```

#### Применение сил

```cpp
void applyForce(const Vector3& force);
void applyTorque(const Vector3& torque);
void applyImpulse(const Vector3& impulse);
void applyAngularImpulse(const Vector3& impulse);
```

#### Геттеры и сеттеры

```cpp
Vector3 getPosition() const;
void setPosition(const Vector3& pos);
Vector3 getVelocity() const;
void setVelocity(const Vector3& vel);
Vector3 getAngularVelocity() const;
void setAngularVelocity(const Vector3& angVel);
float getMass() const;
```

#### Обновление

```cpp
void update(float deltaTime);
```

### Collider3D

Базовый класс для 3D коллайдеров.

#### Типы коллайдеров

```cpp
enum class Type {
    Box,        // Прямоугольный
    Sphere,     // Сферический
    Capsule,    // Капсульный
    Mesh        // Произвольный меш
};
```

#### Основные методы

```cpp
virtual bool intersects(const Collider3D& other) const = 0;
virtual Vector3 getClosestPoint(const Vector3& point) const = 0;
virtual Vector3 getNormal(const Vector3& point) const = 0;
virtual float getVolume() const = 0;
```

#### Настройка

```cpp
void setCenter(const Vector3& center);
void setSize(const Vector3& size);
void setRadius(float radius);
```

### PhysicsWorld3D

3D физический мир.

#### Управление объектами

```cpp
void addBody(std::shared_ptr<RigidBody3D> body);
void removeBody(std::shared_ptr<RigidBody3D> body);
void addCollider(std::shared_ptr<Collider3D> collider);
void removeCollider(std::shared_ptr<Collider3D> collider);
```

#### Обновление

```cpp
void update(float deltaTime);
void step(float deltaTime);
```

#### Настройки

```cpp
void setGravity(const Vector3& gravity);
void setTimeStep(float step);
void clear();
```

---

## Система ввода

### Input3D

Система ввода для 3D движка.

#### Инициализация

```cpp
static Input3D& getInstance();
bool initialize(GLFWwindow* window);
void cleanup();
void update();
```

#### Проверка состояния клавиш

```cpp
bool isKeyPressed(KeyCode key) const;
bool isKeyJustPressed(KeyCode key) const;
bool isKeyJustReleased(KeyCode key) const;
```

#### Проверка состояния мыши

```cpp
bool isMousePressed(MouseButton button) const;
bool isMouseJustPressed(MouseButton button) const;
bool isMouseJustReleased(MouseButton button) const;
```

#### Данные мыши

```cpp
Vector3 getMousePosition() const;
Vector3 getMouseDelta() const;
Vector3 getScrollDelta() const;
```

#### Управление курсором

```cpp
void setCursorVisible(bool visible);
void setCursorLocked(bool locked);
void setCursorPosition(double x, double y);
```

### Controller3D

3D контроллер для навигации.

#### Конструкторы

```cpp
Controller3D();
```

#### Обновление

```cpp
void update(float deltaTime);
void handleInput(const InputState& inputState);
```

#### Движение

```cpp
void move(const Vector3& direction);
void moveForward(float amount);
void moveRight(float amount);
void moveUp(float amount);
```

#### Повороты

```cpp
void rotate(float yaw, float pitch);
void setRotation(float yaw, float pitch);
```

#### Настройки

```cpp
void setMoveSpeed(float speed);
void setRotationSpeed(float speed);
void setSensitivity(float sensitivity);
void setMouseLook(bool enabled);
```

---

## Система объектов

### GameObject3D

3D игровой объект.

#### Конструкторы

```cpp
GameObject3D(const std::string& name = "GameObject3D");
```

#### Управление компонентами

```cpp
template<typename T>
T* addComponent();
template<typename T>
T* getComponent();
template<typename T>
std::vector<T*> getComponents();
template<typename T>
void removeComponent();
```

#### Управление объектом

```cpp
void setActive(bool active);
bool isActive() const;
void destroy();
```

#### Обновление

```cpp
void start();
void update(float deltaTime);
void render();
void cleanup();
```

#### Поиск объектов

```cpp
static GameObject3D* find(const std::string& name);
static std::vector<GameObject3D*> findAllWithTag(const std::string& tag);
static GameObject3D* findWithTag(const std::string& tag);
```

#### Создание объектов

```cpp
static std::shared_ptr<GameObject3D> create(const std::string& name = "GameObject3D");
static std::shared_ptr<GameObject3D> createPrimitive(const std::string& type);
```

### Transform3D

Компонент трансформации для 3D объектов.

#### Конструкторы

```cpp
Transform3D();
```

#### Трансформации

```cpp
void setPosition(const Vector3& pos);
void setRotation(const Quaternion& rot);
void setScale(const Vector3& scl);
void translate(const Vector3& translation);
void rotate(const Quaternion& rotation);
void scaleBy(const Vector3& scale);
```

#### Получение матриц

```cpp
Matrix4 getLocalMatrix() const;
Matrix4 getWorldMatrix() const;
```

#### Мировые координаты

```cpp
Vector3 getWorldPosition() const;
Quaternion getWorldRotation() const;
Vector3 getWorldScale() const;
```

#### Направления

```cpp
Vector3 forward() const;
Vector3 right() const;
Vector3 up() const;
```

#### Иерархия

```cpp
void setParent(Transform3D* parent);
Transform3D* getParent() const;
void addChild(Transform3D* child);
void removeChild(Transform3D* child);
const std::vector<Transform3D*>& getChildren() const;
```

### Component

Базовый класс компонента.

#### Конструкторы

```cpp
Component();
```

#### Жизненный цикл

```cpp
virtual void start() {}
virtual void update(float deltaTime) {}
virtual void render() {}
virtual void cleanup() {}
```

#### Управление

```cpp
void setEnabled(bool enabled);
bool isEnabled() const;
GameObject3D* getGameObject() const;
```

---

## Консольная система

### Console

UTF-8 консольная система с поддержкой эмодзи и цветов.

#### Инициализация

```cpp
static void initialize();
static void cleanup();
static void setTitle(const std::string& title);
```

#### Цветной вывод

```cpp
static void info(const std::string& message);
static void warning(const std::string& message);
static void error(const std::string& message);
static void success(const std::string& message);
static void debug(const std::string& message);
```

#### Низкоуровневый вывод

```cpp
static void print(const std::string& message, ConsoleColor color = ConsoleColor::White);
static void println(const std::string& message, ConsoleColor color = ConsoleColor::White);
```

#### Управление курсором

```cpp
static void clearScreen();
static void setCursorPosition(int x, int y);
static void hideCursor();
static void showCursor();
```

---

## Vulkan подсистема

### VulkanRenderer

Vulkan-based рендерер для высокопроизводительного рендеринга.

#### Инициализация

```cpp
VulkanRenderer();
bool initialize(int width, int height);
void cleanup();
```

#### Рендеринг

```cpp
void beginFrame();
void endFrame();
void renderFrame(const SceneData& sceneData, const CameraParams& cameraParams);
```

#### Конфигурация

```cpp
void setHardwareConfig(const HardwareConfig& config);
void enableRayTracing(bool enable);
void enableDLSS(bool enable);
void enableFSR(bool enable);
```

### HardwareDetector

Обнаружение аппаратных возможностей.

#### Основные методы

```cpp
static VendorType detectGPUVendor();
static bool supportsRayTracing();
static bool supportsDLSS();
static bool supportsFSR();
static bool supportsCUDA();
static bool supportsOptiX();
```

#### Конфигурация

```cpp
static HardwareConfig detectOptimalConfig();
static UpscalerType selectBestUpscaler();
```

---

## Примеры использования

### Создание простой 3D сцены

```cpp
#include <Engine3D/Engine3D.h>

int main() {
    using namespace Engine3D;
    
    // Инициализация рендерера
    auto& renderer = Rendering::Renderer3D::getInstance();
    if (!renderer.initialize(1280, 720)) {
        return -1;
    }
    
    // Создание камеры
    auto cameraObj = Core::GameObject3D::create("Camera");
    auto camera = cameraObj->addComponent<Rendering::Camera3D>();
    camera->setPosition(Math::Vector3(0, 0, -5));
    renderer.setMainCamera(camera);
    
    // Создание куба
    auto cube = Core::GameObject3D::create("Cube");
    auto meshRenderer = cube->addComponent<Core::MeshRenderer3D>();
    auto mesh = Rendering::Mesh3D::createCube(1.0f);
    meshRenderer->setMesh(mesh);
    
    // Игровой цикл
    while (!renderer.shouldClose()) {
        renderer.beginFrame();
        
        // Обновление и рендеринг
        cube->update(0.016f);
        cube->render();
        
        renderer.endFrame();
    }
    
    renderer.cleanup();
    return 0;
}
```

### Физическая симуляция

```cpp
// Создание физического объекта
auto physicsObject = Core::GameObject3D::create("PhysicsBox");

// Добавление физического тела
auto rigidBody = physicsObject->addComponent<Physics::RigidBody3D>();
rigidBody->setMass(1.0f);
rigidBody->setRestitution(0.7f);

// Добавление коллайдера
auto collider = physicsObject->addComponent<Physics::BoxCollider3D>();
collider->setSize(Math::Vector3(1, 1, 1));

// Применение силы
rigidBody->applyImpulse(Math::Vector3(0, 10, 0));
```

### Система ввода

```cpp
void PlayerController::update(float deltaTime) {
    auto& input = Input::Input3D::getInstance();
    auto transform = player->getComponent<Core::Transform3D>();
    
    Math::Vector3 movement(0);
    
    if (input.isKeyPressed(Input::KeyCode::W)) movement.z += 1;
    if (input.isKeyPressed(Input::KeyCode::S)) movement.z -= 1;
    if (input.isKeyPressed(Input::KeyCode::A)) movement.x -= 1;
    if (input.isKeyPressed(Input::KeyCode::D)) movement.x += 1;
    
    movement = movement.normalized() * moveSpeed * deltaTime;
    transform->translate(movement);
}
```

---

## CUDA интеграция

### CudaVulkanInterop

Интеграция CUDA с Vulkan для высокопроизводительных вычислений.

#### Инициализация

```cpp
CudaVulkanInterop();
bool initialize();
void cleanup();
```

#### Управление памятью

```cpp
bool createExternalMemory(VkDeviceMemory vulkanMemory, size_t size);
void* mapCudaMemory();
void unmapCudaMemory();
```

#### Синхронизация

```cpp
bool createSemaphore();
void signalFromCuda();
void waitFromVulkan();
```

#### Обработка данных

```cpp
void processVertexData(const std::vector<Vertex>& vertices);
void processGaussianData(const GaussianData& data);
```

### CudaKernels

CUDA ядра для обработки данных.

#### Гауссовы сплаты

```cpp
void rasterizeGaussians(
    const GaussianData* gaussians,
    int numGaussians,
    const CameraParams& camera,
    float* outputBuffer,
    int width, int height
);
```

#### Сортировка по глубине

```cpp
void depthSort(
    GaussianData* gaussians,
    int numGaussians,
    const Vector3& cameraPos
);
```

#### Оптимизация тайлов

```cpp
void optimizeTileRasterization(
    const TileData* tiles,
    int numTiles,
    float* outputBuffer
);
```

---

## Система ресурсов

### ResourceManager

Управление ресурсами движка.

#### Инициализация

```cpp
ResourceManager();
bool initialize();
void cleanup();
```

#### Загрузка ресурсов

```cpp
template<typename T>
std::shared_ptr<T> loadResource(const std::string& path);

std::shared_ptr<Texture> loadTexture(const std::string& path);
std::shared_ptr<Mesh3D> loadMesh(const std::string& path);
std::shared_ptr<Shader3D> loadShader(const std::string& vertexPath, const std::string& fragmentPath);
```

#### Управление кэшем

```cpp
void unloadResource(const std::string& path);
void clearCache();
size_t getCacheSize() const;
```

#### Асинхронная загрузка

```cpp
std::future<std::shared_ptr<Resource>> loadResourceAsync(const std::string& path);
bool isResourceLoaded(const std::string& path) const;
```

### TextureManager

Специализированный менеджер текстур.

#### Создание текстур

```cpp
std::shared_ptr<Texture> createTexture2D(int width, int height, TextureFormat format);
std::shared_ptr<Texture> createCubemap(const std::array<std::string, 6>& faces);
std::shared_ptr<Texture> createRenderTarget(int width, int height);
```

#### Настройки текстур

```cpp
void setTextureFiltering(TextureID id, FilterMode mode);
void setTextureWrapping(TextureID id, WrapMode mode);
void generateMipmaps(TextureID id);
```

---

## Профилирование

### Profiler

Встроенная система профилирования.

#### Макросы профилирования

```cpp
PROFILE_SCOPE(name);                    // Профилирование области видимости
PROFILE_FUNCTION();                     // Профилирование функции
PROFILE_BLOCK_START(name);              // Начало блока
PROFILE_BLOCK_END(name);                // Конец блока
```

#### Управление профайлером

```cpp
static void initialize();
static void cleanup();
static void beginFrame();
static void endFrame();
```

#### Сбор данных

```cpp
static ProfileData getFrameData();
static void saveToFile(const std::string& filename);
static void clearData();
```

#### Настройки

```cpp
static void setEnabled(bool enabled);
static void setMaxFrames(int maxFrames);
static void setOutputFormat(OutputFormat format);
```

### PerformanceMonitor

Мониторинг производительности в реальном времени.

#### Метрики

```cpp
float getFPS() const;
float getFrameTime() const;
float getCPUUsage() const;
float getGPUUsage() const;
size_t getMemoryUsage() const;
```

#### Статистика рендеринга

```cpp
int getDrawCalls() const;
int getTriangleCount() const;
int getVertexCount() const;
```

#### Уведомления

```cpp
void setFPSThreshold(float minFPS);
void setMemoryThreshold(size_t maxMemory);
void onPerformanceAlert(std::function<void(AlertType)> callback);
```

---

## Безопасный вывод в консоль

### SafeConsole

Система безопасного вывода с поддержкой различных типов данных.

#### Макросы безопасного вывода

```cpp
SAFE_TO_STRING(value);                  // Безопасное преобразование в строку
SAFE_COUT(value);                       // Безопасный вывод в cout
SAFE_CERR(value);                       // Безопасный вывод в cerr
```

#### Поддерживаемые типы

```cpp
// Базовые типы
int, float, double, bool, char
std::string, const char*

// Математические типы
Vector3, Vector4, Matrix4, Quaternion

// Контейнеры
std::vector<T>, std::array<T, N>
```

#### Настройки форматирования

```cpp
void setFloatPrecision(int precision);
void setBoolFormat(const std::string& trueStr, const std::string& falseStr);
void setVectorFormat(const std::string& format);
```

---

## Примеры интеграции

### CUDA-Vulkan рендеринг

```cpp
#include <Engine3D/CudaVulkanInterop.h>

// Инициализация интеграции
CudaVulkanInterop interop;
if (!interop.initialize()) {
    Console::error("Не удалось инициализировать CUDA-Vulkan интеграцию");
    return false;
}

// Обработка данных на CUDA
std::vector<GaussianData> gaussians = loadGaussianData("scene.gs");
interop.processGaussianData(gaussians);

// Синхронизация с Vulkan
interop.signalFromCuda();
vulkanRenderer.waitForCuda();
vulkanRenderer.renderFrame();
```

### Профилирование производительности

```cpp
void renderLoop() {
    PROFILE_FUNCTION();
    
    while (running) {
        PROFILE_SCOPE("Frame");
        
        {
            PROFILE_SCOPE("Update");
            updateScene(deltaTime);
        }
        
        {
            PROFILE_SCOPE("Render");
            renderScene();
        }
        
        // Вывод статистики каждые 60 кадров
        if (frameCount % 60 == 0) {
            auto data = Profiler::getFrameData();
            Console::info("FPS: " + SAFE_TO_STRING(data.fps));
            Console::info("Frame Time: " + SAFE_TO_STRING(data.frameTime) + "ms");
        }
    }
}
```

### Безопасный вывод отладочной информации

```cpp
void debugRenderState() {
    Vector3 cameraPos = camera->getPosition();
    Matrix4 viewMatrix = camera->getViewMatrix();
    int triangleCount = scene->getTriangleCount();
    
    // Безопасный вывод различных типов
    Console::debug("Camera Position: " + SAFE_TO_STRING(cameraPos));
    Console::debug("Triangle Count: " + SAFE_TO_STRING(triangleCount));
    Console::debug("View Matrix: " + SAFE_TO_STRING(viewMatrix));
    
    // Вывод массивов
    std::vector<float> timings = profiler->getFrameTimings();
    Console::debug("Frame Timings: " + SAFE_TO_STRING(timings));
}
```

---

## Версия API

**Версия**: 1.0.0  
**Дата**: 28 сентября 2025  
**Совместимость**: C++17 или новее  
**CUDA**: 11.0+  
**Vulkan**: 1.2+  

### Изменения в версии 1.0.0

- Добавлена CUDA-Vulkan интеграция
- Реализована система безопасного вывода в консоль
- Добавлена встроенная система профилирования
- Улучшена система управления ресурсами
- Добавлена поддержка асинхронной загрузки ресурсов

Для получения последней версии документации посетите официальный репозиторий проекта.
