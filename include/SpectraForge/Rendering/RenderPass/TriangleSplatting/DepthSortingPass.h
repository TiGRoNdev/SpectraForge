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
 * @brief GPU-based depth sorting для треугольников
 * 
 * Ответственность: Сортировка видимых треугольников по глубине (back-to-front)
 * для правильного alpha blending.
 * 
 * Поддерживает 2 алгоритма:
 * 1. Bitonic Sort - O(N log N), стандартный GPU sort
 * 2. Atomic Binning - O(N), оптимизирован для мобильных GPU
 * 
 * SOLID Compliance:
 * - SRP: Только сортировка по глубине
 * - OCP: Можно добавить новые алгоритмы сортировки
 * - LSP: BitonicSort и AtomicBinning взаимозаменяемы
 */
class DepthSortingPass {
public:
    /**
     * @brief Режимы сортировки
     */
    enum class SortMode {
        BitonicSort,    ///< O(N log N) - standard GPU sort, универсальный
        AtomicBinning   ///< O(N) - optimized for mobile GPUs, быстрее на больших N
    };
    
    /**
     * @brief Инициализация depth sorting pass
     * @param device Vulkan device
     * @param allocator VMA allocator
     * @param bufferManager Менеджер буферов
     * @param commandPool Command pool
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
     * @brief Выполнение сортировки по глубине
     * @param cmd Command buffer
     * @param cameraPos Позиция камеры для вычисления глубины
     * @param visibleTriangleCount Количество видимых треугольников
     */
    void execute(vk::CommandBuffer cmd,
                const glm::vec3& cameraPos,
                uint32_t visibleTriangleCount);
    
    /**
     * @brief Установка режима сортировки
     * @param mode BitonicSort или AtomicBinning
     */
    void setSortMode(SortMode mode) { sortMode_ = mode; }
    
    /**
     * @brief Получить текущий режим сортировки
     */
    SortMode getSortMode() const { return sortMode_; }
    
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    SortMode sortMode_ = SortMode::AtomicBinning; // Default: faster
    
    vk::Device device_;
    VmaAllocator allocator_;
    vk::CommandPool commandPool_;
    vk::Queue queue_;
    
    // === Bitonic Sort Resources ===
    vk::ShaderModule bitonicSortShader_;
    vk::Pipeline bitonicSortPipeline_;
    vk::PipelineLayout bitonicSortPipelineLayout_;
    vk::DescriptorSetLayout bitonicSortDescriptorSetLayout_;
    vk::DescriptorPool bitonicSortDescriptorPool_;
    vk::DescriptorSet bitonicSortDescriptorSet_;
    
    // === Depth Key Compute Resources ===
    vk::ShaderModule depthKeyComputeShader_;
    vk::Pipeline depthKeyComputePipeline_;
    vk::PipelineLayout depthKeyComputePipelineLayout_;
    vk::DescriptorSetLayout depthKeyComputeDescriptorSetLayout_;
    vk::DescriptorPool depthKeyComputeDescriptorPool_;
    vk::DescriptorSet depthKeyComputeDescriptorSet_;
    
    // === Atomic Binning Resources ===
    vk::ShaderModule atomicSortShader_;
    vk::Pipeline atomicSortPipeline_;
    vk::PipelineLayout atomicSortPipelineLayout_;
    vk::DescriptorSetLayout atomicSortDescriptorSetLayout_;
    vk::DescriptorPool atomicSortDescriptorPool_;
    vk::DescriptorSet atomicSortDescriptorSet_;
    
    // Helper functions
    bool loadShaders();
    bool createBitonicSortPipeline();
    bool createDepthKeyComputePipeline();
    bool createAtomicSortPipeline();
    bool createDescriptorSets(const TriangleBufferManager& bufferManager);
    
    void computeDepthKeys(vk::CommandBuffer cmd, const glm::vec3& cameraPos, uint32_t triangleCount);
    void sortTrianglesBitonic(vk::CommandBuffer cmd, uint32_t triangleCount);
    void sortTrianglesAtomic(vk::CommandBuffer cmd, uint32_t triangleCount);
    
    // Push constants
    struct DepthKeyPushConstants {
        glm::vec3 cameraPos;
        uint32_t triangleCount;
    };
    
    struct BitonicSortPushConstants {
        uint32_t stage;
        uint32_t passOfStage;
        uint32_t triangleCount;
    };
    
    struct AtomicSortPushConstants {
        uint32_t triangleCount;
        uint32_t phase; // 0=count, 1=prefix_sum, 2=placement
    };
};

} // namespace rendering
} // namespace spectraforge

