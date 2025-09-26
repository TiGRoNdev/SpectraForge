#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include <memory>

namespace Engine3D {
namespace Core {

/**
 * @brief Интерфейс для компонентов, которые можно обновлять
 */
class IUpdatable {
public:
    virtual ~IUpdatable() = default;
    virtual void update(float deltaTime) = 0;
};

/**
 * @brief Интерфейс для компонентов, которые можно рендерить
 */
class IRenderable {
public:
    virtual ~IRenderable() = default;
    virtual void render() = 0;
};

/**
 * @brief Интерфейс для компонентов с жизненным циклом
 */
class ILifecycle {
public:
    virtual ~ILifecycle() = default;
    virtual void start() = 0;
    virtual void cleanup() = 0;
};

/**
 * @brief Интерфейс для трансформируемых объектов
 */
class ITransformable {
public:
    virtual ~ITransformable() = default;
    virtual Math::Vector3 getPosition() const = 0;
    virtual void setPosition(const Math::Vector3& position) = 0;
    virtual Math::Matrix4 getTransformMatrix() const = 0;
};

/**
 * @brief Интерфейс для отрисовываемых объектов
 */
class IDrawable {
public:
    virtual ~IDrawable() = default;
    virtual void draw() const = 0;
};

/**
 * @brief Интерфейс для фабрики примитивов
 */
class IPrimitiveFactory {
public:
    virtual ~IPrimitiveFactory() = default;
    virtual std::shared_ptr<class GameObject3D> createPrimitive(const std::string& type) = 0;
};

/**
 * @brief Интерфейс для стратегии проекции
 */
class IProjectionStrategy {
public:
    virtual ~IProjectionStrategy() = default;
    virtual Math::Matrix4 getProjectionMatrix() const = 0;
    virtual Math::Vector3 project(const Math::Vector3& point) const = 0;
};

} // namespace Core
} // namespace Engine3D
