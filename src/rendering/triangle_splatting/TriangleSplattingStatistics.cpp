#include <SpectraForge/Rendering/RenderPass/TriangleSplatting/TriangleSplattingStatistics.h>
#include <SpectraForge/Core/Console.h>
#include <SpectraForge/Core/SafeConsole.h>
#include <sstream>
#include <iomanip>
#include <cmath>

using SpectraForge::Core::Console;

namespace spectraforge {
namespace rendering {

void TriangleSplattingStatistics::reset() {
    stats_ = Stats();
    Console::info("TriangleSplattingStatistics reset");
}

void TriangleSplattingStatistics::update(uint32_t totalTriangles, uint32_t visibleTriangles) {
    stats_.totalTriangles = totalTriangles;
    stats_.visibleTriangles = visibleTriangles;
    
    if (totalTriangles > 0) {
        stats_.culledTriangles = totalTriangles - visibleTriangles;
        stats_.cullingRatio = static_cast<float>(stats_.culledTriangles) / static_cast<float>(totalTriangles);
    } else {
        stats_.culledTriangles = 0;
        stats_.cullingRatio = 0.0f;
    }
}

void TriangleSplattingStatistics::setTimings(float frustumMs, float sortMs, float rasterMs) {
    stats_.frustumCullingTimeMs = frustumMs;
    stats_.depthSortingTimeMs = sortMs;
    stats_.rasterizationTimeMs = rasterMs;
    stats_.totalFrameTimeMs = frustumMs + sortMs + rasterMs;
    
    calculateFPS();
}

void TriangleSplattingStatistics::setMemoryUsage(uint64_t bytes) {
    stats_.memoryUsageBytes = bytes;
}

void TriangleSplattingStatistics::calculateFPS() {
    if (stats_.totalFrameTimeMs > 0.0f) {
        stats_.fps = 1000.0f / stats_.totalFrameTimeMs;
    } else {
        stats_.fps = 0.0f;
    }
}

void TriangleSplattingStatistics::printStats() const {
    Console::info("=== Triangle Splatting Statistics ===");
    Console::info("Triangles:");
    Console::info("  Total:   " + SpectraForge::Core::SAFE_TO_STRING(stats_.totalTriangles));
    Console::info("  Visible: " + SpectraForge::Core::SAFE_TO_STRING(stats_.visibleTriangles));
    Console::info("  Culled:  " + SpectraForge::Core::SAFE_TO_STRING(stats_.culledTriangles) + 
                 " (" + SpectraForge::Core::SAFE_TO_STRING(static_cast<int>(stats_.cullingRatio * 100)) + "%)");
    
    Console::info("Timings:");
    Console::info("  Frustum Culling: " + SpectraForge::Core::SAFE_TO_STRING(stats_.frustumCullingTimeMs) + " ms");
    Console::info("  Depth Sorting:   " + SpectraForge::Core::SAFE_TO_STRING(stats_.depthSortingTimeMs) + " ms");
    Console::info("  Rasterization:   " + SpectraForge::Core::SAFE_TO_STRING(stats_.rasterizationTimeMs) + " ms");
    Console::info("  Total Frame:     " + SpectraForge::Core::SAFE_TO_STRING(stats_.totalFrameTimeMs) + " ms");
    Console::info("  FPS:             " + SpectraForge::Core::SAFE_TO_STRING(static_cast<int>(stats_.fps)));
    
    Console::info("Memory:");
    const float memoryMB = static_cast<float>(stats_.memoryUsageBytes) / (1024.0f * 1024.0f);
    Console::info("  Usage: " + SpectraForge::Core::SAFE_TO_STRING(memoryMB) + " MB");
    
    Console::info("=====================================");
}

std::string TriangleSplattingStatistics::toJSON() const {
    std::ostringstream json;
    json << std::fixed << std::setprecision(2);
    
    json << "{\n";
    json << "  \"triangles\": {\n";
    json << "    \"total\": " << stats_.totalTriangles << ",\n";
    json << "    \"visible\": " << stats_.visibleTriangles << ",\n";
    json << "    \"culled\": " << stats_.culledTriangles << ",\n";
    json << "    \"cullingRatio\": " << stats_.cullingRatio << "\n";
    json << "  },\n";
    
    json << "  \"timings\": {\n";
    json << "    \"frustumCullingMs\": " << stats_.frustumCullingTimeMs << ",\n";
    json << "    \"depthSortingMs\": " << stats_.depthSortingTimeMs << ",\n";
    json << "    \"rasterizationMs\": " << stats_.rasterizationTimeMs << ",\n";
    json << "    \"totalFrameMs\": " << stats_.totalFrameTimeMs << ",\n";
    json << "    \"fps\": " << stats_.fps << "\n";
    json << "  },\n";
    
    json << "  \"memory\": {\n";
    json << "    \"usageBytes\": " << stats_.memoryUsageBytes << ",\n";
    json << "    \"usageMB\": " << (static_cast<float>(stats_.memoryUsageBytes) / (1024.0f * 1024.0f)) << "\n";
    json << "  }\n";
    
    json << "}";
    
    return json.str();
}

} // namespace rendering
} // namespace spectraforge

