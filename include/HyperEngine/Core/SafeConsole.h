#pragma once

#include <iostream>
#include <sstream>
#include <string>

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
    return value;
}

// Специализация для C-строк
template<>
inline std::string SAFE_TO_STRING<const char*>(const char* const& value) {
    if (value == nullptr) {
        return "[NULL]";
    }
    return std::string(value);
}

}  // namespace Core
}  // namespace HyperEngine

// Макросы для безопасного вывода
#define SAFE_PRINT_LINE(text) \
    std::cout << HyperEngine::Core::SAFE_TO_STRING(text) << std::endl

#define SAFE_PRINT(text) \
    std::cout << HyperEngine::Core::SAFE_TO_STRING(text)

#define SAFE_INFO(text) \
    std::cout << "[INFO] " << HyperEngine::Core::SAFE_TO_STRING(text) << std::endl

#define SAFE_WARNING(text) \
    std::cout << "[WARNING] " << HyperEngine::Core::SAFE_TO_STRING(text) << std::endl

#define SAFE_ERROR(text) \
    std::cerr << "[ERROR] " << HyperEngine::Core::SAFE_TO_STRING(text) << std::endl

#define SAFE_PRINT_FALLBACK(text, fallback) \
    std::cout << HyperEngine::Core::SAFE_TO_STRING(text) << " (fallback: " << HyperEngine::Core::SAFE_TO_STRING(fallback) << ")" << std::endl