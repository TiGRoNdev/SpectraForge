/**
 * @file frustum_culling_pass_test.cpp
 * @brief Unit tests для FrustumCullingPass
 * 
 * TDD RED Phase - Day 1
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/FrustumCullingPass.h>
#include <chrono>

using namespace spectraforge::rendering;

class FrustumCullingPassTest : public ::testing::Test {
protected:
    void SetUp() override {
        device_ = vk::Device(nullptr);
        allocator_ = nullptr;
        
        // Create identity matrix (keeps all triangles visible)
        identityMatrix_ = glm::mat4(1.0f);
        
        // Create matrix that culls everything (project outside frustum)
        cullAllMatrix_ = glm::scale(glm::mat4(1.0f), glm::vec3(0.0f));
    }
    
    vk::Device device_;
    VmaAllocator allocator_;
    glm::mat4 identityMatrix_;
    glm::mat4 cullAllMatrix_;
};

// ============================================================================
// TEST 1: Initialize Success
// ============================================================================
TEST_F(FrustumCullingPassTest, InitializeSuccess) {
    FrustumCullingPass cullingPass;
    
    // Mock buffer manager
    TriangleBufferManager bufferManager;
    
    // ACT
    bool result = cullingPass.initialize(device_, allocator_, bufferManager);
    
    // ASSERT
    EXPECT_TRUE(result);
    EXPECT_TRUE(cullingPass.isInitialized());
}

// ============================================================================
// TEST 2: Execute Culls Offscreen Triangles
// ============================================================================
TEST_F(FrustumCullingPassTest, ExecuteCullsOffscreenTriangles) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ARRANGE - Matrix that culls everything
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    uint32_t triangleCount = 100;
    
    // ACT
    cullingPass.execute(cmd, cullAllMatrix_, triangleCount);
    
    // ASSERT
    EXPECT_EQ(cullingPass.getVisibleCount(), 0u) << "All triangles should be culled";
}

// ============================================================================
// TEST 3: Execute With Identity Matrix Keeps All Triangles
// ============================================================================
TEST_F(FrustumCullingPassTest, ExecuteWithIdentityMatrixKeepsAllTriangles) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ACT
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    uint32_t triangleCount = 100;
    cullingPass.execute(cmd, identityMatrix_, triangleCount);
    
    // ASSERT
    EXPECT_EQ(cullingPass.getVisibleCount(), triangleCount) 
        << "Identity matrix should keep all triangles visible";
}

// ============================================================================
// TEST 4: Get Visible Count Accurate
// ============================================================================
TEST_F(FrustumCullingPassTest, GetVisibleCountAccurate) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    
    // Initially zero
    EXPECT_EQ(cullingPass.getVisibleCount(), 0u);
    
    // After execution
    cullingPass.execute(cmd, identityMatrix_, 50);
    EXPECT_GT(cullingPass.getVisibleCount(), 0u);
}

// ============================================================================
// TEST 5: Atomic Counter Buffer Valid
// ============================================================================
TEST_F(FrustumCullingPassTest, AtomicCounterBufferValid) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ACT
    vk::Buffer atomicBuffer = cullingPass.getAtomicCounterBuffer();
    
    // ASSERT
    EXPECT_NE(atomicBuffer, vk::Buffer(nullptr));
}

// ============================================================================
// TEST 6: Cleanup Frees Resources
// ============================================================================
TEST_F(FrustumCullingPassTest, CleanupFreesResources) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ACT
    cullingPass.cleanup();
    
    // ASSERT
    EXPECT_FALSE(cullingPass.isInitialized());
    EXPECT_EQ(cullingPass.getAtomicCounterBuffer(), vk::Buffer(nullptr));
}

// ============================================================================
// TEST 7: Execute With Zero Triangles Does Not Crash
// ============================================================================
TEST_F(FrustumCullingPassTest, ExecuteWithZeroTrianglesDoesNotCrash) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ACT & ASSERT
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    EXPECT_NO_THROW(cullingPass.execute(cmd, identityMatrix_, 0));
}

// ============================================================================
// TEST 8: Performance Meets 60 FPS Target
// ============================================================================
TEST_F(FrustumCullingPassTest, PerformanceMeets60FPS) {
    FrustumCullingPass cullingPass;
    TriangleBufferManager bufferManager;
    
    ASSERT_TRUE(cullingPass.initialize(device_, allocator_, bufferManager));
    
    // ARRANGE - Large dataset
    uint32_t triangleCount = 100000;
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    
    // ACT - Measure time
    auto start = std::chrono::high_resolution_clock::now();
    cullingPass.execute(cmd, identityMatrix_, triangleCount);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    float frameTimeMs = duration.count() / 1000.0f;
    
    // ASSERT - Should complete within 16.67ms budget (60 FPS)
    EXPECT_LT(frameTimeMs, 16.67f) << "Culling should be fast enough for 60 FPS";
}

