/**
 * @file core_transform3d_test.cpp
 * @brief Комплексные тесты для класса Transform3D
 */

#include <gtest/gtest.h>
#include <SpectraForge/Core/Transform3D.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Quaternion.h>
#include <SpectraForge/Math/Matrix4.h>

using namespace SpectraForge::Core;
using namespace SpectraForge::Math;

class Transform3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        transform = std::make_unique<Transform3D>();
        epsilon = 0.001f;
    }

    void TearDown() override {
        transform.reset();
    }

    bool nearlyEqual(float a, float b) {
        return std::abs(a - b) < epsilon;
    }

    bool vectorsEqual(const Vector3& a, const Vector3& b) {
        return nearlyEqual(a.x, b.x) &&
               nearlyEqual(a.y, b.y) &&
               nearlyEqual(a.z, b.z);
    }

    std::unique_ptr<Transform3D> transform;
    float epsilon;
};

// ============================================================================
// Конструктор и инициализация
// ============================================================================

TEST_F(Transform3DTest, DefaultConstructor) {
    // Arrange & Act
    Transform3D t;
    
    // Assert
    EXPECT_TRUE(vectorsEqual(t.getPosition(), Vector3::zero()));
    EXPECT_TRUE(vectorsEqual(t.getScale(), Vector3::one()));
}

TEST_F(Transform3DTest, ComponentType) {
    // Arrange & Act
    std::string type = transform->getComponentType();
    
    // Assert
    EXPECT_EQ(type, "Transform3D");
}

// ============================================================================
// Position
// ============================================================================

TEST_F(Transform3DTest, SetPosition) {
    // Arrange
    Vector3 pos(1.0f, 2.0f, 3.0f);
    
    // Act
    transform->setPosition(pos);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(transform->getPosition(), pos));
}

TEST_F(Transform3DTest, Translate) {
    // Arrange
    transform->setPosition(Vector3(1, 2, 3));
    Vector3 translation(1, 1, 1);
    
    // Act
    transform->translate(translation);
    
    // Assert
    Vector3 expected(2, 3, 4);
    EXPECT_TRUE(vectorsEqual(transform->getPosition(), expected));
}

TEST_F(Transform3DTest, GetWorldPosition) {
    // Arrange
    transform->setPosition(Vector3(5, 10, 15));
    
    // Act
    Vector3 worldPos = transform->getWorldPosition();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(worldPos, Vector3(5, 10, 15)));
}

// ============================================================================
// Rotation
// ============================================================================

TEST_F(Transform3DTest, SetRotation) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    
    // Act
    transform->setRotation(rot);
    
    // Assert
    Quaternion result = transform->getRotation();
    EXPECT_TRUE(nearlyEqual(result.w, rot.w));
}

TEST_F(Transform3DTest, Rotate) {
    // Arrange
    Quaternion rot1 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    Quaternion rot2 = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    transform->setRotation(rot1);
    
    // Act
    transform->rotate(rot2);
    
    // Assert - должен быть поворот на M_PI / 2
    Quaternion result = transform->getRotation();
    EXPECT_TRUE(nearlyEqual(result.angle(), M_PI / 2.0f));
}

TEST_F(Transform3DTest, GetWorldRotation) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitZ(), M_PI / 3.0f);
    transform->setRotation(rot);
    
    // Act
    Quaternion worldRot = transform->getWorldRotation();
    
    // Assert
    EXPECT_TRUE(nearlyEqual(worldRot.w, rot.w));
}

// ============================================================================
// Scale
// ============================================================================

TEST_F(Transform3DTest, SetScale) {
    // Arrange
    Vector3 scale(2.0f, 3.0f, 4.0f);
    
    // Act
    transform->setScale(scale);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(transform->getScale(), scale));
}

TEST_F(Transform3DTest, ScaleBy) {
    // Arrange
    transform->setScale(Vector3(1, 2, 3));
    Vector3 scaleFactor(2, 2, 2);
    
    // Act
    transform->scaleBy(scaleFactor);
    
    // Assert
    Vector3 expected(2, 4, 6);
    EXPECT_TRUE(vectorsEqual(transform->getScale(), expected));
}

TEST_F(Transform3DTest, GetWorldScale) {
    // Arrange
    Vector3 scale(2, 3, 4);
    transform->setScale(scale);
    
    // Act
    Vector3 worldScale = transform->getWorldScale();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(worldScale, scale));
}

// ============================================================================
// Направления
// ============================================================================

TEST_F(Transform3DTest, ForwardDirection) {
    // Arrange
    transform->setRotation(Quaternion::identity());
    
    // Act
    Vector3 forward = transform->forward();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(forward, Vector3::forward()));
}

TEST_F(Transform3DTest, RightDirection) {
    // Arrange
    transform->setRotation(Quaternion::identity());
    
    // Act
    Vector3 right = transform->right();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(right, Vector3::right()));
}

TEST_F(Transform3DTest, UpDirection) {
    // Arrange
    transform->setRotation(Quaternion::identity());
    
    // Act
    Vector3 up = transform->up();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(up, Vector3::up()));
}

TEST_F(Transform3DTest, RotatedForwardDirection) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f);
    transform->setRotation(rot);
    
    // Act
    Vector3 forward = transform->forward();
    
    // Assert - forward должен повернуться
    EXPECT_FALSE(vectorsEqual(forward, Vector3::forward()));
}

// ============================================================================
// Матрицы трансформации
// ============================================================================

