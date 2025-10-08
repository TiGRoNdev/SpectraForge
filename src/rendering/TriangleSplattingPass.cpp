#include "SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h"
#include "SpectraForge/Rendering/FrameOutput.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cinttypes>
#include <SpectraForge/Rendering/Mesh3D.h>
#include <cmath>

// Конвертация half-float в float
inline float halfToFloat(uint16_t h) {
    uint32_t h_exp = (h & 0x7C00) >> 10;  // Экспонента (5 бит)
    uint32_t h_mant = h & 0x03FF;         // Мантисса (10 бит)

    if (h_exp == 0) {
        // Denormalized number
        if (h_mant == 0) return 0.0f;
        return std::ldexp(static_cast<float>(h_mant), -24);
    } else if (h_exp == 31) {
        // Infinity or NaN
        return (h_mant == 0) ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::quiet_NaN();
    } else {
        // Normalized number
        uint32_t f_exp = h_exp + 112;  // Смещение экспоненты для float
        uint32_t f_mant = h_mant << 13; // Смещение мантиссы для float
        uint32_t f = (h & 0x8000) << 16 | (f_exp << 23) | f_mant; // Знак + экспонента + мантисса
        // Используем memcpy для избежания strict-aliasing
        float result;
        std::memcpy(&result, &f, sizeof(float));
        return result;
    }
}

namespace spectraforge {
namespace rendering {

spectraforge::rendering::TriangleSplattingPass(const Config& config)
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
    pushConstants_.enableEarlyTermination = config_.enableEarlyTermination ? 1u : 0u;
    pushConstants_.alphaThreshold = config_.alphaThreshold;
    // Включаем тайловый биннинг, если буфер создан
    pushConstants_.enableTileBinning = (tileCullingBuffer_ != VK_NULL_HANDLE) ? 1u : 0u;
    
    initialized_ = true;
    std::cout << "[TriangleSplattingPass] Initialized successfully\n";
    
    return true;
}

void TriangleSplattingPass::cleanup() {
    if (!device_) return;
    
    // Wait for device idle before cleanup
    vkDeviceWaitIdle(device_);
    
    // ===== ИСПРАВЛЕНИЕ: Правильный порядок cleanup =====
    
    // 1. Destroy pipelines FIRST (зависят от layouts)
    if (pipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(pipeline_), nullptr);
        pipeline_ = nullptr;
    }
    
