#include "Engine4D/Core/Transform4D.h"
#include <algorithm>

using namespace Engine4D::Math;

namespace Engine4D {
namespace Core {

Transform4D::Transform4D() 
    : position(Vector4::zero())
    , rotation(Quaternion4D::identity())
    , scale(Vector4::one())
    , parent(nullptr)
    , worldMatrixDirty(true) {}

Transform4D::~Transform4D() {
    cleanup();
}

void Transform4D::cleanup() {
    // Remove from parent
    if (parent) {
        parent->removeChild(this);
    }
    
    // Remove all children
    for (auto* child : children) {
        if (child) {
            child->parent = nullptr;
        }
    }
    children.clear();
}

void Transform4D::setPosition(const Vector4& pos) {
    position = pos;
    markWorldMatrixDirty();
}

void Transform4D::setRotation(const Quaternion4D& rot) {
    rotation = rot;
    markWorldMatrixDirty();
}

void Transform4D::setScale(const Vector4& scl) {
    scale = scl;
    markWorldMatrixDirty();
}

void Transform4D::translate(const Vector4& translation) {
    position += translation;
    markWorldMatrixDirty();
}

void Transform4D::rotate(const Quaternion4D& rot) {
    rotation = rotation * rot;
    markWorldMatrixDirty();
}

void Transform4D::scaleBy(const Vector4& scl) {
    scale = Vector4(
        scale.x * scl.x,
        scale.y * scl.y,
        scale.z * scl.z,
        scale.w * scl.w
    );
    markWorldMatrixDirty();
}

Matrix4 Transform4D::getLocalMatrix() const {
    Matrix4 translationMatrix = Matrix4::translation(position);
    Matrix4 rotationMatrix = rotation.toMatrix();
    Matrix4 scaleMatrix = Matrix4::scaling(scale);
    
    return translationMatrix * rotationMatrix * scaleMatrix;
}

Matrix4 Transform4D::getWorldMatrix() const {
    if (worldMatrixDirty) {
        updateWorldMatrix();
    }
    return cachedWorldMatrix;
}

Vector4 Transform4D::getWorldPosition() const {
    if (parent) {
        return parent->getWorldMatrix() * Vector4(position.x, position.y, position.z, position.w);
    }
    return position;
}

Quaternion4D Transform4D::getWorldRotation() const {
    if (parent) {
        return parent->getWorldRotation() * rotation;
    }
    return rotation;
}

Vector4 Transform4D::getWorldScale() const {
    if (parent) {
        Vector4 parentScale = parent->getWorldScale();
        return Vector4(
            parentScale.x * scale.x,
            parentScale.y * scale.y,
            parentScale.z * scale.z,
            parentScale.w * scale.w
        );
    }
    return scale;
}

Vector4 Transform4D::forward() const {
    return rotation.rotate(Vector4::unitZ());
}

Vector4 Transform4D::right() const {
    return rotation.rotate(Vector4::unitX());
}

Vector4 Transform4D::up() const {
    return rotation.rotate(Vector4::unitY());
}

Vector4 Transform4D::wAxis() const {
    return rotation.rotate(Vector4::unitW());
}

void Transform4D::setParent(Transform4D* newParent) {
    if (this->parent) {
        this->parent->removeChild(this);
    }
    
    this->parent = newParent;
    if (newParent) {
        newParent->addChild(this);
    }
    
    markWorldMatrixDirty();
}

void Transform4D::addChild(Transform4D* child) {
    if (child && std::find(children.begin(), children.end(), child) == children.end()) {
        children.push_back(child);
        child->parent = this;
        child->markWorldMatrixDirty();
    }
}

void Transform4D::removeChild(Transform4D* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent = nullptr;
        child->markWorldMatrixDirty();
    }
}

void Transform4D::markWorldMatrixDirty() {
    worldMatrixDirty = true;
    
    // Mark all children as dirty
    for (auto* child : children) {
        if (child) {
            child->markWorldMatrixDirty();
        }
    }
}

void Transform4D::updateWorldMatrix() const {
    if (parent) {
        cachedWorldMatrix = parent->getWorldMatrix() * getLocalMatrix();
    } else {
        cachedWorldMatrix = getLocalMatrix();
    }
    worldMatrixDirty = false;
}

} // namespace Core
} // namespace Engine4D
