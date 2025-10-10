#pragma once

/**
 * @file TriangleSplattingCore.h
 * @brief Основной класс для Vulkan initialization и resource management
 * 
 * SRP: Отвечает ТОЛЬКО за Vulkan ресурсы и их жизненный цикл
 * ~200 строк (vs 3394 в монолите)
 */

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingTypes.h>

namespace spectraforge {
namespace rendering {

/**
 * @brief Core класс для управления Vulkan ресурсами Triangle Splatting
 * 
 * Ответственности:
 * - Vulkan initialization
 * - Output image creation
 * - Shader module loading
 * - Pipeline creation
 * - Descriptor set creation
 * - Resource cleanup
 */
class TriangleSplattingCore {
public:
    using Config = TriangleSplattingConfig;

    TriangleSplattingCore();
    virtual ~TriangleSplattingCore();
    
    // Disable copy
    TriangleSplattingCore(const TriangleSplattingCore&) = delete;
    TriangleSplattingCore& operator=(const TriangleSplattingCore&) = delete;
    
    /**
     * @brief Инициализация Vulkan ресурсов
     * @param context Vulkan context (device, allocator, queues)
     * @param config Конфигурация
     * @return true если успешно
     */
    virtual bool initialize(const VulkanContext& context, const Config& config);
    
    /**
     * @brief Освобождение всех Vulkan ресурсов
     */
    virtual void shutdown();
    
    /**
     * @brief Проверка инициализации
     */
    virtual bool isInitialized() const { return initialized_; }
    
    // ========================================================================
    // Getters для Vulkan ресурсов
    // ========================================================================
    
    virtual vk::Device getDevice() const { return context_.device; }
    virtual VmaAllocator getAllocator() const { return context_.allocator; }

    virtual vk::Image getOutputImage() const { return outputImage_; }
    virtual vk::ImageView getOutputImageView() const { return outputImageView_; }
    virtual vk::Extent2D getOutputImageExtent() const {
        return vk::Extent2D(config_.outputWidth, config_.outputHeight);
    }

    virtual vk::Pipeline getPipeline() const { return pipeline_; }
    virtual vk::PipelineLayout getPipelineLayout() const { return pipelineLayout_; }
    virtual vk::DescriptorSet getDescriptorSet() const { return descriptorSet_; }
    
    // ========================================================================
    // Проверка состояния ресурсов
    // ========================================================================
    
    virtual bool hasShaderModules() const { return computeShader_ != vk::ShaderModule(nullptr); }
    virtual bool hasPipelines() const { return pipeline_ != vk::Pipeline(nullptr); }
    virtual bool hasDescriptorSets() const { return descriptorSet_ != vk::DescriptorSet(nullptr); }
    
private:
    // ========================================================================
    // Private methods для создания ресурсов
    // ========================================================================
    
    bool createOutputImage();
    bool createShaderModule();
    bool createPipeline();
    bool createDescriptorSets();
    
    std::vector<char> loadShaderFile(const std::string& filename);
    
private:
    // Configuration
    Config config_;
    VulkanContext context_;
    bool initialized_ = false;
    
    // Output image (RGBA16F framebuffer)
    vk::Image outputImage_;
    VmaAllocation outputImageAllocation_;
    vk::ImageView outputImageView_;
    
    // Shader и pipeline
    vk::ShaderModule computeShader_;
    vk::Pipeline pipeline_;
    vk::PipelineLayout pipelineLayout_;
    
    // Descriptor sets
    vk::DescriptorSetLayout descriptorSetLayout_;
    vk::DescriptorPool descriptorPool_;
    vk::DescriptorSet descriptorSet_;
};

} // namespace rendering
} // namespace spectraforge

