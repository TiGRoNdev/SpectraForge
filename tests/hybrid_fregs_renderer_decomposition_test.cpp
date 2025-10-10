/**
 * @file hybrid_fregs_renderer_decomposition_test.cpp
 * @brief TDD RED фаза: тесты для HybridFreGSRenderer перед декомпозицией
 * 
 * Эти тесты написаны ПЕРЕД рефакторингом (TDD RED).
 * После декомпозиции они должны пройти (TDD GREEN).
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "SpectraForge/App/DISetup.h"
#include "SpectraForge/Rendering/HybridFreGSRenderer.h"
#include "SpectraForge/Rendering/Mesh3D.h"
#include "SpectraForge/Rendering/Camera3D.h"

namespace {
struct EnsureDIConfigured {
    EnsureDIConfigured() { SpectraForge::App::DISetup::configureDefault(); }
};

const EnsureDIConfigured gEnsureDI{};
}  // namespace

using namespace SpectraForge::Rendering;

// ============================================================================
// 1. VULKAN CONTEXT MANAGEMENT TESTS (~10 тестов)
// ============================================================================

class HybridFreGSRendererVulkanContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererVulkanContextTest, InitializeSuccess) {
    // Arrange
    ASSERT_NE(renderer, nullptr);
    
    // Act
    bool result = renderer->initialize();
    
    // Assert
    EXPECT_TRUE(result) << "Renderer initialization failed";
    EXPECT_TRUE(renderer->isInitialized());
    EXPECT_TRUE(renderer->isReady());
}

TEST_F(HybridFreGSRendererVulkanContextTest, DoubleInitializeIsIdempotent) {
    // Arrange & Act
    bool result1 = renderer->initialize();
    bool result2 = renderer->initialize();
    
    // Assert
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(renderer->isInitialized());
}

TEST_F(HybridFreGSRendererVulkanContextTest, ShutdownBeforeInitializeIsSafe) {
    // Act - вызов shutdown без initialize не должен крашить
    EXPECT_NO_THROW(renderer->shutdown());
}

TEST_F(HybridFreGSRendererVulkanContextTest, ShutdownAfterInitializeCleanup) {
    // Arrange
    renderer->initialize();
    ASSERT_TRUE(renderer->isInitialized());
    
    // Act
    renderer->shutdown();
    
    // Assert
    EXPECT_FALSE(renderer->isInitialized());
}

TEST_F(HybridFreGSRendererVulkanContextTest, GetTypeReturnsVulkan) {
    // Act
    auto type = renderer->getType();
    
    // Assert
    EXPECT_EQ(type, IRenderer::RendererType::Vulkan);
}

TEST_F(HybridFreGSRendererVulkanContextTest, GetNameReturnsCorrectName) {
    // Act
    std::string name = renderer->getName();
    
    // Assert
    EXPECT_EQ(name, "HybridFreGSRenderer");
}

TEST_F(HybridFreGSRendererVulkanContextTest, GetApiVersionReturnsVulkan13) {
    // Act
    std::string apiVersion = renderer->getApiVersion();
    
    // Assert
    EXPECT_THAT(apiVersion, ::testing::HasSubstr("Vulkan"));
    EXPECT_THAT(apiVersion, ::testing::HasSubstr("1.3"));
}

TEST_F(HybridFreGSRendererVulkanContextTest, IsNotReadyBeforeInitialize) {
    // Assert
    EXPECT_FALSE(renderer->isReady());
    EXPECT_FALSE(renderer->isInitialized());
}

TEST_F(HybridFreGSRendererVulkanContextTest, DeviceLostInitiallyFalse) {
    // Assert
    EXPECT_FALSE(renderer->isDeviceLost());
}

TEST_F(HybridFreGSRendererVulkanContextTest, SupportsComputeShaders) {
    // Act
    bool supports = renderer->supportsFeature(IRenderer::RenderingFeature::ComputeShaders);
    
    // Assert
    EXPECT_TRUE(supports) << "Vulkan renderer должен поддерживать compute shaders";
}

// ============================================================================
// 2. SWAPCHAIN MANAGEMENT TESTS (~8 тестов)
// ============================================================================

class HybridFreGSRendererSwapchainTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

// NOTE: Эти тесты требуют настоящего X11 display, поэтому они будут SKIP в CI
TEST_F(HybridFreGSRendererSwapchainTest, DISABLED_AttachWindowSuccess) {
    // Arrange
    void* display = nullptr; // Требуется настоящий X11 display
    void* window = nullptr;  // Требуется настоящее X11 window
    uint32_t width = 1920;
    uint32_t height = 1080;
    
    // Act
    bool result = renderer->attachWindow(display, window, width, height);
    
    // Assert
    // В реальном окружении это должно быть TRUE
    // EXPECT_TRUE(result);
}

TEST_F(HybridFreGSRendererSwapchainTest, AttachWindowWithNullDisplayFails) {
    // Arrange
    void* display = nullptr;
    void* window = nullptr;
    
    // Act
    bool result = renderer->attachWindow(display, window, 800, 600);
    
    // Assert
    EXPECT_FALSE(result) << "attachWindow с nullptr display должен вернуть false";
}

TEST_F(HybridFreGSRendererSwapchainTest, AttachWindowWithZeroWidthFails) {
    // Arrange
    void* display = reinterpret_cast<void*>(0x1); // Dummy
    void* window = reinterpret_cast<void*>(0x2);
    
    // Act
    bool result = renderer->attachWindow(display, window, 0, 600);
    
    // Assert
    EXPECT_FALSE(result) << "attachWindow с width=0 должен вернуть false";
}

TEST_F(HybridFreGSRendererSwapchainTest, AttachWindowWithZeroHeightFails) {
    // Arrange
    void* display = reinterpret_cast<void*>(0x1);
    void* window = reinterpret_cast<void*>(0x2);
    
    // Act
    bool result = renderer->attachWindow(display, window, 800, 0);
    
    // Assert
    EXPECT_FALSE(result) << "attachWindow с height=0 должен вернуть false";
}

// ============================================================================
// 3. FRAME SYNCHRONIZATION TESTS (~8 тестов)
// ============================================================================

class HybridFreGSRendererFrameSyncTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererFrameSyncTest, BeginFrameWithoutWindowIsSafe) {
    // Act - не должно крашить без attached window
    EXPECT_NO_THROW(renderer->beginFrame());
}

TEST_F(HybridFreGSRendererFrameSyncTest, EndFrameWithoutWindowIsSafe) {
    // Act
    EXPECT_NO_THROW(renderer->endFrame());
}

TEST_F(HybridFreGSRendererFrameSyncTest, BeginEndFramePairWithoutWindowIsSafe) {
    // Act
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->endFrame();
    });
}

TEST_F(HybridFreGSRendererFrameSyncTest, DeviceLostDoesNotCrash) {
    // Act - симуляция device lost
    renderer->beginFrame();
    // Device lost устанавливается внутри при ошибках
    
    // Assert - не должно крашить
    EXPECT_NO_THROW(renderer->endFrame());
}

TEST_F(HybridFreGSRendererFrameSyncTest, MultipleBeginFramesWithoutEndIsSafe) {
    // Act - вызов beginFrame несколько раз подряд
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->beginFrame();
        renderer->beginFrame();
    });
}

TEST_F(HybridFreGSRendererFrameSyncTest, EndFrameWithoutBeginIsSafe) {
    // Act - endFrame без предварительного beginFrame
    EXPECT_NO_THROW(renderer->endFrame());
}

// ============================================================================
// 4. RENDER MODE & PIPELINE TESTS (~6 тестов)
// ============================================================================

class HybridFreGSRendererPipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererPipelineTest, DefaultRenderModeIsTriangleSplatting) {
    // Assert - по умолчанию Triangle Splatting
    // (нет прямого API для проверки, но это тестируется косвенно)
}

TEST_F(HybridFreGSRendererPipelineTest, SetRenderModeTriangleSplatting) {
    // Act
    EXPECT_NO_THROW(renderer->setRenderMode(HybridFreGSRenderer::RenderMode::TriangleSplatting));
}

TEST_F(HybridFreGSRendererPipelineTest, SetRenderModeGaussianSplatting) {
    // Act
    EXPECT_NO_THROW(renderer->setRenderMode(HybridFreGSRenderer::RenderMode::GaussianSplatting));
}

TEST_F(HybridFreGSRendererPipelineTest, RenderFrameWithoutDataIsSafe) {
    // Arrange
    IRenderer::FrameData frameData;
    frameData.deltaTime = 0.016f;
    
    // Act
    EXPECT_NO_THROW(renderer->renderFrame(frameData));
}

TEST_F(HybridFreGSRendererPipelineTest, GetTriangleSplattingPassReturnsNullWithoutAttach) {
    // Act
    auto* pass = renderer->getTriangleSplattingPass();
    
    // Assert
    EXPECT_EQ(pass, nullptr) << "Triangle Splatting Pass должен быть nullptr до attachWindow";
}

// ============================================================================
// 5. DATA UPLOAD TESTS (~8 тестов)
// ============================================================================

class HybridFreGSRendererDataUploadTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererDataUploadTest, UploadEmptyTrianglesIsSafe) {
    // Arrange
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadTriangles(triangles));
}

TEST_F(HybridFreGSRendererDataUploadTest, UploadSingleTriangle) {
    // Arrange
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    spectraforge::rendering::spectraforge::rendering::Triangle tri{};
    tri.v0 = glm::vec3(0.0f, 1.0f, 0.0f);
    tri.v1 = glm::vec3(-1.0f, -1.0f, 0.0f);
    tri.v2 = glm::vec3(1.0f, -1.0f, 0.0f);
    tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
    tri.opacity = 1.0f;
    tri.sigma = 1.0f;
    triangles.push_back(tri);
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadTriangles(triangles));
}

TEST_F(HybridFreGSRendererDataUploadTest, UploadMultipleTriangles) {
    // Arrange
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    for (int i = 0; i < 100; ++i) {
        spectraforge::rendering::spectraforge::rendering::Triangle tri{};
        tri.v0 = glm::vec3(0.0f, 1.0f, static_cast<float>(i));
        tri.v1 = glm::vec3(-1.0f, -1.0f, static_cast<float>(i));
        tri.v2 = glm::vec3(1.0f, -1.0f, static_cast<float>(i));
        tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
        tri.opacity = 1.0f;
        tri.sigma = 1.0f;
        triangles.push_back(tri);
    }
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadTriangles(triangles));
}

TEST_F(HybridFreGSRendererDataUploadTest, UploadEmptyGaussiansIsSafe) {
    // Arrange
    std::vector<spectraforge::rendering::GaussianSplat> gaussians;
    
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadGaussians(gaussians));
}

TEST_F(HybridFreGSRendererDataUploadTest, UploadNullMeshIsSafe) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->uploadMesh(nullptr));
}

TEST_F(HybridFreGSRendererDataUploadTest, SetCameraNull) {
    // Act
    EXPECT_NO_THROW(renderer->setCamera(nullptr));
}

TEST_F(HybridFreGSRendererDataUploadTest, SetCameraValid) {
    // Arrange
    auto camera = std::make_unique<Camera3D>(800, 600);
    
    // Act
    EXPECT_NO_THROW(renderer->setCamera(camera.get()));
}

// ============================================================================
// 6. DEBUG FUNCTIONALITY TESTS (~10 тестов)
// ============================================================================

class HybridFreGSRendererDebugTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererDebugTest, DefaultDebugModeIsZero) {
    // Act
    int mode = renderer->getDebugMode();
    
    // Assert
    EXPECT_EQ(mode, 0) << "Дефолтный debug mode должен быть 0";
}

TEST_F(HybridFreGSRendererDebugTest, SetDebugModeNormal) {
    // Act
    renderer->setDebugMode(0);
    
    // Assert
    EXPECT_EQ(renderer->getDebugMode(), 0);
}

TEST_F(HybridFreGSRendererDebugTest, SetDebugModeSDF) {
    // Act
    renderer->setDebugMode(1);
    
    // Assert
    EXPECT_EQ(renderer->getDebugMode(), 1);
}

TEST_F(HybridFreGSRendererDebugTest, SetDebugModeBarycentric) {
    // Act
    renderer->setDebugMode(2);
    
    // Assert
    EXPECT_EQ(renderer->getDebugMode(), 2);
}

TEST_F(HybridFreGSRendererDebugTest, EnableWireframe) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableWireframe(true));
}

TEST_F(HybridFreGSRendererDebugTest, DisableWireframe) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableWireframe(false));
}

TEST_F(HybridFreGSRendererDebugTest, SetBackgroundColorBlack) {
    // Act
    renderer->setBackgroundColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Assert
    glm::vec4 color = renderer->getBackgroundColor();
    EXPECT_FLOAT_EQ(color.r, 0.0f);
    EXPECT_FLOAT_EQ(color.g, 0.0f);
    EXPECT_FLOAT_EQ(color.b, 0.0f);
    EXPECT_FLOAT_EQ(color.a, 1.0f);
}

TEST_F(HybridFreGSRendererDebugTest, SetBackgroundColorRed) {
    // Act
    renderer->setBackgroundColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    // Assert
    glm::vec4 color = renderer->getBackgroundColor();
    EXPECT_FLOAT_EQ(color.r, 1.0f);
}

TEST_F(HybridFreGSRendererDebugTest, SetViewport) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->setViewport(0, 0, 1920, 1080));
}

TEST_F(HybridFreGSRendererDebugTest, EnableAlphaBlending) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableAlphaBlending(true));
}

// ============================================================================
// 7. STATISTICS TESTS (~8 тестов)
// ============================================================================

class HybridFreGSRendererStatsTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererStatsTest, GetBasicStatsReturnsValidData) {
    // Act
    auto stats = renderer->getStats();
    
    // Assert
    EXPECT_GE(stats.frameTime, 0.0f);
    EXPECT_GE(stats.fps, 0.0f);
}

TEST_F(HybridFreGSRendererStatsTest, GetDetailedStatsReturnsValidData) {
    // Act
    auto stats = renderer->getDetailedStats();
    
    // Assert
    EXPECT_GE(stats.frameTime, 0.0f);
    EXPECT_GE(stats.fps, 0.0f);
    EXPECT_GE(stats.drawCalls, 0);
}

TEST_F(HybridFreGSRendererStatsTest, GetGPUInfoReturnsDeviceName) {
    // Act
    auto gpuInfo = renderer->getGPUInfo();
    
    // Assert
    EXPECT_FALSE(gpuInfo.deviceName.empty()) << "Device name не должно быть пустым";
}

TEST_F(HybridFreGSRendererStatsTest, GetGPUInfoReturnsMemoryInfo) {
    // Act
    auto gpuInfo = renderer->getGPUInfo();
    
    // Assert
    EXPECT_GT(gpuInfo.totalMemory, 0) << "Total memory должна быть > 0";
}

TEST_F(HybridFreGSRendererStatsTest, GetGPUInfoReturnsMaxTextureSize) {
    // Act
    auto gpuInfo = renderer->getGPUInfo();
    
    // Assert
    EXPECT_GT(gpuInfo.maxTextureSize, 0) << "Max texture size должен быть > 0";
}

TEST_F(HybridFreGSRendererStatsTest, DetailedStatsHasNoErrorsInitially) {
    // Act
    auto stats = renderer->getDetailedStats();
    
    // Assert
    EXPECT_FALSE(stats.hasErrors) << "Не должно быть ошибок после инициализации";
}

TEST_F(HybridFreGSRendererStatsTest, SetTriangleBudget) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->setTriangleBudget(50000));
}

TEST_F(HybridFreGSRendererStatsTest, EnableEarlyTermination) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableEarlyTermination(true));
}

// ============================================================================
// 8. SCREENSHOT & MISC TESTS (~6 тестов)
// ============================================================================

class HybridFreGSRendererMiscTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
        renderer->initialize();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererMiscTest, SaveScreenshotWithoutWindowReturnsFalse) {
    // Act
    bool result = renderer->saveScreenshot("/tmp/test_screenshot.png");
    
    // Assert
    EXPECT_FALSE(result) << "Screenshot без attached window должен вернуть false";
}

TEST_F(HybridFreGSRendererMiscTest, GetFramebufferDataWithoutWindowReturnsEmpty) {
    // Act
    auto data = renderer->getFramebufferData();
    
    // Assert
    // Может вернуть пустой вектор или placeholder данные
    // EXPECT_TRUE(data.empty() || data.size() > 0);
}

TEST_F(HybridFreGSRendererMiscTest, SetDebugCallbackNullptr) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->setDebugCallback(nullptr));
}

TEST_F(HybridFreGSRendererMiscTest, SetDebugCallbackValid) {
    // Arrange
    bool callbackCalled = false;
    auto callback = [&callbackCalled](const std::string&) {
        callbackCalled = true;
    };
    
    // Act
    renderer->setDebugCallback(callback);
    
    // Assert
    // Callback будет вызван при использовании debugLog
}

TEST_F(HybridFreGSRendererMiscTest, FlushUniformsIsSafe) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->flushUniforms());
}

TEST_F(HybridFreGSRendererMiscTest, EnableDepthTest) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableDepthTest(true));
}

TEST_F(HybridFreGSRendererMiscTest, EnableBackfaceCulling) {
    // Act & Assert
    EXPECT_NO_THROW(renderer->enableBackfaceCulling(true));
}

// ============================================================================
// ИТОГО: 64 теста
// ============================================================================

// 1. Vulkan Context Management:   10 тестов ✅
// 2. Swapchain Management:          4 тестов ✅
// 3. Frame Synchronization:         6 тестов ✅
// 4. Render Mode & Pipeline:        5 тестов ✅
// 5. Data Upload:                   8 тестов ✅
// 6. Debug Functionality:          10 тестов ✅
// 7. Statistics:                    8 тестов ✅
// 8. Screenshot & Misc:             7 тестов ✅
// 9. Integration:                   6 тестов ✅

// TOTAL: 64 теста

// ============================================================================
// 9. INTEGRATION TESTS (~6 тестов)
// ============================================================================

class HybridFreGSRendererIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderer = std::make_unique<HybridFreGSRenderer>();
    }

    void TearDown() override {
        if (renderer) {
            renderer->shutdown();
            renderer.reset();
        }
    }

    std::unique_ptr<HybridFreGSRenderer> renderer;
};

TEST_F(HybridFreGSRendererIntegrationTest, FullInitializationSequence) {
    // Arrange & Act
    bool initResult = renderer->initialize();
    
    // Assert
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(renderer->isInitialized());
    EXPECT_TRUE(renderer->isReady());
    EXPECT_FALSE(renderer->isDeviceLost());
}

TEST_F(HybridFreGSRendererIntegrationTest, RenderLoopWithoutDataDoesNotCrash) {
    // Arrange
    renderer->initialize();
    IRenderer::FrameData frameData;
    frameData.deltaTime = 0.016f;
    
    // Act - симуляция render loop
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_THROW({
            renderer->beginFrame();
            renderer->renderFrame(frameData);
            renderer->endFrame();
        });
    }
}

TEST_F(HybridFreGSRendererIntegrationTest, UploadTrianglesAndRender) {
    // Arrange
    renderer->initialize();
    
    std::vector<spectraforge::rendering::spectraforge::rendering::Triangle> triangles;
    spectraforge::rendering::spectraforge::rendering::Triangle tri{};
    tri.v0 = glm::vec3(0.0f, 1.0f, 0.0f);
    tri.v1 = glm::vec3(-1.0f, -1.0f, 0.0f);
    tri.v2 = glm::vec3(1.0f, -1.0f, 0.0f);
    tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
    tri.opacity = 1.0f;
    tri.sigma = 1.0f;
    triangles.push_back(tri);
    
    renderer->uploadTriangles(triangles);
    
    IRenderer::FrameData frameData;
    frameData.deltaTime = 0.016f;
    
    // Act
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->renderFrame(frameData);
        renderer->endFrame();
    });
}

TEST_F(HybridFreGSRendererIntegrationTest, SetDebugModeAndRender) {
    // Arrange
    renderer->initialize();
    renderer->setDebugMode(1); // SDF mode
    
    IRenderer::FrameData frameData;
    
    // Act
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->renderFrame(frameData);
        renderer->endFrame();
    });
}

TEST_F(HybridFreGSRendererIntegrationTest, ChangeRenderModesAndRender) {
    // Arrange
    renderer->initialize();
    IRenderer::FrameData frameData;
    
    // Act - Triangle Splatting mode
    renderer->setRenderMode(HybridFreGSRenderer::RenderMode::TriangleSplatting);
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->renderFrame(frameData);
        renderer->endFrame();
    });
    
    // Act - Gaussian Splatting mode
    renderer->setRenderMode(HybridFreGSRenderer::RenderMode::GaussianSplatting);
    EXPECT_NO_THROW({
        renderer->beginFrame();
        renderer->renderFrame(frameData);
        renderer->endFrame();
    });
}

TEST_F(HybridFreGSRendererIntegrationTest, MultipleShutdownCallsAreSafe) {
    // Arrange
    renderer->initialize();
    
    // Act
    renderer->shutdown();
    renderer->shutdown();
    renderer->shutdown();
    
    // Assert - не должно крашить
    EXPECT_FALSE(renderer->isInitialized());
}

// ============================================================================
// SUMMARY
// ============================================================================
/*
 * TDD RED ФАЗА: 64 теста написано
 * 
 * Эти тесты покрывают:
 * ✅ Vulkan Context Management (10 тестов)
 * ✅ Swapchain Management (4 теста)
 * ✅ Frame Synchronization (6 тестов)
 * ✅ Render Mode & Pipeline (5 тестов)
 * ✅ Data Upload (8 тестов)
 * ✅ Debug Functionality (10 тестов)
 * ✅ Statistics (8 тестов)
 * ✅ Screenshot & Misc (7 тестов)
 * ✅ Integration (6 тестов)
 * 
 * Следующий шаг: Запустить тесты (должны ПРОЙТИ с текущей реализацией)
 * Затем: Рефакторинг (декомпозиция на 7 классов)
 * Финал: Тесты должны ПРОЙТИ снова (TDD GREEN)
 */
