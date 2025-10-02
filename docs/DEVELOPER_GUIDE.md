# Руководство разработчика SpectraForge

## 🚀 Быстрый старт

### Требования к системе
- Visual Studio 2019/2022 или GCC 10+
- CMake 3.16+
- vcpkg для управления зависимостями
- Git для контроля версий
- CUDA Toolkit 11.0+ (для GPU ускорения)
- Vulkan SDK (для рендеринга)

### Установка окружения разработки

1. **Клонирование репозитория:**
```bash
git clone https://github.com/TiGRoNdev/SpectraForge.git
cd SpectraForge
git submodule update --init --recursive
```

2. **Настройка vcpkg:**
```bash
./vcpkg/bootstrap-vcpkg.bat  # Windows
./vcpkg/bootstrap-vcpkg.sh   # Linux/macOS
```

3. **Сборка проекта:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Структура проекта

```
SpectraForge/
├── 📁 src/                     # Исходный код
│   ├── core/                   # Основные компоненты
│   ├── math/                   # Математическая библиотека
│   ├── rendering/              # Система рендеринга
│   ├── physics/                # Физическая система
│   └── input/                  # Система ввода
├── 📁 src3D/                   # 3D движок
│   ├── Engine3D.cpp           # Основной класс движка
│   ├── VulkanRenderer.cpp     # Vulkan рендерер
│   └── ResourceManager.cpp    # Менеджер ресурсов
├── 📁 srcVulkan/              # Vulkan специфичный код
│   ├── VulkanContext.cpp      # Контекст Vulkan
│   ├── VulkanBuffer.cpp       # Буферы Vulkan
│   └── cuda_kernels.cu        # CUDA ядра
├── 📁 include/                 # Публичные заголовки
│   ├── Engine3D/              # Заголовки 3D движка
│   └── SpectraForge/           # Основные заголовки
├── 📁 tests/                   # Тесты
│   ├── unit/                   # Модульные тесты
│   ├── integration/            # Интеграционные тесты
│   └── performance/            # Тесты производительности
├── 📁 examples/                # Примеры использования
├── 📁 docs/                    # Документация
└── 📁 tools/                   # Инструменты разработки
```

## 🔧 Процесс разработки

### Workflow разработки

1. **Создание ветки для новой функции:**
```bash
git checkout -b feature/my-new-feature
```

2. **Разработка с TDD подходом:**
   - Написать тест
   - Реализовать минимальный код для прохождения теста
   - Рефакторинг

3. **Проверка качества кода:**
```bash
# Форматирование кода
clang-format -i src/**/*.cpp include/**/*.h

# Статический анализ
clang-tidy src/**/*.cpp

# Запуск тестов
cd build && ctest --output-on-failure
```

4. **Создание Pull Request:**
   - Убедиться, что все тесты проходят
   - Обновить документацию при необходимости
   - Описать изменения в PR

### Соглашения о коде

#### Именование
```cpp
// Классы: PascalCase
class VulkanRenderer {};

// Функции и переменные: camelCase
void renderFrame();
int frameCount;

// Константы: UPPER_CASE
const int MAX_VERTEX_COUNT = 1000;

// Namespace: PascalCase
namespace SpectraForge::Rendering {}
```

#### Структура классов
```cpp
class MyClass {
public:
    // Конструкторы
    MyClass();
    explicit MyClass(int value);
    
    // Деструктор
    ~MyClass();
    
    // Публичные методы
    void publicMethod();
    
private:
    // Приватные методы
    void privateMethod();
    
    // Поля данных (с префиксом m_)
    int m_value;
    std::string m_name;
};
```

#### Комментарии и документация
```cpp
/**
 * @brief Краткое описание функции
 * 
 * Подробное описание того, что делает функция.
 * 
 * @param input Описание параметра input
 * @param output Описание параметра output
 * @return Описание возвращаемого значения
 * 
 * @throws std::runtime_error Когда происходит ошибка
 * 
 * @example
 * @code
 * MyClass obj;
 * obj.myFunction(42, result);
 * @endcode
 */
int myFunction(int input, std::string& output);
```

#### Безопасный вывод в консоль
```cpp
#include "SpectraForge/Core/SafeConsole.h"

// Всегда используйте SAFE_TO_STRING для вывода переменных
std::cout << "Значение: " << SAFE_TO_STRING(myVariable) << std::endl;

// Вместо небезопасного:
// std::cout << "Значение: " << myVariable << std::endl;
```

## 🧪 Тестирование

### Структура тестов
```cpp
#include "TestFramework.h"
#include "SpectraForge/Math/Vector3.h"

using namespace SpectraForge::Testing;

class Vector3Test : public SpectraForgeTest {
protected:
    void SetUp() override {
        SpectraForgeTest::SetUp();
        // Настройка для тестов
    }
    
    void TearDown() override {
        // Очистка после тестов
        SpectraForgeTest::TearDown();
    }
    
    Math::Vector3 testVector{1.0f, 2.0f, 3.0f};
};

TEST_F(Vector3Test, CrossProductCalculation) {
    // Arrange
    Math::Vector3 other{4.0f, 5.0f, 6.0f};
    
    // Act
    Math::Vector3 result = testVector.cross(other);
    
    // Assert
    EXPECT_FLOAT_EQ(result.x, -3.0f);
    EXPECT_FLOAT_EQ(result.y, 6.0f);
    EXPECT_FLOAT_EQ(result.z, -3.0f);
}
```

