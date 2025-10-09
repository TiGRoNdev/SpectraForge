/**
 * @file renderer_statistics_test.cpp
 * @brief Unit tests for RendererStatistics (P0.2 - TDD RED)
 */

#include <gtest/gtest.h>
#include "SpectraForge/Rendering/Core/RendererStatistics.h"

using namespace SpectraForge::Rendering::Core;

class RendererStatisticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock physical device (nullptr для TDD RED)
        vk::PhysicalDevice mockDevice{};
        stats = std::make_unique<RendererStatistics>(mockDevice);
    }
    
    std::unique_ptr<RendererStatistics> stats;
};

TEST_F(RendererStatisticsTest, DefaultState) {
    auto renderStats = stats->getStats();
    EXPECT_EQ(0.0f, renderStats.frameTime);
    EXPECT_EQ(0u, renderStats.primitives);
    EXPECT_EQ(100000u, stats->getTriangleBudget());
}

TEST_F(RendererStatisticsTest, UpdateStats) {
    SpectraForge::Rendering::RenderingStats newStats;
    newStats.frameTime = 16.7f;
    newStats.primitives = 50000;
    newStats.fps = 60.0f;
    
    stats->updateStats(newStats);
    
    auto current = stats->getStats();
    EXPECT_FLOAT_EQ(16.7f, current.frameTime);
    EXPECT_EQ(50000u, current.primitives);
    EXPECT_FLOAT_EQ(60.0f, current.fps);
}

TEST_F(RendererStatisticsTest, SetTriangleBudget) {
    stats->setTriangleBudget(200000);
    EXPECT_EQ(200000u, stats->getTriangleBudget());
}

TEST_F(RendererStatisticsTest, GetDetailedStats) {
    auto detailedStats = stats->getDetailedStats();
    EXPECT_GE(detailedStats.visibleTriangles, 0u);
}

TEST_F(RendererStatisticsTest, GetGPUInfo) {
    // RED: Will fail until implemented
    auto gpuInfo = stats->getGPUInfo();
    EXPECT_FALSE(gpuInfo.deviceName.empty());
}

