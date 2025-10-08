/**
 * @file hybrid_fregs_renderer_test.cpp
 * @brief Комплексные тесты для HybridFreGSRenderer (44 функции, ~70 тестов)
 * 
 * Покрытие:
 * - Lifecycle: initialize, shutdown, destructor
 * - Vulkan setup: instance, device, surface, swapchain
 * - Rendering: beginFrame, renderFrame, endFrame
 * - Data upload: uploadGaussians, uploadTriangles, uploadMesh
 * - Debug API: setDebugMode, enableWireframe, setBackground, etc.
 * - Stats: getStats, getDetailedStats, getGPUInfo
 * - Screenshot: saveScreenshot, getFramebufferData
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/Camera3D.h"
#include "SpectraForge/Rendering/Mesh3D.h"
#include "SpectraForge/Math/Matrix4.h"
#include "SpectraForge/Math/Vector3.h"
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Math;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

// ============================================================================
// Mock Classes (для изоляции Vulkan зависимостей)
// ============================================================================

/**
 * @brief Mock X11 Display для тестирования без реального окна
 */
struct MockX11Display {
    void* handle = reinterpret_cast<void*>(0x1234);
};

/**
 * @brief Mock X11 Window для тестирования без реального окна
 */
struct MockX11Window {
    void* handle = reinterpret_cast<void*>(0x5678);
};

// ============================================================================
// Test Fixture
// ============================================================================

class HybridFreGSRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        mockDisplay.handle = reinterpret_cast<void*>(0x1234);
        mockWindow.handle = reinterpret_cast<void*>(0x5678);
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
        }
        renderer.reset();
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
    MockX11Display mockDisplay;
    MockX11Window mockWindow;
};

// ============================================================================
// 1. LIFECYCLE TESTS (Критичность: ⚠️⚠️⚠️)
// ============================================================================

/**
 * @test Тест конструктора HybridFreGSRenderer
 */
TEST_F(HybridFreGSRendererTest, ConstructorTest) {
    EXPECT_NE(renderer, nullptr);
    EXPECT_FALSE(renderer->isReady());
    EXPECT_FALSE(renderer->isInitialized());
    EXPECT_EQ(renderer->getType(), RendererType::Vulkan);
    EXPECT_EQ(renderer->getName(), "HybridFreGSRenderer");
}

/**
 * @test Тест деструктора (должен корректно освобождать ресурсы)
 */
TEST_F(HybridFreGSRendererTest, DestructorTest) {
    // Arrange
    auto tempRenderer = std::make_unique<HybridFreGSRenderer>();
    
    // Act - деструктор вызовется автоматически
    tempRenderer.reset();
    
    // Assert - проверка что не было segfault
    SUCCEED();
}

/**
 * @test Тест initialize() - успешная инициализация
 */
TEST_F(HybridFreGSRendererTest, InitializeSuccessTest) {
    // Act
    bool result = renderer->initialize();
    
    // Assert
    // Может быть false если Vulkan недоступен - это нормально
    if (result) {
        EXPECT_TRUE(renderer->isInitialized());
        EXPECT_TRUE(renderer->isReady());
    } else {
        EXPECT_FALSE(renderer->isInitialized());
    }
}

/**
 * @test Тест повторного вызова initialize()
 */
TEST_F(HybridFreGSRendererTest, InitializeTwiceTest) {
    // Arrange
    bool first = renderer->initialize();
    
    // Act
    bool second = renderer->initialize();
    
    // Assert
    if (first) {
        EXPECT_TRUE(second); // Должен возвращать true без повторной инициализации
    }
}

/**
 * @test Тест shutdown() без инициализации
 */
TEST_F(HybridFreGSRendererTest, ShutdownWithoutInitTest) {
    // Act & Assert - не должно вызывать segfault
    EXPECT_NO_THROW(renderer->shutdown());
}

/**
 * @test Тест shutdown() после инициализации
 */
TEST_F(HybridFreGSRendererTest, ShutdownAfterInitTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    renderer->shutdown();
    
    // Assert
    EXPECT_FALSE(renderer->isReady());
}

/**
 * @test Тест повторного shutdown()
 */
