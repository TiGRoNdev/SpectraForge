#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace spectraforge {
namespace rendering {

/**
 * @brief Общие типы и структуры для Triangle Splatting подсистемы
 */

/**
 * @brief Vulkan context для передачи между компонентами
 */
struct VulkanContext {
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
    VmaAllocator allocator;
    vk::Queue computeQueue;
    vk::Queue graphicsQueue;
    vk::CommandPool commandPool;
    
    VulkanContext()
        : device(nullptr)
        , physicalDevice(nullptr)
        , allocator(nullptr)
        , computeQueue(nullptr)
        , graphicsQueue(nullptr)
        , commandPool(nullptr)
    {}
};

/**
 * @brief Triangle primitive с параметрами для learnable rendering
 * КРИТИЧНО: Должен соответствовать GLSL layout (96 bytes)
 */
#pragma pack(push, 1)
struct Triangle {
    glm::vec3 v0, v1, v2;          // 36 bytes - вершины
    glm::vec3 color;               // 12 bytes - цвет
    float opacity;                 // 4 bytes - прозрачность
    float sigma;                   // 4 bytes - smoothness
    glm::vec3 normal;              // 12 bytes - нормаль
    int materialId;                // 4 bytes - material ID
    glm::vec2 texCoord0, texCoord1, texCoord2; // 24 bytes - UV coords
    
    Triangle()
        : v0(0.0f), v1(0.0f), v2(0.0f)
        , color(0.8f, 0.7f, 0.6f)
        , opacity(1.0f)
        , sigma(1.0f)
        , normal(0.0f, 1.0f, 0.0f)
        , materialId(0)
        , texCoord0(0.0f), texCoord1(0.0f), texCoord2(0.0f)
    {}
};
#pragma pack(pop)

static_assert(sizeof(Triangle) == 96, "Triangle must be EXACTLY 96 bytes!");

/**
 * @brief Конфигурация Triangle Splatting Pass
 */
struct TriangleSplattingConfig {
    uint32_t outputWidth = 1920;
    uint32_t outputHeight = 1080;
    bool enableDepthSort = true;
    bool enableEarlyTermination = true;
    float alphaThreshold = 0.99f;
    bool enableTwoPassRendering = false;
    uint32_t maxTrianglesPerPixel = 64;
};

} // namespace rendering
} // namespace spectraforge

