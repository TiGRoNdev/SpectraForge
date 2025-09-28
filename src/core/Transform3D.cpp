#include "HyperEngine/Core/Transform3D.h"
#include <algorithm>

using namespace HyperEngine::Math;

namespace HyperEngine {
namespace Core {

Transform3D::Transform3D() 
    : position(Vector3::zero())
    , rotation(Quaternion::identity())
    , scale(Vector3::one())
    , parent(nullptr)
    , worldMatrixDirty(true) {}

Transform3D::~Transform3D() {
    cleanup();
}

void Transform3D::update(float deltaTime) {
    // Базовая реализация update для Transform3D
    // В данный момент не требует специфических обновлений
    // Этот метод может быть переопределен в производных классах
    // для добавления анимации или других динамических изменений
}

void Transform3D::cleanup() {
    // Удаляем из родителя
    if (parent) {
        parent->removeChild(this);
    }
    
    // Удаляем всех детей
    for (auto* child : children) {
        if (child) {
            child->parent = nullptr;
        }
    }
    children.clear();
}

void Transform3D::setPosition(const Vector3& pos) {
    position = pos;
    markWorldMatrixDirty();
}

void Transform3D::setRotation(const Quaternion& rot) {
    rotation = rot;
    markWorldMatrixDirty();
}

void Transform3D::setScale(const Vector3& scl) {
    scale = scl;
    markWorldMatrixDirty();
}

void Transform3D::translate(const Vector3& translation) {
    position += translation;
    markWorldMatrixDirty();
}

void Transform3D::rotate(const Quaternion& rot) {
    rotation = rotation * rot;
    markWorldMatrixDirty();
}

void Transform3D::scaleBy(const Vector3& scl) {
    scale = Vector3(
        scale.x * scl.x,
        scale.y * scl.y,
        scale.z * scl.z
    );
    markWorldMatrixDirty();
}

Matrix4 Transform3D::getLocalMatrix() const {
    Matrix4 translationMatrix = Matrix4::translation(position);
    Matrix4 rotationMatrix = rotation.toMatrix();
    Matrix4 scaleMatrix = Matrix4::scaling(scale);
    
    return translationMatrix * rotationMatrix * scaleMatrix;
}

Matrix4 Transform3D::getWorldMatrix() const {
    if (worldMatrixDirty) {
        updateWorldMatrix();
    }
    return cachedWorldMatrix;
}

Vector3 Transform3D::getWorldPosition() const {
    if (parent) {
        return parent->getWorldMatrix().transformPoint(position);
    }
    return position;
}

Quaternion Transform3D::getWorldRotation() const {
    if (parent) {
        return parent->getWorldRotation() * rotation;
    }
    return rotation;
}

Vector3 Transform3D::getWorldScale() const {
    if (parent) {
        Vector3 parentScale = parent->getWorldScale();
        return Vector3(
            parentScale.x * scale.x,
            parentScale.y * scale.y,
            parentScale.z * scale.z
        );
    }
    return scale;
}

Vector3 Transform3D::forward() const {
    return rotation.rotate(Vector3::forward());
}

Vector3 Transform3D::right() const {
    return rotation.rotate(Vector3::right());
}

Vector3 Transform3D::up() const {
    return rotation.rotate(Vector3::up());
}

void Transform3D::setParent(Transform3D* newParent) {
    if (this->parent) {
        this->parent->removeChild(this);
    }
    
    this->parent = newParent;
    if (newParent) {
        newParent->addChild(this);
    }
    
    markWorldMatrixDirty();
}

void Transform3D::addChild(Transform3D* child) {
    if (child && std::find(children.begin(), children.end(), child) == children.end()) {
        children.push_back(child);
        child->parent = this;
        child->markWorldMatrixDirty();
    }
}

void Transform3D::removeChild(Transform3D* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent = nullptr;
        child->markWorldMatrixDirty();
    }
}

void Transform3D::markWorldMatrixDirty() {
    worldMatrixDirty = true;
    
    // Помечаем всех детей как грязных
    for (auto* child : children) {
        if (child) {
            child->markWorldMatrixDirty();
        }
    }
}

void Transform3D::updateWorldMatrix() const {
    if (parent) {
        cachedWorldMatrix = parent->getWorldMatrix() * getLocalMatrix();
    } else {
        cachedWorldMatrix = getLocalMatrix();
    }
    worldMatrixDirty = false;
}

} // namespace Core
} // namespace HyperEngine

