#pragma once

#include "Component.h"
#include "Transform4D.h"
#include "Interfaces.h"
#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include "../Math/Quaternion4D.h"
#include <string>
#include <vector>
#include <memory>

namespace Engine4D {

// Forward declarations
namespace Physics {
    class Collider4D;
    class RigidBody4D;
    class ParticleSystem4D;
}

namespace Rendering {
    class Mesh4D;
    class Shader4D;
    class Camera4D;
}

namespace Core {

/**
 * @brief Компонент рендеринга для 4D объектов
 */
class MeshRenderer4D : public RenderableComponent {
public:
    MeshRenderer4D();
    virtual ~MeshRenderer4D() = default;
    
    // Component interface
    std::string getComponentType() const override { return "MeshRenderer4D"; }
    void render() override;
    
    // Renderer configuration
    void setMesh(std::shared_ptr<Rendering::Mesh4D> newMesh);
    void setShader(std::shared_ptr<Rendering::Shader4D> newShader);
    void setColor(const Math::Vector4& newColor);
    
    // Getters
    std::shared_ptr<Rendering::Mesh4D> getMesh() const { return mesh; }
    std::shared_ptr<Rendering::Shader4D> getShader() const { return shader; }
    const Math::Vector4& getColor() const { return color; }

private:
    std::shared_ptr<Rendering::Mesh4D> mesh;
    std::shared_ptr<Rendering::Shader4D> shader;
    Math::Vector4 color;
    bool castShadows;
    bool receiveShadows;
};

/**
 * @brief Компонент коллайдера для 4D объектов
 */
class Collider4DComponent : public Component4D {
public:
    Collider4DComponent();
    virtual ~Collider4DComponent() = default;
    
    // Component interface
    std::string getComponentType() const override { return "Collider4DComponent"; }
    
    // Collider configuration
    void setCollider(std::shared_ptr<Physics::Collider4D> newCollider);
    void setTrigger(bool newTrigger);
    
    // Collision detection
    bool checkCollision(const Collider4DComponent& other) const;
    
    // Getters
    std::shared_ptr<Physics::Collider4D> getCollider() const { return collider; }
    bool isTrigger() const { return trigger; }

private:
    std::shared_ptr<Physics::Collider4D> collider;
    bool trigger;
};

/**
 * @brief Компонент физического тела для 4D объектов
 */
class RigidBody4DComponent : public UpdatableComponent {
public:
    RigidBody4DComponent();
    virtual ~RigidBody4DComponent() = default;
    
    // Component interface
    std::string getComponentType() const override { return "RigidBody4DComponent"; }
    void update(float deltaTime) override;
    
    // Physics configuration
    void setRigidBody(std::shared_ptr<Physics::RigidBody4D> body);
    void setUseGravity(bool use);
    
    // Force application
    void addForce(const Math::Vector4& force);
    void addTorque(const Math::Vector4& torque);
    void addImpulse(const Math::Vector4& impulse);
    void addAngularImpulse(const Math::Vector4& impulse);
    
    // Getters
    std::shared_ptr<Physics::RigidBody4D> getRigidBody() const { return rigidBody; }
    bool getUseGravity() const { return useGravity; }

private:
    std::shared_ptr<Physics::RigidBody4D> rigidBody;
    bool useGravity;
};

/**
 * @brief Компонент камеры для 4D пространства
 */
class Camera4DComponent : public Component4D {
public:
    Camera4DComponent();
    virtual ~Camera4DComponent() = default;
    
    // Component interface
    std::string getComponentType() const override { return "Camera4DComponent"; }
    
    // Camera configuration
    void setMainCamera(bool main);
    void setFieldOfView(float fov);
    void setNearPlane(float near);
    void setFarPlane(float far);
    
    // Matrix getters
    Math::Matrix4 getViewMatrix() const;
    Math::Matrix4 getProjectionMatrix() const;
    Math::Matrix4 getViewProjectionMatrix() const;
    
    // Camera getters
    Rendering::Camera4D& getCamera() { return *camera; }
    const Rendering::Camera4D& getCamera() const { return *camera; }
    bool getIsMainCamera() const { return mainCamera; }

private:
    std::shared_ptr<Rendering::Camera4D> camera;
    bool mainCamera;
    float fieldOfView;
    float nearPlane;
    float farPlane;
};

/**
 * @brief Компонент системы частиц для 4D
 */
class ParticleSystem4DComponent : public UpdatableRenderableComponent {
public:
    ParticleSystem4DComponent();
    virtual ~ParticleSystem4DComponent() = default;
    
