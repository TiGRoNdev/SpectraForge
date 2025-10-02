# Стандарты кодирования SpectraForge

## 📋 Обзор

Данный документ определяет стандарты кодирования для проекта SpectraForge, основанные на принципах SOLID, современных практиках C++ и требованиях безопасности. Соблюдение этих стандартов обеспечивает читаемость, поддерживаемость и надежность кода.

## 🎯 Основные принципы

### SOLID принципы

1. **Single Responsibility Principle (SRP)**
   - Каждый класс должен иметь только одну причину для изменения
   - Разделяйте функциональность на отдельные классы

2. **Open/Closed Principle (OCP)**
   - Классы должны быть открыты для расширения, но закрыты для модификации
   - Используйте наследование и полиморфизм

3. **Liskov Substitution Principle (LSP)**
   - Объекты производных классов должны быть заменяемы объектами базовых классов
   - Соблюдайте контракты базовых классов

4. **Interface Segregation Principle (ISP)**
   - Клиенты не должны зависеть от интерфейсов, которые они не используют
   - Создавайте специализированные интерфейсы

5. **Dependency Inversion Principle (DIP)**
   - Зависьте от абстракций, а не от конкретных реализаций
   - Используйте dependency injection

## 📝 Соглашения об именовании

### Классы и структуры
```cpp
// ✅ Правильно: PascalCase
class VulkanRenderer {
    // ...
};

struct CameraParams {
    // ...
};

// ❌ Неправильно
class vulkanRenderer { };  // camelCase
class vulkan_renderer { }; // snake_case
```

### Функции и методы
```cpp
// ✅ Правильно: camelCase
void renderFrame();
bool initializeVulkan();
std::shared_ptr<Texture> loadTexture(const std::string& path);

// ❌ Неправильно
void RenderFrame();     // PascalCase
void render_frame();    // snake_case
```

### Переменные
```cpp
// ✅ Правильно: camelCase
int frameCount = 0;
float deltaTime = 0.016f;
std::string texturePath = "assets/texture.png";

// Поля класса с префиксом m_
class MyClass {
private:
    int m_value;
    std::string m_name;
    std::unique_ptr<Resource> m_resource;
};

// ❌ Неправильно
int FrameCount;     // PascalCase
int frame_count;    // snake_case для локальных переменных
```

### Константы
```cpp
// ✅ Правильно: UPPER_CASE или kPascalCase
const int MAX_TEXTURE_SIZE = 4096;
const float kDefaultFOV = 60.0f;
constexpr size_t kMaxVertices = 100000;

// ❌ Неправильно
const int maxTextureSize = 4096;  // camelCase
```

### Пространства имен
```cpp
// ✅ Правильно: PascalCase
namespace SpectraForge {
    namespace Rendering {
        namespace Vulkan {
            // ...
        }
    }
}

// ❌ Неправильно
namespace hyperengine { }  // camelCase
namespace hyper_engine { } // snake_case
```

### Макросы
```cpp
// ✅ Правильно: UPPER_CASE
#define SAFE_TO_STRING(value) SafeConsole::toString(value)
#define VK_CHECK(call) checkVulkanResult(call, #call, __FILE__, __LINE__)

// ❌ Неправильно
#define safeToString(value) // camelCase
```

## 🏗️ Структура файлов

### Заголовочные файлы (.h)
```cpp
#pragma once

// Системные заголовки
#include <memory>
#include <string>
#include <vector>

// Внешние библиотеки
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// Внутренние заголовки
#include "SpectraForge/Core/Object.h"
#include "SpectraForge/Math/Vector3.h"

namespace SpectraForge::Rendering {

/**
 * @brief Vulkan-based renderer for high-performance 3D rendering
 * 
 * This class provides a modern Vulkan implementation with CUDA integration
 * for advanced rendering techniques including Gaussian Splatting and Ray Tracing.
 * 
 * @example
 * @code
 * VulkanRenderer renderer;
 * if (renderer.initialize(1920, 1080)) {
 *     renderer.renderFrame(sceneData, cameraParams);
 * }
 * @endcode
 */
class VulkanRenderer {
public:
    // Конструкторы и деструктор
    VulkanRenderer();
    explicit VulkanRenderer(const RendererConfig& config);
    ~VulkanRenderer();
    
    // Запрет копирования, разрешение перемещения
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&& other) noexcept;
    VulkanRenderer& operator=(VulkanRenderer&& other) noexcept;
    
    // Публичные методы
    bool initialize(int width, int height);
    void renderFrame(const SceneData& sceneData, const CameraParams& cameraParams);
    void cleanup();
    
    // Геттеры и сеттеры
    bool isInitialized() const { return m_initialized; }
    void setRenderScale(float scale) { m_renderScale = scale; }
    float getRenderScale() const { return m_renderScale; }

private:
    // Приватные методы
    bool createVulkanInstance();
    bool setupDebugMessenger();
    void cleanupVulkan();
    
    // Поля данных
    bool m_initialized = false;
    float m_renderScale = 1.0f;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    std::unique_ptr<VulkanContext> m_context;
};

} // namespace SpectraForge::Rendering
```

