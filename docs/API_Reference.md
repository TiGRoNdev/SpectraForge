# API Reference - 4D Game Engine

## Содержание

1. [Математическая библиотека](#математическая-библиотека)
2. [Система рендеринга](#система-рендеринга)
3. [Физическая система](#физическая-система)
4. [Система ввода](#система-ввода)
5. [Система объектов](#система-объектов)

## Математическая библиотека

### Vector4

4-мерный вектор для работы с гиперпространством.

#### Конструкторы

```cpp
Vector4();                                    // [0, 0, 0, 0]
Vector4(float x, float y, float z, float w); // [x, y, z, w]
Vector4(const Vector4& other);               // Копирование
```

#### Операторы

```cpp
Vector4 operator+(const Vector4& other) const;
Vector4 operator-(const Vector4& other) const;
Vector4 operator*(float scalar) const;
Vector4 operator/(float scalar) const;
Vector4& operator+=(const Vector4& other);
Vector4& operator-=(const Vector4& other);
Vector4& operator*=(float scalar);
Vector4& operator/=(float scalar);
bool operator==(const Vector4& other) const;
bool operator!=(const Vector4& other) const;
```

#### Математические операции

```cpp
float dot(const Vector4& other) const;           // Скалярное произведение
Vector4 cross(const Vector4& other) const;       // Векторное произведение
float magnitude() const;                         // Длина вектора
float magnitudeSquared() const;                  // Квадрат длины
Vector4 normalized() const;                      // Нормализованный вектор
void normalize();                                // Нормализация на месте
float distance(const Vector4& other) const;      // Расстояние до другого вектора
Vector4 lerp(const Vector4& other, float t) const; // Линейная интерполяция
```

#### Специальные операции для 4D

```cpp
Vector4 projectTo3D() const;                     // Проекция в 3D
Vector4 perspectiveProject(float distance) const; // Перспективная проекция
Vector4 crossSection(float wValue) const;        // Сечение при заданном w
```

#### Статические методы

```cpp
static Vector4 zero();    // [0, 0, 0, 0]
static Vector4 one();     // [1, 1, 1, 1]
static Vector4 unitX();   // [1, 0, 0, 0]
static Vector4 unitY();   // [0, 1, 0, 0]
static Vector4 unitZ();   // [0, 0, 1, 0]
static Vector4 unitW();   // [0, 0, 0, 1]
```

### Matrix4

4x4 матрица для трансформаций в 4D пространстве.

#### Конструкторы

```cpp
Matrix4();                                    // Единичная матрица
Matrix4(const Matrix4& other);               // Копирование
explicit Matrix4(float diagonal);            // Диагональная матрица
Matrix4(const std::array<std::array<float, 4>, 4>& data);
```

#### Операторы

```cpp
Matrix4 operator+(const Matrix4& other) const;
Matrix4 operator-(const Matrix4& other) const;
Matrix4 operator*(const Matrix4& other) const;
Matrix4 operator*(float scalar) const;
Vector4 operator*(const Vector4& vec) const;
Matrix4& operator+=(const Matrix4& other);
Matrix4& operator-=(const Matrix4& other);
Matrix4& operator*=(const Matrix4& other);
Matrix4& operator*=(float scalar);
bool operator==(const Matrix4& other) const;
bool operator!=(const Matrix4& other) const;
float& operator()(int row, int col);
const float& operator()(int row, int col) const;
```

#### Основные операции

```cpp
Matrix4 transpose() const;                       // Транспонирование
float determinant() const;                       // Определитель
Matrix4 inverse() const;                         // Обратная матрица
Matrix4 adjugate() const;                        // Присоединенная матрица
bool isInvertible() const;                       // Проверка обратимости
void setIdentity();                              // Установка единичной матрицы
void setZero();                                  // Установка нулевой матрицы
```

#### Трансформации

```cpp
static Matrix4 translation(const Vector4& translation);
static Matrix4 translation(float x, float y, float z, float w);
static Matrix4 scaling(const Vector4& scale);
static Matrix4 scaling(float x, float y, float z, float w);
static Matrix4 scaling(float uniformScale);
```

#### Повороты в 4D

```cpp
static Matrix4 rotationXY(float angle);          // Поворот в плоскости XY
static Matrix4 rotationXZ(float angle);          // Поворот в плоскости XZ
static Matrix4 rotationXW(float angle);          // Поворот в плоскости XW
static Matrix4 rotationYZ(float angle);          // Поворот в плоскости YZ
static Matrix4 rotationYW(float angle);          // Поворот в плоскости YW
static Matrix4 rotationZW(float angle);          // Поворот в плоскости ZW
static Matrix4 rotation(const Vector4& axis, float angle);
static Matrix4 eulerAngles(float x, float y, float z, float w);
```

#### Проекции

```cpp
static Matrix4 orthographicProjection();         // Ортогональная проекция 4D->3D
static Matrix4 perspectiveProjection(float distance); // Перспективная проекция
static Matrix4 crossSectionProjection(float wValue);  // Проекция сечения
```

### Quaternion4D

4D кватернион для представления поворотов в гиперпространстве.

#### Конструкторы

```cpp
Quaternion4D();                                 // Единичный кватернион
Quaternion4D(float w, float x, float y, float z, float u = 0.0f, float v = 0.0f);
Quaternion4D(const Quaternion4D& other);       // Копирование
explicit Quaternion4D(const Vector4& axis, float angle);
```

#### Операторы

```cpp
Quaternion4D operator+(const Quaternion4D& other) const;
Quaternion4D operator-(const Quaternion4D& other) const;
Quaternion4D operator*(const Quaternion4D& other) const;
Quaternion4D operator*(float scalar) const;
Quaternion4D& operator+=(const Quaternion4D& other);
Quaternion4D& operator-=(const Quaternion4D& other);
Quaternion4D& operator*=(const Quaternion4D& other);
Quaternion4D& operator*=(float scalar);
bool operator==(const Quaternion4D& other) const;
bool operator!=(const Quaternion4D& other) const;
```

#### Основные операции

```cpp
float magnitude() const;                         // Длина кватерниона
float magnitudeSquared() const;                  // Квадрат длины
Quaternion4D normalized() const;                 // Нормализованный кватернион
void normalize();                                // Нормализация на месте
Quaternion4D conjugate() const;                  // Сопряженный кватернион
Quaternion4D inverse() const;                    // Обратный кватернион
float dot(const Quaternion4D& other) const;      // Скалярное произведение
```

#### Повороты

```cpp
Vector4 rotate(const Vector4& vec) const;        // Поворот вектора
Matrix4 toMatrix() const;                        // Преобразование в матрицу
static Quaternion4D fromMatrix(const Matrix4& mat); // Создание из матрицы
```

#### Создание поворотов в различных плоскостях

```cpp
static Quaternion4D rotationXY(float angle);
static Quaternion4D rotationXZ(float angle);
static Quaternion4D rotationXW(float angle);
static Quaternion4D rotationYZ(float angle);
static Quaternion4D rotationYW(float angle);
static Quaternion4D rotationZW(float angle);
```

#### Интерполяция

```cpp
Quaternion4D slerp(const Quaternion4D& other, float t) const; // Сферическая интерполяция
Quaternion4D lerp(const Quaternion4D& other, float t) const;  // Линейная интерполяция
```

## Система рендеринга

### Mesh4D

4D меш для хранения геометрии.

#### Конструкторы

```cpp
Mesh4D();
```

#### Управление вершинами

```cpp
void addVertex(const Vertex4D& vertex);
void addTriangle(unsigned int i1, unsigned int i2, unsigned int i3);
void addQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4);
```

#### GPU операции

```cpp
void uploadToGPU();
void render() const;
void cleanup();
```

#### Создание примитивов

```cpp
static Mesh4D createTesseract(float size = 1.0f);
static Mesh4D createHypercube(float size = 1.0f);
static Mesh4D createSimplex();
```

### Shader4D

Шейдер для рендеринга 4D объектов.

#### Конструкторы

```cpp
Shader4D();
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
void setVector4(const std::string& name, const Vector4& vector) const;
void setFloat(const std::string& name, float value) const;
void setInt(const std::string& name, int value) const;
void setBool(const std::string& name, bool value) const;
```

### Camera4D

Камера для 4D пространства.

#### Конструкторы

```cpp
Camera4D();
```

#### Настройка позиции

```cpp
void setPosition(const Vector4& pos);
void setTarget(const Vector4& target);
void lookAt(const Vector4& pos, const Vector4& target, const Vector4& up);
```

#### Навигация

```cpp
void move(const Vector4& direction);
void rotate(float angle, const Vector4& axis);
void setCrossSection(float wValue);
```

#### Получение матриц

```cpp
Matrix4 getViewMatrix() const;
Matrix4 getProjectionMatrix() const;
Matrix4 getViewProjectionMatrix() const;
```

### Renderer

Основной класс рендерера.

#### Инициализация

```cpp
static Renderer& getInstance();
bool initialize(int width, int height);
void cleanup();
```

#### Рендеринг

```cpp
void beginFrame();
void endFrame();
void renderMesh(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader);
void renderWireframe(const Mesh4D& mesh, const Matrix4& transform, const Shader4D& shader);
```

#### Настройки

```cpp
void setClearColor(float r, float g, float b, float a);
void clear();
void setViewport(int x, int y, int width, int height);
void enableDepthTest(bool enable);
void enableBlending(bool enable);
```

## Физическая система

### RigidBody4D

4D физическое тело.

#### Конструкторы

```cpp
RigidBody4D();
```

#### Обновление

```cpp
void update(float deltaTime);
```

#### Применение сил

```cpp
void applyForce(const Vector4& force);
void applyTorque(const Vector4& torque);
void applyImpulse(const Vector4& impulse);
void applyAngularImpulse(const Vector4& impulse);
```

#### Геттеры и сеттеры

```cpp
Vector4 getPosition() const;
void setPosition(const Vector4& pos);
Vector4 getVelocity() const;
void setVelocity(const Vector4& vel);
float getMass() const;
void setMass(float m);
```

### Collider4D

Базовый класс для 4D коллайдеров.

#### Типы коллайдеров

```cpp
enum class Type {
    Sphere,     // 4D сфера
    Box,        // 4D параллелепипед
    Plane,      // 4D плоскость
    Mesh        // Произвольный 4D меш
};
```

#### Основные методы

```cpp
virtual bool intersects(const Collider4D& other) const = 0;
virtual Vector4 getClosestPoint(const Vector4& point) const = 0;
virtual Vector4 getNormal(const Vector4& point) const = 0;
virtual float getVolume() const = 0;
```

#### Настройка

```cpp
void setCenter(const Vector4& center);
void setSize(const Vector4& size);
void setRadius(float radius);
void setPlane(const Vector4& normal, float distance);
```

### SphereCollider4D

4D сферический коллайдер.

```cpp
SphereCollider4D(float radius = 1.0f);
```

### BoxCollider4D

4D боксовый коллайдер.

```cpp
BoxCollider4D(const Vector4& size = Vector4::one());

Vector4 getMin() const;
Vector4 getMax() const;
bool intersectsAABB(const BoxCollider4D& other) const;
```

### PlaneCollider4D

4D плоскостной коллайдер.

```cpp
PlaneCollider4D(const Vector4& normal = Vector4::unitW(), float distance = 0.0f);

float distanceToPoint(const Vector4& point) const;
bool isPointAbove(const Vector4& point) const;
```

### PhysicsWorld4D

4D физический мир.

#### Управление объектами

```cpp
void addBody(std::shared_ptr<RigidBody4D> body);
void addCollider(std::shared_ptr<Collider4D> collider);
void removeBody(std::shared_ptr<RigidBody4D> body);
void removeCollider(std::shared_ptr<Collider4D> collider);
```

#### Обновление

```cpp
void update(float deltaTime);
void step(float deltaTime);
```

#### Обнаружение столкновений

```cpp
std::vector<CollisionResult4D> detectCollisions();
CollisionResult4D checkCollision(const Collider4D& a, const Collider4D& b);
```

#### Raycasting

```cpp
struct RaycastHit4D {
    bool hasHit;
    Vector4 point;
    Vector4 normal;
    float distance;
    std::shared_ptr<Collider4D> collider;
};

RaycastHit4D raycast(const Vector4& origin, const Vector4& direction, float maxDistance = 1000.0f);
```

#### Настройки

```cpp
void setGravity(const Vector4& gravity);
void setTimeStep(float step);
void clear();
```

### ParticleSystem4D

Система 4D частиц.

#### Конструкторы

```cpp
ParticleSystem4D();
```

#### Управление частицами

```cpp
void update(float deltaTime);
void emit(int count = 1);
void clear();
int getActiveParticleCount() const;
```

#### Настройка эмиттера

```cpp
void setEmitter(const Vector4& position, const Vector4& velocity);
void setGravity(const Vector4& gravity);
void setEmissionRate(float rate);
void setParticleLife(float life);
void setParticleSize(float size);
void setColorRange(const Vector4& start, const Vector4& end);
```

## Система ввода

### InputManager4D

Система ввода для 4D движка.

#### Инициализация

```cpp
static InputManager4D& getInstance();
bool initialize(GLFWwindow* window);
void cleanup();
void update();
```

#### Управление действиями

```cpp
void addAction(const InputAction4D& action);
void removeAction(const std::string& name);
InputAction4D* getAction(const std::string& name);
```

#### Проверка состояния

```cpp
bool isKeyPressed(Key4D key) const;
bool isKeyJustPressed(Key4D key) const;
bool isKeyJustReleased(Key4D key) const;
bool isMousePressed(MouseButton4D button) const;
bool isMouseJustPressed(MouseButton4D button) const;
bool isMouseJustReleased(MouseButton4D button) const;
```

#### Данные мыши

```cpp
Vector4 getMousePosition() const;
Vector4 getMouseDelta() const;
Vector4 getScrollDelta() const;
```

#### Управление курсором

```cpp
void setCursorVisible(bool visible);
void setCursorLocked(bool locked);
void setCursorPosition(double x, double y);
```

### Controller4D

4D контроллер для навигации.

#### Конструкторы

```cpp
Controller4D();
```

#### Обновление

```cpp
void update(float deltaTime);
void handleInput(const InputState4D& inputState);
```

#### Движение

```cpp
void move(const Vector4& direction);
void moveForward(float amount);
void moveRight(float amount);
void moveUp(float amount);
void moveW(float amount);
```

#### Повороты

```cpp
void rotateXY(float angle);
void rotateXZ(float angle);
void rotateXW(float angle);
void rotateYZ(float angle);
void rotateYW(float angle);
void rotateZW(float angle);
```

#### Настройки

```cpp
void setMoveSpeed(float speed);
void setRotationSpeed(float speed);
void setWSpeed(float speed);
void setSensitivity(float sensitivity);
void setMouseLook(bool enabled);
void setWMovement(bool enabled);
void setRotation(bool enabled);
```

#### Геттеры

```cpp
Vector4 getPosition() const;
Vector4 getRotation() const;
Vector4 getVelocity() const;
Matrix4 getTransformMatrix() const;
```

### InputAction4D

Действие ввода.

#### Конструкторы

```cpp
InputAction4D(const std::string& name);
```

#### Настройка

```cpp
void addKey(Key4D key);
void addMouseButton(MouseButton4D button);
void setOnPressed(std::function<void()> callback);
void setOnReleased(std::function<void()> callback);
void setOnHeld(std::function<void()> callback);
```

## Система объектов

### GameObject4D

4D игровой объект.

#### Конструкторы

```cpp
GameObject4D(const std::string& name = "GameObject4D");
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
void removeComponent(T* component);
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
static GameObject4D* find(const std::string& name);
static std::vector<GameObject4D*> findAllWithTag(const std::string& tag);
static GameObject4D* findWithTag(const std::string& tag);
```

#### Создание объектов

```cpp
static GameObject4D* create(const std::string& name = "GameObject4D");
static GameObject4D* createPrimitive(const std::string& type);
```

### Component4D

Базовый класс компонента.

#### Конструкторы

```cpp
Component4D();
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
```

### Transform4D

Компонент трансформации для 4D объектов.

#### Конструкторы

```cpp
Transform4D();
```

#### Трансформации

```cpp
void setPosition(const Vector4& pos);
void setRotation(const Quaternion4D& rot);
void setScale(const Vector4& scl);
void translate(const Vector4& translation);
void rotate(const Quaternion4D& rotation);
void scaleBy(const Vector4& scale);
```

#### Получение матриц

```cpp
Matrix4 getLocalMatrix() const;
Matrix4 getWorldMatrix() const;
```

#### Мировые координаты

```cpp
Vector4 getWorldPosition() const;
Quaternion4D getWorldRotation() const;
Vector4 getWorldScale() const;
```

#### Направления

```cpp
Vector4 forward() const;
Vector4 right() const;
Vector4 up() const;
Vector4 wAxis() const;
```

#### Иерархия

```cpp
void setParent(Transform4D* parent);
Transform4D* getParent() const;
void addChild(Transform4D* child);
void removeChild(Transform4D* child);
const std::vector<Transform4D*>& getChildren() const;
```

### MeshRenderer4D

Компонент рендеринга для 4D объектов.

#### Конструкторы

```cpp
MeshRenderer4D();
```

#### Настройка

```cpp
void setMesh(std::shared_ptr<Rendering::Mesh4D> mesh);
void setShader(std::shared_ptr<Rendering::Shader4D> shader);
void setColor(const Vector4& color);
```

### Collider4DComponent

Компонент коллайдера для 4D объектов.

#### Конструкторы

```cpp
Collider4DComponent();
```

#### Настройка

```cpp
void setCollider(std::shared_ptr<Physics::Collider4D> collider);
void setTrigger(bool trigger);
```

#### Проверка столкновений

```cpp
bool checkCollision(const Collider4DComponent& other) const;
```

### RigidBody4DComponent

Компонент физического тела для 4D объектов.

#### Конструкторы

```cpp
RigidBody4DComponent();
```

#### Настройка

```cpp
void setRigidBody(std::shared_ptr<Physics::RigidBody4D> body);
void setUseGravity(bool use);
```

#### Применение сил

```cpp
void addForce(const Vector4& force);
void addTorque(const Vector4& torque);
void addImpulse(const Vector4& impulse);
void addAngularImpulse(const Vector4& impulse);
```

### Camera4DComponent

Компонент камеры для 4D пространства.

#### Конструкторы

```cpp
Camera4DComponent();
```

#### Настройка

```cpp
void setMainCamera(bool main);
void setFieldOfView(float fov);
void setNearPlane(float near);
void setFarPlane(float far);
```

#### Получение матриц

```cpp
Matrix4 getViewMatrix() const;
Matrix4 getProjectionMatrix() const;
Matrix4 getViewProjectionMatrix() const;
```

### ParticleSystem4DComponent

Компонент системы частиц для 4D.

#### Конструкторы

```cpp
ParticleSystem4DComponent();
```

#### Настройка

```cpp
void setParticleSystem(std::shared_ptr<Physics::ParticleSystem4D> system);
void setAutoPlay(bool play);
void setEmissionRate(float rate);
```

#### Управление

```cpp
void play();
void stop();
void emit(int count = 1);
```
