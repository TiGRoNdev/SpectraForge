#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda.h>
#include <cuda_runtime.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
// Дополнительные определения для external memory/semaphore
#ifndef VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
#define VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME "VK_KHR_external_memory_win32"
#endif
#ifndef VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
#define VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME "VK_KHR_external_semaphore_win32"
#endif
#endif

// Для Vulkan interop нам не нужен OpenGL
// OpenGL заголовки убраны для избежания конфликтов
// #if CUDA_VERSION >= 10000
// #include <cudaGL.h>
// #include <cuda_gl_interop.h>
// #endif

#endif  // CUDA_VULKAN_INTEROP_SUPPORTED

namespace HyperEngine::Vulkan {
class ResourceManager;
}

namespace HyperEngine::CUDA {

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED

/**
 * @brief Shared ресурс между CUDA и Vulkan
 */
struct SharedResource {
    vk::Buffer vulkanBuffer;
    vk::DeviceMemory vulkanMemory;
    cudaExternalMemory_t cudaExternalMemory;
    CUdeviceptr cudaDevicePtr;
    size_t size;
    bool isValid;
};

/**
 * @brief Синхронизация между CUDA и Vulkan
 */
struct SyncObject {
    vk::Semaphore vulkanSemaphore;
    cudaExternalSemaphore_t cudaExternalSemaphore;
    bool isValid;
};

/**
 * @brief Основной класс для CUDA-Vulkan interop
 *
 * Обеспечивает создание shared ресурсов и синхронизацию
 * между CUDA и Vulkan без копирования данных
 */
class CudaInterop {
  public:
    /**
     * @brief Конструктор
     */
    CudaInterop();

    /**
     * @brief Деструктор
     */
    ~CudaInterop();

    /**
     * @brief Инициализация CUDA-Vulkan interop
     * @param device Vulkan логическое устройство
     * @param physicalDevice Vulkan физическое устройство
     * @param resourceManager Менеджер ресурсов Vulkan
     * @return true если инициализация успешна
     */
    bool initializeInterop(vk::Device dev,
                           vk::PhysicalDevice physDev,
                           Vulkan::ResourceManager* resMgr);

    /**
     * @brief Завершение работы interop
     */
    void cleanup();

    /**
     * @brief Создание shared буфера
     * @param size Размер буфера в байтах
     * @param vulkanUsage Флаги использования Vulkan
     * @param cudaFlags Флаги CUDA
     * @return Shared ресурс
     */
    std::shared_ptr<SharedResource> createSharedBuffer(
        size_t size,
        vk::BufferUsageFlags vulkanUsage = vk::BufferUsageFlagBits::eStorageBuffer,
        unsigned int cudaFlags = cudaMemAttachGlobal);

    /**
     * @brief Освобождение shared ресурса
     * @param resource Ресурс для освобождения
     */
    void freeSharedResource(std::shared_ptr<SharedResource> resource);

    /**
     * @brief Создание объекта синхронизации
     * @return Объект синхронизации
     */
    std::shared_ptr<SyncObject> createSyncObject();

    /**
     * @brief Сигнализация от Vulkan к CUDA
     * @param syncObj Объект синхронизации
     * @param stream CUDA stream
     */
    void signalVulkanToCuda(const std::shared_ptr<SyncObject>& syncObj, cudaStream_t stream = 0);

    /**
     * @brief Ожидание сигнала от CUDA в Vulkan
     * @param syncObj Объект синхронизации
     * @param commandBuffer Command buffer для вставки wait
     */
    void waitCudaFromVulkan(const std::shared_ptr<SyncObject>& syncObj,
                            vk::CommandBuffer commandBuffer);

    /**
     * @brief Импорт Vulkan памяти в CUDA
     * @param vulkanMemory Vulkan память
     * @param size Размер памяти
     * @return CUDA external memory
     */
    cudaExternalMemory_t importVulkanMemory(vk::DeviceMemory vulkanMemory, size_t size);

    /**
     * @brief Экспорт CUDA памяти в Vulkan
     * @param cudaPtr CUDA указатель
     * @param size Размер памяти
     * @return Vulkan память
     */
    vk::DeviceMemory exportCudaMemory(CUdeviceptr cudaPtr, size_t size);

    /**
     * @brief Проверка поддержки interop
     * @return true если interop поддерживается
     */
    static bool isInteropSupported();

    /**
     * @brief Получение информации о возможностях interop
     * @return Строка с информацией
     */
    std::string getInteropCapabilities() const;

    /**
     * @brief Проверка инициализации
     * @return true если инициализирован
     */
    bool isInitialized() const { return initialized; }

  private:
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
    Vulkan::ResourceManager* resourceManager = nullptr;

    CUcontext cudaContext = nullptr;
    CUdevice cudaDevice = -1;

    // Кэш созданных ресурсов
    std::vector<std::shared_ptr<SharedResource>> sharedResources;
    std::vector<std::shared_ptr<SyncObject>> syncObjects;

    bool initialized = false;
    bool hasPlatformSupport = false;

    /**
     * @brief Инициализация CUDA контекста
     * @return true если успешно
     */
    bool initCudaContext();

    /**
     * @brief Проверка поддержки external memory
     * @return true если поддерживается
     */
    bool checkExternalMemorySupport();

    /**
     * @brief Проверка поддержки external semaphores
     * @return true если поддерживается
     */
    bool checkExternalSemaphoreSupport();

    /**
     * @brief Проверка поддержки необходимых Vulkan расширений
     * @return true если все необходимые расширения поддерживаются
     */
    bool checkVulkanExtensionSupport();

    /**
     * @brief Получение CUDA device из Vulkan физического устройства
     * @return true если найдено совпадение
     */
    bool findMatchingCudaDevice();

    /**
     * @brief Тестирование поддержки external memory на устройстве
     * @param device CUDA device ID
     * @return true если external memory поддерживается
     */
    static bool testExternalMemorySupport(int device);

    // Запрет копирования
    CudaInterop(const CudaInterop&) = delete;
    CudaInterop& operator=(const CudaInterop&) = delete;
};

#else  // CUDA_VULKAN_INTEROP_SUPPORTED

/**
 * @brief Заглушка для случая без поддержки CUDA
 */
class CudaInterop {
  public:
    CudaInterop() = default;
    ~CudaInterop() = default;

    bool initializeInterop(vk::Device, vk::PhysicalDevice, Vulkan::ResourceManager*) {
        return false;
    }

    void cleanup() {}

    static bool isInteropSupported() { return false; }
    std::string getInteropCapabilities() const { return "CUDA interop not supported"; }
    bool isInitialized() const { return false; }
};

#endif  // CUDA_VULKAN_INTEROP_SUPPORTED

}  // namespace HyperEngine::CUDA
