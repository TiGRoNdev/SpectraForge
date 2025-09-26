#include "Engine4D/Physics/Physics4D.h"
#include <algorithm>
#include <cmath>

namespace Engine4D {
namespace Physics {

// RigidBody4D
RigidBody4D::RigidBody4D() 
    : position(Vector4::zero())
    , velocity(Vector4::zero())
    , acceleration(Vector4::zero())
    , rotation(Vector4::zero())
    , angularVelocity(Vector4::zero())
    , angularAcceleration(Vector4::zero())
    , mass(1.0f)
    , restitution(0.5f)
    , friction(0.5f)
    , isStatic(false)
    , isKinematic(false) {}

void RigidBody4D::update(float deltaTime) {
    if (isStatic) return;
    
    // Обновляем позицию
    position += velocity * deltaTime + 0.5f * acceleration * deltaTime * deltaTime;
    velocity += acceleration * deltaTime;
    
    // Обновляем поворот
    rotation += angularVelocity * deltaTime + 0.5f * angularAcceleration * deltaTime * deltaTime;
    angularVelocity += angularAcceleration * deltaTime;
    
    // Сбрасываем ускорения
    acceleration = Vector4::zero();
    angularAcceleration = Vector4::zero();
}

void RigidBody4D::applyForce(const Vector4& force) {
    if (isStatic || isKinematic) return;
    acceleration += force / mass;
}

void RigidBody4D::applyTorque(const Vector4& torque) {
    if (isStatic || isKinematic) return;
    angularAcceleration += torque / mass; // Упрощенная модель
}

void RigidBody4D::applyImpulse(const Vector4& impulse) {
    if (isStatic || isKinematic) return;
    velocity += impulse / mass;
}

void RigidBody4D::applyAngularImpulse(const Vector4& impulse) {
    if (isStatic || isKinematic) return;
    angularVelocity += impulse / mass; // Упрощенная модель
}

// Collider4D
Collider4D::Collider4D(Type t) : type(t), center(Vector4::zero()), size(Vector4::one()), 
                                 radius(1.0f), normal(Vector4::unitW()), distance(0.0f) {}

void Collider4D::setCenter(const Vector4& center) {
    this->center = center;
}

void Collider4D::setSize(const Vector4& size) {
    this->size = size;
}

void Collider4D::setRadius(float radius) {
    this->radius = radius;
}

void Collider4D::setPlane(const Vector4& normal, float distance) {
    this->normal = normal.normalized();
    this->distance = distance;
}

// SphereCollider4D
SphereCollider4D::SphereCollider4D(float radius) : Collider4D(Type::Sphere) {
    this->radius = radius;
}

bool SphereCollider4D::intersects(const Collider4D& other) const {
    switch (other.type) {
        case Type::Sphere: {
            const SphereCollider4D& sphere = static_cast<const SphereCollider4D&>(other);
            float distance = (center - sphere.center).magnitude();
            return distance <= (radius + sphere.radius);
        }
        case Type::Box: {
            const BoxCollider4D& box = static_cast<const BoxCollider4D&>(other);
            Vector4 closestPoint = box.getClosestPoint(center);
            float distance = (center - closestPoint).magnitude();
            return distance <= radius;
        }
        case Type::Plane: {
            const PlaneCollider4D& plane = static_cast<const PlaneCollider4D&>(other);
            float distance = plane.distanceToPoint(center);
            return std::abs(distance) <= radius;
        }
        default:
            return false;
    }
}

Vector4 SphereCollider4D::getClosestPoint(const Vector4& point) const {
    Vector4 direction = (point - center).normalized();
    return center + direction * radius;
}

Vector4 SphereCollider4D::getNormal(const Vector4& point) const {
    return (point - center).normalized();
}

float SphereCollider4D::getVolume() const {
    // Объем 4D сферы: (π²/2) * r⁴
    return (3.14159f * 3.14159f / 2.0f) * radius * radius * radius * radius;
}

// BoxCollider4D
BoxCollider4D::BoxCollider4D(const Vector4& size) : Collider4D(Type::Box) {
    this->size = size;
}

bool BoxCollider4D::intersects(const Collider4D& other) const {
    switch (other.type) {
        case Type::Sphere: {
            const SphereCollider4D& sphere = static_cast<const SphereCollider4D&>(other);
            Vector4 closestPoint = getClosestPoint(sphere.center);
            float distance = (sphere.center - closestPoint).magnitude();
            return distance <= sphere.radius;
        }
        case Type::Box: {
            const BoxCollider4D& box = static_cast<const BoxCollider4D&>(other);
            return intersectsAABB(box);
        }
        case Type::Plane: {
            const PlaneCollider4D& plane = static_cast<const PlaneCollider4D&>(other);
            // Проверяем, пересекает ли AABB плоскость
            Vector4 min = getMin();
            Vector4 max = getMax();
            
            // Находим ближайшую и дальнюю точки к плоскости
            Vector4 closest = Vector4(
                plane.normal.x > 0 ? min.x : max.x,
                plane.normal.y > 0 ? min.y : max.y,
                plane.normal.z > 0 ? min.z : max.z,
                plane.normal.w > 0 ? min.w : max.w
            );
            Vector4 farthest = Vector4(
                plane.normal.x > 0 ? max.x : min.x,
                plane.normal.y > 0 ? max.y : min.y,
                plane.normal.z > 0 ? max.z : min.z,
                plane.normal.w > 0 ? max.w : min.w
            );
            
            float closestDist = plane.distanceToPoint(closest);
            float farthestDist = plane.distanceToPoint(farthest);
            
            return closestDist <= 0 && farthestDist >= 0;
        }
        default:
            return false;
    }
}

Vector4 BoxCollider4D::getClosestPoint(const Vector4& point) const {
    Vector4 min = getMin();
    Vector4 max = getMax();
    
    return Vector4(
        std::clamp(point.x, min.x, max.x),
        std::clamp(point.y, min.y, max.y),
        std::clamp(point.z, min.z, max.z),
        std::clamp(point.w, min.w, max.w)
    );
}

Vector4 BoxCollider4D::getNormal(const Vector4& point) const {
    Vector4 closest = getClosestPoint(point);
    Vector4 direction = (point - closest).normalized();
    
    // Определяем, какая грань ближе всего
    Vector4 min = getMin();
    Vector4 max = getMax();
    
    float minDist = std::numeric_limits<float>::max();
    Vector4 normal = Vector4::zero();
    
    // Проверяем каждую грань
    if (std::abs(point.x - min.x) < minDist) {
        minDist = std::abs(point.x - min.x);
        normal = Vector4(-1, 0, 0, 0);
    }
    if (std::abs(point.x - max.x) < minDist) {
        minDist = std::abs(point.x - max.x);
        normal = Vector4(1, 0, 0, 0);
    }
    if (std::abs(point.y - min.y) < minDist) {
        minDist = std::abs(point.y - min.y);
        normal = Vector4(0, -1, 0, 0);
    }
    if (std::abs(point.y - max.y) < minDist) {
        minDist = std::abs(point.y - max.y);
        normal = Vector4(0, 1, 0, 0);
    }
    if (std::abs(point.z - min.z) < minDist) {
        minDist = std::abs(point.z - min.z);
        normal = Vector4(0, 0, -1, 0);
    }
    if (std::abs(point.z - max.z) < minDist) {
        minDist = std::abs(point.z - max.z);
        normal = Vector4(0, 0, 1, 0);
    }
    if (std::abs(point.w - min.w) < minDist) {
        minDist = std::abs(point.w - min.w);
        normal = Vector4(0, 0, 0, -1);
    }
    if (std::abs(point.w - max.w) < minDist) {
        minDist = std::abs(point.w - max.w);
        normal = Vector4(0, 0, 0, 1);
    }
    
    return normal;
}

float BoxCollider4D::getVolume() const {
    return size.x * size.y * size.z * size.w;
}

Vector4 BoxCollider4D::getMin() const {
    return center - size * 0.5f;
}

Vector4 BoxCollider4D::getMax() const {
    return center + size * 0.5f;
}

bool BoxCollider4D::intersectsAABB(const BoxCollider4D& other) const {
    Vector4 min1 = getMin();
    Vector4 max1 = getMax();
    Vector4 min2 = other.getMin();
    Vector4 max2 = other.getMax();
    
    return (min1.x <= max2.x && max1.x >= min2.x) &&
           (min1.y <= max2.y && max1.y >= min2.y) &&
           (min1.z <= max2.z && max1.z >= min2.z) &&
           (min1.w <= max2.w && max1.w >= min2.w);
}

// PlaneCollider4D
PlaneCollider4D::PlaneCollider4D(const Vector4& normal, float distance) : Collider4D(Type::Plane) {
    this->normal = normal.normalized();
    this->distance = distance;
}

bool PlaneCollider4D::intersects(const Collider4D& other) const {
    switch (other.type) {
        case Type::Sphere: {
            const SphereCollider4D& sphere = static_cast<const SphereCollider4D&>(other);
            float distance = distanceToPoint(sphere.center);
            return std::abs(distance) <= sphere.radius;
        }
        case Type::Box: {
            const BoxCollider4D& box = static_cast<const BoxCollider4D&>(other);
            Vector4 min = box.getMin();
            Vector4 max = box.getMax();
            
            Vector4 closest = Vector4(
                normal.x > 0 ? min.x : max.x,
                normal.y > 0 ? min.y : max.y,
                normal.z > 0 ? min.z : max.z,
                normal.w > 0 ? min.w : max.w
            );
            Vector4 farthest = Vector4(
                normal.x > 0 ? max.x : min.x,
                normal.y > 0 ? max.y : min.y,
                normal.z > 0 ? max.z : min.z,
                normal.w > 0 ? max.w : min.w
            );
            
            float closestDist = distanceToPoint(closest);
            float farthestDist = distanceToPoint(farthest);
            
            return closestDist <= 0 && farthestDist >= 0;
        }
        case Type::Plane: {
            // Две плоскости пересекаются, если их нормали не параллельны
            const PlaneCollider4D& plane = static_cast<const PlaneCollider4D&>(other);
            return std::abs(normal.dot(plane.normal)) < 0.999f; // Не параллельны
        }
        default:
            return false;
    }
}

Vector4 PlaneCollider4D::getClosestPoint(const Vector4& point) const {
    float distance = distanceToPoint(point);
    return point - normal * distance;
}

Vector4 PlaneCollider4D::getNormal(const Vector4& point) const {
    return normal;
}

float PlaneCollider4D::getVolume() const {
    return 0.0f; // Плоскость имеет нулевой объем
}

float PlaneCollider4D::distanceToPoint(const Vector4& point) const {
    return point.dot(normal) - distance;
}

bool PlaneCollider4D::isPointAbove(const Vector4& point) const {
    return distanceToPoint(point) > 0.0f;
}

// PhysicsWorld4D
PhysicsWorld4D::PhysicsWorld4D() 
    : gravity(Vector4(0, -9.81f, 0, 0)) // Гравитация вниз по Y
    , timeStep(1.0f / 60.0f)
    , maxIterations(10) {}

void PhysicsWorld4D::addBody(std::shared_ptr<RigidBody4D> body) {
    bodies.push_back(body);
}

void PhysicsWorld4D::addCollider(std::shared_ptr<Collider4D> collider) {
    colliders.push_back(collider);
}

void PhysicsWorld4D::removeBody(std::shared_ptr<RigidBody4D> body) {
    bodies.erase(std::remove(bodies.begin(), bodies.end(), body), bodies.end());
}

void PhysicsWorld4D::removeCollider(std::shared_ptr<Collider4D> collider) {
    colliders.erase(std::remove(colliders.begin(), colliders.end(), collider), colliders.end());
}

void PhysicsWorld4D::update(float deltaTime) {
    // Применяем гравитацию
    for (auto& body : bodies) {
        if (!body->isStatic && !body->isKinematic) {
            body->applyForce(gravity * body->mass);
        }
    }
    
    // Обновляем тела
    for (auto& body : bodies) {
        body->update(deltaTime);
    }
    
    // Обнаруживаем и разрешаем столкновения
    auto collisions = detectCollisions();
    for (const auto& collision : collisions) {
        resolveCollision(collision);
    }
}

void PhysicsWorld4D::step(float deltaTime) {
    update(deltaTime);
}

std::vector<CollisionResult4D> PhysicsWorld4D::detectCollisions() {
    std::vector<CollisionResult4D> collisions;
    
    // Простая проверка всех пар коллайдеров
    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            CollisionResult4D result = checkCollision(*colliders[i], *colliders[j]);
            if (result.hasCollision) {
                collisions.push_back(result);
            }
        }
    }
    
