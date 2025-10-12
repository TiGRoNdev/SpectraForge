#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace SpectraForge::Headless {

struct FrameReport {
    std::size_t frameIndex = 0;
    std::string sceneName;
    std::size_t trianglesRendered = 0;
    float rotationDegrees = 0.0f;
    int debugMode = 0;
    bool externalCameraControl = false;
    std::array<float, 3> cameraPosition{0.0f, 0.0f, 5.0f};
    std::array<float, 3> cameraTarget{0.0f, 0.0f, 0.0f};
};

}  // namespace SpectraForge::Headless