TEST_F(Transform3DTest, GetLocalMatrix) {
    // Arrange
    transform->setPosition(Vector3(1, 2, 3));
    transform->setScale(Vector3(2, 2, 2));
    
    // Act
    Matrix4 localMat = transform->getLocalMatrix();
    
    // Assert - матрица должна содержать трансформации
    Vector3 translation = localMat.getTranslation();
    EXPECT_TRUE(vectorsEqual(translation, Vector3(1, 2, 3)));
}

TEST_F(Transform3DTest, GetWorldMatrix) {
    // Arrange
    transform->setPosition(Vector3(5, 10, 15));
    
    // Act
    Matrix4 worldMat = transform->getWorldMatrix();
    
    // Assert
    Vector3 translation = worldMat.getTranslation();
    EXPECT_TRUE(vectorsEqual(translation, Vector3(5, 10, 15)));
}

TEST_F(Transform3DTest, GetTransformMatrix) {
    // Arrange
    transform->setPosition(Vector3(1, 2, 3));
    
    // Act
    Matrix4 transformMat = transform->getTransformMatrix();
    
    // Assert
    Matrix4 worldMat = transform->getWorldMatrix();
    EXPECT_TRUE(transformMat == worldMat);
}

// ============================================================================
// Иерархия
// ============================================================================

TEST_F(Transform3DTest, SetParent) {
    // Arrange
    Transform3D parent;
    
    // Act
    transform->setParent(&parent);
    
    // Assert
    EXPECT_EQ(transform->getParent(), &parent);
}

TEST_F(Transform3DTest, AddChild) {
    // Arrange
    Transform3D child;
    
    // Act
    transform->addChild(&child);
    
    // Assert
    const auto& children = transform->getChildren();
    EXPECT_EQ(children.size(), 1);
    EXPECT_EQ(children[0], &child);
}

TEST_F(Transform3DTest, RemoveChild) {
    // Arrange
    Transform3D child;
    transform->addChild(&child);
    
    // Act
    transform->removeChild(&child);
    
    // Assert
    const auto& children = transform->getChildren();
    EXPECT_EQ(children.size(), 0);
}

TEST_F(Transform3DTest, ParentChildRelationship) {
    // Arrange
    Transform3D parent;
    Transform3D child;
    
    // Act
    parent.addChild(&child);
    
    // Assert
    EXPECT_EQ(child.getParent(), &parent);
}

TEST_F(Transform3DTest, HierarchicalPosition) {
    // Arrange
    Transform3D parent;
    Transform3D child;
    
    parent.setPosition(Vector3(10, 0, 0));
    child.setPosition(Vector3(5, 0, 0));
    parent.addChild(&child);
    
    // Act
    Vector3 worldPos = child.getWorldPosition();
    
    // Assert - мировая позиция должна учитывать родителя
    EXPECT_TRUE(vectorsEqual(worldPos, Vector3(15, 0, 0)));
}

// ============================================================================
// Update и Cleanup
// ============================================================================

TEST_F(Transform3DTest, Update) {
    // Arrange
    float deltaTime = 0.016f;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(transform->update(deltaTime));
}

TEST_F(Transform3DTest, Cleanup) {
    // Arrange & Act & Assert - не должно падать
    EXPECT_NO_THROW(transform->cleanup());
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(Transform3DTest, ZeroScale) {
    // Arrange & Act
    transform->setScale(Vector3::zero());
    
    // Assert
    EXPECT_TRUE(vectorsEqual(transform->getScale(), Vector3::zero()));
}

TEST_F(Transform3DTest, NegativeScale) {
    // Arrange
    Vector3 negScale(-1, -1, -1);
    
    // Act
    transform->setScale(negScale);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(transform->getScale(), negScale));
}

TEST_F(Transform3DTest, LargePosition) {
    // Arrange
    Vector3 largePos(1e6f, 1e6f, 1e6f);
    
    // Act
    transform->setPosition(largePos);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(transform->getPosition(), largePos));
}

TEST_F(Transform3DTest, ComplexHierarchy) {
    // Arrange
    Transform3D root;
    Transform3D child1;
    Transform3D child2;
    Transform3D grandchild;
    
    root.setPosition(Vector3(10, 0, 0));
    child1.setPosition(Vector3(5, 0, 0));
    child2.setPosition(Vector3(3, 0, 0));
    grandchild.setPosition(Vector3(2, 0, 0));
    
    // Act
    root.addChild(&child1);
    root.addChild(&child2);
    child1.addChild(&grandchild);
    
    // Assert
    EXPECT_EQ(root.getChildren().size(), 2);
    EXPECT_EQ(child1.getChildren().size(), 1);
    Vector3 grandchildWorld = grandchild.getWorldPosition();
    EXPECT_TRUE(vectorsEqual(grandchildWorld, Vector3(17, 0, 0)));
}

TEST_F(Transform3DTest, MatrixCaching) {
    // Arrange
    transform->setPosition(Vector3(1, 2, 3));
    
    // Act - множественные вызовы
    Matrix4 mat1 = transform->getWorldMatrix();
    Matrix4 mat2 = transform->getWorldMatrix();
    Matrix4 mat3 = transform->getWorldMatrix();
    
    // Assert - все должны быть одинаковыми
    EXPECT_TRUE(mat1 == mat2);
    EXPECT_TRUE(mat2 == mat3);
}

TEST_F(Transform3DTest, MatrixInvalidation) {
    // Arrange
    transform->setPosition(Vector3(1, 2, 3));
    Matrix4 mat1 = transform->getWorldMatrix();
    
    // Act - изменяем позицию
    transform->setPosition(Vector3(4, 5, 6));
    Matrix4 mat2 = transform->getWorldMatrix();
    
    // Assert - матрицы должны отличаться
    EXPECT_TRUE(mat1 != mat2);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
