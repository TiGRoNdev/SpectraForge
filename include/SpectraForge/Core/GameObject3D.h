
#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"
#include "Component.h"
#include "Interfaces.h"
#include "Transform3D.h"

namespace SpectraForge {

// Forward declarations
namespace Physics {
class Collider3D;
class RigidBody3D;
class ParticleSystem3D;
}  // namespace Physics

namespace Rendering {
class Mesh3D;
class Shader3D;
class Camera3D;
}  // namespace Rendering

namespace Core {

/**
 * @brief Компонент рендеринга для 3D объектов
 */
class MeshRenderer3D : public RenderableComponent {
  public:
    MeshRenderer3D();
    ~MeshRenderer3D() override = default;

    // Интерфейс Component
    std::string getComponentType() const override { return "MeshRenderer3D"; }
    void render() override;

    // Конфигурация рендерера
    void setMesh(std::shared_ptr<Rendering::Mesh3D> newMesh);
    void setShader(std::shared_ptr<Rendering::Shader3D> newShader);
    void setColor(const Math::Vector3& newColor);

    // Геттеры
    std::shared_ptr<Rendering::Mesh3D> getMesh() const { return mesh; }
    std::shared_ptr<Rendering::Shader3D> getShader() const { return shader; }
    const Math::Vector3& getColor() const { return color; }

  private:
    std::shared_ptr<Rendering::Mesh3D> mesh;
    std::shared_ptr<Rendering::Shader3D> shader;
    Math::Vector3 color;
    bool castShadows;
    bool receiveShadows;
};

/**
 * @brief Компонент коллайдера для 3D объектов
 */
class Collider3DComponent : public Component3D {
  public:
    Collider3DComponent();
    ~Collider3DComponent() override = default;

    // Интерфейс Component
    std::string getComponentType() const override { return "Collider3DComponent"; }

    // Конфигурация коллайдера
    void setCollider(std::shared_ptr<Physics::Collider3D> newCollider);
    void setTrigger(bool newTrigger);

    // Обнаружение столкновений
    bool checkCollision(const Collider3DComponent& other) const;

    // Геттеры
    std::shared_ptr<Physics::Collider3D> getCollider() const { return collider; }
    bool isTrigger() const { return trigger; }

  private:
    std::shared_ptr<Physics::Collider3D> collider;
    bool trigger;
};

/**
 * @brief Компонент физического тела для 3D объектов
 */
class RigidBody3DComponent : public UpdatableComponent {
  public:
    RigidBody3DComponent();
    ~RigidBody3DComponent() override = default;

    // Интерфейс Component
    std::string getComponentType() const override { return "RigidBody3DComponent"; }
    void update(float deltaTime) override;

    // Конфигурация физики
    void setRigidBody(std::shared_ptr<Physics::RigidBody3D> body);
    void setUseGravity(bool use);

    // Применение сил
    void addForce(const Math::Vector3& force);
    void addTorque(const Math::Vector3& torque);
    void addImpulse(const Math::Vector3& impulse);
    void addAngularImpulse(const Math::Vector3& impulse);

    // Геттеры
    std::shared_ptr<Physics::RigidBody3D> getRigidBody() const { return rigidBody; }
    bool getUseGravity() const { return useGravity; }

  private:
    std::shared_ptr<Physics::RigidBody3D> rigidBody;
    bool useGravity;
};

/**
 * @brief Компонент камеры для 3D пространства
 */
class Camera3DComponent : public Component3D {
  public:
    Camera3DComponent();
    ~Camera3DComponent() override = default;

    // Интерфейс Component
    std::string getComponentType() const override { return "Camera3DComponent"; }

    // Конфигурация камеры
    void setMainCamera(bool main);
    void setFieldOfView(float fov);
    void setNearPlane(float near);
    void setFarPlane(float far);

    // Геттеры матриц
    Math::Matrix4 getViewMatrix() const;
    Math::Matrix4 getProjectionMatrix() const;
    Math::Matrix4 getViewProjectionMatrix() const;

    // Геттеры камеры
    Rendering::Camera3D& getCamera() { return *camera; }
    const Rendering::Camera3D& getCamera() const { return *camera; }
    bool getIsMainCamera() const { return mainCamera; }

  private:
    std::shared_ptr<Rendering::Camera3D> camera;
    bool mainCamera;
    float fieldOfView;
    float nearPlane;
    float farPlane;
};

/**
 * @brief Компонент системы частиц для 3D
 */
class ParticleSystem3DComponent : public UpdatableRenderableComponent {
  public:
    ParticleSystem3DComponent();
    ~ParticleSystem3DComponent() override = default;

