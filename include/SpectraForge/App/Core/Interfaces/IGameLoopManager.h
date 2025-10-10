#pragma once

#include <cstdint>

namespace SpectraForge::App::Core {

class IGameLoopManager {
  public:
    virtual ~IGameLoopManager() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual float getDeltaTime() const = 0;
    virtual float getFPS() const = 0;
    virtual float getFrameTime() const = 0;
    virtual void setTargetFPS(float targetFPS) = 0;
    virtual float getTargetFPS() const = 0;
    virtual float getTotalTime() const = 0;
    virtual void reset() = 0;
};

}  // namespace SpectraForge::App::Core
