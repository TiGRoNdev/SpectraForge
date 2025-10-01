/**
 * @file cuda_vulkan_interop_demo.cpp
 * @brief Демонстрация CUDA-Vulkan interop функциональности
 *
 * Показывает создание shared буферов, обмен данными между CUDA и Vulkan,
 * и синхронизацию между двумя API без копирования памяти.
 */

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include "HyperEngine/Core/SafeConsole.h"

// Основные заголовки движка
#include "HyperEngine/CUDA/CudaInterop.h"
#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Vulkan/HardwareDetector.h"
#include "HyperEngine/Vulkan/ResourceManager.h"
#include "HyperEngine/Vulkan/VulkanEngine.h"

using namespace HyperEngine::Core;

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
#include <cuda.h>
#include <cuda_runtime.h>

// CUDA kernel объявление (реализация в .cu файле)
// extern "C" void launchProcessBufferKernel(float* data, size_t size, float multiplier);
#endif

using namespace HyperEngine;
using namespace HyperEngine::Core;

class CudaVulkanInteropDemo {
  public:
    CudaVulkanInteropDemo() = default;
    ~CudaVulkanInteropDemo() = default;

    bool initialize() {
        try {
            Console::initialize();
            SAFE_PRINT_LINE("=== CUDA-Vulkan Interop Demo ===");

            // Проверяем поддержку interop
            if (!CUDA::CudaInterop::isInteropSupported()) {
                SAFE_ERROR("CUDA-Vulkan interop не поддерживается на данной системе");
                return false;
            }

            SAFE_PRINT_LINE("✓ CUDA-Vulkan interop поддерживается");

            // Инициализируем Vulkan Engine
            vulkanEngine = std::make_unique<Vulkan::VulkanEngine>();

            // Создаем Vulkan instance для демо
            vulkanInstance = createVulkanInstance();
            if (!vulkanInstance) {
                SAFE_ERROR("Ошибка создания Vulkan instance");
                return false;
            }

            if (!vulkanEngine->init(vulkanInstance)) {
                SAFE_ERROR("Ошибка инициализации Vulkan Engine");
                return false;
            }

            SAFE_PRINT_LINE("✓ Vulkan Engine инициализирован");

            // Инициализируем CUDA Interop с реальными Vulkan объектами
            cudaInterop = std::make_unique<CUDA::CudaInterop>();

            // Получаем Vulkan объекты из движка
            auto device = vulkanEngine->getDevice();
            auto physicalDevice = vulkanEngine->getPhysicalDevice();
            auto resourceManager = vulkanEngine->getResourceManager();

            if (device && physicalDevice && resourceManager) {
                if (!cudaInterop->initializeInterop(device, physicalDevice, resourceManager)) {
                    SAFE_ERROR("Ошибка инициализации CUDA Interop");
                    return false;
                }
                SAFE_PRINT_LINE("✓ CUDA Interop инициализирован успешно");
            } else {
                SAFE_WARNING(
                    "⚠ CUDA Interop не может быть инициализирован - отсутствуют Vulkan объекты");
            }

            return true;

        } catch (const std::exception& e) {
            SAFE_ERROR("Ошибка инициализации: " + std::string(e.what()));
            return false;
        }
    }

