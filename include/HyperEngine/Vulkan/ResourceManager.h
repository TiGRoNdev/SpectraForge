#pragma once

#if defined(BUILD_VULKAN_RENDERER) || defined(HyperEngine_ENABLE_VULKAN)
#include <vulkan/vulkan.hpp>
using namespace vk;
#else
#include <vulkan/vulkan_core.h>
#endif

#include <vk_mem_alloc.h>
#include <memory>
#include <unordered_map>

#if defined(VULKAN_RENDERER_CUDA_SUPPORT) && defined(CUDA_VULKAN_INTEROP_SUPPORTED)
#include <cuda.h>
#include <cuda_runtime.h>
#endif

namespace HyperEngine::Vulkan {

/**
 * @brief Данные изображения для создания текстур
 */
struct ImageData {
    uint32_t width;
    uint32_t height;
    uint32_t depth = 1;
#ifdef HyperEngine_ENABLE_VULKAN
    vk::Format format;
#else
    uint32_t format = 0;
#endif
    const void* data;
    size_t dataSize;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
};

#if defined(VULKAN_RENDERER_CUDA_SUPPORT) && defined(CUDA_VULKAN_INTEROP_SUPPORTED)
/**
 * @brief CUDA ресурс для interop
 */
struct CUDAResource {
    CUdeviceptr devicePtr;
    size_t size;
    CUmemorytype memoryType;
};
#endif

/**
 * @brief Менеджер ресурсов Vulkan
 *
 * Управляет памятью, буферами и текстурами с использованием VMA.
 * Поддерживает CUDA-Vulkan interop для гибридного рендеринга.
 */
class ResourceManager {
  public:
    /**
     * @brief Конструктор
     */
    ResourceManager();

    /**
     * @brief Деструктор
     */
    ~ResourceManager();

    /**
     * @brief Инициализация менеджера ресурсов
     * @param physicalDevice Vulkan физическое устройство
     * @param device Vulkan логическое устройство
     * @param instance Vulkan instance
     * @return true если инициализация успешна
     */
    bool init(
#ifdef HyperEngine_ENABLE_VULKAN
        vk::PhysicalDevice physicalDevice,
        vk::Device device,
        vk::Instance instance);
#else
        void* physicalDevice,
        void* device,
        void* instance);
#endif

    /**
     * @brief Завершение работы менеджера
     */
    void shutdown();

    /**
     * @brief Выделение буфера
     * @param size Размер буфера в байтах
     * @param usage Флаги использования буфера
     * @param memoryUsage Тип использования памяти
     * @return Vulkan буфер
     */
    vk::Buffer allocateBuffer(size_t size,
                              vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eVertexBuffer,
                              VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

    /**
     * @brief Создание текстуры
     * @param data Данные изображения
     * @return Vulkan изображение
     */
    vk::Image createTexture(const ImageData& data);

    /**
     * @brief Освобождение буфера
     * @param buffer Буфер для освобождения
     */
    void freeBuffer(vk::Buffer buffer);

    /**
     * @brief Освобождение изображения
     * @param image Изображение для освобождения
     */
    void freeImage(vk::Image image);

    /**
     * @brief Отображение памяти буфера
     * @param buffer Буфер для отображения
     * @return Указатель на отображенную память
     */
    void* mapBuffer(vk::Buffer buffer);

    /**
     * @brief Отмена отображения памяти буфера
     * @param buffer Буфер для отмены отображения
     */
    void unmapBuffer(vk::Buffer buffer);

    /**
     * @brief Копирование данных в буфер
     * @param buffer Целевой буфер
     * @param data Данные для копирования
     * @param size Размер данных
     * @param offset Смещение в буфере
     */
    void updateBuffer(vk::Buffer buffer, const void* data, size_t size, size_t offset = 0);

#if defined(VULKAN_RENDERER_CUDA_SUPPORT) && defined(CUDA_VULKAN_INTEROP_SUPPORTED)
    /**
     * @brief Управление CUDA-Vulkan interop
     * @param cudaRes CUDA ресурс
     * @return Vulkan external memory
     */
    vk::DeviceMemory manageInterop(const CUDAResource& cudaRes);

    /**
     * @brief Создание shared буфера для CUDA-Vulkan interop
     * @param size Размер буфера
     * @param usage Флаги использования
     * @return Shared буфер
     */
    vk::Buffer createSharedBuffer(size_t size,
#ifdef HyperEngine_ENABLE_VULKAN
                                  vk::BufferUsageFlags usage);
#else
                                  uint32_t usage);
#endif

    /**
     * @brief Экспорт Vulkan памяти для CUDA
     * @param memory Vulkan память
     * @return CUDA external memory handle
     */
    cudaExternalMemory_t exportMemoryToCUDA(vk::DeviceMemory memory);
#endif

    /**
     * @brief Получение статистики памяти
     * @return Статистика VMA
     */
    VmaTotalStatistics getMemoryStatistics() const;

    /**
     * @brief Получение VMA аллокатора
     * @return VMA аллокатор
     */
    VmaAllocator getAllocator() const { return allocator; }

    /**
     * @brief Поиск подходящего типа памяти
     * @param typeFilter Фильтр типов памяти
     * @param properties Требуемые свойства памяти
     * @return Индекс типа памяти
     */
    uint32_t findMemoryType(uint32_t typeFilter,
#ifdef HyperEngine_ENABLE_VULKAN
                            vk::MemoryPropertyFlags properties);
#else
                            uint32_t properties);
#endif

  private:
#ifdef HyperEngine_ENABLE_VULKAN
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Instance instance;
#else
    void* physicalDevice = nullptr;
    void* device = nullptr;
    void* instance = nullptr;
#endif
    VmaAllocator allocator = VK_NULL_HANDLE;

    // Кэш аллокаций для отслеживания
    struct AllocationInfo {
        VmaAllocation allocation;
        VmaAllocationInfo info;
        size_t size;
    };

    std::unordered_map<VkBuffer, AllocationInfo> bufferAllocations;
    std::unordered_map<VkImage, AllocationInfo> imageAllocations;

    bool initialized = false;

    /**
     * @brief Создание VMA аллокатора
     * @return true если создание успешно
     */
    bool createAllocator();

    /**
     * @brief Создание command buffer для копирования
     * @return Command buffer
     */
    vk::CommandBuffer beginSingleTimeCommands();

    /**
     * @brief Завершение single-time command buffer
     * @param commandBuffer Command buffer для завершения
     */
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    // Запрет копирования
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};

}  // namespace HyperEngine::Vulkan
