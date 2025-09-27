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

#include "Engine3D/Core/Console.h"
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

using namespace Engine3D::Core;

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

    std::cout << "🔧 Инициализация консоли с поддержкой UTF-8..." << std::endl;

    if (!utf8Success) {
        std::cout << "⚠️ Предупреждение: не удалось настроить UTF-8 кодировку" << std::endl;
    }
    
    if (!localeSuccess) {
        std::cout << "⚠️ Предупреждение: не удалось настроить локаль" << std::endl;
    }

    if (!colorSuccess) {
        std::cout << "⚠️ Предупреждение: цветной вывод недоступен" << std::endl;
    }

    if (!vtSuccess) {
        std::cout << "⚠️ Предупреждение: виртуальный терминал недоступен" << std::endl;
    }

    initialized = true;
    
    // Выводим информацию о консоли
    std::cout << "✅ Консоль инициализирована успешно!" << std::endl;
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
    log(LogLevel::DEBUG, message);
}

void Console::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Console::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Console::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Console::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Console::log(LogLevel level, const std::string& message) {
    std::string emoji = emojiSupported ? getLogLevelEmoji(level) : "";
    std::string levelStr = getLogLevelString(level);
    
    std::ostringstream oss;
    if (!emoji.empty()) {
        oss << emoji << " ";
    }
    oss << "[" << levelStr << "] " << message;
    
    std::cout << oss.str() << std::endl;
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
    std::cout << "🧪 Тест отображения Unicode символов:" << std::endl;
    std::cout << "════════════════════════════════════" << std::endl;
    
    // Кириллица
    std::cout << "🔤 Кириллица: Привет, мир! АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" << std::endl;
    
    // Эмодзи
    std::cout << "😀 Эмодзи: 🚀🌟⭐🎯🎮🛠️💎🔥💯✨🎉🎊🌈🦄" << std::endl;
    
    // Математические символы
    std::cout << "🔢 Математика: ∑∏∫√∞±≤≥≠∈∉∀∃∄∅∆∇" << std::endl;
    
    // Стрелки и символы
    std::cout << "➡️ Стрелки: ←↑→↓↖↗↘↙⬅⬆⬇➡⭐✅❌⚠️" << std::endl;
    
    // Рамки и линии
    std::cout << "📦 Рамки: ┌─┬─┐│ │ │├─┼─┤│ │ │└─┴─┘" << std::endl;
    
    // Дроби и специальные символы
    std::cout << "🔣 Спецсимволы: ½¼¾⅓⅔⅛⅜⅝⅞™®©" << std::endl;
    
    std::cout << "════════════════════════════════════" << std::endl;
    std::cout << std::endl;
}

void Console::testColorDisplay() {
    std::cout << std::endl;
    std::cout << "🎨 Тест цветного вывода:" << std::endl;
    std::cout << "═══════════════════════" << std::endl;
    
    std::cout << std::endl;
    
    std::cout << "═══════════════════════" << std::endl;
    std::cout << std::endl;
}

std::string Console::getColorCode(ConsoleColor color) {
    switch (color) {
        case ConsoleColor::DEFAULT: return "\033[0m";
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
        case LogLevel::DEBUG: return "🐛";
        case LogLevel::INFO: return "ℹ️";
        case LogLevel::WARNING: return "⚠️";
        case LogLevel::ERROR: return "❌";
        case LogLevel::CRITICAL: return "🚨";
        default: return "";
    }
}

ConsoleColor Console::getLogLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return ConsoleColor::CYAN;
        case LogLevel::INFO: return ConsoleColor::GREEN;
        case LogLevel::WARNING: return ConsoleColor::YELLOW;
        case LogLevel::ERROR: return ConsoleColor::RED;
        case LogLevel::CRITICAL: return ConsoleColor::BRIGHT_RED;
        default: return ConsoleColor::DEFAULT;
    }
}

std::string Console::getLogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}
