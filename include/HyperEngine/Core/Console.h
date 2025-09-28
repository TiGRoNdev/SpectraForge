#pragma once

#include <iostream>
#include <string>

namespace HyperEngine {
namespace Core {

// Перечисления для уровней логирования и цветов
enum class LogLevel {
    DEBUG_LEVEL,
    INFO_LEVEL,
    WARNING_LEVEL,
    ERROR_LEVEL,
    CRITICAL_LEVEL
};

enum class ConsoleColor {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE,
    RESET
};

/**
 * @brief Безопасная консольная система для отладки
 */
class Console {
  public:
    // Основные методы
    static bool initialize();
    static void cleanup();
    
    // Методы логирования
    static void log(const std::string& message, LogLevel level = LogLevel::INFO_LEVEL);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);
    
    // Утилиты консоли
    static void clear();
    static void setTitle(const std::string& title);
    
    // Проверки поддержки
    static bool supportsUTF8();
    static bool supportsColor();
    static bool supportsEmoji();
    
    // Тестирование
    static std::string getConsoleInfo();
    static void testUnicodeDisplay();
    static void testColorDisplay();
    
    // Безопасный вывод
    static void safePrint(const std::string& text);
    static void safePrintLine(const std::string& text);
    static void safeInfo(const std::string& message);
    static void safeWarning(const std::string& message);
    static void safeError(const std::string& message);
    
    // Безопасность текста (публичные методы)
    static bool isTextSafe(const std::string& text);
    static std::string sanitizeText(const std::string& text);
    
  private:
    // Внутренние методы
    static bool setupUTF8();
    static bool setupLocale();
    static bool enableColorOutput();
    static bool enableVirtualTerminal();
    
    // Утилиты
    static std::string getColorCode(ConsoleColor color);
    static std::string getLogLevelEmoji(LogLevel level);
    static ConsoleColor getLogLevelColor(LogLevel level);
    static std::string getLogLevelString(LogLevel level);
    
    
    // Статические переменные
    static bool initialized;
    static bool utf8Supported;
    static bool colorSupported;
    static bool emojiSupported;
};

}  // namespace Core
}  // namespace HyperEngine
