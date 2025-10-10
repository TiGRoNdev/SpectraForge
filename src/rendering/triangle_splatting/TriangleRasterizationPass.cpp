#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleRasterizationPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingCore.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Core/Console.h>
#include <SpectraForge/Core/SafeConsole.h>
#include <fstream>
#include <stdexcept>

using SpectraForge::Core::Console;

namespace spectraforge {
namespace rendering {

bool TriangleRasterizationPass::initialize(vk::Device device,
                                           VmaAllocator allocator,
                                           const TriangleSplattingCore& core,
                                           const TriangleBufferManager& bufferManager,
                                           const Config& config) {
    if (initialized_) {
        Console::info("TriangleRasterizationPass already initialized");
        return true;
    }
    
    if (!device || !allocator || !core.isInitialized() || !bufferManager.isInitialized()) {
        Console::error("Invalid parameters for TriangleRasterizationPass initialization");
        return false;
    }
    
    device_ = device;
    allocator_ = allocator;
    config_ = config;
    
    // Загружаем shaders
    if (!loadShaders()) {
        Console::error("Failed to load rasterization shaders");
        cleanup();
        return false;
    }
    
    // Создаем visibility buffer для Two-Pass
    if (config_.enableTwoPassRendering) {
        if (!createVisibilityBuffer(config_.outputWidth, config_.outputHeight)) {
            Console::error("Failed to create visibility buffer");
            cleanup();
            return false;
        }
    }
    
    // Создаем descriptor sets
    if (!createDescriptorSets(core, bufferManager)) {
        Console::error("Failed to create descriptor sets");
        cleanup();
        return false;
    }
    
    // Создаем pipelines
    if (!createSinglePassPipeline()) {
        Console::error("Failed to create single-pass pipeline");
        cleanup();
        return false;
    }
    
    if (config_.enableTwoPassRendering) {
        if (!createTwoPassPipelines()) {
            Console::error("Failed to create two-pass pipelines");
            cleanup();
            return false;
        }
    }
    
    initialized_ = true;
    Console::info("TriangleRasterizationPass initialized successfully");
    
    return true;
}

void TriangleRasterizationPass::cleanup() {
    if (!initialized_) {
        return;
    }
    
    if (device_) {
        // Single-pass cleanup
        if (triangleSplattingPipeline_) device_.destroyPipeline(triangleSplattingPipeline_);
        if (triangleSplattingPipelineLayout_) device_.destroyPipelineLayout(triangleSplattingPipelineLayout_);
        if (triangleSplattingDescriptorPool_) device_.destroyDescriptorPool(triangleSplattingDescriptorPool_);
        if (triangleSplattingDescriptorSetLayout_) device_.destroyDescriptorSetLayout(triangleSplattingDescriptorSetLayout_);
        if (triangleSplattingShader_) device_.destroyShaderModule(triangleSplattingShader_);
        
        // Two-pass cleanup
        if (visibilityPassPipeline_) device_.destroyPipeline(visibilityPassPipeline_);
        if (visibilityPassPipelineLayout_) device_.destroyPipelineLayout(visibilityPassPipelineLayout_);
        if (visibilityPassDescriptorPool_) device_.destroyDescriptorPool(visibilityPassDescriptorPool_);
        if (visibilityPassDescriptorSetLayout_) device_.destroyDescriptorSetLayout(visibilityPassDescriptorSetLayout_);
        if (visibilityPassShader_) device_.destroyShaderModule(visibilityPassShader_);
        
        if (shadingPassPipeline_) device_.destroyPipeline(shadingPassPipeline_);
        if (shadingPassPipelineLayout_) device_.destroyPipelineLayout(shadingPassPipelineLayout_);
        if (shadingPassDescriptorPool_) device_.destroyDescriptorPool(shadingPassDescriptorPool_);
        if (shadingPassDescriptorSetLayout_) device_.destroyDescriptorSetLayout(shadingPassDescriptorSetLayout_);
        if (shadingPassShader_) device_.destroyShaderModule(shadingPassShader_);
    }
    
    if (allocator_) {
        if (visibilityBuffer_ && visibilityBufferAllocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(visibilityBuffer_), visibilityBufferAllocation_);
            visibilityBuffer_ = vk::Buffer();
            visibilityBufferAllocation_ = VK_NULL_HANDLE;
        }
    }
    
