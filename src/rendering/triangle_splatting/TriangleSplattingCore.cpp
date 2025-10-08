/**
 * @file TriangleSplattingCore.cpp
 * @brief Реализация TriangleSplattingCore
 */

#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingCore.h>
#include <iostream>
#include <fstream>
#include <cstring>

namespace spectraforge {
namespace rendering {

TriangleSplattingCore::TriangleSplattingCore() {
    std::cout << "[TriangleSplattingCore] Created\n";
}

TriangleSplattingCore::~TriangleSplattingCore() {
    shutdown();
}

bool TriangleSplattingCore::initialize(const VulkanContext& context, const Config& config) {
    if (initialized_) {
        std::cerr << "[TriangleSplattingCore] Already initialized\n";
        return true;
    }
    
    // Валидация context
    if (!context.device) {
        std::cerr << "[TriangleSplattingCore] ❌ Invalid device\n";
        return false;
    }
    
    if (!context.allocator) {
        std::cerr << "[TriangleSplattingCore] ❌ Invalid allocator\n";
        return false;
    }
    
    context_ = context;
    config_ = config;
    
    std::cout << "[TriangleSplattingCore] Initializing with resolution " 
              << config_.outputWidth << "x" << config_.outputHeight << "\n";
    
    // Создание ресурсов в правильном порядке
    if (!createOutputImage()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to create output image\n";
        return false;
    }
    
    if (!createShaderModule()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to create shader module\n";
        return false;
    }
    
    if (!createDescriptorSets()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to create descriptor sets\n";
        return false;
    }
    
    if (!createPipeline()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to create pipeline\n";
        return false;
    }
    
    initialized_ = true;
    std::cout << "[TriangleSplattingCore] ✅ Initialized successfully\n";
    
    return true;
}

void TriangleSplattingCore::shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "[TriangleSplattingCore] Shutting down...\n";
    
    // Wait for device idle
    if (context_.device) {
        vkDeviceWaitIdle(context_.device);
    }
    
    // Destroy in reverse order of creation
    
    // 1. Pipeline
    if (pipeline_) {
        vkDestroyPipeline(context_.device, static_cast<VkPipeline>(pipeline_), nullptr);
        pipeline_ = nullptr;
    }
    
    // 2. Pipeline layout
    if (pipelineLayout_) {
        vkDestroyPipelineLayout(context_.device, static_cast<VkPipelineLayout>(pipelineLayout_), nullptr);
        pipelineLayout_ = nullptr;
    }
    
    // 3. Descriptor pool (frees descriptor sets automatically)
    if (descriptorPool_) {
        vkDestroyDescriptorPool(context_.device, static_cast<VkDescriptorPool>(descriptorPool_), nullptr);
        descriptorPool_ = nullptr;
        descriptorSet_ = nullptr; // Freed by pool
    }
    
    // 4. Descriptor set layout
    if (descriptorSetLayout_) {
        vkDestroyDescriptorSetLayout(context_.device, static_cast<VkDescriptorSetLayout>(descriptorSetLayout_), nullptr);
        descriptorSetLayout_ = nullptr;
    }
    
    // 5. Shader module
    if (computeShader_) {
        vkDestroyShaderModule(context_.device, static_cast<VkShaderModule>(computeShader_), nullptr);
        computeShader_ = nullptr;
    }
    
    // 6. Image view
    if (outputImageView_) {
        vkDestroyImageView(context_.device, static_cast<VkImageView>(outputImageView_), nullptr);
        outputImageView_ = nullptr;
    }
    
    // 7. Image + memory
    if (outputImage_ && context_.allocator) {
        vmaDestroyImage(context_.allocator, static_cast<VkImage>(outputImage_), outputImageAllocation_);
        outputImage_ = nullptr;
        outputImageAllocation_ = nullptr;
    }
    
