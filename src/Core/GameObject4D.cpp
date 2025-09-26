#include "Engine4D/Core/GameObject4D.h"
#include <algorithm>
#include <iostream>

namespace Engine4D {
namespace Core {

// Статические переменные
std::vector<GameObject4D*> GameObject4D::allObjects;

// Component4D
Component4D::Component4D() : gameObject(nullptr), enabled(true) {}

void Component4D::setEnabled(bool enabled) {
    this->enabled = enabled;
}

// Transform4D
Transform4D::Transform4D() 
    : position(Vector4::zero())
    , rotation(Quaternion4D::identity())
    , scale(Vector4::one())
    , parent(nullptr)
    , worldMatrixDirty(true) {}

Transform4D::~Transform4D() {
    // Удаляем из родительского объекта
    if (parent) {
        parent->removeChild(this);
    }
    
    // Удаляем всех детей
    for (auto* child : children) {
        child->parent = nullptr;
    }
    children.clear();
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

void Transform4D::setParent(Transform4D* parent) {
    if (this->parent) {
        this->parent->removeChild(this);
    }
    
    this->parent = parent;
    if (parent) {
        parent->addChild(this);
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
    
    // Помечаем всех детей как грязные
    for (auto* child : children) {
        child->markWorldMatrixDirty();
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

// MeshRenderer4D
MeshRenderer4D::MeshRenderer4D() 
    : color(Vector4::one())
    , castShadows(true)
    , receiveShadows(true) {}

void MeshRenderer4D::setMesh(std::shared_ptr<Rendering::Mesh4D> mesh) {
    this->mesh = mesh;
}

void MeshRenderer4D::setShader(std::shared_ptr<Rendering::Shader4D> shader) {
    this->shader = shader;
}

void MeshRenderer4D::setColor(const Vector4& color) {
    this->color = color;
}

void MeshRenderer4D::render() {
    if (!enabled || !mesh || !shader) return;
    
    Matrix4 transform = gameObject->transform->getWorldMatrix();
    Rendering::Renderer::getInstance().renderMesh(*mesh, transform, *shader);
}

// Collider4DComponent
Collider4DComponent::Collider4DComponent() : isTrigger(false) {}

void Collider4DComponent::setCollider(std::shared_ptr<Physics::Collider4D> collider) {
    this->collider = collider;
}

void Collider4DComponent::setTrigger(bool trigger) {
    this->isTrigger = trigger;
}

bool Collider4DComponent::checkCollision(const Collider4DComponent& other) const {
    if (!enabled || !other.enabled || !collider || !other.collider) {
        return false;
    }
    
    return collider->intersects(*other.collider);
}

// RigidBody4DComponent
RigidBody4DComponent::RigidBody4DComponent() : useGravity(true) {}

void RigidBody4DComponent::setRigidBody(std::shared_ptr<Physics::RigidBody4D> body) {
    this->rigidBody = body;
}

void RigidBody4DComponent::setUseGravity(bool use) {
    this->useGravity = use;
}

void RigidBody4DComponent::addForce(const Vector4& force) {
    if (rigidBody) {
        rigidBody->applyForce(force);
    }
}

void RigidBody4DComponent::addTorque(const Vector4& torque) {
    if (rigidBody) {
        rigidBody->applyTorque(torque);
    }
}

void RigidBody4DComponent::addImpulse(const Vector4& impulse) {
    if (rigidBody) {
        rigidBody->applyImpulse(impulse);
    }
}

void RigidBody4DComponent::addAngularImpulse(const Vector4& impulse) {
    if (rigidBody) {
        rigidBody->applyAngularImpulse(impulse);
    }
}

void RigidBody4DComponent::update(float deltaTime) {
    if (!enabled || !rigidBody) return;
    
    // Синхронизируем позицию с трансформом
    rigidBody->setPosition(gameObject->transform->getWorldPosition());
    
    // Обновляем физику
    rigidBody->update(deltaTime);
    
    // Обновляем трансформ
    gameObject->transform->setPosition(rigidBody->getPosition());
}

// Camera4DComponent
Camera4DComponent::Camera4DComponent() 
    : isMainCamera(false)
    , fieldOfView(45.0f)
    , nearPlane(0.1f)
    , farPlane(100.0f) {
    camera.fieldOfView = fieldOfView;
    camera.nearPlane = nearPlane;
    camera.farPlane = farPlane;
}

void Camera4DComponent::setMainCamera(bool main) {
    this->isMainCamera = main;
}

void Camera4DComponent::setFieldOfView(float fov) {
    this->fieldOfView = fov;
    camera.fieldOfView = fov;
}

void Camera4DComponent::setNearPlane(float near) {
    this->nearPlane = near;
    camera.nearPlane = near;
}

void Camera4DComponent::setFarPlane(float far) {
    this->farPlane = far;
    camera.farPlane = far;
}

Matrix4 Camera4DComponent::getViewMatrix() const {
    return camera.getViewMatrix();
}

Matrix4 Camera4DComponent::getProjectionMatrix() const {
    return camera.getProjectionMatrix();
}

Matrix4 Camera4DComponent::getViewProjectionMatrix() const {
    return camera.getViewProjectionMatrix();
}

// ParticleSystem4DComponent
ParticleSystem4DComponent::ParticleSystem4DComponent() 
    : autoPlay(true)
    , emissionRate(10.0f) {}

void ParticleSystem4DComponent::setParticleSystem(std::shared_ptr<Physics::ParticleSystem4D> system) {
    this->particleSystem = system;
}

void ParticleSystem4DComponent::setAutoPlay(bool play) {
    this->autoPlay = play;
}

void ParticleSystem4DComponent::setEmissionRate(float rate) {
    this->emissionRate = rate;
    if (particleSystem) {
        particleSystem->setEmissionRate(rate);
    }
}

void ParticleSystem4DComponent::play() {
    if (particleSystem) {
        particleSystem->setEmissionRate(emissionRate);
    }
}

void ParticleSystem4DComponent::stop() {
    if (particleSystem) {
        particleSystem->setEmissionRate(0.0f);
    }
}

void ParticleSystem4DComponent::emit(int count) {
    if (particleSystem) {
        particleSystem->emit(count);
    }
}

void ParticleSystem4DComponent::update(float deltaTime) {
    if (!enabled || !particleSystem) return;
    
    // Обновляем позицию эмиттера
    particleSystem->setEmitter(
        gameObject->transform->getWorldPosition(),
        Vector4::zero() // Можно добавить скорость эмиттера
    );
    
    particleSystem->update(deltaTime);
}

void ParticleSystem4DComponent::render() {
    if (!enabled || !particleSystem) return;
    
    // Рендерим частицы
    // В реальной реализации здесь должна быть логика рендеринга частиц
}

// GameObject4D
GameObject4D::GameObject4D(const std::string& name) 
    : name(name)
    , tag("Untagged")
    , active(true)
    , staticObject(false)
    , started(false) {
    
    transform = new Transform4D();
    transform->gameObject = this;
    
    addToAllObjects();
}

GameObject4D::~GameObject4D() {
    removeFromAllObjects();
    
    // Очищаем компоненты
    for (auto& component : components) {
        component->cleanup();
    }
    components.clear();
    
    // Удаляем трансформ
    delete transform;
}

void GameObject4D::setActive(bool active) {
    this->active = active;
}

void GameObject4D::destroy() {
    setActive(false);
    // В реальной реализации здесь должна быть логика отложенного удаления
}

void GameObject4D::start() {
    if (started || !active) return;
    
    started = true;
    
    for (auto& component : components) {
        if (component->enabled) {
            component->start();
        }
    }
}

void GameObject4D::update(float deltaTime) {
    if (!active) return;
    
    for (auto& component : components) {
        if (component->enabled) {
            component->update(deltaTime);
        }
    }
}

void GameObject4D::render() {
    if (!active) return;
    
    for (auto& component : components) {
        if (component->enabled) {
            component->render();
        }
    }
}

void GameObject4D::cleanup() {
    for (auto& component : components) {
        component->cleanup();
    }
}

GameObject4D* GameObject4D::find(const std::string& name) {
    for (auto* obj : allObjects) {
        if (obj->name == name && obj->active) {
            return obj;
        }
    }
    return nullptr;
}

std::vector<GameObject4D*> GameObject4D::findAllWithTag(const std::string& tag) {
    std::vector<GameObject4D*> result;
    for (auto* obj : allObjects) {
        if (obj->tag == tag && obj->active) {
            result.push_back(obj);
        }
    }
    return result;
}

GameObject4D* GameObject4D::findWithTag(const std::string& tag) {
    for (auto* obj : allObjects) {
        if (obj->tag == tag && obj->active) {
            return obj;
        }
    }
    return nullptr;
}

GameObject4D* GameObject4D::create(const std::string& name) {
    return new GameObject4D(name);
}

GameObject4D* GameObject4D::createPrimitive(const std::string& type) {
    GameObject4D* obj = new GameObject4D(type);
    
    if (type == "Tesseract") {
        // Создаем тессеракт
        auto* renderer = obj->addComponent<MeshRenderer4D>();
        auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createTesseract());
        mesh->uploadToGPU();
        renderer->setMesh(mesh);
        
        // Добавляем коллайдер
        auto* collider = obj->addComponent<Collider4DComponent>();
        auto boxCollider = std::make_shared<Physics::BoxCollider4D>(Vector4::one());
        collider->setCollider(boxCollider);
        
    } else if (type == "Sphere") {
        // Создаем 4D сферу
        auto* renderer = obj->addComponent<MeshRenderer4D>();
        auto mesh = std::make_shared<Rendering::Mesh4D>(Rendering::Mesh4D::createSimplex());
        mesh->uploadToGPU();
        renderer->setMesh(mesh);
        
        // Добавляем коллайдер
        auto* collider = obj->addComponent<Collider4DComponent>();
        auto sphereCollider = std::make_shared<Physics::SphereCollider4D>(1.0f);
        collider->setCollider(sphereCollider);
    }
    
    return obj;
}

void GameObject4D::addToAllObjects() {
    allObjects.push_back(this);
}

void GameObject4D::removeFromAllObjects() {
    allObjects.erase(
        std::remove(allObjects.begin(), allObjects.end(), this),
        allObjects.end()
    );
}

} // namespace Core
} // namespace Engine4D
