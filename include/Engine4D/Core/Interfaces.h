#pragma once

#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include <memory>

namespace Engine4D {
namespace Core {

/**
 * @brief Interface for components that can be updated
 */
class IUpdatable {
public:
    virtual ~IUpdatable() = default;
    virtual void update(float deltaTime) = 0;
};

/**
 * @brief Interface for components that can be rendered
 */
class IRenderable {
public:
    virtual ~IRenderable() = default;
    virtual void render() = 0;
};

/**
 * @brief Interface for components that have lifecycle
 */
class ILifecycle {
public:
    virtual ~ILifecycle() = default;
    virtual void start() = 0;
    virtual void cleanup() = 0;
};

/**
 * @brief Interface for transformable objects
 */
class ITransformable {
public:
    virtual ~ITransformable() = default;
    virtual Math::Vector4 getPosition() const = 0;
    virtual void setPosition(const Math::Vector4& position) = 0;
    virtual Math::Matrix4 getTransformMatrix() const = 0;
};

/**
 * @brief Interface for drawable objects
 */
class IDrawable {
public:
    virtual ~IDrawable() = default;
    virtual void draw() const = 0;
};

/**
 * @brief Interface for primitive factory
 */
class IPrimitiveFactory {
public:
    virtual ~IPrimitiveFactory() = default;
    virtual std::shared_ptr<class GameObject4D> createPrimitive(const std::string& type) = 0;
};

/**
 * @brief Interface for projection strategy
 */
class IProjectionStrategy {
public:
    virtual ~IProjectionStrategy() = default;
    virtual Math::Matrix4 getProjectionMatrix() const = 0;
    virtual Math::Vector4 project(const Math::Vector4& point) const = 0;
};

} // namespace Core
} // namespace Engine4D
