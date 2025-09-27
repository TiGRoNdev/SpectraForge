#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include <memory>
#include <vector>

namespace Engine3D {
namespace Physics {

// Forward declarations
class RigidBody3D;
class Collider3D;
class PhysicsWorld3D;

/**
 * @brief Результат столкновения в 3D
 */
struct CollisionResult3D {
    bool hasCollision;
    Math::Vector3 contactPoint;
    Math::Vector3 contactNormal;
    float penetrationDepth;
    std::shared_ptr<RigidBody3D> bodyA;
    std::shared_ptr<RigidBody3D> bodyB;
    
    CollisionResult3D() 
        : hasCollision(false)
        , contactPoint(0, 0, 0)
        , contactNormal(0, 1, 0)
        , penetrationDepth(0.0f) {}
};

/**
 * @brief Результат raycast в 3D
 */
struct RaycastHit3D {
    bool hasHit;
    Math::Vector3 point;
    Math::Vector3 normal;
    float distance;
    std::shared_ptr<Collider3D> collider;
    std::shared_ptr<RigidBody3D> rigidBody;
    
    RaycastHit3D() 
        : hasHit(false)
        , point(0, 0, 0)
        , normal(0, 1, 0)
        , distance(0.0f) {}
};

/**
 * @brief Базовый класс для 3D коллайдеров
 */
class Collider3D {
public:
    enum class Type {
        Sphere,
        Box,
        Plane,
        Capsule,
        Mesh
    };
    
    Collider3D(Type type);
    virtual ~Collider3D() = default;
    
    // Основные методы
    virtual bool intersects(const Collider3D& other) const = 0;
    virtual Math::Vector3 getClosestPoint(const Math::Vector3& point) const = 0;
    virtual Math::Vector3 getNormal(const Math::Vector3& point) const = 0;
    virtual float getVolume() const = 0;
    virtual CollisionResult3D checkCollision(const Collider3D& other) const = 0;
    
    // Raycast
    virtual RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance = 1000.0f) const = 0;
    
    // Настройка
    void setCenter(const Math::Vector3& centerPos) { this->center = centerPos; }
    void setTransform(const Math::Matrix4& transformMatrix) { this->transform = transformMatrix; }
    void setTrigger(bool isTrigger) { this->trigger = isTrigger; }
    void setEnabled(bool isEnabled) { this->enabled = isEnabled; }
    
    // Геттеры
    Type getType() const { return type; }
    const Math::Vector3& getCenter() const { return center; }
    const Math::Matrix4& getTransform() const { return transform; }
    bool isTrigger() const { return trigger; }
    bool isEnabled() const { return enabled; }
    
    // Bounding box
    struct AABB {
        Math::Vector3 min, max;
        
        AABB() : min(0, 0, 0), max(0, 0, 0) {}
        AABB(const Math::Vector3& min, const Math::Vector3& max) : min(min), max(max) {}
        
        bool intersects(const AABB& other) const;
        Math::Vector3 getCenter() const { return (min + max) * 0.5f; }
        Math::Vector3 getSize() const { return max - min; }
    };
    
    virtual AABB getBoundingBox() const = 0;

protected:
    Type type;
    Math::Vector3 center;
    Math::Matrix4 transform;
    bool trigger;
    bool enabled;
};

/**
 * @brief Сферический коллайдер
 */
class SphereCollider3D : public Collider3D {
public:
    SphereCollider3D(float radius = 1.0f);
    
    // Переопределения базового класса
    bool intersects(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance) const override;
    AABB getBoundingBox() const override;
    
    // Специфичные методы
    void setRadius(float radiusValue) { this->radius = radiusValue; }
    float getRadius() const { return radius; }
    
    // Специализированные проверки столкновений
    bool intersectsSphere(const SphereCollider3D& other) const;
    bool intersectsBox(const class BoxCollider3D& other) const;

private:
    float radius;
};

/**
 * @brief Коробочный коллайдер (AABB)
 */
class BoxCollider3D : public Collider3D {
public:
    BoxCollider3D(const Math::Vector3& size = Math::Vector3(1, 1, 1));
    
    // Переопределения базового класса
    bool intersects(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance) const override;
    AABB getBoundingBox() const override;
    
