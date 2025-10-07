/**
 * @file core_vma_memory_test.cpp
 * @brief Комплексные тесты для VMAMemoryManager
 */

#include <gtest/gtest.h>
#include <SpectraForge/Core/VMAMemoryManager.h>

using namespace spectraforge::core;

class VMAMemoryManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Получаем синглтон
        manager = &VMAMemoryManager::getInstance();
    }

    void TearDown() override {
        // Очищаем после каждого теста
        if (manager && manager->isInitialized()) {
            manager->cleanup();
        }
    }

    VMAMemoryManager* manager;
};

// ============================================================================
// Singleton паттерн
// ============================================================================

TEST_F(VMAMemoryManagerTest, SingletonPattern) {
    // Arrange & Act
    VMAMemoryManager& instance1 = VMAMemoryManager::getInstance();
    VMAMemoryManager& instance2 = VMAMemoryManager::getInstance();
    
    // Assert - должны быть одинаковыми
    EXPECT_EQ(&instance1, &instance2);
}

// ============================================================================
// Инициализация
// ============================================================================

TEST_F(VMAMemoryManagerTest, IsInitializedByDefault) {
    // Arrange & Act
    bool initialized = manager->isInitialized();
    
    // Assert - по умолчанию не инициализирован
    EXPECT_FALSE(initialized);
}

// Примечание: Полная инициализация требует Vulkan контекста
// Эти тесты будут работать только в интеграционных тестах

TEST_F(VMAMemoryManagerTest, GetAllocatorBeforeInit) {
    // Arrange & Act
    VmaAllocator allocator = manager->getAllocator();
    
    // Assert - должен быть nullptr до инициализации
    EXPECT_EQ(allocator, nullptr);
}

// ============================================================================
// Статистика
// ============================================================================

TEST_F(VMAMemoryManagerTest, GetStatistics) {
    // Arrange & Act
    MemoryStatistics stats = manager->getStatistics();
    
    // Assert - изначально все нули
    EXPECT_EQ(stats.totalAllocatedBytes, 0);
    EXPECT_EQ(stats.totalUsedBytes, 0);
    EXPECT_EQ(stats.allocationCount, 0);
    EXPECT_EQ(stats.deallocationCount, 0);
}

// ============================================================================
// VMABuffer RAII
// ============================================================================

TEST_F(VMAMemoryManagerTest, VMABufferDefaultConstructor) {
    // Arrange & Act
    VMABuffer buffer;
    
    // Assert
    EXPECT_EQ(buffer.getBuffer(), vk::Buffer());
    EXPECT_EQ(buffer.getAllocation(), nullptr);
    EXPECT_EQ(buffer.getMappedData(), nullptr);
    EXPECT_EQ(buffer.getSize(), 0);
}

TEST_F(VMAMemoryManagerTest, VMABufferMoveConstructor) {
    // Arrange
    VMABuffer buffer1;
    
    // Act
    VMABuffer buffer2(std::move(buffer1));
    
    // Assert - не должно падать
    EXPECT_EQ(buffer2.getSize(), 0);
}

TEST_F(VMAMemoryManagerTest, VMABufferMoveAssignment) {
    // Arrange
    VMABuffer buffer1;
    VMABuffer buffer2;
    
    // Act
    buffer2 = std::move(buffer1);
    
    // Assert
    EXPECT_EQ(buffer2.getSize(), 0);
}

// ============================================================================
// VMAImage RAII
// ============================================================================

TEST_F(VMAMemoryManagerTest, VMAImageDefaultConstructor) {
    // Arrange & Act
    VMAImage image;
    
    // Assert
    EXPECT_EQ(image.getImage(), vk::Image());
    EXPECT_EQ(image.getAllocation(), nullptr);
}

TEST_F(VMAMemoryManagerTest, VMAImageMoveConstructor) {
    // Arrange
    VMAImage image1;
    
    // Act
    VMAImage image2(std::move(image1));
    
    // Assert - не должно падать
    EXPECT_EQ(image2.getImage(), vk::Image());
}

TEST_F(VMAMemoryManagerTest, VMAImageMoveAssignment) {
    // Arrange
    VMAImage image1;
    VMAImage image2;
    
    // Act
    image2 = std::move(image1);
    
    // Assert
    EXPECT_EQ(image2.getImage(), vk::Image());
}

TEST_F(VMAMemoryManagerTest, VMAImageGetters) {
    // Arrange
    VMAImage image;
    
    // Act
    vk::Extent3D extent = image.getExtent();
    vk::Format format = image.getFormat();
    
    // Assert - проверяем, что геттеры не падают
    EXPECT_EQ(extent.width, 0);
    EXPECT_EQ(extent.height, 0);
    EXPECT_EQ(extent.depth, 0);
}

// ============================================================================
// ResourceUsage enum
// ============================================================================

