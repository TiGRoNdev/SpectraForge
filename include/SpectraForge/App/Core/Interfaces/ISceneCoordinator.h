#pragma once

#include <memory>

namespace SpectraForge {
namespace Rendering {
class Camera3D;
}  // namespace Rendering
namespace Vulkan {
struct SceneData;
}  // namespace Vulkan
}  // namespace SpectraForge

namespace SpectraForge::App {
struct SceneInfo;
}

namespace SpectraForge::App::Core {

struct InputState;

class ISceneCoordinator {
  public:
    virtual ~ISceneCoordinator() = default;

    virtual bool loadScene(const SpectraForge::Vulkan::SceneData& data) = 0;
    virtual void updateCamera(float deltaTime, const InputState* input, bool externalControl) = 0;
    virtual std::shared_ptr<SpectraForge::Rendering::Camera3D> getCamera() const = 0;
    virtual void setCamera(std::shared_ptr<SpectraForge::Rendering::Camera3D> camera) = 0;
    virtual void resetCameraForSponza() = 0;
    virtual SpectraForge::App::SceneInfo getSceneInfo() const = 0;
    virtual bool isSceneLoaded() const = 0;
};

}  // namespace SpectraForge::App::Core