    // Специфичные методы
    void setSize(const Math::Vector3& boxSize) { this->size = boxSize; }
    const Math::Vector3& getSize() const { return size; }
    Math::Vector3 getMin() const;
    Math::Vector3 getMax() const;
    
    // Специализированные проверки столкновений
    bool intersectsBox(const BoxCollider3D& other) const;
    bool intersectsSphere(const SphereCollider3D& other) const;

private:
    Math::Vector3 size;
};

/**
 * @brief Плоскостной коллайдер
 */
class PlaneCollider3D : public Collider3D {
public:
    PlaneCollider3D(const Math::Vector3& normal = Math::Vector3(0, 1, 0), float distance = 0.0f);
    
    // Переопределения базового класса
    bool intersects(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance) const override;
    AABB getBoundingBox() const override;
    
    // Специфичные методы
    void setPlane(const Math::Vector3& normal, float distance);
    const Math::Vector3& getNormal() const { return normal; }
    float getDistance() const { return distance; }
    float distanceToPoint(const Math::Vector3& point) const;
    bool isPointAbove(const Math::Vector3& point) const;

private:
    Math::Vector3 normal;
    float distance;
};

/**
 * @brief 3D физическое тело
 */
class RigidBody3D {
public:
    RigidBody3D();
    virtual ~RigidBody3D() = default;
    
    // Обновление
    void update(float deltaTime);
    
    // Применение сил
    void applyForce(const Math::Vector3& force);
    void applyForceAtPosition(const Math::Vector3& force, const Math::Vector3& position);
    void applyTorque(const Math::Vector3& torque);
    void applyImpulse(const Math::Vector3& impulse);
    void applyAngularImpulse(const Math::Vector3& impulse);
    
    // Позиция и ориентация
    void setPosition(const Math::Vector3& pos);
    void setRotation(const Math::Quaternion& rot);
    const Math::Vector3& getPosition() const { return position; }
    const Math::Quaternion& getRotation() const { return rotation; }
    Math::Matrix4 getTransformMatrix() const;
    
    // Скорости
    void setVelocity(const Math::Vector3& vel);
    void setAngularVelocity(const Math::Vector3& angVel);
    const Math::Vector3& getVelocity() const { return velocity; }
    const Math::Vector3& getAngularVelocity() const { return angularVelocity; }
    
    // Массовые характеристики
    void setMass(float m);
    void setInertia(const Math::Vector3& inertia);
    float getMass() const { return mass; }
    float getInverseMass() const { return inverseMass; }
    const Math::Vector3& getInertia() const { return inertia; }
    Math::Vector3 getInverseInertia() const;
    
    // Настройки
    void setKinematic(bool isKinematic) { this->kinematic = isKinematic; }
    void setGravityScale(float scale) { this->gravityScale = scale; }
    void setDrag(float dragValue) { this->drag = dragValue; }
    void setAngularDrag(float angularDragValue) { this->angularDrag = angularDragValue; }
    void setRestitution(float restitutionValue) { this->restitution = restitutionValue; }
    void setFriction(float frictionValue) { this->friction = frictionValue; }
    
    bool isKinematic() const { return kinematic; }
    float getGravityScale() const { return gravityScale; }
    float getDrag() const { return drag; }
    float getAngularDrag() const { return angularDrag; }
    float getRestitution() const { return restitution; }
    float getFriction() const { return friction; }
    
    // Коллайдер
    void setCollider(std::shared_ptr<Collider3D> collider);
    std::shared_ptr<Collider3D> getCollider() const { return collider; }

private:
    // Позиция и ориентация
    Math::Vector3 position;
    Math::Quaternion rotation;
    
    // Скорости
    Math::Vector3 velocity;
    Math::Vector3 angularVelocity;
    
    // Силы и моменты
    Math::Vector3 force;
    Math::Vector3 torque;
    
    // Массовые характеристики
    float mass;
    float inverseMass;
    Math::Vector3 inertia;
    
    // Настройки
    bool kinematic;
    float gravityScale;
    float drag;
    float angularDrag;
    float restitution;
    float friction;
    
    // Коллайдер
    std::shared_ptr<Collider3D> collider;
    