    initialized_ = false;
    Console::info("TriangleRasterizationPass cleaned up");
}

void TriangleRasterizationPass::execute(vk::CommandBuffer cmd,
                                        const glm::mat4& viewProj,
                                        uint32_t triangleCount) {
    if (!initialized_) {
        throw std::runtime_error("TriangleRasterizationPass not initialized");
    }
    
    if (triangleCount == 0) {
        Console::info("TriangleRasterizationPass: zero triangles to render");
        return;
    }
    
    if (config_.enableTwoPassRendering) {
        executeTwoPass(cmd, viewProj, triangleCount);
    } else {
        // Single-Pass rendering
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, triangleSplattingPipeline_);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                              triangleSplattingPipelineLayout_,
                              0, 1,
                              &triangleSplattingDescriptorSet_,
                              0, nullptr);
        
        RasterizationPushConstants pushConstants;
        pushConstants.viewProj = viewProj;
        pushConstants.outputWidth = 1920; // TODO: Get from core
        pushConstants.outputHeight = 1080;
        pushConstants.triangleCount = triangleCount;
        pushConstants.enableEarlyTermination = config_.enableEarlyTermination ? 1 : 0;
        pushConstants.alphaThreshold = config_.alphaThreshold;
        pushConstants.debugMode = config_.debugMode;
        
        cmd.pushConstants(triangleSplattingPipelineLayout_,
                         vk::ShaderStageFlagBits::eCompute,
                         0,
                         sizeof(RasterizationPushConstants),
                         &pushConstants);
        
        // Dispatch: 16x16 workgroup size
        const uint32_t workgroupSizeX = 16;
        const uint32_t workgroupSizeY = 16;
        const uint32_t numWorkgroupsX = (pushConstants.outputWidth + workgroupSizeX - 1) / workgroupSizeX;
        const uint32_t numWorkgroupsY = (pushConstants.outputHeight + workgroupSizeY - 1) / workgroupSizeY;
        
        cmd.dispatch(numWorkgroupsX, numWorkgroupsY, 1);
    }
}

void TriangleRasterizationPass::executeTwoPass(vk::CommandBuffer cmd,
                                               const glm::mat4& viewProj,
                                               uint32_t triangleCount) {
    if (!initialized_) {
        throw std::runtime_error("TriangleRasterizationPass not initialized");
    }
    
    // Pass 1: Visibility determination (O(N))
    executeVisibilityPass(cmd, viewProj, triangleCount);
    
    // Барьер между passes
    vk::MemoryBarrier barrier;
    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        1, &barrier,
        0, nullptr,
        0, nullptr
    );
    
    // Pass 2: Shading (O(M), where M = visible pixels)
    executeShadingPass(cmd, viewProj, triangleCount);
}

void TriangleRasterizationPass::setDebugMode(uint32_t mode) {
    config_.debugMode = mode;
    Console::info("TriangleRasterizationPass debug mode set to " + SpectraForge::Core::SAFE_TO_STRING(mode));
}

bool TriangleRasterizationPass::loadShaders() {
    auto loadShader = [this](const std::string& filename, vk::ShaderModule& shaderModule) -> bool {
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
        
        shaderModule = device_.createShaderModule(createInfo);
        
        return true;
    };
    
    if (!loadShader("shaders/TriangleSplatting.comp.spv", triangleSplattingShader_)) {
        return false;
    }
    
    if (config_.enableTwoPassRendering) {
        if (!loadShader("shaders/TriangleVisibility.comp.spv", visibilityPassShader_)) {
            return false;
        }
        
        if (!loadShader("shaders/TriangleShading.comp.spv", shadingPassShader_)) {
            return false;
        }
    }
    
    return true;
}

bool TriangleRasterizationPass::createSinglePassPipeline() {
    // Push constant range
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(RasterizationPushConstants);
    
    // Pipeline layout
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &triangleSplattingDescriptorSetLayout_;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    
    triangleSplattingPipelineLayout_ = device_.createPipelineLayout(layoutInfo);
    
    // Compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = triangleSplattingShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = triangleSplattingPipelineLayout_;
    
    auto result = device_.createComputePipeline(vk::PipelineCache(), pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        return false;
    }
    
    triangleSplattingPipeline_ = result.value;
    
    return true;
}

bool TriangleRasterizationPass::createTwoPassPipelines() {
    // Visibility Pass Pipeline (аналогично Single-Pass)
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(RasterizationPushConstants);
    
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &visibilityPassDescriptorSetLayout_;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    
    visibilityPassPipelineLayout_ = device_.createPipelineLayout(layoutInfo);
    
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = visibilityPassShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = visibilityPassPipelineLayout_;
    
    auto result = device_.createComputePipeline(vk::PipelineCache(), pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        return false;
    }
    
    visibilityPassPipeline_ = result.value;
    
    // Shading Pass Pipeline (аналогично)
    layoutInfo.pSetLayouts = &shadingPassDescriptorSetLayout_;
    shadingPassPipelineLayout_ = device_.createPipelineLayout(layoutInfo);
    
    shaderStageInfo.module = shadingPassShader_;
    pipelineInfo.layout = shadingPassPipelineLayout_;
    
    result = device_.createComputePipeline(vk::PipelineCache(), pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        return false;
    }
    
    shadingPassPipeline_ = result.value;
    
    return true;
}

