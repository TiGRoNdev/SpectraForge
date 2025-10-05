#include <SpectraForge/rendering/TriangleSplattingPass.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cinttypes>

namespace spectraforge {
namespace rendering {

TriangleSplattingPass::TriangleSplattingPass(const Config& config)
    : config_(config)
{
    std::cout << "[TriangleSplattingPass] Created with resolution " 
              << config_.outputWidth << "x" << config_.outputHeight << "\n";
}

TriangleSplattingPass::~TriangleSplattingPass() {
    cleanup();
}

bool TriangleSplattingPass::initialize(vk::Device device, 
                                       VmaAllocator allocator,
                                       vk::Queue computeQueue,
                                       vk::Queue graphicsQueue,
                                       vk::CommandPool commandPool) {
    if (initialized_) {
        std::cerr << "[TriangleSplattingPass] Already initialized\n";
        return true;
    }
    
    device_ = device;
    allocator_ = allocator;
    computeQueue_ = computeQueue;
    graphicsQueue_ = graphicsQueue;
    commandPool_ = commandPool;
    
    std::cout << "[TriangleSplattingPass] Initializing...\n";
    
    // Create resources in order
    if (!createOutputImage()) {
        std::cerr << "[TriangleSplattingPass] Failed to create output image\n";
        return false;
    }
    
    if (!createBuffers()) {
        std::cerr << "[TriangleSplattingPass] Failed to create buffers\n";
        return false;
    }
    
    // Создаём тайловый буфер ДО дескрипторов, чтобы избежать VK_NULL_HANDLE
    if (!createTileCullingResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Tile Culling resources\n";
        // Не критично: продолжим без тайлового биннинга
    }

    if (!createShaderModule()) {
        std::cerr << "[TriangleSplattingPass] Failed to create shader module\n";
        return false;
    }
    
    if (!createDescriptorSets()) {
        std::cerr << "[TriangleSplattingPass] Failed to create descriptor sets\n";
        return false;
    }
    
    if (!createPipeline()) {
        std::cerr << "[TriangleSplattingPass] Failed to create pipeline\n";
        return false;
    }
    
    // Create Bitonic Sort resources for depth sorting
    if (!createBitonicSortResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Bitonic Sort resources\n";
        return false;
    }
    
    // Create Depth Key Compute resources
    if (!createDepthKeyComputeResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Depth Key Compute resources\n";
        return false;
    }
    
    // Create Frustum Culling resources
    if (!createFrustumCullingResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Frustum Culling resources\n";
        return false;
    }
    
    // Create Tile Culling pipeline resources
    if (!createTileCullingPipelineResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Tile Culling pipeline resources\n";
        // Продолжим без тайлового биннинга
    }

    // Create Indirect Args Compute resources
    if (!createIndirectArgsResources()) {
        std::cerr << "[TriangleSplattingPass] Failed to create Indirect Args Compute resources\n";
        return false;
    }
    
    // Create Two-Pass Rendering resources (Priority #1 Optimization)
    if (config_.enableTwoPassRendering) {
        if (!createTwoPassResources()) {
            std::cerr << "[TriangleSplattingPass] Failed to create Two-Pass Rendering resources\n";
            std::cerr << "[TriangleSplattingPass] Falling back to Single-Pass mode\n";
            config_.enableTwoPassRendering = false;
        } else {
            std::cout << "[TriangleSplattingPass] ✅ Two-Pass Rendering enabled (20-50× expected speedup)\n";
        }
    }
    
    // Initialize push constants
    pushConstants_.viewProj = glm::mat4(1.0f);
    pushConstants_.outputWidth = config_.outputWidth;
    pushConstants_.outputHeight = config_.outputHeight;
    pushConstants_.triangleCount = 0;
    pushConstants_.enableEarlyTerm = config_.enableEarlyTermination ? 1 : 0;
    pushConstants_.alphaThreshold = config_.alphaThreshold;
    // Включаем тайловый биннинг, если буфер создан
    pushConstants_.enableTileBinning = (tileCullingBuffer_ != VK_NULL_HANDLE) ? 1u : 0u;
    
    initialized_ = true;
    std::cout << "[TriangleSplattingPass] Initialized successfully\n";
    
    return true;
}

void TriangleSplattingPass::cleanup() {
    if (!initialized_) return;
    
    std::cout << "[TriangleSplattingPass] Cleaning up...\n";
    
    // Destroy pipeline
    if (pipeline_) {
        device_.destroyPipeline(pipeline_);
        pipeline_ = nullptr;
    }
    
    if (pipelineLayout_) {
        device_.destroyPipelineLayout(pipelineLayout_);
        pipelineLayout_ = nullptr;
    }
    
    // Destroy descriptor sets
    if (descriptorPool_) {
        device_.destroyDescriptorPool(descriptorPool_);
        descriptorPool_ = nullptr;
    }
    
    if (descriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(descriptorSetLayout_);
        descriptorSetLayout_ = nullptr;
    }
    
    // Destroy shader
    if (computeShader_) {
        device_.destroyShaderModule(computeShader_);
        computeShader_ = nullptr;
    }
    
    // Destroy output image
    if (outputImageView_) {
        device_.destroyImageView(outputImageView_);
        outputImageView_ = nullptr;
    }
    
    if (outputImage_) {
        vmaDestroyImage(allocator_, outputImage_, outputImageAllocation_);
        outputImage_ = nullptr;
        outputImageAllocation_ = nullptr;
    }
    
    // Destroy buffers
    if (triangleBuffer_) {
        vmaDestroyBuffer(allocator_, triangleBuffer_, triangleBufferAllocation_);
        triangleBuffer_ = nullptr;
        triangleBufferAllocation_ = nullptr;
    }
    
    if (sortedIndicesBuffer_) {
        vmaDestroyBuffer(allocator_, sortedIndicesBuffer_, sortedIndicesAllocation_);
        sortedIndicesBuffer_ = nullptr;
        sortedIndicesAllocation_ = nullptr;
    }
    
    if (depthKeysBuffer_) {
        vmaDestroyBuffer(allocator_, depthKeysBuffer_, depthKeysAllocation_);
        depthKeysBuffer_ = nullptr;
        depthKeysAllocation_ = nullptr;
    }

    // Destroy tile culling buffer
    if (tileCullingBuffer_) {
        vmaDestroyBuffer(allocator_, tileCullingBuffer_, tileCullingAllocation_);
        tileCullingBuffer_ = nullptr;
        tileCullingAllocation_ = nullptr;
    }

    // Destroy Bitonic Sort resources
    if (bitonicSortPipeline_) {
        device_.destroyPipeline(bitonicSortPipeline_);
        bitonicSortPipeline_ = nullptr;
    }
    
    if (bitonicSortPipelineLayout_) {
        device_.destroyPipelineLayout(bitonicSortPipelineLayout_);
        bitonicSortPipelineLayout_ = nullptr;
    }
    
    if (bitonicSortDescriptorPool_) {
        device_.destroyDescriptorPool(bitonicSortDescriptorPool_);
        bitonicSortDescriptorPool_ = nullptr;
    }
    
    if (bitonicSortDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(bitonicSortDescriptorSetLayout_);
        bitonicSortDescriptorSetLayout_ = nullptr;
    }
    
    if (bitonicSortShader_) {
        device_.destroyShaderModule(bitonicSortShader_);
        bitonicSortShader_ = nullptr;
    }
    
    // Destroy Depth Key Compute resources
    if (depthKeyComputePipeline_) {
        device_.destroyPipeline(depthKeyComputePipeline_);
        depthKeyComputePipeline_ = nullptr;
    }
    
    if (depthKeyComputePipelineLayout_) {
        device_.destroyPipelineLayout(depthKeyComputePipelineLayout_);
        depthKeyComputePipelineLayout_ = nullptr;
    }
    
    if (depthKeyComputeDescriptorPool_) {
        device_.destroyDescriptorPool(depthKeyComputeDescriptorPool_);
        depthKeyComputeDescriptorPool_ = nullptr;
    }
    
    if (depthKeyComputeDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(depthKeyComputeDescriptorSetLayout_);
        depthKeyComputeDescriptorSetLayout_ = nullptr;
    }
    
    if (depthKeyComputeShader_) {
        device_.destroyShaderModule(depthKeyComputeShader_);
        depthKeyComputeShader_ = nullptr;
    }
    
    // Destroy Frustum Culling resources
    if (frustumCullingPipeline_) {
        device_.destroyPipeline(frustumCullingPipeline_);
        frustumCullingPipeline_ = nullptr;
    }
    
    if (frustumCullingPipelineLayout_) {
        device_.destroyPipelineLayout(frustumCullingPipelineLayout_);
        frustumCullingPipelineLayout_ = nullptr;
    }
    
    if (frustumCullingDescriptorPool_) {
        device_.destroyDescriptorPool(frustumCullingDescriptorPool_);
        frustumCullingDescriptorPool_ = nullptr;
    }
    
    if (frustumCullingDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(frustumCullingDescriptorSetLayout_);
        frustumCullingDescriptorSetLayout_ = nullptr;
    }
    
    if (frustumCullingShader_) {
        device_.destroyShaderModule(frustumCullingShader_);
        frustumCullingShader_ = nullptr;
    }
    
    // Destroy frustum culling buffers
    if (visibleIndicesBuffer_) {
        vmaDestroyBuffer(allocator_, visibleIndicesBuffer_, visibleIndicesAllocation_);
        visibleIndicesBuffer_ = nullptr;
        visibleIndicesAllocation_ = nullptr;
    }
    
    if (atomicCounterBuffer_) {
        vmaDestroyBuffer(allocator_, atomicCounterBuffer_, atomicCounterAllocation_);
        atomicCounterBuffer_ = nullptr;
        atomicCounterAllocation_ = nullptr;
    }
    
    if (indirectDispatchBuffer_) {
        vmaDestroyBuffer(allocator_, indirectDispatchBuffer_, indirectDispatchAllocation_);
        indirectDispatchBuffer_ = nullptr;
        indirectDispatchAllocation_ = nullptr;
    }
    
    // Destroy Indirect Args resources
    if (indirectArgsPipeline_) {
        device_.destroyPipeline(indirectArgsPipeline_);
        indirectArgsPipeline_ = nullptr;
    }
    
    if (indirectArgsPipelineLayout_) {
        device_.destroyPipelineLayout(indirectArgsPipelineLayout_);
        indirectArgsPipelineLayout_ = nullptr;
    }
    
    if (indirectArgsDescriptorPool_) {
        device_.destroyDescriptorPool(indirectArgsDescriptorPool_);
        indirectArgsDescriptorPool_ = nullptr;
    }
    
    if (indirectArgsDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(indirectArgsDescriptorSetLayout_);
        indirectArgsDescriptorSetLayout_ = nullptr;
    }
    
    if (indirectArgsShader_) {
        device_.destroyShaderModule(indirectArgsShader_);
        indirectArgsShader_ = nullptr;
    }
    
    // Destroy Two-Pass Rendering resources
    if (visibilityPassPipeline_) {
        device_.destroyPipeline(visibilityPassPipeline_);
        visibilityPassPipeline_ = nullptr;
    }
    
    if (visibilityPassPipelineLayout_) {
        device_.destroyPipelineLayout(visibilityPassPipelineLayout_);
        visibilityPassPipelineLayout_ = nullptr;
    }
    
    if (visibilityPassDescriptorPool_) {
        device_.destroyDescriptorPool(visibilityPassDescriptorPool_);
        visibilityPassDescriptorPool_ = nullptr;
    }
    
    if (visibilityPassDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(visibilityPassDescriptorSetLayout_);
        visibilityPassDescriptorSetLayout_ = nullptr;
    }
    
    if (visibilityPassShader_) {
        device_.destroyShaderModule(visibilityPassShader_);
        visibilityPassShader_ = nullptr;
    }
    
    if (shadingPassPipeline_) {
        device_.destroyPipeline(shadingPassPipeline_);
        shadingPassPipeline_ = nullptr;
    }
    
    if (shadingPassPipelineLayout_) {
        device_.destroyPipelineLayout(shadingPassPipelineLayout_);
        shadingPassPipelineLayout_ = nullptr;
    }
    
    if (shadingPassDescriptorPool_) {
        device_.destroyDescriptorPool(shadingPassDescriptorPool_);
        shadingPassDescriptorPool_ = nullptr;
    }
    
    if (shadingPassDescriptorSetLayout_) {
        device_.destroyDescriptorSetLayout(shadingPassDescriptorSetLayout_);
        shadingPassDescriptorSetLayout_ = nullptr;
    }
    
    if (shadingPassShader_) {
        device_.destroyShaderModule(shadingPassShader_);
        shadingPassShader_ = nullptr;
    }
    
    if (visibilityBuffer_) {
        vmaDestroyBuffer(allocator_, visibilityBuffer_, visibilityBufferAllocation_);
        visibilityBuffer_ = nullptr;
        visibilityBufferAllocation_ = nullptr;
    }
    
    if (pixelCountersBuffer_) {
        vmaDestroyBuffer(allocator_, pixelCountersBuffer_, pixelCountersAllocation_);
        pixelCountersBuffer_ = nullptr;
        pixelCountersAllocation_ = nullptr;
    }
    
    initialized_ = false;
    std::cout << "[TriangleSplattingPass] Cleaned up\n";
}

bool TriangleSplattingPass::createOutputImage() {
    // Create output image (RGBA16F) — соответствует ожиданиям compute шейдера
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = vk::Format::eR16G16B16A16Sfloat;
    imageInfo.extent = vk::Extent3D(config_.outputWidth, config_.outputHeight, 1);
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eStorage | 
                      vk::ImageUsageFlagBits::eTransferSrc | 
                      vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkImage vkImage;
    VkImageCreateInfo vkImageInfo = static_cast<VkImageCreateInfo>(imageInfo);
    
    VkResult result = vmaCreateImage(
        allocator_,
        &vkImageInfo,
        &allocInfo,
        &vkImage,
        &outputImageAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create output image\n";
        return false;
    }
    
    outputImage_ = vk::Image(vkImage);
    
    // Create image view
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = outputImage_;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eR16G16B16A16Sfloat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    outputImageView_ = device_.createImageView(viewInfo);
    
    // NOTE: Image остаётся в UNDEFINED layout здесь
    // Transition в GENERAL будет выполнен в execute() перед первым compute dispatch
    
    std::cout << "[TriangleSplattingPass] Created output image " 
              << config_.outputWidth << "x" << config_.outputHeight << "\n";
    
    return true;
}

bool TriangleSplattingPass::createBuffers() {
    // Create triangle buffer (SSBO)
    vk::BufferCreateInfo triangleBufferInfo;
    triangleBufferInfo.size = sizeof(Triangle) * maxTriangles_;
    triangleBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                               vk::BufferUsageFlagBits::eTransferDst;
    triangleBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo triangleAllocInfo{};
    triangleAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkTriangleBuffer;
    VkBufferCreateInfo vkTriangleBufferInfo = static_cast<VkBufferCreateInfo>(triangleBufferInfo);
    
    VkResult result = vmaCreateBuffer(
        allocator_,
        &vkTriangleBufferInfo,
        &triangleAllocInfo,
        &vkTriangleBuffer,
        &triangleBufferAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create triangle buffer\n";
        return false;
    }
    
    triangleBuffer_ = vk::Buffer(vkTriangleBuffer);
    
    // Create sorted indices buffer (SSBO)
    vk::BufferCreateInfo indicesBufferInfo;
    indicesBufferInfo.size = sizeof(uint32_t) * maxTriangles_;
    indicesBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                              vk::BufferUsageFlagBits::eTransferDst;
    indicesBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo indicesAllocInfo{};
    indicesAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkIndicesBuffer;
    VkBufferCreateInfo vkIndicesBufferInfo = static_cast<VkBufferCreateInfo>(indicesBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkIndicesBufferInfo,
        &indicesAllocInfo,
        &vkIndicesBuffer,
        &sortedIndicesAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create sorted indices buffer\n";
        return false;
    }
    
    sortedIndicesBuffer_ = vk::Buffer(vkIndicesBuffer);
    
    // Create depth keys buffer (for bitonic sort)
    vk::BufferCreateInfo depthKeysBufferInfo;
    depthKeysBufferInfo.size = sizeof(float) * maxTriangles_;
    depthKeysBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                                vk::BufferUsageFlagBits::eTransferDst;
    depthKeysBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo depthKeysAllocInfo{};
    depthKeysAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkDepthKeysBuffer;
    VkBufferCreateInfo vkDepthKeysBufferInfo = static_cast<VkBufferCreateInfo>(depthKeysBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkDepthKeysBufferInfo,
        &depthKeysAllocInfo,
        &vkDepthKeysBuffer,
        &depthKeysAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create depth keys buffer\n";
        return false;
    }
    
    depthKeysBuffer_ = vk::Buffer(vkDepthKeysBuffer);
    
    // Create visible indices buffer (for frustum culling)
    vk::BufferCreateInfo visibleIndicesBufferInfo;
    visibleIndicesBufferInfo.size = sizeof(uint32_t) * maxTriangles_;
    visibleIndicesBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                     vk::BufferUsageFlagBits::eTransferDst;
    visibleIndicesBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo visibleIndicesAllocInfo{};
    visibleIndicesAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkVisibleIndicesBuffer;
    VkBufferCreateInfo vkVisibleIndicesBufferInfo = static_cast<VkBufferCreateInfo>(visibleIndicesBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkVisibleIndicesBufferInfo,
        &visibleIndicesAllocInfo,
        &vkVisibleIndicesBuffer,
        &visibleIndicesAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create visible indices buffer\n";
        return false;
    }
    
    visibleIndicesBuffer_ = vk::Buffer(vkVisibleIndicesBuffer);
    
    // Create atomic counter buffer (for frustum culling visible count)
    vk::BufferCreateInfo atomicCounterBufferInfo;
    atomicCounterBufferInfo.size = sizeof(uint32_t);
    atomicCounterBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                                    vk::BufferUsageFlagBits::eTransferSrc | 
                                    vk::BufferUsageFlagBits::eTransferDst;
    atomicCounterBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo atomicCounterAllocInfo{};
    atomicCounterAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkAtomicCounterBuffer;
    VkBufferCreateInfo vkAtomicCounterBufferInfo = static_cast<VkBufferCreateInfo>(atomicCounterBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkAtomicCounterBufferInfo,
        &atomicCounterAllocInfo,
        &vkAtomicCounterBuffer,
        &atomicCounterAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create atomic counter buffer\n";
        return false;
    }
    
    atomicCounterBuffer_ = vk::Buffer(vkAtomicCounterBuffer);
    
    // Create indirect dispatch buffer (for vkCmdDispatchIndirect)
    vk::BufferCreateInfo indirectDispatchBufferInfo;
    indirectDispatchBufferInfo.size = sizeof(uint32_t) * 3; // x, y, z dispatch dimensions
    indirectDispatchBufferInfo.usage = vk::BufferUsageFlagBits::eIndirectBuffer | 
                                       vk::BufferUsageFlagBits::eStorageBuffer |
                                       vk::BufferUsageFlagBits::eTransferDst;
    indirectDispatchBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo indirectDispatchAllocInfo{};
    indirectDispatchAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkIndirectDispatchBuffer;
    VkBufferCreateInfo vkIndirectDispatchBufferInfo = static_cast<VkBufferCreateInfo>(indirectDispatchBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkIndirectDispatchBufferInfo,
        &indirectDispatchAllocInfo,
        &vkIndirectDispatchBuffer,
        &indirectDispatchAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create indirect dispatch buffer\n";
        return false;
    }
    
    indirectDispatchBuffer_ = vk::Buffer(vkIndirectDispatchBuffer);
    
    std::cout << "[TriangleSplattingPass] Created buffers (max " 
              << maxTriangles_ << " triangles)\n";
    
    return true;
}

bool TriangleSplattingPass::createShaderModule() {
    // Load compiled SPIR-V shader with multiple path attempts (like other shaders)
    std::vector<std::string> paths = {
        "shaders/TriangleSplatting.comp.spv",
        "../shaders/TriangleSplatting.comp.spv",
        "../../shaders/TriangleSplatting.comp.spv",
        "build/shaders/TriangleSplatting.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/TriangleSplatting.comp.spv\n";
        std::cerr << "[TriangleSplattingPass] Make sure to compile shaders with: glslangValidator -V shaders/TriangleSplatting.comp -o shaders/TriangleSplatting.comp.spv\n";
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
    
    computeShader_ = device_.createShaderModule(createInfo);
    
    std::cout << "[TriangleSplattingPass] Loaded shader module (" 
              << fileSize << " bytes)\n";
    
    return true;
}

bool TriangleSplattingPass::createDescriptorSets() {
    // Descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 4> bindings;
    
    // Binding 0: Output image (storage image)
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageImage;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Triangle buffer (SSBO)
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Sorted indices (SSBO)
    bindings[2].binding = 2;
    bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;

    // Binding 3: Tile culling buffer (SSBO) - для tile-based оптимизации
    bindings[3].binding = 3;
    bindings[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = vk::ShaderStageFlagBits::eCompute;

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    descriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // Descriptor pool
    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageImage;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[1].descriptorCount = 3;  // Triangle buffer + sorted indices + tile culling buffer
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    descriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;
    
    auto sets = device_.allocateDescriptorSets(allocInfo);
    descriptorSet_ = sets[0];
    
    // Update descriptor set
    std::array<vk::WriteDescriptorSet, 4> writes;
    
    // Output image
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageView = outputImageView_;
    imageInfo.imageLayout = vk::ImageLayout::eGeneral;
    
    writes[0].dstSet = descriptorSet_;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = vk::DescriptorType::eStorageImage;
    writes[0].pImageInfo = &imageInfo;
    
    // Triangle buffer
    vk::DescriptorBufferInfo triangleBufferInfo;
    triangleBufferInfo.buffer = triangleBuffer_;
    triangleBufferInfo.offset = 0;
    triangleBufferInfo.range = VK_WHOLE_SIZE;
    
    writes[1].dstSet = descriptorSet_;
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    writes[1].pBufferInfo = &triangleBufferInfo;
    
    // Indices buffer: если включён tile binning, используем глобально отсортированные индексы,
    // иначе — видимые индексы после фрустум-куллинга
    vk::DescriptorBufferInfo indicesBufferInfo;
    indicesBufferInfo.buffer = (pushConstants_.enableTileBinning != 0) ? sortedIndicesBuffer_
                                 : (enableFrustumCulling_ ? visibleIndicesBuffer_ : sortedIndicesBuffer_);
    indicesBufferInfo.offset = 0;
    indicesBufferInfo.range = VK_WHOLE_SIZE;

    writes[2].dstSet = descriptorSet_;
    writes[2].dstBinding = 2;
    writes[2].dstArrayElement = 0;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    writes[2].pBufferInfo = &indicesBufferInfo;

    // Tile culling buffer (проверяем что буфер создан)
    if (tileCullingBuffer_) {
        vk::DescriptorBufferInfo tileCullingBufferInfo;
        tileCullingBufferInfo.buffer = tileCullingBuffer_;
        tileCullingBufferInfo.offset = 0;
        tileCullingBufferInfo.range = VK_WHOLE_SIZE;

        writes[3].dstSet = descriptorSet_;
        writes[3].dstBinding = 3;
        writes[3].dstArrayElement = 0;
        writes[3].descriptorCount = 1;
        writes[3].descriptorType = vk::DescriptorType::eStorageBuffer;
        writes[3].pBufferInfo = &tileCullingBufferInfo;

        // Обновляем только первые 4 дескриптора (буфер создан)
        device_.updateDescriptorSets({writes[0], writes[1], writes[2], writes[3]}, nullptr);
        std::cout << "[TriangleSplattingPass] ✅ Tile culling buffer bound to descriptor set\n";
    } else {
        // Обновляем только первые 3 дескриптора (буфер не создан)
        device_.updateDescriptorSets({writes[0], writes[1], writes[2]}, nullptr);
        std::cout << "[TriangleSplattingPass] ⚠️ Tile culling buffer null, bound only 3 descriptors\n";
    }

    std::cout << "[TriangleSplattingPass] Created descriptor sets\n";
    
    return true;
}

bool TriangleSplattingPass::createPipeline() {
    // Push constant range
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);
    
    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    pipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);
    
    // Compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStage;
    shaderStage.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStage.module = computeShader_;
    shaderStage.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = pipelineLayout_;
    
    auto result = device_.createComputePipeline(nullptr, pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create compute pipeline\n";
        return false;
    }
    
    pipeline_ = result.value;
    
    std::cout << "[TriangleSplattingPass] Created compute pipeline\n";
    
    return true;
}

void TriangleSplattingPass::setViewProjection(const glm::mat4& viewProj) {
    pushConstants_.viewProj = viewProj;
}

void TriangleSplattingPass::setCameraPosition(const glm::vec3& cameraPos) {
    cameraPosition_ = cameraPos;
}

void TriangleSplattingPass::setFrustumCullingEnabled(bool enabled) {
    enableFrustumCulling_ = enabled;
}

uint32_t TriangleSplattingPass::getVisibleTriangleCount() const {
    if (!enableFrustumCulling_ || triangleCount_ == 0) {
        return triangleCount_;
    }
    
    // TODO: Implement GPU → CPU readback for visible count
    // For now, return estimated value (80% visible is typical)
    // Real implementation would require queue and cmdPool members
    return static_cast<uint32_t>(triangleCount_ * 0.8f);
}

void TriangleSplattingPass::uploadTriangles(const std::vector<Triangle>& triangles) {
    // ===== КРИТИЧЕСКАЯ ВАЛИДАЦИЯ =====
    if (triangles.empty()) {
        std::cerr << "[TriangleSplattingPass] ❌ ОШИБКА: Нет треугольников для загрузки!\n";
        std::cerr << "[TriangleSplattingPass]   Убедитесь, что вызываете uploadTriangles() ПЕРЕД renderFrame()\n";
        triangleCount_ = 0;
        pushConstants_.triangleCount = 0;
        return;
    }
    
    if (triangles.size() > maxTriangles_) {
        std::cerr << "[TriangleSplattingPass] ⚠️  ВНИМАНИЕ: Слишком много треугольников (" 
                  << triangles.size() << "), обрезаем до " << maxTriangles_ << "\n";
        triangleCount_ = maxTriangles_;
    } else {
        triangleCount_ = static_cast<uint32_t>(triangles.size());
    }
    
    // Логирование успешной загрузки
    std::cout << "[TriangleSplattingPass] 📦 Загружаем " << triangleCount_ 
              << " треугольников на GPU...\n";

    // === DEBUG: вывод первых 5 треугольников ===
    const uint32_t debugCount = std::min<uint32_t>(triangleCount_, 5);
    for (uint32_t i = 0; i < debugCount; ++i) {
        const auto& t = triangles[i];
        std::cout << "  T" << i << ": v0(" << t.v0.x << "," << t.v0.y << "," << t.v0.z << ")  "
                  << "v1(" << t.v1.x << "," << t.v1.y << "," << t.v1.z << ")  "
                  << "v2(" << t.v2.x << "," << t.v2.y << "," << t.v2.z << ")  "
                  << "opacity=" << t.opacity << " sigma=" << t.sigma << "\n";
    }
    
    pushConstants_.triangleCount = triangleCount_;
    
    // Create staging buffer
    vk::BufferCreateInfo stagingBufferInfo;
    stagingBufferInfo.size = sizeof(Triangle) * triangleCount_;
    stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    VkBuffer vkStagingBuffer;
    VmaAllocation stagingAllocation;
    VmaAllocationInfo allocInfo;
    
    VkBufferCreateInfo vkStagingBufferInfo = static_cast<VkBufferCreateInfo>(stagingBufferInfo);
    
    VkResult result = vmaCreateBuffer(
        allocator_,
        &vkStagingBufferInfo,
        &stagingAllocInfo,
        &vkStagingBuffer,
        &stagingAllocation,
        &allocInfo
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create staging buffer\n";
        return;
    }
    
    // Copy data to staging buffer
    std::memcpy(allocInfo.pMappedData, triangles.data(), sizeof(Triangle) * triangleCount_);
    
    // Create identity sorted indices (0, 1, 2, ...)
    std::vector<uint32_t> indices(triangleCount_);
    for (uint32_t i = 0; i < triangleCount_; ++i) {
        indices[i] = i;
    }
    
    // Upload indices via staging buffer
    vk::BufferCreateInfo indicesStagingInfo;
    indicesStagingInfo.size = sizeof(uint32_t) * triangleCount_;
    indicesStagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
    indicesStagingInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VkBuffer vkIndicesStaging;
    VmaAllocation indicesStagingAlloc;
    VmaAllocationInfo indicesAllocInfo;
    
    VkBufferCreateInfo vkIndicesStagingInfo = static_cast<VkBufferCreateInfo>(indicesStagingInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkIndicesStagingInfo,
        &stagingAllocInfo,
        &vkIndicesStaging,
        &indicesStagingAlloc,
        &indicesAllocInfo
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create indices staging buffer\n";
        vmaDestroyBuffer(allocator_, vkStagingBuffer, stagingAllocation);
        return;
    }
    
    std::memcpy(indicesAllocInfo.pMappedData, indices.data(), sizeof(uint32_t) * triangleCount_);
    
    // ===== GPU UPLOAD USING COMMAND BUFFER =====
    
    // Allocate command buffer
    vk::CommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.commandPool = commandPool_;
    cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdAllocInfo.commandBufferCount = 1;
    
    auto cmdBuffers = device_.allocateCommandBuffers(cmdAllocInfo);
    vk::CommandBuffer cmd = cmdBuffers[0];
    
    // Begin command buffer (one-time submit)
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);
    
