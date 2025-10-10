#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <string>

namespace spectraforge {
namespace rendering {

/**
 * @brief Debug utilities для Triangle Splatting
 * 
 * Ответственность: Debug visualization, screenshot capture, wireframe mode
 * 
 * SOLID Compliance:
 * - SRP: Только debug функционал (никакого рендеринга)
 * - ISP: Не заставляет рендереры реализовывать debug методы
 */
class TriangleSplattingDebugger {
public:
    virtual ~TriangleSplattingDebugger() = default;

    /**
     * @brief Установка debug режима
     * @param mode 0=normal, 1=SDF visualization, 2=barycentric coordinates, 3=depth buffer
     */
    virtual void setDebugMode(uint32_t mode);
    
    /**
     * @brief Включение wireframe режима
     */
    virtual void enableWireframe(bool enable);
    
    /**
     * @brief Установка background color
     */
    virtual void setBackgroundColor(const glm::vec4& color);
    
    /**
     * @brief Сохранение кадра в PPM формат
     */
    virtual bool saveFrameToPPM(const std::string& filename,
                                vk::Image image,
                                vk::Device device,
                                VmaAllocator allocator,
                                vk::CommandPool commandPool,
                                vk::Queue queue,
                                uint32_t width,
                                uint32_t height);
    
    /**
     * @brief Сохранение кадра в PNG формат (требует libpng)
     */
    virtual bool saveFrameToPNG(const std::string& filename,
                                vk::Image image,
                                vk::Device device,
                                VmaAllocator allocator,
                                vk::CommandPool commandPool,
                                vk::Queue queue,
                                uint32_t width,
                                uint32_t height);
    
    // Getters
    virtual uint32_t getDebugMode() const { return debugMode_; }
    virtual bool isWireframeEnabled() const { return wireframeEnabled_; }
    virtual glm::vec4 getBackgroundColor() const { return backgroundColor_; }

private:
    uint32_t debugMode_ = 0;
    bool wireframeEnabled_ = false;
    glm::vec4 backgroundColor_ = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Helper для GPU->CPU transfer
    bool copyImageToBuffer(vk::Image image,
                          vk::Buffer buffer,
                          vk::Device device,
                          vk::CommandPool commandPool,
                          vk::Queue queue,
                          uint32_t width,
                          uint32_t height);
};

} // namespace rendering
} // namespace spectraforge

