/**
 * @file utf8_console_demo.cpp
 * @brief Демонстрация поддержки UTF-8 символов в консоли HyperEngine
 * 
 * Этот пример показывает:
 * - Инициализацию UTF-8 консоли
 * - Отображение различных Unicode символов
 * - Цветной вывод в консоль
 * - Логирование с эмодзи
 * - Работу с русским языком
 */

#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Core/GameObject3D.h"
#include "HyperEngine/Rendering/Renderer3D.h"
#include "HyperEngine/Math/Vector3.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

using namespace HyperEngine::Core;
using namespace HyperEngine::Math;

class UTF8Demo {
public:
    UTF8Demo() {
        // Инициализация консоли с UTF-8 поддержкой
        Console::initialize();
    }

    void run() {
        showWelcomeMessage();
        demonstrateBasicUTF8();
        demonstrateLogging();
        demonstrateRussianSupport();
        demonstrateEmoji();
        demonstrateColors();
        demonstrateEngineIntegration();
        showFinalMessage();
    }

private:
    void showWelcomeMessage() {
        Console::setTitle("🚀 HyperEngine UTF-8 Demo");
        Console::clear();
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("🌟═══════════════════════════════════════════════════════════════🌟");
        SAFE_PRINT_LINE("                  🚀 HYPERENGINE UTF-8 ДЕМОНСТРАЦИЯ 🚀");
        SAFE_PRINT_LINE("🌟═══════════════════════════════════════════════════════════════🌟");
        std::cout << std::endl;
        
        SAFE_PRINT_LINE("Добро пожаловать в демонстрацию поддержки Unicode!");
        SAFE_PRINT_LINE("Этот пример покажет возможности отображения UTF-8 символов.");
        std::cout << std::endl;
        
        waitForUser();
    }

