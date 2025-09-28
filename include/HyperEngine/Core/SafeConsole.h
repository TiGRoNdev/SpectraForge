#pragma once

#include "Console.h"
#include <sstream>

namespace HyperEngine {
namespace Core {

/**
 * @brief Безопасное преобразование в строку
 */
template<typename T>
std::string SAFE_TO_STRING(const T& value) {
    try {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    } catch (...) {
        return "[CONVERSION_ERROR]";
    }
}

// Специализация для строк
template<>
inline std::string SAFE_TO_STRING<std::string>(const std::string& value) {
    return Console::isTextSafe(value) ? value : Console::sanitizeText(value);
}

// Специализация для C-строк
template<>
inline std::string SAFE_TO_STRING<const char*>(const char* const& value) {
    if (value == nullptr) {
        return "[NULL]";
    }
    std::string str(value);
    return Console::isTextSafe(str) ? str : Console::sanitizeText(str);
}

}  // namespace Core
}  // namespace HyperEngine

// Макросы для безопасного вывода
#define SAFE_PRINT_LINE(text) \
    HyperEngine::Core::Console::safePrintLine(HyperEngine::Core::SAFE_TO_STRING(text))

#define SAFE_PRINT(text) \
    HyperEngine::Core::Console::safePrint(HyperEngine::Core::SAFE_TO_STRING(text))

#define SAFE_INFO(text) \
    HyperEngine::Core::Console::safeInfo(HyperEngine::Core::SAFE_TO_STRING(text))

#define SAFE_WARNING(text) \
    HyperEngine::Core::Console::safeWarning(HyperEngine::Core::SAFE_TO_STRING(text))

#define SAFE_ERROR(text) \
    HyperEngine::Core::Console::safeError(HyperEngine::Core::SAFE_TO_STRING(text))
