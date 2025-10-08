#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <vector>
#include <cstdint>

namespace spectraforge {
namespace rendering {

// Forward declaration
struct Triangle;

/**
 * @brief Manages GPU buffers for triangle data
 * 
 * Ответственность: Управление Vulkan буферами для хранения треугольников,
 * индексов видимости и сортировки.
 * 
 * SOLID Compliance:
 * - SRP: Только управление буферами (никакой логики рендеринга)
 * - OCP: Закрыт для изменений, открыт для расширения через наследование
 * - DIP: Зависит от абстракций (vk::Device, VmaAllocator)
 */
class TriangleBufferManager {
public:
    /**
     * @brief Инициализация менеджера буферов
     * @param device Vulkan device
     * @param allocator VMA allocator
     * @param maxTriangles Максимальное количество треугольников
     * @return true если успешно
     */
    bool initialize(vk::Device device, 
                   VmaAllocator allocator, 
                   uint32_t maxTriangles);
    
    /**
     * @brief Очистка всех буферов
     */
    void cleanup();
    
    /**
     * @brief Загрузка треугольников на GPU
     * @param triangles Вектор треугольников
     * @param cmd Command buffer для transfer операций
     * @param transferQueue Transfer queue
     * @throws std::runtime_error если triangles.size() > maxTriangles
     */
    void uploadTriangles(const std::vector<Triangle>& triangles,
                        vk::CommandBuffer cmd,
                        vk::Queue transferQueue);
    
    // Getters для буферов
    vk::Buffer getTriangleBuffer() const { return triangleBuffer_; }
    vk::Buffer getVisibleIndicesBuffer() const { return visibleIndicesBuffer_; }
    vk::Buffer getSortedIndicesBuffer() const { return sortedIndicesBuffer_; }
    vk::Buffer getDepthKeysBuffer() const { return depthKeysBuffer_; }
    vk::Buffer getAtomicCounterBuffer() const { return atomicCounterBuffer_; }
    
    // Getters для статистики
    uint32_t getTriangleCount() const { return triangleCount_; }
    uint32_t getMaxTriangles() const { return maxTriangles_; }
    uint64_t getTotalMemoryUsage() const;
    
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    
    vk::Device device_;
    VmaAllocator allocator_;
    
    // Triangle buffer (основной буфер с данными треугольников)
    vk::Buffer triangleBuffer_;
    VmaAllocation triangleBufferAllocation_ = VK_NULL_HANDLE;
    uint32_t triangleCount_ = 0;
    uint32_t maxTriangles_ = 100000;
    
    // Visible indices buffer (индексы видимых треугольников после frustum culling)
    vk::Buffer visibleIndicesBuffer_;
    VmaAllocation visibleIndicesAllocation_ = VK_NULL_HANDLE;
    
    // Sorted indices buffer (индексы в порядке back-to-front)
    vk::Buffer sortedIndicesBuffer_;
    VmaAllocation sortedIndicesAllocation_ = VK_NULL_HANDLE;
    
    // Depth keys buffer (ключи глубины для сортировки)
    vk::Buffer depthKeysBuffer_;
    VmaAllocation depthKeysAllocation_ = VK_NULL_HANDLE;
    
    // Atomic counter buffer (счетчик видимых треугольников)
    vk::Buffer atomicCounterBuffer_;
    VmaAllocation atomicCounterAllocation_ = VK_NULL_HANDLE;
    
    // Helper functions
    bool createBuffer(vk::DeviceSize size,
                     vk::BufferUsageFlags usage,
                     VmaMemoryUsage memoryUsage,
                     vk::Buffer& buffer,
                     VmaAllocation& allocation);
    
    void destroyBuffer(vk::Buffer& buffer, VmaAllocation& allocation);
};

} // namespace rendering
} // namespace spectraforge

