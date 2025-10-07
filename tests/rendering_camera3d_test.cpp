/**
 * @file rendering_camera3d_test.cpp
 * @brief Комплексные тесты для класса Camera3D
 */

#include <gtest/gtest.h>
#include <SpectraForge/Rendering/Camera3D.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Matrix4.h>
#include <SpectraForge/Math/Quaternion.h>
#include <cmath>

using namespace SpectraForge::Rendering;
using namespace SpectraForge::Math;

class Camera3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        camera = std::make_unique<Camera3D>();
        epsilon = 0.001f;
    }

    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }

    bool vectorsEqual(const Vector3& a, const Vector3& b) {
        return nearlyEqual(a.x, b.x) && nearlyEqual(a.y, b.y) && nearlyEqual(a.z, b.z);
    }

    std::unique_ptr<Camera3D> camera;
    float epsilon;
};

// ============================================================================
// Конструктор и базовые свойства
// ============================================================================

TEST_F(Camera3DTest, DefaultConstructor) {
    // Arrange & Act
    Camera3D cam;
    
    // Assert
    EXPECT_TRUE(vectorsEqual(cam.getPosition(), Vector3::zero()));
}

TEST_F(Camera3DTest, SetPosition) {
    // Arrange
    Vector3 pos(1, 2, 3);
    
    // Act
    camera->setPosition(pos);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(camera->getPosition(), pos));
}

TEST_F(Camera3DTest, SetTarget) {
    // Arrange
    Vector3 target(5, 0, 0);
    
    // Act
    camera->setTarget(target);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(camera->getTarget(), target));
}

TEST_F(Camera3DTest, SetRotation) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    
    // Act
    camera->setRotation(rot);
    
    // Assert
    Quaternion result = camera->getRotation();
    EXPECT_TRUE(nearlyEqual(result.w, rot.w));
}

// ============================================================================
// LookAt
// ============================================================================

TEST_F(Camera3DTest, LookAt) {
    // Arrange
    Vector3 pos(0, 0, 10);
    Vector3 target(0, 0, 0);
    Vector3 up(0, 1, 0);
    
    // Act
    camera->lookAt(pos, target, up);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(camera->getPosition(), pos));
    EXPECT_TRUE(vectorsEqual(camera->getTarget(), target));
}

TEST_F(Camera3DTest, LookAtDirection) {
    // Arrange
    Vector3 pos(0, 0, 5);
    Vector3 target(0, 0, 0);
    camera->lookAt(pos, target);
    
    // Act
    Vector3 forward = camera->getForward();
    
    // Assert - forward должен указывать на target
    EXPECT_TRUE(nearlyEqual(forward.z, -1.0f));
}

// ============================================================================
// Направления
// ============================================================================

TEST_F(Camera3DTest, GetForward) {
    // Arrange & Act
    Vector3 forward = camera->getForward();
    
    // Assert
    EXPECT_FALSE(std::isnan(forward.x));
    EXPECT_FALSE(std::isnan(forward.y));
    EXPECT_FALSE(std::isnan(forward.z));
}

TEST_F(Camera3DTest, GetRight) {
    // Arrange & Act
    Vector3 right = camera->getRight();
    
    // Assert
    EXPECT_FALSE(std::isnan(right.x));
}

TEST_F(Camera3DTest, GetUp) {
    // Arrange & Act
    Vector3 up = camera->getUp();
    
    // Assert
    EXPECT_FALSE(std::isnan(up.y));
}

// ============================================================================
// Движение
// ============================================================================

TEST_F(Camera3DTest, Move) {
    // Arrange
    Vector3 initialPos = camera->getPosition();
    Vector3 movement(1, 0, 0);
    
    // Act
    camera->move(movement);
    
    // Assert
    Vector3 newPos = camera->getPosition();
    EXPECT_TRUE(vectorsEqual(newPos, initialPos + movement));
}

TEST_F(Camera3DTest, MoveForward) {
    // Arrange
    camera->setPosition(Vector3::zero());
    
    // Act
    camera->moveForward(5.0f);
    
    // Assert
    Vector3 pos = camera->getPosition();
    EXPECT_FALSE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Camera3DTest, MoveRight) {
    // Arrange
    camera->setPosition(Vector3::zero());
    
    // Act
    camera->moveRight(3.0f);
    
    // Assert
    Vector3 pos = camera->getPosition();
    EXPECT_FALSE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Camera3DTest, MoveUp) {
    // Arrange
    camera->setPosition(Vector3::zero());
    
    // Act
    camera->moveUp(2.0f);
    
    // Assert
    Vector3 pos = camera->getPosition();
    EXPECT_GT(pos.y, 0.0f);
}

