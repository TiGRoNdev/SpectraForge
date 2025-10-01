/**
 * @file VulkanEngine.cpp
 * @brief Реализация главного класса Vulkan движка
 *
 * Координирует работу всех подсистем согласно UML архитектуре из FEATURE_PLAN.
 */

#include "HyperEngine/Vulkan/VulkanEngine.h"
#include "HyperEngine/Vulkan/HardwareDetector.h"
#include "HyperEngine/Vulkan/ResourceManager.h"
#include "HyperEngine/Vulkan/SceneManager.h"
#include "HyperEngine/Vulkan/VulkanRenderer.h"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include "HyperEngine/Core/SafeConsole.h"
#include "HyperEngine/Core/Console.h"

// Define Vulkan-Hpp default dispatcher storage (required for dynamic loader)
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

using namespace HyperEngine::Vulkan;
using namespace HyperEngine::Core;

namespace HyperEngine::Vulkan {

VulkanEngine::VulkanEngine() {
    // Инициализация в init()
}

VulkanEngine::~VulkanEngine() {
    if (initialized) {
        shutdown();
    }
}

bool VulkanEngine::init(vk::Instance vulkanInstance) {
    try {
        SAFE_PRINT_LINE("[VulkanEngine] Инициализация движка...");

        // Сохраняем instance
        this->instance = vulkanInstance;

        // 1. Создаем детектор железа
        hardwareDetector = std::make_unique<HardwareDetector>();

        // Проверяем валидность instance
        if (!instance) {
            SAFE_ERROR("[VulkanEngine] Ошибка: Передан невалидный Vulkan instance");
            return false;
        }

        // Получаем физические устройства
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty()) {
            SAFE_ERROR("[VulkanEngine] Ошибка: Не найдено Vulkan-совместимых устройств");
            return false;
        }

        // Выбираем первое подходящее устройство (можно улучшить логику выбора)
        physicalDevice = physicalDevices[0];

        if (!hardwareDetector->init(physicalDevice)) {
            SAFE_ERROR("[VulkanEngine] Ошибка инициализации детектора железа");
            return false;
        }

        // Выводим информацию о выбранном устройстве
        SAFE_PRINT_LINE("[VulkanEngine] Выбрано устройство: "
                        + SAFE_TO_STRING(hardwareDetector->getDeviceName()));

        std::string vendorName;
        switch (hardwareDetector->detectVendor()) {
            case VendorType::NVIDIA:
                vendorName = "NVIDIA";
                break;
            case VendorType::AMD:
                vendorName = "AMD";
                break;
            case VendorType::INTEL:
                vendorName = "Intel";
                break;
            default:
                vendorName = "Другой";
                break;
        }
        SAFE_PRINT_LINE("[VulkanEngine] Вендор: " + SAFE_TO_STRING(vendorName));

        SAFE_PRINT_LINE("[VulkanEngine] Ray Tracing: "
                        + SAFE_TO_STRING(hardwareDetector->supportsRayTracing()
                                             ? "Поддерживается"
                                             : "Не поддерживается"));
        SAFE_PRINT_LINE("[VulkanEngine] CUDA: "
                        + SAFE_TO_STRING(hardwareDetector->supportsCUDA() ? "Поддерживается"
                                                                          : "Не поддерживается"));

        // 2. Создаем логическое устройство (упрощенная версия)
        device = createLogicalDevice();

        // 3. Создаем менеджер ресурсов
        resourceManager = std::make_unique<ResourceManager>();
        if (!resourceManager->init(physicalDevice, device, instance)) {
            SAFE_ERROR("[VulkanEngine] Ошибка инициализации ResourceManager");
            return false;
        }

        // 4. Создаем менеджер сцены
        sceneManager = std::make_unique<SceneManager>();
        if (!sceneManager->init()) {
            SAFE_ERROR("[VulkanEngine] Ошибка инициализации SceneManager");
            return false;
        }

        // 5. Создаем рендерер
        renderer = std::make_unique<VulkanRenderer>();

        initialized = true;
        SAFE_PRINT_LINE("[VulkanEngine] Инициализация завершена успешно");
        return true;

    } catch (const std::exception& e) {
        SAFE_ERROR("[VulkanEngine] Ошибка инициализации: " + SAFE_TO_STRING(e.what()));
        return false;
    }
}

