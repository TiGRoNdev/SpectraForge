#pragma once

#include <cstdint>

namespace SpectraForge::Rendering {

class IWindowBinder {
  public:
    virtual ~IWindowBinder() = default;

    virtual bool attachWindow(void* display, void* window, uint32_t width, uint32_t height) = 0;
};

}  // namespace SpectraForge::Rendering