    // Интерфейс Component
    std::string getComponentType() const override { return "ParticleSystem3DComponent"; }
    void update(float deltaTime) override;
    void render() override;

    // Конфигурация системы частиц
    void setParticleSystem(std::shared_ptr<Physics::ParticleSystem3D> system);
    void setAutoPlay(bool play);
    void setEmissionRate(float rate);

    // Управление системой частиц
    void play();
    void stop();
    void emit(int count = 1);

    // Геттеры
    std::shared_ptr<Physics::ParticleSystem3D> getParticleSystem() const { return particleSystem; }
    bool getAutoPlay() const { return autoPlay; }
    float getEmissionRate() const { return emissionRate; }

  private:
    std::shared_ptr<Physics::ParticleSystem3D> particleSystem;
    bool autoPlay;
    float emissionRate;
};

/**
 * @brief 3D игровой объект, следующий принципу единственной ответственности и предоставляющий
 * чистый интерфейс
 */
class GameObject3D : public ILifecycle {
  public:
    explicit GameObject3D(const std::string& name = "GameObject3D");
    ~GameObject3D() override;

    // Реализация ILifecycle
    void start() override;
    void cleanup() override;

    // Идентификация объекта
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { this->name = newName; }
    const std::string& getTag() const { return tag; }
    void setTag(const std::string& newTag) { this->tag = newTag; }

    // Управление состоянием объекта
    void setActive(bool newActive);
    bool isActive() const { return active; }
    bool isStatic() const { return staticObject; }
    void setStatic(bool newIsStatic) { staticObject = newIsStatic; }
    void destroy();

    // Доступ к трансформации
    Transform3D* getTransform() { return transform.get(); }
    const Transform3D* getTransform() const { return transform.get(); }

    // Управление компонентами
    template <typename T>
    T* addComponent();
    template <typename T>
    T* getComponent();
    template <typename T>
    std::vector<T*> getComponents();
    template <typename T>
    void removeComponent(T* component);
    void removeComponent(const std::string& componentType);

    // Системные обновления
    void update(float deltaTime);
    void render();

    // Статическое управление объектами
    static GameObject3D* find(const std::string& name);
    static std::vector<GameObject3D*> findAllWithTag(const std::string& tag);
    static GameObject3D* findWithTag(const std::string& tag);
    static GameObject3D* create(const std::string& name = "GameObject3D");
    static GameObject3D* createPrimitive(const std::string& type);

    // Управление реестром объектов
    static void clearAllObjects();
    static const std::vector<GameObject3D*>& getAllObjects();

  private:
    // Свойства объекта
    std::string name;
    std::string tag;
    bool active;
    bool staticObject;
    bool started;

    // Компоненты
    std::shared_ptr<Transform3D> transform;
    std::vector<std::shared_ptr<Component3D>> components;

    // Статический реестр
    static std::vector<GameObject3D*> allObjects;

    // Вспомогательные методы
    void addToAllObjects();
    void removeFromAllObjects();
    void updateComponents(float deltaTime);
    void renderComponents();
};

// Шаблонные методы
template <typename T>
T* GameObject3D::addComponent() {
    static_assert(std::is_base_of<Component3D, T>::value, "T must inherit from Component3D");

    auto component = std::make_shared<T>();
    component->setGameObject(this);
    components.push_back(component);
    return component.get();
}

template <typename T>
T* GameObject3D::getComponent() {
    static_assert(std::is_base_of<Component3D, T>::value, "T must inherit from Component3D");

    for (auto& component : components) {
        if (auto* casted = dynamic_cast<T*>(component.get())) {
            return casted;
        }
    }
    return nullptr;
}

template <typename T>
std::vector<T*> GameObject3D::getComponents() {
    static_assert(std::is_base_of<Component3D, T>::value, "T must inherit from Component3D");

    std::vector<T*> result;
    for (auto& component : components) {
        if (auto* casted = dynamic_cast<T*>(component.get())) {
            result.push_back(casted);
        }
    }
    return result;
}

template <typename T>
void GameObject3D::removeComponent(T* component) {
    static_assert(std::is_base_of<Component3D, T>::value, "T must inherit from Component3D");

    components.erase(std::remove_if(components.begin(),
                                    components.end(),
                                    [component](const std::shared_ptr<Component3D>& comp) {
                                        return comp.get() == component;
                                    }),
                     components.end());
}

}  // namespace Core
}  // namespace SpectraForge
