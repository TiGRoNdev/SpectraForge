/**
 * @file <component>_integration_test.cpp
 * @brief Интеграционные тесты для <System>
 * 
 * Этот файл содержит интеграционные (E2E) тесты, проверяющие
 * взаимодействие нескольких компонентов системы.
 * 
 * @author SpectraForge Team
 * @date 2025-10-08
 * @version 1.0.0
 */

#include <gtest/gtest.h>
#include <SpectraForge/<Module>/<SystemComponent>.h>
#include <SpectraForge/<Module>/<DependentComponent1>.h>
#include <SpectraForge/<Module>/<DependentComponent2>.h>

using namespace SpectraForge::<Module>;

// ============================================================================
// Integration Test Fixture
// ============================================================================

/**
 * @brief Test Fixture для интеграционных тестов <System>
 * 
 * Создает полноценную тестовую среду с реальными компонентами
 * (минимум mocking для интеграционных тестов)
 */
class <System>IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создание реальных компонентов (не mocks!)
        component1 = std::make_shared<<DependentComponent1>>();
        component2 = std::make_shared<<DependentComponent2>>();
        
        // Создание основной системы
        system = std::make_unique<<SystemComponent>>(component1, component2);
    }
    
    void TearDown() override {
        system->shutdown();
        system.reset();
        component1.reset();
        component2.reset();
    }
    
    /**
     * @brief Создание тестовых данных для полного сценария
     */
    TestSceneData createTestScene() {
        TestSceneData scene;
        // Заполнение тестовыми данными
        return scene;
    }
    
    // Реальные компоненты
    std::shared_ptr<<DependentComponent1>> component1;
    std::shared_ptr<<DependentComponent2>> component2;
    std::unique_ptr<<SystemComponent>> system;
};

// ============================================================================
// Full Lifecycle Integration Tests
// ============================================================================

/**
 * @brief Интеграционный тест: полный жизненный цикл системы
 * 
 * @test Проверяет последовательность: Initialize → Load → Process → Shutdown
 */
TEST_F(<System>IntegrationTest, FullLifecycle_NormalFlow_CompletesSuccessfully) {
    // Arrange
    auto testData = createTestScene();
    
    // Act - Initialize
    ASSERT_TRUE(system->initialize()) 
        << "Система должна успешно инициализироваться";
    
    // Act - Load Data
    ASSERT_TRUE(system->loadData(testData))
        << "Данные должны загрузиться корректно";
    
    // Act - Process
    bool processResult = system->process();
    EXPECT_TRUE(processResult)
        << "Обработка должна завершиться успешно";
    
    // Act - Shutdown
    EXPECT_NO_THROW({
        system->shutdown();
    }) << "Shutdown должен выполниться без исключений";
    
    // Assert
    EXPECT_FALSE(system->isActive())
        << "Система должна быть неактивной после shutdown";
}

/**
 * @brief Интеграционный тест: цепочка зависимостей
 * 
 * @test Проверяет, что изменения в Component1 корректно обрабатываются Component2
 */
TEST_F(<System>IntegrationTest, DependencyChain_Component1ToComponent2_DataFlowsCorrectly) {
    // Arrange
    system->initialize();
    
    // Act - Изменение в Component1
    component1->setValue(42);
    system->process();
    
    // Assert - Component2 получил данные
    EXPECT_EQ(component2->getProcessedValue(), 84)
        << "Component2 должен получить и обработать данные от Component1";
}

// ============================================================================
// Error Recovery Integration Tests
// ============================================================================

/**
 * @brief Интеграционный тест: восстановление после ошибки
 * 
 * @test Проверяет, что система корректно восстанавливается после сбоя компонента
 */
TEST_F(<System>IntegrationTest, ErrorRecovery_Component1Fails_SystemRecoversGracefully) {
    // Arrange
    system->initialize();
    
    // Act - Симулируем ошибку в Component1
    component1->simulateError();
    bool result = system->process();
    
    // Assert - Система обработала ошибку
    EXPECT_FALSE(result)
        << "Process должен вернуть false при ошибке";
    
    // Act - Восстановление
    component1->reset();
    result = system->process();
    
    // Assert - Система восстановилась
    EXPECT_TRUE(result)
        << "Система должна восстановиться после reset компонента";
}

// ============================================================================
// Multi-Component Interaction Tests
// ============================================================================

/**
 * @brief Интеграционный тест: параллельная обработка несколькими компонентами
 * 
 * @test Проверяет корректную работу при параллельной обработке
 */
TEST_F(<System>IntegrationTest, ParallelProcessing_MultipleComponents_ProducesCorrectResults) {
    // Arrange
    system->initialize();
    auto testData1 = createTestScene();
    auto testData2 = createTestScene();
    
    // Act - Запуск параллельной обработки
    system->processParallel({testData1, testData2});
    
    // Assert - Оба набора данных обработаны
    auto results = system->getResults();
    EXPECT_EQ(results.size(), 2)
        << "Должны быть обработаны оба набора данных";
    
    EXPECT_TRUE(results[0].isValid())
        << "Первый результат должен быть валидным";
    EXPECT_TRUE(results[1].isValid())
        << "Второй результат должен быть валидным";
}

// ============================================================================
// Resource Management Integration Tests
// ============================================================================

/**
 * @brief Интеграционный тест: управление ресурсами
 * 
 * @test Проверяет корректное управление ресурсами при множественных операциях
 */
