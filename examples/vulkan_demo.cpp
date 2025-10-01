/**
 * @file vulkan_demo.cpp
 * @brief Демонстрация Vulkan Hybrid Renderer
 *
 * Этот пример показывает использование новой Vulkan-based архитектуры
 * с поддержкой FlashGS, OptiX ray tracing и AI upscaling.
 */

#include <chrono>
#include <iostream>
#include <memory>
#include "HyperEngine/Core/SafeConsole.h"

using namespace HyperEngine::Core;

#ifdef VULKAN_RENDERER_BUILD
#include <HyperEngine/Vulkan/HardwareDetector.h>
#include <HyperEngine/Vulkan/ResourceManager.h>
#include <HyperEngine/Vulkan/VulkanEngine.h>

#ifdef VULKAN_RENDERER_CUDA_SUPPORT
#include <HyperEngine/CUDA/FlashGSSplatter.h>
#endif

#ifdef VULKAN_RENDERER_OPTIX_SUPPORT
#include <HyperEngine/OptiX/OptiXRayTracer.h>
#endif

#include <HyperEngine/Upscaling/DLSSUpscaler.h>
#include <HyperEngine/Upscaling/Upscaler.h>
#endif

#include <GLFW/glfw3.h>
#include <HyperEngine/Core/Console.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

using namespace HyperEngine;
using namespace HyperEngine::Core;

/**
 * @brief Класс демо-приложения
 */
class VulkanDemo {
  public:
    VulkanDemo() = default;
    ~VulkanDemo() = default;

    /**
     * @brief Инициализация демо
     * @return true если инициализация успешна
     */
    bool init() {
        SAFE_PRINT_LINE("=== Vulkan Hybrid Renderer Demo ===");

        // Инициализация GLFW
        if (!glfwInit()) {
            SAFE_ERROR("Ошибка инициализации GLFW");
            return false;
        }

        // Создание окна
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(1920, 1080, "Vulkan Hybrid Renderer Demo", nullptr, nullptr);
        if (!window) {
            SAFE_ERROR("Ошибка создания окна");
            glfwTerminate();
            return false;
        }

        // Инициализация Vulkan
        if (!initVulkan()) {
            return false;
        }

#ifdef VULKAN_RENDERER_BUILD
        // Инициализация движка
        engine = std::make_unique<Vulkan::VulkanEngine>();
        if (!engine->init(instance)) {
            SAFE_ERROR("Ошибка инициализации Vulkan Engine");
            return false;
        }

        // Вывод информации о железе
        printHardwareInfo();

        // Настройка сцены
        setupScene();
#endif

        SAFE_PRINT_LINE("Демо инициализировано успешно!");
        return true;
    }

    /**
     * @brief Главный цикл демо
     */
    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        uint32_t frameCount = 0;
        float totalTime = 0.0f;

        SAFE_PRINT_LINE("Запуск главного цикла...");
        SAFE_PRINT_LINE("Управление:");
        SAFE_PRINT_LINE("  ESC - выход");
        SAFE_PRINT_LINE("  WASD - движение камеры");
        SAFE_PRINT_LINE("  Mouse - поворот камеры");

        while (!glfwWindowShouldClose(window)) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            glfwPollEvents();

            // Обработка ввода
            handleInput(deltaTime);

            // Обновление камеры
            updateCamera(deltaTime);

#ifdef VULKAN_RENDERER_BUILD
            // Рендеринг кадра
            if (engine && engine->isInitialized()) {
                CameraParams cameraParams = getCameraParams();
                engine->renderFrame(cameraParams);
            }
#endif

            // Подсчет FPS
            frameCount++;
            totalTime += deltaTime;

            if (totalTime >= 1.0f) {
                float fps = frameCount / totalTime;
                std::string title =
                    "Vulkan Hybrid Renderer Demo - FPS: " + SAFE_TO_STRING(static_cast<int>(fps));
                glfwSetWindowTitle(window, title.c_str());

                frameCount = 0;
                totalTime = 0.0f;
            }
        }
    }

    /**
     * @brief Завершение работы демо
     */
    void shutdown() {
        SAFE_PRINT_LINE("Завершение работы демо...");

#ifdef VULKAN_RENDERER_BUILD
        if (engine) {
            engine->shutdown();
            engine.reset();
        }
#endif

        if (device) {
            device.destroy();
        }

        if (instance) {
            instance.destroy();
        }

        if (window) {
            glfwDestroyWindow(window);
        }

        glfwTerminate();

        SAFE_PRINT_LINE("Демо завершено.");
    }

  private:
    GLFWwindow* window = nullptr;
    vk::Instance instance;
    vk::Device device;

