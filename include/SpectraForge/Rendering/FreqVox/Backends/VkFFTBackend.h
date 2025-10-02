/**
 * @file VkFFTBackend.h
 * @brief VkFFT backend for DCT-II transformations on GPU
 * 
 * Implements Discrete Cosine Transform Type-II according to FreqVox Renderer Math.md section 2.
 * Uses VkFFT library for Vulkan-accelerated DCT operations.
 * 
 * DCT-II forward transform:
 * M̃[u,v] = Σ(p,q) M[p,q] * cos(πu(2p+1)/2P) * cos(πv(2q+1)/2Q)
 * 
 * DCT-II inverse transform (IDCT):
 * M[p,q] = Σ(u,v) M̃[u,v] * cos(πu(2p+1)/2P) * cos(πv(2q+1)/2Q)
 */

#pragma once

#include "SpectraForge/Rendering/FreqVox/FrequencyShading.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

// VkFFT header-only library
#ifdef HYPERENGINE_USE_VKFFT
#define VKFFT_BACKEND 0  // Vulkan backend
#include <vkFFT.h>
#endif

namespace SpectraForge::Rendering::FreqVox::Backends {

/**
 * @brief VkFFT-based backend for DCT-II transformations
 * 
 * Реализует DCT-II на GPU через VkFFT с использованием Vulkan compute shaders.
 * Поддерживает батчированную обработку блоков размером P×Q (обычно 8×8).
 * 
 * @note Требует инициализированный Vulkan device и compute queue
 * @note Thread-safe при условии внешней синхронизации вызовов transform_*
 */
class VkFFTBackend : public IFrequencyBackend {
public:
    /**
     * @brief Конструктор с Vulkan контекстом
     * @param instance Vulkan instance (опционально, для детекции)
     * @param physicalDevice Vulkan физическое устройство (опционально)
     * @param device Vulkan логическое устройство (опционально)
     * 
     * Если параметры не указаны, будет попытка автоматического обнаружения.
     */
    explicit VkFFTBackend(
        vk::Instance instance = vk::Instance{},
        vk::PhysicalDevice physicalDevice = vk::PhysicalDevice{},
        vk::Device device = vk::Device{}
    );
    
    ~VkFFTBackend() override;

    /**
     * @brief Инициализация VkFFT с конфигурацией DCT-II
     * @param config Конфигурация блоков (blockSize, batchCount)
     * @return true если инициализация успешна
     * 
     * Создает VkFFT планы для DCT-II типа 2 согласно Math.md.
     * Выделяет Vulkan буферы для входных/выходных данных.
     */
    bool initialize(const DctBlockConfig& config) override;
    
    /**
     * @brief Освобождение ресурсов VkFFT и Vulkan
     */
    void shutdown() override;
    
    /**
     * @brief Прямое DCT-II преобразование
     * @param io_block_batched Батч данных [batch][blockSize²], изменяется in-place
     * @return true если преобразование выполнено успешно
     * 
     * Выполняет DCT-II согласно формуле из Math.md (строки 30-32):
     * M̃[u,v] = Σ M[p,q] * cos(πu(2p+1)/2P) * cos(πv(2q+1)/2Q)
     */
    bool transform_forward(std::vector<float>& io_block_batched) override;
    
    /**
     * @brief Обратное DCT-II преобразование (IDCT)
     * @param io_block_batched Батч данных в частотной области, изменяется in-place
     * @return true если преобразование выполнено успешно
     * 
     * Выполняет IDCT согласно Math.md (строки 39-42).
     */
    bool transform_inverse(std::vector<float>& io_block_batched) override;

    /**
     * @brief Проверка доступности VkFFT на текущем железе
     * @return true если VkFFT может быть использован
     */
    static bool isAvailable();

private:
    DctBlockConfig cfg_;
    bool initialized_ = false;
    
    // Vulkan контекст (C++ API)
    vk::Instance vulkan_instance_;
    vk::PhysicalDevice vulkan_physical_device_;
    vk::Device vulkan_device_;
    vk::Queue compute_queue_;
    uint32_t compute_queue_family_index_ = UINT32_MAX;
    
    // C API handles для VkFFT (должны жить столько же, сколько и класс)
    VkDevice vk_device_handle_ = VK_NULL_HANDLE;
    VkPhysicalDevice vk_physical_device_handle_ = VK_NULL_HANDLE;
    VkQueue vk_queue_handle_ = VK_NULL_HANDLE;
    VkCommandPool vk_command_pool_handle_ = VK_NULL_HANDLE;
    VkBuffer vk_buffer_handle_ = VK_NULL_HANDLE;
    
    // Vulkan ресурсы
    std::unique_ptr<vk::CommandPool> command_pool_;
    std::unique_ptr<vk::CommandBuffer> command_buffer_;
    std::unique_ptr<vk::Buffer> device_buffer_;  // GPU буфер для данных
    void* device_buffer_memory_ = nullptr;       // Mapped memory
    size_t buffer_size_bytes_ = 0;

#ifdef HYPERENGINE_USE_VKFFT
    // VkFFT структуры
    std::unique_ptr<VkFFTConfiguration> vkfft_config_;
    std::unique_ptr<VkFFTApplication> vkfft_app_forward_;   // DCT-II и IDCT план
#endif

    /**
     * @brief Инициализация Vulkan ресурсов (command pool, buffers)
     * @return true если успешно
     */
    bool initialize_vulkan_resources();
    
    /**
     * @brief Создание VkFFT планов для DCT-II
     * @return true если успешно
     */
    bool create_vkfft_plans();
    
    /**
     * @brief Освобождение VkFFT планов
     */
    void destroy_vkfft_plans();
    
    /**
     * @brief Выполнение VkFFT трансформации
     * @param is_forward true для DCT, false для IDCT
     * @param data Данные для обработки
     * @return true если успешно
     */
    bool execute_transform(bool is_forward, std::vector<float>& data);

    /**
     * @brief Поиск compute queue family
     * @return Индекс семейства или UINT32_MAX если не найдено
     */
    uint32_t find_compute_queue_family();

    // Запрет копирования (RAII)
    VkFFTBackend(const VkFFTBackend&) = delete;
    VkFFTBackend& operator=(const VkFFTBackend&) = delete;
};

} // namespace SpectraForge::Rendering::FreqVox::Backends