    return collisions;
}

CollisionResult4D PhysicsWorld4D::checkCollision(const Collider4D& a, const Collider4D& b) {
    CollisionResult4D result;
    result.hasCollision = false;
    
    // Определяем тип столкновения и вызываем соответствующий метод
    if (a.type == Collider4D::Type::Sphere && b.type == Collider4D::Type::Sphere) {
        return checkSphereSphere(static_cast<const SphereCollider4D&>(a), 
                                static_cast<const SphereCollider4D&>(b));
    } else if (a.type == Collider4D::Type::Sphere && b.type == Collider4D::Type::Box) {
        return checkSphereBox(static_cast<const SphereCollider4D&>(a), 
                             static_cast<const BoxCollider4D&>(b));
    } else if (a.type == Collider4D::Type::Box && b.type == Collider4D::Type::Sphere) {
        return checkSphereBox(static_cast<const SphereCollider4D&>(b), 
                             static_cast<const BoxCollider4D&>(a));
    } else if (a.type == Collider4D::Type::Box && b.type == Collider4D::Type::Box) {
        return checkBoxBox(static_cast<const BoxCollider4D&>(a), 
                          static_cast<const BoxCollider4D&>(b));
    }
    
    return result;
}

CollisionResult4D PhysicsWorld4D::checkSphereSphere(const SphereCollider4D& a, const SphereCollider4D& b) {
    CollisionResult4D result;
    result.hasCollision = false;
    
    Vector4 direction = b.center - a.center;
    float distance = direction.magnitude();
    float minDistance = a.radius + b.radius;
    
    if (distance < minDistance) {
        result.hasCollision = true;
        result.contactPoint = a.center + direction.normalized() * a.radius;
        result.contactNormal = direction.normalized();
        result.penetrationDepth = minDistance - distance;
        result.restitution = std::min(a.restitution, b.restitution);
        result.friction = std::min(a.friction, b.friction);
    }
    
    return result;
}