    // Copy staging → device (triangles)
    vk::BufferCopy triangleCopyRegion;
    triangleCopyRegion.srcOffset = 0;
    triangleCopyRegion.dstOffset = 0;
    triangleCopyRegion.size = sizeof(Triangle) * triangleCount_;
    cmd.copyBuffer(vk::Buffer(vkStagingBuffer), triangleBuffer_, 1, &triangleCopyRegion);
    
    // Copy staging → device (indices)
    vk::BufferCopy indicesCopyRegion;
    indicesCopyRegion.srcOffset = 0;
    indicesCopyRegion.dstOffset = 0;
    indicesCopyRegion.size = sizeof(uint32_t) * triangleCount_;
    cmd.copyBuffer(vk::Buffer(vkIndicesStaging), sortedIndicesBuffer_, 1, &indicesCopyRegion);
    
    // Buffer barriers для compute shader access
    vk::BufferMemoryBarrier triangleBarrier;
    triangleBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    triangleBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    triangleBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    triangleBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    triangleBarrier.buffer = triangleBuffer_;
    triangleBarrier.offset = 0;
    triangleBarrier.size = VK_WHOLE_SIZE;
    
    vk::BufferMemoryBarrier indicesBarrier;
    indicesBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    indicesBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    indicesBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    indicesBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    indicesBarrier.buffer = sortedIndicesBuffer_;
    indicesBarrier.offset = 0;
    indicesBarrier.size = VK_WHOLE_SIZE;
    