void VulkanEngine::renderFrame(const CameraParams& params) {
    // Подавляем предупреждение о неиспользуемом параметре
    (void)params;

    if (!initialized) {
        SAFE_ERROR("[VulkanEngine] Ошибка: Движок не инициализирован");
        return;
    }

    try {
        // Согласно UML архитектуре, выполняем этапы рендеринга:

        // 1. Обновляем сцену
        if (sceneManager) {
            sceneManager->updateDynamics();
        }

        // 2. Выполняем рендеринг через renderer
        if (renderer) {
            // В полной реализации здесь будет:
            // - rasterizePrimary() для Gaussian Splatting
            // - rayTraceSecondary() для вторичных эффектов
            // - denoiseAI() для деноизинга
            // - upscale() для апскейлинга
            // - presentFinalImage() для вывода

            // Пока заглушка
            // renderer->renderFrame(params);
        }

    } catch (const std::exception& e) {
        SAFE_ERROR("[VulkanEngine] Ошибка рендеринга: " + SAFE_TO_STRING(e.what()));
    }
}

void VulkanEngine::shutdown() {
    if (!initialized) {
        return;
    }

    SAFE_PRINT_LINE("[VulkanEngine] Завершение работы движка...");

    // Освобождаем ресурсы в обратном порядке создания
    renderer.reset();
    sceneManager.reset();
    resourceManager.reset();
    hardwareDetector.reset();

    // Освобождаем Vulkan ресурсы
    if (device) {
        device.destroy();
        device = vk::Device{};
    }

    initialized = false;
    SAFE_PRINT_LINE("[VulkanEngine] Завершение работы завершено");
}

vk::Device VulkanEngine::createLogicalDevice() {
    try {
        // Получаем семейства очередей
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        // Ищем семейство очередей с поддержкой графики
        uint32_t graphicsQueueFamily = UINT32_MAX;
        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsQueueFamily = i;
                break;
            }
        }

        if (graphicsQueueFamily == UINT32_MAX) {
            throw std::runtime_error("Не найдено семейство очередей с поддержкой графики");
        }

        // Создаем очередь
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Создаем логическое устройство
        vk::DeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

        // Включаем базовые расширения
        std::vector<const char*> deviceExtensions;

        // Проверяем доступность external memory расширений для CUDA interop
        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool hasExternalMemory = false;
        bool hasExternalSemaphore = false;

        for (const auto& ext : availableExtensions) {
            if (strcmp(ext.extensionName, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME) == 0) {
                hasExternalMemory = true;
            }
            if (strcmp(ext.extensionName, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME) == 0) {
                hasExternalSemaphore = true;
            }
        }

        if (hasExternalMemory) {
            deviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
#ifdef _WIN32
            deviceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
#endif
            SAFE_PRINT_LINE("[VulkanEngine] External memory расширения включены");
        }

        if (hasExternalSemaphore) {
            deviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
#ifdef _WIN32
            deviceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
#endif
            SAFE_PRINT_LINE("[VulkanEngine] External semaphore расширения включены");
        }

        if (!deviceExtensions.empty()) {
            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        vk::Device logicalDevice = physicalDevice.createDevice(deviceCreateInfo);
        SAFE_PRINT_LINE("[VulkanEngine] Логическое устройство создано успешно");

        return logicalDevice;

    } catch (const std::exception& e) {
        SAFE_ERROR("[VulkanEngine] Ошибка создания логического устройства: "
                   + SAFE_TO_STRING(e.what()));
        return vk::Device{};
    }
}

}  // namespace HyperEngine::Vulkan
