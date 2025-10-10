/**
 * @file RenderPass.cpp
 * @brief Base implementation for render passes
 *
 * Implements RAII resource management and common utilities for all passes.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/Rendering/RenderPass/RenderPass.h"
#include "SpectraForge/Core/VulkanContext.h"
#include "SpectraForge/Core/SafeConsole.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

namespace spectraforge {
namespace rendering {

// ============================================================================
// RenderPassBase Implementation
// ============================================================================

RenderPassBase::RenderPassBase(std::string name)
    : name_(std::move(name))
    , initialized_(false)
{
}

RenderPassBase::~RenderPassBase() {
    // RAII: cleanup should be called by derived class
    if (initialized_) {
        std::cerr << "Warning: RenderPass '" << name_ 
                  << "' destroyed without calling cleanup()\n";
    }
}

vk::Pipeline RenderPassBase::createComputePipeline(
    const VulkanContext& context,
    const std::vector<uint32_t>& spirvCode,
    vk::PipelineLayout layout)
{
    vk::Device device = context.getDevice();
    VkDevice rawDev = static_cast<VkDevice>(device);

    // Create shader module (C API)
    VkShaderModuleCreateInfo moduleInfo{};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(spirvCode.data());
    VkShaderModule rawModule = VK_NULL_HANDLE;
    VkResult rm = vkCreateShaderModule(rawDev, &moduleInfo, nullptr, &rawModule);
    if (rm != VK_SUCCESS) {
        throw std::runtime_error("vkCreateShaderModule failed: " + SpectraForge::Core::SAFE_TO_STRING(rm));
    }

    // Create compute pipeline (C API)
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = rawModule;
    stageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = static_cast<VkPipelineLayout>(layout);

    VkPipeline rawPipe = VK_NULL_HANDLE;
    VkResult rp = vkCreateComputePipelines(rawDev, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rawPipe);
    // Destroy shader module
    vkDestroyShaderModule(rawDev, rawModule, nullptr);
    if (rp != VK_SUCCESS) {
        throw std::runtime_error("vkCreateComputePipelines failed: " + SpectraForge::Core::SAFE_TO_STRING(rp));
    }
    return vk::Pipeline(rawPipe);
}

vk::DescriptorSetLayout RenderPassBase::createDescriptorSetLayout(
    const VulkanContext& context,
    const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
{
    vk::Device device = context.getDevice();
    VkDevice rawDev = static_cast<VkDevice>(device);
    std::vector<VkDescriptorSetLayoutBinding> cBindings;
    cBindings.reserve(bindings.size());
    for (const auto& b : bindings) {
        VkDescriptorSetLayoutBinding cb{};
        cb.binding = b.binding;
        cb.descriptorType = static_cast<VkDescriptorType>(b.descriptorType);
        cb.descriptorCount = b.descriptorCount;
        cb.stageFlags = static_cast<VkShaderStageFlags>(b.stageFlags);
        cb.pImmutableSamplers = nullptr;
        cBindings.push_back(cb);
    }
    VkDescriptorSetLayoutCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = static_cast<uint32_t>(cBindings.size());
    ci.pBindings = cBindings.data();
    VkDescriptorSetLayout rawLayout = VK_NULL_HANDLE;
    VkResult r = vkCreateDescriptorSetLayout(rawDev, &ci, nullptr, &rawLayout);
    if (r != VK_SUCCESS) {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed: " + SpectraForge::Core::SAFE_TO_STRING(r));
    }
    return vk::DescriptorSetLayout(rawLayout);
}

void RenderPassBase::recordTimestamp(vk::CommandBuffer cmd, vk::PipelineStageFlagBits stage) {
    // TODO: Implement timestamp queries for profiling
    // Requires query pool setup in VulkanContext
    (void)cmd;
    (void)stage;
}

// ============================================================================
// RenderPassFactory Implementation
// ============================================================================

std::unique_ptr<IRenderPass> RenderPassFactory::createPass(PassType type) {
    switch (type) {
        case PassType::WAVELET_LIFTING:
            // Will be implemented in WaveletPass.cpp
            throw std::runtime_error("WaveletPass not yet implemented");
            
        case PassType::FREQUENCY_GAUSSIAN_SPLAT:
            // Will be implemented in FreGSPass.cpp
            throw std::runtime_error("FreGSPass not yet implemented");
            
        case PassType::FOVEATION_ALIGN:
            // TODO: Phase 3
            throw std::runtime_error("FoveationStage not yet implemented");
            
        case PassType::TEMPORAL_REPROJECTION:
            // TODO: Phase 3
            throw std::runtime_error("TemporalReprojection not yet implemented");
            
        case PassType::UPSCALING_DLSS:
            // TODO: Phase 3
            throw std::runtime_error("DLSSUpscaler not yet implemented");
            
        case PassType::UPSCALING_FSR2:
            // TODO: Phase 3
            throw std::runtime_error("FSR2Upscaler not yet implemented");
            
        default:
            throw std::invalid_argument("Unknown pass type");
    }
}

} // namespace rendering
} // namespace spectraforge