    std::array<vk::BufferMemoryBarrier, 2> barriers = {triangleBarrier, indicesBarrier};
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        static_cast<uint32_t>(barriers.size()), barriers.data(),
        0, nullptr
    );
    
    cmd.end();
    
    // Submit command buffer
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    graphicsQueue_.submit(submitInfo, nullptr);  // Use graphics queue for staging buffer copies
    graphicsQueue_.waitIdle(); // Synchronous upload
    
    // Free command buffer
    device_.freeCommandBuffers(commandPool_, cmdBuffers);
    
    // Cleanup staging buffers
    vmaDestroyBuffer(allocator_, vkStagingBuffer, stagingAllocation);
    vmaDestroyBuffer(allocator_, vkIndicesStaging, indicesStagingAlloc);
    
    std::cout << "[TriangleSplattingPass] Uploaded " << triangleCount_ 
              << " triangles to GPU\n";
}

void TriangleSplattingPass::sortTrianglesByDepth(vk::CommandBuffer cmd) {
    if (triangleCount_ == 0) return;
    
    // Compute depth keys for all triangles
    computeDepthKeys(cmd, cameraPosition_);
    
    // Execute bitonic sort
    const uint32_t LOCAL_SIZE_X = 256;
    const uint32_t BITONIC_BLOCK_SIZE = LOCAL_SIZE_X * 2; // 512
    uint32_t NUM_ELEMENTS = nextPowerOfTwo(triangleCount_);  // Pad to power of 2
    
    // First sort the rows for levels <= BITONIC_BLOCK_SIZE
    for (uint32_t level = 2; level <= BITONIC_BLOCK_SIZE; level *= 2) {
        BitonicSortPushConstants pushConstants;
        pushConstants.h = level;
        pushConstants.algorithm = 0; // LOCAL_BMS
        pushConstants.numElements = NUM_ELEMENTS;
        pushConstants.padding = 0;
        
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
        cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(BitonicSortPushConstants), &pushConstants);
        
        uint32_t groupCountX = std::max(1u, NUM_ELEMENTS / BITONIC_BLOCK_SIZE);
        cmd.dispatch(groupCountX, 1, 1);
        
        // Barrier after each step
        vk::MemoryBarrier memoryBarrier;
        memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );
    }
    
    // Then sort levels > BITONIC_BLOCK_SIZE
    for (uint32_t level = BITONIC_BLOCK_SIZE * 2; level <= NUM_ELEMENTS; level *= 2) {
        // Big flip
        {
            BitonicSortPushConstants pushConstants;
            pushConstants.h = level;
            pushConstants.algorithm = 2; // BIG_FLIP
            pushConstants.numElements = NUM_ELEMENTS;
            pushConstants.padding = 0;
            
            cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
            cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(BitonicSortPushConstants), &pushConstants);
            
            uint32_t groupCountX = std::max(1u, NUM_ELEMENTS / BITONIC_BLOCK_SIZE);
            cmd.dispatch(groupCountX, 1, 1);
            
            vk::MemoryBarrier memoryBarrier;
            memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            
            cmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags{},
                1, &memoryBarrier,
                0, nullptr,
                0, nullptr
            );
        }
        
        // Cascade of disperse operations
        for (uint32_t hh = level / 2; hh > 1; hh /= 2) {
            BitonicSortPushConstants pushConstants;
            pushConstants.numElements = NUM_ELEMENTS;
            pushConstants.padding = 0;
            
            if (hh <= BITONIC_BLOCK_SIZE) {
                // Local disperse
                pushConstants.h = hh;
                pushConstants.algorithm = 1; // LOCAL_DISPERSE
                
                cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
                cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(BitonicSortPushConstants), &pushConstants);
                
                uint32_t groupCountX = std::max(1u, NUM_ELEMENTS / BITONIC_BLOCK_SIZE);
                cmd.dispatch(groupCountX, 1, 1);
                
                break; // Local disperse handles the rest
            } else {
                // Big disperse
                pushConstants.h = hh;
                pushConstants.algorithm = 3; // BIG_DISPERSE
                
                cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
                cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(BitonicSortPushConstants), &pushConstants);
                
                uint32_t groupCountX = std::max(1u, NUM_ELEMENTS / BITONIC_BLOCK_SIZE);
                cmd.dispatch(groupCountX, 1, 1);
            }
            
            vk::MemoryBarrier memoryBarrier;
            memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
            memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            
            cmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags{},
                1, &memoryBarrier,
                0, nullptr,
                0, nullptr
            );
        }
    }
}