CollisionResult4D PhysicsWorld4D::checkSphereBox(const SphereCollider4D& sphere, const BoxCollider4D& box) {
    CollisionResult4D result;
    result.hasCollision = false;
    
    Vector4 closestPoint = box.getClosestPoint(sphere.center);
    Vector4 direction = sphere.center - closestPoint;
    float distance = direction.magnitude();
    
    if (distance < sphere.radius) {
        result.hasCollision = true;
        result.contactPoint = closestPoint;
        result.contactNormal = direction.normalized();
        result.penetrationDepth = sphere.radius - distance;
        result.restitution = std::min(sphere.restitution, box.restitution);
        result.friction = std::min(sphere.friction, box.friction);
    }
    
    return result;
}

CollisionResult4D PhysicsWorld4D::checkBoxBox(const BoxCollider4D& a, const BoxCollider4D& b) {
    CollisionResult4D result;
    result.hasCollision = false;
    
    if (a.intersectsAABB(b)) {
        result.hasCollision = true;
        
        // Находим ось с минимальным перекрытием
        Vector4 minA = a.getMin();
        Vector4 maxA = a.getMax();
        Vector4 minB = b.getMin();
        Vector4 maxB = b.getMax();
        
        float minOverlap = std::numeric_limits<float>::max();
        Vector4 contactNormal = Vector4::zero();
        
        // Проверяем перекрытия по каждой оси
        float overlapX = std::min(maxA.x - minB.x, maxB.x - minA.x);
        if (overlapX < minOverlap) {
            minOverlap = overlapX;
            contactNormal = Vector4(1, 0, 0, 0);
        }
        
        float overlapY = std::min(maxA.y - minB.y, maxB.y - minA.y);
        if (overlapY < minOverlap) {
            minOverlap = overlapY;
            contactNormal = Vector4(0, 1, 0, 0);
        }
        
        float overlapZ = std::min(maxA.z - minB.z, maxB.z - minA.z);
        if (overlapZ < minOverlap) {
            minOverlap = overlapZ;
            contactNormal = Vector4(0, 0, 1, 0);
        }
        
        float overlapW = std::min(maxA.w - minB.w, maxB.w - minA.w);
        if (overlapW < minOverlap) {
            minOverlap = overlapW;
            contactNormal = Vector4(0, 0, 0, 1);
        }
        
        result.contactPoint = (a.center + b.center) * 0.5f;
        result.contactNormal = contactNormal;
        result.penetrationDepth = minOverlap;
        result.restitution = std::min(a.restitution, b.restitution);
        result.friction = std::min(a.friction, b.friction);
    }
    
    return result;
}