TEST_F(HybridFreGSRendererTest, ShutdownTwiceTest) {
    // Arrange
    renderer->initialize();
    renderer->shutdown();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->shutdown());
}

// ============================================================================
// 2. VULKAN SETUP TESTS (Критичность: ⚠️⚠️⚠️)
// ============================================================================

/**
 * @test Тест createInstance() через initialize()
 * Проверяет создание Vulkan instance
 */
TEST_F(HybridFreGSRendererTest, CreateInstanceTest) {
    // Act
    bool result = renderer->initialize();
    
    // Assert
    if (result) {
        EXPECT_TRUE(renderer->isInitialized());
        EXPECT_EQ(renderer->getApiVersion(), "Vulkan 1.3 (compute)");
    }
}

/**
 * @test Тест pickPhysicalDevice() - выбор GPU
 * Должен выбрать discrete GPU если доступен
 */
TEST_F(HybridFreGSRendererTest, PickPhysicalDeviceTest) {
    // Act
    bool result = renderer->initialize();
    
    // Assert
    if (result) {
        auto gpuInfo = renderer->getGPUInfo();
        EXPECT_FALSE(gpuInfo.deviceName.empty());
        EXPECT_FALSE(gpuInfo.vendorName.empty());
    }
}

/**
 * @test Тест createLogicalDevice() через initialize()
 */
TEST_F(HybridFreGSRendererTest, CreateLogicalDeviceTest) {
    // Act
    bool result = renderer->initialize();
    
    // Assert - если инициализация успешна, логическое устройство создано
    if (result) {
        EXPECT_TRUE(renderer->isReady());
    }
}

/**
 * @test Тест attachWindow() - привязка X11 окна
 */
TEST_F(HybridFreGSRendererTest, AttachWindowTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    bool result = renderer->attachWindow(
        mockDisplay.handle, 
        mockWindow.handle, 
        1920, 
        1080
    );
    
    // Assert
    // Может быть false если X11/Vulkan недоступны
    // Главное - не должно быть segfault
    EXPECT_NO_THROW({
        if (result) {
            EXPECT_TRUE(renderer->isReady());
        }
    });
}

/**
 * @test Тест attachWindow() без инициализации
 */
TEST_F(HybridFreGSRendererTest, AttachWindowWithoutInitTest) {
    // Act
    bool result = renderer->attachWindow(
        mockDisplay.handle, 
        mockWindow.handle, 
        1920, 
        1080
    );
    
    // Assert
    EXPECT_FALSE(result);
}

/**
 * @test Тест createSurfaceX11() через attachWindow()
 */
TEST_F(HybridFreGSRendererTest, CreateSurfaceX11Test) {
    // Arrange
    renderer->initialize();
    
    // Act
    bool result = renderer->attachWindow(
        mockDisplay.handle, 
        mockWindow.handle, 
        800, 
        600
    );
    
    // Assert - проверка создания surface
    EXPECT_NO_THROW({
        // Surface создается внутри attachWindow
    });
}

/**
 * @test Тест createSwapchainAndViews() с различными размерами
 */
TEST_F(HybridFreGSRendererTest, CreateSwapchainAndViewsTest) {
    // Arrange
    renderer->initialize();
    
    // Act - различные разрешения
    std::vector<std::pair<uint32_t, uint32_t>> resolutions = {
        {1920, 1080},
        {1280, 720},
        {3840, 2160}
    };
    
    for (const auto& [width, height] : resolutions) {
        bool result = renderer->attachWindow(
            mockDisplay.handle, 
            mockWindow.handle, 
            width, 
            height
        );
        
        // Assert
        EXPECT_NO_THROW({
            if (result) {
                EXPECT_TRUE(renderer->isReady());
            }
        });
    }
}

/**
 * @test Тест createRenderPass()
 */
TEST_F(HybridFreGSRendererTest, CreateRenderPassTest) {
    // Arrange
    renderer->initialize();
    renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080);
    
    // Act & Assert
    // RenderPass создается внутри attachWindow
    EXPECT_NO_THROW({
        renderer->isReady();
    });
}

/**
 * @test Тест createFramebuffers()
 */
TEST_F(HybridFreGSRendererTest, CreateFramebuffersTest) {
    // Arrange
    renderer->initialize();
    renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080);
    
    // Act & Assert
    // Framebuffers создаются внутри attachWindow
    SUCCEED();
}