// ============================================================================
// Rotation
// ============================================================================

TEST_F(Camera3DTest, Rotate) {
    // Arrange
    float pitch = M_PI / 6.0f;
    float yaw = M_PI / 4.0f;
    float roll = 0.0f;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(camera->rotate(pitch, yaw, roll));
}

TEST_F(Camera3DTest, RotateAround) {
    // Arrange
    Vector3 point(0, 0, 0);
    Vector3 axis = Vector3::unitY();
    float angle = M_PI / 2.0f;
    
    // Act & Assert
    EXPECT_NO_THROW(camera->rotateAround(point, axis, angle));
}

// ============================================================================
// Проекция - Perspective
// ============================================================================

TEST_F(Camera3DTest, SetPerspective) {
    // Arrange
    float fov = M_PI / 4.0f;
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    // Act
    camera->setPerspective(fov, aspect, near, far);
    
    // Assert
    EXPECT_TRUE(camera->isPerspective());
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), fov);
    EXPECT_FLOAT_EQ(camera->getAspectRatio(), aspect);
    EXPECT_FLOAT_EQ(camera->getNearPlane(), near);
    EXPECT_FLOAT_EQ(camera->getFarPlane(), far);
}

TEST_F(Camera3DTest, SetFieldOfView) {
    // Arrange
    float fov = M_PI / 3.0f;
    
    // Act
    camera->setFieldOfView(fov);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), fov);
}

TEST_F(Camera3DTest, SetAspectRatio) {
    // Arrange
    float aspect = 21.0f / 9.0f;
    
    // Act
    camera->setAspectRatio(aspect);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getAspectRatio(), aspect);
}

TEST_F(Camera3DTest, SetNearPlane) {
    // Arrange
    float near = 0.01f;
    
    // Act
    camera->setNearPlane(near);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getNearPlane(), near);
}

TEST_F(Camera3DTest, SetFarPlane) {
    // Arrange
    float far = 1000.0f;
    
    // Act
    camera->setFarPlane(far);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getFarPlane(), far);
}

// ============================================================================
// Проекция - Orthographic
// ============================================================================

TEST_F(Camera3DTest, SetOrthographic) {
    // Arrange & Act
    camera->setOrthographic(-10, 10, -10, 10, 0.1f, 100.0f);
    
    // Assert
    EXPECT_FALSE(camera->isPerspective());
}

// ============================================================================
// Матрицы
// ============================================================================

TEST_F(Camera3DTest, GetViewMatrix) {
    // Arrange
    camera->setPosition(Vector3(0, 0, 10));
    camera->lookAt(camera->getPosition(), Vector3::zero());
    
    // Act
    Matrix4 view = camera->getViewMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(view.m[0][0]));
}

TEST_F(Camera3DTest, GetProjectionMatrix) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    
    // Act
    Matrix4 proj = camera->getProjectionMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(proj.m[0][0]));
    EXPECT_NE(proj.m[0][0], 0.0f);
}

TEST_F(Camera3DTest, GetViewProjectionMatrix) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    camera->setPosition(Vector3(0, 0, 10));
    
    // Act
    Matrix4 viewProj = camera->getViewProjectionMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(viewProj.m[0][0]));
}

// ============================================================================
// Coordinate Transformations
// ============================================================================

TEST_F(Camera3DTest, ScreenToWorld) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Vector3 screenPos(400, 300, 0.5f);
    Vector3 screenSize(800, 600, 1);
    
    // Act
    Vector3 worldPos = camera->screenToWorld(screenPos, screenSize);
    
    // Assert
    EXPECT_FALSE(std::isnan(worldPos.x));
    EXPECT_FALSE(std::isnan(worldPos.y));
    EXPECT_FALSE(std::isnan(worldPos.z));
}

TEST_F(Camera3DTest, WorldToScreen) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Vector3 worldPos(0, 0, -5);
    Vector3 screenSize(800, 600, 1);
    
    // Act
    Vector3 screenPos = camera->worldToScreen(worldPos, screenSize);
    
    // Assert
    EXPECT_FALSE(std::isnan(screenPos.x));
    EXPECT_FALSE(std::isnan(screenPos.y));
}

// ============================================================================
// Ray Casting
// ============================================================================

