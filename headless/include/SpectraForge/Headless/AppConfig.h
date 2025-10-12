#pragma once

#include <array>
#include <string>

namespace SpectraForge::Headless {

struct AppConfig {
    int width = 1280;
    int height = 720;
    std::string title = "SpectraForge Headless";
    bool vsync = true;
    std::array<float, 4> backgroundColor{0.0f, 0.0f, 0.0f, 1.0f};
};

}  // namespace SpectraForge::Headless
