/**
 * @file physics_system_test.cpp
 * @brief Комплексные тесты для физической системы Physics3D
 */

#include <gtest/gtest.h>
#include <SpectraForge/Physics/Physics3D.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Quaternion.h>
#include <cmath>

using namespace SpectraForge::Physics;
using namespace SpectraForge::Math;

// ============================================================================
// CollisionResult3D Tests
// ============================================================================

TEST(CollisionResult3DTest, DefaultConstructor) {
    // Arrange & Act
    CollisionResult3D result;
    
    // Assert
    EXPECT_FALSE(result.hasCollision);
    EXPECT_FLOAT_EQ(result.penetrationDepth, 0.0f);
}

TEST(CollisionResult3DTest, SetValues) {
    // Arrange
    CollisionResult3D result;
    
    // Act
    result.hasCollision = true;
    result.contactPoint = Vector3(1, 2, 3);
    result.contactNormal = Vector3(0, 1, 0);
    result.penetrationDepth = 0.5f;
    
    // Assert
    EXPECT_TRUE(result.hasCollision);
    EXPECT_FLOAT_EQ(result.contactPoint.x, 1.0f);
    EXPECT_FLOAT_EQ(result.penetrationDepth, 0.5f);
}

// ============================================================================
// RaycastHit3D Tests
// ============================================================================

TEST(RaycastHit3DTest, DefaultConstructor) {
    // Arrange & Act
    RaycastHit3D hit;
    
    // Assert
    EXPECT_FALSE(hit.hasHit);
    EXPECT_FLOAT_EQ(hit.distance, 0.0f);
}

TEST(RaycastHit3DTest, SetValues) {
    // Arrange
    RaycastHit3D hit;
    
    // Act
    hit.hasHit = true;
    hit.point = Vector3(5, 0, 0);
    hit.normal = Vector3(1, 0, 0);
    hit.distance = 5.0f;
    
    // Assert
    EXPECT_TRUE(hit.hasHit);
    EXPECT_FLOAT_EQ(hit.distance, 5.0f);
}

// ============================================================================
// SphereCollider3D Tests
// ============================================================================

class SphereCollider3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        collider = std::make_shared<SphereCollider3D>(1.0f);
    }

    std::shared_ptr<SphereCollider3D> collider;
};

TEST_F(SphereCollider3DTest, Constructor) {
    // Arrange & Act
    SphereCollider3D sphere(2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(sphere.getRadius(), 2.0f);
    EXPECT_EQ(sphere.getType(), Collider3D::Sphere);
}

TEST_F(SphereCollider3DTest, GetRadius) {
    // Arrange & Act & Assert
    EXPECT_FLOAT_EQ(collider->getRadius(), 1.0f);
}

TEST_F(SphereCollider3DTest, SetRadius) {
    // Arrange & Act
    collider->setRadius(3.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(collider->getRadius(), 3.0f);
}

TEST_F(SphereCollider3DTest, GetVolume) {
    // Arrange & Act
    float volume = collider->getVolume();
    
    // Assert
    // V = 4/3 * π * r^3 = 4.189
    EXPECT_GT(volume, 4.0f);
    EXPECT_LT(volume, 5.0f);
}

TEST_F(SphereCollider3DTest, GetBoundingBox) {
    // Arrange & Act
    auto aabb = collider->getBoundingBox();
    
    // Assert
    EXPECT_FLOAT_EQ(aabb.min.x, -1.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 1.0f);
}

TEST_F(SphereCollider3DTest, GetClosestPoint) {
    // Arrange
    Vector3 point(10, 0, 0);
    
    // Act
    Vector3 closest = collider->getClosestPoint(point);
    
    // Assert - должна быть точка на поверхности сферы
    float dist = closest.magnitude();
    EXPECT_TRUE(std::abs(dist - 1.0f) < 0.01f);
}

TEST_F(SphereCollider3DTest, GetNormal) {
    // Arrange
    Vector3 point(1, 0, 0);
    
    // Act
    Vector3 normal = collider->getNormal(point);
    
    // Assert - нормаль должна указывать наружу
    EXPECT_GT(normal.magnitude(), 0.9f);
}

TEST_F(SphereCollider3DTest, RaycastHit) {
    // Arrange
    Vector3 origin(-10, 0, 0);
    Vector3 direction(1, 0, 0);
    
    // Act
    RaycastHit3D hit = collider->raycast(origin, direction, 20.0f);
    
    // Assert
    EXPECT_TRUE(hit.hasHit);
}

TEST_F(SphereCollider3DTest, RaycastMiss) {
    // Arrange
    Vector3 origin(-10, 10, 0);
    Vector3 direction(1, 0, 0);
    
    // Act
    RaycastHit3D hit = collider->raycast(origin, direction, 20.0f);
    
    // Assert
    EXPECT_FALSE(hit.hasHit);
}

TEST_F(SphereCollider3DTest, IntersectsSphere) {
    // Arrange
    SphereCollider3D other(1.0f);
    other.setCenter(Vector3(1.5f, 0, 0));  // Частичное пересечение
    
    // Act
    bool intersects = collider->intersectsSphere(other);
    
    // Assert
    EXPECT_TRUE(intersects);
}

TEST_F(SphereCollider3DTest, NotIntersectsSphere) {
    // Arrange
    SphereCollider3D other(1.0f);
    other.setCenter(Vector3(10, 0, 0));  // Далеко
    
    // Act
    bool intersects = collider->intersectsSphere(other);
    
    // Assert
    EXPECT_FALSE(intersects);
}

// ============================================================================
// BoxCollider3D Tests
// ============================================================================

class BoxCollider3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        collider = std::make_shared<BoxCollider3D>(Vector3(2, 2, 2));
    }

    std::shared_ptr<BoxCollider3D> collider;
};

