/**
 * @file safe_console_demo.cpp
 * @brief Демонстрация безопасного вывода в консоль с обработкой ошибок кодировки
 *
 * Этот пример показывает:
 * - Использование безопасных методов вывода
 * - Обработку проблемных символов
 * - Автоматический fallback при ошибках кодировки
 * - Безопасное преобразование чисел в строки
 */

#include <iostream>
#include <string>
#include <vector>
#include "SpectraForge/Core/Console.h"
#include "SpectraForge/Core/SafeConsole.h"

using namespace SpectraForge::Core;

class SafeConsoleDemo {
  public:
    SafeConsoleDemo() { Console::initialize(); }

    void run() {
        demonstrateBasicSafety();
        demonstrateNumberConversion();
        demonstrateProblematicText();
        demonstrateLogging();
        demonstrateMacros();
    }

  private:
    void demonstrateBasicSafety() {
        Console::safeInfo("=== Демонстрация базовой безопасности ===");

        // Обычный текст
        Console::safePrintLine("Обычный текст без проблем");

        // Текст с русскими символами
        Console::safePrintLine("Русский текст: Привет, мир!");

        // Текст с эмодзи (может быть проблематичным на некоторых системах)
        Console::safePrintLine("Эмодзи: 🚀 🎮 ⚡ 🔥");

        std::cout << std::endl;
    }

    void demonstrateNumberConversion() {
        Console::safeInfo("=== Демонстрация безопасного преобразования чисел ===");

        int intValue = 42;
        float floatValue = 3.14159f;
        double doubleValue = 2.71828;
        size_t sizeValue = 1024;

        Console::safePrintLine("Целое число: " + SAFE_TO_STRING(intValue));
        Console::safePrintLine("Float: " + SAFE_TO_STRING(floatValue));
        Console::safePrintLine("Double: " + SAFE_TO_STRING(doubleValue));
        Console::safePrintLine("Size_t: " + SAFE_TO_STRING(sizeValue));

        // Демонстрация с большими числами
        long long bigNumber = 9223372036854775807LL;
        Console::safePrintLine("Большое число: " + SAFE_TO_STRING(bigNumber));

        std::cout << std::endl;
    }

    void demonstrateProblematicText() {
        Console::safeInfo("=== Демонстрация обработки проблемного текста ===");

        // Создаем строку с потенциально проблемными символами
        std::string problematicText = "Текст с символами: ♠ ♣ ♥ ♦ ★ ☆ ☀ ☁ ☂ ☃";

        Console::safePrintLine("Проблемный текст: " + problematicText);

        // Проверяем, безопасен ли текст
        if (Console::isTextSafe(problematicText)) {
            Console::safeInfo("Текст безопасен для вывода");
        } else {
            Console::safeWarning("Текст может вызвать проблемы, используем очистку");
            std::string cleanText = Console::sanitizeText(problematicText);
            Console::safePrintLine("Очищенный текст: " + cleanText);
        }

        std::cout << std::endl;
    }

    void demonstrateLogging() {
        Console::safeInfo("=== Демонстрация безопасного логирования ===");

        Console::safeInfo("Информационное сообщение");
        Console::safeWarning("Предупреждение о потенциальной проблеме");
        Console::safeError("Сообщение об ошибке");

        // Логирование с числами
        int errorCode = 404;
        Console::safeError("Ошибка с кодом: " + SAFE_TO_STRING(errorCode));

        std::cout << std::endl;
    }

    void demonstrateMacros() {
        Console::safeInfo("=== Демонстрация макросов ===");

        // Использование макросов для удобства
        SAFE_INFO("Информация через макрос");
        SAFE_WARNING("Предупреждение через макрос");
        SAFE_ERROR("Ошибка через макрос");

        int value = 100;
        SAFE_PRINT_LINE("Значение: " + SAFE_TO_STRING(value));

        // Демонстрация fallback
        std::string text = "Обычный текст";
        std::string fallback = "Резервный текст";
        SAFE_PRINT_FALLBACK(text, fallback);

        std::cout << std::endl;
    }
};

int main() {
    try {
        SafeConsoleDemo demo;

        SAFE_PRINT_LINE("=== Демонстрация безопасного вывода в консоль ===");
        SAFE_PRINT_LINE("Этот пример показывает, как избежать ошибок кодировки");
        std::cout << std::endl;

        demo.run();

        Console::safeInfo("Демонстрация завершена успешно!");

        return 0;

    } catch (const std::exception& e) {
        Console::safeError("Критическая ошибка: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Console::safeError("Неизвестная критическая ошибка");
        return 1;
    }
}
