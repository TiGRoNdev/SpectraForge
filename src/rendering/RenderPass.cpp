/**
 * @file RenderPass.cpp
 * @brief Base implementation for render passes
 *
 * Implements RAII resource management and common utilities for all passes.
 *
 * @author SpectraForge Core Team
 * @date 2025-10-02
 */

#include "SpectraForge/rendering/RenderPass.h"
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
    
    // Create shader module
    vk::ShaderModuleCreateInfo moduleInfo;
    moduleInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    moduleInfo.pCode = spirvCode.data();
    
    vk::ShaderModule shaderModule = device.createShaderModule(moduleInfo);
    
    // Create compute pipeline
    vk::PipelineShaderStageCreateInfo stageInfo;
    stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    stageInfo.module = shaderModule;
    stageInfo.pName = "main";
    
    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = layout;
    
    auto result = device.createComputePipeline(nullptr, pipelineInfo);
    
    // Cleanup shader module (no longer needed after pipeline creation)
    device.destroyShaderModule(shaderModule);
    
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create compute pipeline for " + name_);
    }
    
    return result.value;
}

vk::DescriptorSetLayout RenderPassBase::createDescriptorSetLayout(
    const VulkanContext& context,
    const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
{
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    return context.getDevice().createDescriptorSetLayout(layoutInfo);
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

