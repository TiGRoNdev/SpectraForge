#pragma once

#include <cstdint>
#include <string>

namespace SpectraForge::Vulkan {

struct SceneData;
struct Gaussians;
struct MultiViewImages;
struct DynamicElements;

class ISceneManager {
  public:
    virtual ~ISceneManager() = default;

    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool loadScene(const SceneData& data) = 0;
    virtual void updateDynamics(float deltaTime = 0.016f) = 0;
    virtual Gaussians getGaussians() const = 0;
    virtual const MultiViewImages& getMultiViewImages() const = 0;
    virtual const DynamicElements& getDynamicElements() const = 0;
    virtual bool isSceneLoaded() const = 0;
    virtual void clearScene() = 0;
    virtual uint32_t addObject(const std::string& objectPath) = 0;
    virtual void removeObject(uint32_t objectId) = 0;
    virtual uint32_t getObjectCount() const = 0;
};

}  // namespace SpectraForge::Vulkan
