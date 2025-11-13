#include "SpectraForge/Rendering/Camera3D.h"
#include <cmath>

using namespace SpectraForge::Math;
using namespace SpectraForge::Rendering;

namespace SpectraForge {
namespace Rendering {

// Конструктор
Camera3D::Camera3D()
    : position(0.0f, 0.0f, 5.0f),
      target(0.0f, 0.0f, 0.0f),
      rotation(Quaternion::identity()),
      perspective(true),
      fieldOfView(60.0f),
      aspectRatio(16.0f / 9.0f),
      nearPlane(0.1f),
      farPlane(1000.0f),
      orthoLeft(-5.0f),
      orthoRight(5.0f),
      orthoBottom(-5.0f),
      orthoTop(5.0f),
      viewMatrixDirty(true),
      projectionMatrixDirty(true) {}

// Настройка позиции и ориентации
void Camera3D::setPosition(const Vector3& pos) {
    position = pos;
    markViewMatrixDirty();
}

void Camera3D::setTarget(const Vector3& newTarget) {
    target = newTarget;
    markViewMatrixDirty();
}

void Camera3D::setRotation(const Quaternion& newRotation) {
    rotation = newRotation;
    markViewMatrixDirty();
}

void Camera3D::lookAt(const Vector3& pos, const Vector3& newTarget, const Vector3& up) {
    position = pos;
    target = newTarget;

    Vector3 forward = (newTarget - pos).normalized();
    Vector3 right = up.cross(forward).normalized();
    Vector3 actualUp = forward.cross(right);

    rotation = Quaternion::lookRotation(forward, actualUp);
    markViewMatrixDirty();
}

// Направления камеры
Vector3 Camera3D::getForward() const {
    return rotation.rotate(Vector3::forward());
}

Vector3 Camera3D::getRight() const {
    return rotation.rotate(Vector3::right());
}

Vector3 Camera3D::getUp() const {
    return rotation.rotate(Vector3::up());
}

// Навигация
void Camera3D::move(const Vector3& direction) {
    position += direction;
    target += direction;
    markViewMatrixDirty();
}

void Camera3D::moveForward(float distance) {
    Vector3 forward = getForward();
    move(forward * distance);
}

void Camera3D::moveRight(float distance) {
    Vector3 right = getRight();
    move(right * distance);
}

void Camera3D::moveUp(float distance) {
    Vector3 up = getUp();
    move(up * distance);
}

void Camera3D::rotate(float pitch, float yaw, float roll) {
    Quaternion pitchQuat = Quaternion::fromAxisAngle(Vector3::right(), pitch);
    Quaternion yawQuat = Quaternion::fromAxisAngle(Vector3::up(), yaw);
    Quaternion rollQuat = Quaternion::fromAxisAngle(Vector3::forward(), roll);

    rotation = yawQuat * rotation * pitchQuat * rollQuat;
    rotation.normalize();

    markViewMatrixDirty();
}

void Camera3D::rotateAround(const Vector3& point, const Vector3& axis, float angle) {
    Quaternion rotQuat = Quaternion::fromAxisAngle(axis, angle);

    // Перемещаем камеру относительно точки поворота
    Vector3 offset = position - point;
    offset = rotQuat.rotate(offset);
    position = point + offset;

    // Поворачиваем ориентацию камеры
    rotation = rotQuat * rotation;
    rotation.normalize();

    markViewMatrixDirty();
}

// Настройки проекции
void Camera3D::setPerspective(float fovY, float aspect, float near, float far) {
    perspective = true;
    fieldOfView = fovY;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
    markProjectionMatrixDirty();
}

void Camera3D::setOrthographic(float left,
                               float right,
                               float bottom,
                               float top,
                               float near,
                               float far) {
    perspective = false;
    orthoLeft = left;
    orthoRight = right;
    orthoBottom = bottom;
    orthoTop = top;
    nearPlane = near;
    farPlane = far;
    markProjectionMatrixDirty();
}

void Camera3D::setFieldOfView(float fovY) {
    fieldOfView = fovY;
    if (perspective) {
        markProjectionMatrixDirty();
    }
}

void Camera3D::setAspectRatio(float aspect) {
    aspectRatio = aspect;
    if (perspective) {
        markProjectionMatrixDirty();
    }
}

void Camera3D::setNearPlane(float near) {
    nearPlane = near;
    markProjectionMatrixDirty();
}

void Camera3D::setFarPlane(float far) {
    farPlane = far;
    markProjectionMatrixDirty();
}

// Получение матриц

Math::Matrix4 Camera3D::getViewMatrix() const {
    if (viewMatrixDirty) {
        updateViewMatrix();
        viewMatrixDirty = false;
    }
    return viewMatrix;
}

void Camera3D::updateViewMatrix() const {
    // Стандартная LookAt матрица
    // forward = направление ОТ цели К камере (в OpenGL/Vulkan view space Z смотрит на камеру)
    Math::Vector3 zaxis = (position - target).normalized();  // forward (к камере)
    Math::Vector3 xaxis = Math::Vector3::cross(Math::Vector3(0.0f, 1.0f, 0.0f), zaxis).normalized();  // right (Y-up)
    Math::Vector3 yaxis = Math::Vector3::cross(zaxis, xaxis);  // up
    
    // View матрица в row-major формате (базисные векторы в строках)
    viewMatrix = Math::Matrix4(
        xaxis.x,           xaxis.y,           xaxis.z,           -Math::Vector3::dot(xaxis, position),
        yaxis.x,           yaxis.y,           yaxis.z,           -Math::Vector3::dot(yaxis, position),
        zaxis.x,           zaxis.y,           zaxis.z,           -Math::Vector3::dot(zaxis, position),
        0.0f,              0.0f,              0.0f,              1.0f
    );
}

Math::Matrix4 Camera3D::getProjectionMatrix() const {
    if (projectionMatrixDirty) {
        updateProjectionMatrix();
        projectionMatrixDirty = false;
    }
    return projectionMatrix;
}

void Camera3D::updateProjectionMatrix() const {
    if (perspective) {
        // ИСПРАВЛЕНО: Vulkan perspective matrix (праворукая, Z ∈ [0, 1], Y-flip)
        // Эквивалентно glm::perspectiveRH_ZO с GLM_FORCE_DEPTH_ZERO_TO_ONE
        const float tanHalfFov = tan(fieldOfView / 2.0f);

        projectionMatrix = Math::Matrix4::zero();
        projectionMatrix.m[0][0] = 1.0f / (aspectRatio * tanHalfFov);
        projectionMatrix.m[1][1] = -1.0f / tanHalfFov;            // Y-flip для Vulkan (NDC Y вниз)
        projectionMatrix.m[2][2] = farPlane / (nearPlane - farPlane);        // Vulkan Z ∈ [0,1]
        projectionMatrix.m[2][3] = -(farPlane * nearPlane) / (farPlane - nearPlane);
        projectionMatrix.m[3][2] = -1.0f;
    } else {
        // Orthographic projection
        projectionMatrix = Math::Matrix4(
            2.0f / (orthoRight - orthoLeft), 0.0f, 0.0f, -(orthoRight + orthoLeft) / (orthoRight - orthoLeft),
            0.0f, -2.0f / (orthoTop - orthoBottom), 0.0f, (orthoTop + orthoBottom) / (orthoTop - orthoBottom), // Y-flip
            0.0f, 0.0f, 1.0f / (farPlane - nearPlane), -nearPlane / (farPlane - nearPlane),
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }
    
    projectionMatrixDirty = false;
}

Matrix4 Camera3D::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

// Преобразования координат
Vector3 Camera3D::screenToWorld(const Vector3& screenPos, const Vector3& screenSize) const {
    // Нормализуем экранные координаты к [-1, 1]
    float ndcX = (2.0f * screenPos.x) / screenSize.x - 1.0f;
    float ndcY = 1.0f - (2.0f * screenPos.y) / screenSize.y;
    float ndcZ = 2.0f * screenPos.z - 1.0f;

    // Обратное преобразование
    Matrix4 invViewProj = getViewProjectionMatrix().inverse();
    Vector3 worldPos = invViewProj.transformPoint(Vector3(ndcX, ndcY, ndcZ));

    return worldPos;
}

Vector3 Camera3D::worldToScreen(const Vector3& worldPos, const Vector3& screenSize) const {
    Matrix4 viewProj = getViewProjectionMatrix();
    Vector3 clipPos = viewProj.transformPoint(worldPos);

    // Перспективное деление
    if (clipPos.z != 0.0f) {
        clipPos.x /= clipPos.z;
        clipPos.y /= clipPos.z;
    }

    // Преобразование в экранные координаты
    float screenX = (clipPos.x + 1.0f) * 0.5f * screenSize.x;
    float screenY = (1.0f - clipPos.y) * 0.5f * screenSize.y;

    return Vector3(screenX, screenY, clipPos.z);
}

// Ray casting
Camera3D::Ray Camera3D::screenPointToRay(const Vector3& screenPos,
                                         const Vector3& screenSize) const {
    Vector3 nearPoint = screenToWorld(Vector3(screenPos.x, screenPos.y, 0.0f), screenSize);
    Vector3 farPoint = screenToWorld(Vector3(screenPos.x, screenPos.y, 1.0f), screenSize);

    Ray ray;
    ray.origin = nearPoint;
    ray.direction = (farPoint - nearPoint).normalized();

    return ray;
}

// Frustum culling
Camera3D::Frustum Camera3D::getFrustum() const {
    Frustum frustum;

    Matrix4 viewProj = getViewProjectionMatrix();

    // Извлекаем плоскости фрустума из матрицы view-projection
    // Левая плоскость
    frustum.planes[0] = Vector3(viewProj.m[0][3] + viewProj.m[0][0],
                                viewProj.m[1][3] + viewProj.m[1][0],
                                viewProj.m[2][3] + viewProj.m[2][0]);
    frustum.distances[0] = viewProj.m[3][3] + viewProj.m[3][0];

    // Правая плоскость
    frustum.planes[1] = Vector3(viewProj.m[0][3] - viewProj.m[0][0],
                                viewProj.m[1][3] - viewProj.m[1][0],
                                viewProj.m[2][3] - viewProj.m[2][0]);
    frustum.distances[1] = viewProj.m[3][3] - viewProj.m[3][0];

    // Нижняя плоскость
    frustum.planes[2] = Vector3(viewProj.m[0][3] + viewProj.m[0][1],
                                viewProj.m[1][3] + viewProj.m[1][1],
                                viewProj.m[2][3] + viewProj.m[2][1]);
    frustum.distances[2] = viewProj.m[3][3] + viewProj.m[3][1];

    // Верхняя плоскость
    frustum.planes[3] = Vector3(viewProj.m[0][3] - viewProj.m[0][1],
                                viewProj.m[1][3] - viewProj.m[1][1],
                                viewProj.m[2][3] - viewProj.m[2][1]);
    frustum.distances[3] = viewProj.m[3][3] - viewProj.m[3][1];

    // Ближняя плоскость
    frustum.planes[4] = Vector3(viewProj.m[0][2], viewProj.m[1][2], viewProj.m[2][2]);
    frustum.distances[4] = viewProj.m[3][2];

    // Дальняя плоскость
    frustum.planes[5] = Vector3(viewProj.m[0][3] - viewProj.m[0][2],
                                viewProj.m[1][3] - viewProj.m[1][2],
                                viewProj.m[2][3] - viewProj.m[2][2]);
    frustum.distances[5] = viewProj.m[3][3] - viewProj.m[3][2];

    // Нормализуем плоскости
    for (int i = 0; i < 6; ++i) {
        float length = frustum.planes[i].magnitude();
        if (length > 0.0f) {
            frustum.planes[i] /= length;
            frustum.distances[i] /= length;
        }
    }

    return frustum;
}

bool Camera3D::isInFrustum(const Vector3& point) const {
    Frustum frustum = getFrustum();

    for (int i = 0; i < 6; ++i) {
        if (frustum.planes[i].dot(point) + frustum.distances[i] < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Camera3D::isInFrustum(const Vector3& center, float radius) const {
    Frustum frustum = getFrustum();

    for (int i = 0; i < 6; ++i) {
        if (frustum.planes[i].dot(center) + frustum.distances[i] < -radius) {
            return false;
        }
    }

    return true;
}

}  // namespace Rendering
}  // namespace SpectraForge