TEST_F(BoxCollider3DTest, Constructor) {
    // Arrange & Act
    BoxCollider3D box(Vector3(1, 2, 3));
    
    // Assert
    EXPECT_FLOAT_EQ(box.getSize().x, 1.0f);
    EXPECT_FLOAT_EQ(box.getSize().y, 2.0f);
    EXPECT_FLOAT_EQ(box.getSize().z, 3.0f);
    EXPECT_EQ(box.getType(), Collider3D::Box);
}

TEST_F(BoxCollider3DTest, GetSize) {
    // Arrange & Act
    Vector3 size = collider->getSize();
    
    // Assert
    EXPECT_FLOAT_EQ(size.x, 2.0f);
}

TEST_F(BoxCollider3DTest, SetSize) {
    // Arrange & Act
    collider->setSize(Vector3(3, 4, 5));
    
    // Assert
    Vector3 size = collider->getSize();
    EXPECT_FLOAT_EQ(size.x, 3.0f);
}

TEST_F(BoxCollider3DTest, GetMin) {
    // Arrange & Act
    Vector3 min = collider->getMin();
    
    // Assert
    EXPECT_FLOAT_EQ(min.x, -1.0f);
    EXPECT_FLOAT_EQ(min.y, -1.0f);
    EXPECT_FLOAT_EQ(min.z, -1.0f);
}

TEST_F(BoxCollider3DTest, GetMax) {
    // Arrange & Act
    Vector3 max = collider->getMax();
    
    // Assert
    EXPECT_FLOAT_EQ(max.x, 1.0f);
    EXPECT_FLOAT_EQ(max.y, 1.0f);
    EXPECT_FLOAT_EQ(max.z, 1.0f);
}

TEST_F(BoxCollider3DTest, GetVolume) {
    // Arrange & Act
    float volume = collider->getVolume();
    
    // Assert
    EXPECT_FLOAT_EQ(volume, 8.0f);  // 2*2*2
}

TEST_F(BoxCollider3DTest, GetBoundingBox) {
    // Arrange & Act
    auto aabb = collider->getBoundingBox();
    
    // Assert
    EXPECT_FLOAT_EQ(aabb.min.x, -1.0f);
    EXPECT_FLOAT_EQ(aabb.max.x, 1.0f);
}

TEST_F(BoxCollider3DTest, GetClosestPoint) {
    // Arrange
    Vector3 point(10, 0, 0);
    
    // Act
    Vector3 closest = collider->getClosestPoint(point);
    
    // Assert
    EXPECT_FLOAT_EQ(closest.x, 1.0f);  // На поверхности бокса
}

