/**
 * @file InstancedMeshPass.cpp
 * @brief Vulkan проход для инстансинга через SSBO (20k+ инстансов)
 */

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "SpectraForge/Rendering/RenderPass/InstancedMeshPass.h"
#include "SpectraForge/Core/VMAMemoryManager.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <vector>

namespace spectraforge {
namespace rendering {

bool InstancedMeshPass::initialize(vk::Device device, VmaAllocator allocator) {
    device_ = device;
    allocator_ = allocator;
    if (!create_descriptors()) return false;
    // начальная ёмкость на 32k инстансов
    return create_instance_buffer(sizeof(InstanceDataGPU) * 32768);
}

bool InstancedMeshPass::create_descriptors() {
    // set=0, binding=1: storage buffer (instances)
    vk::DescriptorSetLayoutBinding instBinding{};
    instBinding.binding = 1;
    instBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
    instBinding.descriptorCount = 1;
    instBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutCreateInfo dslc{};
    dslc.bindingCount = 1;
    dslc.pBindings = &instBinding;
    setLayout_ = device_.createDescriptorSetLayout(dslc);

    vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eStorageBuffer, 1 };
    vk::DescriptorPoolCreateInfo dpci{};
    dpci.maxSets = 1;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = &poolSize;
    descriptorPool_ = device_.createDescriptorPool(dpci);

    vk::DescriptorSetAllocateInfo dsai{};
    dsai.descriptorPool = descriptorPool_;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &setLayout_;
    descriptorSet_ = device_.allocateDescriptorSets(dsai).front();
    return true;
}

bool InstancedMeshPass::create_instance_buffer(size_t capacityBytes) {
    auto& vma = spectraforge::core::VMAMemoryManager::getInstance();
    // Освобождаем старый
    if (instanceBufferHandle_ != nullptr) {
        delete instanceBufferHandle_;
        instanceBufferHandle_ = nullptr;
    }
    // Для простоты обновлений — CPU_TO_GPU mapping (в дальнейшем можно перейти на staging + copy)
    auto buf = vma.createBuffer(
        capacityBytes,
        vk::BufferUsageFlagBits::eStorageBuffer,
        spectraforge::core::ResourceUsage::CPU_TO_GPU);
    instanceBufferHandle_ = new spectraforge::core::VMABuffer(std::move(buf));
    instanceBufferCapacity_ = capacityBytes;

    // Обновим дескриптор
    vk::DescriptorBufferInfo dbi{};
    dbi.buffer = instanceBufferHandle_->getBuffer();
    dbi.offset = 0;
    dbi.range = instanceBufferCapacity_;

    vk::WriteDescriptorSet write{};
    write.dstSet = descriptorSet_;
    write.dstBinding = 1;
    write.descriptorType = vk::DescriptorType::eStorageBuffer;
    write.descriptorCount = 1;
    write.pBufferInfo = &dbi;
    device_.updateDescriptorSets(1, &write, 0, nullptr);
    return true;
}

void InstancedMeshPass::upload_instances(const std::vector<InstanceDataGPU>& cpuInstances) {
    size_t bytes = cpuInstances.size() * sizeof(InstanceDataGPU);
    if (bytes > instanceBufferCapacity_) {
        create_instance_buffer(bytes);
    }
    // Прямая запись в CPU_TO_GPU
    void* ptr = instanceBufferHandle_ ? instanceBufferHandle_->map() : nullptr;
    if (ptr) {
        std::memcpy(ptr, cpuInstances.data(), bytes);
        instanceBufferHandle_->unmap();
    }
}

bool InstancedMeshPass::ensure_capacity(size_t bytes) {
    if (bytes <= instanceBufferCapacity_) return true;
    return create_instance_buffer(bytes);
}

void InstancedMeshPass::barrier_host_writes(vk::CommandBuffer cmd) const {
    vk::BufferMemoryBarrier barrier{};
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = instanceBufferHandle_ ? instanceBufferHandle_->getBuffer() : vk::Buffer{};
    barrier.offset = 0;
    barrier.size = instanceBufferCapacity_;
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eHost,
        vk::PipelineStageFlagBits::eVertexShader,
        {},
        0, nullptr,
        1, &barrier,
        0, nullptr);
}

void InstancedMeshPass::bind_and_push(vk::CommandBuffer cmd,
                                      const CameraPushConstants& pc,
                                      vk::PipelineLayout pipelineLayout,
                                      uint32_t setIndex) const {
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           pipelineLayout,
                           setIndex,
                           1, &descriptorSet_,
                           0, nullptr);
    cmd.pushConstants(pipelineLayout,
                      vk::ShaderStageFlagBits::eVertex,
                      0,
                      sizeof(CameraPushConstants),
                      &pc);
}

vk::Buffer InstancedMeshPass::get_instance_buffer() const {
    return instanceBufferHandle_ ? instanceBufferHandle_->getBuffer() : vk::Buffer{};
}

void InstancedMeshPass::cleanup() {
    if (descriptorPool_) device_.destroyDescriptorPool(descriptorPool_);
    if (setLayout_) device_.destroyDescriptorSetLayout(setLayout_);
    if (instanceBufferHandle_) { delete instanceBufferHandle_; instanceBufferHandle_ = nullptr; }
}

} // namespace rendering
} // namespace spectraforge