    void integrate(float deltaTime);
    void clearForces();
};

/**
 * @brief Система частиц в 3D
 */
class ParticleSystem3D {
public:
    struct Particle {
        Math::Vector3 position;
        Math::Vector3 velocity;
        Math::Vector3 acceleration;
        Math::Vector3 color;
        float life;
        float size;
        bool active;
        
        Particle() : position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0),
                    color(1, 1, 1), life(1.0f), size(1.0f), active(false) {}
    };
    
    ParticleSystem3D(size_t maxParticles = 1000);
    virtual ~ParticleSystem3D() = default;
    
    // Управление системой
    void update(float deltaTime);
    void emit(int count = 1);
    void clear();
    void play() { playing = true; }
    void stop() { playing = false; }
    void pause() { paused = true; }
    void resume() { paused = false; }
    
    // Настройка эмиттера
    void setEmitterPosition(const Math::Vector3& position) { emitterPosition = position; }
    void setEmitterVelocity(const Math::Vector3& velocity) { emitterVelocity = velocity; }
    void setGravity(const Math::Vector3& gravityVector) { this->gravity = gravityVector; }
    void setEmissionRate(float rate) { emissionRate = rate; }
    void setParticleLife(float life) { particleLife = life; }
    void setParticleSize(float size) { particleSize = size; }
    void setColorRange(const Math::Vector3& start, const Math::Vector3& end);
    
    // Статистика
    int getActiveParticleCount() const;
    size_t getMaxParticles() const { return particles.size(); }
    bool isPlaying() const { return playing; }
    bool isPaused() const { return paused; }
    
    // Доступ к частицам для рендеринга
    const std::vector<Particle>& getParticles() const { return particles; }

private:
    std::vector<Particle> particles;
    
    // Настройки эмиттера
    Math::Vector3 emitterPosition;
    Math::Vector3 emitterVelocity;
    Math::Vector3 gravity;
    float emissionRate;
    float particleLife;
    float particleSize;
    Math::Vector3 startColor;
    Math::Vector3 endColor;
    
    // Состояние системы
    bool playing;
    bool paused;
    float emissionTimer;
    
    void updateParticle(Particle& particle, float deltaTime);
    Particle* getInactiveParticle();
};

/**
 * @brief 3D физический мир
 */
class PhysicsWorld3D {
public:
    PhysicsWorld3D();
    virtual ~PhysicsWorld3D() = default;
    
    // Управление объектами
    void addRigidBody(std::shared_ptr<RigidBody3D> body);
    void addCollider(std::shared_ptr<Collider3D> collider);
    void removeRigidBody(std::shared_ptr<RigidBody3D> body);
    void removeCollider(std::shared_ptr<Collider3D> collider);
    void clear();
    
    // Обновление симуляции
    void update(float deltaTime);
    void step(float deltaTime);
    
    // Обнаружение столкновений
    std::vector<CollisionResult3D> detectCollisions();
    CollisionResult3D checkCollision(const Collider3D& a, const Collider3D& b);
    
    // Raycasting
    RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance = 1000.0f);
    std::vector<RaycastHit3D> raycastAll(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance = 1000.0f);
    
    // Настройки мира
    void setGravity(const Math::Vector3& gravityVector) { this->gravity = gravityVector; }
    void setTimeStep(float step) { timeStep = step; }
    void setIterations(int iterationCount) { this->iterations = iterationCount; }
    
    const Math::Vector3& getGravity() const { return gravity; }
    float getTimeStep() const { return timeStep; }
    int getIterations() const { return iterations; }
    
    // Статистика
    size_t getRigidBodyCount() const { return rigidBodies.size(); }
    size_t getColliderCount() const { return colliders.size(); }

private:
    std::vector<std::shared_ptr<RigidBody3D>> rigidBodies;
    std::vector<std::shared_ptr<Collider3D>> colliders;
    
    Math::Vector3 gravity;
    float timeStep;
    int iterations;
    
    void applyGravity(float deltaTime);
    void integrateVelocities(float deltaTime);
    void resolveCollisions();
    void integratePositions(float deltaTime);
};

} // namespace Physics
} // namespace Engine3D