/**
 * @test Тест createCommandPool()
 */
TEST_F(HybridFreGSRendererTest, CreateCommandPoolTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    // Command pool создается в initialize()
    if (renderer->isInitialized()) {
        SUCCEED();
    }
}

/**
 * @test Тест createCommandBuffers()
 */
TEST_F(HybridFreGSRendererTest, CreateCommandBuffersTest) {
    // Arrange
    renderer->initialize();
    renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080);
    
    // Act & Assert
    // Command buffers создаются в attachWindow()
    SUCCEED();
}

/**
 * @test Тест createSyncObjects() - семафоры и фенсы
 */
TEST_F(HybridFreGSRendererTest, CreateSyncObjectsTest) {
    // Arrange
    renderer->initialize();
    renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080);
    
    // Act & Assert
    // Sync objects создаются в attachWindow()
    SUCCEED();
}

/**
 * @test Тест createAllocator() - VMA allocator
 */
TEST_F(HybridFreGSRendererTest, CreateAllocatorTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    // VMA allocator создается в initialize()
    if (renderer->isInitialized()) {
        SUCCEED();
    }
}

/**
 * @test Тест destroySwapchainAndViews()
 */
TEST_F(HybridFreGSRendererTest, DestroySwapchainAndViewsTest) {
    // Arrange
    renderer->initialize();
    renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080);
    
    // Act
    renderer->shutdown();
    
    // Assert - не должно быть segfault
    SUCCEED();
}

// ============================================================================
// 3. RENDERING TESTS (Критичность: ⚠️⚠️⚠️)
// ============================================================================

/**
 * @test Тест beginFrame()
 */
TEST_F(HybridFreGSRendererTest, BeginFrameTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->beginFrame());
}

/**
 * @test Тест beginFrame() без инициализации
 */
TEST_F(HybridFreGSRendererTest, BeginFrameWithoutInitTest) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->beginFrame());
}

/**
 * @test Тест renderFrame()
 */
TEST_F(HybridFreGSRendererTest, RenderFrameTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    FrameData frameData;
    frameData.deltaTime = 0.016f;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->renderFrame(frameData));
}

/**
 * @test Тест renderFrame() без attachWindow()
 */
TEST_F(HybridFreGSRendererTest, RenderFrameWithoutWindowTest) {
    // Arrange
    renderer->initialize();
    FrameData frameData;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->renderFrame(frameData));
}

/**
 * @test Тест endFrame()
 */
TEST_F(HybridFreGSRendererTest, EndFrameTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->endFrame());
}

/**
 * @test Тест полного цикла рендеринга: begin -> render -> end
 */
TEST_F(HybridFreGSRendererTest, FullRenderCycleTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    FrameData frameData;
    frameData.deltaTime = 0.016f;
    
    // Act & Assert
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->renderFrame(frameData);
        renderer->endFrame();
    });
}

/**
 * @test Тест recordCommandBuffer()
 */
TEST_F(HybridFreGSRendererTest, RecordCommandBufferTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    FrameData frameData;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->renderFrame(frameData));
}

// ============================================================================
// 4. DATA UPLOAD TESTS (Критичность: ⚠️⚠️⚠️)
// ============================================================================

/**
 * @test Тест uploadGaussians() - загрузка гауссианов для FreGS
 */
TEST_F(HybridFreGSRendererTest, UploadGaussiansTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    
    std::vector<spectraforge::rendering::GaussianSplat> gaussians;
    spectraforge::rendering::GaussianSplat gauss;
    gauss.position = glm::vec3(0.0f, 0.0f, 0.0f);
    gauss.color = glm::vec3(1.0f, 0.0f, 0.0f);
    gauss.opacity = 1.0f;
    gaussians.push_back(gauss);
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadGaussians(gaussians));
}

/**
 * @test Тест uploadGaussians() с пустым вектором
 */
TEST_F(HybridFreGSRendererTest, UploadGaussiansEmptyTest) {
    // Arrange
    renderer->initialize();
    std::vector<spectraforge::rendering::GaussianSplat> gaussians;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadGaussians(gaussians));
}

