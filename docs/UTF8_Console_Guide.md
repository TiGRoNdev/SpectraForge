# 🌐 Руководство по UTF-8 консоли HyperEngine

## 📋 Обзор

HyperEngine теперь включает полную поддержку отображения всех Unicode UTF-8 символов в консоли. Эта система обеспечивает кроссплатформенную совместимость и красивый вывод с эмодзи, цветами и любыми символами Unicode.

## ✨ Основные возможности

### 🎯 Поддерживаемые функции

- ✅ **Полная поддержка UTF-8**: Отображение всех Unicode символов
- ✅ **Эмодзи**: 😀🚀🌟💎🔥 и тысячи других
- ✅ **Цветной вывод**: 16 различных цветов для текста
- ✅ **Логирование с эмодзи**: Автоматические эмодзи для разных уровней
- ✅ **Кроссплатформенность**: Windows, Linux, macOS
- ✅ **Кириллица**: Полная поддержка русского языка
- ✅ **Математические символы**: ∑∏∫√∞±≤≥≠∈∉∀∃
- ✅ **Специальные символы**: ™®©½¼¾⅓⅔⅛⅜⅝⅞

## 🚀 Быстрый старт

### 1. Инициализация

```cpp
#include "Engine3D/Core/Console.h"

int main() {
    // Инициализация UTF-8 консоли
    Engine3D::Core::Console::initialize();
    
    // Ваш код здесь...
    
    return 0;
}
```

### 2. Базовый вывод

```cpp
using namespace Engine3D::Core;

// Простой текст с эмодзи
std::cout << "🚀 Добро пожаловать в HyperEngine!" << std::endl;

// Цветной текст (теперь без цвета, используется std::cout)
std::cout << "Важное сообщение" << std::endl;

// Логирование
Console::info("✅ Система инициализирована");
Console::warning("⚠️ Низкая производительность");
Console::error("❌ Файл не найден");
```

## 📚 Подробная документация

### Класс Console

#### Инициализация

```cpp
// Полная инициализация с диагностикой
bool success = Console::initialize();
if (!success) {
    std::cerr << "Не удалось инициализировать UTF-8 консоль" << std::endl;
}

// Получение информации о консоли
std::cout << Console::getConsoleInfo() << std::endl;
```

#### Основные методы вывода

```cpp
// Базовый вывод
std::cout << "Текст" << std::endl; // с переводом строки
std::cout << "Текст"; // без перевода строки

// Установка заголовка окна консоли
Console::setTitle("🎮 Моя игра");

// Очистка консоли
Console::clear();
```

#### Система логирования

```cpp
// Уровни логирования с автоматическими эмодзи и цветами
Console::debug("Отладочная информация");      // 🐛 [DEBUG] синий
Console::info("Информация");                  // ℹ️ [INFO] зеленый
Console::warning("Предупреждение");           // ⚠️ [WARNING] желтый
Console::error("Ошибка");                     // ❌ [ERROR] красный
Console::critical("Критическая ошибка");      // 🚨 [CRITICAL] ярко-красный

// Или напрямую через уровень
Console::log(LogLevel::INFO, "Сообщение");
```

#### Проверка возможностей

```cpp
if (Console::supportsUTF8()) {
    std::cout << "UTF-8 поддерживается!" << std::endl;
}

if (Console::supportsColor()) {
    std::cout << "Цветной вывод доступен!" << std::endl;
}

if (Console::supportsEmoji()) {
    std::cout << "Эмодзи поддерживаются!" << std::endl;
}
```

### Цвета ConsoleColor

```cpp
enum class ConsoleColor {
    DEFAULT,        // По умолчанию
    BLACK,          // Черный
    RED,            // Красный
    GREEN,          // Зеленый
    YELLOW,         // Желтый
    BLUE,           // Синий
    MAGENTA,        // Пурпурный
    CYAN,           // Голубой
    WHITE,          // Белый
    BRIGHT_BLACK,   // Ярко-черный (серый)
    BRIGHT_RED,     // Ярко-красный
    BRIGHT_GREEN,   // Ярко-зеленый
    BRIGHT_YELLOW,  // Ярко-желтый
    BRIGHT_BLUE,    // Ярко-синий
    BRIGHT_MAGENTA, // Ярко-пурпурный
    BRIGHT_CYAN,    // Ярко-голубой
    BRIGHT_WHITE    // Ярко-белый
};
```

### Уровни логирования LogLevel

```cpp
enum class LogLevel {
    DEBUG = 0,      // 🐛 Отладка
    INFO = 1,       // ℹ️ Информация
    WARNING = 2,    // ⚠️ Предупреждение
    ERROR = 3,      // ❌ Ошибка
    CRITICAL = 4    // 🚨 Критическая ошибка
};
```

## 🎨 Примеры использования

### Игровые сообщения

```cpp
// Инициализация игры
Console::info("🎮 Запуск игры...");
Console::info("📚 Загрузка ресурсов...");
Console::info("🎨 Инициализация рендерера...");
Console::info("🎵 Настройка аудио...");
Console::info("✅ Игра готова к запуску!");

// Игровые события
std::cout << "🎉 Игрок получил новый уровень!" << std::endl;
std::cout << "💎 Найден редкий предмет!" << std::endl;
std::cout << "🏆 Достижение разблокировано!" << std::endl;

// Ошибки
Console::warning("⚠️ Низкий FPS: 15 кадров в секунду");
Console::error("❌ Не удалось подключиться к серверу");
Console::critical("🚨 Критическая ошибка памяти!");
```

