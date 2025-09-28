/**
 * @file Console.cpp
 * @brief Реализация системы поддержки UTF-8 консоли
 */

// Включаем Windows.h до нашего заголовка для решения конфликта макросов
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
// Очищаем конфликтующие макросы
#ifdef ERROR
#undef ERROR
#endif
#ifdef CRITICAL
#undef CRITICAL
#endif
#endif

#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef __linux__
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

#ifndef _WIN32
#include <cstdlib>
#endif

using namespace HyperEngine::Core;

// Инициализация статических переменных
bool Console::initialized = false;
bool Console::utf8Supported = false;
bool Console::colorSupported = false;
bool Console::emojiSupported = false;

bool Console::initialize() {
    if (initialized) {
        return true;
    }

    // Сначала настраиваем UTF-8 и локаль
    bool utf8Success = setupUTF8();
    bool localeSuccess = setupLocale();
    bool colorSuccess = enableColorOutput();
    bool vtSuccess = enableVirtualTerminal();

    SAFE_PRINT_LINE("🔧 Инициализация консоли с поддержкой UTF-8...");

    if (!utf8Success) {
        SAFE_PRINT_LINE("⚠️ Предупреждение: не удалось настроить UTF-8 кодировку");
    }
    
    if (!localeSuccess) {
        SAFE_PRINT_LINE("⚠️ Предупреждение: не удалось настроить локаль");
    }

    if (!colorSuccess) {
        SAFE_PRINT_LINE("⚠️ Предупреждение: цветной вывод недоступен");
    }

    if (!vtSuccess) {
        SAFE_PRINT_LINE("⚠️ Предупреждение: виртуальный терминал недоступен");
    }

    initialized = true;
    
    // Выводим информацию о консоли
    SAFE_PRINT_LINE("✅ Консоль инициализирована успешно!");
    std::cout << getConsoleInfo() << std::endl;

    return true;
}

bool Console::setupUTF8() {
#ifdef _WIN32
    // Настройка UTF-8 для консоли Windows
    if (!SetConsoleCP(CP_UTF8)) {
        return false;
    }
    if (!SetConsoleOutputCP(CP_UTF8)) {
        return false;
    }

    // Не используем _O_U8TEXT для лучшей совместимости с эмодзи
    // Вместо этого полагаемся на установленные кодовые страницы
    
    utf8Supported = true;
    return true;
#else
    // На Unix системах UTF-8 обычно поддерживается по умолчанию
    utf8Supported = true;
    return true;
#endif
}

bool Console::setupLocale() {
#ifdef _WIN32
    // На Windows пытаемся установить русскую локаль с UTF-8
    try {
        std::locale::global(std::locale("ru_RU.UTF-8"));
        std::cout.imbue(std::locale());
        std::cerr.imbue(std::locale());
        return true;
    } catch (const std::exception&) {
        try {
            // Fallback к системной локали
            std::locale::global(std::locale(""));
            std::cout.imbue(std::locale());
            std::cerr.imbue(std::locale());
            return true;
        } catch (const std::exception&) {
            try {
                // Fallback к C локали
                std::locale::global(std::locale("C"));
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }
    }
#else
    // На Unix системах пытаемся установить UTF-8 локаль
    try {
        std::locale::global(std::locale(""));
        std::cout.imbue(std::locale());
        std::cerr.imbue(std::locale());
        return true;
    } catch (const std::exception&) {
        try {
            std::locale::global(std::locale("en_US.UTF-8"));
            std::cout.imbue(std::locale());
            std::cerr.imbue(std::locale());
            return true;
        } catch (const std::exception&) {
            try {
                std::locale::global(std::locale("C"));
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }
    }
#endif
}

bool Console::enableColorOutput() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return false;
    }

    colorSupported = true;
    return true;
#else
    // На Unix системах обычно поддерживается
    const char* term = std::getenv("TERM");
    if (term != nullptr) {
        std::string termStr(term);
        colorSupported = (termStr.find("color") != std::string::npos || 
                         termStr.find("xterm") != std::string::npos ||
                         termStr.find("screen") != std::string::npos);
    } else {
        colorSupported = false;
    }
    return colorSupported;
#endif
}

bool Console::enableVirtualTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return false;
    }

    emojiSupported = true;
    return true;
#else
    // На Unix системах обычно поддерживается
    emojiSupported = true;
    return true;
#endif
}

// Метод Console::print удален, используйте std::cout для вывода текста

void Console::debug(const std::string& message) {
    log(message, LogLevel::DEBUG_LEVEL);
}

void Console::info(const std::string& message) {
    log(message, LogLevel::INFO_LEVEL);
}

void Console::warning(const std::string& message) {
    log(message, LogLevel::WARNING_LEVEL);
}

void Console::error(const std::string& message) {
    log(message, LogLevel::ERROR_LEVEL);
}