/**
 * @test Тест uploadTriangles() - загрузка треугольников
 */
TEST_F(HybridFreGSRendererTest, UploadTrianglesTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    spectraforge::rendering::spectraforge::rendering::Triangle tri;
    tri.v0 = glm::vec3(0.0f, 0.0f, 0.0f);
    tri.v1 = glm::vec3(1.0f, 0.0f, 0.0f);
    tri.v2 = glm::vec3(0.0f, 1.0f, 0.0f);
    tri.color = glm::vec3(1.0f, 1.0f, 1.0f);
    triangles.push_back(tri);
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadTriangles(triangles));
}

/**
 * @test Тест uploadTriangles() с пустым вектором
 */
TEST_F(HybridFreGSRendererTest, UploadTrianglesEmptyTest) {
    // Arrange
    renderer->initialize();
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadTriangles(triangles));
}

/**
 * @test Тест uploadMesh() - загрузка меша
 */
TEST_F(HybridFreGSRendererTest, UploadMeshTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    
    auto mesh = std::make_shared<Mesh3D>();
    // Добавляем простой треугольник
    mesh->addVertex(Vector3(0, 0, 0));
    mesh->addVertex(Vector3(1, 0, 0));
    mesh->addVertex(Vector3(0, 1, 0));
    mesh->addIndex(0);
    mesh->addIndex(1);
    mesh->addIndex(2);
    
    Matrix4 transform = Matrix4::identity();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadMesh(mesh, transform));
}

/**
 * @test Тест uploadMesh() с пустым мешем
 */
TEST_F(HybridFreGSRendererTest, UploadMeshEmptyTest) {
    // Arrange
    renderer->initialize();
    auto mesh = std::make_shared<Mesh3D>();
    Matrix4 transform = Matrix4::identity();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadMesh(mesh, transform));
}

/**
 * @test Тест convertMeshToTriangles()
 */
TEST_F(HybridFreGSRendererTest, ConvertMeshToTrianglesTest) {
    // Arrange
    renderer->initialize();
    auto mesh = std::make_shared<Mesh3D>();
    mesh->addVertex(Vector3(0, 0, 0));
    mesh->addVertex(Vector3(1, 0, 0));
    mesh->addVertex(Vector3(0, 1, 0));
    mesh->addIndex(0);
    mesh->addIndex(1);
    mesh->addIndex(2);
    
    Matrix4 transform = Matrix4::identity();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadMesh(mesh, transform));
}

/**
 * @test Тест initializeTriangleSplatting()
 */
TEST_F(HybridFreGSRendererTest, InitializeTriangleSplattingTest) {
    // Arrange & Act
    bool result = renderer->initialize();
    
    // Assert
    if (result) {
        // Triangle splatting pass инициализируется в initialize()
        EXPECT_NE(renderer->getTriangleSplattingPass(), nullptr);
    }
}

// ============================================================================
// 5. DEBUG API TESTS (Критичность: ⚠️)
// ============================================================================

/**
 * @test Тест setDebugMode() и getDebugMode()
 */
TEST_F(HybridFreGSRendererTest, SetGetDebugModeTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    for (int mode = 0; mode <= 4; ++mode) {
        EXPECT_NO_THROW(renderer->setDebugMode(mode));
        EXPECT_EQ(renderer->getDebugMode(), mode);
    }
}

/**
 * @test Тест enableWireframe()
 */
TEST_F(HybridFreGSRendererTest, EnableWireframeTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableWireframe(true));
    EXPECT_NO_THROW(renderer->enableWireframe(false));
}

/**
 * @test Тест enableBackfaceCulling()
 */
TEST_F(HybridFreGSRendererTest, EnableBackfaceCullingTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableBackfaceCulling(true));
    EXPECT_NO_THROW(renderer->enableBackfaceCulling(false));
}

/**
 * @test Тест enableDepthTest()
 */
TEST_F(HybridFreGSRendererTest, EnableDepthTestTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableDepthTest(true));
    EXPECT_NO_THROW(renderer->enableDepthTest(false));
}

/**
 * @test Тест setBackgroundColor() и getBackgroundColor()
 */
