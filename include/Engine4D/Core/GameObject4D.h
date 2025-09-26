#pragma once

#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include "../Math/Quaternion4D.h"
#include "../Physics/Physics4D.h"
#include "../Rendering/Renderer.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace Engine4D {
namespace Core {

/**
 * @brief Компонент 4D объекта
 */
class Component4D {
public:
    GameObject4D* gameObject;
    bool enabled;
    
    Component4D();
    virtual ~Component4D() = default;
    
    virtual void start() {}
    virtual void update(float deltaTime) {}
    virtual void render() {}
    virtual void cleanup() {}
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled; }
};

/**
 * @brief Компонент трансформации для 4D объектов
 */
class Transform4D : public Component4D {
public:
    Vector4 position;
    Quaternion4D rotation;
    Vector4 scale;
    
    Transform4D();
    virtual ~Transform4D() = default;
    
    Matrix4 getLocalMatrix() const;
    Matrix4 getWorldMatrix() const;
    
    Vector4 getWorldPosition() const;
    Quaternion4D getWorldRotation() const;
    Vector4 getWorldScale() const;
    
    void setPosition(const Vector4& pos);
    void setRotation(const Quaternion4D& rot);
    void setScale(const Vector4& scl);
    
    void translate(const Vector4& translation);
    void rotate(const Quaternion4D& rotation);
    void scaleBy(const Vector4& scale);
    
    Vector4 forward() const;
    Vector4 right() const;
    Vector4 up() const;
    Vector4 wAxis() const; // Четвертая ось для 4D
    
    // Иерархия
    void setParent(Transform4D* parent);
    Transform4D* getParent() const { return parent; }
    void addChild(Transform4D* child);
    void removeChild(Transform4D* child);
    const std::vector<Transform4D*>& getChildren() const { return children; }
    
private:
    Transform4D* parent;
    std::vector<Transform4D*> children;
    mutable Matrix4 cachedWorldMatrix;
    mutable bool worldMatrixDirty;
    
    void markWorldMatrixDirty();
    void updateWorldMatrix() const;
};

/**
 * @brief Компонент рендеринга для 4D объектов
 */
class MeshRenderer4D : public Component4D {
public:
    std::shared_ptr<Rendering::Mesh4D> mesh;
    std::shared_ptr<Rendering::Shader4D> shader;
    Vector4 color;
    bool castShadows;
    bool receiveShadows;
    
    MeshRenderer4D();
    virtual ~MeshRenderer4D() = default;
    
    void setMesh(std::shared_ptr<Rendering::Mesh4D> mesh);
    void setShader(std::shared_ptr<Rendering::Shader4D> shader);
    void setColor(const Vector4& color);
    
    virtual void render() override;
};

/**
 * @brief Компонент коллайдера для 4D объектов
 */
class Collider4DComponent : public Component4D {
public:
    std::shared_ptr<Physics::Collider4D> collider;
    bool isTrigger;
    
    Collider4DComponent();
    virtual ~Collider4DComponent() = default;
    
    void setCollider(std::shared_ptr<Physics::Collider4D> collider);
    void setTrigger(bool trigger);
    
    bool checkCollision(const Collider4DComponent& other) const;
};

/**
 * @brief Компонент физического тела для 4D объектов
 */
class RigidBody4DComponent : public Component4D {
public:
    std::shared_ptr<Physics::RigidBody4D> rigidBody;
    bool useGravity;
    
    RigidBody4DComponent();
    virtual ~RigidBody4DComponent() = default;
    
    void setRigidBody(std::shared_ptr<Physics::RigidBody4D> body);
    void setUseGravity(bool use);
    
    void addForce(const Vector4& force);
    void addTorque(const Vector4& torque);
    void addImpulse(const Vector4& impulse);
    void addAngularImpulse(const Vector4& impulse);
    
    virtual void update(float deltaTime) override;
};

/**
 * @brief Компонент камеры для 4D пространства
 */
class Camera4DComponent : public Component4D {
public:
    Rendering::Camera4D camera;
    bool isMainCamera;
    float fieldOfView;
    float nearPlane;
    float farPlane;
    
    Camera4DComponent();
    virtual ~Camera4DComponent() = default;
    
    void setMainCamera(bool main);
    void setFieldOfView(float fov);
    void setNearPlane(float near);
    void setFarPlane(float far);
    
    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;
};

/**
 * @brief Компонент системы частиц для 4D
 */
class ParticleSystem4DComponent : public Component4D {
public:
    std::shared_ptr<Physics::ParticleSystem4D> particleSystem;
    bool autoPlay;
    float emissionRate;
    
    ParticleSystem4DComponent();
    virtual ~ParticleSystem4DComponent() = default;
    
    void setParticleSystem(std::shared_ptr<Physics::ParticleSystem4D> system);
    void setAutoPlay(bool play);
    void setEmissionRate(float rate);
    
    void play();
    void stop();
    void emit(int count = 1);
    
    virtual void update(float deltaTime) override;
    virtual void render() override;
};

/**
 * @brief 4D игровой объект
 */
class GameObject4D {
public:
    std::string name;
    std::string tag;
    bool active;
    bool staticObject;
    
    Transform4D* transform;
    std::vector<std::shared_ptr<Component4D>> components;
    
    GameObject4D(const std::string& name = "GameObject4D");
    virtual ~GameObject4D();
    
    // Управление компонентами
    template<typename T>
    T* addComponent();
    template<typename T>
    T* getComponent();
    template<typename T>
    std::vector<T*> getComponents();
    template<typename T>
    void removeComponent(T* component);
    
    // Управление объектом
    void setActive(bool active);
    bool isActive() const { return active; }
    void destroy();
    
    // Обновление
    void start();
    void update(float deltaTime);
    void render();
    void cleanup();
    
    // Поиск объектов
    static GameObject4D* find(const std::string& name);
    static std::vector<GameObject4D*> findAllWithTag(const std::string& tag);
    static GameObject4D* findWithTag(const std::string& tag);
    
    // Создание объектов
    static GameObject4D* create(const std::string& name = "GameObject4D");
    static GameObject4D* createPrimitive(const std::string& type); // "Cube", "Sphere", "Tesseract", etc.
    
private:
    static std::vector<GameObject4D*> allObjects;
    bool started;
    
    void addToAllObjects();
    void removeFromAllObjects();
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
