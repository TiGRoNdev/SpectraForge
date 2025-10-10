/**
 * @file vulkan_renderer_test.cpp
 * @brief Тесты минимального VulkanRenderer без заглушек.
 */

#include <gtest/gtest.h>
#include "SpectraForge/Vulkan/VulkanRenderer.h"
#include "SpectraForge/Vulkan/ResourceManager.h"
#include <vulkan/vulkan.hpp>
#include <memory>
#include <iostream>

using namespace SpectraForge::Vulkan;

namespace {

class MockVulkanSetupForRenderer {
public:
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    std::unique_ptr<ResourceManager> resourceManager;
    uint32_t queueFamilyIndex = 0;

    bool create() {
        try {
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

            auto devices = instance.enumeratePhysicalDevices();
            if (devices.empty()) {
                cleanup();
                return false;
            }
            physicalDevice = devices[0];

            auto queueFamilies = physicalDevice.getQueueFamilyProperties();
            for (uint32_t i = 0; i < queueFamilies.size(); i++) {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    queueFamilyIndex = i;
                    break;
                }
            }

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

}  // namespace

TEST(VulkanRendererBasicTest, InitFailsWithoutDeviceAndResourceManager) {
    VulkanRenderer renderer;
    vk::Device device;
    EXPECT_FALSE(renderer.init(device, nullptr));
}

TEST_F(VulkanRendererTest, InitSucceedsWithValidDependencies) {
    EXPECT_TRUE(renderer->init(mockSetup.device, mockSetup.resourceManager.get()));
    EXPECT_TRUE(renderer->isInitialized());
}

TEST_F(VulkanRendererTest, InitFailsWithoutResourceManager) {
    EXPECT_FALSE(renderer->init(mockSetup.device, nullptr));
}

TEST_F(VulkanRendererTest, SetRenderSettingsRejectsInvalidDimensions) {
    EXPECT_TRUE(renderer->init(mockSetup.device, mockSetup.resourceManager.get()));

    RenderSettings invalid{0, 720};
    EXPECT_THROW(renderer->setRenderSettings(invalid), std::invalid_argument);
}

TEST_F(VulkanRendererTest, RasterizePrimaryRequiresInitialization) {
    Gaussians gaussians;
    gaussians.count = 4;
    EXPECT_THROW(renderer->rasterizePrimary(gaussians), std::runtime_error);
}

TEST_F(VulkanRendererTest, RasterizePrimaryRequiresSettings) {
    EXPECT_TRUE(renderer->init(mockSetup.device, mockSetup.resourceManager.get()));
    Gaussians gaussians;
    gaussians.count = 8;
    EXPECT_THROW(renderer->rasterizePrimary(gaussians), std::runtime_error);
}

TEST_F(VulkanRendererTest, RasterizePrimaryReturnsConfiguredDimensions) {
    EXPECT_TRUE(renderer->init(mockSetup.device, mockSetup.resourceManager.get()));
    renderer->setRenderSettings(RenderSettings{1280, 720});

    Gaussians gaussians;
    gaussians.count = 16;

    PrimaryImage image = renderer->rasterizePrimary(gaussians);
    EXPECT_EQ(image.width, 1280u);
    EXPECT_EQ(image.height, 720u);
}

TEST_F(VulkanRendererTest, ShutdownResetsInitialization) {
    EXPECT_TRUE(renderer->init(mockSetup.device, mockSetup.resourceManager.get()));
    renderer->setRenderSettings(RenderSettings{800, 600});

    renderer->shutdown();
    EXPECT_FALSE(renderer->isInitialized());
    EXPECT_THROW(renderer->rasterizePrimary(Gaussians{1}), std::runtime_error);
}