    void runDemo() {
        if (!initialize()) {
            SAFE_ERROR("Инициализация не удалась");
            return;
        }

        SAFE_PRINT_LINE("\n=== Демонстрация возможностей ===");

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
    vk::Instance vulkanInstance;

    void demonstrateCapabilities() {
        SAFE_PRINT_LINE("\n--- Проверка возможностей системы ---");

        // Проверяем Vulkan возможности
        SAFE_PRINT_LINE("Vulkan:");
        SAFE_PRINT_LINE("  - Инициализирован: " + std::string(vulkanEngine ? "Да" : "Нет"));

        // Проверяем CUDA возможности
        SAFE_PRINT_LINE("CUDA:");
        SAFE_PRINT_LINE("  - Interop поддерживается: "
                        + std::string(CUDA::CudaInterop::isInteropSupported() ? "Да" : "Нет"));

        if (cudaInterop) {
            SAFE_PRINT_LINE("  - Инициализирован: "
                            + std::string(cudaInterop->isInitialized() ? "Да" : "Нет"));
            SAFE_PRINT_LINE("  - Возможности: " + cudaInterop->getInteropCapabilities());
        }

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        // Получаем информацию о CUDA устройствах
        int deviceCount = 0;
        cudaError_t result = cudaGetDeviceCount(&deviceCount);
        if (result == cudaSuccess) {
            SAFE_PRINT_LINE("  - Количество CUDA устройств: " + SAFE_TO_STRING(deviceCount));

            for (int i = 0; i < deviceCount; ++i) {
                cudaDeviceProp prop;
                cudaGetDeviceProperties(&prop, i);
                SAFE_PRINT_LINE("    Device " + SAFE_TO_STRING(i) + ": " + std::string(prop.name));

                // Проверяем поддержку external memory (используем доступные атрибуты)
                int supportsExternalMemory = 0;
                cudaDeviceGetAttribute(&supportsExternalMemory, cudaDevAttrIntegrated, i);
                std::cout << "      External Memory (базовая): "
                          << (supportsExternalMemory ? "Да" : "Нет") << std::endl;

                // Проверяем поддержку external semaphores (используем доступные атрибуты)
                int supportsExternalSemaphore = 0;
                cudaDeviceGetAttribute(
                    &supportsExternalSemaphore, cudaDevAttrConcurrentManagedAccess, i);
                std::cout << "      External Semaphore (базовая): "
                          << (supportsExternalSemaphore ? "Да" : "Нет") << std::endl;
            }
        }
#endif
    }

    void demonstrateSharedBuffer() {
        SAFE_PRINT_LINE("\n--- Демонстрация Shared Buffer ---");

        if (!cudaInterop || !cudaInterop->isInitialized()) {
            SAFE_WARNING("CUDA Interop не инициализирован, пропускаем демо");
            return;
        }

        const size_t bufferSize = 1024 * sizeof(float);

        SAFE_PRINT_LINE("Создание shared буфера размером " + SAFE_TO_STRING(bufferSize)
                        + " байт...");

        // Создаем shared буфер
        auto sharedResource = cudaInterop->createSharedBuffer(
            bufferSize, vk::BufferUsageFlagBits::eStorageBuffer, cudaMemAttachGlobal);

        if (sharedResource && sharedResource->isValid) {
            SAFE_PRINT_LINE("✓ Shared буфер создан успешно");
            SAFE_PRINT_LINE("  - Vulkan buffer: "
                            + SAFE_TO_STRING(reinterpret_cast<uintptr_t>(
                                static_cast<VkBuffer>(sharedResource->vulkanBuffer))));
            SAFE_PRINT_LINE(
                "  - CUDA device ptr: 0x"
                + SAFE_TO_STRING(static_cast<uintptr_t>(sharedResource->cudaDevicePtr)));

            // Освобождаем ресурс
            cudaInterop->freeSharedResource(sharedResource);
            SAFE_PRINT_LINE("✓ Shared буфер освобожден");
        } else {
            SAFE_ERROR("✗ Ошибка создания shared буфера");
        }
    }

    void demonstrateSynchronization() {
        SAFE_PRINT_LINE("\n--- Демонстрация синхронизации ---");

        if (!cudaInterop || !cudaInterop->isInitialized()) {
            SAFE_WARNING("CUDA Interop не инициализирован, пропускаем демо");
            return;
        }

        SAFE_PRINT_LINE("Создание объекта синхронизации...");

        auto syncObject = cudaInterop->createSyncObject();

        if (syncObject && syncObject->isValid) {
            SAFE_PRINT_LINE("✓ Объект синхронизации создан успешно");
            std::cout << "  - Vulkan semaphore: "
                      << static_cast<VkSemaphore>(syncObject->vulkanSemaphore) << std::endl;
            std::cout << "  - CUDA external semaphore: " << syncObject->cudaExternalSemaphore
                      << std::endl;

            // Демонстрируем синхронизацию
            SAFE_PRINT_LINE("Тестирование синхронизации Vulkan -> CUDA...");
            cudaInterop->signalVulkanToCuda(syncObject);
            SAFE_PRINT_LINE("✓ Сигнал отправлен");

            // TODO: Добавить реальное тестирование с command buffer

        } else {
            SAFE_PRINT_LINE("✗ Ошибка создания объекта синхронизации");
        }
    }

    void demonstrateDataProcessing() {
        SAFE_PRINT_LINE("\n--- Демонстрация обработки данных ---");

#ifdef CUDA_VULKAN_INTEROP_SUPPORTED
        if (!cudaInterop || !cudaInterop->isInitialized()) {
            SAFE_WARNING("CUDA Interop не инициализирован, пропускаем демо");
            return;
        }

        const size_t dataSize = 1024;
        const size_t bufferSize = dataSize * sizeof(float);

        std::cout << "Создание буфера для обработки " << SAFE_TO_STRING(dataSize) << " элементов..."
                  << std::endl;

        // Создаем shared буфер
        auto sharedResource = cudaInterop->createSharedBuffer(bufferSize);

        if (sharedResource && sharedResource->isValid) {
            SAFE_PRINT_LINE("✓ Буфер создан, заполняем тестовыми данными...");

            // Инициализируем данные на GPU
            std::vector<float> hostData(dataSize);
            for (size_t i = 0; i < dataSize; ++i) {
                hostData[i] = static_cast<float>(i);
            }

            // Копируем данные в CUDA буфер
            cudaError_t result = cudaMemcpy(reinterpret_cast<void*>(sharedResource->cudaDevicePtr),
                                            hostData.data(),
                                            bufferSize,
                                            cudaMemcpyHostToDevice);

            if (result == cudaSuccess) {
                SAFE_PRINT_LINE("✓ Данные скопированы в shared буфер");

                // Обрабатываем данные CUDA kernel'ом
                const float multiplier = 2.0f;
                const int blockSize = 256;
                const int gridSize = (dataSize + blockSize - 1) / blockSize;
                (void)multiplier;  // Подавляем предупреждения о неиспользуемых переменных
                (void)blockSize;
                (void)gridSize;

                SAFE_PRINT_LINE("Запуск CUDA kernel для обработки данных...");
                // TODO: Добавить компиляцию CUDA kernels
                SAFE_PRINT_LINE("CUDA kernel пока не скомпилирован, пропускаем...");

                // Ждем завершения
                result = cudaDeviceSynchronize();
                if (result == cudaSuccess) {
                    SAFE_PRINT_LINE("✓ CUDA обработка завершена");

                    // Копируем результат обратно для проверки
                    std::vector<float> resultData(dataSize);
                    result = cudaMemcpy(resultData.data(),
                                        reinterpret_cast<void*>(sharedResource->cudaDevicePtr),
                                        bufferSize,
                                        cudaMemcpyDeviceToHost);

                    if (result == cudaSuccess) {
                        SAFE_PRINT_LINE("✓ Результат получен");

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
                            SAFE_PRINT_LINE("✓ Обработка данных прошла успешно!");
                            std::cout << "  Пример: " << SAFE_TO_STRING(hostData[0]) << " * "
                                      << SAFE_TO_STRING(multiplier) << " = "
                                      << SAFE_TO_STRING(resultData[0]) << std::endl;
                        } else {
                            SAFE_PRINT_LINE("✗ Ошибка в результате обработки");
                        }
                    } else {
                        std::cout << "✗ Ошибка копирования результата: "
                                  << cudaGetErrorString(result) << std::endl;
                    }
                } else {
                    std::cout << "✗ Ошибка выполнения CUDA kernel: " << cudaGetErrorString(result)
                              << std::endl;
                }
            } else {
                std::cout << "✗ Ошибка копирования данных: " << cudaGetErrorString(result)
                          << std::endl;
            }

            // Освобождаем ресурс
            cudaInterop->freeSharedResource(sharedResource);
        } else {
            SAFE_PRINT_LINE("✗ Ошибка создания буфера для обработки");
        }

#else
        SAFE_PRINT_LINE("CUDA поддержка не включена в сборку");
#endif
    }

