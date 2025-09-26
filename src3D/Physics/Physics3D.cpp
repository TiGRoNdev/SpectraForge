#include "Engine3D/Physics/Physics3D.h"
#include "Engine3D/Math/MathConstants.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace Engine3D::Math;
using namespace Engine3D::Physics;

namespace Engine3D {
namespace Physics {

// AABB methods
bool Collider3D::AABB::intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

// Collider3D base class
Collider3D::Collider3D(Type type) 
    : type(type)
    , center(0, 0, 0)
    , transform(Matrix4::identity())
    , trigger(false)
    , enabled(true) {
}

// SphereCollider3D implementation
SphereCollider3D::SphereCollider3D(float radius) 
    : Collider3D(Type::Sphere)
    , radius(radius) {
}

bool SphereCollider3D::intersects(const Collider3D& other) const {
    switch (other.getType()) {
        case Type::Sphere:
            return intersectsSphere(static_cast<const SphereCollider3D&>(other));
        case Type::Box:
            return intersectsBox(static_cast<const BoxCollider3D&>(other));
        default:
            std::cout << "SphereCollider3D::intersects - unsupported collider type" << std::endl;
            return false;
    }
}

Vector3 SphereCollider3D::getClosestPoint(const Vector3& point) const {
    Vector3 worldCenter = transform.transformPoint(center);
    Vector3 direction = point - worldCenter;
    float distance = direction.magnitude();
    
    if (distance <= radius) {
        return point; // Точка внутри сферы
    }
    
    return worldCenter + direction.normalized() * radius;
}

Vector3 SphereCollider3D::getNormal(const Vector3& point) const {
    Vector3 worldCenter = transform.transformPoint(center);
    return (point - worldCenter).normalized();
}

float SphereCollider3D::getVolume() const {
    return (4.0f / 3.0f) * M_PI * radius * radius * radius;
}

CollisionResult3D SphereCollider3D::checkCollision(const Collider3D& other) const {
    CollisionResult3D result;
    
    if (other.getType() == Type::Sphere) {
        const SphereCollider3D& otherSphere = static_cast<const SphereCollider3D&>(other);
        
        Vector3 centerA = transform.transformPoint(center);
        Vector3 centerB = other.getTransform().transformPoint(other.getCenter());
        Vector3 direction = centerB - centerA;
        float distance = direction.magnitude();
        float combinedRadius = radius + otherSphere.radius;
        
        if (distance < combinedRadius) {
            result.hasCollision = true;
            result.penetrationDepth = combinedRadius - distance;
            
            if (distance > 0.0f) {
                result.contactNormal = direction / distance;
                result.contactPoint = centerA + result.contactNormal * radius;
            } else {
                result.contactNormal = Vector3(1, 0, 0); // Fallback
                result.contactPoint = centerA;
            }
        }
    }
    
    return result;
}

RaycastHit3D SphereCollider3D::raycast(const Vector3& origin, const Vector3& direction, float maxDistance) const {
    RaycastHit3D hit;
    
    Vector3 worldCenter = transform.transformPoint(center);
    Vector3 oc = origin - worldCenter;
    
    float a = direction.dot(direction);
    float b = 2.0f * oc.dot(direction);
    float c = oc.dot(oc) - radius * radius;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant >= 0) {
        float sqrtDiscriminant = std::sqrt(discriminant);
        float t1 = (-b - sqrtDiscriminant) / (2 * a);
        float t2 = (-b + sqrtDiscriminant) / (2 * a);
        
        float t = (t1 >= 0) ? t1 : t2;
        
        if (t >= 0 && t <= maxDistance) {
            hit.hasHit = true;
            hit.distance = t;
            hit.point = origin + direction * t;
            hit.normal = (hit.point - worldCenter).normalized();
        }
    }
    
    return hit;
}

Collider3D::AABB SphereCollider3D::getBoundingBox() const {
    Vector3 worldCenter = transform.transformPoint(center);
    Vector3 extent(radius, radius, radius);
    return AABB(worldCenter - extent, worldCenter + extent);
}

bool SphereCollider3D::intersectsSphere(const SphereCollider3D& other) const {
    Vector3 centerA = transform.transformPoint(center);
    Vector3 centerB = other.transform.transformPoint(other.center);
    float distance = centerA.distance(centerB);
    return distance <= (radius + other.radius);
}

bool SphereCollider3D::intersectsBox(const BoxCollider3D& other) const {
    // Реализация пересечения сферы с коробкой
    Vector3 sphereCenter = transform.transformPoint(center);
    Vector3 boxMin = other.getMin();
    Vector3 boxMax = other.getMax();
    
    Vector3 closestPoint;
    closestPoint.x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
    closestPoint.y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    closestPoint.z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));
    
    float distance = sphereCenter.distance(closestPoint);
    return distance <= radius;
}