void TriangleSplattingPass::execute(vk::CommandBuffer cmd, uint32_t frameIndex) {
    (void)frameIndex; // Suppress unused parameter warning
    
    // ===== КРИТИЧЕСКАЯ ВАЛИДАЦИЯ =====
    if (!initialized_) {
        std::cerr << "[TriangleSplattingPass] ❌ ОШИБКА: Не инициализирован! Вызовите initialize() сначала.\n";
        return;
    }
    
    if (triangleCount_ == 0) {
        std::cerr << "[TriangleSplattingPass] ⚠️  ПРОПУСК: triangleCount == 0\n";
        std::cerr << "[TriangleSplattingPass]   Убедитесь, что вызвали uploadTriangles() перед execute()\n";
        return;
    }
    
    // Логирование успешного старта рендеринга (только первый кадр)
    static bool firstFrameLogged = false;
    if (!firstFrameLogged) {
        std::cout << "[TriangleSplattingPass] ✅ Начинаем рендеринг " << triangleCount_ 
                  << " треугольников\n";
        std::cout << "[TriangleSplattingPass]   Разрешение: " << config_.outputWidth 
                  << "x" << config_.outputHeight << "\n";
        std::cout << "[TriangleSplattingPass]   Depth Sort: " 
                  << (config_.enableDepthSort ? "ON" : "OFF") << "\n";
        std::cout << "[TriangleSplattingPass]   Early Termination: " 
                  << (config_.enableEarlyTermination ? "ON" : "OFF") << "\n";
        std::cout << "[TriangleSplattingPass]   Frustum Culling: " 
                  << (enableFrustumCulling_ ? "ON" : "OFF") << "\n";
        firstFrameLogged = true;
    }
    
    // NOTE: Image layout transition UNDEFINED -> GENERAL выполняется в HybridFreGSRenderer::renderFrame()
    // перед вызовом execute(), чтобы избежать validation errors
    
    // ===== PRE: Reset visibility counters/buffers (per-frame) =====
    if (enableFrustumCulling_) {
        // atomicCounterBuffer_ = 0
        cmd.fillBuffer(atomicCounterBuffer_, 0, sizeof(uint32_t), 0);

        // visibleIndicesBuffer_ = 0xFFFFFFFF (как признак пустого)
        // Размер кратен 4, можно заполнить целиком
        cmd.fillBuffer(visibleIndicesBuffer_, 0, sizeof(uint32_t) * maxTriangles_, 0xFFFFFFFFu);

        // Барьер: transfer write -> shader read/write
        std::array<vk::BufferMemoryBarrier, 2> preBarriers{};
        preBarriers[0].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        preBarriers[0].dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
        preBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preBarriers[0].buffer = atomicCounterBuffer_;
        preBarriers[0].offset = 0;
        preBarriers[0].size = VK_WHOLE_SIZE;

        preBarriers[1].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        preBarriers[1].dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
        preBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        preBarriers[1].buffer = visibleIndicesBuffer_;
        preBarriers[1].offset = 0;
        preBarriers[1].size = VK_WHOLE_SIZE;

        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            static_cast<uint32_t>(preBarriers.size()), preBarriers.data(),
            0, nullptr
        );
    }

    // ===== PHASE 0: Frustum Culling (First!) =====
    // Сначала отсекаем невидимые треугольники, затем сортируем только видимые
    if (enableFrustumCulling_) {
        performFrustumCulling(cmd);
        computeIndirectArgs(cmd);
    }
    
    // ===== PHASE 1: Global depth sort (front-to-back) на видимых треугольниках =====
    sortTrianglesByDepth(cmd);

    // Барьер: sortedIndicesBuffer_ write -> read (tile culling shader)
    {
        vk::BufferMemoryBarrier sb{};
        sb.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        sb.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        sb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        sb.buffer = sortedIndicesBuffer_;
        sb.offset = 0;
        sb.size = VK_WHOLE_SIZE;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            1, &sb,
            0, nullptr
        );
    }

    // ===== PHASE 0.5: Tile Culling (enable tile binning) =====
    if (tileCullingBuffer_ && tileCullingPipeline_) {
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, tileCullingPipeline_);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, tileCullingPipelineLayout_, 0, 1, &tileCullingDescriptorSet_, 0, nullptr);

        struct TilePC {
            glm::mat4 viewProj;
            uint32_t outputWidth;
            uint32_t outputHeight;
            uint32_t triangleCount;
            uint32_t padding;
        } tilePC{};
        tilePC.viewProj = pushConstants_.viewProj;
        tilePC.outputWidth = config_.outputWidth;
        tilePC.outputHeight = config_.outputHeight;
        tilePC.triangleCount = triangleCount_;

        cmd.pushConstants(tileCullingPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(TilePC), &tilePC);

        uint32_t tilesX = std::max(1u, (config_.outputWidth + 15u) / 16u);
        uint32_t tilesY = std::max(1u, (config_.outputHeight + 15u) / 16u);
        cmd.dispatch(tilesX, tilesY, 1);

        // Барьер: tile buffer write -> shader read
        vk::BufferMemoryBarrier tb{};
        tb.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        tb.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        tb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        tb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        tb.buffer = tileCullingBuffer_;
        tb.offset = 0;
        tb.size = VK_WHOLE_SIZE;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            1, &tb,
            0, nullptr
        );

        pushConstants_.enableTileBinning = 1u;
    }

    // Frustum culling уже выполнен в PHASE 0, удаляем дублирование
    
    // ===== PHASE 2: Triangle Splatting Rendering =====
    
    // Выбор режима рендеринга: Two-Pass (O(N+M)) или Single-Pass (O(N×M))
    if (config_.enableTwoPassRendering) {
        // ✅ Two-Pass Rendering: 20-50× faster for large triangle counts
        executeTwoPassRendering(cmd);
    } else {
        // ⚠️  Single-Pass Rendering: slower but simpler (fallback mode)
        // Bind pipeline and descriptor set
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout_, 0, 1, &descriptorSet_, 0, nullptr);
    
    // Push constants
    cmd.pushConstants(pipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstants), &pushConstants_);
    
    // Dispatch compute shader (16x16 local size)
    uint32_t groupsX = (config_.outputWidth + 15) / 16;
    uint32_t groupsY = (config_.outputHeight + 15) / 16;
    cmd.dispatch(groupsX, groupsY, 1);
    
    // Barrier after compute (memory dependency for subsequent reads)
    vk::ImageMemoryBarrier barrier;
    barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eTransferRead;
    barrier.oldLayout = vk::ImageLayout::eGeneral;
    barrier.newLayout = vk::ImageLayout::eGeneral;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = outputImage_;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    } // end Single-Pass mode
}