### Исходные файлы (.cpp)
```cpp
#include "SpectraForge/Rendering/VulkanRenderer.h"

// Дополнительные заголовки
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/Profiler.h"

// Использование пространств имен
using namespace SpectraForge::Rendering;
using namespace SpectraForge::Core;

VulkanRenderer::VulkanRenderer() {
    Console::debug("VulkanRenderer создан");
}

VulkanRenderer::~VulkanRenderer() {
    cleanup();
    Console::debug("VulkanRenderer уничтожен");
}

bool VulkanRenderer::initialize(int width, int height) {
    PROFILE_FUNCTION();
    
    if (m_initialized) {
        Console::warning("VulkanRenderer уже инициализирован");
        return true;
    }
    
    Console::info("Инициализация VulkanRenderer " + 
                 SAFE_TO_STRING(width) + "x" + SAFE_TO_STRING(height));
    
    if (!createVulkanInstance()) {
        Console::error("Не удалось создать Vulkan instance");
        return false;
    }
    
    m_initialized = true;
    Console::success("VulkanRenderer успешно инициализирован");
    return true;
}
```

## 🔒 Безопасность и надежность

### Безопасный вывод в консоль
```cpp
// ✅ Правильно: всегда используйте SAFE_TO_STRING
std::cout << "Значение: " << SAFE_TO_STRING(myVariable) << std::endl;
Console::info("FPS: " + SAFE_TO_STRING(fps));
Console::debug("Position: " + SAFE_TO_STRING(position));

// ❌ Неправильно: небезопасный вывод
std::cout << "Значение: " << myVariable << std::endl;  // Может вызвать крах!
```

### Управление памятью
```cpp
// ✅ Правильно: используйте умные указатели
std::unique_ptr<VulkanRenderer> renderer = std::make_unique<VulkanRenderer>();
std::shared_ptr<Texture> texture = std::make_shared<Texture>();

// RAII для ресурсов
class VulkanBuffer {
public:
    VulkanBuffer(VkDevice device, size_t size) : m_device(device) {
        // Создание буфера
    }
    
    ~VulkanBuffer() {
        cleanup(); // Автоматическая очистка
    }
    
    // Запрет копирования
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    
    // Разрешение перемещения
    VulkanBuffer(VulkanBuffer&& other) noexcept 
        : m_device(other.m_device), m_buffer(other.m_buffer) {
        other.m_buffer = VK_NULL_HANDLE;
    }

private:
    VkDevice m_device;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    
    void cleanup() {
        if (m_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device, m_buffer, nullptr);
            m_buffer = VK_NULL_HANDLE;
        }
    }
};

// ❌ Неправильно: сырые указатели
VulkanRenderer* renderer = new VulkanRenderer(); // Утечка памяти!
```

### Обработка ошибок
```cpp
// ✅ Правильно: проверка результатов Vulkan
#define VK_CHECK(call) \
    do { \
        VkResult result = call; \
        if (result != VK_SUCCESS) { \
            throw VulkanException(result, #call, __FILE__, __LINE__); \
        } \
    } while(0)

VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

// ✅ Правильно: проверка результатов CUDA
#define CUDA_CHECK(call) \
    do { \
        cudaError_t error = call; \
        if (error != cudaSuccess) { \
            throw CudaException(error, #call, __FILE__, __LINE__); \
        } \
    } while(0)

CUDA_CHECK(cudaMalloc(&devicePtr, size));

// ✅ Правильно: использование исключений
class VulkanException : public std::runtime_error {
public:
    VulkanException(VkResult result, const char* call, const char* file, int line)
        : std::runtime_error(formatMessage(result, call, file, line))
        , m_result(result) {
    }
    
    VkResult getResult() const { return m_result; }

private:
    VkResult m_result;
    
    static std::string formatMessage(VkResult result, const char* call, 
                                   const char* file, int line) {
        return "Vulkan error " + std::to_string(result) + 
               " in " + call + " at " + file + ":" + std::to_string(line);
    }
};
```