// BoxCollider3D implementation
BoxCollider3D::BoxCollider3D(const Vector3& size) 
    : Collider3D(Type::Box)
    , size(size) {
}

bool BoxCollider3D::intersects(const Collider3D& other) const {
    switch (other.getType()) {
        case Type::Box:
            return intersectsBox(static_cast<const BoxCollider3D&>(other));
        case Type::Sphere:
            return intersectsSphere(static_cast<const SphereCollider3D&>(other));
        default:
            std::cout << "BoxCollider3D::intersects - unsupported collider type" << std::endl;
            return false;
    }
}

Vector3 BoxCollider3D::getClosestPoint(const Vector3& point) const {
    Vector3 localPoint = transform.inverse().transformPoint(point - center);
    Vector3 halfSize = size * 0.5f;
    
    Vector3 closestPoint;
    closestPoint.x = std::max(-halfSize.x, std::min(localPoint.x, halfSize.x));
    closestPoint.y = std::max(-halfSize.y, std::min(localPoint.y, halfSize.y));
    closestPoint.z = std::max(-halfSize.z, std::min(localPoint.z, halfSize.z));
    
    return transform.transformPoint(closestPoint) + center;
}

Vector3 BoxCollider3D::getNormal(const Vector3& point) const {
    Vector3 localPoint = transform.inverse().transformPoint(point - center);
    Vector3 halfSize = size * 0.5f;
    
    // Находим наиболее близкую грань
    Vector3 absLocal = Vector3(std::abs(localPoint.x), std::abs(localPoint.y), std::abs(localPoint.z));
    Vector3 relativePos = Vector3(absLocal.x / halfSize.x, absLocal.y / halfSize.y, absLocal.z / halfSize.z);
    
    Vector3 normal;
    if (relativePos.x >= relativePos.y && relativePos.x >= relativePos.z) {
        normal = Vector3(localPoint.x > 0 ? 1 : -1, 0, 0);
    } else if (relativePos.y >= relativePos.z) {
        normal = Vector3(0, localPoint.y > 0 ? 1 : -1, 0);
    } else {
        normal = Vector3(0, 0, localPoint.z > 0 ? 1 : -1);
    }
    
    return transform.transformDirection(normal).normalized();
}

float BoxCollider3D::getVolume() const {
    return size.x * size.y * size.z;
}

CollisionResult3D BoxCollider3D::checkCollision(const Collider3D& other) const {
    CollisionResult3D result;
    
    if (other.getType() == Type::Box) {
        const BoxCollider3D& otherBox = static_cast<const BoxCollider3D&>(other);
        
        Vector3 minA = getMin();
        Vector3 maxA = getMax();
        Vector3 minB = otherBox.getMin();
        Vector3 maxB = otherBox.getMax();
        
        // Проверка пересечения AABB
        if (minA.x <= maxB.x && maxA.x >= minB.x &&
            minA.y <= maxB.y && maxA.y >= minB.y &&
            minA.z <= maxB.z && maxA.z >= minB.z) {
            
            result.hasCollision = true;
            
            // Вычисляем проникновение по каждой оси
            float overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
            float overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
            float overlapZ = std::min(maxA.z - minB.z, maxB.z - minA.z);
            
            // Находим ось с минимальным проникновением
            if (overlapX <= overlapY && overlapX <= overlapZ) {
                result.penetrationDepth = overlapX;
                result.contactNormal = Vector3(minA.x > minB.x ? -1 : 1, 0, 0);
            } else if (overlapY <= overlapZ) {
                result.penetrationDepth = overlapY;
                result.contactNormal = Vector3(0, minA.y > minB.y ? -1 : 1, 0);
            } else {
                result.penetrationDepth = overlapZ;
                result.contactNormal = Vector3(0, 0, minA.z > minB.z ? -1 : 1);
            }
            
            // Точка контакта - центр пересечения
            Vector3 intersectionMin = Vector3(
                std::max(minA.x, minB.x),
                std::max(minA.y, minB.y),
                std::max(minA.z, minB.z)
            );
            Vector3 intersectionMax = Vector3(
                std::min(maxA.x, maxB.x),
                std::min(maxA.y, maxB.y),
                std::min(maxA.z, maxB.z)
            );
            result.contactPoint = (intersectionMin + intersectionMax) * 0.5f;
        }
    }
    
    return result;
}

