/**
 * @file RendererDebugger.cpp
 * @brief Implementation of RendererDebugger (P0.2)
 */

#include "SpectraForge/Rendering/Core/RendererDebugger.h"
#include <iostream>

namespace SpectraForge {
namespace Rendering {
namespace Core {

void RendererDebugger::setViewport(int x, int y, int width, int height) {
    currentViewport_.x = static_cast<float>(x);
    currentViewport_.y = static_cast<float>(y);
    currentViewport_.width = static_cast<float>(width);
    currentViewport_.height = static_cast<float>(height);
    currentViewport_.minDepth = 0.0f;
    currentViewport_.maxDepth = 1.0f;
}

void RendererDebugger::log(const std::string& message) const {
    if (debugCallback_) {
        debugCallback_(message);
    }
    std::cout << "[RendererDebugger] " << message << std::endl;
}

}  // namespace Core
}  // namespace Rendering
}  // namespace SpectraForge

