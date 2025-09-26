#pragma once

#include "../Math/Vector4.h"
#include "../Math/Matrix4.h"
#include <vector>
#include <memory>

namespace Engine4D {
namespace Physics {

/**
 * @brief 4D физическое тело
 */
class RigidBody4D {
public:
    Vector4 position;          // Позиция в 4D пространстве
    Vector4 velocity;          // Скорость
    Vector4 acceleration;      // Ускорение
    Vector4 rotation;          // Поворот (углы Эйлера для 6 плоскостей)
    Vector4 angularVelocity;   // Угловая скорость
    Vector4 angularAcceleration; // Угловое ускорение
    
    float mass;                // Масса
    float restitution;         // Коэффициент восстановления (упругость)
    float friction;            // Коэффициент трения
    bool isStatic;             // Статическое тело
    bool isKinematic;          // Кинематическое тело
    
    RigidBody4D();
    virtual ~RigidBody4D() = default;
    
    void update(float deltaTime);
    void applyForce(const Vector4& force);
    void applyTorque(const Vector4& torque);
    void applyImpulse(const Vector4& impulse);
    void applyAngularImpulse(const Vector4& impulse);
    
    // Геттеры и сеттеры
    Vector4 getPosition() const { return position; }
    void setPosition(const Vector4& pos) { position = pos; }
    Vector4 getVelocity() const { return velocity; }
    void setVelocity(const Vector4& vel) { velocity = vel; }
    float getMass() const { return mass; }
    void setMass(float m) { mass = m; }
};

/**
 * @brief 4D коллайдер
 */
class Collider4D {
public:
    enum class Type {
        Sphere,     // 4D сфера
        Box,        // 4D параллелепипед (гиперкуб)
        Plane,      // 4D плоскость
        Mesh        // Произвольный 4D меш
    };
    
    Type type;
    Vector4 center;            // Центр коллайдера
    Vector4 size;              // Размеры (для Box)
    float radius;              // Радиус (для Sphere)
    Vector4 normal;            // Нормаль (для Plane)
    float distance;            // Расстояние от начала координат (для Plane)
    
    Collider4D(Type t);
    virtual ~Collider4D() = default;
    
    virtual bool intersects(const Collider4D& other) const = 0;
    virtual Vector4 getClosestPoint(const Vector4& point) const = 0;
    virtual Vector4 getNormal(const Vector4& point) const = 0;
    virtual float getVolume() const = 0;
    
    // Трансформации
    void setCenter(const Vector4& center);
    void setSize(const Vector4& size);
    void setRadius(float radius);
    void setPlane(const Vector4& normal, float distance);
};

/**
 * @brief 4D сферический коллайдер
 */
class SphereCollider4D : public Collider4D {
public:
    SphereCollider4D(float radius = 1.0f);
    
    bool intersects(const Collider4D& other) const override;
    Vector4 getClosestPoint(const Vector4& point) const override;
    Vector4 getNormal(const Vector4& point) const override;
    float getVolume() const override;
};

/**
 * @brief 4D боксовый коллайдер
 */
class BoxCollider4D : public Collider4D {
public:
    BoxCollider4D(const Vector4& size = Vector4::one());
    
    bool intersects(const Collider4D& other) const override;
    Vector4 getClosestPoint(const Vector4& point) const override;
    Vector4 getNormal(const Vector4& point) const override;
    float getVolume() const override;
    
    // AABB (Axis-Aligned Bounding Box) для 4D
    Vector4 getMin() const;
    Vector4 getMax() const;
    bool intersectsAABB(const BoxCollider4D& other) const;
};

/**
 * @brief 4D плоскостной коллайдер
 */
class PlaneCollider4D : public Collider4D {
public:
    PlaneCollider4D(const Vector4& normal = Vector4::unitW(), float distance = 0.0f);
    
    bool intersects(const Collider4D& other) const override;
    Vector4 getClosestPoint(const Vector4& point) const override;
    Vector4 getNormal(const Vector4& point) const override;
    float getVolume() const override;
    