    initialized_ = false;
    std::cout << "[TriangleSplattingCore] ✅ Shutdown complete\n";
}

// ============================================================================
// Private Implementation
// ============================================================================

bool TriangleSplattingCore::createOutputImage() {
    std::cout << "[TriangleSplattingCore] Creating output image...\n";
    
    // Image create info (RGBA16F format for HDR)
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT; // RGBA16F
    imageInfo.extent.width = config_.outputWidth;
    imageInfo.extent.height = config_.outputHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    // VMA allocation info
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    
    // Create image
    VkImage image;
    VkResult result = vmaCreateImage(context_.allocator, &imageInfo, &allocInfo, 
                                     &image, &outputImageAllocation_, nullptr);
    if (result != VK_SUCCESS) {
        std::cerr << "[TriangleSplattingCore] ❌ vmaCreateImage failed: " << result << "\n";
        return false;
    }
    
    outputImage_ = vk::Image(image);
    
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
    
    outputImageView_ = context_.device.createImageView(viewInfo);
    
    std::cout << "[TriangleSplattingCore] ✅ Output image created (" 
              << config_.outputWidth << "x" << config_.outputHeight << ")\n";
    
    return true;
}

bool TriangleSplattingCore::createShaderModule() {
    std::cout << "[TriangleSplattingCore] Loading compute shader...\n";
    
    // Загружаем скомпилированный SPIR-V шейдер
    auto shaderCode = loadShaderFile("shaders/TriangleSplatting.comp.spv");
    
    if (shaderCode.empty()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to load shader\n";
        return false;
    }
    
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    
    computeShader_ = context_.device.createShaderModule(createInfo);
    
    std::cout << "[TriangleSplattingCore] ✅ Shader loaded (" << shaderCode.size() << " bytes)\n";
    
    return true;
}

bool TriangleSplattingCore::createDescriptorSets() {
    std::cout << "[TriangleSplattingCore] Creating descriptor sets...\n";
    
    // Descriptor set layout
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    
    // Binding 0: Output image (storage image)
    vk::DescriptorSetLayoutBinding outputImageBinding;
    outputImageBinding.binding = 0;
    outputImageBinding.descriptorType = vk::DescriptorType::eStorageImage;
    outputImageBinding.descriptorCount = 1;
    outputImageBinding.stageFlags = vk::ShaderStageFlagBits::eCompute;
    bindings.push_back(outputImageBinding);
    
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    descriptorSetLayout_ = context_.device.createDescriptorSetLayout(layoutInfo);
    
    // Descriptor pool
    vk::DescriptorPoolSize poolSize;
    poolSize.type = vk::DescriptorType::eStorageImage;
    poolSize.descriptorCount = 1;
    
    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    descriptorPool_ = context_.device.createDescriptorPool(poolInfo);
    
    // Allocate descriptor set
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;
    
    auto sets = context_.device.allocateDescriptorSets(allocInfo);
    descriptorSet_ = sets[0];
    
    // Update descriptor set
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageView = outputImageView_;
    imageInfo.imageLayout = vk::ImageLayout::eGeneral;
    
    vk::WriteDescriptorSet write;
    write.dstSet = descriptorSet_;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = vk::DescriptorType::eStorageImage;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;
    
    context_.device.updateDescriptorSets(1, &write, 0, nullptr);
    
    std::cout << "[TriangleSplattingCore] ✅ Descriptor sets created\n";
    
    return true;
}

bool TriangleSplattingCore::createPipeline() {
    std::cout << "[TriangleSplattingCore] Creating compute pipeline...\n";
    
    // Pipeline layout (descriptor set layout + push constants)
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    
    pipelineLayout_ = context_.device.createPipelineLayout(pipelineLayoutInfo);
    
    // Compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = computeShader_;
    shaderStageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout_;
    
    auto result = context_.device.createComputePipeline(nullptr, pipelineInfo);
    
    if (result.result != vk::Result::eSuccess) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to create pipeline\n";
        return false;
    }
    
    pipeline_ = result.value;
    
    std::cout << "[TriangleSplattingCore] ✅ Pipeline created\n";
    
    return true;
}

std::vector<char> TriangleSplattingCore::loadShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "[TriangleSplattingCore] ❌ Failed to open file: " << filename << "\n";
        return {};
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

} // namespace rendering
} // namespace spectraforge