RaycastHit3D BoxCollider3D::raycast(const Vector3& origin, const Vector3& direction, float maxDistance) const {
    RaycastHit3D hit;
    
    Vector3 min = getMin();
    Vector3 max = getMax();
    
    // Ray-AABB intersection алгоритм
    Vector3 invDir = Vector3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
    
    float t1 = (min.x - origin.x) * invDir.x;
    float t2 = (max.x - origin.x) * invDir.x;
    float t3 = (min.y - origin.y) * invDir.y;
    float t4 = (max.y - origin.y) * invDir.y;
    float t5 = (min.z - origin.z) * invDir.z;
    float t6 = (max.z - origin.z) * invDir.z;
    
    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
    
    if (tmax < 0 || tmin > tmax || tmin > maxDistance) {
        return hit; // Нет пересечения
    }
    
    float t = (tmin >= 0) ? tmin : tmax;
    
    if (t >= 0 && t <= maxDistance) {
        hit.hasHit = true;
        hit.distance = t;
        hit.point = origin + direction * t;
        hit.normal = getNormal(hit.point);
    }
    
    return hit;
}

Collider3D::AABB BoxCollider3D::getBoundingBox() const {
    return AABB(getMin(), getMax());
}

Vector3 BoxCollider3D::getMin() const {
    Vector3 worldCenter = transform.transformPoint(center);
    Vector3 halfSize = size * 0.5f;
    return worldCenter - halfSize;
}

Vector3 BoxCollider3D::getMax() const {
    Vector3 worldCenter = transform.transformPoint(center);
    Vector3 halfSize = size * 0.5f;
    return worldCenter + halfSize;
}

bool BoxCollider3D::intersectsBox(const BoxCollider3D& other) const {
    Vector3 minA = getMin();
    Vector3 maxA = getMax();
    Vector3 minB = other.getMin();
    Vector3 maxB = other.getMax();
    
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
           (minA.y <= maxB.y && maxA.y >= minB.y) &&
           (minA.z <= maxB.z && maxA.z >= minB.z);
}

bool BoxCollider3D::intersectsSphere(const SphereCollider3D& other) const {
    return other.intersectsBox(*this);
}

// PlaneCollider3D implementation
PlaneCollider3D::PlaneCollider3D(const Vector3& normal, float distance) 
    : Collider3D(Type::Plane)
    , normal(normal.normalized())
    , distance(distance) {
}

bool PlaneCollider3D::intersects(const Collider3D& other) const {
    // Плоскость может пересекаться с другими коллайдерами
    switch (other.getType()) {
        case Type::Sphere: {
            const SphereCollider3D& sphere = static_cast<const SphereCollider3D&>(other);
            Vector3 sphereCenter = sphere.getTransform().transformPoint(sphere.getCenter());
            float dist = distanceToPoint(sphereCenter);
            return std::abs(dist) <= sphere.getRadius();
        }
        case Type::Box: {
            // Упрощенная проверка - проверяем все вершины коробки
            const BoxCollider3D& box = static_cast<const BoxCollider3D&>(other);
            Vector3 min = box.getMin();
            Vector3 max = box.getMax();
            
            std::vector<Vector3> vertices = {
                Vector3(min.x, min.y, min.z), Vector3(max.x, min.y, min.z),
                Vector3(min.x, max.y, min.z), Vector3(max.x, max.y, min.z),
                Vector3(min.x, min.y, max.z), Vector3(max.x, min.y, max.z),
                Vector3(min.x, max.y, max.z), Vector3(max.x, max.y, max.z)
            };
            
            bool hasPositive = false, hasNegative = false;
            for (const Vector3& vertex : vertices) {
                float dist = distanceToPoint(vertex);
                if (dist > 0) hasPositive = true;
                if (dist < 0) hasNegative = true;
                if (hasPositive && hasNegative) return true;
            }
            return false;
        }
        default:
            return false;
    }
}

Vector3 PlaneCollider3D::getClosestPoint(const Vector3& point) const {
    float dist = distanceToPoint(point);
    return point - normal * dist;
}

