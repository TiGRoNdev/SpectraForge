/**
 * @file WaveletPass.cpp
 * @brief Wavelet Lifting Transform pass implementation
 *
 * Implements 2D Daubechies-4 lifting scheme with full Vulkan resource management.
 * Uses VMA for efficient memory allocation.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/rendering/WaveletPass.h"
#include "SpectraForge/core/VulkanContext.h"
#include <vulkan/vulkan.h>
#include "SpectraForge/core/VMAMemoryManager.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <shaderc/shaderc.hpp>

namespace spectraforge {
namespace rendering {

WaveletPass::WaveletPass(const WaveletPassConfig& config)
    : RenderPassBase("WaveletPass")
    , config_(config)
{
}

WaveletPass::~WaveletPass() {
    cleanup();
}

bool WaveletPass::initialize(const VulkanContext& context) {
    if (initialized_) {
        std::cerr << "WaveletPass already initialized\n";
        return true;
    }

    try {
        // Create output subband images (select format)
        if (!createSubbandImages(context)) {
            throw std::runtime_error("Failed to create subband images");
        }

        std::cout << "[WaveletPass] ensureInputImage..." << std::endl;
        // Ensure input image exists (dummy if not provided by caller),
        // and match input format to selected subband format
        ensureInputImage(context);
        std::cout << "[WaveletPass] ensureInputImage done" << std::endl;

        // Load shader SPIR-V
        std::cout << "[WaveletPass] loadShaderSPIRV..." << std::endl;
        auto spirvCode = loadShaderSPIRV();
        if (spirvCode.empty()) {
            throw std::runtime_error("Failed to load WaveletLifting.comp.spv");
        }
        std::cout << "[WaveletPass] loadShaderSPIRV done, size=" << (spirvCode.size()*4) << " bytes" << std::endl;

        // Create descriptor set layout
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        
        // Binding 0: Input image (readonly)
        bindings.push_back(vk::DescriptorSetLayoutBinding{
            0, // binding
            vk::DescriptorType::eStorageImage,
            1, // descriptorCount
            vk::ShaderStageFlagBits::eCompute
        });
        
        // Bindings 1-4: Output subbands (writeonly)
        for (uint32_t i = 1; i <= 4; ++i) {
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                i,
                vk::DescriptorType::eStorageImage,
                1,
                vk::ShaderStageFlagBits::eCompute
            });
        }

        std::cout << "[WaveletPass] createDescriptorSetLayout..." << std::endl;
        descriptorSetLayout_ = createDescriptorSetLayout(context, bindings);
        std::cout << "[WaveletPass] createDescriptorSetLayout done" << std::endl;

        // Create pipeline layout with push constants
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 96; // Unified push constants size

        // Create pipeline layout via C API to avoid dispatcher issues
        VkPipelineLayoutCreateInfo pli{};
        pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        VkDescriptorSetLayout dsl = static_cast<VkDescriptorSetLayout>(descriptorSetLayout_);
        pli.setLayoutCount = 1;
        pli.pSetLayouts = &dsl;
        VkPushConstantRange pcr{};
        pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pcr.offset = 0;
        pcr.size = 96; // Unified push constants size
        pli.pushConstantRangeCount = 1;
        pli.pPushConstantRanges = &pcr;
        std::cout << "[WaveletPass] createPipelineLayout..." << std::endl;
        VkPipelineLayout rawLayout = VK_NULL_HANDLE;
        VkResult rpl = vkCreatePipelineLayout(static_cast<VkDevice>(context.getDevice()), &pli, nullptr, &rawLayout);
        if (rpl != VK_SUCCESS) {
            throw std::runtime_error("vkCreatePipelineLayout failed: " + std::to_string(rpl));
        }
        pipelineLayout_ = vk::PipelineLayout(rawLayout);
        std::cout << "[WaveletPass] createPipelineLayout done" << std::endl;

        // Create compute pipeline
        std::cout << "[WaveletPass] createComputePipeline..." << std::endl;
        pipeline_ = createComputePipeline(context, spirvCode, pipelineLayout_);
        std::cout << "[WaveletPass] createComputePipeline done" << std::endl;

        // Create descriptor sets
        std::cout << "[WaveletPass] createDescriptorSets..." << std::endl;
        if (!createDescriptorSets(context)) {
            throw std::runtime_error("Failed to create descriptor sets");
        }
        std::cout << "[WaveletPass] createDescriptorSets done" << std::endl;

        // Initialize push constants
        pushConstants_.inputWidth = config_.inputWidth;
        pushConstants_.inputHeight = config_.inputHeight;
        pushConstants_.threshold = config_.threshold;
        pushConstants_.foveationLevel = config_.foveationLevel;

        initialized_ = true;
        std::cout << "WaveletPass initialized successfully\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "WaveletPass initialization failed: " << e.what() << "\n";
        cleanup();
        return false;
    }
}

void WaveletPass::execute(vk::CommandBuffer commandBuffer, uint32_t frameIndex) {
    if (!initialized_) {
        throw std::runtime_error("WaveletPass not initialized");
    }

    if (!inputImage_ || !inputView_) {
        throw std::runtime_error("Input image not set");
    }

    // Ensure required layouts before use in this command buffer
    {
        std::array<vk::ImageMemoryBarrier, 5> barriers{};
        // Input image: read-only by compute
        barriers[0].srcAccessMask = {};
        barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;
        barriers[0].oldLayout = vk::ImageLayout::eUndefined;
        barriers[0].newLayout = vk::ImageLayout::eGeneral;
        barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barriers[0].image = inputImage_;
        barriers[0].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barriers[0].subresourceRange.baseMipLevel = 0;
        barriers[0].subresourceRange.levelCount = 1;
        barriers[0].subresourceRange.baseArrayLayer = 0;
        barriers[0].subresourceRange.layerCount = 1;

        // Output images: write by compute
        const vk::Image outs[4] = { subbands_.imageLL, subbands_.imageLH, subbands_.imageHL, subbands_.imageHH };
        for (int i = 0; i < 4; ++i) {
            barriers[1 + i].srcAccessMask = {};
            barriers[1 + i].dstAccessMask = vk::AccessFlagBits::eShaderWrite;
            barriers[1 + i].oldLayout = vk::ImageLayout::eUndefined;
            barriers[1 + i].newLayout = vk::ImageLayout::eGeneral;
            barriers[1 + i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1 + i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[1 + i].image = outs[i];
            barriers[1 + i].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barriers[1 + i].subresourceRange.baseMipLevel = 0;
            barriers[1 + i].subresourceRange.levelCount = 1;
            barriers[1 + i].subresourceRange.baseArrayLayer = 0;
            barriers[1 + i].subresourceRange.layerCount = 1;
        }

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(barriers.size()), barriers.data()
        );
    }

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_);

    // Bind descriptor sets
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout_,
        0, // firstSet
        1, // descriptorSetCount
        &descriptorSets_[frameIndex % descriptorSets_.size()],
        0, // dynamicOffsetCount
        nullptr
    );

    // Push constants
    commandBuffer.pushConstants(
        pipelineLayout_,
        vk::ShaderStageFlagBits::eCompute,
        0, // offset
        sizeof(PushConstants),
        &pushConstants_
    );

    // Dispatch compute work
    // Workgroup size: 32×32 (from shader)
    uint32_t groupsX = (config_.inputWidth + 31) / 32;
    uint32_t groupsY = (config_.inputHeight + 31) / 32;
    
    commandBuffer.dispatch(groupsX, groupsY, 1);

    // Memory barrier: compute shader writes → next shader reads
    vk::MemoryBarrier memoryBarrier;
    memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader, // srcStage
        vk::PipelineStageFlagBits::eComputeShader, // dstStage
        vk::DependencyFlags{},
        1, &memoryBarrier,
        0, nullptr,
        0, nullptr
    );

    // Update statistics (for profiling)
    statistics_.dispatchCount++;
}

void WaveletPass::cleanup() {
    if (!initialized_) {
        return;
    }

    vk::Device device = context_->getDevice();

    // Cleanup in reverse order of creation
    if (pipeline_) {
        device.destroyPipeline(pipeline_);
        pipeline_ = nullptr;
    }

    if (pipelineLayout_) {
        device.destroyPipelineLayout(pipelineLayout_);
        pipelineLayout_ = nullptr;
    }

    if (descriptorPool_) {
        device.destroyDescriptorPool(descriptorPool_);
        descriptorPool_ = nullptr;
    }

    if (descriptorSetLayout_) {
        device.destroyDescriptorSetLayout(descriptorSetLayout_);
        descriptorSetLayout_ = nullptr;
    }

    // Cleanup VMA image views (images auto-cleanup via RAII)
    if (subbands_.viewLL) {
        device.destroyImageView(subbands_.viewLL);
        subbands_.viewLL = nullptr;
    }
    if (subbands_.viewLH) {
        device.destroyImageView(subbands_.viewLH);
        subbands_.viewLH = nullptr;
    }
    if (subbands_.viewHL) {
        device.destroyImageView(subbands_.viewHL);
        subbands_.viewHL = nullptr;
    }
    if (subbands_.viewHH) {
        device.destroyImageView(subbands_.viewHH);
        subbands_.viewHH = nullptr;
    }

    // VMA images cleanup automatically via RAII (VMAImage destructor)
    subbands_.vmaImageLL = core::VMAImage();
    subbands_.vmaImageLH = core::VMAImage();
    subbands_.vmaImageHL = core::VMAImage();
    subbands_.vmaImageHH = core::VMAImage();

    initialized_ = false;
    
    std::cout << "WaveletPass cleaned up\n";
}

void WaveletPass::setInputImage(vk::Image image, vk::ImageView view) {
    inputImage_ = image;
    inputView_ = view;
}

void WaveletPass::updateConfig(const WaveletPassConfig& config) {
    config_ = config;
    
    // Update push constants
    pushConstants_.inputWidth = config_.inputWidth;
    pushConstants_.inputHeight = config_.inputHeight;
    pushConstants_.threshold = config_.threshold;
    pushConstants_.foveationLevel = config_.foveationLevel;
}

bool WaveletPass::createSubbandImages(const VulkanContext& context) {
    // Store context for cleanup
    context_ = &context;
    // Sanity checks to avoid null handle crashes
    vk::PhysicalDevice physical = context.getPhysicalDevice();
    vk::Device device = context.getDevice();
    // Default dispatcher is initialized in VulkanContextImpl; use standard overloads
    std::cout << "[WaveletPass] createSubbandImages: enter" << std::endl;
    std::cout << "  instance=" << static_cast<const void*>(static_cast<VkInstance>(context.getInstance())) << std::endl;
    std::cout << "  physicalDevice=" << static_cast<const void*>(static_cast<VkPhysicalDevice>(physical)) << std::endl;
    std::cout << "  device=" << static_cast<const void*>(static_cast<VkDevice>(device)) << std::endl;
    if (!static_cast<VkPhysicalDevice>(physical)) {
        throw std::runtime_error("WaveletPass: PhysicalDevice is null");
    }
    if (!static_cast<VkDevice>(device)) {
        throw std::runtime_error("WaveletPass: Device is null");
    }
    
    // Output resolution: half of input
    uint32_t outWidth = config_.inputWidth / 2;
    uint32_t outHeight = config_.inputHeight / 2;

    // Ensure VMA is initialized (defensive for different init orders)
    auto& vmaMgr = core::VMAMemoryManager::getInstance();
    if (!vmaMgr.isInitialized()) {
        if (!vmaMgr.initialize(context.getInstance(), physical, device, VK_API_VERSION_1_3)) {
            throw std::runtime_error("WaveletPass: VMA initialization failed");
        }
    }

    // Create 4 subband images (LL, LH, HL, HH) with a storage-capable format
    // CRITICAL: Use TRANSIENT pool for short-lived resources (1-2 frames)
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    // Choose supported storage-capable format (prefer RG16F, then R16F, then R32F)
    std::cout << "[WaveletPass] Checking storage image format support..." << std::endl;
    auto isStorageSupported = [&](vk::Format fmt) -> bool {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties(static_cast<VkPhysicalDevice>(physical), static_cast<VkFormat>(fmt), &props);
        return (props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
    };
    vk::Format chosenFormat = vk::Format::eR16G16Sfloat;
    if (!isStorageSupported(chosenFormat)) {
        if (isStorageSupported(vk::Format::eR16Sfloat)) {
            chosenFormat = vk::Format::eR16Sfloat;
        } else if (isStorageSupported(vk::Format::eR32Sfloat)) {
            chosenFormat = vk::Format::eR32Sfloat;
        } else {
            throw std::runtime_error("WaveletPass: no suitable storage image format supported");
        }
    }
    std::cout << "[WaveletPass] Chosen subband format: " << static_cast<int>(chosenFormat) << std::endl;
    imageInfo.format = chosenFormat;
    subbandFormat_ = chosenFormat;
    imageInfo.extent = vk::Extent3D{outWidth, outHeight, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    // Only storage + transfer; sampling не требуется для compute-only пайплайна
    imageInfo.usage = vk::ImageUsageFlagBits::eStorage |
                      vk::ImageUsageFlagBits::eTransferSrc |
                      vk::ImageUsageFlagBits::eTransferDst;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    // device already retrieved above

    try {
        // Create images using VMA transient pool (optimal for 1-2 frame lifetime)
        // VMA manager already obtained above as vmaMgr
        
        std::cout << "[WaveletPass] Creating transient images..." << std::endl;
        subbands_.vmaImageLL = vmaMgr.createTransientImage(imageInfo);
        std::cout << "  LL image ok" << std::endl;
        subbands_.vmaImageLH = vmaMgr.createTransientImage(imageInfo);
        std::cout << "  LH image ok" << std::endl;
        subbands_.vmaImageHL = vmaMgr.createTransientImage(imageInfo);
        std::cout << "  HL image ok" << std::endl;
        subbands_.vmaImageHH = vmaMgr.createTransientImage(imageInfo);
        std::cout << "  HH image ok" << std::endl;
        
        // Get Vulkan images from VMA wrappers
        subbands_.imageLL = subbands_.vmaImageLL.getImage();
        subbands_.imageLH = subbands_.vmaImageLH.getImage();
        subbands_.imageHL = subbands_.vmaImageHL.getImage();
        subbands_.imageHH = subbands_.vmaImageHH.getImage();
        std::cout << "  LL image handle=" << static_cast<const void*>(static_cast<VkImage>(subbands_.imageLL)) << std::endl;
        std::cout << "  LH image handle=" << static_cast<const void*>(static_cast<VkImage>(subbands_.imageLH)) << std::endl;
        std::cout << "  HL image handle=" << static_cast<const void*>(static_cast<VkImage>(subbands_.imageHL)) << std::endl;
        std::cout << "  HH image handle=" << static_cast<const void*>(static_cast<VkImage>(subbands_.imageHH)) << std::endl;

        // Initial layout transitions: UNDEFINED -> GENERAL for compute write/read
        {
            vk::Device device2 = context.getDevice();
            vk::CommandBufferAllocateInfo ainfo;
            ainfo.commandPool = context.getCommandPool();
            ainfo.level = vk::CommandBufferLevel::ePrimary;
            ainfo.commandBufferCount = 1;
            auto cmdBufs2 = device2.allocateCommandBuffers(ainfo);
            vk::CommandBuffer cmd2 = cmdBufs2[0];
            vk::CommandBufferBeginInfo binfo2;
            binfo2.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            cmd2.begin(binfo2);

            std::array<vk::Image,4> imgs = { subbands_.imageLL, subbands_.imageLH, subbands_.imageHL, subbands_.imageHH };
            for (vk::Image img : imgs) {
                vk::ImageMemoryBarrier barrier{};
                barrier.srcAccessMask = {};
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
                barrier.oldLayout = vk::ImageLayout::eUndefined;
                barrier.newLayout = vk::ImageLayout::eGeneral;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = img;
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;
                cmd2.pipelineBarrier(
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::PipelineStageFlagBits::eComputeShader,
                    vk::DependencyFlags{},
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );
            }
            cmd2.end();
            vk::SubmitInfo si2; si2.commandBufferCount = 1; si2.pCommandBuffers = &cmd2;
            [[maybe_unused]] auto _res = context.getGraphicsQueue().submit(1, &si2, nullptr);
            context.getGraphicsQueue().waitIdle();
            device2.freeCommandBuffers(context.getCommandPool(), 1, &cmd2);
        }

        // Create image views (C API to avoid dispatch loader issues)
        std::cout << "[WaveletPass] Creating image views..." << std::endl;
        auto makeView = [&](vk::Image image, vk::ImageView& out) {
            VkImageViewCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ci.image = static_cast<VkImage>(image);
            ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ci.format = static_cast<VkFormat>(chosenFormat);
            ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                              VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ci.subresourceRange.baseMipLevel = 0;
            ci.subresourceRange.levelCount = 1;
            ci.subresourceRange.baseArrayLayer = 0;
            ci.subresourceRange.layerCount = 1;
            VkImageView raw = VK_NULL_HANDLE;
            VkResult r = vkCreateImageView(static_cast<VkDevice>(device), &ci, nullptr, &raw);
            if (r != VK_SUCCESS) {
                throw std::runtime_error("vkCreateImageView failed with code " + std::to_string(r));
            }
            out = vk::ImageView(raw);
        };

        try { makeView(subbands_.imageLL, subbands_.viewLL); std::cout << "  LL view ok" << std::endl; }
        catch (const std::exception& e) { std::cerr << "WaveletPass: createImageView LL failed: " << e.what() << "\n"; throw; }
        try { makeView(subbands_.imageLH, subbands_.viewLH); std::cout << "  LH view ok" << std::endl; }
        catch (const std::exception& e) { std::cerr << "WaveletPass: createImageView LH failed: " << e.what() << "\n"; throw; }
        try { makeView(subbands_.imageHL, subbands_.viewHL); std::cout << "  HL view ok" << std::endl; }
        catch (const std::exception& e) { std::cerr << "WaveletPass: createImageView HL failed: " << e.what() << "\n"; throw; }
        try { makeView(subbands_.imageHH, subbands_.viewHH); std::cout << "  HH view ok" << std::endl; }
        catch (const std::exception& e) { std::cerr << "WaveletPass: createImageView HH failed: " << e.what() << "\n"; throw; }

        // Estimate memory usage
        // Rough size estimate by bytes-per-pixel based on chosen format
        size_t bpp = 4; // default for RG16F
        switch (chosenFormat) {
            case vk::Format::eR16G16Sfloat: bpp = 4; break;          // 2*16-bit
            case vk::Format::eR16G16B16A16Sfloat: bpp = 8; break;    // 4*16-bit
            case vk::Format::eR16Sfloat: bpp = 2; break;             // 16-bit
            case vk::Format::eR32Sfloat: bpp = 4; break;             // 32-bit
            default: bpp = 4; break;
        }
        size_t imageSize = outWidth * outHeight * bpp;
        size_t totalMemory = imageSize * 4; // 4 subbands
        
        std::cout << "WaveletPass: Created 4 transient subbands (" 
                  << (totalMemory / 1024 / 1024) << " MB from transient pool)\n";

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to create subband images: " << e.what() << "\n";
        return false;
    }
}

bool WaveletPass::createDescriptorSets(const VulkanContext& context) {
    vk::Device device = context.getDevice();

    // Create descriptor pool
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.push_back({vk::DescriptorType::eStorageImage, 5 * 3}); // 5 images × 3 frames

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 3; // Triple buffering
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    descriptorPool_ = device.createDescriptorPool(poolInfo);

    // Allocate descriptor sets
    std::vector<vk::DescriptorSetLayout> layouts(3, descriptorSetLayout_);
    
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 3;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets_ = device.allocateDescriptorSets(allocInfo);

    // Update descriptor sets
    for (size_t i = 0; i < descriptorSets_.size(); ++i) {
        std::array<vk::DescriptorImageInfo, 5> imageInfos{};
        imageInfos[0] = { {}, inputView_, vk::ImageLayout::eGeneral };
        imageInfos[1] = { {}, subbands_.viewLL, vk::ImageLayout::eGeneral };
        imageInfos[2] = { {}, subbands_.viewLH, vk::ImageLayout::eGeneral };
        imageInfos[3] = { {}, subbands_.viewHL, vk::ImageLayout::eGeneral };
        imageInfos[4] = { {}, subbands_.viewHH, vk::ImageLayout::eGeneral };

        std::array<vk::WriteDescriptorSet, 5> writes{};
        writes[0] = { descriptorSets_[i], 0, 0, 1, vk::DescriptorType::eStorageImage, &imageInfos[0] };
        for (uint32_t j = 1; j <= 4; ++j) {
            writes[j] = { descriptorSets_[i], j, 0, 1, vk::DescriptorType::eStorageImage, &imageInfos[j] };
        }

        device.updateDescriptorSets(writes, {});
    }

    return true;
}

std::vector<uint32_t> WaveletPass::loadShaderSPIRV() const {
    // 1) Try precompiled SPIR-V
    {
        std::vector<std::string> paths = {
            "shaders/WaveletLifting.comp.spv",
            "../shaders/WaveletLifting.comp.spv",
            "../../shaders/WaveletLifting.comp.spv",
            "build/shaders/WaveletLifting.comp.spv"
        };

        for (const auto& path : paths) {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                size_t fileSize = file.tellg();
                std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

                file.seekg(0);
                file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
                file.close();

                std::cout << "Loaded shader from: " << path << " (" << fileSize << " bytes)\n";
                return buffer;
            }
        }
    }

    // 2) Fallback: compile GLSL at runtime using shaderc
    std::vector<std::string> glslPaths = {
        "shaders/WaveletLifting.comp",
        "../shaders/WaveletLifting.comp",
        "../../shaders/WaveletLifting.comp"
    };

    for (const auto& path : glslPaths) {
        std::ifstream file(path);
        if (!file.is_open()) continue;

        std::stringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();
        file.close();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetSourceLanguage(shaderc_source_language_glsl);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        auto result = compiler.CompileGlslToSpv(
            source.c_str(), source.size(),
            shaderc_compute_shader,
            "WaveletLifting.comp",
            options
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << "WaveletLifting: shaderc compile error: " << result.GetErrorMessage() << "\n";
            continue;
        }

        std::vector<uint32_t> spirv(result.cbegin(), result.cend());
        std::cout << "Compiled GLSL to SPIR-V at runtime from: " << path
                  << " (" << spirv.size()*sizeof(uint32_t) << " bytes)\n";
        return spirv;
    }

    std::cerr << "Failed to load or compile WaveletLifting.comp(.spv)\n";
    return {};
}

void WaveletPass::ensureInputImage(const VulkanContext& context) {
    if (inputImage_ && inputView_) {
        return;
    }

    auto device = context.getDevice();
    std::cout << "[WaveletPass] ensureInputImage: begin" << std::endl;

    vk::Format fmt = subbandFormat_ != vk::Format::eUndefined
        ? subbandFormat_
        : vk::Format::eR16G16Sfloat;

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = fmt;
    imageInfo.extent = vk::Extent3D{std::max(1u, config_.inputWidth), std::max(1u, config_.inputHeight), 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    auto& vma = core::VMAMemoryManager::getInstance();
    vmaInputImage_ = vma.createImage(imageInfo, core::ResourceUsage::GPU_ONLY);
    inputImage_ = vmaInputImage_.getImage();

    // Create image view via C API to avoid dispatch issues
    VkImageViewCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.image = static_cast<VkImage>(inputImage_);
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = static_cast<VkFormat>(fmt);
    ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                      VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.subresourceRange.baseMipLevel = 0;
    ci.subresourceRange.levelCount = 1;
    ci.subresourceRange.baseArrayLayer = 0;
    ci.subresourceRange.layerCount = 1;
    VkImageView rawView = VK_NULL_HANDLE;
    VkResult r = vkCreateImageView(static_cast<VkDevice>(device), &ci, nullptr, &rawView);
    if (r != VK_SUCCESS) {
        throw std::runtime_error("ensureInputImage: vkCreateImageView failed: " + std::to_string(r));
    }
    inputView_ = vk::ImageView(rawView);
    std::cout << "[WaveletPass] ensureInputImage: view ok" << std::endl;

    // Initial transition UNDEFINED -> GENERAL for storage read
    {
        vk::Device device = context.getDevice();
        vk::CommandBufferAllocateInfo ainfo;
        ainfo.commandPool = context.getCommandPool();
        ainfo.level = vk::CommandBufferLevel::ePrimary;
        ainfo.commandBufferCount = 1;
        auto cmdBufs = device.allocateCommandBuffers(ainfo);
        vk::CommandBuffer cmd = cmdBufs[0];
        vk::CommandBufferBeginInfo binfo;
        binfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        cmd.begin(binfo);

        vk::ImageMemoryBarrier barrier{};
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eGeneral;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = inputImage_;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{},
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        cmd.end();
        vk::SubmitInfo si; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
        [[maybe_unused]] auto _res2 = context.getGraphicsQueue().submit(1, &si, nullptr);
        context.getGraphicsQueue().waitIdle();
        device.freeCommandBuffers(context.getCommandPool(), 1, &cmd);
    }
}

} // namespace rendering
} // namespace spectraforge

