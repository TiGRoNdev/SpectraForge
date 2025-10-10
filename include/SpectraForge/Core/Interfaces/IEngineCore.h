#pragma once

namespace SpectraForge::Core {

class IEngineCore {
  public:
    virtual ~IEngineCore() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

}  // namespace SpectraForge::Core
