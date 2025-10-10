/**
 * @file Camera3D.cpp
 * @brief 3D Camera implementation (P0.5 - Recreated after OpenGL removal)
 */

#include "SpectraForge/Rendering/Camera3D.h"
#include <cmath>

namespace SpectraForge {
namespace Rendering {

Camera3D::Camera3D()
    : position(0.0f, 0.0f, 0.0f),
      target(0.0f, 0.0f, -1.0f),
      rotation(),
      fieldOfView(60.0f),
      aspectRatio(16.0f / 9.0f),
      nearPlane(0.1f),
      farPlane(1000.0f),
      perspective(true) {}

void Camera3D::setPosition(const Math::Vector3& pos) {
    position = pos;
}

void Camera3D::setTarget(const Math::Vector3& t) {
    target = t;
}

void Camera3D::setRotation(const Math::Quaternion& rot) {
    rotation = rot;
}

void Camera3D::lookAt(const Math::Vector3& pos, const Math::Vector3& target, const Math::Vector3& up) {
    position = pos;
    this->target = target;
    
    // Create view direction
    Math::Vector3 forward = (target - pos).normalized();
    Math::Vector3 right = Math::Vector3::cross(forward, up).normalized();
    Math::Vector3 newUp = Math::Vector3::cross(right, forward).normalized();
    
    // Create rotation from basis vectors
    // For now, store target direction
    (void)right; // Suppress unused warning
    (void)newUp; // Suppress unused warning
}

Math::Vector3 Camera3D::getForward() const {
    return (target - position).normalized();
}

Math::Vector3 Camera3D::getRight() const {
    Math::Vector3 forward = getForward();
    return Math::Vector3::cross(forward, Math::Vector3::up()).normalized();
}

Math::Vector3 Camera3D::getUp() const {
    Math::Vector3 forward = getForward();
    Math::Vector3 right = getRight();
    return Math::Vector3::cross(right, forward).normalized();
}

void Camera3D::move(const Math::Vector3& direction) {
    position = position + direction;
    target = target + direction;
}

void Camera3D::moveForward(float distance) {
    Math::Vector3 forward = getForward();
    move(forward * distance);
}

void Camera3D::moveRight(float distance) {
    Math::Vector3 right = getRight();
    move(right * distance);
}

void Camera3D::moveUp(float distance) {
    Math::Vector3 up = getUp();
    move(up * distance);
}

void Camera3D::rotate(float pitch, float yaw, float roll) {
    (void)pitch; (void)yaw; (void)roll; // TODO: Implement rotation
}

void Camera3D::rotateAround(const Math::Vector3& point, const Math::Vector3& axis, float angle) {
    (void)point; (void)axis; (void)angle; // TODO: Implement rotation
}

void Camera3D::setPerspective(float fovY, float aspect, float near, float far) {
    fieldOfView = fovY;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
    perspective = true;
}

void Camera3D::setOrthographic(float left, float right, float bottom, float top, float near, float far) {
    (void)left; (void)right; (void)bottom; (void)top;
    nearPlane = near;
    farPlane = far;
    perspective = false;
}

void Camera3D::setFieldOfView(float fovY) {
    fieldOfView = fovY;
}

void Camera3D::setAspectRatio(float aspect) {
    aspectRatio = aspect;
}

void Camera3D::setNearPlane(float near) {
    nearPlane = near;
}

void Camera3D::setFarPlane(float far) {
    farPlane = far;
}

Math::Matrix4 Camera3D::getViewMatrix() const {
    return Math::Matrix4::lookAt(position, target, Math::Vector3::up());
}

Math::Matrix4 Camera3D::getProjectionMatrix() const {
    if (perspective) {
        return Math::Matrix4::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);
    } else {
        // Orthographic not fully implemented yet
        return Math::Matrix4::identity();
    }
}

Math::Matrix4 Camera3D::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

Math::Vector3 Camera3D::screenToWorld(const Math::Vector3& screenPos, const Math::Vector3& screenSize) const {
    (void)screenPos; (void)screenSize;
    return Math::Vector3(); // TODO: Implement
}

Math::Vector3 Camera3D::worldToScreen(const Math::Vector3& worldPos, const Math::Vector3& screenSize) const {
    (void)worldPos; (void)screenSize;
    return Math::Vector3(); // TODO: Implement
}

Camera3D::Ray Camera3D::screenPointToRay(const Math::Vector3& screenPos, const Math::Vector3& screenSize) const {
    (void)screenPos; (void)screenSize;
    return {position, getForward()}; // TODO: Implement properly
}

Camera3D::Frustum Camera3D::getFrustum() const {
    Frustum f;
    // TODO: Calculate frustum planes
    return f;
}

bool Camera3D::isInFrustum(const Math::Vector3& point) const {
    (void)point;
    return true; // TODO: Implement frustum culling
}

bool Camera3D::isInFrustum(const Math::Vector3& center, float radius) const {
    (void)center; (void)radius;
    return true; // TODO: Implement frustum culling
}

void Camera3D::updateViewMatrix() const {
    // TODO: Update view matrix cache if needed
}

void Camera3D::updateProjectionMatrix() const {
    // TODO: Update projection matrix cache if needed
}

} // namespace Rendering
} // namespace SpectraForge

