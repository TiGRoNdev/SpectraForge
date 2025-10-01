#pragma once
#include <gtest/gtest.h>

// GMock опционален - подключаем только если доступен
#ifdef __has_include
#if __has_include(<gmock/gmock.h>)
#include <gmock/gmock.h>
#define HYPERENGINE_HAS_GMOCK 1
#endif
#endif

#include <chrono>
#include <iostream>
#include <stdexcept>

namespace HyperEngine::Testing {

/**
 * @brief Базовый класс для всех тестов HyperEngine
 *
 * Предоставляет общую инфраструктуру для инициализации и очистки
 * тестовой среды. Включает настройку консоли и логирования.
 */
class HyperEngineTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Инициализация тестовой среды
        setupTestEnvironment();

        // Настройка логирования для тестов
        setupTestLogging();
    }

    void TearDown() override {
        // Очистка после тестов
        cleanupTestEnvironment();
    }

  private:
    /**
     * @brief Настройка тестового окружения
     */
    void setupTestEnvironment() {
        // Инициализация консоли для вывода UTF-8
        std::cout << "Настройка тестового окружения..." << std::endl;
    }

    /**
     * @brief Настройка логирования для тестов
     */
    void setupTestLogging() {
        // Настройка минимального уровня логирования для тестов
        // (здесь будет добавлена интеграция с системой логирования)
    }

    /**
     * @brief Очистка тестового окружения
     */
    void cleanupTestEnvironment() {
        // Очистка ресурсов после тестов
    }
};

/**
 * @brief Макросы для удобного тестирования производительности и исключений
 */

/**
 * @brief Проверяет, что блок кода не выбрасывает исключений
 * @param ... Блок кода для выполнения и сообщение (variadic для поддержки запятых)
 */
#define EXPECT_NO_THROW_WITH_MESSAGE(...) \
    EXPECT_NO_THROW_WITH_MESSAGE_IMPL(__VA_ARGS__)
    
#define EXPECT_NO_THROW_WITH_MESSAGE_IMPL(statement, message) \
    try {                                                      \
        statement;                                             \
    } catch (const std::exception& e) {                        \
        FAIL() << message << ": " << e.what();                 \
    } catch (...) {                                            \
        FAIL() << message << ": Неизвестное исключение";       \
    }

/**
 * @brief Проверяет, что операция выполняется за указанное время
 * @param ... Блок кода для выполнения и максимальное время (variadic для поддержки запятых)
 */
#define EXPECT_PERFORMANCE_UNDER(...) \
    EXPECT_PERFORMANCE_UNDER_IMPL(__VA_ARGS__)

#define EXPECT_PERFORMANCE_UNDER_IMPL(statement, max_milliseconds)                              \
    {                                                                                           \
        auto start = std::chrono::high_resolution_clock::now();                                 \
        statement;                                                                              \
        auto end = std::chrono::high_resolution_clock::now();                                   \
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);     \
        EXPECT_LT(duration.count(), max_milliseconds)                                           \
            << "Операция заняла " << duration.count() << "ms, ожидалось < " << max_milliseconds \
            << "ms";                                                                            \
    }

/**
 * @brief Проверяет, что значение с плавающей точкой находится в диапазоне
 * @param value Проверяемое значение
 * @param min_val Минимальное допустимое значение
 * @param max_val Максимальное допустимое значение
 */
#define EXPECT_FLOAT_IN_RANGE(value, min_val, max_val)                                   \
    EXPECT_GE(value, min_val) << "Значение " << value << " меньше минимума " << min_val; \
    EXPECT_LE(value, max_val) << "Значение " << value << " больше максимума " << max_val

/**
 * @brief Проверяет, что вектор нормализован (длина ~= 1.0)
 * @param vector Проверяемый вектор (должен иметь метод magnitude())
 */
#define EXPECT_NORMALIZED(vector) \
    EXPECT_NEAR(vector.magnitude(), 1.0f, 1e-6f) << "Вектор не нормализован"

/**
 * @brief Утилиты для работы с тестовыми данными
 */
class TestUtils {
  public:
    /**
     * @brief Генерация случайного числа в диапазоне
     * @param min Минимальное значение
     * @param max Максимальное значение
     * @return Случайное число в диапазоне [min, max]
     */
    static float randomFloat(float min = 0.0f, float max = 1.0f) {
        return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
    }

    /**
     * @brief Генерация случайного целого числа в диапазоне
     * @param min Минимальное значение
     * @param max Максимальное значение
     * @return Случайное число в диапазоне [min, max]
     */
    static int randomInt(int min = 0, int max = 100) { return min + rand() % (max - min + 1); }

    /**
     * @brief Создание временного файла для тестов
     * @param content Содержимое файла
     * @return Путь к созданному файлу
     */
    static std::string createTempFile(const std::string& content) {
        // Подавляем предупреждение о неиспользуемом параметре
        (void)content;
        // Реализация создания временного файла
        // (упрощенная версия для демонстрации)
        return "temp_test_file.tmp";
    }

    /**
     * @brief Удаление временного файла
     * @param filepath Путь к файлу
     */
    static void removeTempFile(const std::string& filepath) {
        // Подавляем предупреждение о неиспользуемом параметре
        (void)filepath;
        // Реализация удаления файла
    }
};

/**
 * @brief Базовый класс для тестов производительности
 */
class PerformanceTest : public HyperEngineTest {
  protected:
    void SetUp() override {
        HyperEngineTest::SetUp();
        warmupSystem();
    }

  private:
    /**
     * @brief Прогрев системы для стабильных измерений производительности
     */
    void warmupSystem() {
        // Выполнение нескольких операций для прогрева кеша и процессора
        for (int i = 0; i < 1000; ++i) {
            volatile float result = static_cast<float>(i) * 1.1f;
            (void)result;  // Подавление предупреждения о неиспользуемой переменной
        }
    }
};

/**
 * @brief Параметризованные тесты для тестирования с различными входными данными
 */
template <typename T>
class ParameterizedTest : public HyperEngineTest, public ::testing::WithParamInterface<T> {
  protected:
    T getParam() { return ::testing::WithParamInterface<T>::GetParam(); }
};

}  // namespace HyperEngine::Testing
