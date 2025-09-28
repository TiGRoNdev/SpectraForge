/**
 * @file CudaInterop.cpp
 * @brief Реализация CUDA-Vulkan interop
 * 
 * Обеспечивает обмен данными между CUDA и Vulkan без копирования
 * через external memory и semaphore extensions.
 */

#include "HyperEngine/CUDA/CudaInterop.h"
#include "HyperEngine/Vulkan/ResourceManager.h"
#include "HyperEngine/Core/Console.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

// Windows-specific interop включен
// #define DISABLE_WIN32_INTEROP

using namespace HyperEngine::CUDA;
using namespace HyperEngine::Core;

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED

namespace HyperEngine::CUDA {

CudaInterop::CudaInterop() {
    // Инициализация в initializeInterop()
}

CudaInterop::~CudaInterop() {
    if (initialized) {
        cleanup();
    }
}

bool CudaInterop::initializeInterop(vk::Device dev, 
                                   vk::PhysicalDevice physDev,
                                   Vulkan::ResourceManager* resMgr) {
    try {
        this->device = dev;
        this->physicalDevice = physDev;
        this->resourceManager = resMgr;
        
        std::cout << "[CudaInterop] Инициализация CUDA-Vulkan interop..." << std::endl;
        
        // Проверяем базовую поддержку interop
        if (!isInteropSupported()) {
            std::cout << "[CudaInterop] Ошибка: Interop не поддерживается на данной платформе" << std::endl;
            return false;
        }
        
        // Проверяем поддержку необходимых Vulkan расширений
        if (!checkVulkanExtensionSupport()) {
            std::cout << "[CudaInterop] Ошибка: Необходимые Vulkan расширения не поддерживаются" << std::endl;
            return false;
        }
        
        // Инициализируем CUDA контекст
        if (!initCudaContext()) {
            std::cout << "[CudaInterop] Ошибка инициализации CUDA контекста" << std::endl;
            return false;
        }
        
        // Проверяем поддержку external memory
        if (!checkExternalMemorySupport()) {
            std::cout << "[CudaInterop] Ошибка: External memory не поддерживается" << std::endl;
            return false;
        }
        
        // Проверяем поддержку external semaphores
        if (!checkExternalSemaphoreSupport()) {
            std::cout << "[CudaInterop] Предупреждение: External semaphores не поддерживаются" << std::endl;
        }
        
        // Ищем совпадающий CUDA device
        if (!findMatchingCudaDevice()) {
            std::cout << "[CudaInterop] Ошибка: Не найдено совпадающее CUDA устройство" << std::endl;
            return false;
        }
        
        initialized = true;
        std::cout << "[CudaInterop] Инициализация завершена успешно" << std::endl;
        std::cout << "[CudaInterop] Возможности: " << getInteropCapabilities() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

void CudaInterop::cleanup() {
    if (!initialized) {
        return;
    }
    
    std::cout << "[CudaInterop] Освобождение ресурсов..." << std::endl;
    
    // Освобождаем все shared ресурсы
    for (auto& resource : sharedResources) {
        if (resource && resource->isValid) {
            freeSharedResource(resource);
        }
    }
    sharedResources.clear();
    
    // Освобождаем sync объекты
    for (auto& syncObj : syncObjects) {
        if (syncObj && syncObj->isValid) {
            if (syncObj->vulkanSemaphore) {
                device.destroySemaphore(syncObj->vulkanSemaphore);
            }
            if (syncObj->cudaExternalSemaphore) {
                cudaDestroyExternalSemaphore(syncObj->cudaExternalSemaphore);
            }
            syncObj->isValid = false;
        }
    }
    syncObjects.clear();
    
    // Освобождаем CUDA контекст
    if (cudaContext) {
        cuCtxDestroy(cudaContext);
        cudaContext = nullptr;
    }
    
    initialized = false;
    std::cout << "[CudaInterop] Освобождение ресурсов завершено" << std::endl;
}

std::shared_ptr<SharedResource> CudaInterop::createSharedBuffer(
    size_t size,
    vk::BufferUsageFlags vulkanUsage,
    unsigned int cudaFlags) {
    
    if (!initialized) {
        std::cout << "[CudaInterop] Ошибка: Interop не инициализирован" << std::endl;
        return nullptr;
    }
    
    try {
        auto resource = std::make_shared<SharedResource>();
        resource->size = size;
        resource->isValid = false;
        
        std::cout << "[CudaInterop] Создание shared буфера размером " << size << " байт" << std::endl;
        
        // Создаем Vulkan буфер с external memory
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = vulkanUsage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        
        // Добавляем external memory flag
        vk::ExternalMemoryBufferCreateInfo extMemInfo{};
        extMemInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        bufferInfo.pNext = &extMemInfo;
        
        resource->vulkanBuffer = device.createBuffer(bufferInfo);
        
        // Получаем требования к памяти
        auto memRequirements = device.getBufferMemoryRequirements(resource->vulkanBuffer);
        
        // Создаем external memory
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = resourceManager->findMemoryType(
            memRequirements.memoryTypeBits, 
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        
        vk::ExportMemoryAllocateInfo exportInfo{};
        exportInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
        allocInfo.pNext = &exportInfo;
        
        resource->vulkanMemory = device.allocateMemory(allocInfo);
        
        // Привязываем буфер к памяти
        device.bindBufferMemory(resource->vulkanBuffer, resource->vulkanMemory, 0);
        
        // Импортируем память в CUDA
        resource->cudaExternalMemory = importVulkanMemory(resource->vulkanMemory, size);
        
        // Получаем CUDA device pointer
        cudaExternalMemoryBufferDesc bufferDesc{};
        bufferDesc.offset = 0;
        bufferDesc.size = size;
        bufferDesc.flags = cudaFlags;
        
        cudaError_t result = cudaExternalMemoryGetMappedBuffer(
            reinterpret_cast<void**>(&resource->cudaDevicePtr),
            resource->cudaExternalMemory,
            &bufferDesc);
        
        if (result != cudaSuccess) {
            throw std::runtime_error("Ошибка получения CUDA device pointer: " + 
                                   std::string(cudaGetErrorString(result)));
        }
        
        resource->isValid = true;
        sharedResources.push_back(resource);
        
        std::cout << "[CudaInterop] Shared буфер создан успешно" << std::endl;
        return resource;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка создания shared буфера: " << e.what() << std::endl;
        return nullptr;
    }
}

void CudaInterop::freeSharedResource(std::shared_ptr<SharedResource> resource) {
    if (!resource || !resource->isValid) {
        return;
    }
    
    std::cout << "[CudaInterop] Освобождение shared ресурса" << std::endl;
    
    // Освобождаем CUDA ресурсы
    if (resource->cudaExternalMemory) {
        cudaDestroyExternalMemory(resource->cudaExternalMemory);
    }
    
    // Освобождаем Vulkan ресурсы
    if (resource->vulkanBuffer) {
        device.destroyBuffer(resource->vulkanBuffer);
    }
    
    if (resource->vulkanMemory) {
        device.freeMemory(resource->vulkanMemory);
    }
    
    resource->isValid = false;
}

std::shared_ptr<SyncObject> CudaInterop::createSyncObject() {
    if (!initialized) {
        std::cout << "[CudaInterop] Ошибка: Interop не инициализирован" << std::endl;
        return nullptr;
    }
    
    try {
        auto syncObj = std::make_shared<SyncObject>();
        syncObj->isValid = false;
        
        std::cout << "[CudaInterop] Создание объекта синхронизации" << std::endl;
        
        // Создаем Vulkan semaphore с external export
        vk::SemaphoreCreateInfo semaphoreInfo{};
        vk::ExportSemaphoreCreateInfo exportInfo{};
        exportInfo.handleTypes = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
        semaphoreInfo.pNext = &exportInfo;
        
        syncObj->vulkanSemaphore = device.createSemaphore(semaphoreInfo);
        
        // Получаем handle для CUDA
        // TODO: Временная заглушка до полной поддержки VK_KHR_external_semaphore_win32
#ifdef _WIN32
        // Пока используем заглушку для handle
        HANDLE handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        
        if (handle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Не удалось создать временный Windows handle для semaphore");
        }
        
        // RAII-обертка для автоматического освобождения handle в случае исключения
        auto handleGuard = [handle]() { 
            if (handle != INVALID_HANDLE_VALUE) {
                CloseHandle(handle);
            }
        };
        
        try {
            std::cout << "[CudaInterop] Временная заглушка для semaphore handle (требуется VK_KHR_external_semaphore_win32)" << std::endl;
            
            // Импортируем в CUDA
            cudaExternalSemaphoreHandleDesc semHandleDesc{};
            semHandleDesc.type = cudaExternalSemaphoreHandleTypeOpaqueWin32;
            semHandleDesc.handle.win32.handle = handle;
            
            cudaError_t result = cudaImportExternalSemaphore(
                &syncObj->cudaExternalSemaphore, &semHandleDesc);
            
            if (result != cudaSuccess) {
                handleGuard(); // Освобождаем handle перед выбросом исключения
                throw std::runtime_error("Ошибка импорта semaphore в CUDA: " + 
                                       std::string(cudaGetErrorString(result)));
            }
            
            // Handle успешно передан в CUDA, не освобождаем его здесь
            // CUDA теперь владеет handle и освободит его при cudaDestroyExternalSemaphore
            
        } catch (...) {
            handleGuard(); // Освобождаем handle в случае любого исключения
            throw;
        }
#else
        void* handle = nullptr;
        std::cout << "[CudaInterop] Non-Windows платформы пока не поддерживаются" << std::endl;
        return nullptr;
#endif
        
        syncObj->isValid = true;
        syncObjects.push_back(syncObj);
        
        std::cout << "[CudaInterop] Объект синхронизации создан успешно" << std::endl;
        return syncObj;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка создания объекта синхронизации: " << e.what() << std::endl;
        return nullptr;
    }
}

void CudaInterop::signalVulkanToCuda(std::shared_ptr<SyncObject> syncObj, cudaStream_t stream) {
    if (!syncObj || !syncObj->isValid) {
        std::cout << "[CudaInterop] Ошибка: Некорректный объект синхронизации" << std::endl;
        return;
    }
    
    cudaExternalSemaphoreWaitParams waitParams{};
    memset(&waitParams, 0, sizeof(waitParams));
    
    cudaError_t result = cudaWaitExternalSemaphoresAsync(
        &syncObj->cudaExternalSemaphore, &waitParams, 1, stream);
    
    if (result != cudaSuccess) {
        std::cout << "[CudaInterop] Ошибка ожидания semaphore в CUDA: " 
                  << cudaGetErrorString(result) << std::endl;
    }
}

void CudaInterop::waitCudaFromVulkan(std::shared_ptr<SyncObject> syncObj, vk::CommandBuffer commandBuffer) {
    if (!syncObj || !syncObj->isValid) {
        std::cout << "[CudaInterop] Ошибка: Некорректный объект синхронизации" << std::endl;
        return;
    }
    
    // Подавляем предупреждение о неиспользуемом параметре
    (void)commandBuffer;
    
    // Вставляем wait в command buffer
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
    (void)waitStage; // Подавляем предупреждение
    
    // TODO: Реализовать правильную вставку wait operation в command buffer
    // Это требует более сложной логики с submission
    std::cout << "[CudaInterop] Ожидание CUDA signal в Vulkan (реализация в progress)" << std::endl;
}

cudaExternalMemory_t CudaInterop::importVulkanMemory(vk::DeviceMemory vulkanMemory, size_t size) {
    try {
        // Подавляем предупреждение о неиспользуемом параметре
        (void)vulkanMemory;
        
        // Получаем Windows handle из Vulkan memory
        // TODO: Временная заглушка до полной поддержки VK_KHR_external_memory_win32
#ifdef _WIN32
        // Пока используем заглушку для handle
        HANDLE handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        
        if (handle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Не удалось создать временный Windows handle для памяти");
        }
        
        // RAII-обертка для автоматического освобождения handle в случае исключения
        auto handleGuard = [handle]() { 
            if (handle != INVALID_HANDLE_VALUE) {
                CloseHandle(handle);
            }
        };
        
        try {
            std::cout << "[CudaInterop] Временная заглушка для memory handle (требуется VK_KHR_external_memory_win32)" << std::endl;
            
            // Создаем CUDA external memory
            cudaExternalMemoryHandleDesc memHandleDesc{};
            memHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
            memHandleDesc.handle.win32.handle = handle;
            memHandleDesc.size = size;
            
            cudaExternalMemory_t extMem;
            cudaError_t cudaResult = cudaImportExternalMemory(&extMem, &memHandleDesc);
            
            if (cudaResult != cudaSuccess) {
                handleGuard(); // Освобождаем handle перед выбросом исключения
                throw std::runtime_error("Ошибка импорта Vulkan памяти в CUDA: " + 
                                       std::string(cudaGetErrorString(cudaResult)));
            }
            
            // Handle успешно передан в CUDA, не освобождаем его здесь
            // CUDA теперь владеет handle и освободит его при cudaDestroyExternalMemory
            return extMem;
            
        } catch (...) {
            handleGuard(); // Освобождаем handle в случае любого исключения
            throw;
        }
#else
        void* handle = nullptr;
        std::cout << "[CudaInterop] Non-Windows платформы пока не поддерживаются" << std::endl;
        return nullptr;
#endif
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] " << e.what() << std::endl;
        return nullptr;
    }
}

vk::DeviceMemory CudaInterop::exportCudaMemory(CUdeviceptr cudaPtr, size_t size) {
    // Подавляем предупреждения о неиспользуемых параметрах
    (void)cudaPtr;
    (void)size;
    
    // TODO: Реализация экспорта CUDA памяти в Vulkan
    // Это более сложная операция, требующая создания CUDA external memory
    std::cout << "[CudaInterop] Экспорт CUDA памяти в Vulkan (пока не реализован)" << std::endl;
    return vk::DeviceMemory{};
}

bool CudaInterop::isInteropSupported() {
    #ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        try {
            // Проверяем наличие CUDA runtime
            int deviceCount = 0;
            cudaError_t result = cudaGetDeviceCount(&deviceCount);
            
            if (result != cudaSuccess) {
                std::cout << "[CudaInterop] CUDA runtime недоступен: " << cudaGetErrorString(result) << std::endl;
                return false;
            }
            
            if (deviceCount == 0) {
                std::cout << "[CudaInterop] CUDA устройства не найдены" << std::endl;
                return false;
            }
            
            std::cout << "[CudaInterop] Найдено CUDA устройств: " << SAFE_TO_STRING(deviceCount) << std::endl;
            
            // Проверяем каждое устройство на поддержку external memory
            for (int device = 0; device < deviceCount; ++device) {
                cudaDeviceProp prop;
                result = cudaGetDeviceProperties(&prop, device);
                if (result != cudaSuccess) {
                    continue;
                }
                
                std::cout << "[CudaInterop] Устройство " << SAFE_TO_STRING(device) << ": " << prop.name << std::endl;
                std::cout << "[CudaInterop] Compute Capability: " << SAFE_TO_STRING(prop.major) << "." << SAFE_TO_STRING(prop.minor) << std::endl;
                
                // Для external memory нужна compute capability >= 6.0
                if (prop.major >= 6) {
                    std::cout << "[CudaInterop] Устройство поддерживает external memory (Compute >= 6.0)" << std::endl;
                    return true;
                }
            }
            
            std::cout << "[CudaInterop] Ни одно устройство не поддерживает external memory (требуется Compute >= 6.0)" << std::endl;
            return false;
            
        } catch (const std::exception& e) {
            std::cout << "[CudaInterop] Ошибка проверки поддержки: " << e.what() << std::endl;
            return false;
        }
    #else
        return false;
    #endif
}

std::string CudaInterop::getInteropCapabilities() const {
    if (!initialized) {
        return "Не инициализирован";
    }
    
    std::string caps = "CUDA-Vulkan Interop: ";
    caps += "External Memory: " + std::string(hasPlatformSupport ? "Да" : "Нет");
    
    // Добавляем информацию о CUDA устройстве
    if (cudaDevice >= 0) {
        caps += ", CUDA Device: " + std::to_string(cudaDevice);
    }
    
    return caps;
}

// Приватные методы

bool CudaInterop::initCudaContext() {
    try {
        std::cout << "[CudaInterop] Инициализация CUDA контекста..." << std::endl;
        
        // Получаем количество устройств
        int deviceCount = 0;
        cudaError_t result = cudaGetDeviceCount(&deviceCount);
        if (result != cudaSuccess) {
            std::cout << "[CudaInterop] Ошибка получения количества CUDA устройств: " 
                      << cudaGetErrorString(result) << std::endl;
            return false;
        }
        
        if (deviceCount == 0) {
            std::cout << "[CudaInterop] CUDA устройства не найдены" << std::endl;
            return false;
        }
        
        std::cout << "[CudaInterop] Найдено CUDA устройств: " << SAFE_TO_STRING(deviceCount) << std::endl;
        
        // Выбираем первое доступное устройство
        cudaDevice = 0;
        result = cudaSetDevice(cudaDevice);
        if (result != cudaSuccess) {
            std::cout << "[CudaInterop] Ошибка установки CUDA устройства " << SAFE_TO_STRING(cudaDevice) << ": " 
                      << cudaGetErrorString(result) << std::endl;
            return false;
        }
        
        // Получаем информацию об устройстве
        cudaDeviceProp prop;
        result = cudaGetDeviceProperties(&prop, cudaDevice);
        if (result == cudaSuccess) {
            std::cout << "[CudaInterop] Выбрано устройство: " << prop.name << std::endl;
            std::cout << "[CudaInterop] Compute Capability: " << SAFE_TO_STRING(prop.major) << "." << SAFE_TO_STRING(prop.minor) << std::endl;
        }
        
        // Инициализируем CUDA Driver API для получения контекста
        CUresult cuResult = cuInit(0);
        if (cuResult != CUDA_SUCCESS) {
            std::cout << "[CudaInterop] Ошибка инициализации CUDA Driver API" << std::endl;
            return false;
        }
        
        // Получаем current context (автоматически создается runtime API)
        cuResult = cuCtxGetCurrent(&cudaContext);
        if (cuResult != CUDA_SUCCESS) {
            std::cout << "[CudaInterop] Ошибка получения CUDA контекста" << std::endl;
            return false;
        }
        
        // Если контекст не создан, создаем его через Runtime API
        if (cudaContext == nullptr) {
            // Используем Runtime API для создания контекста
            cudaError_t result = cudaFree(0); // Принудительно создаем контекст
            if (result != cudaSuccess) {
                std::cout << "[CudaInterop] Ошибка создания CUDA контекста через Runtime API: " 
                          << cudaGetErrorString(result) << std::endl;
                return false;
            }
            
            // Теперь получаем созданный контекст
            cuResult = cuCtxGetCurrent(&cudaContext);
            if (cuResult != CUDA_SUCCESS || cudaContext == nullptr) {
                std::cout << "[CudaInterop] Ошибка получения созданного CUDA контекста" << std::endl;
                return false;
            }
        }
        
        std::cout << "[CudaInterop] CUDA контекст инициализирован успешно" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка инициализации CUDA: " << e.what() << std::endl;
        return false;
    }
}

bool CudaInterop::checkExternalMemorySupport() {
    if (cudaDevice < 0) {
        std::cout << "[CudaInterop] CUDA устройство не инициализировано" << std::endl;
        return false;
    }
    
    try {
        // Получаем свойства устройства
        cudaDeviceProp prop;
        cudaError_t result = cudaGetDeviceProperties(&prop, cudaDevice);
        
        if (result != cudaSuccess) {
            std::cout << "[CudaInterop] Ошибка получения свойств устройства: " << cudaGetErrorString(result) << std::endl;
            return false;
        }
        
        // Проверяем compute capability для external memory
        bool supportsExternalMemory = (prop.major >= 6);
        
        std::cout << "[CudaInterop] Устройство " << SAFE_TO_STRING(cudaDevice) << " (" << prop.name << ")" << std::endl;
        std::cout << "[CudaInterop] Compute Capability: " << SAFE_TO_STRING(prop.major) << "." << SAFE_TO_STRING(prop.minor) << std::endl;
        std::cout << "[CudaInterop] External Memory поддержка: " << (supportsExternalMemory ? "Да" : "Нет") << std::endl;
        
        hasPlatformSupport = supportsExternalMemory;
        return hasPlatformSupport;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка проверки external memory: " << e.what() << std::endl;
        return false;
    }
}

bool CudaInterop::checkExternalSemaphoreSupport() {
    if (cudaDevice < 0) {
        std::cout << "[CudaInterop] CUDA устройство не инициализировано для проверки semaphore" << std::endl;
        return false;
    }
    
    try {
        // Получаем свойства устройства
        cudaDeviceProp prop;
        cudaError_t result = cudaGetDeviceProperties(&prop, cudaDevice);
        
        if (result != cudaSuccess) {
            std::cout << "[CudaInterop] Ошибка получения свойств для semaphore: " << cudaGetErrorString(result) << std::endl;
            return false;
        }
        
        // External semaphores также требуют compute capability >= 6.0
        bool supportsExternalSemaphore = (prop.major >= 6);
        
        std::cout << "[CudaInterop] External Semaphore поддержка: " << (supportsExternalSemaphore ? "Да" : "Нет") << std::endl;
        
        return supportsExternalSemaphore;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка проверки external semaphore: " << e.what() << std::endl;
        return false;
    }
}

bool CudaInterop::checkVulkanExtensionSupport() {
    try {
        std::cout << "[CudaInterop] Проверка поддержки Vulkan расширений..." << std::endl;
        
        // Получаем список поддерживаемых расширений
        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        
        // Список необходимых расширений для CUDA-Vulkan interop
        std::vector<std::string> requiredExtensions = {
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
#ifdef _WIN32
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
#else
            VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME
#endif
        };
        
        // Проверяем каждое необходимое расширение
        for (const auto& required : requiredExtensions) {
            bool found = false;
            for (const auto& available : availableExtensions) {
                if (std::string(available.extensionName.data()) == required) {
                    found = true;
                    break;
                }
            }
            
            if (found) {
                std::cout << "[CudaInterop] ✓ Расширение поддерживается: " << required << std::endl;
            } else {
                std::cout << "[CudaInterop] ✗ Расширение НЕ поддерживается: " << required << std::endl;
                return false;
            }
        }
        
        std::cout << "[CudaInterop] Все необходимые Vulkan расширения поддерживаются" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "[CudaInterop] Ошибка проверки Vulkan расширений: " << e.what() << std::endl;
        return false;
    }
}

bool CudaInterop::findMatchingCudaDevice() {
    // TODO: Реализовать поиск CUDA устройства, соответствующего Vulkan физическому устройству
    // Это можно сделать через сравнение PCI bus ID или UUID
    
    // Пока используем простую проверку
    if (cudaDevice >= 0) {
        std::cout << "[CudaInterop] Используем CUDA устройство " << SAFE_TO_STRING(cudaDevice) << std::endl;
        return true;
    }
    
    return false;
}

} // namespace HyperEngine::CUDA

#else // CUDA_VULKAN_INTEROP_SUPPORTED

// Заглушки для случая без поддержки CUDA
namespace HyperEngine::CUDA {
// Все методы уже реализованы в заголовочном файле как inline
}

#endif // CUDA_VULKAN_INTEROP_SUPPORTED

// Экспортируемая функция для тестирования
extern "C" {
    void cuda_interop_test() {
        std::cout << "[CudaInterop Test] Тестирование CUDA-Vulkan interop..." << std::endl;
        
        HyperEngine::CUDA::CudaInterop interop;
        
        if (HyperEngine::CUDA::CudaInterop::isInteropSupported()) {
            std::cout << "[CudaInterop Test] Interop поддерживается" << std::endl;
            
            // TODO: Добавить более детальное тестирование при наличии Vulkan контекста
        } else {
            std::cout << "[CudaInterop Test] Interop не поддерживается на данной платформе" << std::endl;
        }
    }
}