#ifdef VULKAN_RENDERER_BUILD
    std::unique_ptr<Vulkan::VulkanEngine> engine;
#endif

    // Параметры камеры
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraSpeed = 2.5f;
    float mouseSensitivity = 0.1f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool firstMouse = true;
    double lastX = 960.0;
    double lastY = 540.0;

    /**
     * @brief Инициализация Vulkan
     */
    bool initVulkan() {
        try {
            // Создание Vulkan instance
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "Vulkan Hybrid Renderer Demo";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "HyperEngine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;

            // Получение расширений GLFW
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions,
                                                glfwExtensions + glfwExtensionCount);

            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            instance = vk::createInstance(createInfo);

            SAFE_PRINT_LINE("Vulkan instance создан успешно");
            return true;

        } catch (const vk::SystemError& err) {
            std::cerr << "Ошибка создания Vulkan instance: " << err.what() << std::endl;
            return false;
        }
    }

#ifdef VULKAN_RENDERER_BUILD
    /**
     * @brief Вывод информации о железе
     */
    void printHardwareInfo() {
        if (!engine || !engine->getHardwareDetector()) {
            return;
        }

        auto detector = engine->getHardwareDetector();

        SAFE_PRINT_LINE("\n=== Информация о железе ===");
        std::cout << "GPU: " << detector->getDeviceName() << std::endl;

        auto vendor = detector->detectVendor();
        std::string vendorName;
        switch (vendor) {
            case Vulkan::VendorType::NVIDIA:
                vendorName = "NVIDIA";
                break;
            case Vulkan::VendorType::AMD:
                vendorName = "AMD";
                break;
            case Vulkan::VendorType::INTEL:
                vendorName = "Intel";
                break;
            default:
                vendorName = "Other";
                break;
        }
        std::cout << "Вендор: " << vendorName << std::endl;

        std::cout << "VRAM: " << SAFE_TO_STRING(detector->getVRAMSize() / 1024 / 1024) << " MB"
                  << std::endl;
        std::cout << "Ray Tracing: " << (detector->supportsRayTracing() ? "Да" : "Нет")
                  << std::endl;
        std::cout << "CUDA: " << (detector->supportsCUDA() ? "Да" : "Нет") << std::endl;
        std::cout << "OptiX: " << (detector->supportsOptiX() ? "Да" : "Нет") << std::endl;

        auto upscaler = detector->selectUpscalerPath();
        std::string upscalerName;
        switch (upscaler) {
            case Vulkan::UpscalerType::DLSS:
                upscalerName = "DLSS";
                break;
            case Vulkan::UpscalerType::FSR:
                upscalerName = "FSR";
                break;
            default:
                upscalerName = "None";
                break;
        }
        std::cout << "Upscaler: " << upscalerName << std::endl;
        SAFE_PRINT_LINE("=========================");
    }

    /**
     * @brief Настройка сцены
     */
    void setupScene() {
        SAFE_PRINT_LINE("Настройка демо-сцены...");

        // Здесь будет код для настройки тестовой сцены
        // с гауссианами, геометрией для ray tracing и т.д.

        SAFE_PRINT_LINE("Сцена настроена.");
    }
#endif

    /**
     * @brief Обработка ввода
     */
    void handleInput(float deltaTime) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        float velocity = cameraSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos += velocity * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos -= velocity * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
        }
    }

    /**
     * @brief Обновление камеры
     */
    void updateCamera(float deltaTime) {
        (void)deltaTime;  // Подавляем предупреждение о неиспользуемом параметре

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos);
        lastX = xpos;
        lastY = ypos;

        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
    }

    /**
     * @brief Получение параметров камеры
     */
    CameraParams getCameraParams() {
        CameraParams params;

        params.viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        params.projectionMatrix =
            glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
        params.position = cameraPos;
        params.direction = cameraFront;
        params.fov = 45.0f;
        params.nearPlane = 0.1f;
        params.farPlane = 100.0f;

        return params;
    }
};

/**
 * @brief Главная функция
 */
int main() {
    Console::initialize();
    Console::setTitle("🚀 Vulkan Hybrid Renderer Demo");
    SAFE_PRINT_LINE("Запуск Vulkan Hybrid Renderer Demo...");

    VulkanDemo demo;

    if (!demo.init()) {
        SAFE_ERROR("Ошибка инициализации демо");
        return -1;
    }

    try {
        demo.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка во время выполнения: " << e.what() << std::endl;
        demo.shutdown();
        return -1;
    }

    demo.shutdown();
    return 0;
}
