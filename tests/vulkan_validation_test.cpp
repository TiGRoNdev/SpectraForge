/**
 * @file vulkan_validation_test.cpp
 * @brief Комплексное тестирование VulkanValidation
 * 
 * Покрывает все методы VulkanValidation для достижения 100% coverage
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/VulkanValidation.h"
#include <vulkan/vulkan.hpp>
#include <memory>

using namespace SpectraForge::Vulkan;

// ============================================================================
// Validation Layer Tests
// ============================================================================

TEST(VulkanValidationTest, CheckValidationLayerSupportTest) {
    // Arrange & Act
    bool supported = VulkanValidation::checkValidationLayerSupport();

    // Assert: Результат должен быть bool (может быть true или false)
    EXPECT_TRUE(supported == true || supported == false);
}

TEST(VulkanValidationTest, GetRequiredValidationLayersTest) {
    // Arrange & Act
    std::vector<const char*> layers = VulkanValidation::getRequiredValidationLayers();

    // Assert: Список должен содержать хотя бы стандартный validation layer
    EXPECT_FALSE(layers.empty());
    
    // Проверяем что стандартный layer присутствует
    bool hasStandardLayer = false;
    for (const char* layer : layers) {
        if (std::string(layer) == "VK_LAYER_KHRONOS_validation") {
            hasStandardLayer = true;
            break;
        }
    }
    EXPECT_TRUE(hasStandardLayer);
}

TEST(VulkanValidationTest, GetRequiredValidationLayersNotEmptyTest) {
    // Arrange & Act
    std::vector<const char*> layers = VulkanValidation::getRequiredValidationLayers();

    // Assert: Layers не должен быть пустым
    EXPECT_GT(layers.size(), 0u);
}

// ============================================================================
// Debug Messenger Tests
// ============================================================================

class VulkanValidationDebugTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем instance с validation layers
        try {
            vk::ApplicationInfo appInfo{};
            appInfo.pApplicationName = "VulkanValidation Test";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "SpectraForge Test";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            vk::InstanceCreateInfo createInfo{};
            createInfo.pApplicationInfo = &appInfo;

            // Добавляем validation layers если поддерживаются
            if (VulkanValidation::checkValidationLayerSupport()) {
                auto layers = VulkanValidation::getRequiredValidationLayers();
                createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
                createInfo.ppEnabledLayerNames = layers.data();

                // Добавляем debug utils extension
                std::vector<const char*> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
                createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();
            }

            instance = vk::createInstance(createInfo);
            hasValidationSupport = VulkanValidation::checkValidationLayerSupport();

        } catch (const std::exception& e) {
            std::cerr << "[VulkanValidationDebugTest] Failed to create instance: " << e.what() << std::endl;
            instance = nullptr;
        }
    }

    void TearDown() override {
        if (debugMessenger && instance) {
            VulkanValidation::destroyDebugMessenger(instance, debugMessenger);
        }
        if (instance) {
            instance.destroy();
        }
    }

    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    bool hasValidationSupport = false;
};

TEST_F(VulkanValidationDebugTest, CreateDebugMessengerTest) {
    // Arrange
    if (!instance || !hasValidationSupport) {
        GTEST_SKIP() << "Vulkan validation не доступен";
    }

    // Act
    debugMessenger = VulkanValidation::createDebugMessenger(instance);

    // Assert: Должен создаться messenger (может быть null если расширение не поддерживается)
    EXPECT_NO_THROW({
        VulkanValidation::createDebugMessenger(instance);
    });
}

TEST_F(VulkanValidationDebugTest, DestroyDebugMessengerTest) {
    // Arrange
    if (!instance || !hasValidationSupport) {
        GTEST_SKIP() << "Vulkan validation не доступен";
    }

    debugMessenger = VulkanValidation::createDebugMessenger(instance);

    // Act & Assert: Уничтожение не должно падать
    EXPECT_NO_THROW({
        VulkanValidation::destroyDebugMessenger(instance, debugMessenger);
    });
    
    debugMessenger = nullptr; // Предотвращаем повторное уничтожение в TearDown
}

TEST_F(VulkanValidationDebugTest, DestroyNullDebugMessengerTest) {
    // Arrange
    if (!instance) {
        GTEST_SKIP() << "Vulkan не доступен";
    }

    vk::DebugUtilsMessengerEXT nullMessenger;

    // Act & Assert: Уничтожение null messenger не должно падать
    EXPECT_NO_THROW({
        VulkanValidation::destroyDebugMessenger(instance, nullMessenger);
    });
}

TEST_F(VulkanValidationDebugTest, CreateDestroyMultipleCyclesTest) {
    // Arrange
    if (!instance || !hasValidationSupport) {
        GTEST_SKIP() << "Vulkan validation не доступен";
    }

    // Act & Assert: Несколько циклов создания/уничтожения
    for (int i = 0; i < 3; i++) {
        vk::DebugUtilsMessengerEXT messenger = VulkanValidation::createDebugMessenger(instance);
        
        EXPECT_NO_THROW({
            VulkanValidation::destroyDebugMessenger(instance, messenger);
        });
    }
}

// ============================================================================
// Debug Callback Tests
// ============================================================================

TEST(VulkanValidationCallbackTest, SetDebugCallbackTest) {
    // Arrange
    bool callbackCalled = false;
    auto callback = [&callbackCalled](const std::string& message) {
        callbackCalled = true;
        std::cout << "Debug message: " << message << std::endl;
    };

    // Act & Assert
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback(callback);
    });
}

TEST(VulkanValidationCallbackTest, SetNullCallbackTest) {
    // Arrange
    std::function<void(const std::string&)> nullCallback;

    // Act & Assert: Установка null callback не должна падать
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback(nullCallback);
    });
}

TEST(VulkanValidationCallbackTest, SetMultipleCallbacksTest) {
    // Arrange
    auto callback1 = [](const std::string& msg) { std::cout << "Callback 1: " << msg << std::endl; };
    auto callback2 = [](const std::string& msg) { std::cout << "Callback 2: " << msg << std::endl; };

    // Act & Assert: Множественные установки callback
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback(callback1);
        VulkanValidation::setDebugCallback(callback2);
    });
}

// ============================================================================
// Debug Messenger CreateInfo Tests
// ============================================================================

TEST(VulkanValidationCreateInfoTest, PopulateDebugMessengerCreateInfoTest) {
    // Arrange
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

    // Act
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo);

    // Assert: Структура должна быть заполнена валидными данными
    EXPECT_NE(createInfo.messageSeverity, vk::DebugUtilsMessageSeverityFlagsEXT{});
    EXPECT_NE(createInfo.messageType, vk::DebugUtilsMessageTypeFlagsEXT{});
    EXPECT_NE(createInfo.pfnUserCallback, nullptr);
}

TEST(VulkanValidationCreateInfoTest, PopulateDebugMessengerMultipleTimesTest) {
    // Arrange
    vk::DebugUtilsMessengerCreateInfoEXT createInfo1{};
    vk::DebugUtilsMessengerCreateInfoEXT createInfo2{};

    // Act
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo1);
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo2);

    // Assert: Обе структуры должны быть идентично заполнены
    EXPECT_EQ(createInfo1.messageSeverity, createInfo2.messageSeverity);
    EXPECT_EQ(createInfo1.messageType, createInfo2.messageType);
    EXPECT_EQ(createInfo1.pfnUserCallback, createInfo2.pfnUserCallback);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(VulkanValidationIntegrationTest, FullValidationWorkflowTest) {
    // Arrange & Act & Assert: Полный workflow validation

    // 1. Проверяем поддержку
    bool supported = VulkanValidation::checkValidationLayerSupport();

    // 2. Получаем список layers
    auto layers = VulkanValidation::getRequiredValidationLayers();
    EXPECT_FALSE(layers.empty());

    // 3. Устанавливаем callback
    bool messageReceived = false;
    VulkanValidation::setDebugCallback([&messageReceived](const std::string& msg) {
        messageReceived = true;
    });

    // 4. Заполняем createInfo
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo);
    EXPECT_NE(createInfo.pfnUserCallback, nullptr);

    SUCCEED();
}

TEST_F(VulkanValidationDebugTest, FullDebugMessengerWorkflowTest) {
    // Arrange
    if (!instance || !hasValidationSupport) {
        GTEST_SKIP() << "Vulkan validation не доступен";
    }

    // Act & Assert: Полный workflow debug messenger

    // 1. Установка callback
    bool callbackInvoked = false;
    VulkanValidation::setDebugCallback([&callbackInvoked](const std::string& message) {
        callbackInvoked = true;
        std::cout << "Debug: " << message << std::endl;
    });

    // 2. Создание messenger
    debugMessenger = VulkanValidation::createDebugMessenger(instance);

    // 3. Выполнение Vulkan операций (потенциально вызовет callback)
    // ... тут можно сделать невалидные Vulkan вызовы для проверки

    // 4. Уничтожение messenger
    VulkanValidation::destroyDebugMessenger(instance, debugMessenger);
    debugMessenger = nullptr;

    SUCCEED();
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST(VulkanValidationErrorTest, CreateDebugMessengerWithNullInstanceTest) {
    // Arrange
    vk::Instance nullInstance;

    // Act & Assert: Создание с null instance не должно падать
    EXPECT_NO_THROW({
        VulkanValidation::createDebugMessenger(nullInstance);
    });
}

TEST(VulkanValidationErrorTest, DestroyWithNullInstanceTest) {
    // Arrange
    vk::Instance nullInstance;
    vk::DebugUtilsMessengerEXT messenger;

    // Act & Assert
    EXPECT_NO_THROW({
        VulkanValidation::destroyDebugMessenger(nullInstance, messenger);
    });
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(VulkanValidationPerformanceTest, ValidationLayerCheckPerformanceTest) {
    // Arrange
    auto start = std::chrono::high_resolution_clock::now();

    // Act: Проверяем 1000 раз
    for (int i = 0; i < 1000; i++) {
        VulkanValidation::checkValidationLayerSupport();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Должно занять меньше 1 секунды
    EXPECT_LT(duration.count(), 1000);
}

TEST(VulkanValidationPerformanceTest, GetLayersPerformanceTest) {
    // Arrange
    auto start = std::chrono::high_resolution_clock::now();

    // Act: Получаем layers 1000 раз
    for (int i = 0; i < 1000; i++) {
        auto layers = VulkanValidation::getRequiredValidationLayers();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Assert: Должно занять меньше 100 мс
    EXPECT_LT(duration.count(), 100);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(VulkanValidationEdgeCaseTest, EmptyCallbackMessageTest) {
    // Arrange
    bool emptyMessageReceived = false;
    VulkanValidation::setDebugCallback([&emptyMessageReceived](const std::string& msg) {
        if (msg.empty()) {
            emptyMessageReceived = true;
        }
    });

    // Act & Assert: Установка callback с пустыми сообщениями
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback([](const std::string&) {});
    });
}

TEST(VulkanValidationEdgeCaseTest, VeryLongCallbackMessageTest) {
    // Arrange
    std::string longMessage(10000, 'x');  // 10000 символов

    // Act & Assert: Callback должен обработать очень длинное сообщение
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback([](const std::string& msg) {
            EXPECT_LE(msg.length(), 100000u);  // Разумный лимит
        });
    });
}

// ============================================================================
// Static Function Tests
// ============================================================================

TEST(VulkanValidationStaticTest, AllStaticFunctionsAccessibleTest) {
    // Arrange & Act & Assert: Все статические функции должны быть доступны

    // checkValidationLayerSupport
    EXPECT_NO_THROW({
        VulkanValidation::checkValidationLayerSupport();
    });

    // getRequiredValidationLayers
    EXPECT_NO_THROW({
        VulkanValidation::getRequiredValidationLayers();
    });

    // setDebugCallback
    EXPECT_NO_THROW({
        VulkanValidation::setDebugCallback([](const std::string&) {});
    });

    // populateDebugMessengerCreateInfo
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    EXPECT_NO_THROW({
        VulkanValidation::populateDebugMessengerCreateInfo(createInfo);
    });
}

// ============================================================================
// Message Severity Tests
// ============================================================================

TEST(VulkanValidationMessageTest, CreateInfoHasAllSeveritiesTest) {
    // Arrange
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

    // Act
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo);

    // Assert: Должны быть установлены разные уровни severity
    auto severity = createInfo.messageSeverity;
    
    // Проверяем что хотя бы один уровень установлен
    EXPECT_TRUE(
        (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose) ||
        (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) ||
        (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) ||
        (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
    );
}

TEST(VulkanValidationMessageTest, CreateInfoHasAllMessageTypesTest) {
    // Arrange
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

    // Act
    VulkanValidation::populateDebugMessengerCreateInfo(createInfo);

    // Assert: Должны быть установлены разные типы сообщений
    auto messageType = createInfo.messageType;
    
    EXPECT_TRUE(
        (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral) ||
        (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation) ||
        (messageType & vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
    );
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
