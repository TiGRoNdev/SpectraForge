#pragma once

#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"
#include "Component.h"
#include "Interfaces.h"

namespace HyperEngine {
namespace Core {

/**
 * @brief Компонент трансформации для 3D объектов, следующий принципу единственной ответственности
 */
class Transform3D : public UpdatableComponent, public ITransformable {
  public:
    Transform3D();
    ~Transform3D() override;

    // Интерфейс Component
    std::string getComponentType() const override { return "Transform3D"; }
    void update(float deltaTime) override;
    void cleanup() override;

    // Интерфейс ITransformable
    Math::Vector3 getPosition() const override { return position; }
    void setPosition(const Math::Vector3& pos) override;
    Math::Matrix4 getTransformMatrix() const override { return getWorldMatrix(); }

    // Операции локальной трансформации
    Math::Matrix4 getLocalMatrix() const;
    Math::Matrix4 getWorldMatrix() const;

    // Локальные геттеры
    const Math::Quaternion& getRotation() const { return rotation; }
    const Math::Vector3& getScale() const { return scale; }

    // Локальные сеттеры
    void setRotation(const Math::Quaternion& rot);
    void setScale(const Math::Vector3& scl);

    // Операции трансформации
    void translate(const Math::Vector3& translation);
    void rotate(const Math::Quaternion& rotation);
    void scaleBy(const Math::Vector3& scl);

    // Геттеры мировых координат
    Math::Vector3 getWorldPosition() const;
    Math::Quaternion getWorldRotation() const;
    Math::Vector3 getWorldScale() const;

    // Векторы направления
    Math::Vector3 forward() const;
    Math::Vector3 right() const;
    Math::Vector3 up() const;

    // Управление иерархией
    void setParent(Transform3D* parent);
    Transform3D* getParent() const { return parent; }
    void addChild(Transform3D* child);
    void removeChild(Transform3D* child);
    const std::vector<Transform3D*>& getChildren() const { return children; }

  private:
    // Данные трансформации
    Math::Vector3 position;
    Math::Quaternion rotation;
    Math::Vector3 scale;

    // Иерархия
    Transform3D* parent;
    std::vector<Transform3D*> children;

    // Кэш для производительности
    mutable Math::Matrix4 cachedWorldMatrix;
    mutable bool worldMatrixDirty;

    // Вспомогательные методы
    void markWorldMatrixDirty();
    void updateWorldMatrix() const;
};

}  // namespace Core
}  // namespace HyperEngine