Vector3 PlaneCollider3D::getNormal(const Vector3& point) const {
    return normal;
}

float PlaneCollider3D::getVolume() const {
    return 0.0f; // Плоскость не имеет объема
}

CollisionResult3D PlaneCollider3D::checkCollision(const Collider3D& other) const {
    CollisionResult3D result;
    
    if (other.getType() == Type::Sphere) {
        const SphereCollider3D& sphere = static_cast<const SphereCollider3D&>(other);
        Vector3 sphereCenter = sphere.getTransform().transformPoint(sphere.getCenter());
        float dist = distanceToPoint(sphereCenter);
        
        if (std::abs(dist) <= sphere.getRadius()) {
            result.hasCollision = true;
            result.penetrationDepth = sphere.getRadius() - std::abs(dist);
            result.contactNormal = dist > 0 ? normal : -normal;
            result.contactPoint = sphereCenter - result.contactNormal * dist;
        }
    }
    
    return result;
}

RaycastHit3D PlaneCollider3D::raycast(const Vector3& origin, const Vector3& direction, float maxDistance) const {
    RaycastHit3D hit;
    
    float denominator = normal.dot(direction);
    
    if (std::abs(denominator) > 1e-6f) { // Луч не параллелен плоскости
        float t = -(normal.dot(origin) + distance) / denominator;
        
        if (t >= 0 && t <= maxDistance) {
            hit.hasHit = true;
            hit.distance = t;
            hit.point = origin + direction * t;
            hit.normal = normal;
        }
    }
    
    return hit;
}

Collider3D::AABB PlaneCollider3D::getBoundingBox() const {
    // Плоскость имеет бесконечные размеры
    const float inf = 1e6f;
    return AABB(Vector3(-inf, -inf, -inf), Vector3(inf, inf, inf));
}

void PlaneCollider3D::setPlane(const Vector3& newNormal, float newDistance) {
    normal = newNormal.normalized();
    distance = newDistance;
}

float PlaneCollider3D::distanceToPoint(const Vector3& point) const {
    return normal.dot(point) + distance;
}

bool PlaneCollider3D::isPointAbove(const Vector3& point) const {
    return distanceToPoint(point) > 0;
}

// RigidBody3D implementation
RigidBody3D::RigidBody3D() 
    : position(0, 0, 0)
    , rotation(Quaternion::identity())
    , velocity(0, 0, 0)
    , angularVelocity(0, 0, 0)
    , force(0, 0, 0)
    , torque(0, 0, 0)
    , mass(1.0f)
    , inverseMass(1.0f)
    , inertia(1, 1, 1)
    , kinematic(false)
    , gravityScale(1.0f)
    , drag(0.0f)
    , angularDrag(0.05f)
    , restitution(0.5f)
    , friction(0.5f) {
}

void RigidBody3D::update(float deltaTime) {
    if (kinematic) {
        return; // Кинематические тела не симулируются
    }
    
    integrate(deltaTime);
    clearForces();
}

void RigidBody3D::applyForce(const Vector3& newForce) {
    if (!kinematic) {
        force += newForce;
    }
}

void RigidBody3D::applyForceAtPosition(const Vector3& newForce, const Vector3& worldPosition) {
    if (!kinematic) {
        applyForce(newForce);
        Vector3 torqueArm = worldPosition - position;
        applyTorque(torqueArm.cross(newForce));
    }
}

void RigidBody3D::applyTorque(const Vector3& newTorque) {
    if (!kinematic) {
        torque += newTorque;
    }
}

void RigidBody3D::applyImpulse(const Vector3& impulse) {
    if (!kinematic) {
        velocity += impulse * inverseMass;
    }
}

void RigidBody3D::applyAngularImpulse(const Vector3& impulse) {
    if (!kinematic) {
        Vector3 invInertia = getInverseInertia();
        angularVelocity += Vector3(impulse.x * invInertia.x, impulse.y * invInertia.y, impulse.z * invInertia.z);
    }
}

void RigidBody3D::setPosition(const Vector3& pos) {
    position = pos;
    if (collider) {
        Matrix4 transform = getTransformMatrix();
        collider->setTransform(transform);
    }
}

void RigidBody3D::setRotation(const Quaternion& rot) {
    rotation = rot;
    if (collider) {
        Matrix4 transform = getTransformMatrix();
        collider->setTransform(transform);
    }
}

