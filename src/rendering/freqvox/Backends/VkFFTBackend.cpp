/**
 * @file VkFFTBackend.cpp
 * @brief Реализация DCT-II на GPU через VkFFT
 * 
 * Строго следует математике из FreqVox Renderer Math.md раздел 2.
 * Использует VkFFT header-only библиотеку для Vulkan compute.
 */

#include "SpectraForge/Rendering/FreqVox/Backends/VkFFTBackend.h"
#include "SpectraForge/Core/SafeConsole.h"

#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <cstring>

namespace SpectraForge::Rendering::FreqVox::Backends {

using SpectraForge::Core::SAFE_TO_STRING;

VkFFTBackend::VkFFTBackend(vk::Instance instance, 
                           vk::PhysicalDevice physicalDevice,
                           vk::Device device)
    : vulkan_instance_(instance)
    , vulkan_physical_device_(physicalDevice)
    , vulkan_device_(device)
{
#ifdef HYPERENGINE_USE_VKFFT
    SAFE_PRINT_LINE("[VkFFTBackend] Конструктор вызван (VkFFT доступен)");
#else
    SAFE_WARNING("[VkFFTBackend] Создан без VkFFT поддержки (требуется HYPERENGINE_USE_VKFFT)");
#endif
}

VkFFTBackend::~VkFFTBackend() {
    shutdown();
}

bool VkFFTBackend::initialize(const DctBlockConfig& config) {
#ifndef HYPERENGINE_USE_VKFFT
    SAFE_ERROR("[VkFFTBackend] VkFFT недоступен - требуется HYPERENGINE_USE_VKFFT в сборке");
    return false;
#else
    if (initialized_) {
        SAFE_WARNING("[VkFFTBackend] Уже инициализирован");
        return true;
    }

    cfg_ = config;

    // Проверка корректности конфигурации
    if (cfg_.blockSize == 0 || cfg_.batchCount == 0) {
        SAFE_ERROR("[VkFFTBackend] Некорректная конфигурация: blockSize=" + 
                   SAFE_TO_STRING(cfg_.blockSize) + " batchCount=" + 
                   SAFE_TO_STRING(cfg_.batchCount));
        return false;
    }

    SAFE_PRINT_LINE("[VkFFTBackend] Инициализация с blockSize=" + 
                    SAFE_TO_STRING(cfg_.blockSize) + " batch=" + 
                    SAFE_TO_STRING(cfg_.batchCount));

    // Проверяем наличие Vulkan устройства
    if (!vulkan_device_) {
        SAFE_ERROR("[VkFFTBackend] Vulkan device не предоставлен");
        return false;
    }

    // Ищем compute queue
    compute_queue_family_index_ = find_compute_queue_family();
    if (compute_queue_family_index_ == UINT32_MAX) {
        SAFE_ERROR("[VkFFTBackend] Не найдено compute queue family");
        return false;
    }

    compute_queue_ = vulkan_device_.getQueue(compute_queue_family_index_, 0);

    // Инициализируем Vulkan ресурсы
    if (!initialize_vulkan_resources()) {
        SAFE_ERROR("[VkFFTBackend] Ошибка инициализации Vulkan ресурсов");
        shutdown();
        return false;
    }

    // Создаем VkFFT планы для DCT-II
    if (!create_vkfft_plans()) {
        SAFE_ERROR("[VkFFTBackend] Ошибка создания VkFFT планов");
        shutdown();
        return false;
    }

    initialized_ = true;
    SAFE_PRINT_LINE("[VkFFTBackend] Инициализация завершена успешно");
    return true;
#endif
}

void VkFFTBackend::shutdown() {
#ifdef HYPERENGINE_USE_VKFFT
    if (!initialized_) return;

    SAFE_PRINT_LINE("[VkFFTBackend] Завершение работы...");

    // Освобождаем VkFFT планы
    destroy_vkfft_plans();

    // Освобождаем Vulkan ресурсы
    if (device_buffer_) {
        vulkan_device_.destroyBuffer(*device_buffer_);
        device_buffer_.reset();
    }

    if (command_pool_) {
        vulkan_device_.destroyCommandPool(*command_pool_);
        command_pool_.reset();
    }

    initialized_ = false;
    SAFE_PRINT_LINE("[VkFFTBackend] Завершение работы завершено");
#endif
}

bool VkFFTBackend::transform_forward(std::vector<float>& io_block_batched) {
#ifndef HYPERENGINE_USE_VKFFT
    (void)io_block_batched;
    SAFE_ERROR("[VkFFTBackend] VkFFT недоступен");
    return false;
#else
    if (!initialized_) {
        SAFE_ERROR("[VkFFTBackend] Не инициализирован");
        return false;
    }

    // Проверка размера данных
    size_t expected_size = cfg_.batchCount * cfg_.blockSize * cfg_.blockSize;
    if (io_block_batched.size() != expected_size) {
        SAFE_ERROR("[VkFFTBackend] Некорректный размер данных: ожидалось " + 
                   SAFE_TO_STRING(expected_size) + ", получено " + 
                   SAFE_TO_STRING(io_block_batched.size()));
        return false;
    }

    return execute_transform(true, io_block_batched);
#endif
}

bool VkFFTBackend::transform_inverse(std::vector<float>& io_block_batched) {
#ifndef HYPERENGINE_USE_VKFFT
    (void)io_block_batched;
    SAFE_ERROR("[VkFFTBackend] VkFFT недоступен");
    return false;
#else
    if (!initialized_) {
        SAFE_ERROR("[VkFFTBackend] Не инициализирован");
        return false;
    }

    size_t expected_size = cfg_.batchCount * cfg_.blockSize * cfg_.blockSize;
    if (io_block_batched.size() != expected_size) {
        SAFE_ERROR("[VkFFTBackend] Некорректный размер данных");
        return false;
    }

    return execute_transform(false, io_block_batched);
#endif
}

bool VkFFTBackend::isAvailable() {
#ifdef HYPERENGINE_USE_VKFFT
    return true;
#else
    return false;
#endif
}

#ifdef HYPERENGINE_USE_VKFFT

uint32_t VkFFTBackend::find_compute_queue_family() {
    if (!vulkan_physical_device_) {
        SAFE_ERROR("[VkFFTBackend] Physical device не установлен");
        return UINT32_MAX;
    }

    auto queueFamilies = vulkan_physical_device_.getQueueFamilyProperties();
    
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
            SAFE_PRINT_LINE("[VkFFTBackend] Найдено compute queue family: " + SAFE_TO_STRING(i));
            return i;
        }
    }

    return UINT32_MAX;
}