## 📐 Форматирование кода

### Отступы и пробелы
```cpp
// ✅ Правильно: 4 пробела для отступов
class MyClass {
public:
    void myMethod() {
        if (condition) {
            doSomething();
        }
    }
    
private:
    int m_value;
};

// ❌ Неправильно: табы или неправильное количество пробелов
class MyClass {
  public:  // 2 пробела
	void myMethod() {  // Таб
		// ...
	}
};
```

### Фигурные скобки
```cpp
// ✅ Правильно: скобки на новой строке для функций и классов
class MyClass 
{
public:
    void myMethod() 
    {
        if (condition) {
            // Для коротких блоков скобки на той же строке
            doSomething();
        }
        
        // Для длинных блоков - на новой строке
        if (complexCondition) 
        {
            doComplexOperation();
            doAnotherOperation();
            finalizeOperation();
        }
    }
};
```

### Длина строк
```cpp
// ✅ Правильно: максимум 100 символов
void myFunction(const std::string& parameter1, 
               const std::string& parameter2,
               int parameter3) {
    // ...
}

// Разбиение длинных выражений
auto result = someVeryLongFunctionName(parameter1, parameter2) +
              anotherLongFunctionName(parameter3, parameter4) *
              yetAnotherFunction(parameter5);
```

## 📚 Документация кода

### Doxygen комментарии
```cpp
/**
 * @brief Краткое описание функции
 * 
 * Подробное описание того, что делает функция.
 * Может содержать несколько абзацев.
 * 
 * @param input Описание входного параметра
 * @param output Описание выходного параметра
 * @return Описание возвращаемого значения
 * 
 * @throws std::runtime_error Когда происходит критическая ошибка
 * @throws VulkanException При ошибках Vulkan API
 * 
 * @note Важные замечания об использовании
 * @warning Предупреждения о потенциальных проблемах
 * 
 * @example
 * @code
 * VulkanRenderer renderer;
 * if (renderer.initialize(1920, 1080)) {
 *     renderer.renderFrame(sceneData, cameraParams);
 * }
 * @endcode
 * 
 * @see VulkanContext
 * @see RenderingPipeline
 * @since v1.0.0
 */
bool initialize(int width, int height);
```

### Комментарии в коде
```cpp
void complexFunction() {
    // Инициализация Vulkan instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    
    // TODO: Добавить поддержку validation layers в debug режиме
    // FIXME: Исправить утечку памяти при ошибке инициализации
    // NOTE: Этот код работает только с Vulkan 1.2+
    
    /* 
     * Многострочный комментарий для объяснения
     * сложной логики или алгоритма
     */
    
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
}
```

## 🧪 Тестирование

### Структура тестов
```cpp
#include <gtest/gtest.h>
#include "SpectraForge/Math/Vector3.h"

using namespace SpectraForge::Math;

class Vector3Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Настройка перед каждым тестом
        testVector = Vector3(1.0f, 2.0f, 3.0f);
    }
    
    void TearDown() override {
        // Очистка после каждого теста
    }
    
    Vector3 testVector;
};

TEST_F(Vector3Test, DefaultConstructor) {
    // Arrange
    Vector3 vec;
    
    // Act & Assert
    EXPECT_FLOAT_EQ(vec.x, 0.0f);
    EXPECT_FLOAT_EQ(vec.y, 0.0f);
    EXPECT_FLOAT_EQ(vec.z, 0.0f);
}

TEST_F(Vector3Test, CrossProduct) {
    // Arrange
    Vector3 other(4.0f, 5.0f, 6.0f);
    Vector3 expected(-3.0f, 6.0f, -3.0f);
    
    // Act
    Vector3 result = testVector.cross(other);
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, expected.x);
    EXPECT_FLOAT_EQ(result.y, expected.y);
    EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST_F(Vector3Test, NormalizationThrowsOnZeroVector) {
    // Arrange
    Vector3 zeroVector(0.0f, 0.0f, 0.0f);
    
    // Act & Assert
    EXPECT_THROW(zeroVector.normalize(), std::runtime_error);
}
```