    // Component interface
    std::string getComponentType() const override { return "ParticleSystem4DComponent"; }
    void update(float deltaTime) override;
    void render() override;
    
    // Particle system configuration
    void setParticleSystem(std::shared_ptr<Physics::ParticleSystem4D> system);
    void setAutoPlay(bool play);
    void setEmissionRate(float rate);
    
    // Particle system control
    void play();
    void stop();
    void emit(int count = 1);
    
    // Getters
    std::shared_ptr<Physics::ParticleSystem4D> getParticleSystem() const { return particleSystem; }
    bool getAutoPlay() const { return autoPlay; }
    float getEmissionRate() const { return emissionRate; }

private:
    std::shared_ptr<Physics::ParticleSystem4D> particleSystem;
    bool autoPlay;
    float emissionRate;
};

/**
 * @brief 4D игровой объект following SRP and providing clean interface
 */
class GameObject4D : public ILifecycle {
public:
    explicit GameObject4D(const std::string& name = "GameObject4D");
    virtual ~GameObject4D();

    // ILifecycle implementation
    void start() override;
    void cleanup() override;

    // Object identification
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { this->name = newName; }
    const std::string& getTag() const { return tag; }
    void setTag(const std::string& newTag) { this->tag = newTag; }

    // Object state management
    void setActive(bool newActive);
    bool isActive() const { return active; }
    bool isStatic() const { return staticObject; }
    void setStatic(bool newIsStatic) { staticObject = newIsStatic; }
    void destroy();

    // Transform access
    Transform4D* getTransform() { return transform.get(); }
    const Transform4D* getTransform() const { return transform.get(); }

    // Component management
    template<typename T>
    T* addComponent();
    template<typename T>
    T* getComponent();
    template<typename T>
    std::vector<T*> getComponents();
    template<typename T>
    void removeComponent(T* component);
    void removeComponent(const std::string& componentType);

    // System updates
    void update(float deltaTime);
    void render();

    // Static object management
    static GameObject4D* find(const std::string& name);
    static std::vector<GameObject4D*> findAllWithTag(const std::string& tag);
    static GameObject4D* findWithTag(const std::string& tag);
    static GameObject4D* create(const std::string& name = "GameObject4D");
    static GameObject4D* createPrimitive(const std::string& type);

    // Object registry management
    static void clearAllObjects();
    static const std::vector<GameObject4D*>& getAllObjects();

private:
    // Object properties
    std::string name;
    std::string tag;
    bool active;
    bool staticObject;
    bool started;

    // Components
    std::shared_ptr<Transform4D> transform;
    std::vector<std::shared_ptr<Component4D>> components;

    // Static registry
    static std::vector<GameObject4D*> allObjects;

    // Helper methods
    void addToAllObjects();
    void removeFromAllObjects();
    void updateComponents(float deltaTime);
    void renderComponents();
};

// Шаблонные методы
template<typename T>
T* GameObject4D::addComponent() {
    static_assert(std::is_base_of<Component4D, T>::value, "T must inherit from Component4D");
    
    auto component = std::make_shared<T>();
    component->gameObject = this;
    components.push_back(component);
    return component.get();
}

template<typename T>
T* GameObject4D::getComponent() {
    static_assert(std::is_base_of<Component4D, T>::value, "T must inherit from Component4D");
    
    for (auto& component : components) {
        if (auto* casted = dynamic_cast<T*>(component.get())) {
            return casted;
        }
    }
    return nullptr;
}

template<typename T>
std::vector<T*> GameObject4D::getComponents() {
    static_assert(std::is_base_of<Component4D, T>::value, "T must inherit from Component4D");
    
    std::vector<T*> result;
    for (auto& component : components) {
        if (auto* casted = dynamic_cast<T*>(component.get())) {
            result.push_back(casted);
        }
    }
    return result;
}

template<typename T>
void GameObject4D::removeComponent(T* component) {
    static_assert(std::is_base_of<Component4D, T>::value, "T must inherit from Component4D");
    
    components.erase(
        std::remove_if(components.begin(), components.end(),
            [component](const std::shared_ptr<Component4D>& comp) {
                return comp.get() == component;
            }),
        components.end()
    );
}

} // namespace Core
} // namespace Engine4D