TEST_F(BoxCollider3DTest, IntersectsBox) {
    // Arrange
    BoxCollider3D other(Vector3(2, 2, 2));
    other.setCenter(Vector3(1, 0, 0));  // Частичное пересечение
    
    // Act
    bool intersects = collider->intersectsBox(other);
    
    // Assert
    EXPECT_TRUE(intersects);
}

TEST_F(BoxCollider3DTest, RaycastBox) {
    // Arrange
    Vector3 origin(-10, 0, 0);
    Vector3 direction(1, 0, 0);
    
    // Act
    RaycastHit3D hit = collider->raycast(origin, direction, 20.0f);
    
    // Assert
    EXPECT_TRUE(hit.hasHit);
}

// ============================================================================
// PlaneCollider3D Tests
// ============================================================================

class PlaneCollider3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        collider = std::make_shared<PlaneCollider3D>(Vector3(0, 1, 0), 0.0f);
    }

    std::shared_ptr<PlaneCollider3D> collider;
};

TEST_F(PlaneCollider3DTest, Constructor) {
    // Arrange & Act
    PlaneCollider3D plane(Vector3(0, 1, 0), 2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(plane.getNormalVector().y, 1.0f);
    EXPECT_FLOAT_EQ(plane.getDistance(), 2.0f);
    EXPECT_EQ(plane.getType(), Collider3D::Plane);
}

TEST_F(PlaneCollider3DTest, GetNormalVector) {
    // Arrange & Act
    Vector3 normal = collider->getNormalVector();
    
    // Assert
    EXPECT_FLOAT_EQ(normal.y, 1.0f);
}

TEST_F(PlaneCollider3DTest, GetDistance) {
    // Arrange & Act
    float dist = collider->getDistance();
    
    // Assert
    EXPECT_FLOAT_EQ(dist, 0.0f);
}

TEST_F(PlaneCollider3DTest, SetPlane) {
    // Arrange & Act
    collider->setPlane(Vector3(1, 0, 0), 5.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(collider->getNormalVector().x, 1.0f);
    EXPECT_FLOAT_EQ(collider->getDistance(), 5.0f);
}

TEST_F(PlaneCollider3DTest, DistanceToPoint) {
    // Arrange
    Vector3 point(0, 5, 0);
    
    // Act
    float dist = collider->distanceToPoint(point);
    
    // Assert
    EXPECT_FLOAT_EQ(dist, 5.0f);
}

TEST_F(PlaneCollider3DTest, IsPointAbove) {
    // Arrange
    Vector3 above(0, 5, 0);
    Vector3 below(0, -5, 0);
    
    // Act & Assert
    EXPECT_TRUE(collider->isPointAbove(above));
    EXPECT_FALSE(collider->isPointAbove(below));
}

TEST_F(PlaneCollider3DTest, GetClosestPoint) {
    // Arrange
    Vector3 point(5, 10, 5);
    
    // Act
    Vector3 closest = collider->getClosestPoint(point);
    
    // Assert
    EXPECT_FLOAT_EQ(closest.y, 0.0f);  // На плоскости
}

TEST_F(PlaneCollider3DTest, RaycastPlane) {
    // Arrange
    Vector3 origin(0, 10, 0);
    Vector3 direction(0, -1, 0);
    
    // Act
    RaycastHit3D hit = collider->raycast(origin, direction, 20.0f);
    
    // Assert
    EXPECT_TRUE(hit.hasHit);
    EXPECT_FLOAT_EQ(hit.distance, 10.0f);
}

// ============================================================================
// RigidBody3D Tests
// ============================================================================

class RigidBody3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        body = std::make_shared<RigidBody3D>();
    }

    std::shared_ptr<RigidBody3D> body;
};

TEST_F(RigidBody3DTest, Constructor) {
    // Arrange & Act
    RigidBody3D rb;
    
    // Assert
    EXPECT_FLOAT_EQ(rb.getMass(), 1.0f);
    EXPECT_FALSE(rb.isKinematic());
}

TEST_F(RigidBody3DTest, SetPosition) {
    // Arrange
    Vector3 pos(1, 2, 3);
    
    // Act
    body->setPosition(pos);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getPosition().x, 1.0f);
}