### Прогресс и статистика

```cpp
// Загрузка с прогрессом
Console::info("📦 Загрузка уровня: 0%");
Console::info("📦 Загрузка уровня: ████░░░░░░ 40%");
Console::info("📦 Загрузка уровня: ████████░░ 80%");
Console::info("📦 Загрузка уровня: ██████████ 100% ✅");

// Статистика
std::cout << "📊 Статистика игры:" << std::endl;
std::cout << "   🎯 Очков: 1,234,567" << std::endl;
std::cout << "   ⏱️ Время: 12:34:56" << std::endl;
std::cout << "   🏃 Скорость: 25.3 м/с" << std::endl;
```

### Техническая информация

```cpp
// Системная информация
Console::debug("💻 ОС: Windows 11");
Console::debug("🎮 GPU: NVIDIA RTX 4090");
Console::debug("🧠 RAM: 32 GB");
Console::debug("💾 VRAM: 24 GB");

// Производительность
Console::info("⚡ FPS: 144");
Console::info("🖥️ Разрешение: 3840x2160");
Console::info("🎨 Качество: Ultra");
Console::warning("🔥 Температура GPU: 82°C");
```

## 🛠️ Интеграция с движком

### В основном приложении

```cpp
#include "Engine3D/Core/Console.h"
#include "Engine3D/Core/GameObject3D.h"
#include "Engine3D/Rendering/Renderer3D.h"

int main() {
    // Инициализация консоли - первым делом!
    Engine3D::Core::Console::initialize();
    Engine3D::Core::Console::setTitle("🚀 Моя игра на HyperEngine");
    
    Console::info("🔧 Инициализация движка...");
    
    // Инициализация рендерера
    if (!Renderer3D::getInstance().initialize(1920, 1080)) {
        Console::error("❌ Не удалось инициализировать рендерер");
        return -1;
    }
    Console::info("✅ Рендерер инициализирован");
    
    // Главный цикл
    Console::info("🔄 Запуск главного цикла...");
    while (running) {
        // Ваш код...
    }
    
    Console::info("🏁 Завершение работы");
    return 0;
}
```

### В системах движка

```cpp
// В системе рендеринга
void Renderer3D::beginFrame() {
    Console::debug("🎨 Начало кадра");
    // ...
}

// В системе физики
void PhysicsWorld::addRigidBody(RigidBody3D* body) {
    Console::debug("⚡ Добавлено физическое тело: " + body->getName());
    // ...
}

// В системе ввода
void InputManager::initialize() {
    Console::info("🎮 Система ввода инициализирована");
    // ...
}
```

## 🔧 Конфигурация

### Настройка для проекта

1. **Включите заголовок**:
```cpp
#include "Engine3D/Core/Console.h"
```

2. **Добавьте в CMakeLists.txt**:
```cmake
target_link_libraries(YourProject PRIVATE Engine3D)
```

3. **Инициализируйте в main()**:
```cpp
Engine3D::Core::Console::initialize();
```

### Макросы для удобства

```cpp
// Используйте готовые макросы
LOG_DEBUG("Отладочное сообщение");
LOG_INFO("Информационное сообщение");
LOG_WARNING("Предупреждение");
LOG_ERROR("Ошибка");
LOG_CRITICAL("Критическая ошибка");
```

## 🧪 Тестирование

### Демонстрация возможностей

```cpp
// Тест Unicode символов
Console::testUnicodeDisplay();

// Тест цветов
Console::testColorDisplay();

// Информация о консоли
std::cout << Console::getConsoleInfo() << std::endl;
```

### Запуск демо

```bash
# Запуск демонстрационного приложения
./UTF8Console_Demo

# Обновленное 3D демо с UTF-8
./Engine3D_Demo
```

## 🐛 Решение проблем

### Общие проблемы

1. **Кракозябры в Windows**:
   - Убедитесь, что вызвали `Console::initialize()`
   - Проверьте кодировку исходных файлов (должна быть UTF-8)

2. **Отсутствуют эмодзи**:
   - Обновите шрифт терминала
   - На Windows используйте Windows Terminal или ConEmu

3. **Нет цветов**:
   - Проверьте переменную окружения TERM
   - Убедитесь, что терминал поддерживает ANSI коды

### Диагностика

```cpp
// Проверка возможностей
if (!Console::supportsUTF8()) {
    Console::warning("UTF-8 не поддерживается");
}

if (!Console::supportsColor()) {
    Console::warning("Цветной вывод недоступен");
}

if (!Console::supportsEmoji()) {
    Console::warning("Эмодзи могут отображаться некорректно");
}

// Вывод полной информации
std::cout << Console::getConsoleInfo() << std::endl;
```

## 📝 Примеры

Полные примеры использования смотрите в:
- `examples/utf8_console_demo.cpp` - демонстрация всех возможностей
- `examples/3d_demo.cpp` - интеграция с 3D движком

## 🔮 Планы развития

- [ ] Поддержка форматированного вывода (printf-style)
- [ ] Прогресс-бары с Unicode символами
- [ ] Таблицы и сетки
- [ ] Анимированный вывод
- [ ] Темы оформления

---

*Документация актуальна для HyperEngine версии 1.0.0*

