#include "Engine4D/Rendering/ProjectionStrategy.h"

using namespace Engine4D::Math;

namespace Engine4D {
namespace Rendering {

// OrthographicProjection implementation
Matrix4 OrthographicProjection::getProjectionMatrix() const {
    return Matrix4::orthographicProjection();
}

Vector4 OrthographicProjection::project(const Vector4& point) const {
    return point.projectTo3D();
}

// PerspectiveProjection implementation
PerspectiveProjection::PerspectiveProjection(float distance) : distance(distance) {}

Matrix4 PerspectiveProjection::getProjectionMatrix() const {
    return Matrix4::perspectiveProjection(distance);
}

Vector4 PerspectiveProjection::project(const Vector4& point) const {
    return point.perspectiveProject(distance);
}

// CrossSectionProjection implementation
CrossSectionProjection::CrossSectionProjection(float wValue) : wValue(wValue) {}

Matrix4 CrossSectionProjection::getProjectionMatrix() const {
    return Matrix4::crossSectionProjection(wValue);
}

Vector4 CrossSectionProjection::project(const Vector4& point) const {
    return point.crossSection(wValue);
}

} // namespace Rendering
} // namespace Engine4D
