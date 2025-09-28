#pragma once

/**
 * @file Console.h
 * @brief Система поддержки UTF-8 консоли для кроссплатформенного отображения Unicode символов
 */

#include <iostream>
#include <locale>
#include <string>
#include "Engine3D/Core/Console.h"

namespace Engine3D {
namespace Core {

/**
 * @brief Цвета для консольного вывода
 */
enum class ConsoleColor {
    DEFAULT = 0,
    BLACK = 1,
    RED = 2,
    GREEN = 3,
    YELLOW = 4,
    BLUE = 5,
    MAGENTA = 6,
    CYAN = 7,
    WHITE = 8,
    BRIGHT_BLACK = 9,
    BRIGHT_RED = 10,
    BRIGHT_GREEN = 11,
    BRIGHT_YELLOW = 12,
    BRIGHT_BLUE = 13,
    BRIGHT_MAGENTA = 14,
    BRIGHT_CYAN = 15,
    BRIGHT_WHITE = 16
};

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    DEBUG_LEVEL = 0,
    INFO_LEVEL = 1,
    WARNING_LEVEL = 2,
    ERROR_LEVEL = 3,
    CRITICAL_LEVEL = 4
};

/**
 * @brief Класс для управления консолью с поддержкой UTF-8
 */
class Console {
  public:
    /**
     * @brief Инициализирует консоль с поддержкой UTF-8
     * @return true если инициализация прошла успешно
     */
    static bool initialize();

    /**
     * @brief Настраивает UTF-8 кодировку для консоли
     * @return true если настройка прошла успешно
     */
    static bool setupUTF8();

    /**
     * @brief Настраивает системную локаль
     * @return true если настройка прошла успешно
     */
    static bool setupLocale();

    /**
     * @brief Включает поддержку цветного вывода
     * @return true если поддержка включена успешно
     */
    static bool enableColorOutput();

    /**
     * @brief Включает поддержку виртуального терминала Windows (для эмодзи)
     * @return true если поддержка включена успешно
     */
    static bool enableVirtualTerminal();

    // Метод print удален, используйте std::cout для вывода текста

    /**
     * @brief Выводит отладочное сообщение
     * @param message Сообщение
     */
    static void debug(const std::string& message);

    /**
     * @brief Выводит информационное сообщение
     * @param message Сообщение
     */
    static void info(const std::string& message);

    /**
     * @brief Выводит предупреждение
     * @param message Сообщение
     */
    static void warning(const std::string& message);

    /**
     * @brief Выводит ошибку
     * @param message Сообщение
     */
    static void error(const std::string& message);

    /**
     * @brief Выводит критическую ошибку
     * @param message Сообщение
     */
    static void critical(const std::string& message);

    /**
     * @brief Выводит сообщение с заданным уровнем логирования
     * @param level Уровень логирования
     * @param message Сообщение
     */
    static void log(LogLevel level, const std::string& message);

    /**
     * @brief Очищает консоль
     */
    static void clear();

    /**
     * @brief Устанавливает заголовок консоли (только Windows)
     * @param title Заголовок
     */
    static void setTitle(const std::string& title);

    /**
     * @brief Проверяет, поддерживает ли консоль UTF-8
     * @return true если поддерживает
     */
    static bool supportsUTF8();

    /**
     * @brief Проверяет, поддерживает ли консоль цветной вывод
     * @return true если поддерживает
     */
    static bool supportsColor();

    /**
     * @brief Проверяет, поддерживает ли консоль эмодзи
     * @return true если поддерживает
     */
    static bool supportsEmoji();

    /**
     * @brief Получает информацию о консоли для диагностики
     * @return Строка с информацией о консоли
     */
    static std::string getConsoleInfo();

    /**
     * @brief Тестирует отображение различных Unicode символов
     */
    static void testUnicodeDisplay();

    /**
     * @brief Демонстрирует возможности цветного вывода
     */
    static void testColorDisplay();

    /**
     * @brief Безопасный вывод строки с обработкой ошибок кодировки
     * @param text Текст для вывода
     * @param fallbackText Резервный текст при ошибке кодировки
     * @return true если вывод прошел успешно
     */
    static bool safePrint(const std::string& text, const std::string& fallbackText = "");

    /**
     * @brief Безопасный вывод числа как строки
     * @param value Числовое значение
     * @return Строковое представление числа
     */
    template <typename T>
    static std::string safeToString(const T& value) {
        try {
            return std::to_string(value);
        } catch (...) {
            return "[число]";
        }
    }

    /**
     * @brief Проверяет, содержит ли строка проблемные символы
     * @param text Текст для проверки
     * @return true если текст безопасен для вывода
     */
    static bool isTextSafe(const std::string& text);

    /**
     * @brief Очищает строку от проблемных символов
     * @param text Исходный текст
     * @return Очищенный текст
     */
    static std::string sanitizeText(const std::string& text);

    /**
     * @brief Безопасный вывод с автоматической обработкой ошибок
     * @param message Сообщение для вывода
     */
    static void safePrintLine(const std::string& message);

    /**
     * @brief Безопасный вывод информационного сообщения
     * @param message Сообщение
     */
    static void safeInfo(const std::string& message);

    /**
     * @brief Безопасный вывод предупреждения
     * @param message Сообщение
     */
    static void safeWarning(const std::string& message);

    /**
     * @brief Безопасный вывод ошибки
     * @param message Сообщение
     */
    static void safeError(const std::string& message);

  private:
    static bool initialized;
    static bool utf8Supported;
    static bool colorSupported;
    static bool emojiSupported;

    /**
     * @brief Получает ANSI код цвета
     * @param color Цвет
     * @return ANSI код
     */
    static std::string getColorCode(ConsoleColor color);

    /**
     * @brief Получает эмодзи для уровня логирования
     * @param level Уровень логирования
     * @return Эмодзи как строка
     */
    static std::string getLogLevelEmoji(LogLevel level);

    /**
     * @brief Получает цвет для уровня логирования
     * @param level Уровень логирования
     * @return Цвет
     */
    static ConsoleColor getLogLevelColor(LogLevel level);

    /**
     * @brief Получает строковое представление уровня логирования
     * @param level Уровень логирования
     * @return Строка
     */
    static std::string getLogLevelString(LogLevel level);
};

}  // namespace Core
}  // namespace Engine3D

// Удобные макросы для логирования
#define LOG_DEBUG(msg) Engine3D::Core::Console::debug(msg)
#define LOG_INFO(msg) Engine3D::Core::Console::info(msg)
#define LOG_WARNING(msg) Engine3D::Core::Console::warning(msg)
#define LOG_ERROR(msg) Engine3D::Core::Console::error(msg)
#define LOG_CRITICAL(msg) Engine3D::Core::Console::critical(msg)

// Макросы для безопасного вывода
#define SAFE_PRINT(text) Engine3D::Core::Console::safePrint(text)
#define SAFE_PRINT_FALLBACK(text, fallback) Engine3D::Core::Console::safePrint(text, fallback)
#define SAFE_PRINT_LINE(text) Engine3D::Core::Console::safePrintLine(text)
#define SAFE_TO_STRING(value) Engine3D::Core::Console::safeToString(value)
#define SAFE_INFO(msg) Engine3D::Core::Console::safeInfo(msg)
#define SAFE_WARNING(msg) Engine3D::Core::Console::safeWarning(msg)
#define SAFE_ERROR(msg) Engine3D::Core::Console::safeError(msg)