// ============================================================================
// Bitonic Sort Helper Methods
// ============================================================================

bool TriangleSplattingPass::createBitonicSortResources() {
    std::cout << "[TriangleSplattingPass] Creating Bitonic Sort resources...\n";

    // 1. Load Bitonic Sort shader with multiple path attempts
    std::vector<std::string> paths = {
        "shaders/BitonicSort.comp.spv",
        "../shaders/BitonicSort.comp.spv",
        "../../shaders/BitonicSort.comp.spv",
        "build/shaders/BitonicSort.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded BitonicSort shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/BitonicSort.comp.spv\n";
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> shaderCode(fileSize);
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();
    
    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    
    bitonicSortShader_ = device_.createShaderModule(shaderModuleInfo);
    
    // 2. Create descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
    
    // Binding 0: Depth keys buffer
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Indices buffer
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    bitonicSortDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // 3. Create pipeline layout with push constants
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(BitonicSortPushConstants);
    
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &bitonicSortDescriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    bitonicSortPipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);
    
    // 4. Create compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = bitonicSortShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = bitonicSortPipelineLayout_;
    
    auto result = device_.createComputePipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Bitonic Sort pipeline\n";
        return false;
    }
    bitonicSortPipeline_ = result.value;
    
    // 5. Create descriptor pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 2; // depth keys + indices
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    bitonicSortDescriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // 6. Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = bitonicSortDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &bitonicSortDescriptorSetLayout_;
    
    auto descriptorSets = device_.allocateDescriptorSets(allocInfo);
    bitonicSortDescriptorSet_ = descriptorSets[0];
    
    // 7. Update descriptor set
    std::array<vk::WriteDescriptorSet, 2> descriptorWrites;
    
    vk::DescriptorBufferInfo depthKeysBufferInfo;
    depthKeysBufferInfo.buffer = depthKeysBuffer_;
    depthKeysBufferInfo.offset = 0;
    depthKeysBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[0].dstSet = bitonicSortDescriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &depthKeysBufferInfo;
    
    vk::DescriptorBufferInfo indicesBufferInfo;
    indicesBufferInfo.buffer = sortedIndicesBuffer_;
    indicesBufferInfo.offset = 0;
    indicesBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[1].dstSet = bitonicSortDescriptorSet_;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &indicesBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    
    std::cout << "[TriangleSplattingPass] Created Bitonic Sort resources\n";
    return true;
}

bool TriangleSplattingPass::createDepthKeyComputeResources() {
    std::cout << "[TriangleSplattingPass] Creating Depth Key Compute resources...\n";

    // 1. Load Depth Key Compute shader with multiple path attempts
    std::vector<std::string> paths = {
        "shaders/DepthKeyCompute.comp.spv",
        "../shaders/DepthKeyCompute.comp.spv",
        "../../shaders/DepthKeyCompute.comp.spv",
        "build/shaders/DepthKeyCompute.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded DepthKeyCompute shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/DepthKeyCompute.comp.spv\n";
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> shaderCode(fileSize);
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();
    
    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    
    depthKeyComputeShader_ = device_.createShaderModule(shaderModuleInfo);
    
    // 2. Create descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 4> bindings;
    
    // Binding 0: Triangle buffer (input)
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Depth keys buffer (output)
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Sorted indices buffer (output - initialized)
    bindings[2].binding = 2;
    bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 3: Visible indices buffer (input from frustum culling)
    bindings[3].binding = 3;
    bindings[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    depthKeyComputeDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // 3. Create pipeline layout with push constants
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(DepthKeyComputePushConstants);
    
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &depthKeyComputeDescriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    depthKeyComputePipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);
    
    // 4. Create compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = depthKeyComputeShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = depthKeyComputePipelineLayout_;
    
    auto result = device_.createComputePipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Depth Key Compute pipeline\n";
        return false;
    }
    depthKeyComputePipeline_ = result.value;
    
    // 5. Create descriptor pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 4; // triangles + depth keys + sorted indices + visible indices
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    depthKeyComputeDescriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // 6. Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = depthKeyComputeDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &depthKeyComputeDescriptorSetLayout_;
    
    auto descriptorSets = device_.allocateDescriptorSets(allocInfo);
    depthKeyComputeDescriptorSet_ = descriptorSets[0];
    
    // 7. Update descriptor set
    std::array<vk::WriteDescriptorSet, 4> descriptorWrites;
    
    vk::DescriptorBufferInfo triangleBufferInfo;
    triangleBufferInfo.buffer = triangleBuffer_;
    triangleBufferInfo.offset = 0;
    triangleBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[0].dstSet = depthKeyComputeDescriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &triangleBufferInfo;
    
    vk::DescriptorBufferInfo depthKeysBufferInfo;
    depthKeysBufferInfo.buffer = depthKeysBuffer_;
    depthKeysBufferInfo.offset = 0;
    depthKeysBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[1].dstSet = depthKeyComputeDescriptorSet_;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &depthKeysBufferInfo;
    
    vk::DescriptorBufferInfo indicesBufferInfo;
    indicesBufferInfo.buffer = sortedIndicesBuffer_;
    indicesBufferInfo.offset = 0;
    indicesBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[2].dstSet = depthKeyComputeDescriptorSet_;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &indicesBufferInfo;
    
    vk::DescriptorBufferInfo visibleIndicesBufferInfo;
    visibleIndicesBufferInfo.buffer = visibleIndicesBuffer_;
    visibleIndicesBufferInfo.offset = 0;
    visibleIndicesBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[3].dstSet = depthKeyComputeDescriptorSet_;
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &visibleIndicesBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    
    std::cout << "[TriangleSplattingPass] Created Depth Key Compute resources\n";
    return true;
}

void TriangleSplattingPass::computeDepthKeys(vk::CommandBuffer cmd, const glm::vec3& cameraPos) {
    if (triangleCount_ == 0) return;
    
    // Bind depth key compute pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, depthKeyComputePipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, depthKeyComputePipelineLayout_, 0, 1, &depthKeyComputeDescriptorSet_, 0, nullptr);
    
    // Push constants
    DepthKeyComputePushConstants pushConstants;
    pushConstants.cameraPos = cameraPos;
    pushConstants.triangleCount = triangleCount_; // Will be visible count if culling enabled
    pushConstants.useCulling = enableFrustumCulling_ ? 1 : 0;
    pushConstants.padding = 0;
    
    cmd.pushConstants(depthKeyComputePipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(DepthKeyComputePushConstants), &pushConstants);
    
    // Dispatch
    if (enableFrustumCulling_) {
        // Use indirect dispatch (workgroup count computed by IndirectArgsCompute)
        cmd.dispatchIndirect(indirectDispatchBuffer_, 0);
    } else {
        // Direct dispatch (all triangles)
        uint32_t groupCount = (triangleCount_ + 255) / 256;
        cmd.dispatch(groupCount, 1, 1);
    }
    
    // Memory barrier (depth keys must be ready before sorting)
    vk::MemoryBarrier memoryBarrier;
    memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        1, &memoryBarrier,
        0, nullptr,
        0, nullptr
    );
}

bool TriangleSplattingPass::createFrustumCullingResources() {
    std::cout << "[TriangleSplattingPass] Creating Frustum Culling resources...\n";

    // 1. Load Frustum Culling shader with multiple path attempts
    std::vector<std::string> paths = {
        "shaders/FrustumCulling.comp.spv",
        "../shaders/FrustumCulling.comp.spv",
        "../../shaders/FrustumCulling.comp.spv",
        "build/shaders/FrustumCulling.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded FrustumCulling shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/FrustumCulling.comp.spv\n";
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> shaderCode(fileSize);
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();
    
    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    
    frustumCullingShader_ = device_.createShaderModule(shaderModuleInfo);
    
    // 2. Create descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings;
    
    // Binding 0: Triangle buffer (input)
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Visible indices buffer (output)
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Atomic counter buffer (output)
    bindings[2].binding = 2;
    bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    frustumCullingDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // 3. Create pipeline layout with push constants
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(FrustumCullingPushConstants);
    
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &frustumCullingDescriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    frustumCullingPipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);
    
    // 4. Create compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = frustumCullingShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = frustumCullingPipelineLayout_;
    
    auto result = device_.createComputePipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Frustum Culling pipeline\n";
        return false;
    }
    frustumCullingPipeline_ = result.value;
    
    // 5. Create descriptor pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 3; // triangles + visible indices + atomic counter
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    frustumCullingDescriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // 6. Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = frustumCullingDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &frustumCullingDescriptorSetLayout_;
    
    auto descriptorSets = device_.allocateDescriptorSets(allocInfo);
    frustumCullingDescriptorSet_ = descriptorSets[0];
    
    // 7. Update descriptor set
    std::array<vk::WriteDescriptorSet, 3> descriptorWrites;
    
    vk::DescriptorBufferInfo triangleBufferInfo;
    triangleBufferInfo.buffer = triangleBuffer_;
    triangleBufferInfo.offset = 0;
    triangleBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[0].dstSet = frustumCullingDescriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &triangleBufferInfo;
    
    vk::DescriptorBufferInfo visibleIndicesBufferInfo;
    visibleIndicesBufferInfo.buffer = visibleIndicesBuffer_;
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
    atomicCounterBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[2].dstSet = frustumCullingDescriptorSet_;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &atomicCounterBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    
    std::cout << "[TriangleSplattingPass] Created Frustum Culling resources\n";
    return true;
}

