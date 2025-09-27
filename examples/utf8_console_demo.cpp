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

#include "Engine3D/Core/Console.h"
#include "Engine3D/Core/GameObject3D.h"
#include "Engine3D/Rendering/Renderer3D.h"
#include "Engine3D/Math/Vector3.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

using namespace Engine3D::Core;
using namespace Engine3D::Math;

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
        std::cout << "🌟═══════════════════════════════════════════════════════════════🌟" << std::endl;
        std::cout << "                  🚀 HYPERENGINE UTF-8 ДЕМОНСТРАЦИЯ 🚀" << std::endl;
        std::cout << "🌟═══════════════════════════════════════════════════════════════🌟" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Добро пожаловать в демонстрацию поддержки Unicode!" << std::endl;
        std::cout << "Этот пример покажет возможности отображения UTF-8 символов." << std::endl;
        std::cout << std::endl;
        
        waitForUser();
    }

    void demonstrateBasicUTF8() {
        std::cout << "📝 1. БАЗОВАЯ ПОДДЕРЖКА UTF-8" << std::endl;
        std::cout << "═══════════════════════════════" << std::endl;
        
        // Кириллица
        std::cout << "🔤 Кириллические символы:" << std::endl;
        std::cout << "   Строчные: абвгдежзийклмнопрстуфхцчшщъыьэюя" << std::endl;
        std::cout << "   Заглавные: АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" << std::endl;
        std::cout << "   Ё и ё тоже работают!" << std::endl;
        
        // Латиница с диакритиками
        std::cout << std::endl << "🌍 Диакритические знаки:" << std::endl;
        std::cout << "   Français: café, naïve, résumé" << std::endl;
        std::cout << "   Deutsch: Müller, Größe, Weiß" << std::endl;
        std::cout << "   Español: niño, piña, señor" << std::endl;
        
        // Символы валют
        std::cout << std::endl << "💰 Валютные символы:" << std::endl;
        std::cout << "   $ € £ ¥ ₽ ₴ ₸ ₹ ₩ ₪" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateLogging() {
        std::cout << "📋 2. СИСТЕМА ЛОГИРОВАНИЯ С ЭМОДЗИ" << std::endl;
        std::cout << "═══════════════════════════════════════" << std::endl;
        
        Console::debug("Это отладочное сообщение для разработчиков");
        Console::info("Информационное сообщение о состоянии системы");
        Console::warning("Предупреждение о потенциальной проблеме");
        Console::error("Сообщение об ошибке в работе программы");
        Console::critical("Критическая ошибка требующая немедленного внимания!");
        
        std::cout << std::endl;
        std::cout << "Логирование автоматически добавляет эмодзи и цвета!" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateRussianSupport() {
        std::cout << "🇷🇺 3. ПОЛНАЯ ПОДДЕРЖКА РУССКОГО ЯЗЫКА" << std::endl;
        std::cout << "═══════════════════════════════════════════" << std::endl;
        
        Console::info("Инициализация игрового движка...");
        Console::info("Загрузка ресурсов и текстур...");
        Console::warning("Низкая производительность видеокарты");
        Console::error("Не удалось загрузить модель персонажа");
        Console::info("Подключение к серверу установлено");
        
        std::cout << std::endl;
        std::cout << "💡 Все сообщения движка теперь на русском языке!" << std::endl;
        std::cout << "🎮 Интерфейс полностью локализован" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateEmoji() {
        std::cout << "😀 4. ШИРОКАЯ ПОДДЕРЖКА ЭМОДЗИ" << std::endl;
        std::cout << "═══════════════════════════════════" << std::endl;
        
        // Игровые эмодзи
        std::cout << "🎮 Игровые: 🕹️🎯🎲🎪🎭🎨🎬🎵🎶🎸🎹🥁🎤🎧" << std::endl;
        
        // Технические эмодзи
        std::cout << "💻 Технические: ⚙️🔧🔨🛠️💡🔋🔌💾💿📀🖥️⌨️🖱️🖨️📱" << std::endl;
        
        // Состояния
        std::cout << "📊 Состояния: ✅❌⚠️🚀🔥💯⭐✨🌟💎🏆🎉🎊" << std::endl;
        
        // Стрелки и направления
        std::cout << "➡️ Навигация: ⬅️⬆️⬇️➡️↖️↗️↘️↙️🔄🔃🔁🔂⏪⏩⏮️⏭️" << std::endl;
        
        // Разработческие
        std::cout << "👨‍💻 Разработка: 🐛🔍🧪🔬📝📋📊📈📉🗂️📁📂🗃️🗄️" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateColors() {
        std::cout << "🎨 5. ЦВЕТНОЙ ВЫВОД В КОНСОЛЬ" << std::endl;
        std::cout << "═══════════════════════════════════" << std::endl;
        
        Console::testColorDisplay();
        
        // Демонстрация использования в игровом контексте
        std::cout << "🎮 Игровые сообщения:" << std::endl;
        std::cout << "   Игрок получил опыт +100" << std::endl;
        std::cout << "   Найден редкий предмет!" << std::endl;
        std::cout << "   Здоровье критически низкое!" << std::endl;
        std::cout << "   Уровень повышен!" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void demonstrateEngineIntegration() {
        std::cout << "⚙️ 6. ИНТЕГРАЦИЯ С ДВИЖКОМ" << std::endl;
        std::cout << "═══════════════════════════════" << std::endl;
        
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
        std::cout << "🌟 Движок готов к работе с Unicode!" << std::endl;
        
        std::cout << std::endl;
        waitForUser();
    }

    void showFinalMessage() {
        std::cout << std::endl;
        std::cout << "🎉═══════════════════════════════════════════════════════════════🎉" << std::endl;
        std::cout << "                    ✨ ДЕМОНСТРАЦИЯ ЗАВЕРШЕНА ✨" << std::endl;
        std::cout << "🎉═══════════════════════════════════════════════════════════════🎉" << std::endl;
        std::cout << std::endl;
        
        std::cout << "🚀 HyperEngine теперь полностью поддерживает:" << std::endl;
        std::cout << "   ✅ Отображение всех Unicode UTF-8 символов" << std::endl;
        std::cout << "   ✅ Эмодзи в консольном выводе" << std::endl;
        std::cout << "   ✅ Цветной текст для лучшей читаемости" << std::endl;
        std::cout << "   ✅ Полная локализация на русский язык" << std::endl;
        std::cout << "   ✅ Кроссплатформенная совместимость" << std::endl;
        
        std::cout << std::endl;
        std::cout << "💡 Теперь вы можете использовать любые символы в своих проектах!" << std::endl;
        std::cout << "🔗 Спасибо за использование HyperEngine!" << std::endl;
        
        std::cout << std::endl;
        std::cout << "Нажмите Enter для выхода..." << std::endl;
        std::cin.get();
    }

    void waitForUser() {
        std::cout << "📍 Нажмите Enter для продолжения..." << std::endl;
        std::cin.ignore();
    }
};

int main() {
    UTF8Demo demo;
    demo.run();
    return 0;
}