Matrix4 RigidBody3D::getTransformMatrix() const {
    Matrix4 translationMatrix = Matrix4::translation(position);
    Matrix4 rotationMatrix = rotation.toMatrix();
    return translationMatrix * rotationMatrix;
}

void RigidBody3D::setVelocity(const Vector3& vel) {
    velocity = vel;
}

void RigidBody3D::setAngularVelocity(const Vector3& angVel) {
    angularVelocity = angVel;
}

void RigidBody3D::setMass(float m) {
    mass = m;
    inverseMass = (m > 0.0f) ? 1.0f / m : 0.0f;
}

void RigidBody3D::setInertia(const Vector3& newInertia) {
    inertia = newInertia;
}

Vector3 RigidBody3D::getInverseInertia() const {
    return Vector3(
        inertia.x > 0.0f ? 1.0f / inertia.x : 0.0f,
        inertia.y > 0.0f ? 1.0f / inertia.y : 0.0f,
        inertia.z > 0.0f ? 1.0f / inertia.z : 0.0f
    );
}

void RigidBody3D::setCollider(std::shared_ptr<Collider3D> newCollider) {
    collider = newCollider;
    if (collider) {
        Matrix4 transform = getTransformMatrix();
        collider->setTransform(transform);
    }
}

void RigidBody3D::integrate(float deltaTime) {
    // Интегрируем скорости
    Vector3 acceleration = force * inverseMass;
    velocity += acceleration * deltaTime;
    
    // Применяем сопротивление
    velocity *= (1.0f - drag * deltaTime);
    angularVelocity *= (1.0f - angularDrag * deltaTime);
    
    // Интегрируем позиции
    position += velocity * deltaTime;
    
    // Интегрируем поворот
    if (angularVelocity.magnitudeSquared() > 1e-6f) {
        Quaternion angularVelQuat(0, angularVelocity.x, angularVelocity.y, angularVelocity.z);
        Quaternion deltaRotation = angularVelQuat * rotation * (deltaTime * 0.5f);
        rotation += deltaRotation;
        rotation.normalize();
    }
    
    // Обновляем коллайдер
    if (collider) {
        Matrix4 transform = getTransformMatrix();
        collider->setTransform(transform);
    }
}

void RigidBody3D::clearForces() {
    force = Vector3::zero();
    torque = Vector3::zero();
}

// ParticleSystem3D implementation
ParticleSystem3D::ParticleSystem3D(size_t maxParticles) 
    : particles(maxParticles)
    , emitterPosition(0, 0, 0)
    , emitterVelocity(0, 0, 0)
    , gravity(0, -9.81f, 0)
    , emissionRate(10.0f)
    , particleLife(2.0f)
    , particleSize(1.0f)
    , startColor(1, 1, 1)
    , endColor(1, 1, 1)
    , playing(false)
    , paused(false)
    , emissionTimer(0.0f) {
}

void ParticleSystem3D::update(float deltaTime) {
    if (!playing || paused) {
        return;
    }
    
    // Эмиссия новых частиц
    emissionTimer += deltaTime;
    float emissionInterval = 1.0f / emissionRate;
    
    while (emissionTimer >= emissionInterval) {
        emit(1);
        emissionTimer -= emissionInterval;
    }
    
    // Обновляем существующие частицы
    for (Particle& particle : particles) {
        if (particle.active) {
            updateParticle(particle, deltaTime);
        }
    }
}

void ParticleSystem3D::emit(int count) {
    for (int i = 0; i < count; ++i) {
        Particle* particle = getInactiveParticle();
        if (particle) {
            particle->position = emitterPosition;
            particle->velocity = emitterVelocity + Vector3(
                (rand() / float(RAND_MAX) - 0.5f) * 2.0f,
                (rand() / float(RAND_MAX) - 0.5f) * 2.0f,
                (rand() / float(RAND_MAX) - 0.5f) * 2.0f
            );
            particle->acceleration = Vector3::zero();
            particle->color = startColor;
            particle->life = particleLife;
            particle->size = particleSize;
            particle->active = true;
        }
    }
}

void ParticleSystem3D::clear() {
    for (Particle& particle : particles) {
        particle.active = false;
    }
}

void ParticleSystem3D::setColorRange(const Vector3& start, const Vector3& end) {
    startColor = start;
    endColor = end;
}

