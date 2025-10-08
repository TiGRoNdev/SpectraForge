#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/FrustumCullingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Core/Console.h>
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace spectraforge {
namespace rendering {

bool FrustumCullingPass::initialize(vk::Device device,
                                    VmaAllocator allocator,
                                    const TriangleBufferManager& bufferManager,
                                    vk::CommandPool commandPool,
                                    vk::Queue queue) {
    if (initialized_) {
        Console::warn("FrustumCullingPass already initialized");
        return true;
    }
    
    if (!device || !allocator || !bufferManager.isInitialized()) {
        Console::error("Invalid parameters for FrustumCullingPass initialization");
        return false;
    }
    
    device_ = device;
    allocator_ = allocator;
    commandPool_ = commandPool;
    queue_ = queue;
    
    // Создаем atomic counter buffer
    if (!createAtomicCounterBuffer()) {
        Console::error("Failed to create atomic counter buffer");
        return false;
    }
    
    // Загружаем shader
    if (!loadShader("shaders/FrustumCulling.comp.spv")) {
        Console::error("Failed to load frustum culling shader");
        cleanup();
        return false;
    }
    
    // Создаем descriptor sets
    if (!createDescriptorSets(bufferManager)) {
        Console::error("Failed to create descriptor sets");
        cleanup();
        return false;
    }
    
    // Создаем pipeline
    if (!createPipeline()) {
        Console::error("Failed to create frustum culling pipeline");
        cleanup();
        return false;
    }
    
    initialized_ = true;
    Console::info("FrustumCullingPass initialized successfully");
    
    return true;
}

void FrustumCullingPass::cleanup() {
    if (!initialized_) {
        return;
    }
    
    if (device_) {
        if (frustumCullingPipeline_) {
            device_.destroyPipeline(frustumCullingPipeline_);
            frustumCullingPipeline_ = vk::Pipeline();
        }
        
        if (frustumCullingPipelineLayout_) {
            device_.destroyPipelineLayout(frustumCullingPipelineLayout_);
            frustumCullingPipelineLayout_ = vk::PipelineLayout();
        }
        
        if (frustumCullingDescriptorPool_) {
            device_.destroyDescriptorPool(frustumCullingDescriptorPool_);
            frustumCullingDescriptorPool_ = vk::DescriptorPool();
        }
        
        if (frustumCullingDescriptorSetLayout_) {
            device_.destroyDescriptorSetLayout(frustumCullingDescriptorSetLayout_);
            frustumCullingDescriptorSetLayout_ = vk::DescriptorSetLayout();
        }
        
        if (frustumCullingShader_) {
            device_.destroyShaderModule(frustumCullingShader_);
            frustumCullingShader_ = vk::ShaderModule();
        }
    }
    
    if (allocator_) {
        if (atomicCounterBuffer_ && atomicCounterAllocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(atomicCounterBuffer_), atomicCounterAllocation_);
            atomicCounterBuffer_ = vk::Buffer();
            atomicCounterAllocation_ = VK_NULL_HANDLE;
        }
        
        if (readbackBuffer_ && readbackAllocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(readbackBuffer_), readbackAllocation_);
            readbackBuffer_ = vk::Buffer();
            readbackAllocation_ = VK_NULL_HANDLE;
        }
    }
    
    initialized_ = false;
    Console::info("FrustumCullingPass cleaned up");
}

void FrustumCullingPass::execute(vk::CommandBuffer cmd,
                                 const glm::mat4& viewProj,
                                 uint32_t triangleCount) {
    if (!initialized_) {
        throw std::runtime_error("FrustumCullingPass not initialized");
    }
    
    if (triangleCount == 0) {
        Console::warn("FrustumCullingPass: zero triangles to cull");
        return;
    }
    
    // Сброс atomic counter
    resetAtomicCounter(cmd);
    
    // Bind pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, frustumCullingPipeline_);
    
    // Bind descriptor set
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                          frustumCullingPipelineLayout_,
                          0, 1,
                          &frustumCullingDescriptorSet_,
                          0, nullptr);
    
    // Push constants
    PushConstants pushConstants;
    pushConstants.viewProj = viewProj;
    pushConstants.triangleCount = triangleCount;
    
    cmd.pushConstants(frustumCullingPipelineLayout_,
                     vk::ShaderStageFlagBits::eCompute,
                     0,
                     sizeof(PushConstants),
                     &pushConstants);
    
    // Dispatch compute shader (256 threads per workgroup)
    const uint32_t workgroupSize = 256;
    const uint32_t numWorkgroups = (triangleCount + workgroupSize - 1) / workgroupSize;
    
    cmd.dispatch(numWorkgroups, 1, 1);
    
    // Барьер для синхронизации
    vk::BufferMemoryBarrier barrier;
    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = atomicCounterBuffer_;
    barrier.offset = 0;
    barrier.size = sizeof(uint32_t);
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        0, nullptr,
        1, &barrier,
        0, nullptr
    );
}