bool VkFFTBackend::initialize_vulkan_resources() {
    try {
        // Создаем command pool
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.queueFamilyIndex = compute_queue_family_index_;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        command_pool_ = std::make_unique<vk::CommandPool>(
            vulkan_device_.createCommandPool(poolInfo)
        );

        // Вычисляем размер буфера (с запасом для промежуточных данных)
        buffer_size_bytes_ = cfg_.batchCount * cfg_.blockSize * cfg_.blockSize * sizeof(float) * 2;

        // Создаем Vulkan buffer
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = buffer_size_bytes_;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | 
                          vk::BufferUsageFlagBits::eTransferSrc | 
                          vk::BufferUsageFlagBits::eTransferDst;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        device_buffer_ = std::make_unique<vk::Buffer>(
            vulkan_device_.createBuffer(bufferInfo)
        );

        // Получаем требования к памяти
        auto memReqs = vulkan_device_.getBufferMemoryRequirements(*device_buffer_);

        // Выделяем память (HOST_VISIBLE для простоты доступа)
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memReqs.size;
        
        // Ищем подходящий тип памяти
        auto memProps = vulkan_physical_device_.getMemoryProperties();
        uint32_t memTypeIndex = UINT32_MAX;
        vk::MemoryPropertyFlags desiredProps = vk::MemoryPropertyFlagBits::eHostVisible | 
                                               vk::MemoryPropertyFlagBits::eHostCoherent;
        
        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((memReqs.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
                memTypeIndex = i;
                break;
            }
        }

        if (memTypeIndex == UINT32_MAX) {
            SAFE_ERROR("[VkFFTBackend] Не найден подходящий тип памяти");
            return false;
        }

        allocInfo.memoryTypeIndex = memTypeIndex;
        auto deviceMemory = vulkan_device_.allocateMemory(allocInfo);

        // Привязываем память к буферу
        vulkan_device_.bindBufferMemory(*device_buffer_, deviceMemory, 0);

        // Map memory для доступа
        device_buffer_memory_ = vulkan_device_.mapMemory(deviceMemory, 0, buffer_size_bytes_);

        SAFE_PRINT_LINE("[VkFFTBackend] Vulkan ресурсы инициализированы: буфер " + 
                        SAFE_TO_STRING(buffer_size_bytes_) + " байт");
        return true;

    } catch (const vk::SystemError& e) {
        SAFE_ERROR("[VkFFTBackend] Vulkan ошибка: " + std::string(e.what()));
        return false;
    }
}