    if (bitonicSortPipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(bitonicSortPipeline_), nullptr);
        bitonicSortPipeline_ = nullptr;
    }
    
    if (depthKeyComputePipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(depthKeyComputePipeline_), nullptr);
        depthKeyComputePipeline_ = nullptr;
    }
    
    if (frustumCullingPipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(frustumCullingPipeline_), nullptr);
        frustumCullingPipeline_ = nullptr;
    }
    
    if (tileCullingPipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(tileCullingPipeline_), nullptr);
        tileCullingPipeline_ = nullptr;
    }
    
    if (indirectArgsPipeline_) {
        vkDestroyPipeline(device_, static_cast<VkPipeline>(indirectArgsPipeline_), nullptr);
        indirectArgsPipeline_ = nullptr;
    }
    
    // 2. Destroy pipeline layouts
    if (pipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(pipelineLayout_), nullptr);
        pipelineLayout_ = nullptr;
    }
    
    if (bitonicSortPipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(bitonicSortPipelineLayout_), nullptr);
        bitonicSortPipelineLayout_ = nullptr;
    }
    
    if (depthKeyComputePipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(depthKeyComputePipelineLayout_), nullptr);
        depthKeyComputePipelineLayout_ = nullptr;
    }
    
    if (frustumCullingPipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(frustumCullingPipelineLayout_), nullptr);
        frustumCullingPipelineLayout_ = nullptr;
    }
    
    if (tileCullingPipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(tileCullingPipelineLayout_), nullptr);
        tileCullingPipelineLayout_ = nullptr;
    }
    
    if (indirectArgsPipelineLayout_) {
        vkDestroyPipelineLayout(device_, static_cast<VkPipelineLayout>(indirectArgsPipelineLayout_), nullptr);
        indirectArgsPipelineLayout_ = nullptr;
    }
    
    // 3. Destroy shader modules
    if (computeShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(computeShader_), nullptr);
        computeShader_ = nullptr;
    }
    
    if (bitonicSortShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(bitonicSortShader_), nullptr);
        bitonicSortShader_ = nullptr;
    }
    
    if (depthKeyComputeShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(depthKeyComputeShader_), nullptr);
        depthKeyComputeShader_ = nullptr;
    }
    
    if (frustumCullingShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(frustumCullingShader_), nullptr);
        frustumCullingShader_ = nullptr;
    }
    
    if (tileCullingShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(tileCullingShader_), nullptr);
        tileCullingShader_ = nullptr;
    }
    
    if (indirectArgsShader_) {
        vkDestroyShaderModule(device_, static_cast<VkShaderModule>(indirectArgsShader_), nullptr);
        indirectArgsShader_ = nullptr;
    }
    
    // 4. Destroy descriptor pools (automatically frees descriptor sets)
    if (descriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(descriptorPool_), nullptr);
        descriptorPool_ = VK_NULL_HANDLE;
        // Descriptor sets are automatically freed when pool is destroyed
        descriptorSet_ = VK_NULL_HANDLE;
    }
    
    if (bitonicSortDescriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(bitonicSortDescriptorPool_), nullptr);
        bitonicSortDescriptorPool_ = nullptr;
    }
    
    if (depthKeyComputeDescriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(depthKeyComputeDescriptorPool_), nullptr);
        depthKeyComputeDescriptorPool_ = nullptr;
    }
    
    if (frustumCullingDescriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(frustumCullingDescriptorPool_), nullptr);
        frustumCullingDescriptorPool_ = nullptr;
    }
    
    if (tileCullingDescriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(tileCullingDescriptorPool_), nullptr);
        tileCullingDescriptorPool_ = nullptr;
    }
    
    if (indirectArgsDescriptorPool_) {
        vkDestroyDescriptorPool(device_, static_cast<VkDescriptorPool>(indirectArgsDescriptorPool_), nullptr);
        indirectArgsDescriptorPool_ = nullptr;
    }
    
    // 5. Destroy descriptor set layouts
    if (descriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(descriptorSetLayout_), nullptr);
        descriptorSetLayout_ = nullptr;
    }
    
    if (bitonicSortDescriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(bitonicSortDescriptorSetLayout_), nullptr);
        bitonicSortDescriptorSetLayout_ = nullptr;
    }
    
    if (depthKeyComputeDescriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(depthKeyComputeDescriptorSetLayout_), nullptr);
        depthKeyComputeDescriptorSetLayout_ = nullptr;
    }
    
    if (frustumCullingDescriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(frustumCullingDescriptorSetLayout_), nullptr);
        frustumCullingDescriptorSetLayout_ = nullptr;
    }
    
    if (tileCullingDescriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(tileCullingDescriptorSetLayout_), nullptr);
        tileCullingDescriptorSetLayout_ = nullptr;
    }
    
    if (indirectArgsDescriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(device_, static_cast<VkDescriptorSetLayout>(indirectArgsDescriptorSetLayout_), nullptr);
        indirectArgsDescriptorSetLayout_ = nullptr;
    }
    
    // 6. Destroy buffers and images (через VMA)
    if (triangleBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, triangleBuffer_, triangleBufferAllocation_);
        triangleBuffer_ = VK_NULL_HANDLE;
        triangleBufferAllocation_ = VK_NULL_HANDLE;
    }
    
    if (sortedIndicesBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(sortedIndicesBuffer_), sortedIndicesAllocation_);
        sortedIndicesBuffer_ = nullptr;
        sortedIndicesAllocation_ = nullptr;
    }
    
    if (depthKeysBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(depthKeysBuffer_), depthKeysAllocation_);
        depthKeysBuffer_ = nullptr;
        depthKeysAllocation_ = nullptr;
    }
    
    if (visibleIndicesBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(visibleIndicesBuffer_), visibleIndicesAllocation_);
        visibleIndicesBuffer_ = nullptr;
        visibleIndicesAllocation_ = nullptr;
    }
    
    if (tileCullingBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(tileCullingBuffer_), tileCullingAllocation_);
        tileCullingBuffer_ = nullptr;
        tileCullingAllocation_ = nullptr;
    }
    
    if (materialTexturesBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(materialTexturesBuffer_), materialTexturesAllocation_);
        materialTexturesBuffer_ = nullptr;
        materialTexturesAllocation_ = nullptr;
    }
    
    if (textureDataBuffer_ && allocator_) {
        vmaDestroyBuffer(allocator_, static_cast<VkBuffer>(textureDataBuffer_), textureDataAllocation_);
        textureDataBuffer_ = nullptr;
        textureDataAllocation_ = nullptr;
    }
    
    if (outputImage_ && allocator_) {
        vmaDestroyImage(allocator_, outputImage_, outputImageAllocation_);
        outputImage_ = VK_NULL_HANDLE;
        outputImageAllocation_ = VK_NULL_HANDLE;
    }
    
    if (outputImageView_) {
        vkDestroyImageView(device_, outputImageView_, nullptr);
        outputImageView_ = VK_NULL_HANDLE;
    }
    
    std::cout << "[TriangleSplattingPass] Cleanup completed successfully" << std::endl;

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
    
    // Create material textures buffer (SSBO) - для texture lookup
    vk::BufferCreateInfo materialTexturesBufferInfo;
    materialTexturesBufferInfo.size = sizeof(uint32_t) * 1024; // Max 1024 materials
    materialTexturesBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                      vk::BufferUsageFlagBits::eTransferDst;
    materialTexturesBufferInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo materialTexturesAllocInfo{};
    materialTexturesAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBuffer vkMaterialTexturesBuffer;
    VkBufferCreateInfo vkMaterialTexturesBufferInfo = static_cast<VkBufferCreateInfo>(materialTexturesBufferInfo);

    result = vmaCreateBuffer(
        allocator_,
        &vkMaterialTexturesBufferInfo,
        &materialTexturesAllocInfo,
        &vkMaterialTexturesBuffer,
        &materialTexturesAllocation_,
        nullptr
    );

    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create material textures buffer\n";
        return false;
    }

    materialTexturesBuffer_ = vk::Buffer(vkMaterialTexturesBuffer);

    // Create texture data buffer (SSBO) - для packed texture data
    vk::BufferCreateInfo textureDataBufferInfo2;
    textureDataBufferInfo2.size = sizeof(uint32_t) * 1024 * 1024; // Max 1M RGBA8 texels
    textureDataBufferInfo2.usage = vk::BufferUsageFlagBits::eStorageBuffer |
                                  vk::BufferUsageFlagBits::eTransferDst;
    textureDataBufferInfo2.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo textureDataAllocInfo{};
    textureDataAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBuffer vkTextureDataBuffer;
    VkBufferCreateInfo vkTextureDataBufferInfo = static_cast<VkBufferCreateInfo>(textureDataBufferInfo2);

    result = vmaCreateBuffer(
        allocator_,
        &vkTextureDataBufferInfo,
        &textureDataAllocInfo,
        &vkTextureDataBuffer,
        &textureDataAllocation_,
        nullptr
    );

    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingPass] Failed to create texture data buffer\n";
        return false;
    }

    textureDataBuffer_ = vk::Buffer(vkTextureDataBuffer);
    
    // Validate that essential buffers are created
    if (triangleBuffer_ == VK_NULL_HANDLE) {
        std::cerr << "[TriangleSplattingPass] ❌ Critical error: triangleBuffer_ is null!\n";
        return false;
    }

    if (sortedIndicesBuffer_ == VK_NULL_HANDLE) {
        std::cerr << "[TriangleSplattingPass] ❌ Critical error: sortedIndicesBuffer_ is null!\n";
        return false;
    }

    std::cout << "[TriangleSplattingPass] ✅ Buffers created successfully:\n";
    std::cout << "  - Triangle buffer: " << (triangleBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
    std::cout << "  - Sorted indices buffer: " << (sortedIndicesBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
    std::cout << "  - Depth keys buffer: " << (depthKeysBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
    std::cout << "  - Visible indices buffer: " << (visibleIndicesBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
    std::cout << "  - Tile culling buffer: " << (tileCullingBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "⚠️ Not yet created (created later)") << "\n";
    std::cout << "  - Material textures buffer: " << (materialTexturesBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
    std::cout << "  - Texture data buffer: " << (textureDataBuffer_ != VK_NULL_HANDLE ? "✅ OK" : "❌ FAILED") << "\n";
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

    // Check buffer states right after shader loading
    std::cout << "[TriangleSplattingPass] DEBUG: Buffer states after shader loading:\n";
    std::cout << "[TriangleSplattingPass] DEBUG: tileCullingBuffer_ = " << tileCullingBuffer_ << std::endl;
    std::cout << "[TriangleSplattingPass] DEBUG: materialTexturesBuffer_ = " << materialTexturesBuffer_ << std::endl;
    std::cout << "[TriangleSplattingPass] DEBUG: textureDataBuffer_ = " << textureDataBuffer_ << std::endl;

    return true;
}

bool TriangleSplattingPass::createDescriptorSets() {
    std::cout << "[TriangleSplattingPass] DEBUG: Entering createDescriptorSets()\n";

    // Descriptor set layout
    std::array<vk::DescriptorSetLayoutBinding, 6> bindings;
    
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

    // Binding 4: MaterialTextures buffer (SSBO) - для texture lookup
    bindings[4].binding = 4;
    bindings[4].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = vk::ShaderStageFlagBits::eCompute;

    // Binding 5: TextureData buffer (SSBO) - для packed texture data
    bindings[5].binding = 5;
    bindings[5].descriptorType = vk::DescriptorType::eStorageBuffer;
    bindings[5].descriptorCount = 1;
    bindings[5].stageFlags = vk::ShaderStageFlagBits::eCompute;

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    descriptorSetLayout_ = device_.createDescriptorSetLayout(layoutInfo);
    std::cout << "[TriangleSplattingPass] DEBUG: Descriptor set layout created\n";
    
    // Descriptor pool
    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].type = vk::DescriptorType::eStorageImage;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = vk::DescriptorType::eStorageBuffer;
    poolSizes[1].descriptorCount = 5;  // Triangle buffer + sorted indices + tile culling buffer + material textures + texture data
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    
    descriptorPool_ = device_.createDescriptorPool(poolInfo);
    std::cout << "[TriangleSplattingPass] DEBUG: Descriptor pool created\n";

    // Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;
    
    auto sets = device_.allocateDescriptorSets(allocInfo);
    descriptorSet_ = sets[0];
    std::cout << "[TriangleSplattingPass] DEBUG: Descriptor set allocated\n";
    
    // Update descriptor set
    std::array<vk::WriteDescriptorSet, 6> writes;
    
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

    // Create buffer info structures that will remain valid until updateDescriptorSets is called
    vk::DescriptorBufferInfo tileCullingBufferInfo;
    vk::DescriptorBufferInfo materialTexturesBufferInfo;
    vk::DescriptorBufferInfo textureDataBufferInfo;

    // Tile culling buffer (проверяем что буфер создан и корректен)
    if (tileCullingBuffer_ && tileCullingBuffer_ != VK_NULL_HANDLE) {
        tileCullingBufferInfo.buffer = tileCullingBuffer_;
        tileCullingBufferInfo.offset = 0;
        tileCullingBufferInfo.range = VK_WHOLE_SIZE;

        writes[3].dstSet = descriptorSet_;
        writes[3].dstBinding = 3;
        writes[3].dstArrayElement = 0;
        writes[3].descriptorCount = 1;
        writes[3].descriptorType = vk::DescriptorType::eStorageBuffer;
        writes[3].pBufferInfo = &tileCullingBufferInfo;
    }

    // Material textures buffer (проверяем что буфер создан и корректен)
    if (materialTexturesBuffer_ && materialTexturesBuffer_ != VK_NULL_HANDLE) {
        materialTexturesBufferInfo.buffer = materialTexturesBuffer_;
        materialTexturesBufferInfo.offset = 0;
        materialTexturesBufferInfo.range = VK_WHOLE_SIZE;

        writes[4].dstSet = descriptorSet_;
        writes[4].dstBinding = 4;
        writes[4].dstArrayElement = 0;
        writes[4].descriptorCount = 1;
        writes[4].descriptorType = vk::DescriptorType::eStorageBuffer;
        writes[4].pBufferInfo = &materialTexturesBufferInfo;
    }

    // Texture data buffer (проверяем что буфер создан и корректен)
    if (textureDataBuffer_ && textureDataBuffer_ != VK_NULL_HANDLE) {
        textureDataBufferInfo.buffer = textureDataBuffer_;
        textureDataBufferInfo.offset = 0;
        textureDataBufferInfo.range = VK_WHOLE_SIZE;

        writes[5].dstSet = descriptorSet_;
        writes[5].dstBinding = 5;
        writes[5].dstArrayElement = 0;
        writes[5].descriptorCount = 1;
        writes[5].descriptorType = vk::DescriptorType::eStorageBuffer;
        writes[5].pBufferInfo = &textureDataBufferInfo;
    }

    // Collect valid descriptor writes
    std::vector<vk::WriteDescriptorSet> validWrites;

    // Always include output image and triangle buffer
    std::cout << "[TriangleSplattingPass] DEBUG: Adding output image to valid writes\n";
    validWrites.push_back(writes[0]); // Output image
    std::cout << "[TriangleSplattingPass] DEBUG: Adding triangle buffer to valid writes\n";
    validWrites.push_back(writes[1]); // Triangle buffer
    std::cout << "[TriangleSplattingPass] DEBUG: Adding indices buffer to valid writes\n";
    validWrites.push_back(writes[2]); // Indices buffer

    // Check and validate each buffer before adding to descriptors
    std::cout << "[TriangleSplattingPass] DEBUG: Buffer validation before descriptor binding:\n";
    std::cout << "[TriangleSplattingPass] DEBUG: tileCullingBuffer_ = " << tileCullingBuffer_ << std::endl;
    std::cout << "[TriangleSplattingPass] DEBUG: materialTexturesBuffer_ = " << materialTexturesBuffer_ << std::endl;
    std::cout << "[TriangleSplattingPass] DEBUG: textureDataBuffer_ = " << textureDataBuffer_ << std::endl;

    // Add tile culling buffer if available and valid
    if (tileCullingBuffer_ && tileCullingBuffer_ != VK_NULL_HANDLE) {
        std::cout << "[TriangleSplattingPass] ✅ Adding tile culling buffer to descriptors\n";
        validWrites.push_back(writes[3]);
    } else {
        std::cout << "[TriangleSplattingPass] ⚠️ Skipping tile culling buffer (null or invalid)\n";
    }

    // Add material textures buffer if available and valid
    if (materialTexturesBuffer_ && materialTexturesBuffer_ != VK_NULL_HANDLE) {
        std::cout << "[TriangleSplattingPass] ✅ Adding material textures buffer to descriptors\n";
        validWrites.push_back(writes[4]);
    } else {
        std::cout << "[TriangleSplattingPass] ⚠️ Skipping material textures buffer (null or invalid)\n";
    }

    // Add texture data buffer if available and valid
    if (textureDataBuffer_ && textureDataBuffer_ != VK_NULL_HANDLE) {
        std::cout << "[TriangleSplattingPass] ✅ Adding texture data buffer to descriptors\n";
        validWrites.push_back(writes[5]);
    } else {
        std::cout << "[TriangleSplattingPass] ⚠️ Skipping texture data buffer (null or invalid)\n";
    }

    // Update descriptor sets with valid writes only
    if (!validWrites.empty()) {
        std::cout << "[TriangleSplattingPass] DEBUG: About to call updateDescriptorSets with " << validWrites.size() << " writes\n";
        for (size_t i = 0; i < validWrites.size(); ++i) {
            if (validWrites[i].descriptorType == vk::DescriptorType::eStorageImage && validWrites[i].pImageInfo != nullptr) {
                std::cout << "[TriangleSplattingPass] DEBUG: Write " << i << " - binding: " << validWrites[i].dstBinding
                          << ", image: " << validWrites[i].pImageInfo->imageView << std::endl;
            } else if (validWrites[i].descriptorType == vk::DescriptorType::eStorageBuffer && validWrites[i].pBufferInfo != nullptr) {
                std::cout << "[TriangleSplattingPass] DEBUG: Write " << i << " - binding: " << validWrites[i].dstBinding
                          << ", buffer: " << validWrites[i].pBufferInfo->buffer << std::endl;
            } else {
                std::cout << "[TriangleSplattingPass] DEBUG: Write " << i << " - binding: " << validWrites[i].dstBinding
                          << ", descriptorType: " << vk::to_string(validWrites[i].descriptorType)
                          << ", pBufferInfo: " << (validWrites[i].pBufferInfo ? "OK" : "NULL")
                          << ", pImageInfo: " << (validWrites[i].pImageInfo ? "OK" : "NULL") << std::endl;
            }
        }

        std::cout << "[TriangleSplattingPass] DEBUG: Calling updateDescriptorSets()...\n";
        try {
            device_.updateDescriptorSets(validWrites, nullptr);
        } catch (const std::exception& e) {
            std::cerr << "[TriangleSplattingPass] ❌ Exception in updateDescriptorSets: " << e.what() << std::endl;
            return false;
        }
        std::cout << "[TriangleSplattingPass] ✅ " << validWrites.size() << " descriptors bound successfully\n";
    } else {
        std::cerr << "[TriangleSplattingPass] ❌ No valid descriptors to bind!\n";
        return false;
    }

    std::cout << "[TriangleSplattingPass] Created descriptor sets\n";
    
    return true;
}

bool TriangleSplattingPass::createPipeline() {
    // Push constant range
    vk::PushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 96; // Unified push constants size
    
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

void TriangleSplattingPass::setDebugMode(uint32_t mode) {
    pushConstants_.debugMode = mode;
}

void TriangleSplattingPass::setLighting(const glm::vec3& lightDirection, float lightIntensity,
                                       const glm::vec3& ambientColor, float ambientIntensity) {
    lightDirection_ = glm::normalize(lightDirection);
    lightIntensity_ = lightIntensity;
    ambientColor_ = ambientColor;
    ambientIntensity_ = ambientIntensity;
}

void TriangleSplattingPass::setLightingEnabled(bool enabled) {
    enableLighting_ = enabled;
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
    // ===== КРИТИЧНА: Bounds checking и валидация =====
    if (triangles.empty()) {
        std::cerr << "[TriangleSplattingPass] ERROR: Empty triangle array!" << std::endl;
        triangleCount_ = 0;
        pushConstants_.triangleCount = 0;
        return;
    }

    if (triangles.size() > maxTriangles_) {
        std::cerr << "[TriangleSplattingPass] CRITICAL: Triangle count " << triangles.size()
                  << " exceeds buffer capacity " << maxTriangles_ << "!" << std::endl;
        triangleCount_ = maxTriangles_;
        std::cerr << "[TriangleSplattingPass] Using truncated count: " << maxTriangles_ << std::endl;
    } else {
        triangleCount_ = static_cast<uint32_t>(triangles.size());
    }

    // Validate triangle data
    uint32_t validTriangles = 0;
    const size_t limit = std::min(triangles.size(), static_cast<size_t>(maxTriangles_));
    for (size_t i = 0; i < limit; ++i) {
        const auto& tri = triangles[i];
        bool isValid = true;
        isValid &= std::isfinite(tri.v0.x) && std::isfinite(tri.v0.y) && std::isfinite(tri.v0.z);
        isValid &= std::isfinite(tri.v1.x) && std::isfinite(tri.v1.y) && std::isfinite(tri.v1.z);
        isValid &= std::isfinite(tri.v2.x) && std::isfinite(tri.v2.y) && std::isfinite(tri.v2.z);
        isValid &= std::isfinite(tri.color.r) && std::isfinite(tri.color.g) && std::isfinite(tri.color.b);
        isValid &= std::isfinite(tri.opacity) && tri.opacity >= 0.0f && tri.opacity <= 1.0f;
        isValid &= std::isfinite(tri.sigma) && tri.sigma > 0.0f;
        if (!isValid) {
            std::cerr << "[TriangleSplattingPass] WARNING: Invalid triangle " << i << " detected and skipped" << std::endl;
        } else {
            validTriangles++;
        }
    }

    if (validTriangles == 0) {
        std::cerr << "[TriangleSplattingPass] CRITICAL: No valid triangles found!" << std::endl;
        triangleCount_ = 0;
        pushConstants_.triangleCount = 0;
        return;
    }

    std::cout << "[TriangleSplattingPass] Uploading " << validTriangles << " valid triangles to GPU..." << std::endl;
    pushConstants_.triangleCount = validTriangles;
    
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

std::vector<spectraforge::rendering::Triangle> 
TriangleSplattingPass::convertMeshToTriangles(const SpectraForge::Rendering::Mesh3D& mesh, float sigma) {
    std::vector<Triangle> triangles;
    
    const auto& vertices = mesh.getVertices();
    const auto& indices = mesh.getIndices();
    
    // Convert each triangle face
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;
        
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];
        
        if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
            std::cerr << "⚠️  Invalid vertex index in mesh conversion\n";
            continue;
        }
        
        const auto& v0 = vertices[idx0];
        const auto& v1 = vertices[idx1];
        const auto& v2 = vertices[idx2];
        
        // Create triangle
        Triangle tri;
        tri.v0 = glm::vec3(v0.position.x, v0.position.y, v0.position.z);
        tri.v1 = glm::vec3(v1.position.x, v1.position.y, v1.position.z);
        tri.v2 = glm::vec3(v2.position.x, v2.position.y, v2.position.z);
        
        tri.texCoord0 = glm::vec2(v0.u, v0.v);
        tri.texCoord1 = glm::vec2(v1.u, v1.v);
        tri.texCoord2 = glm::vec2(v2.u, v2.v);
        
        // Average vertex colors
        glm::vec3 avgColor = (glm::vec3(v0.color.x, v0.color.y, v0.color.z) +
                             glm::vec3(v1.color.x, v1.color.y, v1.color.z) +
                             glm::vec3(v2.color.x, v2.color.y, v2.color.z)) / 3.0f;
        tri.color = avgColor;
        tri.opacity = 1.0f;
        tri.sigma = sigma;
        
        // Calculate face normal (average of vertex normals)
        glm::vec3 normal = (glm::vec3(v0.normal.x, v0.normal.y, v0.normal.z) +
                           glm::vec3(v1.normal.x, v1.normal.y, v1.normal.z) +
                           glm::vec3(v2.normal.x, v2.normal.y, v2.normal.z)) / 3.0f;
        tri.normal = glm::normalize(normal);
        
        tri.materialId = 0; // Default material
        
        triangles.push_back(tri);
    }
    
    return triangles;
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
        UnifiedPushConstants pushConstants;
        pushConstants.outputWidth = 0;
        pushConstants.outputHeight = 0;
        pushConstants.triangleCount = NUM_ELEMENTS; // reuse field
        pushConstants.enableEarlyTermination = 0;
        pushConstants.alphaThreshold = 0.0f;
        pushConstants.enableTileBinning = level; // encode 'h' here
        pushConstants.debugMode = 0;             // algorithm id (0 LOCAL_BMS)
        pushConstants.padding = 0;
        
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
        cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &pushConstants);
        
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
            UnifiedPushConstants pushConstants;
            pushConstants.outputWidth = 0;
            pushConstants.outputHeight = 0;
            pushConstants.triangleCount = NUM_ELEMENTS;
            pushConstants.enableEarlyTermination = 0;
            pushConstants.alphaThreshold = 0.0f;
            pushConstants.enableTileBinning = level; // h
            pushConstants.debugMode = 2;             // algorithm id (2 BIG_FLIP)
            pushConstants.padding = 0;
            
            cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
            cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &pushConstants);
            
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
            UnifiedPushConstants pushConstants;
            pushConstants.outputWidth = 0;
            pushConstants.outputHeight = 0;
            pushConstants.triangleCount = NUM_ELEMENTS;
            pushConstants.enableEarlyTermination = 0;
            pushConstants.alphaThreshold = 0.0f;
            pushConstants.padding = 0;
            
            if (hh <= BITONIC_BLOCK_SIZE) {
                // Local disperse
                pushConstants.enableTileBinning = hh; // h
                pushConstants.debugMode = 1;          // algorithm id (1 LOCAL_DISPERSE)
                
                cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
                cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &pushConstants);
                
                uint32_t groupCountX = std::max(1u, NUM_ELEMENTS / BITONIC_BLOCK_SIZE);
                cmd.dispatch(groupCountX, 1, 1);
                
                break; // Local disperse handles the rest
            } else {
                // Big disperse
                pushConstants.enableTileBinning = hh; // h
                pushConstants.debugMode = 3;          // algorithm id (3 BIG_DISPERSE)
                
                cmd.bindPipeline(vk::PipelineBindPoint::eCompute, bitonicSortPipeline_);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, bitonicSortPipelineLayout_, 0, 1, &bitonicSortDescriptorSet_, 0, nullptr);
                cmd.pushConstants(bitonicSortPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &pushConstants);
                
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
    cmd.pushConstants(pipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &pushConstants_);
    
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
    pushConstantRange.size = 96; // Unified push constants size
    
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
    pushConstantRange.size = 96; // Unified push constants size
    
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
    
    // Push constants (unified)
    UnifiedPushConstants dkpc = pushConstants_;
    (void)cameraPos;
    dkpc.triangleCount = triangleCount_;
    cmd.pushConstants(depthKeyComputePipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &dkpc);
    
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
    pushConstantRange.size = 96; // Unified push constants size
    
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
    UnifiedPushConstants fcpc = pushConstants_;
    fcpc.triangleCount = triangleCount_;
    cmd.pushConstants(frustumCullingPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &fcpc);
    
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
    pushConstantRange.size = 96; // Unified push constants size
    
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
    
    // Push constants (threads per group) — unified
    UnifiedPushConstants iapc = pushConstants_;
    iapc.outputWidth = 256; // encode threadsPerGroup
    iapc.outputHeight = std::max(1u, (triangleCount_ + 255) / 256); // encode maxWorkGroups
    cmd.pushConstants(indirectArgsPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &iapc);
    
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
    visPushConstantRange.size = 96;
    
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
    shadPushConstantRange.size = 96;
    
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
    UnifiedPushConstants vppc = pushConstants_;
    vppc.triangleCount = triangleCount_;
    cmd.pushConstants(visibilityPassPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &vppc);
    
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
    UnifiedPushConstants sppc = pushConstants_;
    sppc.triangleCount = triangleCount_;
    cmd.pushConstants(shadingPassPipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, 96, &sppc);
    
    // Dispatch: pixel-parallel (16×16 threads per workgroup)
    uint32_t groupsX = (config_.outputWidth + 15) / 16;
    uint32_t groupsY = (config_.outputHeight + 15) / 16;
    cmd.dispatch(groupsX, groupsY, 1);
    
    std::cout << "[TriangleSplattingPass] Shading Pass dispatched: " << groupsX << "×" << groupsY << " workgroups\n";
}

