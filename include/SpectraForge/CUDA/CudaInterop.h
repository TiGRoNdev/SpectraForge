
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
#include <vulkan/vulkan.hpp>
#endif

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda.h>
#include <cuda_runtime.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#endif
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

namespace SpectraForge::Vulkan {
class ResourceManager;
}

namespace SpectraForge::CUDA {

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED

/**
 * @brief Shared ресурс между CUDA и Vulkan
 */
struct SharedResource {
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::Buffer vulkanBuffer;
    vk::DeviceMemory vulkanMemory;
#else
    void* vulkanBuffer;
    void* vulkanMemory;
#endif
    cudaExternalMemory_t cudaExternalMemory;
    CUdeviceptr cudaDevicePtr;
    size_t size;
    bool isValid;
};

/**
 * @brief Синхронизация между CUDA и Vulkan
 */
struct SyncObject {
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::Semaphore vulkanSemaphore;
#else
    void* vulkanSemaphore;
#endif
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
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    bool initializeInterop(vk::Device dev,
                           vk::PhysicalDevice physDev,
                           Vulkan::ResourceManager* resMgr);
#else
    bool initializeInterop(void* dev, void* physDev, void* resMgr);
#endif

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
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
        vk::BufferUsageFlags vulkanUsage = vk::BufferUsageFlagBits::eStorageBuffer,
#else
        uint32_t vulkanUsage = 0x00000008,  // VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
#endif
        unsigned int cudaFlags = cudaMemAttachGlobal);

// Преобразующий оверлоад объявлять не будем, чтобы избежать дублирующих определений.

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
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
                            vk::CommandBuffer commandBuffer);
#else
                            void* commandBuffer);
#endif

    /**
     * @brief Импорт Vulkan памяти в CUDA
     * @param vulkanMemory Vulkan память
     * @param size Размер памяти
     * @return CUDA external memory
     */
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    cudaExternalMemory_t importVulkanMemory(vk::DeviceMemory vulkanMemory, size_t size);
#else
    cudaExternalMemory_t importVulkanMemory(void* vulkanMemory, size_t size);
#endif

    /**
     * @brief Экспорт CUDA памяти в Vulkan
     * @param cudaPtr CUDA указатель
     * @param size Размер памяти
     * @return Vulkan память
     */
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::DeviceMemory exportCudaMemory(CUdeviceptr cudaPtr, size_t size);
#else
    void* exportCudaMemory(CUdeviceptr cudaPtr, size_t size);
#endif

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
#if defined(VULKAN_RENDERER_BUILD) || defined(SpectraForge_ENABLE_VULKAN)
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
#else
    void* device;
    void* physicalDevice;
#endif
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
     * @return true если поддерживаются
     */
#ifdef HyperEngine_ENABLE_VULKAN
    bool checkVulkanExtensionSupport();
#else
    bool checkVulkanExtensionSupport();
#endif

    /**
     * @brief Поиск соответствующего CUDA device
     * @return true если найден
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

    bool initializeInterop(void*, void*, void*) { return false; }

    void cleanup() {}

    static bool isInteropSupported() { return false; }
    std::string getInteropCapabilities() const { return "CUDA interop not supported"; }
    bool isInitialized() const { return false; }
};

#endif  // CUDA_VULKAN_INTEROP_SUPPORTED

}  // namespace SpectraForge::CUDA
