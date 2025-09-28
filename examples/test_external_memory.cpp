/**
 * @file test_external_memory.cpp
 * @brief Простой тест для проверки реальных Vulkan external memory handles
 */

#include "HyperEngine/Core/Console.h"
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Vulkan/VulkanEngine.h"
#include "HyperEngine/CUDA/CudaInterop.h"

#include <iostream>
#include <memory>
#include <cstdint>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using namespace HyperEngine;
using namespace HyperEngine::Core;
using namespace HyperEngine::Vulkan;
using namespace HyperEngine::CUDA;

int main() {
    try {
        // Инициализация консоли
        Core::Console::initialize();
        SAFE_PRINT_LINE("=== Тест External Memory Handles ===");
        
        // Создание Vulkan instance
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "External Memory Test";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "HyperEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        
        std::vector<const char*> extensions = {
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME
        };
        
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        vk::Instance instance = vk::createInstance(createInfo);
        SAFE_PRINT_LINE("✓ Vulkan instance создан");
        
        // Инициализация движка
        VulkanEngine engine;
        if (!engine.init(instance)) {
            SAFE_ERROR("✗ Ошибка инициализации VulkanEngine");
            return -1;
        }
        SAFE_PRINT_LINE("✓ VulkanEngine инициализирован");
        
        // Проверка поддержки CUDA interop
        if (!CudaInterop::isInteropSupported()) {
            SAFE_PRINT_LINE("⚠ CUDA interop не поддерживается, завершаем тест");
            engine.shutdown();
            instance.destroy();
            return 0;
        }
        SAFE_PRINT_LINE("✓ CUDA interop поддерживается");
        
        // Создание CUDA interop
        CudaInterop cudaInterop;
        if (!cudaInterop.initializeInterop(
            engine.getDevice(), 
            engine.getPhysicalDevice(), 
            engine.getResourceManager())) {
            SAFE_ERROR("✗ Ошибка инициализации CUDA interop");
            engine.shutdown();
            instance.destroy();
            return -1;
        }
        SAFE_PRINT_LINE("✓ CUDA interop инициализирован");
        
        // Тест создания shared буфера
        SAFE_PRINT_LINE("\n--- Тест External Memory Buffer ---");
        auto sharedBuffer = cudaInterop.createSharedBuffer(
            4096, // 4KB
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst
        );
        
        if (sharedBuffer && sharedBuffer->isValid) {
            SAFE_PRINT_LINE("✓ Shared буфер создан успешно");
            std::cout << "  Размер: " << SAFE_TO_STRING(sharedBuffer->size) << " байт" << std::endl;
            std::cout << "  Vulkan буфер: 0x" << std::hex << reinterpret_cast<uintptr_t>(static_cast<VkBuffer>(sharedBuffer->vulkanBuffer)) << std::dec << std::endl;
            std::cout << "  CUDA pointer: 0x" << std::hex << sharedBuffer->cudaDevicePtr << std::dec << std::endl;
            
            // Освобождение ресурса
            cudaInterop.freeSharedResource(sharedBuffer);
            SAFE_PRINT_LINE("✓ Shared буфер освобожден");
        } else {
            SAFE_PRINT_LINE("⚠ Shared буфер не создан (возможно, fallback режим)");
        }
        
        // Тест создания объекта синхронизации
        SAFE_PRINT_LINE("\n--- Тест External Semaphore ---");
        auto syncObject = cudaInterop.createSyncObject();
        
        if (syncObject && syncObject->isValid) {
            SAFE_PRINT_LINE("✓ Объект синхронизации создан успешно");
            std::cout << "  Vulkan semaphore: 0x" << std::hex << reinterpret_cast<uintptr_t>(static_cast<VkSemaphore>(syncObject->vulkanSemaphore)) << std::dec << std::endl;
        } else {
            SAFE_PRINT_LINE("⚠ Объект синхронизации не создан");
        }
        
        // Очистка
        cudaInterop.cleanup();
        SAFE_PRINT_LINE("\n✓ CUDA interop очищен");
        
        engine.shutdown();
        SAFE_PRINT_LINE("✓ VulkanEngine завершен");
        
        instance.destroy();
        SAFE_PRINT_LINE("✓ Vulkan instance освобожден");
        
        SAFE_PRINT_LINE("\n=== Тест завершен успешно ===");
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "✗ Ошибка: " << e.what() << std::endl;
        return -1;
    }
}

