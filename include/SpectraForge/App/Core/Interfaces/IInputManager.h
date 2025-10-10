#pragma once

struct GLFWwindow;

namespace SpectraForge::App::Core {

struct InputState;

class IInputManager {
  public:
    virtual ~IInputManager() = default;

    virtual void setupCallbacks(GLFWwindow* window) = 0;
    virtual void update() = 0;
    virtual bool isKeyPressed(int key) const = 0;
    virtual bool isKeyJustPressed(int key) const = 0;
    virtual bool isMouseButtonPressed(int button) const = 0;
    virtual void getMousePosition(float& x, float& y) const = 0;
    virtual void getMouseDelta(float& dx, float& dy) const = 0;
    virtual const InputState* getState() const = 0;
    virtual void reset() = 0;
    virtual void setMouseCaptured(bool captured) = 0;
};

}  // namespace SpectraForge::App::Core
