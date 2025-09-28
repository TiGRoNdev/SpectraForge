/**
 * @file Logger.cpp
 * @brief Реализация логгера
 */

#include "HyperEngine/Core/Logger.h"
#include "HyperEngine/Core/SafeConsole.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace HyperEngine::Core;

namespace HyperEngine {
namespace Core {

Logger::Logger(const std::string& logFile, LogLevel level)
    : currentLevel(level), logFilePath(logFile) {
    
    if (!logFilePath.empty()) {
        logFileStream.open(logFilePath, std::ios::app);
        if (logFileStream.is_open()) {
            fileOutput = true;
            log(LogLevel::INFO_LEVEL, "Логгер инициализирован с файлом: " + logFilePath);
        } else {
            SAFE_ERROR("Не удалось открыть файл лога: " + logFilePath);
        }
    }
}

Logger::~Logger() {
    if (logFileStream.is_open()) {
        log(LogLevel::INFO_LEVEL, "Логгер завершает работу");
        logFileStream.close();
    }
}

void Logger::logInfo(const std::string& message) {
    log(LogLevel::INFO_LEVEL, message);
}

void Logger::logWarning(const std::string& message) {
    log(LogLevel::WARNING_LEVEL, message);
}

void Logger::logError(const std::string& message) {
    log(LogLevel::ERROR_LEVEL, message);
}

void Logger::logDebug(const std::string& message) {
    log(LogLevel::DEBUG_LEVEL, message);
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < currentLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTime();
    std::string levelStr = levelToString(level);
    std::string fullMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

    // Вывод в консоль
    if (consoleOutput) {
        switch (level) {
            case LogLevel::DEBUG_LEVEL:
                SAFE_PRINT_LINE("🔍 " + fullMessage);
                break;
            case LogLevel::INFO_LEVEL:
                SAFE_INFO(fullMessage);
                break;
            case LogLevel::WARNING_LEVEL:
                SAFE_WARNING(fullMessage);
                break;
            case LogLevel::ERROR_LEVEL:
                SAFE_ERROR(fullMessage);
                break;
        }
    }

    // Вывод в файл
    if (fileOutput && logFileStream.is_open()) {
        logFileStream << fullMessage << std::endl;
        logFileStream.flush();
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG_LEVEL:   return "DEBUG";
        case LogLevel::INFO_LEVEL:    return "INFO";
        case LogLevel::WARNING_LEVEL: return "WARN";
        case LogLevel::ERROR_LEVEL:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

}  // namespace Core
}  // namespace HyperEngine
