#pragma once

#include <string>
#include <iostream>

namespace HyperEngine {
namespace Core {

/**
 * @brief Безопасная консольная система для отладки
 * 
 * Временная заглушка для обеспечения совместимости
 * Будет полностью реализована в следующих этапах
 */
class Console {
public:
    static void initialize() {
        // Заглушка для инициализации
    }
    
    static void cleanup() {
        // Заглушка для очистки
    }
    
    static void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
    }
    
    static void error(const std::string& message) {
        std::cerr << "[ERROR] " << message << std::endl;
    }
    
    static void warning(const std::string& message) {
        std::cout << "[WARNING] " << message << std::endl;
    }
};

} // namespace Core
} // namespace HyperEngine
