/**
 * @file core_logger_test.cpp
 * @brief Комплексные тесты для класса Logger
 */

#include <gtest/gtest.h>
#include <SpectraForge/Core/Logger.h>
#include <SpectraForge/Core/LogLevel.h>
#include <SpectraForge/Core/SafeConsole.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>

using namespace SpectraForge::Core;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testLogFile = "test_log.txt";
        // Удаляем файл если существует
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }

    void TearDown() override {
        // Очистка
        if (std::filesystem::exists(testLogFile)) {
            std::filesystem::remove(testLogFile);
        }
    }

    std::string readLogFile() {
        std::ifstream file(testLogFile);
        if (!file.is_open()) return "";
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        return content;
    }

    std::string testLogFile;
};

// ============================================================================
// Конструктор
// ============================================================================

TEST_F(LoggerTest, DefaultConstructor) {
    // Arrange & Act
    Logger logger;
    
    // Assert - должен создаться без ошибок
    EXPECT_EQ(logger.getLogLevel(), LogLevel::INFO_LEVEL);
}

TEST_F(LoggerTest, ConstructorWithFile) {
    // Arrange & Act
    Logger logger(testLogFile, LogLevel::DEBUG_LEVEL);
    
    // Assert
    EXPECT_EQ(logger.getLogLevel(), LogLevel::DEBUG_LEVEL);
}

TEST_F(LoggerTest, ConstructorWithLogLevel) {
    // Arrange & Act
    Logger logger("", LogLevel::WARNING_LEVEL);
    
    // Assert
    EXPECT_EQ(logger.getLogLevel(), LogLevel::WARNING_LEVEL);
}

// ============================================================================
// Базовое логирование
// ============================================================================

TEST_F(LoggerTest, LogInfo) {
    // Arrange
    Logger logger;
    logger.setConsoleOutput(false);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(logger.logInfo("Test info message"));
}

TEST_F(LoggerTest, LogWarning) {
    // Arrange
    Logger logger;
    logger.setConsoleOutput(false);
    
    // Act & Assert
    EXPECT_NO_THROW(logger.logWarning("Test warning message"));
}

TEST_F(LoggerTest, LogError) {
    // Arrange
    Logger logger;
    logger.setConsoleOutput(false);
    
    // Act & Assert
    EXPECT_NO_THROW(logger.logError("Test error message"));
}

TEST_F(LoggerTest, LogDebug) {
    // Arrange
    Logger logger;
    logger.setConsoleOutput(false);
    
    // Act & Assert
    EXPECT_NO_THROW(logger.logDebug("Test debug message"));
}

// ============================================================================
// Уровни логирования
// ============================================================================

TEST_F(LoggerTest, SetLogLevel) {
    // Arrange
    Logger logger;
    
    // Act
    logger.setLogLevel(LogLevel::ERROR_LEVEL);
    
    // Assert
    EXPECT_EQ(logger.getLogLevel(), LogLevel::ERROR_LEVEL);
}

TEST_F(LoggerTest, FilterDebugMessages) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logDebug("This should not appear");
    logger.logInfo("This should appear");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("This should appear") != std::string::npos);
    EXPECT_TRUE(content.find("This should not appear") == std::string::npos);
}

TEST_F(LoggerTest, FilterInfoMessages) {
    // Arrange
    Logger logger(testLogFile, LogLevel::WARNING_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Info message");
    logger.logWarning("Warning message");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Info message") == std::string::npos);
}

TEST_F(LoggerTest, FilterWarningMessages) {
    // Arrange
    Logger logger(testLogFile, LogLevel::ERROR_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logWarning("Warning message");
    logger.logError("Error message");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") == std::string::npos);
}

TEST_F(LoggerTest, AllLevelsWithDebug) {
    // Arrange
    Logger logger(testLogFile, LogLevel::DEBUG_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logDebug("Debug");
    logger.logInfo("Info");
    logger.logWarning("Warning");
    logger.logError("Error");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Debug") != std::string::npos);
    EXPECT_TRUE(content.find("Info") != std::string::npos);
    EXPECT_TRUE(content.find("Warning") != std::string::npos);
    EXPECT_TRUE(content.find("Error") != std::string::npos);
}

// ============================================================================
// Вывод в файл
// ============================================================================

TEST_F(LoggerTest, FileOutput) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Test message");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, DisableFileOutput) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(false);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Test message");
    
    // Assert - файл не должен быть создан или быть пустым
    if (std::filesystem::exists(testLogFile)) {
        std::string content = readLogFile();
        EXPECT_TRUE(content.empty() || content.find("Test message") == std::string::npos);
    }
}

TEST_F(LoggerTest, MultipleMessages) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Message 1");
    logger.logInfo("Message 2");
    logger.logInfo("Message 3");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Message 1") != std::string::npos);
    EXPECT_TRUE(content.find("Message 2") != std::string::npos);
    EXPECT_TRUE(content.find("Message 3") != std::string::npos);
}