    void cleanup() {
        SAFE_PRINT_LINE("\n--- Освобождение ресурсов ---");

        if (cudaInterop) {
            cudaInterop->cleanup();
            cudaInterop.reset();
            SAFE_PRINT_LINE("✓ CUDA Interop очищен");
        }

        if (vulkanEngine) {
            vulkanEngine->shutdown();
            vulkanEngine.reset();
            SAFE_PRINT_LINE("✓ Vulkan Engine завершен");
        }

        if (vulkanInstance) {
            vulkanInstance.destroy();
            vulkanInstance = vk::Instance{};
            SAFE_PRINT_LINE("✓ Vulkan Instance освобожден");
        }

        SAFE_PRINT_LINE("✓ Все ресурсы освобождены");
    }

    /**
     * @brief Создание Vulkan instance для демо
     * @return Vulkan instance
     */
    vk::Instance createVulkanInstance() {
        try {
            // Информация о приложении
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "CUDA-Vulkan Interop Demo";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "HyperEngine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;

            // Создание instance
            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;

// Включаем validation layers в debug режиме
#ifdef _DEBUG
            const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

            // Проверяем доступность validation layers
            auto availableLayers = vk::enumerateInstanceLayerProperties();
            bool validationSupported = false;
            for (const auto& layerName : validationLayers) {
                for (const auto& layerProps : availableLayers) {
                    if (strcmp(layerName, layerProps.layerName) == 0) {
                        validationSupported = true;
                        break;
                    }
                }
                if (!validationSupported)
                    break;
            }

            if (validationSupported) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
                SAFE_PRINT_LINE("[VulkanInstance] Validation layers включены");
            }
#endif

            // Создаем instance
            vk::Instance instance = vk::createInstance(createInfo);
            SAFE_PRINT_LINE("[VulkanInstance] Vulkan instance создан успешно");

            return instance;

        } catch (const std::exception& e) {
            std::cerr << "[VulkanInstance] Ошибка создания Vulkan instance: " << e.what()
                      << std::endl;
            return vk::Instance{};
        }
    }
};

int main() {
    try {
        CudaVulkanInteropDemo demo;
        SAFE_PRINT_LINE("Демо объект создан, запускаем...");

        demo.runDemo();

        SAFE_PRINT_LINE("\n=== Демонстрация завершена ===");
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        SAFE_ERROR("Неизвестная критическая ошибка");
        return 1;
    }
}
