#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <memory>

namespace spectraforge {
namespace rendering {

// Forward declarations
class TriangleBufferManager;

/**
 * @brief GPU-based frustum culling для треугольников
 * 
 * Ответственность: Отсеивание треугольников вне frustum на GPU с помощью compute shader.
 * Использует atomic counter для подсчета видимых треугольников.
 * 
 * SOLID Compliance:
 * - SRP: Только frustum culling (никакой сортировки/рендеринга)
 * - OCP: Можно расширить через наследование (например, OcclusionCullingPass)
 * - DIP: Зависит от абстракций (TriangleBufferManager interface)
 * 
 * Performance: O(N) parallel на GPU, workgroup size 256 threads
 */
class FrustumCullingPass {
public:
    /**
     * @brief Инициализация frustum culling pass
     * @param device Vulkan device
     * @param allocator VMA allocator
     * @param bufferManager Менеджер буферов (DI)
     * @param commandPool Command pool для создания одноразовых команд
     * @param queue Compute queue
     * @return true если успешно
     */
    bool initialize(vk::Device device,
                   VmaAllocator allocator,
                   const TriangleBufferManager& bufferManager,
                   vk::CommandPool commandPool,
                   vk::Queue queue);
    
    /**
     * @brief Очистка ресурсов
     */
    void cleanup();
    
    /**
     * @brief Выполнение frustum culling
     * @param cmd Command buffer для записи команд
     * @param viewProj View-Projection matrix для frustum test
     * @param triangleCount Количество треугольников для проверки
     */
    void execute(vk::CommandBuffer cmd,
                const glm::mat4& viewProj,
                uint32_t triangleCount);
    
    /**
     * @brief Получить количество видимых треугольников
     * @return Количество треугольников, прошедших frustum test
     * @note Требует GPU readback, может быть медленным (используйте спарсели)
     */
    uint32_t getVisibleCount() const;
    
    /**
     * @brief Получить буфер atomic counter
     * @return vk::Buffer с результатом culling
     */
    vk::Buffer getAtomicCounterBuffer() const { return atomicCounterBuffer_; }
    
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    
    vk::Device device_;
    VmaAllocator allocator_;
    vk::CommandPool commandPool_;
    vk::Queue queue_;
    
    // Shader и pipeline
    vk::ShaderModule frustumCullingShader_;
    vk::Pipeline frustumCullingPipeline_;
    vk::PipelineLayout frustumCullingPipelineLayout_;
    
    // Descriptor sets
    vk::DescriptorSetLayout frustumCullingDescriptorSetLayout_;
    vk::DescriptorPool frustumCullingDescriptorPool_;
    vk::DescriptorSet frustumCullingDescriptorSet_;
    
    // Atomic counter buffer (результат culling)
    vk::Buffer atomicCounterBuffer_;
    VmaAllocation atomicCounterAllocation_ = VK_NULL_HANDLE;
    
    // Readback buffer для CPU-side доступа
    vk::Buffer readbackBuffer_;
    VmaAllocation readbackAllocation_ = VK_NULL_HANDLE;
    
    // Helper functions
    bool loadShader(const std::string& filename);
    bool createPipeline();
    bool createDescriptorSets(const TriangleBufferManager& bufferManager);
    bool createAtomicCounterBuffer();
    void resetAtomicCounter(vk::CommandBuffer cmd);
    
    // Push constants structure
    struct PushConstants {
        glm::mat4 viewProj;
        uint32_t triangleCount;
    };
};

} // namespace rendering
} // namespace spectraforge