    void demonstrateBasicUTF8() {
        SAFE_PRINT_LINE("📝 1. БАЗОВАЯ ПОДДЕРЖКА UTF-8");
        SAFE_PRINT_LINE("═══════════════════════════════");
        
        // Кириллица
        SAFE_PRINT_LINE("🔤 Кириллические символы:");
        SAFE_PRINT_LINE("   Строчные: абвгдежзийклмнопрстуфхцчшщъыьэюя");
        SAFE_PRINT_LINE("   Заглавные: АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ");
        SAFE_PRINT_LINE("   Ё и ё тоже работают!");
        
        // Латиница с диакритиками
        std::cout << std::endl << "🌍 Диакритические знаки:" << std::endl;
        SAFE_PRINT_LINE("   Français: café, naïve, résumé");
        SAFE_PRINT_LINE("   Deutsch: Müller, Größe, Weiß");
        SAFE_PRINT_LINE("   Español: niño, piña, señor");
        
        // Символы валют
        std::cout << std::endl << "💰 Валютные символы:" << std::endl;
        SAFE_PRINT_LINE("   $ € £ ¥ ₽ ₴ ₸ ₹ ₩ ₪");
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateLogging() {
        SAFE_PRINT_LINE("📋 2. СИСТЕМА ЛОГИРОВАНИЯ С ЭМОДЗИ");
        SAFE_PRINT_LINE("═══════════════════════════════════════");
        
        Console::debug("Это отладочное сообщение для разработчиков");
        Console::info("Информационное сообщение о состоянии системы");
        Console::warning("Предупреждение о потенциальной проблеме");
        Console::error("Сообщение об ошибке в работе программы");
        Console::critical("Критическая ошибка требующая немедленного внимания!");
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("Логирование автоматически добавляет эмодзи и цвета!");
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateRussianSupport() {
        SAFE_PRINT_LINE("🇷🇺 3. ПОЛНАЯ ПОДДЕРЖКА РУССКОГО ЯЗЫКА");
        SAFE_PRINT_LINE("═══════════════════════════════════════════");
        
        Console::info("Инициализация игрового движка...");
        Console::info("Загрузка ресурсов и текстур...");
        Console::warning("Низкая производительность видеокарты");
        Console::error("Не удалось загрузить модель персонажа");
        Console::info("Подключение к серверу установлено");
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("💡 Все сообщения движка теперь на русском языке!");
        SAFE_PRINT_LINE("🎮 Интерфейс полностью локализован");
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateEmoji() {
        SAFE_PRINT_LINE("😀 4. ШИРОКАЯ ПОДДЕРЖКА ЭМОДЗИ");
        SAFE_PRINT_LINE("═══════════════════════════════════");
        
        // Игровые эмодзи
        SAFE_PRINT_LINE("🎮 Игровые: 🕹️🎯🎲🎪🎭🎨🎬🎵🎶🎸🎹🥁🎤🎧");
        
        // Технические эмодзи
        SAFE_PRINT_LINE("💻 Технические: ⚙️🔧🔨🛠️💡🔋🔌💾💿📀🖥️⌨️🖱️🖨️📱");
        
        // Состояния
        SAFE_PRINT_LINE("📊 Состояния: ✅❌⚠️🚀🔥💯⭐✨🌟💎🏆🎉🎊");
        
        // Стрелки и направления
        SAFE_PRINT_LINE("➡️ Навигация: ⬅️⬆️⬇️➡️↖️↗️↘️↙️🔄🔃🔁🔂⏪⏩⏮️⏭️");
        
        // Разработческие
        SAFE_PRINT_LINE("👨‍💻 Разработка: 🐛🔍🧪🔬📝📋📊📈📉🗂️📁📂🗃️🗄️");
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateColors() {
        SAFE_PRINT_LINE("🎨 5. ЦВЕТНОЙ ВЫВОД В КОНСОЛЬ");
        SAFE_PRINT_LINE("═══════════════════════════════════");
        
        Console::testColorDisplay();
        
        // Демонстрация использования в игровом контексте
        SAFE_PRINT_LINE("🎮 Игровые сообщения:");
        SAFE_PRINT_LINE("   Игрок получил опыт +100");
        SAFE_PRINT_LINE("   Найден редкий предмет!");
        SAFE_PRINT_LINE("   Здоровье критически низкое!");
        SAFE_PRINT_LINE("   Уровень повышен!");
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateEngineIntegration() {
        SAFE_PRINT_LINE("⚙️ 6. ИНТЕГРАЦИЯ С ДВИЖКОМ");
        SAFE_PRINT_LINE("═══════════════════════════════");
        
        // Имитация инициализации движка
        Console::info("🔧 Инициализация HyperEngine 3D...");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("📚 Загрузка математических библиотек...");
        
        // Демонстрация с математическими объектами
        Vector3 position(1.5f, 2.8f, -3.2f);
        std::ostringstream oss;
        oss << "📍 Позиция объекта: (" << position.x << ", " << position.y << ", " << position.z << ")";
        Console::info(oss.str());
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("🎨 Инициализация системы рендеринга...");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("🎵 Загрузка аудио подсистемы...");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("🎮 Настройка системы ввода...");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("⚡ Инициализация физического движка...");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Console::info("✅ Все системы инициализированы успешно!");
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("🌟 Движок готов к работе с Unicode!");
        
        std::cout << std::endl;
        waitForUser();
    }

    void showFinalMessage() {
        std::cout << std::endl;
        SAFE_PRINT_LINE("🎉═══════════════════════════════════════════════════════════════🎉");
        SAFE_PRINT_LINE("                    ✨ ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА ✨");
        SAFE_PRINT_LINE("🎉═══════════════════════════════════════════════════════════════🎉");
        std::cout << std::endl;
        
        SAFE_PRINT_LINE("🚀 HyperEngine теперь полностью поддерживает:");
        SAFE_PRINT_LINE("   ✅ Отображение всех Unicode UTF-8 символов");
        SAFE_PRINT_LINE("   ✅ Эмодзи в консольном выводе");
        SAFE_PRINT_LINE("   ✅ Цветной текст для лучшей читаемости");
        SAFE_PRINT_LINE("   ✅ Полная локализация на русский язык");
        SAFE_PRINT_LINE("   ✅ Кроссплатформенная совместимость");
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("💡 Теперь вы можете использовать любые символы в своих проектах!");
        SAFE_PRINT_LINE("🔗 Спасибо за использование HyperEngine!");
        
        std::cout << std::endl;
        SAFE_PRINT_LINE("Нажмите Enter для выхода...");
        std::cin.get();
    }

    void waitForUser() {
        SAFE_PRINT_LINE("📍 Нажмите Enter для продолжения...");
        std::cin.ignore();
    }
};

int main() {
    UTF8Demo demo;
    demo.run();
    return 0;
}
