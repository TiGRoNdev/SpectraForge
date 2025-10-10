/**
 * @file InstancedMeshPass.cpp
 * @brief Vulkan проход для инстансинга через SSBO (20k+ инстансов)
 */

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "SpectraForge/Rendering/RenderPass/InstancedMeshPass.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <vector>

#include <cstring>

namespace spectraforge {
namespace rendering {

bool InstancedMeshPass::initialize(vk::Device device, VmaAllocator allocator) {
    device_ = device;
    (void)allocator; // VMAMemoryManager manages allocations globally
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
    if (!instanceBuffer_.create(capacityBytes,
                                vk::BufferUsageFlagBits::eStorageBuffer,
                                spectraforge::core::ResourceUsage::CPU_TO_GPU)) {
        SAFE_ERROR("[InstancedMeshPass] Failed to create instance buffer");
        return false;
    }

    try {
        instanceBuffer_.writeDescriptor(device_,
                                        descriptorSet_,
                                        1,
                                        vk::DescriptorType::eStorageBuffer,
                                        0,
                                        VK_WHOLE_SIZE);
    } catch (const std::exception& e) {
        SAFE_ERROR("[InstancedMeshPass] Descriptor update failed: " + SpectraForge::Core::SAFE_TO_STRING(e.what()));
        instanceBuffer_.reset();
        return false;
    }

    instanceBufferCapacity_ = instanceBuffer_.size();
    return true;
}

void InstancedMeshPass::upload_instances(const std::vector<InstanceDataGPU>& cpuInstances) {
    size_t bytes = cpuInstances.size() * sizeof(InstanceDataGPU);
    if (bytes > instanceBufferCapacity_) {
        create_instance_buffer(bytes);
    }
    // Прямая запись в CPU_TO_GPU
    if (!instanceBuffer_.valid()) {
        SAFE_ERROR("[InstancedMeshPass] Instance buffer is not initialized");
        return;
    }
    void* ptr = instanceBuffer_.map();
    if (ptr) {
        std::memcpy(ptr, cpuInstances.data(), bytes);
        instanceBuffer_.unmap();
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
    barrier.buffer = instanceBuffer_.valid() ? instanceBuffer_.handle() : vk::Buffer{};
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
    return instanceBuffer_.valid() ? instanceBuffer_.handle() : vk::Buffer{};
}

void InstancedMeshPass::cleanup() {
    if (descriptorPool_) device_.destroyDescriptorPool(descriptorPool_);
    if (setLayout_) device_.destroyDescriptorSetLayout(setLayout_);
    instanceBuffer_.reset();
    instanceBufferCapacity_ = 0;
}

} // namespace rendering
} // namespace spectraforge

