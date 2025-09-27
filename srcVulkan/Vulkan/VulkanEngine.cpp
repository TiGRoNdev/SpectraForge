/**
 * @file VulkanEngine.cpp
 * @brief Реализация главного класса Vulkan движка
 * 
 * Координирует работу всех подсистем согласно UML архитектуре из FEATURE_PLAN.
 */

#include "Engine3D/Vulkan/VulkanEngine.h"
#include "Engine3D/Vulkan/VulkanRenderer.h"
#include "Engine3D/Vulkan/SceneManager.h"
#include "Engine3D/Vulkan/ResourceManager.h"
#include "Engine3D/Vulkan/HardwareDetector.h"

#include <iostream>
#include <stdexcept>

using namespace Engine3D::Vulkan;

namespace Engine3D::Vulkan {

VulkanEngine::VulkanEngine() {
    // Инициализация в init()
}

VulkanEngine::~VulkanEngine() {
    if (initialized) {
        shutdown();
    }
}

bool VulkanEngine::init(vk::Instance instance) {
    try {
        std::cout << "[VulkanEngine] Инициализация движка..." << std::endl;
        
        // 1. Создаем детектор железа
        hardwareDetector = std::make_unique<HardwareDetector>();
        
        // Получаем физические устройства
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty()) {
            std::cerr << "[VulkanEngine] Ошибка: Не найдено Vulkan-совместимых устройств" << std::endl;
            return false;
        }
        
        // Выбираем первое подходящее устройство (можно улучшить логику выбора)
        vk::PhysicalDevice selectedDevice = physicalDevices[0];
        
        if (!hardwareDetector->init(selectedDevice)) {
            std::cerr << "[VulkanEngine] Ошибка инициализации детектора железа" << std::endl;
            return false;
        }
        
        // Выводим информацию о выбранном устройстве
        std::cout << "[VulkanEngine] Выбрано устройство: " << hardwareDetector->getDeviceName() << std::endl;
        std::cout << "[VulkanEngine] Вендор: ";
        switch (hardwareDetector->detectVendor()) {
            case VendorType::NVIDIA: std::cout << "NVIDIA"; break;
            case VendorType::AMD: std::cout << "AMD"; break;
            case VendorType::INTEL: std::cout << "Intel"; break;
            default: std::cout << "Другой"; break;
        }
        std::cout << std::endl;
        
        std::cout << "[VulkanEngine] Ray Tracing: " << (hardwareDetector->supportsRayTracing() ? "Поддерживается" : "Не поддерживается") << std::endl;
        std::cout << "[VulkanEngine] CUDA: " << (hardwareDetector->supportsCUDA() ? "Поддерживается" : "Не поддерживается") << std::endl;
        
        // 2. Создаем логическое устройство (упрощенная версия)
        // В полной реализации здесь будет создание device с нужными расширениями
        
        // 3. Создаем менеджер ресурсов
        resourceManager = std::make_unique<ResourceManager>();
        // Пока не инициализируем, так как нужно логическое устройство
        
        // 4. Создаем менеджер сцены
        sceneManager = std::make_unique<SceneManager>();
        if (!sceneManager->init()) {
            std::cerr << "[VulkanEngine] Ошибка инициализации SceneManager" << std::endl;
            return false;
        }
        
        // 5. Создаем рендерер
        renderer = std::make_unique<VulkanRenderer>();
        
        initialized = true;
        std::cout << "[VulkanEngine] Инициализация завершена успешно" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[VulkanEngine] Ошибка инициализации: " << e.what() << std::endl;
        return false;
    }
}

void VulkanEngine::renderFrame(const CameraParams& params) {
    if (!initialized) {
        std::cerr << "[VulkanEngine] Ошибка: Движок не инициализирован" << std::endl;
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
        std::cerr << "[VulkanEngine] Ошибка рендеринга: " << e.what() << std::endl;
    }
}

void VulkanEngine::shutdown() {
    if (!initialized) {
        return;
    }
    
    std::cout << "[VulkanEngine] Завершение работы движка..." << std::endl;
    
    // Освобождаем ресурсы в обратном порядке создания
    renderer.reset();
    sceneManager.reset();
    resourceManager.reset();
    hardwareDetector.reset();
    
    initialized = false;
    std::cout << "[VulkanEngine] Завершение работы завершено" << std::endl;
}

} // namespace Engine3D::Vulkan
