#include "SpectraForge/Rendering/Utils/DynamicBuffer.h"

#include <stdexcept>
#include <string>

#include "SpectraForge/Core/SafeConsole.h"

namespace spectraforge::rendering::utils {

namespace {
constexpr const char* kLogTag = "[DynamicBuffer] ";
}

bool DynamicBuffer::create(size_t size,
                           vk::BufferUsageFlags usage,
                           spectraforge::core::ResourceUsage memoryUsage) {
    reset();

    if (size == 0) {
        SAFE_ERROR(std::string{kLogTag} + "requested zero-sized allocation");
        return false;
    }

    try {
        auto& allocator = spectraforge::core::VMAMemoryManager::getInstance();
        buffer_ = allocator.createBuffer(size, usage, memoryUsage);
        size_ = size;
        usage_ = usage;
        memoryUsage_ = memoryUsage;
        SAFE_PRINT_LINE(std::string{kLogTag} + "allocated " + SpectraForge::Core::SAFE_TO_STRING(size) + " bytes");
        return true;
    } catch (const std::exception& e) {
        SAFE_ERROR(std::string{kLogTag} + "allocation failed: " + e.what());
        reset();
        return false;
    }
}

bool DynamicBuffer::ensure(size_t requiredSize,
                           vk::BufferUsageFlags usage,
                           spectraforge::core::ResourceUsage memoryUsage) {
    if (valid() && size_ >= requiredSize && usage_ == usage && memoryUsage_ == memoryUsage) {
        return true;
    }
    return create(requiredSize, usage, memoryUsage);
}

void DynamicBuffer::reset() {
    buffer_ = spectraforge::core::VMABuffer{};
    size_ = 0;
    usage_ = {};
    memoryUsage_ = spectraforge::core::ResourceUsage::GPU_ONLY;
}

bool DynamicBuffer::valid() const {
    return static_cast<VkBuffer>(buffer_.getBuffer()) != VK_NULL_HANDLE;
}

size_t DynamicBuffer::size() const {
    return size_;
}

vk::Buffer DynamicBuffer::handle() const {
    return buffer_.getBuffer();
}

void* DynamicBuffer::map() {
    if (!valid()) {
        SAFE_ERROR(std::string{kLogTag} + "map() called on invalid buffer");
        return nullptr;
    }
    return buffer_.map();
}

void DynamicBuffer::unmap() {
    if (!valid()) {
        return;
    }
    buffer_.unmap();
}

vk::DescriptorBufferInfo DynamicBuffer::descriptor(vk::DeviceSize offset,
                                                   vk::DeviceSize range) const {
    if (!valid()) {
        throw std::runtime_error(std::string{kLogTag} + "descriptor() on invalid buffer");
    }
    if (offset > size_) {
        throw std::runtime_error(std::string{kLogTag} + "descriptor offset exceeds size");
    }

    vk::DescriptorBufferInfo info{};
    info.buffer = buffer_.getBuffer();
    info.offset = offset;
    if (range == VK_WHOLE_SIZE) {
        info.range = static_cast<vk::DeviceSize>(size_ - offset);
    } else {
        if (offset + range > size_) {
            throw std::runtime_error(std::string{kLogTag} + "descriptor range exceeds size");
        }
        info.range = range;
    }
    return info;
}

void DynamicBuffer::writeDescriptor(vk::Device device,
                                    vk::DescriptorSet set,
                                    uint32_t binding,
                                    vk::DescriptorType type,
                                    vk::DeviceSize offset,
                                    vk::DeviceSize range) const {
    auto info = descriptor(offset, range);

    vk::WriteDescriptorSet write{};
    write.dstSet = set;
    write.dstBinding = binding;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pBufferInfo = &info;

    device.updateDescriptorSets(write, nullptr);
}

} // namespace spectraforge::rendering::utils

