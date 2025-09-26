# API Reference - 3D Game Engine

## Содержание

1. [Математическая библиотека](#математическая-библиотека)
2. [Система рендеринга](#система-рендеринга)
3. [Физическая система](#физическая-система)
4. [Система ввода](#система-ввода)
5. [Система объектов](#система-объектов)

## Математическая библиотека

### Vector3

3-мерный вектор для работы с 3D пространством.

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
float operator[](int index) const;
float& operator[](int index);
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
Vector3 reflect(const Vector3& normal) const;    // Отражение от нормали
Vector3 project(const Vector3& other) const;     // Проекция на другой вектор
float angle(const Vector3& other) const;         // Угол между векторами
```

#### Статические методы

```cpp
static Vector3 zero();      // [0, 0, 0]
static Vector3 one();       // [1, 1, 1]
static Vector3 unitX();     // [1, 0, 0]
static Vector3 unitY();     // [0, 1, 0]
static Vector3 unitZ();     // [0, 0, 1]
static Vector3 forward();   // [0, 0, -1] - направление вперед в 3D
static Vector3 back();      // [0, 0, 1]
static Vector3 left();      // [-1, 0, 0]
static Vector3 right();     // [1, 0, 0]
static Vector3 up();        // [0, 1, 0]
static Vector3 down();      // [0, -1, 0]
```

### Matrix4

4x4 матрица для трансформаций в 3D пространстве (использует гомогенные координаты).

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
Vector3 operator*(const Vector3& vec) const;  // Умножение на 3D вектор
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
static Matrix4 translation(const Vector3& translation);
static Matrix4 translation(float x, float y, float z);
static Matrix4 scaling(const Vector3& scale);
static Matrix4 scaling(float x, float y, float z);
static Matrix4 scaling(float uniformScale);
```

#### Повороты

```cpp
static Matrix4 rotationX(float angle);           // Поворот вокруг оси X
static Matrix4 rotationY(float angle);           // Поворот вокруг оси Y
static Matrix4 rotationZ(float angle);           // Поворот вокруг оси Z
static Matrix4 rotation(const Vector3& axis, float angle); // Поворот вокруг произвольной оси
static Matrix4 rotationFromQuaternion(const Quaternion& q); // Из кватерниона
static Matrix4 eulerAngles(float pitch, float yaw, float roll); // Углы Эйлера
```

#### Проекции и камера

```cpp
static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
static Matrix4 perspective(float fovY, float aspect, float near, float far);
static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
```

### Quaternion

Кватернион для представления поворотов в 3D пространстве.

#### Конструкторы

```cpp
Quaternion();                                     // Единичный кватернион
Quaternion(float w, float x, float y, float z);
Quaternion(const Quaternion& other);             // Копирование
explicit Quaternion(const Vector3& axis, float angle);  // Из оси и угла
explicit Quaternion(const Vector3& eulerAngles); // Из углов Эйлера
```

#### Операторы

```cpp
Quaternion operator+(const Quaternion& other) const;
Quaternion operator-(const Quaternion& other) const;
Quaternion operator*(const Quaternion& other) const;   // Композиция поворотов
Quaternion operator*(float scalar) const;
Quaternion operator/(float scalar) const;
Quaternion& operator+=(const Quaternion& other);
Quaternion& operator-=(const Quaternion& other);
Quaternion& operator*=(const Quaternion& other);
Quaternion& operator*=(float scalar);
Quaternion& operator/=(float scalar);
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
Vector3 toEulerAngles() const;                   // Преобразование в углы Эйлера
void toAxisAngle(Vector3& axis, float& angle) const; // Преобразование в ось-угол
```

#### Создание поворотов

```cpp
static Quaternion fromAxisAngle(const Vector3& axis, float angle);
static Quaternion fromEulerAngles(float pitch, float yaw, float roll);
static Quaternion fromEulerAngles(const Vector3& eulerAngles);
static Quaternion fromMatrix(const Matrix4& mat);
static Quaternion lookRotation(const Vector3& forward, const Vector3& up = Vector3::up());
```

#### Интерполяция

```cpp
Quaternion slerp(const Quaternion& other, float t) const; // Сферическая интерполяция
Quaternion lerp(const Quaternion& other, float t) const;  // Линейная интерполяция
Quaternion nlerp(const Quaternion& other, float t) const; // Нормализованная линейная интерполяция
```

## Система объектов

### GameObject3D

3D игровой объект, следующий принципу единственной ответственности.

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
static GameObject3D* find(const std::string& name);
static std::vector<GameObject3D*> findAllWithTag(const std::string& tag);
static GameObject3D* findWithTag(const std::string& tag);
```

#### Создание объектов

```cpp
static GameObject3D* create(const std::string& name = "GameObject3D");
static GameObject3D* createPrimitive(const std::string& type);
```

### Component3D

Базовый класс компонента.

#### Конструкторы

```cpp
Component3D();
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

## Математические константы

```cpp
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI * 0.5f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;
constexpr float EPSILON = 1e-6f;
```

## Утилитарные функции

```cpp
inline float toRadians(float degrees);
inline float toDegrees(float radians);
inline float clamp(float value, float min, float max);
inline float lerp(float a, float b, float t);
inline bool isNearlyEqual(float a, float b, float epsilon = EPSILON);
inline bool isNearlyZero(float value, float epsilon = EPSILON);
```

## Примеры использования

### Создание и трансформация объекта

```cpp
using namespace Engine3D;
using namespace Engine3D::Math;

// Создание объекта
auto* gameObject = GameObject3D::create("Player");

// Получение трансформации
Transform3D* transform = gameObject->getTransform();

// Установка позиции
transform->setPosition(Vector3(10.0f, 0.0f, 5.0f));

// Поворот
Quaternion rotation = Quaternion::fromEulerAngles(0.0f, toRadians(45.0f), 0.0f);
transform->setRotation(rotation);

// Масштабирование
transform->setScale(Vector3(2.0f, 2.0f, 2.0f));
```

### Работа с матрицами

```cpp
// Создание матрицы вида камеры
Vector3 cameraPos(0.0f, 10.0f, 10.0f);
Vector3 target(0.0f, 0.0f, 0.0f);
Vector3 up = Vector3::up();
Matrix4 viewMatrix = Matrix4::lookAt(cameraPos, target, up);

// Создание матрицы проекции
float fov = toRadians(60.0f);
float aspect = 16.0f / 9.0f;
float nearPlane = 0.1f;
float farPlane = 1000.0f;
Matrix4 projMatrix = Matrix4::perspective(fov, aspect, nearPlane, farPlane);

// Комбинирование матриц
Matrix4 mvpMatrix = projMatrix * viewMatrix * transform->getWorldMatrix();
```

### Работа с кватернионами

```cpp
// Создание поворота из углов Эйлера
Vector3 eulerAngles(toRadians(30.0f), toRadians(45.0f), toRadians(0.0f));
Quaternion q1 = Quaternion::fromEulerAngles(eulerAngles);

// Создание поворота из оси и угла
Vector3 axis = Vector3::up();
float angle = toRadians(90.0f);
Quaternion q2 = Quaternion::fromAxisAngle(axis, angle);

// Интерполяция между поворотами
Quaternion interpolated = q1.slerp(q2, 0.5f);

// Применение поворота к вектору
Vector3 direction = Vector3::forward();
Vector3 rotatedDirection = interpolated.rotate(direction);
```
