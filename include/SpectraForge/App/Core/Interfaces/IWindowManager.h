#pragma once

#include <cstdint>
#include <string>

namespace SpectraForge {
namespace Core {
class Window;
}  // namespace Core
}  // namespace SpectraForge

namespace SpectraForge::App::Core {

class IWindowManager {
  public:
    virtual ~IWindowManager() = default;

    virtual bool initializeSystem() = 0;
    virtual bool createWindow(const std::string& title, uint32_t width, uint32_t height) = 0;
    virtual SpectraForge::Core::Window* getWindow() const = 0;
    virtual bool shouldClose() const = 0;
    virtual void pollEvents() = 0;
    virtual void swapBuffers() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
};

}  // namespace SpectraForge::App::Core
