/**
 * @file VMAMemoryManager.cpp
 * @brief Implementation of VMA memory management
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/Core/VMAMemoryManager.h"
#include <iostream>
#include <stdexcept>

namespace spectraforge {
namespace core {

// ============================================================================
// VMABuffer Implementation
// ============================================================================

VMABuffer::~VMABuffer() {
    if (allocation_ != nullptr && allocator_ != nullptr) {
        if (mappedData_ != nullptr) {
            vmaUnmapMemory(allocator_, allocation_);
        }
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
    }
}

VMABuffer::VMABuffer(VMABuffer&& other) noexcept
    : allocator_(other.allocator_)
    , buffer_(other.buffer_)
    , allocation_(other.allocation_)
    , mappedData_(other.mappedData_)
    , size_(other.size_)
{
    other.allocator_ = nullptr;
    other.buffer_ = nullptr;
    other.allocation_ = nullptr;
    other.mappedData_ = nullptr;
    other.size_ = 0;
}

VMABuffer& VMABuffer::operator=(VMABuffer&& other) noexcept {
    if (this != &other) {
        // Cleanup current resources
        if (allocation_ != nullptr && allocator_ != nullptr) {
            if (mappedData_ != nullptr) {
                vmaUnmapMemory(allocator_, allocation_);
            }
            vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(buffer_), allocation_);
        }
        
        // Move from other
        allocator_ = other.allocator_;
        buffer_ = other.buffer_;
        allocation_ = other.allocation_;
        mappedData_ = other.mappedData_;
        size_ = other.size_;
        
        // Clear other
        other.allocator_ = nullptr;
        other.buffer_ = nullptr;
        other.allocation_ = nullptr;
        other.mappedData_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

void* VMABuffer::map() {
    if (mappedData_ != nullptr) {
        return mappedData_;  // Already mapped
    }
    
    VkResult result = vmaMapMemory(allocator_, allocation_, &mappedData_);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to map buffer memory: " << result << "\n";
        return nullptr;
    }
    
    return mappedData_;
}

void VMABuffer::unmap() {
    if (mappedData_ != nullptr) {
        vmaUnmapMemory(allocator_, allocation_);
        mappedData_ = nullptr;
    }
}

// ============================================================================
// VMAImage Implementation
// ============================================================================

VMAImage::~VMAImage() {
    if (allocation_ != nullptr && allocator_ != nullptr) {
        vmaDestroyImage(allocator_, static_cast<VkImage>(image_), allocation_);
    }
}

VMAImage::VMAImage(VMAImage&& other) noexcept
    : allocator_(other.allocator_)
    , image_(other.image_)
    , allocation_(other.allocation_)
    , extent_(other.extent_)
    , format_(other.format_)
{
    other.allocator_ = nullptr;
    other.image_ = nullptr;
    other.allocation_ = nullptr;
}

VMAImage& VMAImage::operator=(VMAImage&& other) noexcept {
    if (this != &other) {
        // Cleanup current resources
        if (allocation_ != nullptr && allocator_ != nullptr) {
            vmaDestroyImage(allocator_, static_cast<VkImage>(image_), allocation_);
        }
        
        // Move from other
        allocator_ = other.allocator_;
        image_ = other.image_;
        allocation_ = other.allocation_;
        extent_ = other.extent_;
        format_ = other.format_;
        
        // Clear other
        other.allocator_ = nullptr;
        other.image_ = nullptr;
        other.allocation_ = nullptr;
    }
    return *this;
}

// ============================================================================
// VMAMemoryManager Implementation
// ============================================================================

VMAMemoryManager& VMAMemoryManager::getInstance() {
    static VMAMemoryManager instance;
    return instance;
}

VMAMemoryManager::~VMAMemoryManager() {
    cleanup();
}

bool VMAMemoryManager::initialize(
    vk::Instance instance,
    vk::PhysicalDevice physicalDevice,
    vk::Device device,
    uint32_t vulkanApiVersion)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        std::cerr << "VMAMemoryManager already initialized\n";
        return true;
    }
    
    device_ = device;
    
    // Create VMA allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = vulkanApiVersion;
    allocatorInfo.physicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
    allocatorInfo.device = static_cast<VkDevice>(device);
    allocatorInfo.instance = static_cast<VkInstance>(instance);
    
    // Provide Vulkan function pointers for dynamic import to prevent null dereference
    // when VMA is built with VMA_DYNAMIC_VULKAN_FUNCTIONS enabled.
    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    
    // Enable budget tracking for statistics
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    
    VkResult result = vmaCreateAllocator(&allocatorInfo, &allocator_);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create VMA allocator: " << result << "\n";
        return false;
    }
    
    // Create transient resource pool with proper memory type selection
    VmaPoolCreateInfo poolInfo = {};
    poolInfo.flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;
    poolInfo.blockSize = 32 * 1024 * 1024;  // 32 MB blocks for transient resources
    poolInfo.minBlockCount = 1;
    poolInfo.maxBlockCount = 4;  // Max 128 MB for transients

    // Determine suitable memory type for typical transient image allocations
    VkImageCreateInfo tmpImageInfo{};
    tmpImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    tmpImageInfo.imageType = VK_IMAGE_TYPE_2D;
    tmpImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    tmpImageInfo.extent = { 64, 64, 1 };
    tmpImageInfo.mipLevels = 1;
    tmpImageInfo.arrayLayers = 1;
    tmpImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    tmpImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    tmpImageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    tmpImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    tmpImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo tmpAllocInfo{};
    tmpAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    uint32_t memoryTypeIndex = UINT32_MAX;
    result = vmaFindMemoryTypeIndexForImageInfo(allocator_, &tmpImageInfo, &tmpAllocInfo, &memoryTypeIndex);
    if (result == VK_SUCCESS && memoryTypeIndex != UINT32_MAX) {
        poolInfo.memoryTypeIndex = memoryTypeIndex;
        result = vmaCreatePool(allocator_, &poolInfo, &transientPool_);
        if (result != VK_SUCCESS) {
            std::cerr << "Warning: Failed to create transient pool: " << result << "\n";
            transientPool_ = nullptr;  // Continue without transient pool
        }
    } else {
        std::cerr << "Warning: Failed to determine memory type for transient pool: " << result << "\n";
        transientPool_ = nullptr;
    }
    
    initialized_ = true;
    std::cout << "VMAMemoryManager initialized successfully\n";
    
    return true;
}

void VMAMemoryManager::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return;
    }
    
    // Print final statistics
    auto stats = getStatistics();
    std::cout << "=== VMA Memory Statistics (Final) ===\n";
    std::cout << "Total Allocated: " << (stats.totalAllocatedBytes / 1024 / 1024) << " MB\n";
    std::cout << "Peak Used: " << (stats.peakUsedBytes / 1024 / 1024) << " MB\n";
    std::cout << "Allocations: " << stats.allocationCount << "\n";
    std::cout << "Deallocations: " << stats.deallocationCount << "\n";
    std::cout << "=====================================\n";
    
    // Destroy transient pool
    if (transientPool_ != nullptr) {
        vmaDestroyPool(allocator_, transientPool_);
        transientPool_ = nullptr;
    }
    
    // Destroy allocator
    if (allocator_ != nullptr) {
        vmaDestroyAllocator(allocator_);
        allocator_ = nullptr;
    }
    
    initialized_ = false;
}

VMABuffer VMAMemoryManager::createBuffer(
    size_t size,
    vk::BufferUsageFlags usage,
    ResourceUsage resourceUsage)
{
    if (!initialized_) {
        throw std::runtime_error("VMAMemoryManager not initialized");
    }
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = static_cast<VkBufferUsageFlags>(usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = getVmaAllocationInfo(resourceUsage);
    
    VkBuffer buffer;
    VmaAllocation allocation;
    
    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA buffer: " + std::to_string(result));
    }
    
    // Update statistics
    updateStatistics(resourceUsage, size, true);
    
    // Create RAII wrapper
    VMABuffer vmaBuffer;
    vmaBuffer.allocator_ = allocator_;
    vmaBuffer.buffer_ = vk::Buffer(buffer);
    vmaBuffer.allocation_ = allocation;
    vmaBuffer.size_ = size;
    
    return vmaBuffer;
}

VMAImage VMAMemoryManager::createImage(
    const vk::ImageCreateInfo& imageInfo,
    ResourceUsage resourceUsage)
{
    if (!initialized_) {
        throw std::runtime_error("VMAMemoryManager not initialized");
    }
    
    VkImageCreateInfo vkImageInfo = static_cast<VkImageCreateInfo>(imageInfo);
    VmaAllocationCreateInfo allocInfo = getVmaAllocationInfo(resourceUsage);
    
    VkImage image;
    VmaAllocation allocation;
    
    VkResult result = vmaCreateImage(allocator_, &vkImageInfo, &allocInfo, &image, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VMA image: " + std::to_string(result));
    }
    
    // Estimate image size for statistics
    size_t estimatedSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth;
    estimatedSize *= 4;  // Rough estimate (4 bytes per pixel)
    
    updateStatistics(resourceUsage, estimatedSize, true);
    
    // Create RAII wrapper
    VMAImage vmaImage;
    vmaImage.allocator_ = allocator_;
    vmaImage.image_ = vk::Image(image);
    vmaImage.allocation_ = allocation;
    vmaImage.extent_ = imageInfo.extent;
    vmaImage.format_ = imageInfo.format;
    
    return vmaImage;
}

VMABuffer VMAMemoryManager::createTransientBuffer(size_t size, vk::BufferUsageFlags usage) {
    if (!initialized_) {
        throw std::runtime_error("VMAMemoryManager not initialized");
    }
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = static_cast<VkBufferUsageFlags>(usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    // Use transient pool if available
    if (transientPool_ != nullptr) {
        allocInfo.pool = transientPool_;
    }
    
    VkBuffer buffer;
    VmaAllocation allocation;
    
    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        // Fallback: try without dedicated flag and without pool
        allocInfo.flags = 0;
        allocInfo.pool = VK_NULL_HANDLE;
        result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create transient buffer: " + std::to_string(result));
        }
    }
    
    updateStatistics(ResourceUsage::TRANSIENT, size, true);
    
    VMABuffer vmaBuffer;
    vmaBuffer.allocator_ = allocator_;
    vmaBuffer.buffer_ = vk::Buffer(buffer);
    vmaBuffer.allocation_ = allocation;
    vmaBuffer.size_ = size;
    
    return vmaBuffer;
}

VMAImage VMAMemoryManager::createTransientImage(const vk::ImageCreateInfo& imageInfo) {
    if (!initialized_) {
        throw std::runtime_error("VMAMemoryManager not initialized");
    }
    
    VkImageCreateInfo vkImageInfo = static_cast<VkImageCreateInfo>(imageInfo);
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    // Use transient pool if available
    if (transientPool_ != nullptr) {
        allocInfo.pool = transientPool_;
    }
    
    VkImage image;
    VmaAllocation allocation;
    
    VkResult result = vmaCreateImage(allocator_, &vkImageInfo, &allocInfo, &image, &allocation, nullptr);
    
    if (result != VK_SUCCESS) {
        // Fallback: try without dedicated flag and without pool
        allocInfo.flags = 0;
        allocInfo.pool = VK_NULL_HANDLE;
        result = vmaCreateImage(allocator_, &vkImageInfo, &allocInfo, &image, &allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create transient image: " + std::to_string(result));
        }
    }
    
    size_t estimatedSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth * 4;
    updateStatistics(ResourceUsage::TRANSIENT, estimatedSize, true);
    
    VMAImage vmaImage;
    vmaImage.allocator_ = allocator_;
    vmaImage.image_ = vk::Image(image);
    vmaImage.allocation_ = allocation;
    vmaImage.extent_ = imageInfo.extent;
    vmaImage.format_ = imageInfo.format;
    
    return vmaImage;
}

MemoryStatistics VMAMemoryManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return statistics_;
}

VmaAllocationCreateInfo VMAMemoryManager::getVmaAllocationInfo(ResourceUsage usage) const {
    VmaAllocationCreateInfo allocInfo = {};
    
    switch (usage) {
        case ResourceUsage::GPU_ONLY:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            break;
            
        case ResourceUsage::CPU_TO_GPU:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
            
        case ResourceUsage::GPU_TO_CPU:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                             VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
            
        case ResourceUsage::CPU_ONLY:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                             VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
            
        case ResourceUsage::TRANSIENT:
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            if (transientPool_ != nullptr) {
                allocInfo.pool = transientPool_;
            }
            break;
    }
    
    return allocInfo;
}

void VMAMemoryManager::updateStatistics(ResourceUsage usage, size_t size, bool isAllocation) {
    // Note: mutex already locked by caller
    
    if (isAllocation) {
        statistics_.totalAllocatedBytes += size;
        statistics_.totalUsedBytes += size;
        statistics_.allocationCount++;
        
        if (statistics_.totalUsedBytes > statistics_.peakUsedBytes) {
            statistics_.peakUsedBytes = statistics_.totalUsedBytes;
        }
        
        statistics_.usageBreakdown[usage] += size;
    } else {
        statistics_.totalUsedBytes -= size;
        statistics_.deallocationCount++;
        
        statistics_.usageBreakdown[usage] -= size;
    }
}

} // namespace core
} // namespace spectraforge