void PhysicsWorld4D::resolveCollision(const CollisionResult4D& collision) {
    // Упрощенное разрешение столкновений
    // В реальной реализации здесь должна быть более сложная логика
}

void PhysicsWorld4D::setGravity(const Vector4& gravity) {
    this->gravity = gravity;
}

void PhysicsWorld4D::setTimeStep(float step) {
    this->timeStep = step;
}

void PhysicsWorld4D::clear() {
    bodies.clear();
    colliders.clear();
}

PhysicsWorld4D::RaycastHit4D PhysicsWorld4D::raycast(const Vector4& origin, const Vector4& direction, float maxDistance) {
    RaycastHit4D hit;
    hit.hasHit = false;
    hit.distance = maxDistance;
    
    Vector4 normalizedDir = direction.normalized();
    
    for (auto& collider : colliders) {
        // Упрощенная проверка пересечения луча с коллайдером
        // В реальной реализации здесь должна быть более точная логика
        Vector4 closestPoint = collider->getClosestPoint(origin);
        Vector4 toClosest = closestPoint - origin;
        float distance = toClosest.magnitude();
        
        if (distance < hit.distance && distance < maxDistance) {
            hit.hasHit = true;
            hit.point = closestPoint;
            hit.normal = collider->getNormal(closestPoint);
            hit.distance = distance;
            hit.collider = collider;
        }
    }
    
    return hit;
}