// ============================================================================
// Консольный вывод
// ============================================================================

TEST_F(LoggerTest, EnableConsoleOutput) {
    // Arrange
    Logger logger;
    
    // Act
    logger.setConsoleOutput(true);
    
    // Assert - не должно падать
    EXPECT_NO_THROW(logger.logInfo("Console message"));
}

TEST_F(LoggerTest, DisableConsoleOutput) {
    // Arrange
    Logger logger;
    
    // Act
    logger.setConsoleOutput(false);
    
    // Assert - не должно падать
    EXPECT_NO_THROW(logger.logInfo("No console message"));
}

// ============================================================================
// Форматирование
// ============================================================================

TEST_F(LoggerTest, LogLevelInMessage) {
    // Arrange
    Logger logger(testLogFile, LogLevel::DEBUG_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Test");
    logger.logWarning("Test");
    logger.logError("Test");
    logger.logDebug("Test");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("INFO") != std::string::npos);
    EXPECT_TRUE(content.find("WARNING") != std::string::npos);
    EXPECT_TRUE(content.find("ERROR") != std::string::npos);
    EXPECT_TRUE(content.find("DEBUG") != std::string::npos);
}

TEST_F(LoggerTest, TimestampInMessage) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Test");
    
    // Assert - должна быть временная метка
    std::string content = readLogFile();
    EXPECT_FALSE(content.empty());
    // Проверяем наличие типичного формата времени (например, ":")
    EXPECT_TRUE(content.find(":") != std::string::npos);
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(LoggerTest, EmptyMessage) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(logger.logInfo(""));
}

TEST_F(LoggerTest, LongMessage) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    std::string longMsg(10000, 'A');
    
    // Act & Assert
    EXPECT_NO_THROW(logger.logInfo(longMsg));
    
    std::string content = readLogFile();
    EXPECT_TRUE(content.find(longMsg) != std::string::npos);
}

TEST_F(LoggerTest, SpecialCharacters) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act
    logger.logInfo("Special: \n\t\r\\\"'");
    
    // Assert
    std::string content = readLogFile();
    EXPECT_TRUE(content.find("Special") != std::string::npos);
}

TEST_F(LoggerTest, UnicodeCharacters) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act & Assert
    EXPECT_NO_THROW(logger.logInfo("Unicode: Привет мир 世界 🌍"));
}

TEST_F(LoggerTest, ConcurrentLogging) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Act - многопоточное логирование
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&logger, i]() {
            logger.logInfo("Thread message " + SpectraForge::Core::SAFE_TO_STRING(i));
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Assert - все сообщения должны быть записаны
    std::string content = readLogFile();
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(content.find("Thread message " + SpectraForge::Core::SAFE_TO_STRING(i)) != std::string::npos);
    }
}

TEST_F(LoggerTest, InvalidFilePath) {
    // Arrange & Act - попытка создать логгер с невалидным путем
    Logger logger("/invalid/path/log.txt", LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    // Assert - не должно падать, но и файл не создастся
    EXPECT_NO_THROW(logger.logInfo("Test"));
}

TEST_F(LoggerTest, ReopenFileAfterDeletion) {
    // Arrange
    Logger logger(testLogFile, LogLevel::INFO_LEVEL);
    logger.setFileOutput(true);
    logger.setConsoleOutput(false);
    
    logger.logInfo("Before deletion");
    
    // Act - удаляем файл
    if (std::filesystem::exists(testLogFile)) {
        std::filesystem::remove(testLogFile);
    }
    
    // Assert - логирование должно продолжать работать
    EXPECT_NO_THROW(logger.logInfo("After deletion"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