int ParticleSystem3D::getActiveParticleCount() const {
    int count = 0;
    for (const Particle& particle : particles) {
        if (particle.active) {
            count++;
        }
    }
    return count;
}

void ParticleSystem3D::updateParticle(Particle& particle, float deltaTime) {
    // Обновляем физику
    particle.acceleration = gravity;
    particle.velocity += particle.acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;
    
    // Обновляем время жизни
    particle.life -= deltaTime;
    if (particle.life <= 0.0f) {
        particle.active = false;
        return;
    }
    
    // Интерполируем цвет
    float lifeRatio = particle.life / particleLife;
    particle.color = startColor * lifeRatio + endColor * (1.0f - lifeRatio);
}

ParticleSystem3D::Particle* ParticleSystem3D::getInactiveParticle() {
    for (Particle& particle : particles) {
        if (!particle.active) {
            return &particle;
        }
    }
    return nullptr; // Все частицы активны
}

// PhysicsWorld3D implementation
PhysicsWorld3D::PhysicsWorld3D() 
    : gravity(0, -9.81f, 0)
    , timeStep(1.0f / 60.0f)
    , iterations(4) {
}

void PhysicsWorld3D::addRigidBody(std::shared_ptr<RigidBody3D> body) {
    if (body) {
        rigidBodies.push_back(body);
        std::cout << "Added rigid body. Total: " << rigidBodies.size() << std::endl;
    }
}

void PhysicsWorld3D::addCollider(std::shared_ptr<Collider3D> collider) {
    if (collider) {
        colliders.push_back(collider);
        std::cout << "Added collider. Total: " << colliders.size() << std::endl;
    }
}

void PhysicsWorld3D::removeRigidBody(std::shared_ptr<RigidBody3D> body) {
    rigidBodies.erase(
        std::remove(rigidBodies.begin(), rigidBodies.end(), body),
        rigidBodies.end()
    );
}

void PhysicsWorld3D::removeCollider(std::shared_ptr<Collider3D> collider) {
    colliders.erase(
        std::remove(colliders.begin(), colliders.end(), collider),
        colliders.end()
    );
}

void PhysicsWorld3D::clear() {
    rigidBodies.clear();
    colliders.clear();
    std::cout << "Cleared physics world" << std::endl;
}

void PhysicsWorld3D::update(float deltaTime) {
    step(deltaTime);
}

void PhysicsWorld3D::step(float deltaTime) {
    // Применяем гравитацию
    applyGravity(deltaTime);
    
    // Интегрируем скорости
    integrateVelocities(deltaTime);
    
    // Разрешаем столкновения (несколько итераций для стабильности)
    for (int i = 0; i < iterations; ++i) {
        resolveCollisions();
    }
    
    // Интегрируем позиции
    integratePositions(deltaTime);
}

std::vector<CollisionResult3D> PhysicsWorld3D::detectCollisions() {
    std::vector<CollisionResult3D> results;
    
    // Проверяем столкновения между всеми парами rigid body
    for (size_t i = 0; i < rigidBodies.size(); ++i) {
        for (size_t j = i + 1; j < rigidBodies.size(); ++j) {
            auto bodyA = rigidBodies[i];
            auto bodyB = rigidBodies[j];
            
            if (bodyA->getCollider() && bodyB->getCollider()) {
                CollisionResult3D result = bodyA->getCollider()->checkCollision(*bodyB->getCollider());
                if (result.hasCollision) {
                    result.bodyA = bodyA;
                    result.bodyB = bodyB;
                    results.push_back(result);
                }
            }
        }
    }
    
    return results;
}

CollisionResult3D PhysicsWorld3D::checkCollision(const Collider3D& a, const Collider3D& b) {
    return a.checkCollision(b);
}

RaycastHit3D PhysicsWorld3D::raycast(const Vector3& origin, const Vector3& direction, float maxDistance) {
    RaycastHit3D closestHit;
    float closestDistance = maxDistance;
    
    // Проверяем raycast со всеми коллайдерами
    for (auto& collider : colliders) {
        if (collider && collider->isEnabled()) {
            RaycastHit3D hit = collider->raycast(origin, direction, maxDistance);
            if (hit.hasHit && hit.distance < closestDistance) {
                closestHit = hit;
                closestDistance = hit.distance;
                closestHit.collider = collider;
            }
        }
    }
    
    // Проверяем raycast с коллайдерами rigid body
    for (auto& body : rigidBodies) {
        if (body && body->getCollider() && body->getCollider()->isEnabled()) {
            RaycastHit3D hit = body->getCollider()->raycast(origin, direction, maxDistance);
            if (hit.hasHit && hit.distance < closestDistance) {
                closestHit = hit;
                closestDistance = hit.distance;
                closestHit.collider = body->getCollider();
                closestHit.rigidBody = body;
            }
        }
    }
    
    return closestHit;
}

