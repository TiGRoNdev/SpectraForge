/**
 * @file VMAMemoryManager.h
 * @brief Vulkan Memory Allocator (VMA) wrapper for efficient GPU memory management
 *
 * Provides RAII-based memory management with specialized pools for transient resources.
 * Implements best practices from VMA documentation for optimal performance.
 *
 * Features:
 * - Memory pools for short-lived resources (render passes)
 * - Automatic defragmentation
 * - Budget tracking and statistics
 * - Thread-safe allocation/deallocation
 *
 * @see https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace spectraforge {
namespace core {

/**
 * @brief Resource usage hints for VMA allocation
 */
enum class ResourceUsage {
    GPU_ONLY,              ///< Device-local, no CPU access (images, vertex buffers)
    CPU_TO_GPU,            ///< Staging buffers, uniform buffers (updated frequently)
    GPU_TO_CPU,            ///< Readback buffers (download GPU results)
    CPU_ONLY,              ///< Temporary CPU-side storage
    TRANSIENT              ///< Short-lived resources (1-2 frames)
};

/**
 * @brief Memory allocation statistics
 */
struct MemoryStatistics {
    uint64_t totalAllocatedBytes = 0;
    uint64_t totalUsedBytes = 0;
    uint64_t peakUsedBytes = 0;
    uint32_t allocationCount = 0;
    uint32_t deallocationCount = 0;
    
    // Per-usage-type breakdown
    std::unordered_map<ResourceUsage, uint64_t> usageBreakdown;
};

/**
 * @brief RAII wrapper for VMA buffer allocation
 */
class VMABuffer {
public:
    VMABuffer() = default;
    ~VMABuffer();
    
    // Delete copy, allow move
    VMABuffer(const VMABuffer&) = delete;
    VMABuffer& operator=(const VMABuffer&) = delete;
    VMABuffer(VMABuffer&& other) noexcept;
    VMABuffer& operator=(VMABuffer&& other) noexcept;
    
    vk::Buffer getBuffer() const { return buffer_; }
    VmaAllocation getAllocation() const { return allocation_; }
    void* getMappedData() const { return mappedData_; }
    size_t getSize() const { return size_; }
    
    /**
     * @brief Map buffer memory for CPU access
     * @return Pointer to mapped memory, or nullptr on failure
     */
    void* map();
    
    /**
     * @brief Unmap buffer memory
     */
    void unmap();
    
private:
    friend class VMAMemoryManager;
    
    VmaAllocator allocator_ = nullptr;
    vk::Buffer buffer_;
    VmaAllocation allocation_ = nullptr;
    void* mappedData_ = nullptr;
    size_t size_ = 0;
};

/**
 * @brief RAII wrapper for VMA image allocation
 */
class VMAImage {
public:
    VMAImage() = default;
    ~VMAImage();
    
    // Delete copy, allow move
    VMAImage(const VMAImage&) = delete;
    VMAImage& operator=(const VMAImage&) = delete;
    VMAImage(VMAImage&& other) noexcept;
    VMAImage& operator=(VMAImage&& other) noexcept;
    
    vk::Image getImage() const { return image_; }
    VmaAllocation getAllocation() const { return allocation_; }
    vk::Extent3D getExtent() const { return extent_; }
    vk::Format getFormat() const { return format_; }
    
private:
    friend class VMAMemoryManager;
    
    VmaAllocator allocator_ = nullptr;
    vk::Image image_;
    VmaAllocation allocation_ = nullptr;
    vk::Extent3D extent_;
    vk::Format format_;
};

/**
 * @brief Central manager for VMA allocations (Singleton pattern)
 * 
 * SRP: Single responsibility - GPU memory management
 * OCP: Extensible through ResourceUsage enum
 * DIP: Depends on VMA abstraction, not concrete Vulkan details
 */
class VMAMemoryManager {
public:
    /**
     * @brief Get singleton instance
     */
    static VMAMemoryManager& getInstance();
    
    /**
     * @brief Initialize VMA with Vulkan context
     * @param instance Vulkan instance
     * @param physicalDevice Physical device
     * @param device Logical device
     * @param vulkanApiVersion Vulkan API version (e.g., VK_API_VERSION_1_3)
     * @return true on success
     */
    bool initialize(
        vk::Instance instance,
        vk::PhysicalDevice physicalDevice,
        vk::Device device,
        uint32_t vulkanApiVersion = VK_API_VERSION_1_3
    );
    
    /**
     * @brief Cleanup VMA and all allocations
     */
    void cleanup();
    
    /**
     * @brief Create buffer with VMA
     * @param size Buffer size in bytes
     * @param usage Vulkan buffer usage flags
     * @param resourceUsage Memory usage hint
     * @return RAII buffer wrapper
     */
    VMABuffer createBuffer(
        size_t size,
        vk::BufferUsageFlags usage,
        ResourceUsage resourceUsage = ResourceUsage::GPU_ONLY
    );
    
    /**
     * @brief Create image with VMA
     * @param imageInfo Vulkan image create info
     * @param resourceUsage Memory usage hint
     * @return RAII image wrapper
     */
    VMAImage createImage(
        const vk::ImageCreateInfo& imageInfo,
        ResourceUsage resourceUsage = ResourceUsage::GPU_ONLY
    );
    
    /**
     * @brief Create transient buffer (short-lived, 1-2 frames)
     * @param size Buffer size
     * @param usage Buffer usage flags
     * @return RAII buffer from transient pool
     */
    VMABuffer createTransientBuffer(size_t size, vk::BufferUsageFlags usage);
    
    /**
     * @brief Create transient image (short-lived, 1-2 frames)
     * @param imageInfo Image create info
     * @return RAII image from transient pool
     */
    VMAImage createTransientImage(const vk::ImageCreateInfo& imageInfo);
    
    /**
     * @brief Get current memory statistics
     */
    MemoryStatistics getStatistics() const;
    
    /**
     * @brief Check if VMA is initialized
     */
    bool isInitialized() const { return allocator_ != nullptr; }
    
    /**
     * @brief Get VMA allocator handle (for advanced usage)
     */
    VmaAllocator getAllocator() const { return allocator_; }
    
private:
    VMAMemoryManager() = default;
    ~VMAMemoryManager();
    
    // Delete copy and move
    VMAMemoryManager(const VMAMemoryManager&) = delete;
    VMAMemoryManager& operator=(const VMAMemoryManager&) = delete;
    
    /**
     * @brief Convert ResourceUsage to VMA allocation flags
     */
    VmaAllocationCreateInfo getVmaAllocationInfo(ResourceUsage usage) const;
    
    /**
     * @brief Update statistics after allocation
     */
    void updateStatistics(ResourceUsage usage, size_t size, bool isAllocation);
    
    VmaAllocator allocator_ = nullptr;
    VmaPool transientPool_ = nullptr;
    
    mutable std::mutex mutex_;  ///< Thread-safe operations
    MemoryStatistics statistics_;
    
    vk::Device device_;
    bool initialized_ = false;
};

} // namespace core
} // namespace spectraforge