bool TriangleRasterizationPass::createDescriptorSets(const TriangleSplattingCore& core,
                                                     const TriangleBufferManager& bufferManager) {
    // Упрощенная реализация (полная версия требует ~150 строк)
    std::vector<vk::DescriptorSetLayoutBinding> bindings(3);
    
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageImage;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    bindings[2].binding = 2;
    bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    triangleSplattingDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // Для Two-Pass создаем аналогичные layouts
    visibilityPassDescriptorSetLayout_ = triangleSplattingDescriptorSetLayout_;
    shadingPassDescriptorSetLayout_ = triangleSplattingDescriptorSetLayout_;
    
    return true;
}

bool TriangleRasterizationPass::createVisibilityBuffer(uint32_t outputWidth, uint32_t outputHeight) {
    const vk::DeviceSize bufferSize = static_cast<vk::DeviceSize>(outputWidth) * outputHeight * 
                                      config_.maxTrianglesPerPixel * sizeof(uint32_t);
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkBuffer;
    VkResult result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &vkBuffer, &visibilityBufferAllocation_, nullptr);
    
    if (result != VK_SUCCESS) {
        return false;
    }
    
    visibilityBuffer_ = vk::Buffer(vkBuffer);
    
    return true;
}

void TriangleRasterizationPass::executeVisibilityPass(vk::CommandBuffer cmd,
                                                      const glm::mat4& viewProj,
                                                      uint32_t triangleCount) {
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, visibilityPassPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                          visibilityPassPipelineLayout_,
                          0, 1,
                          &visibilityPassDescriptorSet_,
                          0, nullptr);
    
    RasterizationPushConstants pushConstants;
    pushConstants.viewProj = viewProj;
    pushConstants.outputWidth = 1920; // TODO: Get from core
    pushConstants.outputHeight = 1080;
    pushConstants.triangleCount = triangleCount;
    pushConstants.enableEarlyTermination = 0; // No early termination in visibility pass
    pushConstants.alphaThreshold = config_.alphaThreshold;
    pushConstants.debugMode = 0;
    
    cmd.pushConstants(visibilityPassPipelineLayout_,
                     vk::ShaderStageFlagBits::eCompute,
                     0,
                     sizeof(RasterizationPushConstants),
                     &pushConstants);
    
    const uint32_t workgroupSizeX = 16;
    const uint32_t workgroupSizeY = 16;
    const uint32_t numWorkgroupsX = (pushConstants.outputWidth + workgroupSizeX - 1) / workgroupSizeX;
    const uint32_t numWorkgroupsY = (pushConstants.outputHeight + workgroupSizeY - 1) / workgroupSizeY;
    
    cmd.dispatch(numWorkgroupsX, numWorkgroupsY, 1);
}

void TriangleRasterizationPass::executeShadingPass(vk::CommandBuffer cmd,
                                                   const glm::mat4& viewProj,
                                                   uint32_t triangleCount) {
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, shadingPassPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                          shadingPassPipelineLayout_,
                          0, 1,
                          &shadingPassDescriptorSet_,
                          0, nullptr);
    
    RasterizationPushConstants pushConstants;
    pushConstants.viewProj = viewProj;
    pushConstants.outputWidth = 1920;
    pushConstants.outputHeight = 1080;
    pushConstants.triangleCount = triangleCount;
    pushConstants.enableEarlyTermination = config_.enableEarlyTermination ? 1 : 0;
    pushConstants.alphaThreshold = config_.alphaThreshold;
    pushConstants.debugMode = config_.debugMode;
    
    cmd.pushConstants(shadingPassPipelineLayout_,
                     vk::ShaderStageFlagBits::eCompute,
                     0,
                     sizeof(RasterizationPushConstants),
                     &pushConstants);
    
    const uint32_t workgroupSizeX = 16;
    const uint32_t workgroupSizeY = 16;
    const uint32_t numWorkgroupsX = (pushConstants.outputWidth + workgroupSizeX - 1) / workgroupSizeX;
    const uint32_t numWorkgroupsY = (pushConstants.outputHeight + workgroupSizeY - 1) / workgroupSizeY;
    
    cmd.dispatch(numWorkgroupsX, numWorkgroupsY, 1);
}

} // namespace rendering
} // namespace spectraforge