TEST_F(RigidBody3DTest, SetRotation) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    
    // Act
    body->setRotation(rot);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getRotation().w, rot.w);
}

TEST_F(RigidBody3DTest, SetVelocity) {
    // Arrange
    Vector3 vel(5, 0, 0);
    
    // Act
    body->setVelocity(vel);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getVelocity().x, 5.0f);
}

TEST_F(RigidBody3DTest, SetAngularVelocity) {
    // Arrange
    Vector3 angVel(0, 1, 0);
    
    // Act
    body->setAngularVelocity(angVel);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getAngularVelocity().y, 1.0f);
}

TEST_F(RigidBody3DTest, SetMass) {
    // Arrange & Act
    body->setMass(2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getMass(), 2.0f);
    EXPECT_FLOAT_EQ(body->getInverseMass(), 0.5f);
}

TEST_F(RigidBody3DTest, SetInertia) {
    // Arrange
    Vector3 inertia(1, 2, 3);
    
    // Act
    body->setInertia(inertia);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getInertia().x, 1.0f);
}

TEST_F(RigidBody3DTest, ApplyForce) {
    // Arrange
    Vector3 force(10, 0, 0);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(body->applyForce(force));
}

TEST_F(RigidBody3DTest, ApplyForceAtPosition) {
    // Arrange
    Vector3 force(10, 0, 0);
    Vector3 position(0, 1, 0);
    
    // Act & Assert
    EXPECT_NO_THROW(body->applyForceAtPosition(force, position));
}

TEST_F(RigidBody3DTest, ApplyTorque) {
    // Arrange
    Vector3 torque(0, 5, 0);
    
    // Act & Assert
    EXPECT_NO_THROW(body->applyTorque(torque));
}

TEST_F(RigidBody3DTest, ApplyImpulse) {
    // Arrange
    Vector3 impulse(100, 0, 0);
    
    // Act & Assert
    EXPECT_NO_THROW(body->applyImpulse(impulse));
}

TEST_F(RigidBody3DTest, SetCollider) {
    // Arrange
    auto collider = std::make_shared<SphereCollider3D>(1.0f);
    
    // Act
    body->setCollider(collider);
    
    // Assert
    EXPECT_NE(body->getCollider(), nullptr);
}

TEST_F(RigidBody3DTest, SetRestitution) {
    // Arrange & Act
    body->setRestitution(0.8f);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getRestitution(), 0.8f);
}

TEST_F(RigidBody3DTest, SetFriction) {
    // Arrange & Act
    body->setFriction(0.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getFriction(), 0.5f);
}

TEST_F(RigidBody3DTest, SetDrag) {
    // Arrange & Act
    body->setDrag(0.1f);
    
    // Assert
    EXPECT_FLOAT_EQ(body->getDrag(), 0.1f);
}

TEST_F(RigidBody3DTest, SetKinematic) {
    // Arrange & Act
    body->setKinematic(true);
    
    // Assert
    EXPECT_TRUE(body->isKinematic());
}

TEST_F(RigidBody3DTest, Update) {
    // Arrange
    float deltaTime = 0.016f;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(body->update(deltaTime));
}

TEST_F(RigidBody3DTest, ClearForces) {
    // Arrange
    body->applyForce(Vector3(100, 0, 0));
    
    // Act & Assert
    EXPECT_NO_THROW(body->clearForces());
}

TEST_F(RigidBody3DTest, GetTransformMatrix) {
    // Arrange
    body->setPosition(Vector3(1, 2, 3));
    
    // Act
    Matrix4 transform = body->getTransformMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(transform.m[0][0]));
}

// ============================================================================
// ParticleSystem3D Tests
// ============================================================================

class ParticleSystem3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        system = std::make_unique<ParticleSystem3D>(100);
    }

    std::unique_ptr<ParticleSystem3D> system;
};

TEST_F(ParticleSystem3DTest, Constructor) {
    // Arrange & Act
    ParticleSystem3D ps(50);
    
    // Assert
    EXPECT_EQ(ps.getActiveParticleCount(), 0);
}

TEST_F(ParticleSystem3DTest, Emit) {
    // Arrange & Act
    system->emit(10);
    
    // Assert
    EXPECT_GT(system->getActiveParticleCount(), 0);
}

