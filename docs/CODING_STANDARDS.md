# Стандарты кодирования SpectraForge

## Обзор

Этот документ описывает стандарты кодирования для проекта SpectraForge. Соблюдение этих стандартов обеспечивает читаемость, поддерживаемость и качество кода.

## Содержание

- [Общие принципы](#общие-принципы)
- [Форматирование кода](#форматирование-кода)
- [Соглашения об именовании](#соглашения-об-именовании)
- [Структура файлов](#структура-файлов)
- [Комментарии и документация](#комментарии-и-документация)
- [Безопасность](#безопасность)
- [Инструменты разработки](#инструменты-разработки)
- [Автоматизация](#автоматизация)

## Общие принципы

### SOLID принципы
- **Single Responsibility Principle (SRP)**: Каждый класс должен иметь только одну причину для изменения
- **Open/Closed Principle (OCP)**: Классы должны быть открыты для расширения, но закрыты для модификации
- **Liskov Substitution Principle (LSP)**: Производные классы должны быть заменяемы базовыми
- **Interface Segregation Principle (ISP)**: Клиенты не должны зависеть от интерфейсов, которые они не используют
- **Dependency Inversion Principle (DIP)**: Зависимости должны быть от абстракций, а не от конкретных реализаций

### Современный C++
- Используйте C++20 стандарт
- Предпочитайте умные указатели сырым указателям
- Используйте RAII для управления ресурсами
- Применяйте const correctness
- Используйте auto где это улучшает читаемость

## Форматирование кода

### Основные правила
- **Отступы**: 4 пробела (не табы)
- **Максимальная длина строки**: 100 символов
- **Кодировка**: UTF-8
- **Окончания строк**: LF (Unix-style)

### Стиль скобок
```cpp
// Правильно
if (condition) {
    doSomething();
}

class MyClass {
public:
    void method();
};

// Неправильно
if (condition)
{
    doSomething();
}
```

### Пробелы
```cpp
// Правильно
int result = calculate(a, b, c);
if (x > 0 && y < 10) {
    // код
}

// Неправильно
int result=calculate(a,b,c);
if(x>0&&y<10){
    // код
}
```

### Автоматическое форматирование
Проект использует `clang-format` для автоматического форматирования. Конфигурация находится в файле `.clang-format`.

**Команды для форматирования:**
```bash
# Форматирование всех файлов
./scripts/format_code.bat  # Windows
./scripts/format_code.sh   # Linux/macOS

# Форматирование конкретного файла
clang-format -i --style=file src/MyFile.cpp
```

## Соглашения об именовании

### Классы и структуры
```cpp
class VulkanRenderer {        // PascalCase
public:
    struct BufferInfo {       // PascalCase для вложенных типов
        size_t size;
        void* data;
    };
};
```

### Функции и методы
```cpp
void calculateMatrix();       // camelCase
bool isValid() const;         // camelCase
void setPosition(float x);    // camelCase
```

### Переменные
```cpp
int frameCount = 0;           // camelCase
float deltaTime = 0.0f;       // camelCase
bool isInitialized = false;   // camelCase
```

### Константы
```cpp
const int kMaxBufferSize = 1024;           // kPascalCase
constexpr float kPi = 3.14159f;            // kPascalCase
static const char* kDefaultShaderPath;     // kPascalCase
```

### Макросы
```cpp
#define HYPERENGINE_VERSION_MAJOR 1        // UPPER_CASE
#define SAFE_TO_STRING(x) std::to_string(x) // UPPER_CASE
```

### Пространства имен
```cpp
namespace SpectraForge {
    namespace Vulkan {
        class Renderer { /* ... */ };
    }
}
```

## Структура файлов

### Заголовочные файлы (.h)
```cpp
#pragma once

// Системные заголовки
#include <vector>
#include <memory>

// Сторонние библиотеки
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

// Локальные заголовки
#include "SpectraForge/Core/Base.h"
#include "SpectraForge/Renderer/Buffer.h"

namespace SpectraForge {

/**
 * @brief Краткое описание класса
 * 
 * Подробное описание функциональности класса.
 */
class MyClass {
public:
    MyClass();
    ~MyClass();
    
    // Публичные методы
    void publicMethod();
    
private:
    // Приватные члены
    int m_privateVariable;
};

} // namespace SpectraForge
```

### Исходные файлы (.cpp)
```cpp
#include "SpectraForge/MyClass.h"

// Системные заголовки
#include <iostream>
#include <algorithm>

// Сторонние библиотеки
#include <spdlog/spdlog.h>

// Локальные заголовки
#include "SpectraForge/Core/Logger.h"

using namespace SpectraForge;

MyClass::MyClass() : m_privateVariable(0) {
    // Инициализация
}

MyClass::~MyClass() {
    // Очистка ресурсов
}

void MyClass::publicMethod() {
    // Реализация
}
```

### Include Guards
Используйте `#pragma once` вместо традиционных include guards:

```cpp
// Правильно
#pragma once

// Неправильно
#ifndef HYPERENGINE_MYCLASS_H
#define HYPERENGINE_MYCLASS_H
// ...
#endif
```

## Комментарии и документация

### Doxygen комментарии
```cpp
/**
 * @brief Рендерит кадр с использованием Vulkan API
 * 
 * @param deltaTime Время с предыдущего кадра в секундах
 * @param camera Камера для рендеринга
 * @return true если рендеринг прошел успешно, false в противном случае
 * 
 * @note Этот метод должен вызываться в основном потоке
 * @warning Убедитесь, что Vulkan контекст инициализирован
 */
bool renderFrame(float deltaTime, const Camera& camera);
```

### Inline комментарии
```cpp
// Вычисляем матрицу проекции для 4D пространства
Matrix4D projectionMatrix = calculateProjection4D(fov, aspect, near, far);

/* 
 * Многострочный комментарий для сложной логики
 * Объясняет алгоритм или нестандартное решение
 */
```

### TODO и FIXME
```cpp
// TODO(#123): Оптимизировать алгоритм сортировки
// FIXME(#456): Исправить утечку памяти в деструкторе
```

## Безопасность

### Избегайте небезопасных функций
```cpp
// Неправильно
char buffer[256];
strcpy(buffer, source);  // Небезопасно!

// Правильно
std::string buffer = source;
// или
std::strncpy(buffer, source, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

### Используйте умные указатели
```cpp
// Неправильно
MyClass* obj = new MyClass();
// ... забыли delete

// Правильно
auto obj = std::make_unique<MyClass>();
// или
std::shared_ptr<MyClass> obj = std::make_shared<MyClass>();
```

### Безопасный вывод в консоль
```cpp
// Неправильно
std::cout << "Value: " << someVariable << std::endl;

// Правильно
std::cout << "Value: " << SAFE_TO_STRING(someVariable) << std::endl;
```

## Инструменты разработки

### Настройка IDE

#### Visual Studio Code
Проект включает настройки для VS Code в `.vscode/settings.json`:
- Автоматическое форматирование при сохранении
- Интеграция с clang-format и clang-tidy
- Настройки IntelliSense для C++

#### Visual Studio
Используйте файл `.editorconfig` для настройки форматирования.

### Статический анализ
```bash
# Запуск clang-tidy
clang-tidy src/**/*.cpp --config-file=.clang-tidy

# Запуск cppcheck
cppcheck --enable=all --std=c++20 src/ include/
```

## Автоматизация

### Pre-commit hooks
Проект использует pre-commit hooks для автоматической проверки:

```bash
# Установка pre-commit hooks
pip install pre-commit
pre-commit install

# Запуск всех проверок
pre-commit run --all-files
```

### Git hooks
Установите локальные git hooks:

```bash
# Linux/macOS
./scripts/setup_git_hooks.sh

# Windows
scripts\setup_git_hooks.bat
```

### CI/CD
GitHub Actions автоматически проверяет:
- Форматирование кода
- Статический анализ
- Сборку проекта
- Запуск тестов

## Проверка качества кода

### Локальная проверка
```bash
# Полная проверка качества
./scripts/quality_check.sh

# Быстрая проверка перед коммитом
./scripts/pre_commit_check.sh

# Форматирование кода
./scripts/format_code.bat  # Windows
```

### Метрики качества
- **Покрытие тестами**: минимум 80%
- **Цикломатическая сложность**: максимум 10
- **Дублирование кода**: максимум 3%
- **Технический долг**: рейтинг A

## Исключения из правил

В редких случаях возможны исключения из правил:
1. Документируйте причину исключения
2. Получите одобрение команды
3. Добавьте комментарий в код
4. Регулярно пересматривайте необходимость исключения

```cpp
// EXCEPTION: Используем сырой указатель для совместимости с C API
// TODO(#789): Заменить на умный указатель после обновления библиотеки
VkDevice* rawDevice = getVulkanDevice();
```

## Ресурсы

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Clang-Format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [Doxygen Documentation](https://www.doxygen.nl/manual/)

## Обновления

Этот документ регулярно обновляется. Последние изменения см. в [CHANGELOG.md](../CHANGELOG.md).

---

*Версия документа: 2.0*  
*Последнее обновление: 2024*