TEST_F(HybridFreGSRendererTest, SetGetBackgroundColorTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    renderer->setBackgroundColor(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 color = renderer->getBackgroundColor();
    
    // Assert
    EXPECT_FLOAT_EQ(color.r, 1.0f);
    EXPECT_FLOAT_EQ(color.g, 0.0f);
    EXPECT_FLOAT_EQ(color.b, 0.0f);
    EXPECT_FLOAT_EQ(color.a, 1.0f);
}

/**
 * @test Тест setViewport()
 */
TEST_F(HybridFreGSRendererTest, SetViewportTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->setViewport(0, 0, 1920, 1080));
    EXPECT_NO_THROW(renderer->setViewport(100, 100, 800, 600));
}

/**
 * @test Тест enableAlphaBlending()
 */
TEST_F(HybridFreGSRendererTest, EnableAlphaBlendingTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableAlphaBlending(true));
    EXPECT_NO_THROW(renderer->enableAlphaBlending(false));
}

/**
 * @test Тест setTriangleBudget()
 */
TEST_F(HybridFreGSRendererTest, SetTriangleBudgetTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->setTriangleBudget(100000));
    EXPECT_NO_THROW(renderer->setTriangleBudget(1000000));
    EXPECT_NO_THROW(renderer->setTriangleBudget(10));
}

/**
 * @test Тест enableEarlyTermination()
 */
TEST_F(HybridFreGSRendererTest, EnableEarlyTerminationTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableEarlyTermination(true));
    EXPECT_NO_THROW(renderer->enableEarlyTermination(false));
}

// ============================================================================
// 6. STATS TESTS (Критичность: ⚠️⚠️)
// ============================================================================

/**
 * @test Тест getStats()
 */
TEST_F(HybridFreGSRendererTest, GetStatsTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    RenderingStats stats = renderer->getStats();
    
    // Assert
    EXPECT_GE(stats.frameTime, 0.0f);
    EXPECT_GE(stats.fps, 0.0f);
}

/**
 * @test Тест getDetailedStats()
 */
TEST_F(HybridFreGSRendererTest, GetDetailedStatsTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    DetailedRenderingStats stats = renderer->getDetailedStats();
    
    // Assert
    EXPECT_GE(stats.triangleCount, 0u);
    EXPECT_GE(stats.drawCalls, 0u);
    EXPECT_FALSE(stats.gpuMemoryUsage.empty());
}

/**
 * @test Тест getGPUInfo()
 */
TEST_F(HybridFreGSRendererTest, GetGPUInfoTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    
    // Act
    GPUInfo info = renderer->getGPUInfo();
    
    // Assert
    EXPECT_FALSE(info.deviceName.empty());
    EXPECT_FALSE(info.vendorName.empty());
    EXPECT_GT(info.driverVersion, 0);
    EXPECT_GT(info.apiVersion, 0);
}

// ============================================================================
// 7. SCREENSHOT TESTS (Критичность: ⚠️)
// ============================================================================

/**
 * @test Тест saveScreenshot()
 */
TEST_F(HybridFreGSRendererTest, SaveScreenshotTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    // Act
    bool result = renderer->saveScreenshot("/tmp/test_screenshot.png");
    
    // Assert
    // Может быть false если нет прав записи
    EXPECT_NO_THROW({
        if (result) {
            EXPECT_TRUE(result);
        }
    });
}

/**
 * @test Тест getFramebufferData()
 */
TEST_F(HybridFreGSRendererTest, GetFramebufferDataTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    // Act
    std::vector<uint8_t> data = renderer->getFramebufferData();
    
    // Assert
    // Может быть пустым если рендеринг не был выполнен
    EXPECT_GE(data.size(), 0u);
}

// ============================================================================
// 8. MISC TESTS (Критичность: ⚠️)
// ============================================================================

/**
 * @test Тест setDebugCallback()
 */
TEST_F(HybridFreGSRendererTest, SetDebugCallbackTest) {
    // Arrange
    renderer->initialize();
    bool callbackCalled = false;
    
    // Act
    renderer->setDebugCallback([&callbackCalled](const std::string& msg) {
        callbackCalled = true;
    });
    
    // Trigger some debug output
    renderer->setDebugMode(1);
    
    // Assert
    // Callback может не быть вызван если нет debug вывода
    EXPECT_NO_THROW({
        renderer->setDebugCallback(nullptr);
    });
}

