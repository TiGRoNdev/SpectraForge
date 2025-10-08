/**
 * @file triangle_buffer_manager_test.cpp
 * @brief Unit tests для TriangleBufferManager
 * 
 * TDD RED Phase - Day 1
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleBufferManager.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplattingPass.h>

using namespace spectraforge::rendering;

class TriangleBufferManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock Vulkan context
        device_ = vk::Device(nullptr);  // Mock device
        allocator_ = nullptr;           // Mock allocator
        maxTriangles_ = 10000;
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    vk::Device device_;
    VmaAllocator allocator_;
    uint32_t maxTriangles_;
};

// ============================================================================
// TEST 1: Initialize Success
// ============================================================================
TEST_F(TriangleBufferManagerTest, InitializeSuccess) {
    TriangleBufferManager manager;
    
    // ACT
    bool result = manager.initialize(device_, allocator_, maxTriangles_);
    
    // ASSERT
    EXPECT_TRUE(result);
    EXPECT_TRUE(manager.isInitialized());
}

// ============================================================================
// TEST 2: Initialize With Invalid Allocator Fails
// ============================================================================
TEST_F(TriangleBufferManagerTest, InitializeWithInvalidAllocatorFails) {
    TriangleBufferManager manager;
    
    // ACT
    bool result = manager.initialize(device_, nullptr, maxTriangles_);
    
    // ASSERT
    EXPECT_FALSE(result) << "Should fail with null allocator";
}

// ============================================================================
// TEST 3: Upload Triangles Success
// ============================================================================
TEST_F(TriangleBufferManagerTest, UploadTrianglesSuccess) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ARRANGE
    std::vector<spectraforge::rendering::Triangle> triangles;
    spectraforge::rendering::Triangle tri;
    tri.v0 = glm::vec3(0.0f, 1.0f, 0.0f);
    tri.v1 = glm::vec3(-1.0f, -1.0f, 0.0f);
    tri.v2 = glm::vec3(1.0f, -1.0f, 0.0f);
    tri.color = glm::vec3(1.0f, 0.0f, 0.0f);
    tri.opacity = 1.0f;
    triangles.push_back(tri);
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    
    // ACT
    manager.uploadTriangles(triangles, cmd, queue);
    
    // ASSERT
    EXPECT_EQ(manager.getTriangleCount(), 1u);
}

// ============================================================================
// TEST 4: Upload Empty Triangles Does Not Crash
// ============================================================================
TEST_F(TriangleBufferManagerTest, UploadEmptyTrianglesDoesNotCrash) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ACT
    std::vector<spectraforge::rendering::Triangle> emptyTriangles;
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    
    EXPECT_NO_THROW(manager.uploadTriangles(emptyTriangles, cmd, queue));
    EXPECT_EQ(manager.getTriangleCount(), 0u);
}

// ============================================================================
// TEST 5: Upload Exceeds Max Triangles Throws
// ============================================================================
TEST_F(TriangleBufferManagerTest, UploadExceedsMaxTrianglesThrows) {
    TriangleBufferManager manager;
    maxTriangles_ = 10;  // Small limit
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ARRANGE - Create 11 triangles (exceeds limit)
    std::vector<spectraforge::rendering::Triangle> triangles(11);
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    
    // ACT & ASSERT
    EXPECT_THROW(manager.uploadTriangles(triangles, cmd, queue), std::runtime_error);
}

// ============================================================================
// TEST 6: Get Triangle Count Returns Correct Value
// ============================================================================
TEST_F(TriangleBufferManagerTest, GetTriangleCountReturnsCorrectValue) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // Initially zero
    EXPECT_EQ(manager.getTriangleCount(), 0u);
    
    // After upload
    std::vector<spectraforge::rendering::Triangle> triangles(5);
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    manager.uploadTriangles(triangles, cmd, queue);
    
    EXPECT_EQ(manager.getTriangleCount(), 5u);
}

// ============================================================================
// TEST 7: Get Buffers Return Valid Handles
// ============================================================================
TEST_F(TriangleBufferManagerTest, GetBuffersReturnValidHandles) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ACT
    vk::Buffer triangleBuffer = manager.getTriangleBuffer();
    vk::Buffer visibleIndicesBuffer = manager.getVisibleIndicesBuffer();
    vk::Buffer sortedIndicesBuffer = manager.getSortedIndicesBuffer();
    vk::Buffer depthKeysBuffer = manager.getDepthKeysBuffer();
    
    // ASSERT
    EXPECT_NE(triangleBuffer, vk::Buffer(nullptr));
    EXPECT_NE(visibleIndicesBuffer, vk::Buffer(nullptr));
    EXPECT_NE(sortedIndicesBuffer, vk::Buffer(nullptr));
    EXPECT_NE(depthKeysBuffer, vk::Buffer(nullptr));
}

// ============================================================================
// TEST 8: Cleanup Frees All Buffers
// ============================================================================
TEST_F(TriangleBufferManagerTest, CleanupFreesAllBuffers) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ACT
    manager.cleanup();
    
    // ASSERT
    EXPECT_FALSE(manager.isInitialized());
    EXPECT_EQ(manager.getTriangleBuffer(), vk::Buffer(nullptr));
}

// ============================================================================
// TEST 9: Multiple Uploads Work
// ============================================================================
TEST_F(TriangleBufferManagerTest, MultipleUploadsWork) {
    TriangleBufferManager manager;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    
    // First upload
    std::vector<spectraforge::rendering::Triangle> triangles1(5);
    manager.uploadTriangles(triangles1, cmd, queue);
    EXPECT_EQ(manager.getTriangleCount(), 5u);
    
    // Second upload (should replace)
    std::vector<spectraforge::rendering::Triangle> triangles2(10);
    manager.uploadTriangles(triangles2, cmd, queue);
    EXPECT_EQ(manager.getTriangleCount(), 10u);
}

// ============================================================================
// TEST 10: Upload Large Dataset Performance
// ============================================================================
TEST_F(TriangleBufferManagerTest, UploadLargeDatasetPerformance) {
    TriangleBufferManager manager;
    maxTriangles_ = 100000;
    ASSERT_TRUE(manager.initialize(device_, allocator_, maxTriangles_));
    
    // ARRANGE - Large dataset
    std::vector<spectraforge::rendering::Triangle> triangles(50000);
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    vk::Queue queue = vk::Queue(nullptr);
    
    // ACT - Measure time
    auto start = std::chrono::high_resolution_clock::now();
    manager.uploadTriangles(triangles, cmd, queue);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // ASSERT - Should complete in reasonable time (<100ms)
    EXPECT_LT(duration.count(), 100) << "Upload should be fast";
    EXPECT_EQ(manager.getTriangleCount(), 50000u);
}