std::vector<RaycastHit3D> PhysicsWorld3D::raycastAll(const Vector3& origin, const Vector3& direction, float maxDistance) {
    std::vector<RaycastHit3D> hits;
    
    // Проверяем raycast со всеми коллайдерами
    for (auto& collider : colliders) {
        if (collider && collider->isEnabled()) {
            RaycastHit3D hit = collider->raycast(origin, direction, maxDistance);
            if (hit.hasHit) {
                hit.collider = collider;
                hits.push_back(hit);
            }
        }
    }
    
    // Проверяем raycast с коллайдерами rigid body
    for (auto& body : rigidBodies) {
        if (body && body->getCollider() && body->getCollider()->isEnabled()) {
            RaycastHit3D hit = body->getCollider()->raycast(origin, direction, maxDistance);
            if (hit.hasHit) {
                hit.collider = body->getCollider();
                hit.rigidBody = body;
                hits.push_back(hit);
            }
        }
    }
    
    // Сортируем по расстоянию
    std::sort(hits.begin(), hits.end(), 
        [](const RaycastHit3D& a, const RaycastHit3D& b) {
            return a.distance < b.distance;
        });
    
    return hits;
}

void PhysicsWorld3D::applyGravity(float deltaTime) {
    for (auto& body : rigidBodies) {
        if (body && !body->isKinematic() && body->getGravityScale() > 0.0f) {
            Vector3 gravityForce = gravity * body->getMass() * body->getGravityScale();
            body->applyForce(gravityForce);
        }
    }
}

void PhysicsWorld3D::integrateVelocities(float deltaTime) {
    for (auto& body : rigidBodies) {
        if (body && !body->isKinematic()) {
            body->update(deltaTime);
        }
    }
}

void PhysicsWorld3D::resolveCollisions() {
    std::vector<CollisionResult3D> collisions = detectCollisions();
    
    for (const CollisionResult3D& collision : collisions) {
        if (!collision.hasCollision || !collision.bodyA || !collision.bodyB) {
            continue;
        }
        
        auto bodyA = collision.bodyA;
        auto bodyB = collision.bodyB;
        
        // Пропускаем кинематические тела
        if (bodyA->isKinematic() && bodyB->isKinematic()) {
            continue;
        }
        
        // Разделяем тела
        float totalInverseMass = bodyA->getInverseMass() + bodyB->getInverseMass();
        if (totalInverseMass > 0.0f) {
            Vector3 separation = collision.contactNormal * collision.penetrationDepth;
            
            if (!bodyA->isKinematic()) {
                float ratioA = bodyA->getInverseMass() / totalInverseMass;
                bodyA->setPosition(bodyA->getPosition() - separation * ratioA);
            }
            
            if (!bodyB->isKinematic()) {
                float ratioB = bodyB->getInverseMass() / totalInverseMass;
                bodyB->setPosition(bodyB->getPosition() + separation * ratioB);
            }
        }
        
        // Применяем импульсы для разрешения столкновения
        Vector3 relativeVelocity = bodyB->getVelocity() - bodyA->getVelocity();
        float velAlongNormal = relativeVelocity.dot(collision.contactNormal);
        
        if (velAlongNormal > 0) {
            continue; // Объекты уже разлетаются
        }
        
        float restitution = std::min(bodyA->getRestitution(), bodyB->getRestitution());
        float impulseScalar = -(1 + restitution) * velAlongNormal / totalInverseMass;
        Vector3 impulse = collision.contactNormal * impulseScalar;
        
        if (!bodyA->isKinematic()) {
            bodyA->applyImpulse(-impulse);
        }
        if (!bodyB->isKinematic()) {
            bodyB->applyImpulse(impulse);
        }
    }
}

void PhysicsWorld3D::integratePositions(float deltaTime) {
    // Позиции уже интегрированы в integrateVelocities через RigidBody3D::update
}

} // namespace Physics
} // namespace Engine3D