void TriangleSplattingPass::performFrustumCulling(vk::CommandBuffer cmd) {
    if (triangleCount_ == 0) return;
    
    // Reset atomic counter to 0
    cmd.fillBuffer(atomicCounterBuffer_, 0, sizeof(uint32_t), 0);
    
    // Memory barrier (atomic counter must be 0 before culling)
    vk::BufferMemoryBarrier bufferBarrier;
    bufferBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    bufferBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.buffer = atomicCounterBuffer_;
    bufferBarrier.offset = 0;
    bufferBarrier.size = VK_WHOLE_SIZE;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        1, &bufferBarrier,
        0, nullptr
    );
    
    // Bind frustum culling pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, frustumCullingPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, frustumCullingPipelineLayout_, 0, 1, &frustumCullingDescriptorSet_, 0, nullptr);
    
    // Push constants (view-projection matrix + triangle count)
    FrustumCullingPushConstants pushConstants;
    pushConstants.viewProj = pushConstants_.viewProj;
    pushConstants.triangleCount = triangleCount_;
    std::memset(pushConstants.padding, 0, sizeof(pushConstants.padding));
    
    cmd.pushConstants(frustumCullingPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(FrustumCullingPushConstants), &pushConstants);
    
    // Dispatch (256 threads per workgroup)
    uint32_t groupCount = (triangleCount_ + 255) / 256;
    cmd.dispatch(groupCount, 1, 1);
    
    // Memory barrier (visible indices and atomic counter must be ready)
    vk::BufferMemoryBarrier visibleBarrier;
    visibleBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    visibleBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    visibleBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    visibleBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    visibleBarrier.buffer = visibleIndicesBuffer_;
    visibleBarrier.offset = 0;
    visibleBarrier.size = VK_WHOLE_SIZE;
    
    vk::BufferMemoryBarrier counterBarrier;
    counterBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    counterBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    counterBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    counterBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    counterBarrier.buffer = atomicCounterBuffer_;
    counterBarrier.offset = 0;
    counterBarrier.size = VK_WHOLE_SIZE;
    
    std::array<vk::BufferMemoryBarrier, 2> barriers = {visibleBarrier, counterBarrier};
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        static_cast<uint32_t>(barriers.size()), barriers.data(),
        0, nullptr
    );
}

bool TriangleSplattingPass::createIndirectArgsResources() {
    std::cout << "[TriangleSplattingPass] Creating Indirect Args Compute resources...\n";

    // 1. Load Indirect Args Compute shader with multiple path attempts
    std::vector<std::string> paths = {
        "shaders/IndirectArgsCompute.comp.spv",
        "../shaders/IndirectArgsCompute.comp.spv",
        "../../shaders/IndirectArgsCompute.comp.spv",
        "build/shaders/IndirectArgsCompute.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded IndirectArgsCompute shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/IndirectArgsCompute.comp.spv\n";
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> shaderCode(fileSize);
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();
    
    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    
    indirectArgsShader_ = device_.createShaderModule(shaderModuleInfo);
    
    // 2. Create descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
    
    // Binding 0: Indirect dispatch args buffer (output)
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Visible count buffer (input)
    bindings[1].binding = 1;
    bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    indirectArgsDescriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    
    // 3. Create pipeline layout with push constants
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(IndirectArgsPushConstants);
    
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &indirectArgsDescriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    indirectArgsPipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo);
    
    // 4. Create compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = indirectArgsShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = indirectArgsPipelineLayout_;
    
    auto result = device_.createComputePipeline(nullptr, pipelineInfo);
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Indirect Args pipeline\n";
        return false;
    }
    indirectArgsPipeline_ = result.value;
    
    // 5. Create descriptor pool
    std::array<vk::DescriptorPoolSize, 1> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 2; // indirect args + visible count
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    indirectArgsDescriptorPool_ = device_.createDescriptorPool(poolInfo);
    
    // 6. Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = indirectArgsDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &indirectArgsDescriptorSetLayout_;
    
    auto descriptorSets = device_.allocateDescriptorSets(allocInfo);
    indirectArgsDescriptorSet_ = descriptorSets[0];
    
    // 7. Update descriptor set
    std::array<vk::WriteDescriptorSet, 2> descriptorWrites;
    
    vk::DescriptorBufferInfo indirectArgsBufferInfo;
    indirectArgsBufferInfo.buffer = indirectDispatchBuffer_;
    indirectArgsBufferInfo.offset = 0;
    indirectArgsBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[0].dstSet = indirectArgsDescriptorSet_;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &indirectArgsBufferInfo;
    
    vk::DescriptorBufferInfo visibleCountBufferInfo;
    visibleCountBufferInfo.buffer = atomicCounterBuffer_;
    visibleCountBufferInfo.offset = 0;
    visibleCountBufferInfo.range = VK_WHOLE_SIZE;
    
    descriptorWrites[1].dstSet = indirectArgsDescriptorSet_;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &visibleCountBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    
    std::cout << "[TriangleSplattingPass] Created Indirect Args Compute resources\n";
    return true;
}

bool TriangleSplattingPass::createTileCullingResources() {
    std::cout << "[TriangleSplattingPass] Creating Tile Culling resources...\n";

    // Вычисляем количество тайлов (16x16 тайлы) с округлением вверх
    const uint32_t tilesX = std::max(1u, (config_.outputWidth + 15u) / 16u);
    const uint32_t tilesY = std::max(1u, (config_.outputHeight + 15u) / 16u);
    tileCount_ = tilesX * tilesY;

    // Создаем буфер для хранения данных tile culling
    // Структура: для каждого тайла храним количество видимых треугольников и их индексы
    const uint32_t maxTrianglesPerTile = 256; // Максимум треугольников на тайл
    const uint32_t tileDataSize = sizeof(uint32_t) + maxTrianglesPerTile * sizeof(uint32_t); // count + indices
    const uint32_t totalBufferSize = tileCount_ * tileDataSize;

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = totalBufferSize;
    bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBuffer vkBuffer;
    VkBufferCreateInfo vkBufferInfo = static_cast<VkBufferCreateInfo>(bufferInfo);

    if (vmaCreateBuffer(allocator_, &vkBufferInfo, &allocInfo, &vkBuffer, &tileCullingAllocation_, nullptr) != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create tile culling buffer\n";
        return false;
    }

    tileCullingBuffer_ = vk::Buffer(vkBuffer);

    std::cout << "[TriangleSplattingPass] Created tile culling buffer (" << tileCount_ << " tiles, " << totalBufferSize << " bytes)\n";
    return true;
}