uint32_t FrustumCullingPass::getVisibleCount() const {
    if (!initialized_) {
        return 0;
    }
    
    // Копируем из GPU buffer в readback buffer
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool_;
    allocInfo.commandBufferCount = 1;
    
    vk::CommandBuffer cmd = device_.allocateCommandBuffers(allocInfo)[0];
    
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    cmd.begin(beginInfo);
    
    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = sizeof(uint32_t);
    
    cmd.copyBuffer(atomicCounterBuffer_, readbackBuffer_, 1, &copyRegion);
    
    cmd.end();
    
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    queue_.submit(1, &submitInfo, vk::Fence());
    queue_.waitIdle();
    
    device_.freeCommandBuffers(commandPool_, 1, &cmd);
    
    // Читаем из readback buffer
    void* data;
    vmaMapMemory(allocator_, readbackAllocation_, &data);
    uint32_t visibleCount = *static_cast<uint32_t*>(data);
    vmaUnmapMemory(allocator_, readbackAllocation_);
    
    return visibleCount;
}

bool FrustumCullingPass::loadShader(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        Console::error("Failed to open shader file: " + filename);
        return false;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    
    frustumCullingShader_ = device_.createShaderModule(createInfo);
    
    return true;
}

bool FrustumCullingPass::createPipeline() {
    // Push constant range
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);
    
    // Pipeline layout
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &frustumCullingDescriptorSetLayout_;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    
    frustumCullingPipelineLayout_ = device_.createPipelineLayout(layoutInfo);
    
    // Compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = frustumCullingShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = frustumCullingPipelineLayout_;
    
    auto result = device_.createComputePipeline(vk::PipelineCache(), pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        Console::error("Failed to create compute pipeline");
        return false;
    }
    
    frustumCullingPipeline_ = result.value;
    
    return true;
}

bool FrustumCullingPass::createDescriptorSets(const TriangleBufferManager& bufferManager) {
    // Descriptor set layout
    std::vector<vk::DescriptorSetLayoutBinding> bindings(3);
    
    // Binding 0: Triangle buffer (readonly)
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Visible indices buffer (writeonly)
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Atomic counter buffer (read-write)
    bindings[2].binding = 2;
    bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    frustumCullingDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // Descriptor pool
    std::vector<vk::DescriptorPoolSize> poolSizes(1);
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 3; // 3 storage buffers
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    
    frustumCullingDescriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = frustumCullingDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &frustumCullingDescriptorSetLayout_;
    
    frustumCullingDescriptorSet_ = device_.allocateDescriptorSets(allocInfo)[0];
    
    // Update descriptor set
    std::vector<vk::WriteDescriptorSet> descriptorWrites(3);
    
    vk::DescriptorBufferInfo triangleBufferInfo;
    triangleBufferInfo.buffer = bufferManager.getTriangleBuffer();
    triangleBufferInfo.offset = 0;
    triangleBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[0].dstSet = frustumCullingDescriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &triangleBufferInfo;
    
    vk::DescriptorBufferInfo visibleIndicesBufferInfo;
    visibleIndicesBufferInfo.buffer = bufferManager.getVisibleIndicesBuffer();
    visibleIndicesBufferInfo.offset = 0;
    visibleIndicesBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[1].dstSet = frustumCullingDescriptorSet_;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &visibleIndicesBufferInfo;
    
    vk::DescriptorBufferInfo atomicCounterBufferInfo;
    atomicCounterBufferInfo.buffer = atomicCounterBuffer_;
    atomicCounterBufferInfo.offset = 0;
    atomicCounterBufferInfo.range = sizeof(uint32_t);
    
    descriptorWrites[2].dstSet = frustumCullingDescriptorSet_;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &atomicCounterBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), 
                                descriptorWrites.data(), 
                                0, nullptr);
    
    return true;
}

bool FrustumCullingPass::createAtomicCounterBuffer() {
    // Atomic counter buffer (GPU-side)
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uint32_t);
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkBuffer;
    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &vkBuffer, &atomicCounterAllocation_, nullptr);
    
    if (result != VK_SUCCESS) {
        return false;
    }
    
    atomicCounterBuffer_ = vk::Buffer(vkBuffer);
    
    // Readback buffer (CPU-side)
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    
    result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &vkBuffer, &readbackAllocation_, nullptr);
    
    if (result != VK_SUCCESS) {
        return false;
    }
    
    readbackBuffer_ = vk::Buffer(vkBuffer);
    
    return true;
}

void FrustumCullingPass::resetAtomicCounter(vk::CommandBuffer cmd) {
    cmd.fillBuffer(atomicCounterBuffer_, 0, sizeof(uint32_t), 0);
    
    vk::BufferMemoryBarrier barrier;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = atomicCounterBuffer_;
    barrier.offset = 0;
    barrier.size = sizeof(uint32_t);
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        0, nullptr,
        1, &barrier,
        0, nullptr
    );
}

} // namespace rendering
} // namespace spectraforge