// ParticleSystem4D
ParticleSystem4D::ParticleSystem4D() 
    : emitterPosition(Vector4::zero())
    , emitterVelocity(Vector4::zero())
    , gravity(Vector4(0, -9.81f, 0, 0))
    , emissionRate(10.0f)
    , particleLife(2.0f)
    , particleSize(0.1f)
    , startColor(Vector4(1, 1, 1, 1))
    , endColor(Vector4(1, 0, 0, 0)) {}

void ParticleSystem4D::update(float deltaTime) {
    for (auto& particle : particles) {
        if (particle.active) {
            updateParticle(particle, deltaTime);
        }
    }
    
    // Создаем новые частицы
    static float timeSinceLastEmission = 0.0f;
    timeSinceLastEmission += deltaTime;
    
    if (timeSinceLastEmission >= 1.0f / emissionRate) {
        emit();
        timeSinceLastEmission = 0.0f;
    }
}

void ParticleSystem4D::emit(int count) {
    for (int i = 0; i < count; ++i) {
        // Ищем неактивную частицу
        for (auto& particle : particles) {
            if (!particle.active) {
                particle = createParticle();
                break;
            }
        }
        
        // Если все частицы активны, создаем новую
        if (particles.size() < 1000) { // Ограничиваем количество частиц
            particles.push_back(createParticle());
        }
    }
}

void ParticleSystem4D::setEmitter(const Vector4& position, const Vector4& velocity) {
    emitterPosition = position;
    emitterVelocity = velocity;
}

void ParticleSystem4D::setGravity(const Vector4& gravity) {
    this->gravity = gravity;
}

void ParticleSystem4D::setEmissionRate(float rate) {
    emissionRate = rate;
}

void ParticleSystem4D::setParticleLife(float life) {
    particleLife = life;
}

void ParticleSystem4D::setParticleSize(float size) {
    particleSize = size;
}

void ParticleSystem4D::setColorRange(const Vector4& start, const Vector4& end) {
    startColor = start;
    endColor = end;
}

void ParticleSystem4D::clear() {
    particles.clear();
}

int ParticleSystem4D::getActiveParticleCount() const {
    int count = 0;
    for (const auto& particle : particles) {
        if (particle.active) count++;
    }
    return count;
}

void ParticleSystem4D::updateParticle(Particle4D& particle, float deltaTime) {
    // Обновляем физику
    particle.velocity += particle.acceleration * deltaTime;
    particle.position += particle.velocity * deltaTime;
    
    // Применяем гравитацию
    particle.velocity += gravity * deltaTime;
    
    // Обновляем время жизни
    particle.life -= deltaTime;
    
    // Обновляем цвет
    float lifeRatio = particle.life / particle.maxLife;
    particle.color = startColor.lerp(endColor, 1.0f - lifeRatio);
    
    // Деактивируем частицу, если время жизни истекло
    if (particle.life <= 0.0f) {
        particle.active = false;
    }
}

ParticleSystem4D::Particle4D ParticleSystem4D::createParticle() {
    Particle4D particle;
    particle.position = emitterPosition;
    particle.velocity = emitterVelocity + Vector4(
        (rand() % 100 - 50) / 100.0f,
        (rand() % 100 - 50) / 100.0f,
        (rand() % 100 - 50) / 100.0f,
        (rand() % 100 - 50) / 100.0f
    ) * 2.0f; // Добавляем случайность
    particle.acceleration = Vector4::zero();
    particle.color = startColor;
    particle.life = particleLife;
    particle.maxLife = particleLife;
    particle.size = particleSize;
    particle.active = true;
    
    return particle;
}

} // namespace Physics
} // namespace Engine4D
