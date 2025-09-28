#pragma once

#include <memory>
#include <vector>
#include "../Math/Matrix4.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"

namespace HyperEngine {
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
        : hasCollision(false),
          contactPoint(0, 0, 0),
          contactNormal(0, 1, 0),
          penetrationDepth(0.0f) {}
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

    RaycastHit3D() : hasHit(false), point(0, 0, 0), normal(0, 1, 0), distance(0.0f) {}
};

/**
 * @brief Базовый класс для 3D коллайдеров
 */
class Collider3D {
  public:
    enum Type {
        BOX,
        SPHERE,
        CAPSULE,
        MESH
    };

    Collider3D(Type type) : type(type), isTrigger(false) {}
    virtual ~Collider3D() = default;

    Type getType() const { return type; }
    bool getIsTrigger() const { return isTrigger; }
    void setIsTrigger(bool trigger) { isTrigger = trigger; }

    virtual bool intersects(const Collider3D& other) const = 0;
    virtual CollisionResult3D checkCollision(const Collider3D& other) const = 0;
    virtual Math::Vector3 getClosestPoint(const Math::Vector3& point) const = 0;

  protected:
    Type type;
    bool isTrigger;
};

/**
 * @brief Box коллайдер
 */
class BoxCollider3D : public Collider3D {
  public:
    BoxCollider3D(const Math::Vector3& size = Math::Vector3(1, 1, 1));
    
    const Math::Vector3& getSize() const { return size; }
    void setSize(const Math::Vector3& newSize) { size = newSize; }

    bool intersects(const Collider3D& other) const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;

  private:
    Math::Vector3 size;
};

/**
 * @brief Sphere коллайдер
 */
class SphereCollider3D : public Collider3D {
  public:
    SphereCollider3D(float radius = 1.0f);
    
    float getRadius() const { return radius; }
    void setRadius(float newRadius) { radius = newRadius; }

    bool intersects(const Collider3D& other) const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;

  private:
    float radius;
};

/**
 * @brief Физическое тело в 3D
 */
class RigidBody3D {
  public:
    enum BodyType {
        STATIC,
        KINEMATIC,
        DYNAMIC
    };

    RigidBody3D(BodyType type = DYNAMIC);
    virtual ~RigidBody3D() = default;

    // Основные свойства
    BodyType getBodyType() const { return bodyType; }
    void setBodyType(BodyType type) { bodyType = type; }

    float getMass() const { return mass; }
    void setMass(float newMass);

    // Позиция и ориентация
    const Math::Vector3& getPosition() const { return position; }
    void setPosition(const Math::Vector3& pos) { position = pos; }

    const Math::Quaternion& getRotation() const { return rotation; }
    void setRotation(const Math::Quaternion& rot) { rotation = rot; }

    // Скорости
    const Math::Vector3& getVelocity() const { return velocity; }
    void setVelocity(const Math::Vector3& vel) { velocity = vel; }

    const Math::Vector3& getAngularVelocity() const { return angularVelocity; }
    void setAngularVelocity(const Math::Vector3& angVel) { angularVelocity = angVel; }

    // Применение сил
    void addForce(const Math::Vector3& force);
    void addTorque(const Math::Vector3& torque);
    void addImpulse(const Math::Vector3& impulse);
    void addAngularImpulse(const Math::Vector3& impulse);

    // Коллайдеры
    void addCollider(std::shared_ptr<Collider3D> collider);
    void removeCollider(std::shared_ptr<Collider3D> collider);
    const std::vector<std::shared_ptr<Collider3D>>& getColliders() const { return colliders; }

    // Физическое обновление
    void integrate(float deltaTime);
    void clearForces();

    // Свойства материала
    float getRestitution() const { return restitution; }
    void setRestitution(float rest) { restitution = rest; }

    float getFriction() const { return friction; }
    void setFriction(float fric) { friction = fric; }

    float getDrag() const { return drag; }
    void setDrag(float newDrag) { drag = newDrag; }

    float getAngularDrag() const { return angularDrag; }
    void setAngularDrag(float newAngularDrag) { angularDrag = newAngularDrag; }

    // Ограничения
    bool getFreezePositionX() const { return freezePositionX; }
    void setFreezePositionX(bool freeze) { freezePositionX = freeze; }

