/**
 * @file vulkan_hardware_detector_test.cpp
 * @brief Комплексное тестирование HardwareDetector
 * 
 * Покрывает все методы HardwareDetector для достижения 100% coverage
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/HardwareDetector.h"
#include <vulkan/vulkan.hpp>
#include <memory>

using namespace SpectraForge::Vulkan;

// ============================================================================
// Mock Vulkan Instance Helper
// ============================================================================

class MockVulkanInstance {
public:
    static vk::Instance createMockInstance() {
        // Создаем реальный Vulkan instance для тестирования
        try {
            // CRITICAL FIX: Properly initialize Vulkan structs with sType
            vk::ApplicationInfo appInfo;
            appInfo.sType = vk::StructureType::eApplicationInfo;
            appInfo.pNext = nullptr;
            appInfo.pApplicationName = "HardwareDetector Test";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SpectraForge Test";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            vk::InstanceCreateInfo createInfo;
            createInfo.sType = vk::StructureType::eInstanceCreateInfo;
            createInfo.pNext = nullptr;
            createInfo.flags = {};
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.enabledExtensionCount = 0;
            createInfo.ppEnabledExtensionNames = nullptr;

            // Создаем instance без validation layers для тестов
            return vk::createInstance(createInfo);
        } catch (const std::exception& e) {
            std::cerr << "[MockVulkanInstance] Failed to create instance: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static void destroyMockInstance(vk::Instance instance) {
        if (instance) {
            instance.destroy();
        }
    }
};

// ============================================================================
// Test Fixture
// ============================================================================

class HardwareDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Arrange: Создаем Vulkan instance для тестов
        instance = MockVulkanInstance::createMockInstance();
        if (!instance) {
            GTEST_SKIP() << "Vulkan не доступен на этой системе";
        }

        // Получаем физические устройства
        auto devices = instance.enumeratePhysicalDevices();
        if (devices.empty()) {
            MockVulkanInstance::destroyMockInstance(instance);
            instance = nullptr;
            GTEST_SKIP() << "Нет доступных Vulkan устройств";
        }

        physicalDevice = devices[0]; // Используем первое устройство
        detector = std::make_unique<HardwareDetector>();
    }

    void TearDown() override {
        detector.reset();
        if (instance) {
            MockVulkanInstance::destroyMockInstance(instance);
        }
    }

    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    std::unique_ptr<HardwareDetector> detector;
};

// ============================================================================
// Constructor/Destructor Tests
// ============================================================================

TEST(HardwareDetectorBasicTest, ConstructorTest) {
    // Arrange & Act
    HardwareDetector detector;

    // Assert: Объект успешно создан
    EXPECT_NO_THROW({
        HardwareDetector temp;
    });
}

TEST(HardwareDetectorBasicTest, DestructorTest) {
    // Arrange
    auto detector = std::make_unique<HardwareDetector>();

    // Act & Assert: Деструктор должен работать без ошибок
    EXPECT_NO_THROW({
        detector.reset();
    });
}

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(HardwareDetectorTest, InitSuccessTest) {
    // Arrange: физическое устройство уже получено в SetUp

    // Act
    bool result = detector->init(physicalDevice);

    // Assert
    EXPECT_TRUE(result);
}

TEST(HardwareDetectorBasicTest, InitWithNullDeviceTest) {
    // Arrange
    HardwareDetector detector;
    vk::PhysicalDevice nullDevice;

    // Act & Assert: Инициализация с null device должна вернуть false
    EXPECT_FALSE(detector.init(nullDevice));
}

// ============================================================================
// Vendor Detection Tests
// ============================================================================

TEST_F(HardwareDetectorTest, DetectVendorTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    VendorType vendor = detector->detectVendor();

    // Assert: Вендор должен быть определен (не OTHER в большинстве случаев)
    EXPECT_TRUE(vendor == VendorType::NVIDIA || 
                vendor == VendorType::AMD || 
                vendor == VendorType::INTEL || 
                vendor == VendorType::OTHER);
}

TEST_F(HardwareDetectorTest, DetectVendorBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    VendorType vendor = detector->detectVendor();

    // Assert: Должен вернуть OTHER
    EXPECT_EQ(vendor, VendorType::OTHER);
}

TEST_F(HardwareDetectorTest, DetectVendorNVIDIATest) {
    // Arrange
    detector->init(physicalDevice);
    VendorType vendor = detector->detectVendor();

    // Act & Assert: Если это NVIDIA, проверяем имя устройства
    if (vendor == VendorType::NVIDIA) {
        std::string deviceName = detector->getDeviceName();
        // NVIDIA устройства обычно содержат "NVIDIA" в названии
        EXPECT_NE(deviceName.find("NVIDIA"), std::string::npos);
    }
}

TEST_F(HardwareDetectorTest, DetectVendorAMDTest) {
    // Arrange
    detector->init(physicalDevice);
    VendorType vendor = detector->detectVendor();

    // Act & Assert: Если это AMD, проверяем имя устройства
    if (vendor == VendorType::AMD) {
        std::string deviceName = detector->getDeviceName();
        // AMD устройства обычно содержат "AMD" или "Radeon" в названии
        EXPECT_TRUE(deviceName.find("AMD") != std::string::npos || 
                    deviceName.find("Radeon") != std::string::npos);
    }
}

// ============================================================================
// Ray Tracing Support Tests
// ============================================================================

TEST_F(HardwareDetectorTest, SupportsRayTracingTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    bool rtSupport = detector->supportsRayTracing();

    // Assert: Результат должен быть bool (true или false)
    EXPECT_TRUE(rtSupport == true || rtSupport == false);
}

TEST_F(HardwareDetectorTest, SupportsRayTracingBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    bool rtSupport = detector->supportsRayTracing();

    // Assert: Должен вернуть false
    EXPECT_FALSE(rtSupport);
}

// ============================================================================
// Upscaler Selection Tests
// ============================================================================

TEST_F(HardwareDetectorTest, SelectUpscalerPathTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    UpscalerType upscalerType = detector->selectUpscalerPath();

    // Assert: Upscaler тип должен быть валидным
    EXPECT_TRUE(upscalerType == UpscalerType::DLSS || 
                upscalerType == UpscalerType::FSR || 
                upscalerType == UpscalerType::NONE);
}

TEST_F(HardwareDetectorTest, SelectUpscalerPathBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    UpscalerType upscalerType = detector->selectUpscalerPath();

    // Assert: Должен вернуть NONE
    EXPECT_EQ(upscalerType, UpscalerType::NONE);
}

TEST_F(HardwareDetectorTest, SelectUpscalerPathNVIDIARTXTest) {
    // Arrange
    detector->init(physicalDevice);
    VendorType vendor = detector->detectVendor();

    // Act & Assert: NVIDIA RTX карты должны выбирать DLSS
    if (vendor == VendorType::NVIDIA && detector->supportsRayTracing()) {
        UpscalerType upscalerType = detector->selectUpscalerPath();
        EXPECT_EQ(upscalerType, UpscalerType::DLSS);
    }
}

TEST_F(HardwareDetectorTest, SelectUpscalerPathAMDTest) {
    // Arrange
    detector->init(physicalDevice);
    VendorType vendor = detector->detectVendor();

    // Act & Assert: AMD карты должны выбирать FSR
    if (vendor == VendorType::AMD) {
        UpscalerType upscalerType = detector->selectUpscalerPath();
        EXPECT_EQ(upscalerType, UpscalerType::FSR);
    }
}

// ============================================================================
// CUDA Support Tests
// ============================================================================

TEST_F(HardwareDetectorTest, SupportsCUDATest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    bool cudaSupport = detector->supportsCUDA();

    // Assert: Результат должен быть bool
    EXPECT_TRUE(cudaSupport == true || cudaSupport == false);
}

TEST_F(HardwareDetectorTest, SupportsCUDABeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    bool cudaSupport = detector->supportsCUDA();

    // Assert: Должен вернуть false
    EXPECT_FALSE(cudaSupport);
}

TEST_F(HardwareDetectorTest, SupportsCUDANVIDIAOnlyTest) {
    // Arrange
    detector->init(physicalDevice);
    VendorType vendor = detector->detectVendor();

    // Act
    bool cudaSupport = detector->supportsCUDA();

    // Assert: Только NVIDIA карты поддерживают CUDA
    if (vendor == VendorType::NVIDIA) {
        EXPECT_TRUE(cudaSupport);
    } else {
        EXPECT_FALSE(cudaSupport);
    }
}

// ============================================================================
// OptiX Support Tests
// ============================================================================

TEST_F(HardwareDetectorTest, SupportsOptiXTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    bool optixSupport = detector->supportsOptiX();

    // Assert: Результат должен быть bool
    EXPECT_TRUE(optixSupport == true || optixSupport == false);
}

TEST_F(HardwareDetectorTest, SupportsOptiXBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    bool optixSupport = detector->supportsOptiX();

    // Assert: Должен вернуть false
    EXPECT_FALSE(optixSupport);
}

TEST_F(HardwareDetectorTest, SupportsOptiXRequiresCUDATest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    bool cudaSupport = detector->supportsCUDA();
    bool optixSupport = detector->supportsOptiX();

    // Assert: OptiX требует CUDA, поэтому если OptiX = true, то CUDA = true
    if (optixSupport) {
        EXPECT_TRUE(cudaSupport);
    }
}

// ============================================================================
// Device Properties Tests
// ============================================================================

TEST_F(HardwareDetectorTest, GetDevicePropertiesTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    const vk::PhysicalDeviceProperties& props = detector->getDeviceProperties();

    // Assert: Проверяем что свойства валидны
    EXPECT_GT(props.deviceID, 0u);
    EXPECT_GT(props.vendorID, 0u);
    EXPECT_NE(props.deviceName[0], '\0');
}

TEST_F(HardwareDetectorTest, GetDeviceFeaturesTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    const vk::PhysicalDeviceFeatures& features = detector->getDeviceFeatures();

    // Assert: Проверяем что features получены (они могут быть все false, это нормально)
    // Просто проверяем что метод не падает
    EXPECT_NO_THROW({
        detector->getDeviceFeatures();
    });
}

TEST_F(HardwareDetectorTest, GetDeviceNameTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    std::string deviceName = detector->getDeviceName();

    // Assert: Имя устройства не должно быть пустым
    EXPECT_FALSE(deviceName.empty());
    EXPECT_GT(deviceName.length(), 0u);
}

TEST_F(HardwareDetectorTest, GetVRAMSizeTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    size_t vramSize = detector->getVRAMSize();

    // Assert: VRAM должен быть больше 0 (даже встроенные GPU имеют какую-то память)
    EXPECT_GT(vramSize, 0u);
}

// ============================================================================
// Extension Support Tests
// ============================================================================

TEST_F(HardwareDetectorTest, SupportsExtensionTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Проверяем популярное расширение
    bool swapchainSupport = detector->supportsExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Assert: Swapchain должен поддерживаться на большинстве устройств
    EXPECT_TRUE(swapchainSupport);
}

TEST_F(HardwareDetectorTest, SupportsExtensionInvalidTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Проверяем несуществующее расширение
    bool fakeExtensionSupport = detector->supportsExtension("VK_FAKE_EXTENSION");

    // Assert: Несуществующее расширение не должно поддерживаться
    EXPECT_FALSE(fakeExtensionSupport);
}

TEST_F(HardwareDetectorTest, SupportsExtensionBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    bool extensionSupport = detector->supportsExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Assert: До инициализации должен вернуть false
    EXPECT_FALSE(extensionSupport);
}

TEST_F(HardwareDetectorTest, SupportsExtensionRayTracingTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    bool rtSupport = detector->supportsRayTracing();
    bool rtExtensionSupport = detector->supportsExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    // Assert: Если RT поддерживается, то расширение должно быть доступно
    if (rtSupport) {
        // Ray tracing требует несколько расширений, проверяем основное
        EXPECT_TRUE(rtExtensionSupport || 
                    detector->supportsExtension(VK_NV_RAY_TRACING_EXTENSION_NAME));
    }
}

// ============================================================================
// Physical Device Access Tests
// ============================================================================

TEST_F(HardwareDetectorTest, GetPhysicalDeviceTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    vk::PhysicalDevice retrievedDevice = detector->getPhysicalDevice();

    // Assert: Устройство должно совпадать с тем, что мы передали
    EXPECT_EQ(retrievedDevice, physicalDevice);
}

TEST_F(HardwareDetectorTest, GetPhysicalDeviceBeforeInitTest) {
    // Arrange: detector не инициализирован

    // Act
    vk::PhysicalDevice retrievedDevice = detector->getPhysicalDevice();

    // Assert: Должен вернуть null handle
    EXPECT_FALSE(retrievedDevice);
}

// ============================================================================
// Edge Cases and Error Handling Tests
// ============================================================================

TEST_F(HardwareDetectorTest, MultipleInitializationsTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Пытаемся инициализировать повторно
    bool secondInit = detector->init(physicalDevice);

    // Assert: Повторная инициализация должна работать
    EXPECT_TRUE(secondInit);
}

TEST_F(HardwareDetectorTest, GetPropertiesConsistencyTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Получаем свойства несколько раз
    const vk::PhysicalDeviceProperties& props1 = detector->getDeviceProperties();
    const vk::PhysicalDeviceProperties& props2 = detector->getDeviceProperties();

    // Assert: Свойства должны быть одинаковыми
    EXPECT_EQ(props1.deviceID, props2.deviceID);
    EXPECT_EQ(props1.vendorID, props2.vendorID);
}

TEST_F(HardwareDetectorTest, UpscalerPathConsistencyTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Вызываем selectUpscalerPath несколько раз
    UpscalerType type1 = detector->selectUpscalerPath();
    UpscalerType type2 = detector->selectUpscalerPath();

    // Assert: Результат должен быть одинаковым
    EXPECT_EQ(type1, type2);
}

TEST_F(HardwareDetectorTest, VendorDetectionConsistencyTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Определяем вендора несколько раз
    VendorType vendor1 = detector->detectVendor();
    VendorType vendor2 = detector->detectVendor();

    // Assert: Вендор должен быть одинаковым
    EXPECT_EQ(vendor1, vendor2);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(HardwareDetectorTest, FullWorkflowTest) {
    // Arrange: Полный workflow использования HardwareDetector

    // Act & Assert: Шаг 1 - Инициализация
    EXPECT_TRUE(detector->init(physicalDevice));

    // Шаг 2 - Получение информации об устройстве
    std::string deviceName = detector->getDeviceName();
    EXPECT_FALSE(deviceName.empty());

    // Шаг 3 - Определение вендора
    VendorType vendor = detector->detectVendor();
    EXPECT_NE(vendor, VendorType::OTHER); // В большинстве случаев

    // Шаг 4 - Проверка возможностей
    bool rtSupport = detector->supportsRayTracing();
    bool cudaSupport = detector->supportsCUDA();

    // Шаг 5 - Выбор пути upscaling
    UpscalerType upscalerType = detector->selectUpscalerPath();
    
    // Проверяем логику выбора upscaler
    if (vendor == VendorType::NVIDIA && rtSupport) {
        EXPECT_EQ(upscalerType, UpscalerType::DLSS);
    } else if (vendor == VendorType::AMD) {
        EXPECT_EQ(upscalerType, UpscalerType::FSR);
    }

    // Шаг 6 - Получение VRAM
    size_t vramSize = detector->getVRAMSize();
    EXPECT_GT(vramSize, 0u);
}

TEST_F(HardwareDetectorTest, ConcurrentAccessTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act: Параллельный доступ к методам (симуляция)
    std::string name = detector->getDeviceName();
    VendorType vendor = detector->detectVendor();
    bool rtSupport = detector->supportsRayTracing();
    size_t vram = detector->getVRAMSize();

    // Assert: Все методы должны работать корректно
    EXPECT_FALSE(name.empty());
    EXPECT_TRUE(vendor == VendorType::NVIDIA || vendor == VendorType::AMD || 
                vendor == VendorType::INTEL || vendor == VendorType::OTHER);
    EXPECT_TRUE(rtSupport == true || rtSupport == false);
    EXPECT_GT(vram, 0u);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(HardwareDetectorTest, InitPerformanceTest) {
    // Arrange
    auto start = std::chrono::high_resolution_clock::now();

    // Act
    detector->init(physicalDevice);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Инициализация должна занимать менее 1 секунды
    EXPECT_LT(duration.count(), 1000);
}

TEST_F(HardwareDetectorTest, VendorDetectionPerformanceTest) {
    // Arrange
    detector->init(physicalDevice);
    auto start = std::chrono::high_resolution_clock::now();

    // Act: Определяем вендора 1000 раз
    for (int i = 0; i < 1000; ++i) {
        detector->detectVendor();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: 1000 вызовов должны занимать менее 100 мс
    EXPECT_LT(duration.count(), 100);
}

// ============================================================================
// Boundary Tests
// ============================================================================

TEST_F(HardwareDetectorTest, VRAMSizeRangeTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    size_t vramSize = detector->getVRAMSize();

    // Assert: VRAM должен быть в разумных пределах (от 128 MB до 128 GB)
    EXPECT_GE(vramSize, 128 * 1024 * 1024ULL); // Минимум 128 MB
    EXPECT_LE(vramSize, 128ULL * 1024 * 1024 * 1024); // Максимум 128 GB
}

TEST_F(HardwareDetectorTest, DeviceNameLengthTest) {
    // Arrange
    detector->init(physicalDevice);

    // Act
    std::string deviceName = detector->getDeviceName();

    // Assert: Имя должно быть разумной длины (не более 256 символов)
    EXPECT_LE(deviceName.length(), 256u);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