    float distanceToPoint(const Vector4& point) const;
    bool isPointAbove(const Vector4& point) const;
};

/**
 * @brief Результат столкновения
 */
struct CollisionResult4D {
    bool hasCollision;         // Есть ли столкновение
    Vector4 contactPoint;      // Точка контакта
    Vector4 contactNormal;     // Нормаль контакта
    float penetrationDepth;    // Глубина проникновения
    Vector4 relativeVelocity;  // Относительная скорость
    float restitution;         // Коэффициент восстановления
    float friction;            // Коэффициент трения
};

/**
 * @brief 4D физический мир
 */
class PhysicsWorld4D {
public:
    std::vector<std::shared_ptr<RigidBody4D>> bodies;
    std::vector<std::shared_ptr<Collider4D>> colliders;
    
    Vector4 gravity;           // Гравитация в 4D
    float timeStep;            // Шаг времени
    int maxIterations;         // Максимальное количество итераций
    
    PhysicsWorld4D();
    ~PhysicsWorld4D() = default;
    
    void addBody(std::shared_ptr<RigidBody4D> body);
    void addCollider(std::shared_ptr<Collider4D> collider);
    void removeBody(std::shared_ptr<RigidBody4D> body);
    void removeCollider(std::shared_ptr<Collider4D> collider);
    
    void update(float deltaTime);
    void step(float deltaTime);
    
    // Обнаружение столкновений
    std::vector<CollisionResult4D> detectCollisions();
    CollisionResult4D checkCollision(const Collider4D& a, const Collider4D& b);
    
    // Raycasting в 4D
    struct RaycastHit4D {
        bool hasHit;
        Vector4 point;
        Vector4 normal;
        float distance;
        std::shared_ptr<Collider4D> collider;
    };
    
    RaycastHit4D raycast(const Vector4& origin, const Vector4& direction, float maxDistance = 1000.0f);
    
    // Утилиты
    void setGravity(const Vector4& gravity);
    void setTimeStep(float step);
    void clear();
    
private:
    // Вспомогательные методы для обнаружения столкновений
    CollisionResult4D checkSphereSphere(const SphereCollider4D& a, const SphereCollider4D& b);
    CollisionResult4D checkSphereBox(const SphereCollider4D& sphere, const BoxCollider4D& box);
    CollisionResult4D checkBoxBox(const BoxCollider4D& a, const BoxCollider4D& b);
    CollisionResult4D checkSpherePlane(const SphereCollider4D& sphere, const PlaneCollider4D& plane);
    CollisionResult4D checkBoxPlane(const BoxCollider4D& box, const PlaneCollider4D& plane);
    
    // Разрешение столкновений
    void resolveCollision(const CollisionResult4D& collision);
    void applyImpulse(RigidBody4D& bodyA, RigidBody4D& bodyB, const CollisionResult4D& collision);
};

/**
 * @brief 4D частицы
 */
class ParticleSystem4D {
public:
    struct Particle4D {
        Vector4 position;
        Vector4 velocity;
        Vector4 acceleration;
        Vector4 color;
        float life;
        float maxLife;
        float size;
        bool active;
        
        Particle4D() : life(0.0f), maxLife(1.0f), size(1.0f), active(false) {}
    };
    
    std::vector<Particle4D> particles;
    Vector4 emitterPosition;
    Vector4 emitterVelocity;
    Vector4 gravity;
    float emissionRate;
    float particleLife;
    float particleSize;
    Vector4 startColor;
    Vector4 endColor;
    
    ParticleSystem4D();
    ~ParticleSystem4D() = default;
    
    void update(float deltaTime);
    void emit(int count = 1);
    void setEmitter(const Vector4& position, const Vector4& velocity);
    void setGravity(const Vector4& gravity);
    void setEmissionRate(float rate);
    void setParticleLife(float life);
    void setParticleSize(float size);
    void setColorRange(const Vector4& start, const Vector4& end);
    
    void clear();
    int getActiveParticleCount() const;
    
private:
    void updateParticle(Particle4D& particle, float deltaTime);
    void spawnParticle();
    Particle4D createParticle();
};

} // namespace Physics
} // namespace Engine4D
