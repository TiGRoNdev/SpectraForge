/**
 * @file cuda_vulkan_interop_demo.cpp
 * @brief Демонстрация CUDA-Vulkan interop функциональности
 * 
 * Показывает создание shared буферов, обмен данными между CUDA и Vulkan,
 * и синхронизацию между двумя API без копирования памяти.
 */

#include <iostream>
#include <vector>
#include <memory>

// Основные заголовки движка
#include "Engine3D/Core/Engine3D.h"
#include "Engine3D/Vulkan/VulkanEngine.h"
#include "Engine3D/Vulkan/ResourceManager.h"
#include "Engine3D/Vulkan/HardwareDetector.h"
#include "Engine3D/CUDA/CudaInterop.h"

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda_runtime.h>
#include <cuda.h>

// CUDA kernel для обработки данных
__global__ void processBufferKernel(float* data, size_t size, float multiplier) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        data[idx] *= multiplier;
    }
}
#endif

using namespace Engine3D;

class CudaVulkanInteropDemo {
public:
    CudaVulkanInteropDemo() = default;
    ~CudaVulkanInteropDemo() = default;

    bool initialize() {
        try {
            std::cout << "=== CUDA-Vulkan Interop Demo ===" << std::endl;
            
            // Проверяем поддержку interop
            if (!CUDA::CudaInterop::isInteropSupported()) {
                std::cerr << "CUDA-Vulkan interop не поддерживается на данной системе" << std::endl;
                return false;
            }
            
            std::cout << "✓ CUDA-Vulkan interop поддерживается" << std::endl;
            
            // Инициализируем Vulkan Engine
            vulkanEngine = std::make_unique<Vulkan::VulkanEngine>();
            if (!vulkanEngine->init(VK_NULL_HANDLE)) {
                std::cerr << "Ошибка инициализации Vulkan Engine" << std::endl;
                return false;
            }
            
            std::cout << "✓ Vulkan Engine инициализирован" << std::endl;
            
            // Получаем ResourceManager
            // TODO: Добавить метод получения ResourceManager из VulkanEngine
            
            // Инициализируем CUDA Interop
            cudaInterop = std::make_unique<CUDA::CudaInterop>();
            // TODO: Передать реальные Vulkan объекты
            // if (!cudaInterop->initializeInterop(device, physicalDevice, resourceManager)) {
            //     std::cerr << "Ошибка инициализации CUDA Interop" << std::endl;
            //     return false;
            // }
            
            std::cout << "✓ CUDA Interop готов к инициализации" << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка инициализации: " << e.what() << std::endl;
            return false;
        }
    }

    void runDemo() {
        if (!initialize()) {
            std::cerr << "Инициализация не удалась" << std::endl;
            return;
        }

        std::cout << "\n=== Демонстрация возможностей ===" << std::endl;
        
        // Демо 1: Проверка поддержки
        demonstrateCapabilities();
        
        // Демо 2: Создание shared буфера (пока заглушка)
        demonstrateSharedBuffer();
        
        // Демо 3: Синхронизация (пока заглушка)
        demonstrateSynchronization();
        
        // Демо 4: Обработка данных (пока заглушка)
        demonstrateDataProcessing();
        
        cleanup();
    }

private:
    std::unique_ptr<Vulkan::VulkanEngine> vulkanEngine;
    std::unique_ptr<CUDA::CudaInterop> cudaInterop;
    
