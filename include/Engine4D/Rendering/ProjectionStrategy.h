#pragma once

#include "../Core/Interfaces.h"
#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"

namespace Engine4D {
namespace Rendering {

/**
 * @brief Orthographic projection strategy
 */
class OrthographicProjection : public Core::IProjectionStrategy {
public:
    OrthographicProjection() = default;
    virtual ~OrthographicProjection() = default;

    Math::Matrix4 getProjectionMatrix() const override;
    Math::Vector4 project(const Math::Vector4& point) const override;
};

/**
 * @brief Perspective projection strategy
 */
class PerspectiveProjection : public Core::IProjectionStrategy {
public:
    explicit PerspectiveProjection(float distance = 10.0f);
    virtual ~PerspectiveProjection() = default;

    Math::Matrix4 getProjectionMatrix() const override;
    Math::Vector4 project(const Math::Vector4& point) const override;

    void setDistance(float distance) { this->distance = distance; }
    float getDistance() const { return distance; }

private:
    float distance;
};

/**
 * @brief Cross-section projection strategy
 */
class CrossSectionProjection : public Core::IProjectionStrategy {
public:
    explicit CrossSectionProjection(float wValue = 0.0f);
    virtual ~CrossSectionProjection() = default;

    Math::Matrix4 getProjectionMatrix() const override;
    Math::Vector4 project(const Math::Vector4& point) const override;

    void setWValue(float wValue) { this->wValue = wValue; }
    float getWValue() const { return wValue; }

private:
    float wValue;
};

} // namespace Rendering
} // namespace Engine4D