TEST_F(<System>IntegrationTest, ResourceManagement_MultipleOperations_NoLeaks) {
    // Arrange
    system->initialize();
    size_t initialMemory = system->getMemoryUsage();
    
    // Act - Выполняем 100 операций
    for (int i = 0; i < 100; ++i) {
        auto data = createTestScene();
        system->loadData(data);
        system->process();
        system->clear();
    }
    
    // Assert - Память не утекла
    size_t finalMemory = system->getMemoryUsage();
    EXPECT_LE(finalMemory, initialMemory * 1.1f) // +10% допустимо
        << "Утечка памяти: начальная=" << initialMemory 
        << ", конечная=" << finalMemory;
}

// ============================================================================
// Performance Integration Tests
// ============================================================================

/**
 * @brief Интеграционный тест: производительность системы
 * 
 * @test Проверяет, что система обрабатывает данные за приемлемое время
 */
TEST_F(<System>IntegrationTest, Performance_LargeDataSet_CompletesInTime) {
    // Arrange
    system->initialize();
    auto largeDataSet = createLargeTestScene(10000); // 10k элементов
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Act
    system->loadData(largeDataSet);
    bool result = system->process();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Assert
    EXPECT_TRUE(result)
        << "Обработка должна завершиться успешно";
    
    EXPECT_LT(duration.count(), 500) // Менее 500мс
        << "Обработка заняла слишком много времени: " 
        << duration.count() << "мс";
}

/**
 * @brief Интеграционный тест: throughput системы
 * 
 * @test Проверяет пропускную способность (операций в секунду)
 */
TEST_F(<System>IntegrationTest, Throughput_ContinuousProcessing_MeetsRequirements) {
    // Arrange
    system->initialize();
    const int targetOpsPerSec = 60; // Например, 60 FPS
    const int testDurationSec = 2;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    int completedOps = 0;
    
    // Act
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
        
        if (elapsed.count() >= testDurationSec) {
            break;
        }
        
        auto data = createTestScene();
        system->loadData(data);
        if (system->process()) {
            ++completedOps;
        }
    }
    
    // Assert
    float actualOpsPerSec = static_cast<float>(completedOps) / testDurationSec;
    EXPECT_GE(actualOpsPerSec, targetOpsPerSec)
        << "Throughput недостаточен: " << actualOpsPerSec 
        << " ops/sec (требуется " << targetOpsPerSec << ")";
}

// ============================================================================
// State Consistency Integration Tests
// ============================================================================

/**
 * @brief Интеграционный тест: консистентность состояния
 * 
 * @test Проверяет, что состояние системы остается консистентным при различных операциях
 */
TEST_F(<System>IntegrationTest, StateConsistency_VariousOperations_RemainsConsistent) {
    // Arrange
    system->initialize();
    
    // Act & Assert - Последовательность операций
    auto data1 = createTestScene();
    ASSERT_TRUE(system->loadData(data1));
    EXPECT_TRUE(system->isDataLoaded());
    
    ASSERT_TRUE(system->process());
    EXPECT_TRUE(system->hasResults());
    
    system->clear();
    EXPECT_FALSE(system->isDataLoaded());
    EXPECT_FALSE(system->hasResults());
    
    // Act & Assert - Повторная загрузка
    auto data2 = createTestScene();
    ASSERT_TRUE(system->loadData(data2));
    EXPECT_TRUE(system->isDataLoaded());
}

// ============================================================================
// Concurrency Integration Tests (если применимо)
// ============================================================================

/**
 * @brief Интеграционный тест: потокобезопасность
 * 
 * @test Проверяет корректную работу при многопоточном доступе
 */
TEST_F(<System>IntegrationTest, Concurrency_MultipleThreads_NoRaceConditions) {
    // Arrange
    system->initialize();
    const int numThreads = 4;
    const int opsPerThread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    // Act - Запуск параллельных потоков
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, &successCount, opsPerThread]() {
            for (int i = 0; i < opsPerThread; ++i) {
                auto data = createTestScene();
                if (system->processThreadSafe(data)) {
                    ++successCount;
                }
            }
        });
    }
    
    // Ожидание завершения
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Assert
    EXPECT_EQ(successCount, numThreads * opsPerThread)
        << "Некоторые операции не выполнились в многопоточной среде";
}

// ============================================================================
// End-to-End Scenario Tests
// ============================================================================

/**
 * @brief E2E тест: реалистичный сценарий использования
 * 
 * @test Проверяет полный сценарий работы приложения
 */
TEST_F(<System>IntegrationTest, E2E_RealisticWorkflow_CompletesSuccessfully) {
    // Arrange - Инициализация
    ASSERT_TRUE(system->initialize());
    
    // Act - Scenario Step 1: Загрузка ресурсов
    auto resources = loadTestResources();
    ASSERT_TRUE(system->loadResources(resources));
    
    // Act - Scenario Step 2: Настройка параметров
    system->configure({
        {"quality", "high"},
        {"mode", "production"}
    });
    
    // Act - Scenario Step 3: Обработка данных
    auto scene = createTestScene();
    ASSERT_TRUE(system->loadData(scene));
    ASSERT_TRUE(system->process());
    
    // Act - Scenario Step 4: Получение результатов
    auto results = system->getResults();
    EXPECT_FALSE(results.empty());
    
    // Act - Scenario Step 5: Экспорт
    bool exportSuccess = system->exportResults("test_output.dat");
    EXPECT_TRUE(exportSuccess);
    
    // Act - Scenario Step 6: Cleanup
    system->shutdown();
    
    // Assert - Финальные проверки
    EXPECT_FALSE(system->isActive());
    EXPECT_TRUE(fileExists("test_output.dat"));
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Создание большого тестового набора данных
 */
TestSceneData createLargeTestScene(size_t size) {
    TestSceneData scene;
    scene.items.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        scene.items.push_back(createTestItem());
    }
    return scene;
}

/**
 * @brief Проверка существования файла
 */
bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