void TriangleSplattingPass::saveFrameToPPM(const std::string& filename) {
    if (!initialized_ || !outputImage_) {
        std::cerr << "[TriangleSplattingPass] Cannot save frame - not initialized!\n";
        return;
    }
    
    // Create staging buffer for image download (RGBA16F = 8 bytes per pixel)
    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = config_.outputWidth * config_.outputHeight * 8; // RGBA16F
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    
    VkBuffer stagingBufferVk;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    vmaCreateBuffer(allocator_, &stagingInfo, &allocInfo, &stagingBufferVk, &stagingAlloc, &stagingAllocInfo);
    
    vk::Buffer stagingBuffer(stagingBufferVk);
    
    // Create one-time command buffer
    vk::CommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.commandPool = commandPool_;
    cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdAllocInfo.commandBufferCount = 1;
    
    auto cmdBuffers = device_.allocateCommandBuffers(cmdAllocInfo);
    vk::CommandBuffer cmd = cmdBuffers[0];
    
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);
    
    // Transition image to TRANSFER_SRC_OPTIMAL
    vk::ImageMemoryBarrier toTransfer;
    toTransfer.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    toTransfer.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    toTransfer.oldLayout = vk::ImageLayout::eGeneral;
    toTransfer.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = outputImage_;
    toTransfer.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    toTransfer.subresourceRange.baseMipLevel = 0;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.baseArrayLayer = 0;
    toTransfer.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags{},
        0, nullptr, 0, nullptr, 1, &toTransfer
    );
    
    // Copy image to staging buffer
    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{config_.outputWidth, config_.outputHeight, 1};
    
    cmd.copyImageToBuffer(outputImage_, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, 1, &region);
    
    // Transition back to GENERAL
    vk::ImageMemoryBarrier toGeneral;
    toGeneral.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    toGeneral.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    toGeneral.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
    toGeneral.newLayout = vk::ImageLayout::eGeneral;
    toGeneral.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toGeneral.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toGeneral.image = outputImage_;
    toGeneral.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    toGeneral.subresourceRange.baseMipLevel = 0;
    toGeneral.subresourceRange.levelCount = 1;
    toGeneral.subresourceRange.baseArrayLayer = 0;
    toGeneral.subresourceRange.layerCount = 1;
    
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr, 0, nullptr, 1, &toGeneral
    );
    
    cmd.end();
    
    // Submit and wait
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    auto result = graphicsQueue_.submit(1, &submitInfo, nullptr);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit graphics queue");
    }
    graphicsQueue_.waitIdle();
    
    // Read data from staging buffer (already mapped)
    uint8_t* data = reinterpret_cast<uint8_t*>(stagingAllocInfo.pMappedData);
    
    // DEBUG: Сохраняем сырые данные для анализа
    std::string rawFilename = filename.substr(0, filename.find_last_of('.')) + "_raw.bin";
    std::ofstream rawFile(rawFilename, std::ios::binary);
    rawFile.write(reinterpret_cast<const char*>(data), config_.outputWidth * config_.outputHeight * 8);
    rawFile.close();
    std::cout << "[TriangleSplattingPass] DEBUG: Saved raw data to " << rawFilename << std::endl;

    // Write PPM file (P3 format - ASCII)
    std::ofstream file(filename);
    file << "P3\n" << config_.outputWidth << " " << config_.outputHeight << "\n255\n";

    for (uint32_t y = 0; y < config_.outputHeight; ++y) {
        for (uint32_t x = 0; x < config_.outputWidth; ++x) {
            uint32_t idx = (y * config_.outputWidth + x) * 8; // RGBA16F = 8 bytes per pixel
            // Конвертируем float16 в uint8 для PPM формата
            uint16_t r16 = *reinterpret_cast<uint16_t*>(&data[idx]);
            uint16_t g16 = *reinterpret_cast<uint16_t*>(&data[idx+2]);
            uint16_t b16 = *reinterpret_cast<uint16_t*>(&data[idx+4]);

            // DEBUG: Выводим сырые значения для диагностики
            if (x < 5 && y < 5) {
                std::cout << "[TriangleSplattingPass] DEBUG: Pixel (" << x << "," << y << ") raw: R=" << r16 << " G=" << g16 << " B=" << b16 << std::endl;
            }

            // Конвертируем half-float в 0-255 диапазон
            float r = halfToFloat(r16);
            float g = halfToFloat(g16);
            float b = halfToFloat(b16);

            // Clamp и конвертируем в 0-255
            int r8 = std::max(0, std::min(255, static_cast<int>(r * 255.0f)));
            int g8 = std::max(0, std::min(255, static_cast<int>(g * 255.0f)));
            int b8 = std::max(0, std::min(255, static_cast<int>(b * 255.0f)));

            file << r8 << " " << g8 << " " << b8 << " ";
        }
        file << "\n";
    }
    
    file.close();
    
    // Cleanup
    device_.freeCommandBuffers(commandPool_, 1, &cmd);
    vmaDestroyBuffer(allocator_, stagingBufferVk, stagingAlloc);
    
    std::cout << "[TriangleSplattingPass] ✅ Frame saved to " << filename << " (" << config_.outputWidth << "x" << config_.outputHeight << ")\n";
}

