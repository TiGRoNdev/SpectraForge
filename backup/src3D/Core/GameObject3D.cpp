#include "HyperEngine/Core/GameObject3D.h"
#include "HyperEngine/Core/Component.h"
#include <algorithm>
#include <iostream>

using namespace HyperEngine::Core;
using namespace HyperEngine::Math;

namespace HyperEngine {
namespace Core {

// Статический реестр объектов
std::vector<GameObject3D*> GameObject3D::allObjects;

// Конструкторы и деструкторы
GameObject3D::GameObject3D(const std::string& name)
    : name(name)
    , tag("")
    , active(true)
    , staticObject(false)
    , started(false) {
    
    // Создаем компонент трансформации
    transform = std::make_shared<Transform3D>();
    transform->setGameObject(this);
    
    // Добавляем объект в реестр
    addToAllObjects();
}

GameObject3D::~GameObject3D() {
    cleanup();
    removeFromAllObjects();
}

// Реализация ILifecycle
void GameObject3D::start() {
    if (started) return;
    
    started = true;
    transform->start();
    
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            component->start();
        }
    }
}

void GameObject3D::cleanup() {
    for (auto& component : components) {
        if (component) {
            component->cleanup();
        }
    }
    
    if (transform) {
        transform->cleanup();
    }
    
    components.clear();
    transform.reset();
}

// Управление состоянием объекта
void GameObject3D::setActive(bool newActive) {
    active = newActive;
}

void GameObject3D::destroy() {
    cleanup();
    removeFromAllObjects();
}

// Удаление компонента по типу
void GameObject3D::removeComponent(const std::string& componentType) {
    components.erase(
        std::remove_if(components.begin(), components.end(),
            [&componentType](const std::shared_ptr<Component3D>& comp) {
                return comp && comp->getComponentType() == componentType;
            }),
        components.end()
    );
}

// Системные обновления
void GameObject3D::update(float deltaTime) {
    if (!active) return;
    
    // Запускаем объект, если еще не запущен
    if (!started) {
        start();
    }
    
    // Обновляем трансформацию
    if (transform) {
        transform->update(deltaTime);
    }
    
    // Обновляем компоненты
    updateComponents(deltaTime);
}

void GameObject3D::render() {
    if (!active) return;
    
    // Рендерим компоненты
    renderComponents();
}

// Статическое управление объектами
GameObject3D* GameObject3D::find(const std::string& name) {
    for (GameObject3D* obj : allObjects) {
        if (obj && obj->getName() == name) {
            return obj;
        }
    }
    return nullptr;
}

std::vector<GameObject3D*> GameObject3D::findAllWithTag(const std::string& tag) {
    std::vector<GameObject3D*> result;
    for (GameObject3D* obj : allObjects) {
        if (obj && obj->getTag() == tag) {
            result.push_back(obj);
        }
    }
    return result;
}

GameObject3D* GameObject3D::findWithTag(const std::string& tag) {
    for (GameObject3D* obj : allObjects) {
        if (obj && obj->getTag() == tag) {
            return obj;
        }
    }
    return nullptr;
}

GameObject3D* GameObject3D::create(const std::string& name) {
    return new GameObject3D(name);
}

GameObject3D* GameObject3D::createPrimitive(const std::string& type) {
    GameObject3D* obj = create("Primitive_" + type);
    
    // Добавляем MeshRenderer компонент
    auto* meshRenderer = obj->addComponent<MeshRenderer3D>();
    
    // TODO: Реализовать создание примитивных мешей
    if (type == "Cube") {
        // meshRenderer->setMesh(MeshFactory::createCube());
    } else if (type == "Sphere") {
        // meshRenderer->setMesh(MeshFactory::createSphere());
    } else if (type == "Plane") {
        // meshRenderer->setMesh(MeshFactory::createPlane());
    }
    
    return obj;
}

// Управление реестром объектов
void GameObject3D::clearAllObjects() {
    // Создаем копию списка, чтобы избежать проблем при удалении
    std::vector<GameObject3D*> objectsToDelete = allObjects;
    
    for (GameObject3D* obj : objectsToDelete) {
        if (obj) {
            delete obj;
        }
    }
    
    allObjects.clear();
}

const std::vector<GameObject3D*>& GameObject3D::getAllObjects() {
    return allObjects;
}

// Вспомогательные методы
void GameObject3D::addToAllObjects() {
    allObjects.push_back(this);
}

void GameObject3D::removeFromAllObjects() {
    allObjects.erase(
        std::remove(allObjects.begin(), allObjects.end(), this),
        allObjects.end()
    );
}

void GameObject3D::updateComponents(float deltaTime) {
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            // Проверяем, является ли компонент обновляемым
            if (auto* updatable = dynamic_cast<IUpdatable*>(component.get())) {
                updatable->update(deltaTime);
            }
        }
    }
}

void GameObject3D::renderComponents() {
    for (auto& component : components) {
        if (component && component->isEnabled()) {
            // Проверяем, является ли компонент рендерируемым
            if (auto* renderable = dynamic_cast<IRenderable*>(component.get())) {
                renderable->render();
            }
        }
    }
}

// Реализация компонентов

// MeshRenderer3D
MeshRenderer3D::MeshRenderer3D() 
    : color(1.0f, 1.0f, 1.0f)
    , castShadows(true)
    , receiveShadows(true) {
}

void MeshRenderer3D::render() {
    if (!mesh || !shader) {
        return;
    }
    
    // TODO: Реализовать рендеринг через систему рендеринга
    std::cout << "Rendering mesh for object: " << gameObject->getName() << std::endl;
}

