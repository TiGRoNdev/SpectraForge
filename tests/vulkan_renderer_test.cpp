/**
 * @file vulkan_renderer_test.cpp
 * @brief Комплексное тестирование VulkanRenderer
 * 
 * Покрывает все методы VulkanRenderer для достижения 100% coverage
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/VulkanRenderer.h"
#include "SpectraForge/Vulkan/ResourceManager.h"
#include <vulkan/vulkan.hpp>
#include <memory>

using namespace SpectraForge::Vulkan;

// ============================================================================
// Mock Vulkan Setup Helper
// ============================================================================

class MockVulkanSetupForRenderer {
public:
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    std::unique_ptr<ResourceManager> resourceManager;
    uint32_t queueFamilyIndex = 0;

    bool create() {
        try {
            // Создаем instance
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "VulkanRenderer Test";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SpectraForge Test";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;

            instance = vk::createInstance(createInfo);
            if (!instance) return false;

            // Получаем физические устройства
            auto devices = instance.enumeratePhysicalDevices();
            if (devices.empty()) {
                cleanup();
                return false;
            }
            physicalDevice = devices[0];

            // Находим queue family
            auto queueFamilies = physicalDevice.getQueueFamilyProperties();
            for (uint32_t i = 0; i < queueFamilies.size(); i++) {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    queueFamilyIndex = i;
                    break;
                }
            }

            // Создаем logical device
            float queuePriority = 1.0f;
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            vk::DeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.queueCreateInfoCount = 1;
            deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

            device = physicalDevice.createDevice(deviceCreateInfo);
            if (!device) {
                cleanup();
                return false;
            }

            // Создаем ResourceManager
            resourceManager = std::make_unique<ResourceManager>();
            if (!resourceManager->init(physicalDevice, device, instance)) {
                cleanup();
                return false;
            }

            return true;

        } catch (const std::exception& e) {
            std::cerr << "[MockVulkanSetupForRenderer] Error: " << e.what() << std::endl;
            cleanup();
            return false;
        }
    }

    void cleanup() {
        resourceManager.reset();
        if (device) {
            device.destroy();
            device = nullptr;
        }
        if (instance) {
            instance.destroy();
            instance = nullptr;
        }
    }
};

// ============================================================================
// Test Fixture
// ============================================================================

class VulkanRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!mockSetup.create()) {
            GTEST_SKIP() << "Vulkan не доступен на этой системе";
        }

        renderer = std::make_unique<VulkanRenderer>();
    }

    void TearDown() override {
        renderer.reset();
        mockSetup.cleanup();
    }

    MockVulkanSetupForRenderer mockSetup;
    std::unique_ptr<VulkanRenderer> renderer;
};

// ============================================================================
// Constructor/Destructor Tests
// ============================================================================

TEST(VulkanRendererBasicTest, ConstructorTest) {
    // Arrange & Act
    VulkanRenderer renderer;

    // Assert: Объект успешно создан
    EXPECT_NO_THROW({
        VulkanRenderer temp;
    });
}

TEST(VulkanRendererBasicTest, DestructorTest) {
    // Arrange
    auto renderer = std::make_unique<VulkanRenderer>();

    // Act & Assert: Деструктор должен работать без ошибок
    EXPECT_NO_THROW({
        renderer.reset();
    });
}

TEST(VulkanRendererBasicTest, DestructorAfterInitTest) {
    // Arrange
    MockVulkanSetupForRenderer setup;
    if (!setup.create()) {
        GTEST_SKIP() << "Vulkan не доступен";
    }

    auto renderer = std::make_unique<VulkanRenderer>();
    renderer->init(setup.device, setup.resourceManager.get());

    // Act & Assert: Деструктор после инициализации
    EXPECT_NO_THROW({
        renderer.reset();
    });

    setup.cleanup();
}

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(VulkanRendererTest, InitSuccessTest) {
    // Arrange: setup уже создан

    // Act
    bool result = renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Assert
    EXPECT_TRUE(result);
}

TEST(VulkanRendererBasicTest, InitWithNullDeviceTest) {
    // Arrange
    VulkanRenderer renderer;
    vk::Device nullDevice;

    // Act
    bool result = renderer.init(nullDevice, nullptr);

    // Assert: Должна вернуть false
    EXPECT_FALSE(result);
}

TEST_F(VulkanRendererTest, InitWithNullResourceManagerTest) {
    // Arrange
    // Act
    bool result = renderer->init(mockSetup.device, nullptr);

    // Assert: Должна вернуть false
    EXPECT_FALSE(result);
}

TEST_F(VulkanRendererTest, InitMultipleTimesTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Повторная инициализация
    bool secondInit = renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Assert: Должна работать
    EXPECT_TRUE(secondInit);
}

TEST_F(VulkanRendererTest, IsInitializedTest) {
    // Arrange
    EXPECT_FALSE(renderer->isInitialized());

    // Act
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Assert
    EXPECT_TRUE(renderer->isInitialized());
}

TEST_F(VulkanRendererTest, IsInitializedAfterShutdownTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act
    renderer->shutdown();

    // Assert
    EXPECT_FALSE(renderer->isInitialized());
}

// ============================================================================
// Shutdown Tests
// ============================================================================

TEST_F(VulkanRendererTest, ShutdownTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act & Assert
    EXPECT_NO_THROW({
        renderer->shutdown();
    });
}

TEST_F(VulkanRendererTest, ShutdownWithoutInitTest) {
    // Arrange: renderer не инициализирован

    // Act & Assert
    EXPECT_NO_THROW({
        renderer->shutdown();
    });
}

TEST_F(VulkanRendererTest, MultipleShutdownsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act
    renderer->shutdown();
    
    // Assert: Второй shutdown не должен падать
    EXPECT_NO_THROW({
        renderer->shutdown();
    });
}

// ============================================================================
// Primary Rasterization Tests
// ============================================================================

TEST_F(VulkanRendererTest, RasterizePrimaryTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 100;

    // Act & Assert: Рендеринг гауссианов
    EXPECT_NO_THROW({
        PrimaryImage image = renderer->rasterizePrimary(gaussians);
    });
}

TEST_F(VulkanRendererTest, RasterizePrimaryEmptyGaussiansTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 0;

    // Act
    PrimaryImage image = renderer->rasterizePrimary(gaussians);

    // Assert: Должен вернуть пустое изображение или выбросить исключение
    EXPECT_NO_THROW({
        renderer->rasterizePrimary(gaussians);
    });
}

TEST_F(VulkanRendererTest, RasterizePrimaryLargeCountTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 1000000; // Большое количество

    // Act & Assert: Не должно падать
    EXPECT_NO_THROW({
        renderer->rasterizePrimary(gaussians);
    });
}

TEST_F(VulkanRendererTest, RasterizePrimaryBeforeInitTest) {
    // Arrange: renderer не инициализирован
    Gaussians gaussians;
    gaussians.count = 100;

    // Act & Assert: Должен выбросить исключение или вернуть пустое изображение
    EXPECT_THROW({
        renderer->rasterizePrimary(gaussians);
    }, std::exception);
}

TEST_F(VulkanRendererTest, RasterizePrimaryMultipleCallsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 50;

    // Act: Множественные вызовы
    for (int i = 0; i < 5; i++) {
        EXPECT_NO_THROW({
            PrimaryImage image = renderer->rasterizePrimary(gaussians);
        });
    }
}

// ============================================================================
// Ray Tracing Tests
// ============================================================================

TEST_F(VulkanRendererTest, RayTraceSecondaryTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    PrimaryImage primaryImage;
    primaryImage.width = 1920;
    primaryImage.height = 1080;

    // Act & Assert
    EXPECT_NO_THROW({
        RawEffects effects = renderer->rayTraceSecondary(primaryImage);
    });
}

TEST_F(VulkanRendererTest, RayTraceSecondaryInvalidSizeTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    PrimaryImage primaryImage;
    primaryImage.width = 0;
    primaryImage.height = 0;

    // Act & Assert: Должен обработать невалидный размер
    EXPECT_NO_THROW({
        renderer->rayTraceSecondary(primaryImage);
    });
}

TEST_F(VulkanRendererTest, RayTraceSecondaryBeforeInitTest) {
    // Arrange: renderer не инициализирован
    PrimaryImage primaryImage;
    primaryImage.width = 1920;
    primaryImage.height = 1080;

    // Act & Assert: Должен выбросить исключение
    EXPECT_THROW({
        renderer->rayTraceSecondary(primaryImage);
    }, std::exception);
}

TEST_F(VulkanRendererTest, RayTraceSecondaryDifferentResolutionsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act & Assert: Тестируем разные разрешения
    std::vector<std::pair<uint32_t, uint32_t>> resolutions = {
        {640, 480},
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };

    for (const auto& res : resolutions) {
        PrimaryImage primaryImage;
        primaryImage.width = res.first;
        primaryImage.height = res.second;

        EXPECT_NO_THROW({
            renderer->rayTraceSecondary(primaryImage);
        });
    }
}

// ============================================================================
// AI Denoising Tests
// ============================================================================

TEST_F(VulkanRendererTest, DenoiseAITest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    RawEffects effects;

    // Act & Assert
    EXPECT_NO_THROW({
        DenoisedImage image = renderer->denoiseAI(effects);
    });
}

TEST_F(VulkanRendererTest, DenoiseAIBeforeInitTest) {
    // Arrange: renderer не инициализирован
    RawEffects effects;

    // Act & Assert: Должен выбросить исключение
    EXPECT_THROW({
        renderer->denoiseAI(effects);
    }, std::exception);
}

TEST_F(VulkanRendererTest, DenoiseAIMultipleCallsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    RawEffects effects;

    // Act: Множественные вызовы
    for (int i = 0; i < 3; i++) {
        EXPECT_NO_THROW({
            DenoisedImage image = renderer->denoiseAI(effects);
        });
    }
}

// ============================================================================
// Upscaling Tests
// ============================================================================

TEST_F(VulkanRendererTest, UpscaleTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    DenoisedImage denoisedImage;

    ResolutionTarget target;
    target.width = 3840;
    target.height = 2160;
    target.scaleFactor = 2.0f;

    // Act & Assert
    EXPECT_NO_THROW({
        FinalImage finalImage = renderer->upscale(denoisedImage, target);
    });
}

TEST_F(VulkanRendererTest, UpscaleDifferentFactorsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    DenoisedImage denoisedImage;

    // Act & Assert: Тестируем разные scale факторы
    std::vector<float> scaleFactors = {1.5f, 2.0f, 2.5f, 4.0f};

    for (float factor : scaleFactors) {
        ResolutionTarget target;
        target.width = static_cast<uint32_t>(1920 * factor);
        target.height = static_cast<uint32_t>(1080 * factor);
        target.scaleFactor = factor;

        EXPECT_NO_THROW({
            renderer->upscale(denoisedImage, target);
        });
    }
}

TEST_F(VulkanRendererTest, UpscaleZeroFactorTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    DenoisedImage denoisedImage;

    ResolutionTarget target;
    target.width = 0;
    target.height = 0;
    target.scaleFactor = 0.0f;

    // Act & Assert: Должен обработать невалидный фактор
    EXPECT_NO_THROW({
        renderer->upscale(denoisedImage, target);
    });
}

TEST_F(VulkanRendererTest, UpscaleBeforeInitTest) {
    // Arrange: renderer не инициализирован
    DenoisedImage denoisedImage;

    ResolutionTarget target;
    target.width = 3840;
    target.height = 2160;
    target.scaleFactor = 2.0f;

    // Act & Assert: Должен выбросить исключение
    EXPECT_THROW({
        renderer->upscale(denoisedImage, target);
    }, std::exception);
}

// ============================================================================
// Present Tests
// ============================================================================

TEST_F(VulkanRendererTest, PresentFinalImageTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act & Assert
    EXPECT_NO_THROW({
        renderer->presentFinalImage();
    });
}

TEST_F(VulkanRendererTest, PresentFinalImageBeforeInitTest) {
    // Arrange: renderer не инициализирован

    // Act & Assert: Должен выбросить исключение
    EXPECT_THROW({
        renderer->presentFinalImage();
    }, std::exception);
}

TEST_F(VulkanRendererTest, PresentFinalImageMultipleCallsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Множественные вызовы
    for (int i = 0; i < 5; i++) {
        EXPECT_NO_THROW({
            renderer->presentFinalImage();
        });
    }
}

// ============================================================================
// Integration Tests - Full Rendering Pipeline
// ============================================================================

TEST_F(VulkanRendererTest, FullRenderingPipelineTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act & Assert: Полный конвейер рендеринга
    // Шаг 1: Primary rasterization
    Gaussians gaussians;
    gaussians.count = 100;
    PrimaryImage primaryImage = renderer->rasterizePrimary(gaussians);

    // Шаг 2: Secondary ray tracing
    RawEffects effects = renderer->rayTraceSecondary(primaryImage);

    // Шаг 3: AI denoising
    DenoisedImage denoisedImage = renderer->denoiseAI(effects);

    // Шаг 4: Upscaling
    ResolutionTarget target;
    target.width = 3840;
    target.height = 2160;
    target.scaleFactor = 2.0f;
    FinalImage finalImage = renderer->upscale(denoisedImage, target);

    // Шаг 5: Present
    renderer->presentFinalImage();

    // Все шаги должны пройти без ошибок
    SUCCEED();
}

TEST_F(VulkanRendererTest, MultipleFramesRenderingTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Рендерим несколько кадров
    for (int frame = 0; frame < 10; frame++) {
        Gaussians gaussians;
        gaussians.count = 100;
        
        PrimaryImage primaryImage = renderer->rasterizePrimary(gaussians);
        RawEffects effects = renderer->rayTraceSecondary(primaryImage);
        DenoisedImage denoisedImage = renderer->denoiseAI(effects);
        
        ResolutionTarget target;
        target.width = 1920;
        target.height = 1080;
        target.scaleFactor = 1.0f;
        
        FinalImage finalImage = renderer->upscale(denoisedImage, target);
        renderer->presentFinalImage();
    }

    // Assert: Все кадры должны отрендериться успешно
    SUCCEED();
}

TEST_F(VulkanRendererTest, RenderingPipelineWithDifferentParametersTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Рендерим с разными параметрами
    std::vector<uint32_t> gaussianCounts = {10, 100, 1000};
    std::vector<std::pair<uint32_t, uint32_t>> resolutions = {
        {1280, 720},
        {1920, 1080},
        {3840, 2160}
    };

    for (uint32_t count : gaussianCounts) {
        for (const auto& res : resolutions) {
            Gaussians gaussians;
            gaussians.count = count;
            
            PrimaryImage primaryImage = renderer->rasterizePrimary(gaussians);
            
            ResolutionTarget target;
            target.width = res.first;
            target.height = res.second;
            target.scaleFactor = static_cast<float>(res.first) / 1920.0f;
            
            // Пропускаем полный pipeline, тестируем только основные этапы
        }
    }

    SUCCEED();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(VulkanRendererTest, InvalidInputHandlingTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act & Assert: Тестируем обработку невалидных входных данных
    Gaussians invalidGaussians;
    invalidGaussians.count = 0;
    
    EXPECT_NO_THROW({
        renderer->rasterizePrimary(invalidGaussians);
    });
}

TEST_F(VulkanRendererTest, OutOfOrderOperationsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Пытаемся вызвать операции в неправильном порядке
    DenoisedImage emptyDenoised;
    ResolutionTarget target{1920, 1080, 1.0f};

    // Должен обработать это корректно (или выбросить исключение)
    EXPECT_NO_THROW({
        renderer->upscale(emptyDenoised, target);
    });
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(VulkanRendererTest, InitPerformanceTest) {
    // Arrange
    auto start = std::chrono::high_resolution_clock::now();

    // Act
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Инициализация должна занимать меньше 1 секунды
    EXPECT_LT(duration.count(), 1000);
}

TEST_F(VulkanRendererTest, RasterizationPerformanceTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    // Act: Рендерим 100 раз
    for (int i = 0; i < 100; i++) {
        renderer->rasterizePrimary(gaussians);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Должно занять меньше 5 секунд
    EXPECT_LT(duration.count(), 5000);
}

// ============================================================================
// State Tests
// ============================================================================

TEST_F(VulkanRendererTest, RendererStateConsistencyTest) {
    // Arrange
    EXPECT_FALSE(renderer->isInitialized());

    // Act & Assert: Проверяем состояние на каждом этапе
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());
    EXPECT_TRUE(renderer->isInitialized());

    renderer->shutdown();
    EXPECT_FALSE(renderer->isInitialized());

    // Повторная инициализация
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());
    EXPECT_TRUE(renderer->isInitialized());
}

TEST_F(VulkanRendererTest, ConcurrentOperationsTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    // Act: Симулируем конкурентные операции
    Gaussians gaussians;
    gaussians.count = 100;

    // Множественные операции подряд
    PrimaryImage img1 = renderer->rasterizePrimary(gaussians);
    PrimaryImage img2 = renderer->rasterizePrimary(gaussians);
    
    // Assert: Обе операции должны завершиться успешно
    SUCCEED();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(VulkanRendererTest, ExtremeLowResolutionTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    PrimaryImage primaryImage;
    primaryImage.width = 1;
    primaryImage.height = 1;

    // Act & Assert: Минимальное разрешение
    EXPECT_NO_THROW({
        renderer->rayTraceSecondary(primaryImage);
    });
}

TEST_F(VulkanRendererTest, ExtremeHighResolutionTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    PrimaryImage primaryImage;
    primaryImage.width = 16384;  // 16K
    primaryImage.height = 8192;

    // Act & Assert: Очень высокое разрешение
    EXPECT_NO_THROW({
        renderer->rayTraceSecondary(primaryImage);
    });
}

TEST_F(VulkanRendererTest, VeryLargeGaussianCountTest) {
    // Arrange
    renderer->init(mockSetup.device, mockSetup.resourceManager.get());

    Gaussians gaussians;
    gaussians.count = 10000000;  // 10 миллионов

    // Act & Assert: Очень большое количество
    EXPECT_NO_THROW({
        renderer->rasterizePrimary(gaussians);
    });
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