void TriangleSplattingPass::saveFrameToPNG(const std::string& filename) {
    if (!initialized_ || !outputImage_) {
        std::cerr << "[TriangleSplattingPass] Cannot save frame - not initialized!\n";
        return;
    }

    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = config_.outputWidth * config_.outputHeight * 8; // RGBA16F
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBufferVk;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    vmaCreateBuffer(allocator_, &stagingInfo, &allocInfo, &stagingBufferVk, &stagingAlloc, &stagingAllocInfo);
    vk::Buffer stagingBuffer(stagingBufferVk);

    vk::CommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.commandPool = commandPool_;
    cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdAllocInfo.commandBufferCount = 1;

    auto cmdBuffers = device_.allocateCommandBuffers(cmdAllocInfo);
    vk::CommandBuffer cmd = cmdBuffers[0];
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmd.begin(beginInfo);

    vk::ImageMemoryBarrier toTransfer;
    toTransfer.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    toTransfer.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    toTransfer.oldLayout = vk::ImageLayout::eGeneral;
    toTransfer.newLayout = vk::ImageLayout::eTransferSrcOptimal;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = outputImage_;
    toTransfer.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    toTransfer.subresourceRange.baseMipLevel = 0;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.baseArrayLayer = 0;
    toTransfer.subresourceRange.layerCount = 1;

    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags{},
        0, nullptr, 0, nullptr, 1, &toTransfer
    );

    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{config_.outputWidth, config_.outputHeight, 1};
    cmd.copyImageToBuffer(outputImage_, vk::ImageLayout::eTransferSrcOptimal, stagingBuffer, 1, &region);

    vk::ImageMemoryBarrier toGeneral;
    toGeneral.srcAccessMask = vk::AccessFlagBits::eTransferRead;
    toGeneral.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
    toGeneral.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
    toGeneral.newLayout = vk::ImageLayout::eGeneral;
    toGeneral.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toGeneral.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toGeneral.image = outputImage_;
    toGeneral.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    toGeneral.subresourceRange.baseMipLevel = 0;
    toGeneral.subresourceRange.levelCount = 1;
    toGeneral.subresourceRange.baseArrayLayer = 0;
    toGeneral.subresourceRange.layerCount = 1;
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        0, nullptr, 0, nullptr, 1, &toGeneral
    );
    cmd.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    auto result = graphicsQueue_.submit(1, &submitInfo, nullptr);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit graphics queue");
    }
    graphicsQueue_.waitIdle();

    uint8_t* data = reinterpret_cast<uint8_t*>(stagingAllocInfo.pMappedData);

    // Convert RGBA16F -> 8-bit RGBA buffer for PNG
    const uint32_t w = config_.outputWidth;
    const uint32_t h = config_.outputHeight;
    std::vector<uint8_t> rgba8;
    rgba8.resize(static_cast<size_t>(w) * h * 4);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint32_t idx16 = (y * w + x) * 8;
            uint16_t r16 = *reinterpret_cast<uint16_t*>(&data[idx16 + 0]);
            uint16_t g16 = *reinterpret_cast<uint16_t*>(&data[idx16 + 2]);
            uint16_t b16 = *reinterpret_cast<uint16_t*>(&data[idx16 + 4]);
            uint16_t a16 = *reinterpret_cast<uint16_t*>(&data[idx16 + 6]);

            float r = halfToFloat(r16);
            float g = halfToFloat(g16);
            float b = halfToFloat(b16);
            float a = halfToFloat(a16);

            auto to8 = [](float v) -> uint8_t {
                v = std::max(0.0f, std::min(1.0f, v));
                return static_cast<uint8_t>(v * 255.0f + 0.5f);
            };

            size_t idx8 = static_cast<size_t>(y) * w * 4 + x * 4;
            rgba8[idx8 + 0] = to8(r);
            rgba8[idx8 + 1] = to8(g);
            rgba8[idx8 + 2] = to8(b);
            rgba8[idx8 + 3] = to8(a);
        }
    }

    // Делегируем сохранение PNG в отдельную библиотеку вывода
    const bool ok = save_rgba8_to_png(filename, w, h, rgba8);
 
    device_.freeCommandBuffers(commandPool_, 1, &cmd);
    vmaDestroyBuffer(allocator_, stagingBufferVk, stagingAlloc);
 
    if (ok) {
        std::cout << "[TriangleSplattingPass] ✅ Frame saved to " << filename
                  << " (" << w << "x" << h << ")\n";
    } else {
        std::cerr << "[TriangleSplattingPass] ❌ Failed to save PNG to: " << filename << "\n";
    }
}

} // namespace rendering
} // namespace spectraforge
