#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
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
class SphereCollider3D;
class BoxCollider3D;
class PlaneCollider3D;

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
    enum Type { Sphere, Box, Plane };

    struct AABB {
        Math::Vector3 min;
        Math::Vector3 max;

        AABB() : min(0, 0, 0), max(0, 0, 0) {}
        AABB(const Math::Vector3& minPoint, const Math::Vector3& maxPoint)
            : min(minPoint), max(maxPoint) {}

        bool intersects(const AABB& other) const;
    };

    Collider3D(Type type);
    virtual ~Collider3D() = default;

    Type getType() const { return type; }
    bool isTrigger() const { return trigger; }
    void setTrigger(bool isTrigger) { trigger = isTrigger; }
    bool isEnabled() const { return enabled; }
    void setEnabled(bool isEnabled) { enabled = isEnabled; }

    const Math::Vector3& getCenter() const { return center; }
    void setCenter(const Math::Vector3& newCenter) { center = newCenter; }

    const Math::Matrix4& getTransform() const { return transform; }
    void setTransform(const Math::Matrix4& newTransform) { transform = newTransform; }

    virtual bool intersects(const Collider3D& other) const = 0;
    virtual CollisionResult3D checkCollision(const Collider3D& other) const = 0;
    virtual Math::Vector3 getClosestPoint(const Math::Vector3& point) const = 0;
    virtual Math::Vector3 getNormal(const Math::Vector3& point) const = 0;
    virtual float getVolume() const = 0;
    virtual RaycastHit3D raycast(const Math::Vector3& origin,
                                 const Math::Vector3& direction,
                                 float maxDistance) const = 0;
    virtual AABB getBoundingBox() const = 0;

  protected:
    Type type;
    Math::Vector3 center;
    Math::Matrix4 transform;
    bool trigger;
    bool enabled;
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
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    RaycastHit3D raycast(const Math::Vector3& origin,
                         const Math::Vector3& direction,
                         float maxDistance) const override;
    AABB getBoundingBox() const override;

  public:
    bool intersectsSphere(const SphereCollider3D& other) const;
    bool intersectsBox(const BoxCollider3D& other) const;

  private:
    float radius;
};

/**
 * @brief Box коллайдер
 */
class BoxCollider3D : public Collider3D {
  public:
    BoxCollider3D(const Math::Vector3& size = Math::Vector3(1, 1, 1));

    const Math::Vector3& getSize() const { return size; }
    void setSize(const Math::Vector3& newSize) { size = newSize; }

    Math::Vector3 getMin() const;
    Math::Vector3 getMax() const;

    bool intersects(const Collider3D& other) const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    RaycastHit3D raycast(const Math::Vector3& origin,
                         const Math::Vector3& direction,
                         float maxDistance) const override;
    AABB getBoundingBox() const override;

  public:
    bool intersectsBox(const BoxCollider3D& other) const;
    bool intersectsSphere(const SphereCollider3D& other) const;

  private:
    Math::Vector3 size;
};

/**
 * @brief Plane коллайдер
 */
class PlaneCollider3D : public Collider3D {
  public:
    PlaneCollider3D(const Math::Vector3& normal = Math::Vector3(0, 1, 0), float distance = 0.0f);

    const Math::Vector3& getNormalVector() const { return normal; }
    float getDistance() const { return distance; }
    void setPlane(const Math::Vector3& newNormal, float newDistance);

    float distanceToPoint(const Math::Vector3& point) const;
    bool isPointAbove(const Math::Vector3& point) const;

    bool intersects(const Collider3D& other) const override;
    CollisionResult3D checkCollision(const Collider3D& other) const override;
    Math::Vector3 getClosestPoint(const Math::Vector3& point) const override;
    Math::Vector3 getNormal(const Math::Vector3& point) const override;
    float getVolume() const override;
    RaycastHit3D raycast(const Math::Vector3& origin,
                         const Math::Vector3& direction,
                         float maxDistance) const override;
    AABB getBoundingBox() const override;

  private:
    Math::Vector3 normal;
    float distance;
};

/**
 * @brief Физическое тело в 3D
 */
class RigidBody3D {
  public:
    RigidBody3D();
    virtual ~RigidBody3D() = default;

    // Позиция и ориентация
    const Math::Vector3& getPosition() const { return position; }
    void setPosition(const Math::Vector3& pos);

    const Math::Quaternion& getRotation() const { return rotation; }
    void setRotation(const Math::Quaternion& rot);

    // Скорости
    const Math::Vector3& getVelocity() const { return velocity; }
    void setVelocity(const Math::Vector3& vel);

    const Math::Vector3& getAngularVelocity() const { return angularVelocity; }
    void setAngularVelocity(const Math::Vector3& angVel);

    // Массовые свойства
    float getMass() const { return mass; }
    void setMass(float m);

    float getInverseMass() const { return inverseMass; }

    const Math::Vector3& getInertia() const { return inertia; }
    void setInertia(const Math::Vector3& newInertia);
    Math::Vector3 getInverseInertia() const;

    // Применение сил
    void applyForce(const Math::Vector3& newForce);
    void applyForceAtPosition(const Math::Vector3& newForce, const Math::Vector3& worldPosition);
    void applyTorque(const Math::Vector3& newTorque);
    void applyImpulse(const Math::Vector3& impulse);
    void applyAngularImpulse(const Math::Vector3& impulse);

