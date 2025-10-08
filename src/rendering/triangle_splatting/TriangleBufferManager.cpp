#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingTypes.h>
#include <SpectraForge/Core/Console.h>
#include <stdexcept>
#include <cstring>

using SpectraForge::Core::Console;

namespace spectraforge {
namespace rendering {

bool TriangleBufferManager::initialize(vk::Device device, 
                                       VmaAllocator allocator, 
                                       uint32_t maxTriangles) {
    if (initialized_) {
        Console::info("TriangleBufferManager already initialized");
        return true;
    }
    
    if (!device || !allocator) {
        Console::error("Invalid device or allocator");
        return false;
    }
    
    device_ = device;
    allocator_ = allocator;
    maxTriangles_ = maxTriangles;
    
    // Создаем все буферы
    const vk::DeviceSize triangleBufferSize = static_cast<vk::DeviceSize>(maxTriangles) * sizeof(Triangle);
    const vk::DeviceSize indexBufferSize = static_cast<vk::DeviceSize>(maxTriangles) * sizeof(uint32_t);
    const vk::DeviceSize depthKeyBufferSize = static_cast<vk::DeviceSize>(maxTriangles) * sizeof(float);
    
    // Triangle buffer (STORAGE для compute shaders)
    if (!createBuffer(triangleBufferSize,
                     vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                     VMA_MEMORY_USAGE_GPU_ONLY,
                     triangleBuffer_,
                     triangleBufferAllocation_)) {
        Console::error("Failed to create triangle buffer");
        return false;
    }
    
    // Visible indices buffer
    if (!createBuffer(indexBufferSize,
                     vk::BufferUsageFlagBits::eStorageBuffer,
                     VMA_MEMORY_USAGE_GPU_ONLY,
                     visibleIndicesBuffer_,
                     visibleIndicesAllocation_)) {
        Console::error("Failed to create visible indices buffer");
        cleanup();
        return false;
    }
    
    // Sorted indices buffer
    if (!createBuffer(indexBufferSize,
                     vk::BufferUsageFlagBits::eStorageBuffer,
                     VMA_MEMORY_USAGE_GPU_ONLY,
                     sortedIndicesBuffer_,
                     sortedIndicesAllocation_)) {
        Console::error("Failed to create sorted indices buffer");
        cleanup();
        return false;
    }
    
    // Depth keys buffer
    if (!createBuffer(depthKeyBufferSize,
                     vk::BufferUsageFlagBits::eStorageBuffer,
                     VMA_MEMORY_USAGE_GPU_ONLY,
                     depthKeysBuffer_,
                     depthKeysAllocation_)) {
        Console::error("Failed to create depth keys buffer");
        cleanup();
        return false;
    }
    
    // Atomic counter buffer (1 uint32_t)
    if (!createBuffer(sizeof(uint32_t),
                     vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                     VMA_MEMORY_USAGE_GPU_ONLY,
                     atomicCounterBuffer_,
                     atomicCounterAllocation_)) {
        Console::error("Failed to create atomic counter buffer");
        cleanup();
        return false;
    }
    
    initialized_ = true;
    Console::info("TriangleBufferManager initialized successfully (max triangles: " + 
                 std::to_string(maxTriangles) + ")");
    
    return true;
}

void TriangleBufferManager::cleanup() {
    if (!initialized_) {
        return;
    }
    
    destroyBuffer(triangleBuffer_, triangleBufferAllocation_);
    destroyBuffer(visibleIndicesBuffer_, visibleIndicesAllocation_);
    destroyBuffer(sortedIndicesBuffer_, sortedIndicesAllocation_);
    destroyBuffer(depthKeysBuffer_, depthKeysAllocation_);
    destroyBuffer(atomicCounterBuffer_, atomicCounterAllocation_);
    
    triangleCount_ = 0;
    initialized_ = false;
    
    Console::info("TriangleBufferManager cleaned up");
}

void TriangleBufferManager::uploadTriangles(const std::vector<Triangle>& triangles,
                                            vk::CommandBuffer cmd,
                                            vk::Queue transferQueue) {
    if (!initialized_) {
        throw std::runtime_error("TriangleBufferManager not initialized");
    }
    
    if (triangles.empty()) {
        Console::info("Uploading 0 triangles");
        triangleCount_ = 0;
        return;
    }
    
    if (triangles.size() > maxTriangles_) {
        throw std::runtime_error("Triangle count (" + std::to_string(triangles.size()) + 
                                ") exceeds max (" + std::to_string(maxTriangles_) + ")");
    }
    
    // Создаем staging buffer
    vk::Buffer stagingBuffer;
    VmaAllocation stagingAllocation;
    
    const vk::DeviceSize bufferSize = triangles.size() * sizeof(Triangle);
    
    if (!createBuffer(bufferSize,
                     vk::BufferUsageFlagBits::eTransferSrc,
                     VMA_MEMORY_USAGE_CPU_TO_GPU,
                     stagingBuffer,
                     stagingAllocation)) {
        throw std::runtime_error("Failed to create staging buffer");
    }
    
    // Копируем данные в staging buffer
    void* data;
    vmaMapMemory(allocator_, stagingAllocation, &data);
    std::memcpy(data, triangles.data(), static_cast<size_t>(bufferSize));
    vmaUnmapMemory(allocator_, stagingAllocation);
    
    // Копируем из staging на GPU
    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    
    cmd.copyBuffer(stagingBuffer, triangleBuffer_, 1, &copyRegion);
    
    // Барьер для синхронизации
    vk::BufferMemoryBarrier barrier;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = triangleBuffer_;
    barrier.offset = 0;
    barrier.size = bufferSize;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        0, nullptr,
        1, &barrier,
        0, nullptr
    );
    
    // Удаляем staging buffer
    destroyBuffer(stagingBuffer, stagingAllocation);
    
    triangleCount_ = static_cast<uint32_t>(triangles.size());
    
    Console::info("Uploaded " + std::to_string(triangleCount_) + " triangles to GPU");
}

uint64_t TriangleBufferManager::getTotalMemoryUsage() const {
    if (!initialized_) {
        return 0;
    }
    
    uint64_t total = 0;
    
    // Triangle buffer
    total += static_cast<uint64_t>(maxTriangles_) * sizeof(Triangle);
    
    // Index buffers (2x)
    total += static_cast<uint64_t>(maxTriangles_) * sizeof(uint32_t) * 2;
    
    // Depth keys
    total += static_cast<uint64_t>(maxTriangles_) * sizeof(float);
    
    // Atomic counter
    total += sizeof(uint32_t);
    
    return total;
}

bool TriangleBufferManager::createBuffer(vk::DeviceSize size,
                                        vk::BufferUsageFlags usage,
                                        VmaMemoryUsage memoryUsage,
                                        vk::Buffer& buffer,
                                        VmaAllocation& allocation) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = static_cast<VkBufferUsageFlags>(usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;
    
    VkBuffer vkBuffer;
    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &vkBuffer, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        return false;
    }
    
    buffer = vk::Buffer(vkBuffer);
    return true;
}

void TriangleBufferManager::destroyBuffer(vk::Buffer& buffer, VmaAllocation& allocation) {
    if (buffer && allocation != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer), allocation);
        buffer = vk::Buffer();
        allocation = VK_NULL_HANDLE;
    }
}

} // namespace rendering
} // namespace spectraforge