void Console::critical(const std::string& message) {
    log(message, LogLevel::ERROR_LEVEL); // Используем ERROR_LEVEL вместо CRITICAL, так как у нас нет CRITICAL
}

void Console::log(const std::string& message, LogLevel level) {
    std::string emoji = emojiSupported ? getLogLevelEmoji(level) : "";
    std::string levelStr = getLogLevelString(level);
    
    std::ostringstream oss;
    if (!emoji.empty()) {
        oss << emoji << " ";
    }
    oss << "[" << levelStr << "] " << message;
    
    // Используем безопасный вывод с fallback
    std::string fullMessage = oss.str();
    std::string fallbackMessage = "[" + levelStr + "] " + message;
    
    safePrint(fullMessage + "\n");
}

void Console::clear() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Console::setTitle(const std::string& title) {
#ifdef _WIN32
    SetConsoleTitleA(title.c_str());
#else
    std::cout << "\033]0;" << title << "\007";
    std::cout.flush();
#endif
}

bool Console::supportsUTF8() {
    return utf8Supported;
}

bool Console::supportsColor() {
    return colorSupported;
}

bool Console::supportsEmoji() {
    return emojiSupported;
}

std::string Console::getConsoleInfo() {
    std::ostringstream oss;
    oss << "📊 Информация о консоли:" << std::endl;
    oss << "   UTF-8: " << (utf8Supported ? "✅ Поддерживается" : "❌ Не поддерживается") << std::endl;
    oss << "   Цвета: " << (colorSupported ? "✅ Поддерживаются" : "❌ Не поддерживаются") << std::endl;
    oss << "   Эмодзи: " << (emojiSupported ? "✅ Поддерживаются" : "❌ Не поддерживаются") << std::endl;
    oss << "   Локаль: " << std::locale().name();
    
    return oss.str();
}

void Console::testUnicodeDisplay() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("🧪 Тест отображения Unicode символов:");
    SAFE_PRINT_LINE("════════════════════════════════════");
    
    // Кириллица
    SAFE_PRINT_LINE("🔤 Кириллица: Привет, мир! АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ");
    
    // Эмодзи
    SAFE_PRINT_LINE("😀 Эмодзи: 🚀🌟⭐🎯🎮🛠️💎🔥💯✨🎉🎊🌈🦄");
    
    // Математические символы
    SAFE_PRINT_LINE("🔢 Математика: ∑∏∫√∞±≤≥≠∈∉∀∃∄∅∆∇");
    
    // Стрелки и символы
    SAFE_PRINT_LINE("➡️ Стрелки: ←↑→↓↖↗↘↙⬅⬆⬇➡⭐✅❌⚠️");
    
    // Рамки и линии
    SAFE_PRINT_LINE("📦 Рамки: ┌─┬─┐│ │ │├─┼─┤│ │ │└─┴─┘");
    
    // Дроби и специальные символы
    SAFE_PRINT_LINE("🔣 Спецсимволы: ½¼¾⅓⅔⅛⅜⅝⅞™®©");
    
    SAFE_PRINT_LINE("════════════════════════════════════");
    std::cout << std::endl;
}

void Console::testColorDisplay() {
    std::cout << std::endl;
    SAFE_PRINT_LINE("🎨 Тест цветного вывода:");
    SAFE_PRINT_LINE("═══════════════════════");
    
    std::cout << std::endl;
    
    SAFE_PRINT_LINE("═══════════════════════");
    std::cout << std::endl;
}

std::string Console::getColorCode(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::RESET: return "\033[0m";
        case ConsoleColor::BLACK: return "\033[30m";
        case ConsoleColor::RED: return "\033[31m";
        case ConsoleColor::GREEN: return "\033[32m";
        case ConsoleColor::YELLOW: return "\033[33m";
        case ConsoleColor::BLUE: return "\033[34m";
        case ConsoleColor::MAGENTA: return "\033[35m";
        case ConsoleColor::CYAN: return "\033[36m";
        case ConsoleColor::WHITE: return "\033[37m";
        case ConsoleColor::BRIGHT_BLACK: return "\033[90m";
        case ConsoleColor::BRIGHT_RED: return "\033[91m";
        case ConsoleColor::BRIGHT_GREEN: return "\033[92m";
        case ConsoleColor::BRIGHT_YELLOW: return "\033[93m";
        case ConsoleColor::BRIGHT_BLUE: return "\033[94m";
        case ConsoleColor::BRIGHT_MAGENTA: return "\033[95m";
        case ConsoleColor::BRIGHT_CYAN: return "\033[96m";
        case ConsoleColor::BRIGHT_WHITE: return "\033[97m";
        default: return "\033[0m";
    }
}

std::string Console::getLogLevelEmoji(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG_LEVEL: return "🐛";
        case LogLevel::INFO_LEVEL: return "ℹ️";
        case LogLevel::WARNING_LEVEL: return "⚠️";
        case LogLevel::ERROR_LEVEL: return "❌";
        default: return "";
    }
}

