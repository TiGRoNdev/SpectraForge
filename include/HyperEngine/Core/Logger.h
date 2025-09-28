/**
 * @file Logger.h
 * @brief Реализация логгера согласно SOLID принципам
 */

#pragma once

#include "EngineCore.h"
#include <string>
#include <fstream>
#include <memory>
#include <mutex>

namespace HyperEngine {
namespace Core {

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

/**
 * @brief Конкретная реализация логгера
 * 
 * Следует принципу SRP - отвечает только за логирование
 */
class Logger : public ILogger {
public:
    /**
     * @brief Конструктор
     * @param logFile Путь к файлу лога (опционально)
     * @param level Минимальный уровень логирования
     */
    explicit Logger(const std::string& logFile = "", LogLevel level = LogLevel::INFO);

    /**
     * @brief Деструктор
     */
    ~Logger() override;

    // ILogger interface
    void logInfo(const std::string& message) override;
    void logWarning(const std::string& message) override;
    void logError(const std::string& message) override;
    void logDebug(const std::string& message) override;

    /**
     * @brief Установка уровня логирования
     * @param level Новый уровень
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Получение текущего уровня логирования
     * @return Текущий уровень
     */
    LogLevel getLogLevel() const { return currentLevel; }

    /**
     * @brief Включение/выключение вывода в консоль
     * @param enable true для включения
     */
    void setConsoleOutput(bool enable) { consoleOutput = enable; }

    /**
     * @brief Включение/выключение вывода в файл
     * @param enable true для включения
     */
    void setFileOutput(bool enable) { fileOutput = enable; }

private:
    LogLevel currentLevel;
    std::string logFilePath;
    std::ofstream logFileStream;
    bool consoleOutput = true;
    bool fileOutput = false;
    mutable std::mutex logMutex;

    /**
     * @brief Внутренний метод логирования
     * @param level Уровень сообщения
     * @param message Сообщение
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Получение строкового представления уровня
     * @param level Уровень
     * @return Строковое представление
     */
    std::string levelToString(LogLevel level) const;

    /**
     * @brief Получение текущего времени в виде строки
     * @return Строка с временем
     */
    std::string getCurrentTime() const;
};

}  // namespace Core
}  // namespace HyperEngine