    // Коллайдер
    std::shared_ptr<Collider3D> getCollider() const { return collider; }
    void setCollider(std::shared_ptr<Collider3D> newCollider);

    // Свойства материала
    float getRestitution() const { return restitution; }
    void setRestitution(float rest) { restitution = rest; }

    float getFriction() const { return friction; }
    void setFriction(float fric) { friction = fric; }

    float getDrag() const { return drag; }
    void setDrag(float newDrag) { drag = newDrag; }

    float getAngularDrag() const { return angularDrag; }
    void setAngularDrag(float newAngularDrag) { angularDrag = newAngularDrag; }

    float getGravityScale() const { return gravityScale; }
    void setGravityScale(float scale) { gravityScale = scale; }

    // Кинематическое тело
    bool isKinematic() const { return kinematic; }
    void setKinematic(bool isKinematic) { kinematic = isKinematic; }

    // Физическое обновление
    void update(float deltaTime);
    void clearForces();

    Math::Matrix4 getTransformMatrix() const;

  private:
    // Трансформация
    Math::Vector3 position;
    Math::Quaternion rotation;

    // Динамика
    Math::Vector3 velocity;
    Math::Vector3 angularVelocity;
    Math::Vector3 force;
    Math::Vector3 torque;

    // Массовые свойства
    float mass;
    float inverseMass;
    Math::Vector3 inertia;

    // Коллайдер
    std::shared_ptr<Collider3D> collider;

    // Материал
    float restitution;
    float friction;
    float drag;
    float angularDrag;
    float gravityScale;

    // Флаги
    bool kinematic;

    void integrate(float deltaTime);
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
        Math::Vector3 color;
        float life;
        float size;
        bool active;

        Particle()
            : position(0, 0, 0),
              velocity(0, 0, 0),
              acceleration(0, 0, 0),
              color(1, 1, 1),
              life(0.0f),
              size(1.0f),
              active(false) {}
    };

    ParticleSystem3D(size_t maxParticles = 1000);
    ~ParticleSystem3D() = default;

    void update(float deltaTime);
    void emit(int count = 1);
    void clear();

    // Настройки эмиссии
    void setEmitterPosition(const Math::Vector3& pos) { emitterPosition = pos; }
    void setEmitterVelocity(const Math::Vector3& vel) { emitterVelocity = vel; }
    void setGravity(const Math::Vector3& grav) { gravity = grav; }
    void setEmissionRate(float rate) { emissionRate = rate; }
    void setParticleLife(float life) { particleLife = life; }
    void setParticleSize(float size) { particleSize = size; }
    void setColorRange(const Math::Vector3& start, const Math::Vector3& end);

    // Управление
    void play() {
        playing = true;
        paused = false;
    }
    void pause() { paused = true; }
    void stop() {
        playing = false;
        paused = false;
    }

    const std::vector<Particle>& getParticles() const { return particles; }
    int getActiveParticleCount() const;

  private:
    std::vector<Particle> particles;
    Math::Vector3 emitterPosition;
    Math::Vector3 emitterVelocity;
    Math::Vector3 gravity;
    float emissionRate;
    float particleLife;
    float particleSize;
    Math::Vector3 startColor;
    Math::Vector3 endColor;
    bool playing;
    bool paused;
    float emissionTimer;

    void updateParticle(Particle& particle, float deltaTime);
    Particle* getInactiveParticle();
};

/**
 * @brief Физический мир для 3D
 */
class PhysicsWorld3D {
  public:
    PhysicsWorld3D();
    ~PhysicsWorld3D() = default;

    // Управление миром
    void update(float deltaTime);
    void step(float deltaTime);
    void clear();

    // Управление телами и коллайдерами
    void addRigidBody(std::shared_ptr<RigidBody3D> body);
    void removeRigidBody(std::shared_ptr<RigidBody3D> body);
    void addCollider(std::shared_ptr<Collider3D> collider);
    void removeCollider(std::shared_ptr<Collider3D> collider);

    // Обнаружение столкновений
    std::vector<CollisionResult3D> detectCollisions();
    CollisionResult3D checkCollision(const Collider3D& a, const Collider3D& b);

    // Raycast
    RaycastHit3D raycast(const Math::Vector3& origin,
                         const Math::Vector3& direction,
                         float maxDistance = 1000.0f);
    std::vector<RaycastHit3D> raycastAll(const Math::Vector3& origin,
                                         const Math::Vector3& direction,
                                         float maxDistance = 1000.0f);

    // Настройки мира
    const Math::Vector3& getGravity() const { return gravity; }
    void setGravity(const Math::Vector3& grav) { gravity = grav; }

    float getTimeStep() const { return timeStep; }
    void setTimeStep(float step) { timeStep = step; }

    int getIterations() const { return iterations; }
    void setIterations(int iter) { iterations = iter; }

    // Статистика
    int getRigidBodyCount() const { return static_cast<int>(rigidBodies.size()); }
    int getColliderCount() const { return static_cast<int>(colliders.size()); }

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

}  // namespace Physics
}  // namespace HyperEngine