#include "Engine4D/Core/GameObject4D.h"
#include "Engine4D/Core/PrimitiveFactory.h"
#include "Engine4D/Rendering/Renderer.h"
#include "Engine4D/Physics/Physics4D.h"
#include <algorithm>
#include <iostream>

using namespace Engine4D::Math;

namespace Engine4D {
namespace Core {

// Статические переменные
std::vector<GameObject4D*> GameObject4D::allObjects;

// GameObject4D Implementation
GameObject4D::GameObject4D(const std::string& name) 
    : name(name)
    , tag("Untagged")
    , active(true)
    , staticObject(false)
    , started(false) {
    
    transform = std::make_shared<Transform4D>();
    transform->setGameObject(this);
    
    addToAllObjects();
}

GameObject4D::~GameObject4D() {
    removeFromAllObjects();
    cleanup();
}

void GameObject4D::start() {
    if (started || !active) return;
    
    started = true;
    
    // Start transform
    transform->start();
    
    // Start all components
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            component->start();
        }
    }
}

void GameObject4D::cleanup() {
    // Cleanup all components
    for (auto& component : components) {
        if (component) {
            component->cleanup();
        }
    }
    components.clear();
    
    // Cleanup transform
    if (transform) {
        transform->cleanup();
    }
}

void GameObject4D::setActive(bool newActive) {
    this->active = newActive;
}

void GameObject4D::destroy() {
    setActive(false);
    // Mark for destruction - in a real implementation this would be handled by a scene manager
}

void GameObject4D::removeComponent(const std::string& componentType) {
    components.erase(
        std::remove_if(components.begin(), components.end(),
            [&componentType](const std::shared_ptr<Component4D>& comp) {
                return comp && comp->getComponentType() == componentType;
            }),
        components.end()
    );
}

void GameObject4D::update(float deltaTime) {
    if (!active) return;
    
    updateComponents(deltaTime);
}

void GameObject4D::render() {
    if (!active) return;
    
    renderComponents();
}

void GameObject4D::updateComponents(float deltaTime) {
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            // Try to cast to IUpdatable and update if possible
            if (auto* updatable = dynamic_cast<IUpdatable*>(component.get())) {
                updatable->update(deltaTime);
            }
        }
    }
}

void GameObject4D::renderComponents() {
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            // Try to cast to IRenderable and render if possible
            if (auto* renderable = dynamic_cast<IRenderable*>(component.get())) {
                renderable->render();
            }
        }
    }
}

// Static methods
GameObject4D* GameObject4D::find(const std::string& name) {
    for (auto* obj : allObjects) {
        if (obj && obj->name == name && obj->active) {
            return obj;
        }
    }
    return nullptr;
}

std::vector<GameObject4D*> GameObject4D::findAllWithTag(const std::string& tag) {
    std::vector<GameObject4D*> result;
    for (auto* obj : allObjects) {
        if (obj && obj->tag == tag && obj->active) {
            result.push_back(obj);
        }
    }
    return result;
}

GameObject4D* GameObject4D::findWithTag(const std::string& tag) {
    for (auto* obj : allObjects) {
        if (obj && obj->tag == tag && obj->active) {
            return obj;
        }
    }
    return nullptr;
}

GameObject4D* GameObject4D::create(const std::string& name) {
    return new GameObject4D(name);
}

GameObject4D* GameObject4D::createPrimitive(const std::string& type) {
    auto factory = PrimitiveFactory::getInstance();
    auto primitive = factory.createPrimitive(type);
    return primitive.get(); // Note: This transfers ownership - consider using shared_ptr throughout
}

void GameObject4D::clearAllObjects() {
    for (auto* obj : allObjects) {
        delete obj;
    }
    allObjects.clear();
}

const std::vector<GameObject4D*>& GameObject4D::getAllObjects() {
    return allObjects;
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

// MeshRenderer4D Implementation
MeshRenderer4D::MeshRenderer4D() 
    : color(Vector4::one())
    , castShadows(true)
    , receiveShadows(true) {}

void MeshRenderer4D::setMesh(std::shared_ptr<Rendering::Mesh4D> newMesh) {
    this->mesh = newMesh;
}

void MeshRenderer4D::setShader(std::shared_ptr<Rendering::Shader4D> newShader) {
    this->shader = newShader;
}

void MeshRenderer4D::setColor(const Vector4& newColor) {
    this->color = newColor;
}

void MeshRenderer4D::render() {
    if (!enabled || !mesh || !shader || !gameObject) return;
    
    Matrix4 transform = gameObject->getTransform()->getWorldMatrix();
    Rendering::Renderer::getInstance().renderMesh(*mesh, transform, *shader);
}

// Collider4DComponent Implementation
Collider4DComponent::Collider4DComponent() : trigger(false) {}

void Collider4DComponent::setCollider(std::shared_ptr<Physics::Collider4D> newCollider) {
    this->collider = newCollider;
}

void Collider4DComponent::setTrigger(bool newTrigger) {
    this->trigger = newTrigger;
}

bool Collider4DComponent::checkCollision(const Collider4DComponent& other) const {
    if (!enabled || !other.enabled || !collider || !other.collider) {
        return false;
    }
    
    return collider->intersects(*other.collider);
}

// RigidBody4DComponent Implementation  
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
    if (!enabled || !rigidBody || !gameObject) return;
    
    // Sync position with transform
    rigidBody->setPosition(gameObject->getTransform()->getWorldPosition());
    
    // Update physics
    rigidBody->update(deltaTime);
    
    // Update transform
    gameObject->getTransform()->setPosition(rigidBody->getPosition());
}

// Camera4DComponent Implementation
Camera4DComponent::Camera4DComponent() 
    : mainCamera(false)
    , fieldOfView(45.0f)
    , nearPlane(0.1f)
    , farPlane(100.0f) {
    camera = std::make_shared<Rendering::Camera4D>();
    camera->fov = fieldOfView;
    camera->nearPlane = nearPlane;
    camera->farPlane = farPlane;
}

void Camera4DComponent::setMainCamera(bool main) {
    this->mainCamera = main;
}

void Camera4DComponent::setFieldOfView(float fov) {
    this->fieldOfView = fov;
    if (camera) {
        camera->fov = fov;
    }
}

void Camera4DComponent::setNearPlane(float near) {
    this->nearPlane = near;
    if (camera) {
        camera->nearPlane = near;
    }
}

void Camera4DComponent::setFarPlane(float far) {
    this->farPlane = far;
    if (camera) {
        camera->farPlane = far;
    }
}

Matrix4 Camera4DComponent::getViewMatrix() const {
    return camera ? camera->getViewMatrix() : Matrix4::identity();
}

Matrix4 Camera4DComponent::getProjectionMatrix() const {
    return camera ? camera->getProjectionMatrix() : Matrix4::identity();
}

Matrix4 Camera4DComponent::getViewProjectionMatrix() const {
    return camera ? camera->getViewProjectionMatrix() : Matrix4::identity();
}

// ParticleSystem4DComponent Implementation
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
    if (!enabled || !particleSystem || !gameObject) return;
    
    // Update emitter position
    particleSystem->setEmitter(
        gameObject->getTransform()->getWorldPosition(),
        Vector4::zero() // Could add emitter velocity
    );
    
    particleSystem->update(deltaTime);
}

void ParticleSystem4DComponent::render() {
    if (!enabled || !particleSystem) return;
    
    // Render particles - real implementation would include particle rendering logic
}

} // namespace Core
} // namespace Engine4D
