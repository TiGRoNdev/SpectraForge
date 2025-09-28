/**
 * @file vulkan_basic_demo.cpp
 * @brief Базовое демо новой Vulkan архитектуры
 * 
 * Демонстрирует работу основных Vulkan классов согласно UML архитектуре
 * из FEATURE_PLAN этапа 2.1.
 */

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// Vulkan headers
#include <vulkan/vulkan.hpp>

// Engine headers
#include "HyperEngine/Vulkan/VulkanEngine.h"
#include "HyperEngine/Vulkan/HardwareDetector.h"
#include "HyperEngine/Vulkan/ResourceManager.h"
#include "HyperEngine/Vulkan/VulkanRenderer.h"
#include "HyperEngine/Vulkan/SceneManager.h"
#include "HyperEngine/Core/Console.h"

using namespace HyperEngine;
using namespace HyperEngine::Vulkan;
using namespace HyperEngine::Core;

/**
 * @brief Создание базового Vulkan instance
 */
vk::Instance createVulkanInstance() {
    try {
        // Информация о приложении
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Vulkan Basic Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "HyperEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        
        // Создание instance
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        
        // Базовые расширения (без surface для простоты)
        std::vector<const char*> extensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };
        
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        // Validation layers для отладки
        std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
        
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        
        return vk::createInstance(createInfo);
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка создания Vulkan instance: " << e.what() << std::endl;
        throw;
    }
}

/**
 * @brief Главная функция демо
 */
int main() {
    try {
        Console::initialize();

        // Создание Vulkan instance
        SAFE_PRINT_LINE("=== Vulkan Basic Demo ===");
        SAFE_PRINT_LINE("Демонстрация основных Vulkan классов HyperEngine");
        std::cout << std::endl;
        
        // 1. Создаем Vulkan instance
        SAFE_PRINT_LINE("1. Создание Vulkan instance...");
        vk::Instance instance = createVulkanInstance();
        SAFE_PRINT_LINE("   ✓ Vulkan instance создан");
        
        // 2. Создаем и инициализируем VulkanEngine
        std::cout << std::endl << "2. Инициализация VulkanEngine..." << std::endl;
        auto engine = std::make_unique<VulkanEngine>();
        
        if (!engine->init(instance)) {
            SAFE_ERROR("   ✗ Ошибка инициализации VulkanEngine");
            return -1;
        }
        SAFE_PRINT_LINE("   ✓ VulkanEngine инициализирован");
        
        // 3. Тестируем HardwareDetector
        std::cout << std::endl << "3. Тестирование HardwareDetector..." << std::endl;
        auto* hardwareDetector = engine->getHardwareDetector();
        if (hardwareDetector) {
            std::cout << "   Устройство: " << hardwareDetector->getDeviceName() << std::endl;
            
            size_t vramSize = hardwareDetector->getVRAMSize();
            if (vramSize > 0) {
                std::cout << "   VRAM: " << SAFE_TO_STRING(vramSize / (1024 * 1024)) << " MB" << std::endl;
            } else {
                SAFE_PRINT_LINE("   VRAM: Не удалось определить");
            }
            
            VendorType vendor = hardwareDetector->detectVendor();
            std::cout << "   Вендор: ";
            switch (vendor) {
                case VendorType::NVIDIA: std::cout << "NVIDIA"; break;
                case VendorType::AMD: std::cout << "AMD"; break;
                case VendorType::INTEL: std::cout << "Intel"; break;
                default: std::cout << "Другой"; break;
            }
            std::cout << std::endl;
            
            std::cout << "   Ray Tracing: " << (hardwareDetector->supportsRayTracing() ? "Поддерживается" : "Не поддерживается") << std::endl;
            std::cout << "   CUDA: " << (hardwareDetector->supportsCUDA() ? "Поддерживается" : "Не поддерживается") << std::endl;
            std::cout << "   OptiX: " << (hardwareDetector->supportsOptiX() ? "Поддерживается" : "Не поддерживается") << std::endl;
            
            UpscalerType upscaler = hardwareDetector->selectUpscalerPath();
            std::cout << "   Рекомендуемый upscaler: ";
            switch (upscaler) {
                case UpscalerType::DLSS: std::cout << "DLSS"; break;
                case UpscalerType::FSR: std::cout << "FSR"; break;
                default: std::cout << "Нет"; break;
            }
            std::cout << std::endl;
        }
        
        // 4. Тестируем SceneManager
        std::cout << std::endl << "4. Тестирование SceneManager..." << std::endl;
        auto* sceneManager = engine->getSceneManager();
        if (sceneManager) {
            try {
                // Создаем тестовую сцену
                SceneData testScene;
                testScene.scenePath = "test_scene.json";
                testScene.meshPaths = {"cube.obj", "sphere.obj", "plane.obj"};
                testScene.texturePaths = {"diffuse.png", "normal.png", "roughness.png"};
                
                if (sceneManager->loadScene(testScene)) {
                    SAFE_PRINT_LINE("   ✓ Тестовая сцена загружена");
                    std::cout << "   Объектов в сцене: " << sceneManager->getObjectCount() << std::endl;
                    
                    // Тестируем обновление динамики
                    sceneManager->updateDynamics(0.016f); // 60 FPS
                    
                    // Получаем гауссианы
                    Gaussians gaussians = sceneManager->getGaussians();
                    std::cout << "   Гауссианов для рендеринга: " << SAFE_TO_STRING(gaussians.count) << std::endl;
                } else {
                    SAFE_PRINT_LINE("   ✗ Ошибка загрузки тестовой сцены");
                }
            } catch (const std::exception& e) {
                std::cerr << "   ✗ Исключение в SceneManager: " << e.what() << std::endl;
            }
        } else {
            SAFE_PRINT_LINE("   ✗ SceneManager недоступен");
        }
        
        // 5. Тестируем рендеринг (заглушки)
        std::cout << std::endl << "5. Тестирование рендеринга..." << std::endl;
        auto* renderer = engine->getRenderer();
        if (renderer && sceneManager) {
            try {
                // Создаем параметры камеры
                CameraParams camera;
                camera.position = {0.0f, 0.0f, 5.0f};
                camera.direction = {0.0f, 0.0f, -1.0f};
                camera.fov = 45.0f;
                camera.nearPlane = 0.1f;
                camera.farPlane = 100.0f;
                
                // Выполняем один кадр рендеринга
                SAFE_PRINT_LINE("   Выполнение тестового кадра...");
                engine->renderFrame(camera);
                SAFE_PRINT_LINE("   ✓ Тестовый кадр выполнен");
            } catch (const std::exception& e) {
                std::cerr << "   ✗ Ошибка рендеринга: " << e.what() << std::endl;
            }
        } else {
            SAFE_PRINT_LINE("   ✗ Рендерер или SceneManager недоступны");
        }
        
        // 6. Завершение работы
        std::cout << std::endl << "6. Завершение работы..." << std::endl;
        engine->shutdown();
        SAFE_PRINT_LINE("   ✓ VulkanEngine завершен");
        
        // Уничтожаем Vulkan instance
        instance.destroy();
        SAFE_PRINT_LINE("   ✓ Vulkan instance уничтожен");
        
        std::cout << std::endl << "=== Демо завершено успешно ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << std::endl << "Ошибка выполнения демо: " << e.what() << std::endl;
        return -1;
    }
}

