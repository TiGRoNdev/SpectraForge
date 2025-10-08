/**
 * @file depth_sorting_pass_test.cpp
 * @brief Unit tests для DepthSortingPass
 * 
 * TDD RED Phase - Day 1
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/DepthSortingPass.h>

using namespace spectraforge::rendering;

class DepthSortingPassTest : public ::testing::Test {
protected:
    void SetUp() override {
        device_ = vk::Device(nullptr);
        allocator_ = nullptr;
        cameraPos_ = glm::vec3(0.0f, 0.0f, 5.0f);
    }
    
    vk::Device device_;
    VmaAllocator allocator_;
    glm::vec3 cameraPos_;
};

TEST_F(DepthSortingPassTest, InitializeSuccess) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    
    bool result = sortingPass.initialize(device_, allocator_, bufferManager);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(sortingPass.isInitialized());
}

TEST_F(DepthSortingPassTest, BitonicSortCorrectOrder) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    sortingPass.setSortMode(DepthSortingPass::SortMode::BitonicSort);
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    sortingPass.execute(cmd, cameraPos_, 100);
    
    // After sorting, triangles should be ordered back-to-front
    EXPECT_TRUE(sortingPass.isSorted());
}

TEST_F(DepthSortingPassTest, AtomicBinningSortCorrectOrder) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    sortingPass.setSortMode(DepthSortingPass::SortMode::AtomicBinning);
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    sortingPass.execute(cmd, cameraPos_, 100);
    
    EXPECT_TRUE(sortingPass.isSorted());
}

TEST_F(DepthSortingPassTest, SetSortModeSwitchesAlgorithm) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    // Switch modes
    sortingPass.setSortMode(DepthSortingPass::SortMode::BitonicSort);
    EXPECT_EQ(sortingPass.getSortMode(), DepthSortingPass::SortMode::BitonicSort);
    
    sortingPass.setSortMode(DepthSortingPass::SortMode::AtomicBinning);
    EXPECT_EQ(sortingPass.getSortMode(), DepthSortingPass::SortMode::AtomicBinning);
}

TEST_F(DepthSortingPassTest, ComputeDepthKeysAccurate) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    sortingPass.execute(cmd, cameraPos_, 50);
    
    // Depth keys should be computed for all triangles
    EXPECT_GT(sortingPass.getDepthKeyCount(), 0u);
}

TEST_F(DepthSortingPassTest, SortLargeDataset) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    uint32_t largeCount = 50000;
    
    auto start = std::chrono::high_resolution_clock::now();
    sortingPass.execute(cmd, cameraPos_, largeCount);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 50) << "Sorting should be fast";
}

TEST_F(DepthSortingPassTest, AtomicSortFasterThanBitonic) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    uint32_t triangleCount = 10000;
    
    // Bitonic sort
    sortingPass.setSortMode(DepthSortingPass::SortMode::BitonicSort);
    auto startBitonic = std::chrono::high_resolution_clock::now();
    sortingPass.execute(cmd, cameraPos_, triangleCount);
    auto endBitonic = std::chrono::high_resolution_clock::now();
    auto bitonicTime = std::chrono::duration_cast<std::chrono::microseconds>(endBitonic - startBitonic);
    
    // Atomic binning sort
    sortingPass.setSortMode(DepthSortingPass::SortMode::AtomicBinning);
    auto startAtomic = std::chrono::high_resolution_clock::now();
    sortingPass.execute(cmd, cameraPos_, triangleCount);
    auto endAtomic = std::chrono::high_resolution_clock::now();
    auto atomicTime = std::chrono::duration_cast<std::chrono::microseconds>(endAtomic - startAtomic);
    
    // Atomic should be faster (O(N) vs O(N log N))
    EXPECT_LT(atomicTime.count(), bitonicTime.count()) 
        << "Atomic binning should be faster";
}

TEST_F(DepthSortingPassTest, CleanupFreesResources) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    sortingPass.cleanup();
    
    EXPECT_FALSE(sortingPass.isInitialized());
}

TEST_F(DepthSortingPassTest, ExecuteWithZeroTrianglesDoesNotCrash) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    EXPECT_NO_THROW(sortingPass.execute(cmd, cameraPos_, 0));
}

TEST_F(DepthSortingPassTest, SortStability) {
    DepthSortingPass sortingPass;
    TriangleBufferManager bufferManager;
    ASSERT_TRUE(sortingPass.initialize(device_, allocator_, bufferManager));
    
    vk::CommandBuffer cmd = vk::CommandBuffer(nullptr);
    
    // Sort twice with same camera position
    sortingPass.execute(cmd, cameraPos_, 100);
    auto order1 = sortingPass.getSortedIndices();
    
    sortingPass.execute(cmd, cameraPos_, 100);
    auto order2 = sortingPass.getSortedIndices();
    
    // Order should be the same
    EXPECT_EQ(order1.size(), order2.size());
    for (size_t i = 0; i < order1.size(); ++i) {
        EXPECT_EQ(order1[i], order2[i]) << "Sort should be stable";
    }
}

