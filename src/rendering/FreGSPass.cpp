/**
 * @file FreGSPass.cpp
 * @brief Frequency-Encoded Gaussian Splatting pass implementation
 *
 * Implements analytic frequency-domain Gaussian splatting.
 * Each thread independently processes one pixel - no cross-thread communication.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/Rendering/RenderPass/FreGSPass.h"
#include "SpectraForge/Core/VulkanContext.h"
#include "SpectraForge/Core/VMAMemoryManager.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#ifdef SPECTRAFORGE_ENABLE_SHADERC
#include <shaderc/shaderc.hpp>
#endif
#include <cstring>

namespace spectraforge {
namespace rendering {

FreGSPass::FreGSPass(const FreGSPassConfig& config)
    : RenderPassBase("FreGSPass")
    , config_(config)
{
}

FreGSPass::~FreGSPass() {
    cleanup();
}

bool FreGSPass::initialize(const VulkanContext& context) {
    if (initialized_) {
        std::cerr << "FreGSPass already initialized\n";
        return true;
    }

    try {
        // Create output accumulator image
        if (!createOutputImage(context)) {
            throw std::runtime_error("Failed to create output image");
        }

        // Create Gaussian storage buffer
        if (!createGaussianBuffer(context)) {
            throw std::runtime_error("Failed to create Gaussian buffer");
        }

        // Load shader SPIR-V
        auto spirvCode = loadShaderSPIRV();
        if (spirvCode.empty()) {
            throw std::runtime_error("Failed to load GaussFreqSplat.comp.spv");
        }

        // Create descriptor set layout
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        
        // Bindings 0-3: Input subbands (readonly)
        for (uint32_t i = 0; i < 4; ++i) {
            bindings.push_back(vk::DescriptorSetLayoutBinding{
                i,
                vk::DescriptorType::eStorageImage,
                1,
                vk::ShaderStageFlagBits::eCompute
            });
        }
        
        // Binding 4: Output accumulator (read-write)
        bindings.push_back(vk::DescriptorSetLayoutBinding{
            4,
            vk::DescriptorType::eStorageImage,
            1,
            vk::ShaderStageFlagBits::eCompute
        });
        
        // Binding 5: Gaussian buffer (readonly)
        bindings.push_back(vk::DescriptorSetLayoutBinding{
            5,
            vk::DescriptorType::eStorageBuffer,
            1,
            vk::ShaderStageFlagBits::eCompute
        });

        descriptorSetLayout_ = createDescriptorSetLayout(context, bindings);

        // Create pipeline layout with push constants
        vk::PushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 96; // Unified push constants size

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        pipelineLayout_ = context.getDevice().createPipelineLayout(pipelineLayoutInfo);

        // Create compute pipeline
        pipeline_ = createComputePipeline(context, spirvCode, pipelineLayout_);

        // Create descriptor sets
        if (!createDescriptorSets(context)) {
            throw std::runtime_error("Failed to create descriptor sets");
        }

        // Initialize push constants
        pushConstants_.outputWidth = config_.outputWidth;
        pushConstants_.outputHeight = config_.outputHeight;
        pushConstants_.freqScale = config_.freqScale;
        pushConstants_.subbandLevel = config_.subbandLevel;
        pushConstants_.foveaRadius = config_.foveaRadius;
        pushConstants_.padding0 = 0; // std140 padding
        pushConstants_.foveaCenter = config_.foveaCenter;
        pushConstants_.maxGaussians = config_.maxGaussians;

        initialized_ = true;
        std::cout << "FreGSPass initialized successfully\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "FreGSPass initialization failed: " << e.what() << "\n";
        cleanup();
        return false;
    }
}

void FreGSPass::execute(vk::CommandBuffer commandBuffer, uint32_t frameIndex) {
    if (!initialized_) {
        throw std::runtime_error("FreGSPass not initialized");
    }

    if (!inputSubbands_) {
        throw std::runtime_error("Input subbands not set");
    }

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_);

    // Bind descriptor sets
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout_,
        0,
        1,
        &descriptorSets_[frameIndex % descriptorSets_.size()],
        0,
        nullptr
    );

    // Push constants
    commandBuffer.pushConstants(
        pipelineLayout_,
        vk::ShaderStageFlagBits::eCompute,
        0,
        sizeof(PushConstants),
        &pushConstants_
    );

    // Dispatch compute work
    // Workgroup size: 16×16 (from shader)
    // CRITICAL: All 256 threads (16×16) independently process their pixels
    uint32_t groupsX = (config_.outputWidth + 15) / 16;
    uint32_t groupsY = (config_.outputHeight + 15) / 16;
    
    commandBuffer.dispatch(groupsX, groupsY, 1);

    // Memory barrier
    vk::MemoryBarrier memoryBarrier;
    memoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags{},
        1, &memoryBarrier,
        0, nullptr,
        0, nullptr
    );

    statistics_.dispatchCount++;
}

void FreGSPass::cleanup() {
    if (!initialized_) {
        return;
    }

    vk::Device device = context_->getDevice();

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

    // Cleanup image view (image auto-cleanup via RAII)
    if (outputView_) {
        device.destroyImageView(outputView_);
        outputView_ = nullptr;
    }

    // VMA resources cleanup automatically via RAII
    vmaOutputImage_ = core::VMAImage();
    vmaGaussianBuffer_ = core::VMABuffer();

    initialized_ = false;
    
    std::cout << "FreGSPass cleaned up\n";
}

void FreGSPass::setInputSubbands(const WaveletSubbands& subbands) {
    inputSubbands_ = &subbands;
}

void FreGSPass::uploadGaussians(const std::vector<GaussianSplat>& gaussians) {
    if (gaussians.empty()) {
        std::cerr << "Warning: Uploading empty Gaussian array\n";
        return;
    }

    gaussianCount_ = static_cast<uint32_t>(gaussians.size());
    
    // Upload to VMA buffer (CPU_TO_GPU for frequent updates)
    if (vmaGaussianBuffer_.getBuffer()) {
        void* mappedData = vmaGaussianBuffer_.map();
        if (mappedData) {
            // Write header (count + padding for std140)
            uint32_t header[4] = { gaussianCount_, 0, 0, 0 };
            std::memcpy(mappedData, header, sizeof(header));
            
            // Write Gaussian data
            void* gaussData = static_cast<char*>(mappedData) + sizeof(header);
            size_t dataSize = gaussians.size() * sizeof(GaussianSplat);
            std::memcpy(gaussData, gaussians.data(), dataSize);
            
            vmaGaussianBuffer_.unmap();
            
            std::cout << "Uploaded " << gaussianCount_ << " Gaussians (" 
                     << (dataSize / 1024) << " KB)\n";
        }
    }
    
    // Update push constants
    pushConstants_.maxGaussians = std::min(gaussianCount_, config_.maxGaussians);
}

void FreGSPass::updateFoveation(glm::vec2 gazePoint, float radius) {
    pushConstants_.foveaCenter = gazePoint;
    pushConstants_.foveaRadius = radius;
}

bool FreGSPass::createOutputImage(const VulkanContext& context) {
    // Store context for cleanup
    context_ = &context;
    
    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = vk::Format::eR16G16B16A16Sfloat; // rgba16f
    imageInfo.extent = vk::Extent3D{config_.outputWidth, config_.outputHeight, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled |
                      vk::ImageUsageFlagBits::eTransferSrc;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    vk::Device device = context.getDevice();

    try {
        // Create image using VMA (GPU_ONLY for accumulator)
        auto& vma = core::VMAMemoryManager::getInstance();
        vmaOutputImage_ = vma.createImage(imageInfo, core::ResourceUsage::GPU_ONLY);
        
        // Get Vulkan image from VMA wrapper
        outputImage_ = vmaOutputImage_.getImage();
        std::cout << "[FreGSPass] output image handle=" << static_cast<const void*>(static_cast<VkImage>(outputImage_)) << "\n";

        // Transition UNDEFINED->GENERAL for storage usage
        {
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
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            barrier.oldLayout = vk::ImageLayout::eUndefined;
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
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags{},
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            cmd.end();
            vk::SubmitInfo si; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
            [[maybe_unused]] auto _res = context.getGraphicsQueue().submit(1, &si, nullptr);
            context.getGraphicsQueue().waitIdle();
            device.freeCommandBuffers(context.getCommandPool(), 1, &cmd);
        }

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

        outputView_ = device.createImageView(viewInfo);

        // Estimate memory
        size_t imageSize = config_.outputWidth * config_.outputHeight * 8; // rgba16f = 8 bytes
        std::cout << "FreGSPass: Created output accumulator (" 
                  << (imageSize / 1024 / 1024) << " MB GPU_ONLY)\n";

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to create output image: " << e.what() << "\n";
        return false;
    }
}

bool FreGSPass::createGaussianBuffer([[maybe_unused]] const VulkanContext& context) {
    // Create buffer for Gaussian splats
    // Structure: uint32_t count + padding + GaussianSplat array
    size_t bufferSize = sizeof(uint32_t) * 4 + // header with padding
                       sizeof(GaussianSplat) * config_.maxGaussians;

    try {
        // Create buffer using VMA (CPU_TO_GPU for frequent updates)
        auto& vma = core::VMAMemoryManager::getInstance();
        
        vmaGaussianBuffer_ = vma.createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
            core::ResourceUsage::CPU_TO_GPU  // Mapped, sequential write
        );
        
        // Get Vulkan buffer from VMA wrapper
        gaussianBuffer_ = vmaGaussianBuffer_.getBuffer();

        std::cout << "FreGSPass: Created Gaussian buffer (" 
                  << (bufferSize / 1024) << " KB CPU_TO_GPU)\n";

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to create Gaussian buffer: " << e.what() << "\n";
        return false;
    }
}

bool FreGSPass::createDescriptorSets(const VulkanContext& context) {
    vk::Device device = context.getDevice();

    // Create descriptor pool
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.push_back({vk::DescriptorType::eStorageImage, 5 * 3}); // 5 images × 3 frames
    poolSizes.push_back({vk::DescriptorType::eStorageBuffer, 1 * 3}); // 1 buffer × 3 frames

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.maxSets = 3;
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
        if (inputSubbands_) {
            imageInfos[0] = { {}, inputSubbands_->viewLL, vk::ImageLayout::eGeneral };
            imageInfos[1] = { {}, inputSubbands_->viewLH, vk::ImageLayout::eGeneral };
            imageInfos[2] = { {}, inputSubbands_->viewHL, vk::ImageLayout::eGeneral };
            imageInfos[3] = { {}, inputSubbands_->viewHH, vk::ImageLayout::eGeneral };
        } else {
            // Если саббенды не заданы, заполняем нулевыми view (валидный set, но execute упадёт ранее)
            imageInfos[0] = { {}, VK_NULL_HANDLE, vk::ImageLayout::eGeneral };
            imageInfos[1] = { {}, VK_NULL_HANDLE, vk::ImageLayout::eGeneral };
            imageInfos[2] = { {}, VK_NULL_HANDLE, vk::ImageLayout::eGeneral };
            imageInfos[3] = { {}, VK_NULL_HANDLE, vk::ImageLayout::eGeneral };
        }
        imageInfos[4] = { {}, outputView_, vk::ImageLayout::eGeneral };

        std::array<vk::WriteDescriptorSet, 6> writes{};
        for (uint32_t j = 0; j < 4; ++j) {
            writes[j] = { descriptorSets_[i], j, 0, 1, vk::DescriptorType::eStorageImage, &imageInfos[j] };
        }
        writes[4] = { descriptorSets_[i], 4, 0, 1, vk::DescriptorType::eStorageImage, &imageInfos[4] };

        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = gaussianBuffer_;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        writes[5] = { descriptorSets_[i], 5, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &bufferInfo };

        device.updateDescriptorSets(writes, {});
    }

    return true;
}

std::vector<uint32_t> FreGSPass::loadShaderSPIRV() const {
    // 1) Try precompiled SPIR-V first
    {
        std::vector<std::string> paths = {
            "shaders/GaussFreqSplat.comp.spv",
            "../shaders/GaussFreqSplat.comp.spv",
            "../../shaders/GaussFreqSplat.comp.spv",
            "build/shaders/GaussFreqSplat.comp.spv"
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

    // 2) Fallback: compile GLSL at runtime (optional)
#ifdef SPECTRAFORGE_ENABLE_SHADERC
    {
        std::vector<std::string> glslPaths = {
            "shaders/GaussFreqSplat.comp",
            "../shaders/GaussFreqSplat.comp",
            "../../shaders/GaussFreqSplat.comp"
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
                "GaussFreqSplat.comp",
                options
            );

            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                std::cerr << "GaussFreqSplat: shaderc compile error: " << result.GetErrorMessage() << "\n";
                continue;
            }

            std::vector<uint32_t> spirv(result.cbegin(), result.cend());
            std::cout << "Compiled GLSL to SPIR-V at runtime from: " << path
                      << " (" << spirv.size()*sizeof(uint32_t) << " bytes)\n";
            return spirv;
        }
    }
#endif

    std::cerr << "Failed to load or compile GaussFreqSplat.comp(.spv)\n";
    return {};
}

void FreGSPass::updateViewProjection(const glm::mat4& viewProj) {
    pushConstants_.viewProj = viewProj;
}

} // namespace rendering
} // namespace spectraforge