bool TriangleSplattingPass::createTileCullingPipelineResources() {
    // Load Tile Culling shader with multiple path attempts
    std::vector<std::string> paths = {
        "shaders/TileCulling.comp.spv",
        "../shaders/TileCulling.comp.spv",
        "../../shaders/TileCulling.comp.spv",
        "build/shaders/TileCulling.comp.spv"
    };

    std::ifstream file;
    for (const auto& path : paths) {
        file.open(path, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::cout << "[TriangleSplattingPass] Loaded TileCulling shader from: " << path << std::endl;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader file: shaders/TileCulling.comp.spv\n";
        return false;
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> shaderCode(fileSize);
    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = shaderCode.size();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    tileCullingShader_ = device_.createShaderModule(shaderModuleInfo);

    // Descriptor set layout: 0=triangles, 1=sortedIndices, 2=tileCullingBuffer
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
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

    vk::DescriptorSetLayoutCreateInfo dslInfo{};
    dslInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    dslInfo.pBindings = bindings.data();
    tileCullingDescriptorSetLayout_ = device_.createDescriptorSetLayout(dslInfo);

    // Pipeline layout with push constants
    vk::PushConstantRange pcr{};
    pcr.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pcr.offset = 0;
    pcr.size = sizeof(glm::mat4) + sizeof(uint32_t) * 4; // viewProj + 4 uints

    vk::PipelineLayoutCreateInfo pli{};
    pli.setLayoutCount = 1;
    pli.pSetLayouts = &tileCullingDescriptorSetLayout_;
    pli.pushConstantRangeCount = 1;
    pli.pPushConstantRanges = &pcr;
    tileCullingPipelineLayout_ = device_.createPipelineLayout(pli);

    // Pipeline
    vk::PipelineShaderStageCreateInfo stage{};
    stage.stage = vk::ShaderStageFlagBits::eCompute;
    stage.module = tileCullingShader_;
    stage.pName = "main";

    vk::ComputePipelineCreateInfo cpi{};
    cpi.stage = stage;
    cpi.layout = tileCullingPipelineLayout_;
    auto res = device_.createComputePipeline(nullptr, cpi);
    if (res.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Tile Culling pipeline\n";
        return false;
    }
    tileCullingPipeline_ = res.value;

    // Descriptor pool and set
    std::array<vk::DescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[0].descriptorCount = 3;

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    tileCullingDescriptorPool_ = device_.createDescriptorPool(poolInfo);

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = tileCullingDescriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &tileCullingDescriptorSetLayout_;
    tileCullingDescriptorSet_ = device_.allocateDescriptorSets(allocInfo)[0];

    // Update descriptors
    std::array<vk::WriteDescriptorSet, 3> writes{};

    vk::DescriptorBufferInfo triInfo{};
    triInfo.buffer = triangleBuffer_;
    triInfo.offset = 0;
    triInfo.range = VK_WHOLE_SIZE;
    writes[0].dstSet = tileCullingDescriptorSet_;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    writes[0].pBufferInfo = &triInfo;

    vk::DescriptorBufferInfo sortedInfo{};
    sortedInfo.buffer = sortedIndicesBuffer_;
    sortedInfo.offset = 0;
    sortedInfo.range = VK_WHOLE_SIZE;
    writes[1].dstSet = tileCullingDescriptorSet_;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    writes[1].pBufferInfo = &sortedInfo;

    vk::DescriptorBufferInfo tileOutInfo{};
    tileOutInfo.buffer = tileCullingBuffer_;
    tileOutInfo.offset = 0;
    tileOutInfo.range = VK_WHOLE_SIZE;
    writes[2].dstSet = tileCullingDescriptorSet_;
    writes[2].dstBinding = 2;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    writes[2].pBufferInfo = &tileOutInfo;

    device_.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    std::cout << "[TriangleSplattingPass] Created Tile Culling pipeline resources\n";
    return true;
}

void TriangleSplattingPass::computeIndirectArgs(vk::CommandBuffer cmd) {
    // Bind indirect args compute pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, indirectArgsPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, indirectArgsPipelineLayout_, 0, 1, &indirectArgsDescriptorSet_, 0, nullptr);
    
    // Push constants (threads per group)
    IndirectArgsPushConstants pushConstants;
    pushConstants.threadsPerGroup = 256; // Match depth key compute workgroup size
    std::memset(pushConstants.padding, 0, sizeof(pushConstants.padding));
    
    cmd.pushConstants(indirectArgsPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(IndirectArgsPushConstants), &pushConstants);
    
    // Dispatch (single thread computes all args)
    cmd.dispatch(1, 1, 1);
    
    // Memory barrier (indirect dispatch buffer must be ready)
    vk::BufferMemoryBarrier indirectArgsBarrier;
    indirectArgsBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    indirectArgsBarrier.dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead;
    indirectArgsBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    indirectArgsBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    indirectArgsBarrier.buffer = indirectDispatchBuffer_;
    indirectArgsBarrier.offset = 0;
    indirectArgsBarrier.size = VK_WHOLE_SIZE;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eDrawIndirect,
        vk::DependencyFlags{},
        0, nullptr,
        1, &indirectArgsBarrier,
        0, nullptr
    );
}

uint32_t TriangleSplattingPass::nextPowerOfTwo(uint32_t n) {
    if (n == 0) return 1;
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    
    return n;
}

// ============================================================================
// Two-Pass Rendering Implementation (Priority #1 Optimization)
// ============================================================================

bool TriangleSplattingPass::createTwoPassResources() {
    std::cout << "[TriangleSplattingPass] Creating Two-Pass Rendering resources...\n";
    
    // ===== 1. Create Visibility Buffer =====
    uint32_t numPixels = config_.outputWidth * config_.outputHeight;
    uint32_t visibilityBufferSize = numPixels * (1 + config_.maxTrianglesPerPixel) * sizeof(uint32_t);
    
    vk::BufferCreateInfo visibilityBufferInfo;
    visibilityBufferInfo.size = visibilityBufferSize;
    visibilityBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                                  vk::BufferUsageFlagBits::eTransferDst;
    visibilityBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo visibilityAllocInfo{};
    visibilityAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkVisibilityBuffer;
    VkBufferCreateInfo vkVisibilityBufferInfo = static_cast<VkBufferCreateInfo>(visibilityBufferInfo);
    
    VkResult result = vmaCreateBuffer(
        allocator_,
        &vkVisibilityBufferInfo,
        &visibilityAllocInfo,
        &vkVisibilityBuffer,
        &visibilityBufferAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create visibility buffer\n";
        return false;
    }
    
    visibilityBuffer_ = vk::Buffer(vkVisibilityBuffer);
    
    std::cout << "[TriangleSplattingPass] Created visibility buffer: " 
              << (visibilityBufferSize / 1024 / 1024) << " MB\n";
    
    // ===== 2. Create Pixel Counters Buffer =====
    vk::BufferCreateInfo countersBufferInfo;
    countersBufferInfo.size = numPixels * sizeof(uint32_t);
    countersBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                                vk::BufferUsageFlagBits::eTransferDst;
    countersBufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo countersAllocInfo{};
    countersAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkBuffer vkCountersBuffer;
    VkBufferCreateInfo vkCountersBufferInfo = static_cast<VkBufferCreateInfo>(countersBufferInfo);
    
    result = vmaCreateBuffer(
        allocator_,
        &vkCountersBufferInfo,
        &countersAllocInfo,
        &vkCountersBuffer,
        &pixelCountersAllocation_,
        nullptr
    );
    
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create pixel counters buffer\n";
        return false;
    }
    
    pixelCountersBuffer_ = vk::Buffer(vkCountersBuffer);
    
    std::cout << "[TriangleSplattingPass] Created pixel counters buffer: " 
              << (numPixels * sizeof(uint32_t) / 1024 / 1024) << " MB\n";
    
    // ===== 3. Load Visibility Pass Shader =====
    std::ifstream visFile("shaders/TriangleVisibility.comp.spv", std::ios::binary | std::ios::ate);
    
    if (!visFile.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader: shaders/TriangleVisibility.comp.spv\n";
        return false;
    }
    
    size_t visFileSize = static_cast<size_t>(visFile.tellg());
    std::vector<char> visShaderCode(visFileSize);
    visFile.seekg(0);
    visFile.read(visShaderCode.data(), visFileSize);
    visFile.close();
    
    vk::ShaderModuleCreateInfo visShaderModuleInfo;
    visShaderModuleInfo.codeSize = visShaderCode.size();
    visShaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(visShaderCode.data());
    
    visibilityPassShader_ = device_.createShaderModule(visShaderModuleInfo);
    
    // ===== 4. Load Shading Pass Shader =====
    std::ifstream shadFile("shaders/TriangleShading.comp.spv", std::ios::binary | std::ios::ate);
    
    if (!shadFile.is_open()) {
        std::cerr << "[TriangleSplattingPass] Failed to open shader: shaders/TriangleShading.comp.spv\n";
        return false;
    }
    
    size_t shadFileSize = static_cast<size_t>(shadFile.tellg());
    std::vector<char> shadShaderCode(shadFileSize);
    shadFile.seekg(0);
    shadFile.read(shadShaderCode.data(), shadFileSize);
    shadFile.close();
    
    vk::ShaderModuleCreateInfo shadShaderModuleInfo;
    shadShaderModuleInfo.codeSize = shadShaderCode.size();
    shadShaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(shadShaderCode.data());
    
    shadingPassShader_ = device_.createShaderModule(shadShaderModuleInfo);
    
    // ===== 5. Create Visibility Pass Descriptor Set Layout =====
    std::array<vk::DescriptorSetLayoutBinding, 4> visBindings;
    
    // Binding 0: Triangle buffer
    visBindings[0].binding = 0;
    visBindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    visBindings[0].descriptorCount = 1;
    visBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Sorted indices
    visBindings[1].binding = 1;
    visBindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    visBindings[1].descriptorCount = 1;
    visBindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Visibility buffer (output)
    visBindings[2].binding = 2;
    visBindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    visBindings[2].descriptorCount = 1;
    visBindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 3: Pixel counters
    visBindings[3].binding = 3;
    visBindings[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    visBindings[3].descriptorCount = 1;
    visBindings[3].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo visLayoutInfo;
    visLayoutInfo.bindingCount = static_cast<uint32_t>(visBindings.size());
    visLayoutInfo.pBindings = visBindings.data();
    
    visibilityPassDescriptorSetLayout_ = device_.createDescriptorSetLayout(visLayoutInfo);
    
    // ===== 6. Create Visibility Pass Pipeline Layout =====
    vk::PushConstantRange visPushConstantRange;
    visPushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    visPushConstantRange.offset = 0;
    visPushConstantRange.size = sizeof(TwoPassPushConstants);
    
    vk::PipelineLayoutCreateInfo visPipelineLayoutInfo;
    visPipelineLayoutInfo.setLayoutCount = 1;
    visPipelineLayoutInfo.pSetLayouts = &visibilityPassDescriptorSetLayout_;
    visPipelineLayoutInfo.pushConstantRangeCount = 1;
    visPipelineLayoutInfo.pPushConstantRanges = &visPushConstantRange;
    
    visibilityPassPipelineLayout_ = device_.createPipelineLayout(visPipelineLayoutInfo);
    
    // ===== 7. Create Visibility Pass Pipeline =====
    vk::PipelineShaderStageCreateInfo visShaderStageInfo;
    visShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    visShaderStageInfo.module = visibilityPassShader_;
    visShaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo visPipelineInfo;
    visPipelineInfo.stage = visShaderStageInfo;
    visPipelineInfo.layout = visibilityPassPipelineLayout_;
    
    auto visResult = device_.createComputePipeline(nullptr, visPipelineInfo);
    if (visResult.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Visibility Pass pipeline\n";
        return false;
    }
    visibilityPassPipeline_ = visResult.value;
    
    // ===== 8. Create Shading Pass Descriptor Set Layout =====
    std::array<vk::DescriptorSetLayoutBinding, 3> shadBindings;
    
    // Binding 0: Output image
    shadBindings[0].binding = 0;
    shadBindings[0].descriptorType = vk::DescriptorType::eStorageImage;
    shadBindings[0].descriptorCount = 1;
    shadBindings[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 1: Triangle buffer
    shadBindings[1].binding = 1;
    shadBindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    shadBindings[1].descriptorCount = 1;
    shadBindings[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    // Binding 2: Visibility buffer (input)
    shadBindings[2].binding = 2;
    shadBindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    shadBindings[2].descriptorCount = 1;
    shadBindings[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    
    vk::DescriptorSetLayoutCreateInfo shadLayoutInfo;
    shadLayoutInfo.bindingCount = static_cast<uint32_t>(shadBindings.size());
    shadLayoutInfo.pBindings = shadBindings.data();
    
    shadingPassDescriptorSetLayout_ = device_.createDescriptorSetLayout(shadLayoutInfo);
    
    // ===== 9. Create Shading Pass Pipeline Layout =====
    vk::PushConstantRange shadPushConstantRange;
    shadPushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    shadPushConstantRange.offset = 0;
    shadPushConstantRange.size = sizeof(TwoPassPushConstants);
    
    vk::PipelineLayoutCreateInfo shadPipelineLayoutInfo;
    shadPipelineLayoutInfo.setLayoutCount = 1;
    shadPipelineLayoutInfo.pSetLayouts = &shadingPassDescriptorSetLayout_;
    shadPipelineLayoutInfo.pushConstantRangeCount = 1;
    shadPipelineLayoutInfo.pPushConstantRanges = &shadPushConstantRange;
    
    shadingPassPipelineLayout_ = device_.createPipelineLayout(shadPipelineLayoutInfo);
    
    // ===== 10. Create Shading Pass Pipeline =====
    vk::PipelineShaderStageCreateInfo shadShaderStageInfo;
    shadShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shadShaderStageInfo.module = shadingPassShader_;
    shadShaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo shadPipelineInfo;
    shadPipelineInfo.stage = shadShaderStageInfo;
    shadPipelineInfo.layout = shadingPassPipelineLayout_;
    
    auto shadResult = device_.createComputePipeline(nullptr, shadPipelineInfo);
    if (shadResult.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingPass] Failed to create Shading Pass pipeline\n";
        return false;
    }
    shadingPassPipeline_ = shadResult.value;
    
    // ===== 11. Create Descriptor Pools =====
    std::array<vk::DescriptorPoolSize, 1> visPoolSizes;
    visPoolSizes[0].type = vk::DescriptorType::eStorageBuffer;
    visPoolSizes[0].descriptorCount = 4;
    
    vk::DescriptorPoolCreateInfo visPoolInfo;
    visPoolInfo.maxSets = 1;
    visPoolInfo.poolSizeCount = static_cast<uint32_t>(visPoolSizes.size());
    visPoolInfo.pPoolSizes = visPoolSizes.data();
    
    visibilityPassDescriptorPool_ = device_.createDescriptorPool(visPoolInfo);
    
    std::array<vk::DescriptorPoolSize, 2> shadPoolSizes;
    shadPoolSizes[0].type = vk::DescriptorType::eStorageImage;
    shadPoolSizes[0].descriptorCount = 1;
    shadPoolSizes[1].type = vk::DescriptorType::eStorageBuffer;
    shadPoolSizes[1].descriptorCount = 2;
    
    vk::DescriptorPoolCreateInfo shadPoolInfo;
    shadPoolInfo.maxSets = 1;
    shadPoolInfo.poolSizeCount = static_cast<uint32_t>(shadPoolSizes.size());
    shadPoolInfo.pPoolSizes = shadPoolSizes.data();
    
    shadingPassDescriptorPool_ = device_.createDescriptorPool(shadPoolInfo);
    
    // ===== 12. Allocate Descriptor Sets =====
    vk::DescriptorSetAllocateInfo visAllocInfo;
    visAllocInfo.descriptorPool = visibilityPassDescriptorPool_;
    visAllocInfo.descriptorSetCount = 1;
    visAllocInfo.pSetLayouts = &visibilityPassDescriptorSetLayout_;
    
    auto visSets = device_.allocateDescriptorSets(visAllocInfo);
    visibilityPassDescriptorSet_ = visSets[0];
    
    vk::DescriptorSetAllocateInfo shadAllocInfo;
    shadAllocInfo.descriptorPool = shadingPassDescriptorPool_;
    shadAllocInfo.descriptorSetCount = 1;
    shadAllocInfo.pSetLayouts = &shadingPassDescriptorSetLayout_;
    
    auto shadSets = device_.allocateDescriptorSets(shadAllocInfo);
    shadingPassDescriptorSet_ = shadSets[0];
    
    // ===== 13. Update Descriptor Sets =====
    
    // Visibility Pass Descriptors
    std::array<vk::WriteDescriptorSet, 4> visWrites;
    
    vk::DescriptorBufferInfo visTriBufferInfo;
    visTriBufferInfo.buffer = triangleBuffer_;
    visTriBufferInfo.offset = 0;
    visTriBufferInfo.range = VK_WHOLE_SIZE;
    
    visWrites[0].dstSet = visibilityPassDescriptorSet_;
    visWrites[0].dstBinding = 0;
    visWrites[0].dstArrayElement = 0;
    visWrites[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    visWrites[0].descriptorCount = 1;
    visWrites[0].pBufferInfo = &visTriBufferInfo;
    
    vk::DescriptorBufferInfo visSortedIndicesInfo;
    visSortedIndicesInfo.buffer = sortedIndicesBuffer_;
    visSortedIndicesInfo.offset = 0;
    visSortedIndicesInfo.range = VK_WHOLE_SIZE;
    
    visWrites[1].dstSet = visibilityPassDescriptorSet_;
    visWrites[1].dstBinding = 1;
    visWrites[1].dstArrayElement = 0;
    visWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    visWrites[1].descriptorCount = 1;
    visWrites[1].pBufferInfo = &visSortedIndicesInfo;
    
    vk::DescriptorBufferInfo visVisibilityBufferInfo;
    visVisibilityBufferInfo.buffer = visibilityBuffer_;
    visVisibilityBufferInfo.offset = 0;
    visVisibilityBufferInfo.range = VK_WHOLE_SIZE;
    
    visWrites[2].dstSet = visibilityPassDescriptorSet_;
    visWrites[2].dstBinding = 2;
    visWrites[2].dstArrayElement = 0;
    visWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    visWrites[2].descriptorCount = 1;
    visWrites[2].pBufferInfo = &visVisibilityBufferInfo;
    
    vk::DescriptorBufferInfo visCountersInfo;
    visCountersInfo.buffer = pixelCountersBuffer_;
    visCountersInfo.offset = 0;
    visCountersInfo.range = VK_WHOLE_SIZE;
    
    visWrites[3].dstSet = visibilityPassDescriptorSet_;
    visWrites[3].dstBinding = 3;
    visWrites[3].dstArrayElement = 0;
    visWrites[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    visWrites[3].descriptorCount = 1;
    visWrites[3].pBufferInfo = &visCountersInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(visWrites.size()), visWrites.data(), 0, nullptr);
    
    // Shading Pass Descriptors
    std::array<vk::WriteDescriptorSet, 3> shadWrites;
    
    vk::DescriptorImageInfo shadImageInfo;
    shadImageInfo.imageView = outputImageView_;
    shadImageInfo.imageLayout = vk::ImageLayout::eGeneral;
    
    shadWrites[0].dstSet = shadingPassDescriptorSet_;
    shadWrites[0].dstBinding = 0;
    shadWrites[0].dstArrayElement = 0;
    shadWrites[0].descriptorType = vk::DescriptorType::eStorageImage;
    shadWrites[0].descriptorCount = 1;
    shadWrites[0].pImageInfo = &shadImageInfo;
    
    vk::DescriptorBufferInfo shadTriBufferInfo;
    shadTriBufferInfo.buffer = triangleBuffer_;
    shadTriBufferInfo.offset = 0;
    shadTriBufferInfo.range = VK_WHOLE_SIZE;
    
    shadWrites[1].dstSet = shadingPassDescriptorSet_;
    shadWrites[1].dstBinding = 1;
    shadWrites[1].dstArrayElement = 0;
    shadWrites[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    shadWrites[1].descriptorCount = 1;
    shadWrites[1].pBufferInfo = &shadTriBufferInfo;
    
    vk::DescriptorBufferInfo shadVisibilityBufferInfo;
    shadVisibilityBufferInfo.buffer = visibilityBuffer_;
    shadVisibilityBufferInfo.offset = 0;
    shadVisibilityBufferInfo.range = VK_WHOLE_SIZE;
    
    shadWrites[2].dstSet = shadingPassDescriptorSet_;
    shadWrites[2].dstBinding = 2;
    shadWrites[2].dstArrayElement = 0;
    shadWrites[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    shadWrites[2].descriptorCount = 1;
    shadWrites[2].pBufferInfo = &shadVisibilityBufferInfo;
    
    device_.updateDescriptorSets(static_cast<uint32_t>(shadWrites.size()), shadWrites.data(), 0, nullptr);
    
    std::cout << "[TriangleSplattingPass] ✅ Two-Pass Rendering resources created successfully\n";
    return true;
}

void TriangleSplattingPass::executeTwoPassRendering(vk::CommandBuffer cmd) {
    if (!config_.enableTwoPassRendering || triangleCount_ == 0) {
        return;
    }
    
    std::cout << "[TriangleSplattingPass] Executing Two-Pass Rendering...\n";
    
    // ===== PRE: Clear buffers =====
    cmd.fillBuffer(pixelCountersBuffer_, 0, VK_WHOLE_SIZE, 0);
    cmd.fillBuffer(visibilityBuffer_, 0, VK_WHOLE_SIZE, 0xFFFFFFFFu);
    
    // Barrier: transfer → compute shader
    std::array<vk::BufferMemoryBarrier, 2> preBarriers{};
    preBarriers[0].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    preBarriers[0].dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
    preBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preBarriers[0].buffer = pixelCountersBuffer_;
    preBarriers[0].offset = 0;
    preBarriers[0].size = VK_WHOLE_SIZE;
    
    preBarriers[1].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    preBarriers[1].dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
    preBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    preBarriers[1].buffer = visibilityBuffer_;
    preBarriers[1].offset = 0;
    preBarriers[1].size = VK_WHOLE_SIZE;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        static_cast<uint32_t>(preBarriers.size()), preBarriers.data(),
        0, nullptr
    );
    
    // ===== PASS 1: Visibility (O(N)) =====
    executeVisibilityPass(cmd);
    
    // Barrier: visibility write → shading read
    std::array<vk::BufferMemoryBarrier, 2> midBarriers{};
    midBarriers[0].srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    midBarriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;
    midBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    midBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    midBarriers[0].buffer = visibilityBuffer_;
    midBarriers[0].offset = 0;
    midBarriers[0].size = VK_WHOLE_SIZE;
    
    midBarriers[1].srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    midBarriers[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
    midBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    midBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    midBarriers[1].buffer = pixelCountersBuffer_;
    midBarriers[1].offset = 0;
    midBarriers[1].size = VK_WHOLE_SIZE;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr,
        static_cast<uint32_t>(midBarriers.size()), midBarriers.data(),
        0, nullptr
    );
    
    // ===== PASS 2: Shading (O(M)) =====
    executeShadingPass(cmd);
}

void TriangleSplattingPass::executeVisibilityPass(vk::CommandBuffer cmd) {
    // Bind visibility pass pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, visibilityPassPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, visibilityPassPipelineLayout_, 0, 1, &visibilityPassDescriptorSet_, 0, nullptr);
    
    // Push constants
    TwoPassPushConstants pc{};
    pc.viewProj = pushConstants_.viewProj;
    pc.outputWidth = config_.outputWidth;
    pc.outputHeight = config_.outputHeight;
    pc.triangleCount = triangleCount_;
    pc.maxTrianglesPerPixel = config_.maxTrianglesPerPixel;
    pc.enableEarlyTermination = 0; // Not used in visibility pass
    pc.alphaThreshold = 0.0f;      // Not used in visibility pass
    pc.padding = 0;
    
    cmd.pushConstants(visibilityPassPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(TwoPassPushConstants), &pc);
    
    // Dispatch: triangle-parallel (256 threads per workgroup)
    uint32_t groupCount = (triangleCount_ + 255) / 256;
    cmd.dispatch(groupCount, 1, 1);
    
    std::cout << "[TriangleSplattingPass] Visibility Pass dispatched: " << groupCount << " workgroups\n";
}

void TriangleSplattingPass::executeShadingPass(vk::CommandBuffer cmd) {
    // Bind shading pass pipeline
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, shadingPassPipeline_);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, shadingPassPipelineLayout_, 0, 1, &shadingPassDescriptorSet_, 0, nullptr);
    
    // Push constants
    TwoPassPushConstants pc{};
    pc.viewProj = pushConstants_.viewProj;
    pc.outputWidth = config_.outputWidth;
    pc.outputHeight = config_.outputHeight;
    pc.triangleCount = triangleCount_;
    pc.maxTrianglesPerPixel = config_.maxTrianglesPerPixel;
    pc.enableEarlyTermination = config_.enableEarlyTermination ? 1 : 0;
    pc.alphaThreshold = config_.alphaThreshold;
    pc.padding = 0;
    
    cmd.pushConstants(shadingPassPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(TwoPassPushConstants), &pc);
    
    // Dispatch: pixel-parallel (16×16 threads per workgroup)
    uint32_t groupsX = (config_.outputWidth + 15) / 16;
    uint32_t groupsY = (config_.outputHeight + 15) / 16;
    cmd.dispatch(groupsX, groupsY, 1);
    
    std::cout << "[TriangleSplattingPass] Shading Pass dispatched: " << groupsX << "×" << groupsY << " workgroups\n";
}

} // namespace rendering
} // namespace spectraforge