TEST_F(ParticleSystem3DTest, Update) {
    // Arrange
    system->emit(5);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(system->update(0.016f));
}

TEST_F(ParticleSystem3DTest, Clear) {
    // Arrange
    system->emit(10);
    
    // Act
    system->clear();
    
    // Assert
    EXPECT_EQ(system->getActiveParticleCount(), 0);
}

TEST_F(ParticleSystem3DTest, SetEmitterPosition) {
    // Arrange & Act
    system->setEmitterPosition(Vector3(1, 2, 3));
    
    // Assert - не должно падать
    EXPECT_NO_THROW(system->emit(1));
}

TEST_F(ParticleSystem3DTest, SetGravity) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(system->setGravity(Vector3(0, -9.8f, 0)));
}

TEST_F(ParticleSystem3DTest, Play) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(system->play());
}

TEST_F(ParticleSystem3DTest, Pause) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(system->pause());
}

TEST_F(ParticleSystem3DTest, Stop) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(system->stop());
}

TEST_F(ParticleSystem3DTest, GetParticles) {
    // Arrange & Act
    const auto& particles = system->getParticles();
    
    // Assert
    EXPECT_EQ(particles.size(), 100);
}

// ============================================================================
// PhysicsWorld3D Tests
// ============================================================================

class PhysicsWorld3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        world = std::make_unique<PhysicsWorld3D>();
    }

    std::unique_ptr<PhysicsWorld3D> world;
};

TEST_F(PhysicsWorld3DTest, Constructor) {
    // Arrange & Act
    PhysicsWorld3D w;
    
    // Assert
    EXPECT_EQ(w.getRigidBodyCount(), 0);
    EXPECT_EQ(w.getColliderCount(), 0);
}

TEST_F(PhysicsWorld3DTest, AddRigidBody) {
    // Arrange
    auto body = std::make_shared<RigidBody3D>();
    
    // Act
    world->addRigidBody(body);
    
    // Assert
    EXPECT_EQ(world->getRigidBodyCount(), 1);
}

TEST_F(PhysicsWorld3DTest, RemoveRigidBody) {
    // Arrange
    auto body = std::make_shared<RigidBody3D>();
    world->addRigidBody(body);
    
    // Act
    world->removeRigidBody(body);
    
    // Assert
    EXPECT_EQ(world->getRigidBodyCount(), 0);
}

TEST_F(PhysicsWorld3DTest, AddCollider) {
    // Arrange
    auto collider = std::make_shared<SphereCollider3D>(1.0f);
    
    // Act
    world->addCollider(collider);
    
    // Assert
    EXPECT_EQ(world->getColliderCount(), 1);
}

TEST_F(PhysicsWorld3DTest, SetGravity) {
    // Arrange & Act
    world->setGravity(Vector3(0, -10, 0));
    
    // Assert
    EXPECT_FLOAT_EQ(world->getGravity().y, -10.0f);
}

TEST_F(PhysicsWorld3DTest, Update) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(world->update(0.016f));
}

TEST_F(PhysicsWorld3DTest, Step) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(world->step(0.016f));
}

TEST_F(PhysicsWorld3DTest, Clear) {
    // Arrange
    auto body = std::make_shared<RigidBody3D>();
    world->addRigidBody(body);
    
    // Act
    world->clear();
    
    // Assert
    EXPECT_EQ(world->getRigidBodyCount(), 0);
}

TEST_F(PhysicsWorld3DTest, Raycast) {
    // Arrange
    Vector3 origin(0, 10, 0);
    Vector3 direction(0, -1, 0);
    
    // Act
    RaycastHit3D hit = world->raycast(origin, direction, 20.0f);
    
    // Assert
    EXPECT_FALSE(hit.hasHit);  // Нет коллайдеров в мире
}

TEST_F(PhysicsWorld3DTest, SetTimeStep) {
    // Arrange & Act
    world->setTimeStep(0.02f);
    
    // Assert
    EXPECT_FLOAT_EQ(world->getTimeStep(), 0.02f);
}

TEST_F(PhysicsWorld3DTest, SetIterations) {
    // Arrange & Act
    world->setIterations(10);
    
    // Assert
    EXPECT_EQ(world->getIterations(), 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
