#pragma once

#include "Interfaces.h"
#include <string>

namespace Engine4D {
namespace Core {

// Forward declaration
class GameObject4D;

/**
 * @brief Base component class following SRP
 */
class Component4D : public ILifecycle {
public:
    Component4D();
    virtual ~Component4D() = default;

    // ILifecycle implementation
    void start() override {}
    void cleanup() override {}

    // Component management
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
    
    // GameObject association
    void setGameObject(GameObject4D* gameObject);
    GameObject4D* getGameObject() const { return gameObject; }
    
    // Component identification
    virtual std::string getComponentType() const = 0;

protected:
    GameObject4D* gameObject;
    bool enabled;
};

/**
 * @brief Component for objects that need updates
 */
class UpdatableComponent : public Component4D, public IUpdatable {
public:
    virtual ~UpdatableComponent() = default;
    std::string getComponentType() const override { return "UpdatableComponent"; }
};

/**
 * @brief Component for objects that can be rendered
 */
class RenderableComponent : public Component4D, public IRenderable {
public:
    virtual ~RenderableComponent() = default;
    std::string getComponentType() const override { return "RenderableComponent"; }
};

/**
 * @brief Component for objects that need both update and render
 */
class UpdatableRenderableComponent : public Component4D, public IUpdatable, public IRenderable {
public:
    virtual ~UpdatableRenderableComponent() = default;
    std::string getComponentType() const override { return "UpdatableRenderableComponent"; }
};

} // namespace Core
} // namespace Engine4D