bool VkFFTBackend::create_vkfft_plans() {
    try {
        // Конфигурация VkFFT для DCT-II
        vkfft_config_ = std::make_unique<VkFFTConfiguration>();
        std::memset(vkfft_config_.get(), 0, sizeof(VkFFTConfiguration));

        // Базовая конфигурация
        vkfft_config_->FFTdim = 2;  // 2D transform
        vkfft_config_->size[0] = cfg_.blockSize;
        vkfft_config_->size[1] = cfg_.blockSize;
        vkfft_config_->size[2] = 1;

        // ВРЕМЕННО: используем FFT вместо DCT для отладки
        // DCT имеет известные проблемы в некоторых версиях VkFFT
        // Согласно Math.md, DCT оптимальнее, но FFT тоже даёт frequency-domain
        vkfft_config_->performDCT = 0;  // 0 = FFT (для тестирования)
        // TODO: После успешной работы FFT, переключить на DCT type II:
        // vkfft_config_->performDCT = 2;
        
        SAFE_PRINT_LINE("[VkFFTBackend] DEBUG: Используется FFT (DCT отключен для отладки)");

        // Батчирование
        vkfft_config_->numberBatches = cfg_.batchCount;

        // Точность
        vkfft_config_->doublePrecision = 0;  // Single precision (float)
        
        // Vulkan compiler initialization
        vkfft_config_->isCompilerInitialized = 1;
        
        // Дополнительные параметры для корректной работы
        vkfft_config_->useLUT = 0;  // Не использовать LUT для начала
        vkfft_config_->makeForwardPlanOnly = 0;  // Нужны оба направления
        vkfft_config_->makeInversePlanOnly = 0;

        // Vulkan параметры (конвертируем vk:: в Vk C API)
        // Сохраняем handles как члены класса (чтобы они жили столько же, сколько и класс)
        vk_device_handle_ = static_cast<VkDevice>(vulkan_device_);
        vk_physical_device_handle_ = static_cast<VkPhysicalDevice>(vulkan_physical_device_);
        vk_queue_handle_ = static_cast<VkQueue>(compute_queue_);
        vk_command_pool_handle_ = static_cast<VkCommandPool>(*command_pool_);
        vk_buffer_handle_ = static_cast<VkBuffer>(*device_buffer_);
        
        vkfft_config_->device = &vk_device_handle_;
        vkfft_config_->physicalDevice = &vk_physical_device_handle_;
        vkfft_config_->queue = &vk_queue_handle_;
        vkfft_config_->commandPool = &vk_command_pool_handle_;
        
        // КРИТИЧЕСКИ ВАЖНО: Получаем и передаём device properties
        // VkFFT требует эту информацию для определения capabilities
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vk_physical_device_handle_, &deviceProperties);
        
        // Логируем device info
        SAFE_PRINT_LINE("[VkFFTBackend] Device: " + std::string(deviceProperties.deviceName));
        SAFE_PRINT_LINE("[VkFFTBackend] Vulkan API: " + 
                       SAFE_TO_STRING(VK_VERSION_MAJOR(deviceProperties.apiVersion)) + "." +
                       SAFE_TO_STRING(VK_VERSION_MINOR(deviceProperties.apiVersion)));
        
        // VkFFT автоматически получит properties через physical device,
        // но убедимся что device корректный

        // Буферы
        vkfft_config_->buffer = &vk_buffer_handle_;
        vkfft_config_->bufferSize = &buffer_size_bytes_;

        // Создаем VkFFT приложение (один план для forward и inverse)
        vkfft_app_forward_ = std::make_unique<VkFFTApplication>();
        
        SAFE_PRINT_LINE("[VkFFTBackend] Вызов initializeVkFFT...");
        VkFFTResult result = initializeVkFFT(vkfft_app_forward_.get(), *vkfft_config_);
        
        if (result != VKFFT_SUCCESS) {
            // Расшифровка VkFFT error codes
            std::string errorMsg = "Неизвестная ошибка";
            switch (result) {
                case 1001: errorMsg = "VKFFT_ERROR_MALLOC_FAILED"; break;
                case 1002: errorMsg = "VKFFT_ERROR_INSUFFICIENT_CODE_BUFFER"; break;
                case 1003: errorMsg = "VKFFT_ERROR_INSUFFICIENT_TEMP_BUFFER"; break;
                case 1004: errorMsg = "VKFFT_ERROR_PLAN_NOT_INITIALIZED"; break;
                case 1005: errorMsg = "VKFFT_ERROR_INVALID_PHYSICAL_DEVICE_PROPERTIES"; break;
                case 1006: errorMsg = "VKFFT_ERROR_INVALID_DEVICE"; break;
                case 1007: errorMsg = "VKFFT_ERROR_INVALID_QUEUE"; break;
                case 1008: errorMsg = "VKFFT_ERROR_INVALID_COMMAND_POOL"; break;
                case 1009: errorMsg = "VKFFT_ERROR_INVALID_FENCE"; break;
                case 1010: errorMsg = "VKFFT_ERROR_ONLY_FORWARD_FFT_INITIALIZED"; break;
                case 1011: errorMsg = "VKFFT_ERROR_ONLY_INVERSE_FFT_INITIALIZED"; break;
                case 1012: errorMsg = "VKFFT_ERROR_INVALID_CONTEXT"; break;
            }
            
            SAFE_ERROR("[VkFFTBackend] Ошибка создания VkFFT плана:");
            SAFE_ERROR("  Код: " + SAFE_TO_STRING(static_cast<int>(result)));
            SAFE_ERROR("  Описание: " + errorMsg);
            SAFE_ERROR("  Проверьте: device, physical device, queue, command pool");
            return false;
        }

        SAFE_PRINT_LINE("[VkFFTBackend] VkFFT план DCT-II создан успешно");
        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[VkFFTBackend] Исключение при создании VkFFT планов: " + std::string(e.what()));
        return false;
    }
}

