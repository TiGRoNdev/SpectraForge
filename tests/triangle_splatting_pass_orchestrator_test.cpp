/**
 * @file triangle_splatting_pass_orchestrator_test.cpp
 * @brief Unit tests для TriangleSplattingPass orchestrator с подменой подсистем.
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingCore.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/FrustumCullingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/DepthSortingPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleRasterizationPass.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingDebugger.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingStatistics.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingTypes.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace spectraforge::rendering;

namespace {

class StubTriangleSplattingCore : public TriangleSplattingCore {
public:
    bool initialize(const VulkanContext&, const Config&) override {
        initializeCalled = true;
        return true;
    }

    void shutdown() override { shutdownCalled = true; }

    bool isInitialized() const override { return initializeCalled && !shutdownCalled; }

    vk::Image getOutputImage() const override { return fakeImage_; }
    vk::ImageView getOutputImageView() const override { return fakeImageView_; }

    bool initializeCalled = false;
    bool shutdownCalled = false;
    vk::Image fakeImage_{};
    vk::ImageView fakeImageView_{};
};

class StubTriangleBufferManager : public TriangleBufferManager {
public:
    bool initialize(vk::Device, VmaAllocator, uint32_t maxTriangles) override {
        initialized = true;
        maxTriangles_ = maxTriangles;
        return true;
    }

    void cleanup() override {
        cleanupCalled = true;
        initialized = false;
        triangleCount = 0;
    }

    void uploadTriangles(const std::vector<Triangle>& triangles,
                         vk::CommandBuffer,
                         vk::Queue) override {
        lastUploadCount = static_cast<uint32_t>(triangles.size());
        triangleCount = lastUploadCount;
    }

    uint32_t getTriangleCount() const override { return triangleCount; }
    uint32_t getMaxTriangles() const override { return maxTriangles_; }
    bool isInitialized() const override { return initialized; }

    bool initialized = false;
    bool cleanupCalled = false;
    uint32_t lastUploadCount = 0;
    uint32_t triangleCount = 0;
    uint32_t maxTriangles_ = 0;
};

class StubFrustumCullingPass : public FrustumCullingPass {
public:
    bool initialize(vk::Device, VmaAllocator, const TriangleBufferManager&, vk::CommandPool, vk::Queue) override {
        initializeCalled = true;
        return true;
    }

    void cleanup() override { cleanupCalled = true; }

    void execute(vk::CommandBuffer, const glm::mat4&, uint32_t triangleCount) override {
        executeCalled = true;
        visibleCount = triangleCount > 0 ? triangleCount - 1 : 0;
    }

    uint32_t getVisibleCount() const override { return visibleCount; }

    bool initializeCalled = false;
    bool cleanupCalled = false;
    bool executeCalled = false;
    uint32_t visibleCount = 0;
};

class StubDepthSortingPass : public DepthSortingPass {
public:
    bool initialize(vk::Device, VmaAllocator, const TriangleBufferManager&, vk::CommandPool, vk::Queue) override {
        initializeCalled = true;
        return true;
    }

    void cleanup() override { cleanupCalled = true; }

    void execute(vk::CommandBuffer, const glm::vec3& cameraPos, uint32_t visibleTriangleCount) override {
        executeCalled = true;
        lastCameraPos = cameraPos;
        lastTriangleCount = visibleTriangleCount;
    }

    void setSortMode(SortMode mode) override {
        customSortMode = mode;
    }

    SortMode getSortMode() const override { return customSortMode; }

    bool initializeCalled = false;
    bool cleanupCalled = false;
    bool executeCalled = false;
    glm::vec3 lastCameraPos = glm::vec3(0.0f);
    uint32_t lastTriangleCount = 0;
    SortMode customSortMode = SortMode::AtomicBinning;
};

class StubTriangleRasterizationPass : public TriangleRasterizationPass {
public:
    bool initialize(vk::Device, VmaAllocator, const TriangleSplattingCore&, const TriangleBufferManager&, const Config& config) override {
        initializeCalled = true;
        configCopy = config;
        return true;
    }

    void cleanup() override { cleanupCalled = true; }

    void execute(vk::CommandBuffer, const glm::mat4& viewProj, uint32_t triangleCount) override {
        executeCalled = true;
        lastTriangleCount = triangleCount;
        lastViewProj = viewProj;
    }

    void executeTwoPass(vk::CommandBuffer cmd, const glm::mat4& viewProj, uint32_t triangleCount) override {
        execute(cmd, viewProj, triangleCount);
        twoPassUsed = true;
    }

    void setDebugMode(uint32_t mode) override { lastDebugMode = mode; }

    void updateConfig(const Config& config) override { configCopy = config; }

    const Config& getConfig() const override { return configCopy; }

    bool isInitialized() const override { return initializeCalled && !cleanupCalled; }

    bool initializeCalled = false;
    bool cleanupCalled = false;
    bool executeCalled = false;
    bool twoPassUsed = false;
    uint32_t lastTriangleCount = 0;
    glm::mat4 lastViewProj = glm::mat4(1.0f);
    uint32_t lastDebugMode = 0;
    Config configCopy{};
};

class StubTriangleSplattingDebugger : public TriangleSplattingDebugger {
public:
    void setDebugMode(uint32_t mode) override { debugMode = mode; }

    uint32_t debugMode = 0;
};

class StubTriangleSplattingStatistics : public TriangleSplattingStatistics {
public:
    void reset() override {
        resetCalled = true;
        TriangleSplattingStatistics::reset();
    }

    void update(uint32_t totalTriangles, uint32_t visibleTriangles) override {
        lastTotal = totalTriangles;
        lastVisible = visibleTriangles;
        TriangleSplattingStatistics::update(totalTriangles, visibleTriangles);
    }

    bool resetCalled = false;
    uint32_t lastTotal = 0;
    uint32_t lastVisible = 0;
};

class StubSubsystemFactory : public TriangleSplattingPass::SubsystemFactory {
public:
    std::unique_ptr<TriangleSplattingCore> createCore() override {
        auto instance = std::make_unique<StubTriangleSplattingCore>();
        core = instance.get();
        return instance;
    }

    std::unique_ptr<TriangleBufferManager> createBufferManager() override {
        auto instance = std::make_unique<StubTriangleBufferManager>();
        bufferManager = instance.get();
        return instance;
    }

    std::unique_ptr<FrustumCullingPass> createFrustumCullingPass() override {
        auto instance = std::make_unique<StubFrustumCullingPass>();
        culling = instance.get();
        return instance;
    }

    std::unique_ptr<DepthSortingPass> createDepthSortingPass() override {
        auto instance = std::make_unique<StubDepthSortingPass>();
        sorting = instance.get();
        return instance;
    }

    std::unique_ptr<TriangleRasterizationPass> createRasterizationPass() override {
        auto instance = std::make_unique<StubTriangleRasterizationPass>();
        raster = instance.get();
        return instance;
    }

    std::unique_ptr<TriangleSplattingDebugger> createDebugger() override {
        auto instance = std::make_unique<StubTriangleSplattingDebugger>();
        debugger = instance.get();
        return instance;
    }

    std::unique_ptr<TriangleSplattingStatistics> createStatistics() override {
        auto instance = std::make_unique<StubTriangleSplattingStatistics>();
        statistics = instance.get();
        return instance;
    }

    bool requiresValidVulkanContext() const override { return false; }

    StubTriangleSplattingCore* core = nullptr;
    StubTriangleBufferManager* bufferManager = nullptr;
    StubFrustumCullingPass* culling = nullptr;
    StubDepthSortingPass* sorting = nullptr;
    StubTriangleRasterizationPass* raster = nullptr;
    StubTriangleSplattingDebugger* debugger = nullptr;
    StubTriangleSplattingStatistics* statistics = nullptr;
};

} // namespace

class TriangleSplattingPassOrchestratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.outputWidth = 800;
        config_.outputHeight = 600;
        factory_ = std::make_shared<StubSubsystemFactory>();
    }

    TriangleSplattingPass::Config config_{};
    std::shared_ptr<StubSubsystemFactory> factory_{};
};

TEST_F(TriangleSplattingPassOrchestratorTest, InitializeAllSubsystemsThroughFactory) {
    TriangleSplattingPass orchestrator(config_, factory_);

    ASSERT_TRUE(orchestrator.initialize(vk::Device(nullptr), nullptr,
                                        vk::Queue(nullptr), vk::Queue(nullptr),
                                        vk::CommandPool(nullptr)));

    ASSERT_NE(factory_->core, nullptr);
    EXPECT_TRUE(factory_->core->initializeCalled);
    ASSERT_NE(factory_->bufferManager, nullptr);
    EXPECT_TRUE(factory_->bufferManager->initialized);
    ASSERT_NE(factory_->culling, nullptr);
    EXPECT_TRUE(factory_->statistics->resetCalled);
}

TEST_F(TriangleSplattingPassOrchestratorTest, ExecuteCoordinatesAllSubsystems) {
    TriangleSplattingPass orchestrator(config_, factory_);
    ASSERT_TRUE(orchestrator.initialize(vk::Device(nullptr), nullptr,
                                        vk::Queue(nullptr), vk::Queue(nullptr),
                                        vk::CommandPool(nullptr)));

    std::vector<Triangle> triangles(5);
    orchestrator.uploadTriangles(triangles);
    orchestrator.setCameraPosition(glm::vec3(0.0f, 0.0f, 5.0f));

    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    EXPECT_NO_THROW(orchestrator.execute(cmd, 0));

    ASSERT_NE(factory_->bufferManager, nullptr);
    EXPECT_EQ(factory_->bufferManager->triangleCount, 5u);
    ASSERT_NE(factory_->culling, nullptr);
    EXPECT_TRUE(factory_->culling->executeCalled);
    EXPECT_EQ(orchestrator.getVisibleTriangleCount(), factory_->culling->visibleCount);
    ASSERT_NE(factory_->sorting, nullptr);
    EXPECT_TRUE(factory_->sorting->executeCalled);
    EXPECT_EQ(factory_->sorting->lastTriangleCount, factory_->culling->visibleCount);
    ASSERT_NE(factory_->raster, nullptr);
    EXPECT_TRUE(factory_->raster->executeCalled);
    ASSERT_NE(factory_->statistics, nullptr);
    EXPECT_EQ(factory_->statistics->lastTotal, 5u);
    EXPECT_EQ(factory_->statistics->lastVisible, factory_->culling->visibleCount);
}

TEST_F(TriangleSplattingPassOrchestratorTest, UploadTrianglesDelegatesToBufferManager) {
    TriangleSplattingPass orchestrator(config_, factory_);
    ASSERT_TRUE(orchestrator.initialize(vk::Device(nullptr), nullptr,
                                        vk::Queue(nullptr), vk::Queue(nullptr),
                                        vk::CommandPool(nullptr)));

    std::vector<Triangle> triangles(3);
    orchestrator.uploadTriangles(triangles);

    ASSERT_NE(factory_->bufferManager, nullptr);
    EXPECT_EQ(factory_->bufferManager->lastUploadCount, 3u);
    EXPECT_EQ(orchestrator.getTriangleCount(), 3u);
}

TEST_F(TriangleSplattingPassOrchestratorTest, SettersUpdateInternalStateAndDebugger) {
    TriangleSplattingPass orchestrator(config_, factory_);
    ASSERT_TRUE(orchestrator.initialize(vk::Device(nullptr), nullptr,
                                        vk::Queue(nullptr), vk::Queue(nullptr),
                                        vk::CommandPool(nullptr)));

    glm::mat4 viewProj = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    glm::vec3 camera(0.0f, 1.0f, 2.0f);

    EXPECT_NO_THROW(orchestrator.setViewProjection(viewProj));
    EXPECT_NO_THROW(orchestrator.setCameraPosition(camera));
    EXPECT_NO_THROW(orchestrator.setFrustumCullingEnabled(false));

    orchestrator.setDebugMode(42);
    ASSERT_NE(factory_->debugger, nullptr);
    EXPECT_EQ(factory_->debugger->debugMode, 42u);
    ASSERT_NE(factory_->raster, nullptr);
    EXPECT_EQ(factory_->raster->lastDebugMode, 42u);
}

TEST_F(TriangleSplattingPassOrchestratorTest, CleanupResetsInitializationFlag) {
    TriangleSplattingPass orchestrator(config_, factory_);
    ASSERT_TRUE(orchestrator.initialize(vk::Device(nullptr), nullptr,
                                        vk::Queue(nullptr), vk::Queue(nullptr),
                                        vk::CommandPool(nullptr)));

    orchestrator.cleanup();
    EXPECT_FALSE(orchestrator.isInitialized());
}