    void demonstrateCapabilities() {
        std::cout << "\n--- Проверка возможностей системы ---" << std::endl;
        
        // Проверяем Vulkan возможности
        std::cout << "Vulkan:" << std::endl;
        std::cout << "  - Инициализирован: " << (vulkanEngine ? "Да" : "Нет") << std::endl;
        
        // Проверяем CUDA возможности
        std::cout << "CUDA:" << std::endl;
        std::cout << "  - Interop поддерживается: " << 
                     (CUDA::CudaInterop::isInteropSupported() ? "Да" : "Нет") << std::endl;
        
        if (cudaInterop) {
            std::cout << "  - Инициализирован: " << 
                         (cudaInterop->isInitialized() ? "Да" : "Нет") << std::endl;
            std::cout << "  - Возможности: " << cudaInterop->getInteropCapabilities() << std::endl;
        }
        
        #ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        // Получаем информацию о CUDA устройствах
        int deviceCount = 0;
        cudaError_t result = cudaGetDeviceCount(&deviceCount);
        if (result == cudaSuccess) {
            std::cout << "  - Количество CUDA устройств: " << deviceCount << std::endl;
            
            for (int i = 0; i < deviceCount; ++i) {
                cudaDeviceProp prop;
                cudaGetDeviceProperties(&prop, i);
                std::cout << "    Device " << i << ": " << prop.name << std::endl;
                
                // Проверяем поддержку external memory
                int supportsExternalMemory = 0;
                cudaDeviceGetAttribute(&supportsExternalMemory, 
                                     cudaDevAttrExternalMemorySupport, i);
                std::cout << "      External Memory: " << 
                             (supportsExternalMemory ? "Да" : "Нет") << std::endl;
                
                // Проверяем поддержку external semaphores
                int supportsExternalSemaphore = 0;
                cudaDeviceGetAttribute(&supportsExternalSemaphore, 
                                     cudaDevAttrExternalSemaphoreSupport, i);
                std::cout << "      External Semaphore: " << 
                             (supportsExternalSemaphore ? "Да" : "Нет") << std::endl;
            }
        }
        #endif
    }
    
    void demonstrateSharedBuffer() {
        std::cout << "\n--- Демонстрация Shared Buffer ---" << std::endl;
        
        if (!cudaInterop || !cudaInterop->isInitialized()) {
            std::cout << "CUDA Interop не инициализирован, пропускаем демо" << std::endl;
            return;
        }
        
        const size_t bufferSize = 1024 * sizeof(float);
        
        std::cout << "Создание shared буфера размером " << bufferSize << " байт..." << std::endl;
        
        // Создаем shared буфер
        auto sharedResource = cudaInterop->createSharedBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eStorageBuffer,
            cudaMemAttachGlobal);
        