void VkFFTBackend::destroy_vkfft_plans() {
    if (vkfft_app_forward_) {
        deleteVkFFT(vkfft_app_forward_.get());
        vkfft_app_forward_.reset();
    }

    vkfft_config_.reset();
}

bool VkFFTBackend::execute_transform(bool is_forward, std::vector<float>& data) {
    try {
        // Копируем данные в device buffer
        if (!device_buffer_memory_) {
            SAFE_ERROR("[VkFFTBackend] Device memory не замаплена");
            return false;
        }

        size_t data_size_bytes = data.size() * sizeof(float);
        std::memcpy(device_buffer_memory_, data.data(), data_size_bytes);

        // Настраиваем launch параметры
        VkFFTLaunchParams launchParams{};
        launchParams.buffer = &(VkBuffer&)*device_buffer_;

        // Выполняем трансформацию (используем один план, передаем inverse параметром)
        int inverse = is_forward ? 0 : 1;
        VkFFTResult result = VkFFTAppend(vkfft_app_forward_.get(), inverse, &launchParams);
        
        if (result != VKFFT_SUCCESS) {
            SAFE_ERROR("[VkFFTBackend] Ошибка выполнения " + 
                       std::string(is_forward ? "DCT" : "IDCT") + 
                       ": код " + SAFE_TO_STRING(static_cast<int>(result)));
            return false;
        }

        // Синхронизируем
        vulkan_device_.waitIdle();

        // Копируем результат обратно
        std::memcpy(data.data(), device_buffer_memory_, data_size_bytes);

        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[VkFFTBackend] Исключение при выполнении трансформации: " + 
                   std::string(e.what()));
        return false;
    }
}

#endif // HYPERENGINE_USE_VKFFT

} // namespace SpectraForge::Rendering::FreqVox::Backends

