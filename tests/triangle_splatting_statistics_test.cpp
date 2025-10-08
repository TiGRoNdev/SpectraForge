/**
 * @file triangle_splatting_statistics_test.cpp
 * @brief Unit tests для TriangleSplattingStatistics
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingStatistics.h>

using namespace spectraforge::rendering;

TEST(TriangleSplattingStatisticsTest, ResetClearsStats) {
    TriangleSplattingStatistics stats;
    
    stats.update(100, 80);
    stats.reset();
    
    auto result = stats.getStats();
    EXPECT_EQ(result.totalTriangles, 0u);
    EXPECT_EQ(result.visibleTriangles, 0u);
}

TEST(TriangleSplattingStatisticsTest, UpdateCalculatesCullingRatio) {
    TriangleSplattingStatistics stats;
    
    stats.update(100, 50);
    
    auto result = stats.getStats();
    EXPECT_EQ(result.totalTriangles, 100u);
    EXPECT_EQ(result.visibleTriangles, 50u);
    EXPECT_FLOAT_EQ(result.cullingRatio, 0.5f);
}

TEST(TriangleSplattingStatisticsTest, SetTimingsCorrect) {
    TriangleSplattingStatistics stats;
    
    stats.setTimings(1.5f, 2.0f, 10.0f);
    
    auto result = stats.getStats();
    EXPECT_FLOAT_EQ(result.frustumCullingTimeMs, 1.5f);
    EXPECT_FLOAT_EQ(result.depthSortingTimeMs, 2.0f);
    EXPECT_FLOAT_EQ(result.rasterizationTimeMs, 10.0f);
    EXPECT_FLOAT_EQ(result.totalFrameTimeMs, 13.5f);
}

TEST(TriangleSplattingStatisticsTest, SetMemoryUsageCorrect) {
    TriangleSplattingStatistics stats;
    
    uint64_t memoryBytes = 1024 * 1024 * 100; // 100 MB
    stats.setMemoryUsage(memoryBytes);
    
    auto result = stats.getStats();
    EXPECT_EQ(result.memoryUsageBytes, memoryBytes);
}

TEST(TriangleSplattingStatisticsTest, PrintStatsDoesNotCrash) {
    TriangleSplattingStatistics stats;
    
    stats.update(100, 80);
    stats.setTimings(1.0f, 2.0f, 10.0f);
    
    EXPECT_NO_THROW(stats.printStats());
}