    bool getFreezePositionY() const { return freezePositionY; }
    void setFreezePositionY(bool freeze) { freezePositionY = freeze; }

    bool getFreezePositionZ() const { return freezePositionZ; }
    void setFreezePositionZ(bool freeze) { freezePositionZ = freeze; }

    bool getFreezeRotationX() const { return freezeRotationX; }
    void setFreezeRotationX(bool freeze) { freezeRotationX = freeze; }

    bool getFreezeRotationY() const { return freezeRotationY; }
    void setFreezeRotationY(bool freeze) { freezeRotationY = freeze; }

    bool getFreezeRotationZ() const { return freezeRotationZ; }
    void setFreezeRotationZ(bool freeze) { freezeRotationZ = freeze; }

  private:
    BodyType bodyType;
    float mass;
    float invMass;

    // Трансформация
    Math::Vector3 position;
    Math::Quaternion rotation;

    // Динамика
    Math::Vector3 velocity;
    Math::Vector3 angularVelocity;
    Math::Vector3 force;
    Math::Vector3 torque;

    // Коллайдеры
    std::vector<std::shared_ptr<Collider3D>> colliders;

    // Материал
    float restitution;
    float friction;
    float drag;
    float angularDrag;

    // Ограничения
    bool freezePositionX, freezePositionY, freezePositionZ;
    bool freezeRotationX, freezeRotationY, freezeRotationZ;

    void updateInverseMass();
};

/**
 * @brief Система частиц для 3D
 */
class ParticleSystem3D {
  public:
    struct Particle {
        Math::Vector3 position;
        Math::Vector3 velocity;
        Math::Vector3 acceleration;
        float life;
        float maxLife;
        Math::Vector3 color;
        float size;
    };

    ParticleSystem3D(int maxParticles = 1000);
    ~ParticleSystem3D() = default;

    void update(float deltaTime);
    void emit(int count = 1);
    void clear();

    // Настройки эмиссии
    void setEmissionRate(float rate) { emissionRate = rate; }
    void setLifetime(float min, float max) { minLifetime = min; maxLifetime = max; }
    void setVelocity(const Math::Vector3& min, const Math::Vector3& max) { minVelocity = min; maxVelocity = max; }
    void setSize(float min, float max) { minSize = min; maxSize = max; }
    void setColor(const Math::Vector3& color) { particleColor = color; }

    const std::vector<Particle>& getParticles() const { return particles; }
    int getActiveParticleCount() const { return activeParticles; }

  private:
    std::vector<Particle> particles;
    int maxParticles;
    int activeParticles;

    // Параметры эмиссии
    float emissionRate;
    float minLifetime, maxLifetime;
    Math::Vector3 minVelocity, maxVelocity;
    float minSize, maxSize;
    Math::Vector3 particleColor;

    // Внутренние переменные
    float emissionTimer;

    void createParticle(int index);
    void updateParticle(Particle& particle, float deltaTime);
};

/**
 * @brief Физический мир для 3D
 */
class PhysicsWorld3D {
  public:
    PhysicsWorld3D();
    ~PhysicsWorld3D() = default;

    // Управление миром
    void step(float deltaTime);
    void clear();

    // Управление телами
    void addRigidBody(std::shared_ptr<RigidBody3D> body);
    void removeRigidBody(std::shared_ptr<RigidBody3D> body);

    // Raycast
    RaycastHit3D raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance = 1000.0f);
    std::vector<RaycastHit3D> raycastAll(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance = 1000.0f);

    // Настройки мира
    const Math::Vector3& getGravity() const { return gravity; }
    void setGravity(const Math::Vector3& grav) { gravity = grav; }

    int getIterations() const { return iterations; }
    void setIterations(int iter) { iterations = iter; }

    // Статистика
    int getRigidBodyCount() const { return static_cast<int>(rigidBodies.size()); }
    int getCollisionCount() const { return collisionCount; }

  private:
    std::vector<std::shared_ptr<RigidBody3D>> rigidBodies;
    Math::Vector3 gravity;
    int iterations;
    int collisionCount;

    void detectCollisions();
    void resolveCollisions();
    void integrateRigidBodies(float deltaTime);
    CollisionResult3D checkCollision(std::shared_ptr<RigidBody3D> bodyA, std::shared_ptr<RigidBody3D> bodyB);
};

}  // namespace Physics
}  // namespace HyperEngine