### Тестирование с Mock объектами
```cpp
TEST_F(RendererTest, InitializationWithMockDependencies) {
    // Arrange
    auto mockResourceManager = std::make_shared<MockResourceManager>();
    auto mockHardwareDetector = std::make_shared<MockHardwareDetector>();
    
    EXPECT_CALL(*mockHardwareDetector, detectVendor())
        .WillOnce(Return(VendorType::NVIDIA));
    
    VulkanRenderer renderer(mockResourceManager, mockHardwareDetector);
    
    // Act & Assert
    EXPECT_TRUE(renderer.initialize());
}
```

## 🎮 Архитектура движка

### Основные компоненты

#### Engine3D - Главный класс движка
```cpp
class Engine3D {
public:
    bool initialize();
    void run();
    void shutdown();
    
private:
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<SceneManager> m_sceneManager;
};
```

#### VulkanRenderer - Система рендеринга
```cpp
class VulkanRenderer {
public:
    bool initialize(const RendererConfig& config);
    void renderFrame();
    void cleanup();
    
private:
    VulkanContext m_context;
    std::vector<VulkanBuffer> m_buffers;
    VulkanPipeline m_pipeline;
};
```

#### ResourceManager - Управление ресурсами
```cpp
class ResourceManager {
public:
    template<typename T>
    std::shared_ptr<T> loadResource(const std::string& path);
    
    void unloadResource(const std::string& path);
    void cleanup();
    
private:
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
};
```

### CUDA-Vulkan интеграция

```cpp
// Пример использования CUDA ядер для обработки данных
class CudaVulkanInterop {
public:
    bool initializeInterop();
    void processVertexData(const std::vector<Vertex>& vertices);
    
private:
    cudaExternalMemory_t m_externalMemory;
    VkDeviceMemory m_vulkanMemory;
};
```

## 🚀 Развертывание и релиз

### Создание релиза

1. **Обновление версии:**
```cmake
# В CMakeLists.txt
set(HYPERENGINE_VERSION_MAJOR 1)
set(HYPERENGINE_VERSION_MINOR 2)
set(HYPERENGINE_VERSION_PATCH 0)
```

2. **Обновление CHANGELOG.md:**
```markdown
## [1.2.0] - 2024-01-15

### Added
- Новая система рендеринга
- Поддержка DirectX 12

### Changed
- Улучшена производительность на 25%

### Fixed
- Исправлена утечка памяти в TextureManager
```

3. **Создание тега:**
```bash
git tag -a v1.2.0 -m "Release version 1.2.0"
git push origin v1.2.0
```

4. **GitHub Release:**
   - Автоматически создается через GitHub Actions
   - Включает скомпилированные бинарии для всех платформ
   - Содержит release notes из CHANGELOG.md

## 🐛 Отладка и профилирование

### Использование встроенного профайлера
```cpp
#include "SpectraForge/Core/Profiler.h"

void myFunction() {
    PROFILE_SCOPE("MyFunction");
    
    // Ваш код здесь
    {
        PROFILE_SCOPE("NestedOperation");
        // Вложенная операция
    }
}
```

### Анализ производительности
```bash
# Сборка с профилированием
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_PROFILING=ON

# Запуск с профайлером
./SpectraForge_Demo --profile --profile-output=profile.json

# Анализ результатов
python tools/analyze_profile.py profile.json
```

### Отладка CUDA кода
```bash
# Компиляция с отладочной информацией
nvcc -g -G -O0 src/cuda_kernels.cu

# Использование cuda-gdb
cuda-gdb ./SpectraForge_Demo
```

### Отладка Vulkan
```bash
# Включение validation layers
export VK_LAYER_PATH=/path/to/vulkan/layers
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation

# Запуск с RenderDoc для анализа кадров
renderdoc ./SpectraForge_Demo
```

## 🔧 Инструменты разработки

### Сборка проекта
```bash
# Быстрая сборка
./build_with_vcpkg.bat

# Сборка с тестами
cmake -DBUILD_TESTING=ON ..
cmake --build . --target all

# Сборка только CUDA ядер
./compile_cuda_kernels.bat
```

### Компиляция шейдеров
```bash
# Компиляция всех шейдеров
./compile_shaders.bat

# Компиляция конкретного шейдера
glslc shaders/vertex.vert -o shaders/vertex.spv
```

### Интеграция CUDA библиотек
```bash
# Интеграция скомпилированных CUDA ядер
./integrate_cuda_library.bat
```

## 📚 Дополнительные ресурсы

- [API Reference](api/API_Reference.md) - Полная справка по API
- [Architecture Guide](architecture/ARCHITECTURE.md) - Архитектурное руководство
- [Performance Guidelines](guides/PERFORMANCE.md) - Рекомендации по производительности
- [Coding Standards](guides/CODING_STANDARDS.md) - Стандарты кодирования
- [Examples](guides/Examples.md) - Примеры использования
- [CUDA-Vulkan Interop](guides/CUDA_VULKAN_INTEROP_REPORT.md) - Руководство по интеграции CUDA и Vulkan

## 🤝 Участие в разработке

### Процесс внесения изменений

1. Форкните репозиторий
2. Создайте ветку для вашей функции (`git checkout -b feature/AmazingFeature`)
3. Зафиксируйте изменения (`git commit -m 'Add some AmazingFeature'`)
4. Отправьте в ветку (`git push origin feature/AmazingFeature`)
5. Откройте Pull Request

### Стандарты качества

- Все новые функции должны иметь тесты
- Покрытие кода должно быть не менее 80%
- Код должен проходить статический анализ
- Документация должна быть обновлена

### Сообщество

- [GitHub Issues](https://github.com/TiGRoNdev/SpectraForge/issues) - Сообщения об ошибках и запросы функций
- [GitHub Discussions](https://github.com/TiGRoNdev/SpectraForge/discussions) - Общие вопросы и обсуждения

---

*Этот документ регулярно обновляется. Последнее обновление: январь 2024*