void MeshRenderer3D::setMesh(std::shared_ptr<Rendering::Mesh3D> newMesh) {
    mesh = newMesh;
}

void MeshRenderer3D::setShader(std::shared_ptr<Rendering::Shader3D> newShader) {
    shader = newShader;
}

void MeshRenderer3D::setColor(const Vector3& newColor) {
    color = newColor;
}

// Collider3DComponent
Collider3DComponent::Collider3DComponent() 
    : trigger(false) {
}

void Collider3DComponent::setCollider(std::shared_ptr<Physics::Collider3D> newCollider) {
    collider = newCollider;
}

void Collider3DComponent::setTrigger(bool newTrigger) {
    trigger = newTrigger;
}

bool Collider3DComponent::checkCollision(const Collider3DComponent& other) const {
    if (!collider || !other.collider) {
        return false;
    }
    
    // TODO: Реализовать проверку столкновений
    return false;
}

// RigidBody3DComponent
RigidBody3DComponent::RigidBody3DComponent() 
    : useGravity(true) {
}

void RigidBody3DComponent::update(float deltaTime) {
    if (!rigidBody) {
        return;
    }
    
    // TODO: Обновление физического тела
    // rigidBody->update(deltaTime);
}

void RigidBody3DComponent::setRigidBody(std::shared_ptr<Physics::RigidBody3D> body) {
    rigidBody = body;
}

void RigidBody3DComponent::setUseGravity(bool use) {
    useGravity = use;
}

void RigidBody3DComponent::addForce(const Vector3& force) {
    if (rigidBody) {
        // TODO: Применение силы
        // rigidBody->applyForce(force);
    }
}

void RigidBody3DComponent::addTorque(const Vector3& torque) {
    if (rigidBody) {
        // TODO: Применение момента силы
        // rigidBody->applyTorque(torque);
    }
}

void RigidBody3DComponent::addImpulse(const Vector3& impulse) {
    if (rigidBody) {
        // TODO: Применение импульса
        // rigidBody->applyImpulse(impulse);
    }
}

void RigidBody3DComponent::addAngularImpulse(const Vector3& impulse) {
    if (rigidBody) {
        // TODO: Применение углового импульса
        // rigidBody->applyAngularImpulse(impulse);
    }
}

// Camera3DComponent
Camera3DComponent::Camera3DComponent() 
    : mainCamera(false)
    , fieldOfView(60.0f)
    , nearPlane(0.1f)
    , farPlane(1000.0f) {
    // TODO: Создание камеры
    // camera = std::make_shared<Rendering::Camera3D>();
}

void Camera3DComponent::setMainCamera(bool main) {
    mainCamera = main;
    
    // TODO: Установка главной камеры в системе рендеринга
    if (main && camera) {
        // Renderer::getInstance().setMainCamera(camera);
    }
}

void Camera3DComponent::setFieldOfView(float fov) {
    fieldOfView = fov;
    if (camera) {
        // TODO: Обновление FOV камеры
        // camera->setFieldOfView(fov);
    }
}

void Camera3DComponent::setNearPlane(float near) {
    nearPlane = near;
    if (camera) {
        // TODO: Обновление near plane камеры
        // camera->setNearPlane(near);
    }
}

void Camera3DComponent::setFarPlane(float far) {
    farPlane = far;
    if (camera) {
        // TODO: Обновление far plane камеры
        // camera->setFarPlane(far);
    }
}

Matrix4 Camera3DComponent::getViewMatrix() const {
    if (camera) {
        // TODO: Получение view матрицы
        // return camera->getViewMatrix();
    }
    return Matrix4::identity();
}

Matrix4 Camera3DComponent::getProjectionMatrix() const {
    if (camera) {
        // TODO: Получение projection матрицы
        // return camera->getProjectionMatrix();
    }
    return Matrix4::identity();
}

Matrix4 Camera3DComponent::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

// ParticleSystem3DComponent
ParticleSystem3DComponent::ParticleSystem3DComponent() 
    : autoPlay(true)
    , emissionRate(10.0f) {
}

void ParticleSystem3DComponent::update(float deltaTime) {
    if (particleSystem && autoPlay) {
        // TODO: Обновление системы частиц
        // particleSystem->update(deltaTime);
    }
}

void ParticleSystem3DComponent::render() {
    if (particleSystem) {
        // TODO: Рендеринг частиц
        // particleSystem->render();
    }
}

void ParticleSystem3DComponent::setParticleSystem(std::shared_ptr<Physics::ParticleSystem3D> system) {
    particleSystem = system;
}

void ParticleSystem3DComponent::setAutoPlay(bool play) {
    autoPlay = play;
}

void ParticleSystem3DComponent::setEmissionRate(float rate) {
    emissionRate = rate;
    if (particleSystem) {
        // TODO: Установка скорости эмиссии
        // particleSystem->setEmissionRate(rate);
    }
}

void ParticleSystem3DComponent::play() {
    if (particleSystem) {
        // TODO: Запуск системы частиц
        // particleSystem->play();
    }
}

void ParticleSystem3DComponent::stop() {
    if (particleSystem) {
        // TODO: Остановка системы частиц
        // particleSystem->stop();
    }
}

void ParticleSystem3DComponent::emit(int count) {
    if (particleSystem) {
        // TODO: Эмиссия частиц
        // particleSystem->emit(count);
    }
}

} // namespace Core
} // namespace HyperEngine

