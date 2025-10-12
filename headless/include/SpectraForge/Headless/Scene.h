#pragma once

#include <cstddef>
#include <string>

namespace SpectraForge::Headless {

struct Scene {
    std::string name{"Unnamed Scene"};
    std::size_t triangleCount = 0;
    std::string description;
};

}  // namespace SpectraForge::Headless
