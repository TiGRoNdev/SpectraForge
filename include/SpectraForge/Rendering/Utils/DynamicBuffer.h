#pragma once

#include <vulkan/vulkan.hpp>

#include "SpectraForge/Core/VMAMemoryManager.h"

namespace spectraforge::rendering::utils {

/**
 * @brief RAII wrapper over VMA buffers that unifies allocation, descriptor updates
 *        and host mapping for compute/graphics passes.
 */
class DynamicBuffer {
public:
    DynamicBuffer() = default;
    ~DynamicBuffer() = default;

    DynamicBuffer(const DynamicBuffer&) = delete;
    DynamicBuffer& operator=(const DynamicBuffer&) = delete;
    DynamicBuffer(DynamicBuffer&&) noexcept = default;
    DynamicBuffer& operator=(DynamicBuffer&&) noexcept = default;

    /**
     * @brief Allocate a buffer with the requested properties. Releases any existing allocation.
     * @param size          Size of the buffer in bytes.
     * @param usage         Usage flags for vk::Buffer creation.
     * @param memoryUsage   Memory domain hint for VMA allocation.
     * @return true on success, false on failure (errors are logged).
     */
    bool create(size_t size,
                vk::BufferUsageFlags usage,
                spectraforge::core::ResourceUsage memoryUsage);

    /**
     * @brief Ensure the buffer has at least the requested capacity, recreating if necessary.
     * @return true if the buffer is valid and meets the requested capacity.
     */
    bool ensure(size_t requiredSize,
                vk::BufferUsageFlags usage,
                spectraforge::core::ResourceUsage memoryUsage);

    /**
     * @brief Destroy the underlying allocation.
     */
    void reset();

    [[nodiscard]] bool valid() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] vk::Buffer handle() const;

    void* map();
    void unmap();

    [[nodiscard]] vk::DescriptorBufferInfo descriptor(vk::DeviceSize offset = 0,
                                                      vk::DeviceSize range = VK_WHOLE_SIZE) const;

    void writeDescriptor(vk::Device device,
                         vk::DescriptorSet set,
                         uint32_t binding,
                         vk::DescriptorType type,
                         vk::DeviceSize offset = 0,
                         vk::DeviceSize range = VK_WHOLE_SIZE) const;

private:
    spectraforge::core::VMABuffer buffer_{};
    size_t size_ = 0;
    vk::BufferUsageFlags usage_{};
    spectraforge::core::ResourceUsage memoryUsage_ = spectraforge::core::ResourceUsage::GPU_ONLY;
};

} // namespace spectraforge::rendering::utils