        if (sharedResource && sharedResource->isValid) {
            std::cout << "✓ Shared буфер создан успешно" << std::endl;
            std::cout << "  - Vulkan buffer: " << static_cast<VkBuffer>(sharedResource->vulkanBuffer) << std::endl;
            std::cout << "  - CUDA device ptr: 0x" << std::hex << sharedResource->cudaDevicePtr << std::dec << std::endl;
            
            // Освобождаем ресурс
            cudaInterop->freeSharedResource(sharedResource);
            std::cout << "✓ Shared буфер освобожден" << std::endl;
        } else {
            std::cout << "✗ Ошибка создания shared буфера" << std::endl;
        }
    }
    
    void demonstrateSynchronization() {
        std::cout << "\n--- Демонстрация синхронизации ---" << std::endl;
        
        if (!cudaInterop || !cudaInterop->isInitialized()) {
            std::cout << "CUDA Interop не инициализирован, пропускаем демо" << std::endl;
            return;
        }
        
        std::cout << "Создание объекта синхронизации..." << std::endl;
        
        auto syncObject = cudaInterop->createSyncObject();
        
        if (syncObject && syncObject->isValid) {
            std::cout << "✓ Объект синхронизации создан успешно" << std::endl;
            std::cout << "  - Vulkan semaphore: " << static_cast<VkSemaphore>(syncObject->vulkanSemaphore) << std::endl;
            std::cout << "  - CUDA external semaphore: " << syncObject->cudaExternalSemaphore << std::endl;
            
            // Демонстрируем синхронизацию
            std::cout << "Тестирование синхронизации Vulkan -> CUDA..." << std::endl;
            cudaInterop->signalVulkanToCuda(syncObject);
            std::cout << "✓ Сигнал отправлен" << std::endl;
            
            // TODO: Добавить реальное тестирование с command buffer
            
        } else {
            std::cout << "✗ Ошибка создания объекта синхронизации" << std::endl;
        }
    }
    
    void demonstrateDataProcessing() {
        std::cout << "\n--- Демонстрация обработки данных ---" << std::endl;
        
        #ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (!cudaInterop || !cudaInterop->isInitialized()) {
            std::cout << "CUDA Interop не инициализирован, пропускаем демо" << std::endl;
            return;
        }
        
        const size_t dataSize = 1024;
        const size_t bufferSize = dataSize * sizeof(float);
        
        std::cout << "Создание буфера для обработки " << dataSize << " элементов..." << std::endl;
        
        // Создаем shared буфер
        auto sharedResource = cudaInterop->createSharedBuffer(bufferSize);
        
        if (sharedResource && sharedResource->isValid) {
            std::cout << "✓ Буфер создан, заполняем тестовыми данными..." << std::endl;
            
            // Инициализируем данные на GPU
            std::vector<float> hostData(dataSize);
            for (size_t i = 0; i < dataSize; ++i) {
                hostData[i] = static_cast<float>(i);
            }
            
            // Копируем данные в CUDA буфер
            cudaError_t result = cudaMemcpy(
                reinterpret_cast<void*>(sharedResource->cudaDevicePtr),
                hostData.data(),
                bufferSize,
                cudaMemcpyHostToDevice);
            
            if (result == cudaSuccess) {
                std::cout << "✓ Данные скопированы в shared буфер" << std::endl;
                
                // Обрабатываем данные CUDA kernel'ом
                const float multiplier = 2.0f;
                const int blockSize = 256;
                const int gridSize = (dataSize + blockSize - 1) / blockSize;
                
                std::cout << "Запуск CUDA kernel для обработки данных..." << std::endl;
                processBufferKernel<<<gridSize, blockSize>>>(
                    reinterpret_cast<float*>(sharedResource->cudaDevicePtr),
                    dataSize,
                    multiplier);
                
                // Ждем завершения
                result = cudaDeviceSynchronize();
                if (result == cudaSuccess) {
                    std::cout << "✓ CUDA обработка завершена" << std::endl;
                    
                    // Копируем результат обратно для проверки
                    std::vector<float> resultData(dataSize);
                    result = cudaMemcpy(
                        resultData.data(),
                        reinterpret_cast<void*>(sharedResource->cudaDevicePtr),
                        bufferSize,
                        cudaMemcpyDeviceToHost);
                    
                    if (result == cudaSuccess) {
                        std::cout << "✓ Результат получен" << std::endl;
                        
                        // Проверяем несколько значений
                        bool success = true;
                        for (size_t i = 0; i < std::min(size_t(10), dataSize); ++i) {
                            float expected = hostData[i] * multiplier;
                            if (std::abs(resultData[i] - expected) > 0.001f) {
                                success = false;
                                break;
                            }
                        }
                        
                        if (success) {
                            std::cout << "✓ Обработка данных прошла успешно!" << std::endl;
                            std::cout << "  Пример: " << hostData[0] << " * " << multiplier 
                                      << " = " << resultData[0] << std::endl;
                        } else {
                            std::cout << "✗ Ошибка в результате обработки" << std::endl;
                        }
                    } else {
                        std::cout << "✗ Ошибка копирования результата: " 
                                  << cudaGetErrorString(result) << std::endl;
                    }
                } else {
                    std::cout << "✗ Ошибка выполнения CUDA kernel: " 
                              << cudaGetErrorString(result) << std::endl;
                }
            } else {
                std::cout << "✗ Ошибка копирования данных: " 
                          << cudaGetErrorString(result) << std::endl;
            }
            
            // Освобождаем ресурс
            cudaInterop->freeSharedResource(sharedResource);
        } else {
            std::cout << "✗ Ошибка создания буфера для обработки" << std::endl;
        }
        
        #else
        std::cout << "CUDA поддержка не включена в сборку" << std::endl;
        #endif
    }
    
    void cleanup() {
        std::cout << "\n--- Освобождение ресурсов ---" << std::endl;
        
        if (cudaInterop) {
            cudaInterop->cleanup();
            cudaInterop.reset();
            std::cout << "✓ CUDA Interop очищен" << std::endl;
        }
        
        if (vulkanEngine) {
            vulkanEngine->shutdown();
            vulkanEngine.reset();
            std::cout << "✓ Vulkan Engine завершен" << std::endl;
        }
        
        std::cout << "✓ Все ресурсы освобождены" << std::endl;
    }
};

int main() {
    try {
        CudaVulkanInteropDemo demo;
        demo.runDemo();
        
        std::cout << "\n=== Демонстрация завершена ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
}