ConsoleColor Console::getLogLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG_LEVEL: return ConsoleColor::CYAN;
        case LogLevel::INFO_LEVEL: return ConsoleColor::GREEN;
        case LogLevel::WARNING_LEVEL: return ConsoleColor::YELLOW;
        case LogLevel::ERROR_LEVEL: return ConsoleColor::RED;
        default: return ConsoleColor::RESET;
    }
}

std::string Console::getLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG_LEVEL: return "DEBUG";
        case LogLevel::INFO_LEVEL: return "INFO";
        case LogLevel::WARNING_LEVEL: return "WARNING";
        case LogLevel::ERROR_LEVEL: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Console::safePrint(const std::string& text) {
    try {
        // Проверяем, безопасен ли текст для вывода
        if (isTextSafe(text)) {
            std::cout << text;
            std::cout.flush();
        } else {
            // Используем очищенную версию
            std::string safeText = sanitizeText(text);
            std::cout << safeText;
            std::cout.flush();
        }
    } catch (const std::exception&) {
        // В случае ошибки выводим базовое сообщение
        try {
            std::cout << "[Ошибка вывода]";
            std::cout.flush();
        } catch (...) {
            // Последняя попытка - выводим только ASCII
            std::cout << "[Output Error]";
            std::cout.flush();
        }
    }
}

bool Console::isTextSafe(const std::string& text) {
    try {
        // Проверяем каждый символ на корректность
        for (size_t i = 0; i < text.length(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);
            
            // Проверяем ASCII символы (всегда безопасны)
            if (c < 128) {
                continue;
            }
            
            // Проверяем UTF-8 последовательности
            if ((c & 0xE0) == 0xC0) { // 2-байтовая последовательность
                if (i + 1 >= text.length()) return false;
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                if ((c2 & 0xC0) != 0x80) return false;
                i += 1;
            } else if ((c & 0xF0) == 0xE0) { // 3-байтовая последовательность
                if (i + 2 >= text.length()) return false;
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                unsigned char c3 = static_cast<unsigned char>(text[i + 2]);
                if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return false;
                i += 2;
            } else if ((c & 0xF8) == 0xF0) { // 4-байтовая последовательность (эмодзи)
                if (i + 3 >= text.length()) return false;
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                unsigned char c3 = static_cast<unsigned char>(text[i + 2]);
                unsigned char c4 = static_cast<unsigned char>(text[i + 3]);
                if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) return false;
                
                // Если эмодзи не поддерживаются, считаем небезопасным
                if (!emojiSupported) return false;
                i += 3;
            } else {
                // Некорректная UTF-8 последовательность
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::string Console::sanitizeText(const std::string& text) {
    std::string result;
    result.reserve(text.length());
    
    try {
        for (size_t i = 0; i < text.length(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);
            
            // Оставляем ASCII символы как есть
            if (c < 128) {
                result += text[i];
                continue;
            }
            
            // Обрабатываем UTF-8 последовательности
            if ((c & 0xE0) == 0xC0 && i + 1 < text.length()) { // 2-байтовая
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                if ((c2 & 0xC0) == 0x80) {
                    result += text[i];
                    result += text[i + 1];
                    i += 1;
                    continue;
                }
            } else if ((c & 0xF0) == 0xE0 && i + 2 < text.length()) { // 3-байтовая
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                unsigned char c3 = static_cast<unsigned char>(text[i + 2]);
                if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
                    result += text[i];
                    result += text[i + 1];
                    result += text[i + 2];
                    i += 2;
                    continue;
                }
            } else if ((c & 0xF8) == 0xF0 && i + 3 < text.length()) { // 4-байтовая (эмодзи)
                unsigned char c2 = static_cast<unsigned char>(text[i + 1]);
                unsigned char c3 = static_cast<unsigned char>(text[i + 2]);
                unsigned char c4 = static_cast<unsigned char>(text[i + 3]);
                if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
                    if (emojiSupported) {
                        result += text[i];
                        result += text[i + 1];
                        result += text[i + 2];
                        result += text[i + 3];
                    } else {
                        result += "?"; // Заменяем эмодзи на знак вопроса
                    }
                    i += 3;
                    continue;
                }
            }
            
            // Если символ не удалось распознать, заменяем на знак вопроса
            result += "?";
        }
    } catch (...) {
        return "[Текст поврежден]";
    }
    
    return result;
}

void Console::safePrintLine(const std::string& message) {
    safePrint(message + "\n");
}

void Console::safeInfo(const std::string& message) {
    std::string prefix = emojiSupported ? "ℹ️ " : "[INFO] ";
    safePrintLine(prefix + message);
}

void Console::safeWarning(const std::string& message) {
    std::string prefix = emojiSupported ? "⚠️ " : "[WARNING] ";
    safePrintLine(prefix + message);
}

void Console::safeError(const std::string& message) {
    std::string prefix = emojiSupported ? "❌ " : "[ERROR] ";
    safePrintLine(prefix + message);
}