TEST_F(Camera3DTest, ScreenPointToRay) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    camera->setPosition(Vector3(0, 0, 10));
    Vector3 screenPos(400, 300, 0);
    Vector3 screenSize(800, 600, 1);
    
    // Act
    Camera3D::Ray ray = camera->screenPointToRay(screenPos, screenSize);
    
    // Assert
    EXPECT_FALSE(std::isnan(ray.origin.x));
    EXPECT_FALSE(std::isnan(ray.direction.x));
}

// ============================================================================
// Frustum Culling
// ============================================================================

TEST_F(Camera3DTest, GetFrustum) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    
    // Act
    Camera3D::Frustum frustum = camera->getFrustum();
    
    // Assert - проверяем, что плоскости заполнены
    for (int i = 0; i < 6; ++i) {
        EXPECT_FALSE(std::isnan(frustum.planes[i].x));
    }
}

TEST_F(Camera3DTest, IsInFrustumPoint) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    camera->setPosition(Vector3(0, 0, 10));
    camera->lookAt(camera->getPosition(), Vector3::zero());
    
    Vector3 pointInFront(0, 0, 0);    // В поле зрения
    Vector3 pointBehind(0, 0, 100);   // За камерой
    
    // Act
    bool inFrustum1 = camera->isInFrustum(pointInFront);
    bool inFrustum2 = camera->isInFrustum(pointBehind);
    
    // Assert
    EXPECT_TRUE(inFrustum1);
    EXPECT_FALSE(inFrustum2);
}

TEST_F(Camera3DTest, IsInFrustumSphere) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    camera->setPosition(Vector3(0, 0, 10));
    camera->lookAt(camera->getPosition(), Vector3::zero());
    
    Vector3 center(0, 0, 0);
    float radius = 1.0f;
    
    // Act
    bool inFrustum = camera->isInFrustum(center, radius);
    
    // Assert
    EXPECT_TRUE(inFrustum);
}

// ============================================================================
// Утилиты
// ============================================================================

TEST_F(Camera3DTest, UpdateViewMatrix) {
    // Arrange
    camera->setPosition(Vector3(1, 2, 3));
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(camera->updateViewMatrix());
}

TEST_F(Camera3DTest, UpdateProjectionMatrix) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    
    // Act & Assert
    EXPECT_NO_THROW(camera->updateProjectionMatrix());
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(Camera3DTest, ZeroFieldOfView) {
    // Arrange & Act
    camera->setFieldOfView(0.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), 0.0f);
}

TEST_F(Camera3DTest, LargeFieldOfView) {
    // Arrange & Act
    camera->setFieldOfView(M_PI);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), M_PI);
}

TEST_F(Camera3DTest, NearEqualsFar) {
    // Arrange
    camera->setNearPlane(10.0f);
    camera->setFarPlane(10.0f);
    
    // Act
    Matrix4 proj = camera->getProjectionMatrix();
    
    // Assert - может быть вырожденная матрица
    EXPECT_FALSE(std::isnan(proj.m[0][0]));
}

TEST_F(Camera3DTest, NegativeNearPlane) {
    // Arrange & Act
    camera->setNearPlane(-1.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(camera->getNearPlane(), -1.0f);
}

TEST_F(Camera3DTest, ExtremeAspectRatio) {
    // Arrange & Act
    camera->setAspectRatio(0.1f);  // Очень узкий
    Matrix4 proj1 = camera->getProjectionMatrix();
    
    camera->setAspectRatio(10.0f);  // Очень широкий
    Matrix4 proj2 = camera->getProjectionMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(proj1.m[0][0]));
    EXPECT_FALSE(std::isnan(proj2.m[0][0]));
}

TEST_F(Camera3DTest, MultipleMovements) {
    // Arrange
    camera->setPosition(Vector3::zero());
    
    // Act - цепочка движений
    camera->moveForward(5);
    camera->moveRight(3);
    camera->moveUp(2);
    camera->move(Vector3(1, 1, 1));
    
    // Assert
    Vector3 pos = camera->getPosition();
    EXPECT_FALSE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Camera3DTest, MatrixCaching) {
    // Arrange
    camera->setPerspective(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    camera->setPosition(Vector3(0, 0, 10));
    
    // Act - множественные запросы матриц
    Matrix4 view1 = camera->getViewMatrix();
    Matrix4 view2 = camera->getViewMatrix();
    Matrix4 proj1 = camera->getProjectionMatrix();
    Matrix4 proj2 = camera->getProjectionMatrix();
    
    // Assert - матрицы должны быть одинаковыми (кэширование)
    EXPECT_TRUE(view1 == view2);
    EXPECT_TRUE(proj1 == proj2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