TEST_F(VMAMemoryManagerTest, ResourceUsageValues) {
    // Arrange & Act & Assert - проверяем, что все значения определены
    ResourceUsage usage1 = ResourceUsage::GPU_ONLY;
    ResourceUsage usage2 = ResourceUsage::CPU_TO_GPU;
    ResourceUsage usage3 = ResourceUsage::GPU_TO_CPU;
    ResourceUsage usage4 = ResourceUsage::CPU_ONLY;
    ResourceUsage usage5 = ResourceUsage::TRANSIENT;
    
    EXPECT_NE(static_cast<int>(usage1), static_cast<int>(usage2));
    EXPECT_NE(static_cast<int>(usage2), static_cast<int>(usage3));
    EXPECT_NE(static_cast<int>(usage3), static_cast<int>(usage4));
    EXPECT_NE(static_cast<int>(usage4), static_cast<int>(usage5));
}

// ============================================================================
// MemoryStatistics структура
// ============================================================================

TEST_F(VMAMemoryManagerTest, MemoryStatisticsDefaultValues) {
    // Arrange & Act
    MemoryStatistics stats;
    
    // Assert
    EXPECT_EQ(stats.totalAllocatedBytes, 0);
    EXPECT_EQ(stats.totalUsedBytes, 0);
    EXPECT_EQ(stats.peakUsedBytes, 0);
    EXPECT_EQ(stats.allocationCount, 0);
    EXPECT_EQ(stats.deallocationCount, 0);
    EXPECT_TRUE(stats.usageBreakdown.empty());
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(VMAMemoryManagerTest, MultipleCleanupCalls) {
    // Arrange & Act & Assert - множественные вызовы cleanup не должны падать
    EXPECT_NO_THROW(manager->cleanup());
    EXPECT_NO_THROW(manager->cleanup());
    EXPECT_NO_THROW(manager->cleanup());
}

TEST_F(VMAMemoryManagerTest, GetStatisticsAfterCleanup) {
    // Arrange
    manager->cleanup();
    
    // Act
    MemoryStatistics stats = manager->getStatistics();
    
    // Assert
    EXPECT_EQ(stats.totalAllocatedBytes, 0);
}

TEST_F(VMAMemoryManagerTest, IsInitializedAfterCleanup) {
    // Arrange
    manager->cleanup();
    
    // Act
    bool initialized = manager->isInitialized();
    
    // Assert
    EXPECT_FALSE(initialized);
}

// ============================================================================
// Buffer mapping (без инициализации)
// ============================================================================

TEST_F(VMAMemoryManagerTest, VMABufferMapUninitialized) {
    // Arrange
    VMABuffer buffer;
    
    // Act
    void* data = buffer.map();
    
    // Assert - должен вернуть nullptr для неинициализированного буфера
    EXPECT_EQ(data, nullptr);
}

TEST_F(VMAMemoryManagerTest, VMABufferUnmapUninitialized) {
    // Arrange
    VMABuffer buffer;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(buffer.unmap());
}

TEST_F(VMAMemoryManagerTest, VMABufferMultipleUnmap) {
    // Arrange
    VMABuffer buffer;
    
    // Act & Assert - множественные unmap не должны падать
    EXPECT_NO_THROW(buffer.unmap());
    EXPECT_NO_THROW(buffer.unmap());
}

// ============================================================================
// RAII гарантии
// ============================================================================

TEST_F(VMAMemoryManagerTest, VMABufferDestructorSafe) {
    // Arrange & Act & Assert
    {
        VMABuffer buffer;
    }  // Деструктор вызывается здесь, не должно падать
    SUCCEED();
}

TEST_F(VMAMemoryManagerTest, VMAImageDestructorSafe) {
    // Arrange & Act & Assert
    {
        VMAImage image;
    }  // Деструктор вызывается здесь
    SUCCEED();
}

TEST_F(VMAMemoryManagerTest, MovedBufferDestructorSafe) {
    // Arrange
    VMABuffer buffer1;
    
    // Act
    {
        VMABuffer buffer2 = std::move(buffer1);
    }  // buffer2 уничтожается
    
    // Assert - buffer1 все еще валиден
    EXPECT_EQ(buffer1.getSize(), 0);
}

TEST_F(VMAMemoryManagerTest, MovedImageDestructorSafe) {
    // Arrange
    VMAImage image1;
    
    // Act
    {
        VMAImage image2 = std::move(image1);
    }
    
    // Assert
    EXPECT_EQ(image1.getImage(), vk::Image());
}

// ============================================================================
// Thread Safety (базовый тест)
// ============================================================================

TEST_F(VMAMemoryManagerTest, ConcurrentStatisticsAccess) {
    // Arrange
    const int threadCount = 10;
    std::vector<std::thread> threads;
    
    // Act - множественные потоки читают статистику
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([this]() {
            MemoryStatistics stats = manager->getStatistics();
            (void)stats;  // Используем stats
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Assert - не должно быть race conditions
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