### Именование тестов
```cpp
// ✅ Правильно: описательные имена тестов
TEST(VulkanRendererTest, InitializeSucceedsWithValidParameters)
TEST(VulkanRendererTest, InitializeFailsWithInvalidDevice)
TEST(VulkanRendererTest, RenderFrameThrowsWhenNotInitialized)

// ❌ Неправильно: неясные имена
TEST(VulkanRendererTest, Test1)
TEST(VulkanRendererTest, TestInit)
TEST(VulkanRendererTest, TestRender)
```

## 🔧 Инструменты качества кода

### clang-format конфигурация
```yaml
# .clang-format
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
BreakBeforeBraces: Allman
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
```

### clang-tidy конфигурация
```yaml
# .clang-tidy
Checks: >
  -*,
  bugprone-*,
  cert-*,
  cppcoreguidelines-*,
  google-*,
  hicpp-*,
  misc-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: camelBack
  - key: readability-identifier-naming.VariableCase
    value: camelBack
  - key: readability-identifier-naming.PrivateMemberPrefix
    value: m_
```

## 📋 Чек-лист качества кода

### Перед коммитом
- [ ] Код соответствует стандартам именования
- [ ] Все переменные для вывода обернуты в `SAFE_TO_STRING()`
- [ ] Используются умные указатели вместо сырых
- [ ] Добавлены Doxygen комментарии для публичных методов
- [ ] Написаны unit тесты для новой функциональности
- [ ] Код прошел проверку clang-format и clang-tidy
- [ ] Нет предупреждений компилятора
- [ ] Проведено code review

### Архитектурные требования
- [ ] Соблюдены принципы SOLID
- [ ] Использованы подходящие паттерны проектирования
- [ ] Минимизированы зависимости между модулями
- [ ] Обеспечена exception safety
- [ ] Реализован RAII для управления ресурсами

## 🚫 Антипаттерны

### Что НЕ делать
```cpp
// ❌ Небезопасный вывод
std::cout << myVariable << std::endl;

// ❌ Сырые указатели
MyClass* obj = new MyClass();

// ❌ Игнорирование ошибок
vkCreateInstance(&createInfo, nullptr, &instance); // Не проверяем результат

// ❌ Магические числа
if (value > 42) { /* ... */ }

// ❌ Длинные функции
void doEverything() {
    // 200+ строк кода
}

// ❌ Глобальные переменные
int globalCounter = 0;

// ❌ Использование using namespace в заголовках
using namespace std; // В .h файле

// ❌ Неинформативные имена
void f(int x, int y);
int a, b, c;
```

### Что делать ВМЕСТО этого
```cpp
// ✅ Безопасный вывод
std::cout << SAFE_TO_STRING(myVariable) << std::endl;

// ✅ Умные указатели
auto obj = std::make_unique<MyClass>();

// ✅ Проверка ошибок
VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

// ✅ Именованные константы
const int MAX_CONNECTIONS = 42;
if (value > MAX_CONNECTIONS) { /* ... */ }

// ✅ Короткие, сфокусированные функции
void initializeVulkan();
void setupRenderPipeline();
void createCommandBuffers();

// ✅ Инкапсуляция состояния
class ConnectionManager {
private:
    int m_connectionCount = 0;
};

// ✅ Локальное использование using
void myFunction() {
    using namespace std::chrono;
    // ...
}

// ✅ Описательные имена
void calculateDistance(const Vector3& point1, const Vector3& point2);
int connectionCount, maxConnections, activeUsers;
```

## 🎯 Заключение

Соблюдение этих стандартов кодирования обеспечивает:

1. **Читаемость**: Код легко понимать и поддерживать
2. **Безопасность**: Минимизация ошибок времени выполнения
3. **Производительность**: Эффективное использование ресурсов
4. **Масштабируемость**: Легкость расширения и модификации
5. **Качество**: Высокие стандарты разработки

Помните: хороший код пишется не только для компилятора, но и для других разработчиков (включая вас в будущем). Инвестиции в качество кода окупаются снижением времени на отладку и поддержку.