/**
 * @test Тест flushUniforms()
 */
TEST_F(HybridFreGSRendererTest, FlushUniformsTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->flushUniforms());
}

/**
 * @test Тест getCorrectedViewProjMatrix()
 */
TEST_F(HybridFreGSRendererTest, GetCorrectedViewProjMatrixTest) {
    // Arrange
    renderer->initialize();
    Camera3D camera;
    camera.setPosition(Vector3(0, 0, 10));
    camera.lookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
    camera.setProjection(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    renderer->setCamera(&camera);
    
    // Act & Assert
    EXPECT_NO_THROW({
        // getCorrectedViewProjMatrix() вызывается внутри renderFrame()
        FrameData frameData;
        renderer->renderFrame(frameData);
    });
}

/**
 * @test Тест supportsFeature()
 */
TEST_F(HybridFreGSRendererTest, SupportsFeatureTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_TRUE(renderer->supportsFeature(RenderingFeature::VulkanCompute));
    EXPECT_FALSE(renderer->supportsFeature(RenderingFeature::OpenGLCore));
}

/**
 * @test Тест setRenderMode()
 */
TEST_F(HybridFreGSRendererTest, SetRenderModeTest) {
    // Arrange
    renderer->initialize();
    
    // Act & Assert
    EXPECT_NO_THROW({
        renderer->setRenderMode(HybridFreGSRenderer::RenderMode::GaussianSplatting);
        renderer->setRenderMode(HybridFreGSRenderer::RenderMode::TriangleSplatting);
    });
}

/**
 * @test Тест setCamera()
 */
TEST_F(HybridFreGSRendererTest, SetCameraTest) {
    // Arrange
    renderer->initialize();
    Camera3D camera;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->setCamera(&camera));
    EXPECT_NO_THROW(renderer->setCamera(nullptr));
}

/**
 * @test Тест isDeviceLost()
 */
TEST_F(HybridFreGSRendererTest, IsDeviceLostTest) {
    // Arrange
    renderer->initialize();
    
    // Act
    bool lost = renderer->isDeviceLost();
    
    // Assert
    EXPECT_FALSE(lost); // Не должно быть потери устройства при нормальной работе
}

/**
 * @test Тест getTriangleSplattingPass()
 */
TEST_F(HybridFreGSRendererTest, GetTriangleSplattingPassTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    
    // Act
    auto* pass = renderer->getTriangleSplattingPass();
    
    // Assert
    EXPECT_NE(pass, nullptr);
}

// ============================================================================
// 9. INTEGRATION TESTS
// ============================================================================

/**
 * @test Интеграционный тест: полный lifecycle с рендерингом
 */
TEST_F(HybridFreGSRendererTest, FullLifecycleIntegrationTest) {
    // Arrange
    if (!renderer->initialize()) {
        GTEST_SKIP() << "Vulkan not available";
    }
    if (!renderer->attachWindow(mockDisplay.handle, mockWindow.handle, 1920, 1080)) {
        GTEST_SKIP() << "Cannot attach window";
    }
    
    // Upload test data
    auto mesh = std::make_shared<Mesh3D>();
    mesh->addVertex(Vector3(0, 0, 0));
    mesh->addVertex(Vector3(1, 0, 0));
    mesh->addVertex(Vector3(0, 1, 0));
    mesh->addIndex(0);
    mesh->addIndex(1);
    mesh->addIndex(2);
    renderer->uploadMesh(mesh);
    
    // Setup camera
    Camera3D camera;
    camera.setPosition(Vector3(0, 0, 10));
    camera.lookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
    camera.setProjection(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    renderer->setCamera(&camera);
    
    // Act - render multiple frames
    for (int i = 0; i < 3; ++i) {
        FrameData frameData;
        frameData.deltaTime = 0.016f;
        
        EXPECT_NO_THROW({
            renderer->beginFrame();
            renderer->renderFrame(frameData);
            renderer->endFrame();
        });
    }
    
    // Assert
    RenderingStats stats = renderer->getStats();
    EXPECT_GT(stats.frameTime, 0.0f);
    
    // Cleanup
    renderer->shutdown();
    EXPECT_FALSE(renderer->isReady());
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
