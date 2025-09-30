
#pragma once

#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"

namespace HyperEngine {
namespace Rendering {

/**
 * @brief Камера для 3D пространства
 */
class Camera3D {
  public:
    Camera3D();
    virtual ~Camera3D() = default;

    // Настройка позиции и ориентации
    void setPosition(const Math::Vector3& pos);
    void setTarget(const Math::Vector3& target);
    void setRotation(const Math::Quaternion& rotation);
    void lookAt(const Math::Vector3& pos,
                const Math::Vector3& target,
                const Math::Vector3& up = Math::Vector3::up());

    // Геттеры позиции и ориентации
    const Math::Vector3& getPosition() const { return position; }
    const Math::Vector3& getTarget() const { return target; }
    const Math::Quaternion& getRotation() const { return rotation; }

    // Направления камеры
    Math::Vector3 getForward() const;
    Math::Vector3 getRight() const;
    Math::Vector3 getUp() const;

    // Навигация
    void move(const Math::Vector3& direction);
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float pitch, float yaw, float roll = 0.0f);
    void rotateAround(const Math::Vector3& point, const Math::Vector3& axis, float angle);

    // Настройки проекции
    void setPerspective(float fovY, float aspect, float near, float far);
    void setOrthographic(float left, float right, float bottom, float top, float near, float far);
    void setFieldOfView(float fovY);
    void setAspectRatio(float aspect);
    void setNearPlane(float near);
    void setFarPlane(float far);

    // Геттеры параметров проекции
    float getFieldOfView() const { return fieldOfView; }
    float getAspectRatio() const { return aspectRatio; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }
    bool isPerspective() const { return perspective; }

    // Получение матриц
    Math::Matrix4 getViewMatrix() const;
    Math::Matrix4 getProjectionMatrix() const;
    Math::Matrix4 getViewProjectionMatrix() const;

    // Преобразования координат
    Math::Vector3 screenToWorld(const Math::Vector3& screenPos,
                                const Math::Vector3& screenSize) const;
    Math::Vector3 worldToScreen(const Math::Vector3& worldPos,
                                const Math::Vector3& screenSize) const;

    // Ray casting из позиции на экране
    struct Ray {
        Math::Vector3 origin;
        Math::Vector3 direction;
    };
    Ray screenPointToRay(const Math::Vector3& screenPos, const Math::Vector3& screenSize) const;

    // Frustum culling
    struct Frustum {
        Math::Vector3 planes[6];  // left, right, bottom, top, near, far
        float distances[6];
    };
    Frustum getFrustum() const;
    bool isInFrustum(const Math::Vector3& point) const;
    bool isInFrustum(const Math::Vector3& center, float radius) const;

    // Утилиты
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;

  private:
    // Позиция и ориентация
    Math::Vector3 position;
    mutable Math::Vector3 target;
    Math::Quaternion rotation;

    // Параметры проекции
    bool perspective;
    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    // Для ортогональной проекции
    float orthoLeft, orthoRight, orthoBottom, orthoTop;

    // Кэшированные матрицы
    mutable Math::Matrix4 viewMatrix;
    mutable Math::Matrix4 projectionMatrix;
    mutable bool viewMatrixDirty;
    mutable bool projectionMatrixDirty;

    void markViewMatrixDirty() { viewMatrixDirty = true; }
    void markProjectionMatrixDirty() { projectionMatrixDirty = true; }
};

}  // namespace Rendering
}  // namespace HyperEngine
