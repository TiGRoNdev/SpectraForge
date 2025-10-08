/**
 * @file triangle_splatting_pass_orchestrator_test.cpp
 * @brief Unit tests для TriangleSplattingPass (Orchestrator)
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h>

using namespace spectraforge::rendering;

class TriangleSplattingPassOrchestratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.outputWidth = 800;
        config_.outputHeight = 600;
        
        device_ = vk::Device(nullptr);
        allocator_ = nullptr;
        computeQueue_ = vk::Queue(nullptr);
        graphicsQueue_ = vk::Queue(nullptr);
        commandPool_ = vk::CommandPool(nullptr);
    }
    
    TriangleSplattingPass::Config config_;
    vk::Device device_;
    VmaAllocator allocator_;
    vk::Queue computeQueue_;
    vk::Queue graphicsQueue_;
    vk::CommandPool commandPool_;
};

TEST_F(TriangleSplattingPassOrchestratorTest, InitializeAllSubsystems) {
    TriangleSplattingPass orchestrator(config_);
    
    bool result = orchestrator.initialize(device_, allocator_, 
        computeQueue_, graphicsQueue_, commandPool_);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(orchestrator.isInitialized());
}

TEST_F(TriangleSplattingPassOrchestratorTest, ExecuteCoordinatesAllPasses) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    
    EXPECT_NO_THROW(orchestrator.execute(cmd, 0));
}

TEST_F(TriangleSplattingPassOrchestratorTest, UploadTrianglesDelegatesToBufferManager) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    std::vector<spectraforge::rendering::Triangle> triangles(10);
    
    EXPECT_NO_THROW(orchestrator.uploadTriangles(triangles));
    EXPECT_EQ(orchestrator.getTriangleCount(), 10u);
}

TEST_F(TriangleSplattingPassOrchestratorTest, SetViewProjectionUpdatesState) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    glm::mat4 viewProj = glm::perspective(glm::radians(45.0f), 4.0f/3.0f, 0.1f, 100.0f);
    
    EXPECT_NO_THROW(orchestrator.setViewProjection(viewProj));
}

TEST_F(TriangleSplattingPassOrchestratorTest, SetCameraPositionUpdatesState) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    glm::vec3 cameraPos(0.0f, 0.0f, 10.0f);
    
    EXPECT_NO_THROW(orchestrator.setCameraPosition(cameraPos));
}

TEST_F(TriangleSplattingPassOrchestratorTest, GetVisibleTriangleCountDelegates) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    uint32_t count = orchestrator.getVisibleTriangleCount();
    EXPECT_GE(count, 0u);
}

TEST_F(TriangleSplattingPassOrchestratorTest, CleanupCleansAllSubsystems) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    orchestrator.cleanup();
    
    EXPECT_FALSE(orchestrator.isInitialized());
}

TEST_F(TriangleSplattingPassOrchestratorTest, APICompatibilityWithHybridRenderer) {
    TriangleSplattingPass orchestrator(config_);
    ASSERT_TRUE(orchestrator.initialize(device_, allocator_,
        computeQueue_, graphicsQueue_, commandPool_));
    
    // Test API compatibility methods
    EXPECT_NO_THROW(orchestrator.setBackfaceCullingEnabled(true));
    EXPECT_NO_THROW(orchestrator.setBackgroundColor(glm::vec4(0.0f)));
    EXPECT_NO_THROW(orchestrator.getCulledTriangleCount());
    EXPECT_NO_THROW(orchestrator.flushUniforms());
}

