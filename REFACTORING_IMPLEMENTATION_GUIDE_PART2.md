# 📋 Руководство по реализации рефакторинга HyperEngine - Часть 2

*Продолжение основного руководства. Этапы 5-8.*

---

## 📋 Этап 5: Настройка CI/CD пайплайна

### 5.1 Создание GitHub Actions workflow

Создать `.github/workflows/ci-cd.yml`:

```yaml
name: HyperEngine CI/CD Pipeline

on:
  push:
    branches: [ main, develop, refactoring/* ]
  pull_request:
    branches: [ main, develop ]
  release:
    types: [ published ]

env:
  BUILD_TYPE: Release
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"

jobs:
  # Проверка качества кода
  code-quality:
    name: Code Quality Checks
    runs-on: ubuntu-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        submodules: recursive
    
    - name: Setup C++ environment
      uses: aminya/setup-cpp@v1
      with:
        compiler: gcc-11
        cmake: true
        ninja: true
        clang-tidy: true
        cppcheck: true
    
    - name: Cache vcpkg
      uses: actions/cache@v3
      with:
        path: |
          vcpkg_installed
          vcpkg
        key: ${{ runner.os }}-vcpkg-${{ hashFiles('vcpkg.json') }}
    
    - name: Run clang-format check
      run: |
        find src include tests -name "*.cpp" -o -name "*.h" | \
        xargs clang-format --dry-run --Werror --style=file
    
    - name: Run clang-tidy
      run: |
        cmake -B build-lint -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        find src include -name "*.cpp" | head -20 | \
        xargs clang-tidy --config-file=.clang-tidy -p build-lint
    
    - name: Run cppcheck
      run: |
        cppcheck --enable=all --error-exitcode=1 \
          --suppress=missingIncludeSystem \
          --suppress=unusedFunction \
          src/ include/

  # Сборка и тестирование на разных платформах
  build-and-test:
    name: Build and Test
    needs: code-quality
    strategy:
      fail-fast: false
      matrix:
        os: [ubuntu-22.04, windows-2022, macos-12]
        build-type: [Debug, Release]
        include:
          - os: ubuntu-22.04
            triplet: x64-linux
          - os: windows-2022
            triplet: x64-windows
          - os: macos-12
            triplet: x64-osx
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        submodules: recursive
    
    - name: Setup vcpkg
      uses: lukka/run-vcpkg@v11
      with:
        vcpkgGitCommitId: 'latest'
    
    - name: Configure CMake
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=${{ matrix.build-type }} \
          -DCMAKE_TOOLCHAIN_FILE=${{ github.workspace }}/vcpkg/scripts/buildsystems/vcpkg.cmake \
          -DVCPKG_TARGET_TRIPLET=${{ matrix.triplet }} \
          -DBUILD_TESTING=ON \
          -DBUILD_BENCHMARKS=ON
    
    - name: Build
      run: cmake --build build --config ${{ matrix.build-type }} --parallel
    
    - name: Run tests
      working-directory: build
      run: ctest --output-on-failure --parallel --build-config ${{ matrix.build-type }}
    
    - name: Generate coverage report (Linux Debug only)
      if: matrix.os == 'ubuntu-22.04' && matrix.build-type == 'Debug'
      run: |
        cmake -B build-coverage \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="--coverage" \
          -DCMAKE_TOOLCHAIN_FILE=${{ github.workspace }}/vcpkg/scripts/buildsystems/vcpkg.cmake \
          -DBUILD_TESTING=ON
        
        cmake --build build-coverage --parallel
        cd build-coverage
        ctest --output-on-failure
        
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' '*/vcpkg/*' '*/tests/*' --output-file coverage_filtered.info
    
    - name: Upload coverage to Codecov
      if: matrix.os == 'ubuntu-22.04' && matrix.build-type == 'Debug'
      uses: codecov/codecov-action@v3
      with:
        files: build-coverage/coverage_filtered.info
        fail_ci_if_error: false

  # Тесты производительности
  performance-tests:
    name: Performance Tests
    needs: build-and-test
    runs-on: ubuntu-22.04
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        submodules: recursive
    
    - name: Setup vcpkg
      uses: lukka/run-vcpkg@v11
      with:
        vcpkgGitCommitId: 'latest'
    
    - name: Build performance tests
      run: |
        cmake -B build-perf \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_TOOLCHAIN_FILE=${{ github.workspace }}/vcpkg/scripts/buildsystems/vcpkg.cmake \
          -DBUILD_BENCHMARKS=ON
        
        cmake --build build-perf --parallel
    
    - name: Run benchmarks
      run: |
        cd build-perf
        ./tests/performance/math_benchmarks --benchmark_format=json > benchmark_results.json
    
    - name: Store benchmark results
      uses: benchmark-action/github-action-benchmark@v1
      with:
        tool: 'googlecpp'
        output-file-path: build-perf/benchmark_results.json
        github-token: ${{ secrets.GITHUB_TOKEN }}
        auto-push: true
        comment-on-alert: true
        alert-threshold: '150%'

  # Анализ безопасности
  security-scan:
    name: Security Scan
    runs-on: ubuntu-latest
    permissions:
      security-events: write
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v4
    
    - name: Run Trivy vulnerability scanner
      uses: aquasecurity/trivy-action@master
      with:
        scan-type: 'fs'
        scan-ref: '.'
        format: 'sarif'
        output: 'trivy-results.sarif'
    
    - name: Upload Trivy scan results
      uses: github/codeql-action/upload-sarif@v2
      if: always()
      with:
        sarif_file: 'trivy-results.sarif'
```

### 5.2 Настройка pre-commit hooks

Обновить `.pre-commit-config.yaml`:

```yaml
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: trailing-whitespace
        exclude: \.(md|rst)$
      - id: end-of-file-fixer
        exclude: \.(md|rst)$
      - id: check-yaml
      - id: check-added-large-files
        args: ['--maxkb=1000']
      - id: check-merge-conflict
      - id: mixed-line-ending

  - repo: https://github.com/pocc/pre-commit-hooks
    rev: v1.3.5
    hooks:
      - id: clang-format
        args: [--style=file, --fallback-style=none]
        types_or: [c++, c]
      - id: clang-tidy
        args: [--config-file=.clang-tidy]
        types_or: [c++, c]
        exclude: ^(tests/|examples/)
      - id: cppcheck
        args: [--enable=all, --suppress=missingIncludeSystem]
        types_or: [c++, c]

  - repo: https://github.com/cmake-lint/cmake-lint
    rev: 1.4.2
    hooks:
      - id: cmakelint
        args: [--line-width=100]

  - repo: local
    hooks:
      - id: copyright-check
        name: Check copyright headers
        entry: scripts/check_copyright.sh
        language: script
        types_or: [c++, c]
```

### 5.3 Валидация этапа 5

Чек-лист этапа 5:
- [ ] Создан полный CI/CD пайплайн в GitHub Actions
- [ ] Настроена проверка качества кода
- [ ] Добавлена сборка на множественных платформах
- [ ] Настроен сбор метрик покрытия кода
- [ ] Добавлены тесты производительности в пайплайн
- [ ] Настроен анализ безопасности
- [ ] Обновлены pre-commit hooks

```bash
# Настроить pre-commit hooks
pip install pre-commit
pre-commit install

# Коммит изменений этапа 5
git add .
git commit -m "Этап 5: Настройка CI/CD пайплайна

- Создан полный GitHub Actions workflow
- Добавлена проверка качества кода
- Настроена сборка на множественных платформах
- Добавлены тесты производительности
- Настроен анализ безопасности
- Обновлены pre-commit hooks"
```

---

## 📋 Этап 6: Улучшение документации

### 6.1 Настройка автогенерации API документации

Обновить `Doxyfile`:

```
# Настройки Doxygen для HyperEngine
PROJECT_NAME           = "HyperEngine"
PROJECT_NUMBER         = "v1.0.0"
PROJECT_BRIEF          = "Современный игровой движок с поддержкой 3D и экспериментального 4D рендеринга"

# Входные файлы
INPUT                  = include/ src/ docs/
FILE_PATTERNS          = *.h *.hpp *.cpp *.md
RECURSIVE              = YES
EXCLUDE                = build*/ vcpkg*/ tests/ examples/

# Выходная документация
OUTPUT_DIRECTORY       = docs/generated/
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
HTML_OUTPUT            = html
HTML_THEME             = Doxygen

# Настройки обработки
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
EXTRACT_LOCAL_CLASSES  = YES

# Диаграммы и графики
HAVE_DOT               = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
CLASS_DIAGRAMS         = YES
UML_LOOK               = YES
COLLABORATION_GRAPH    = YES

# Markdown поддержка
MARKDOWN_SUPPORT       = YES
AUTOLINK_SUPPORT       = YES

# Настройки качества
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = YES
```

### 6.2 Создание руководства разработчика

Создать `docs/DEVELOPER_GUIDE.md`:

```markdown
# Руководство разработчика HyperEngine

## 🚀 Быстрый старт

### Требования к системе
- Visual Studio 2019/2022 или GCC 10+
- CMake 3.16+
- vcpkg для управления зависимостями
- Git для контроля версий

### Установка окружения разработки

1. **Клонирование репозитория:**
```bash
git clone https://github.com/username/HyperEngine.git
cd HyperEngine
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
HyperEngine/
├── 📁 src/                     # Исходный код
│   ├── core/                   # Основные компоненты
│   ├── math/                   # Математическая библиотека
│   ├── rendering/              # Система рендеринга
│   ├── physics/                # Физическая система
│   └── input/                  # Система ввода
├── 📁 include/                 # Публичные заголовки
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
namespace HyperEngine::Rendering {}
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

## 🧪 Тестирование

### Структура тестов
```cpp
#include "TestFramework.h"
#include "HyperEngine/Math/Vector3.h"

using namespace HyperEngine::Testing;

class Vector3Test : public HyperEngineTest {
protected:
    void SetUp() override {
        HyperEngineTest::SetUp();
        // Настройка для тестов
    }
    
    void TearDown() override {
        // Очистка после тестов
        HyperEngineTest::TearDown();
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
#include "HyperEngine/Core/Profiler.h"

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
./HyperEngine_Demo --profile --profile-output=profile.json

# Анализ результатов
python tools/analyze_profile.py profile.json
```

## 📚 Дополнительные ресурсы

- [API Reference](docs/api/API_Reference.md) - Полная справка по API
- [Architecture Guide](docs/architecture/ARCHITECTURE.md) - Архитектурное руководство
- [Performance Guidelines](docs/guides/PERFORMANCE.md) - Рекомендации по производительности
- [Coding Standards](docs/guides/CODING_STANDARDS.md) - Стандарты кодирования
```

### 6.3 Валидация этапа 6

Чек-лист этапа 6:
- [ ] Настроена автогенерация API документации с Doxygen
- [ ] Создано руководство разработчика
- [ ] Обновлена архитектурная документация
- [ ] Созданы примеры использования
- [ ] Настроена автоматическая публикация документации

```bash
# Генерация документации
doxygen Doxyfile

# Коммит изменений этапа 6
git add .
git commit -m "Этап 6: Улучшение документации

- Настроена автогенерация API документации
- Создано руководство разработчика
- Обновлена архитектурная документация
- Созданы подробные примеры использования"
```

---

*Часть 2 завершена. Продолжение в части 3 с этапами 7-8.*
