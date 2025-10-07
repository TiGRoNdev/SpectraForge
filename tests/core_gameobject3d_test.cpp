/**
 * @file core_gameobject3d_test.cpp
 * @brief Комплексные тесты для класса GameObject3D
 */

#include <gtest/gtest.h>
#include <SpectraForge/Core/GameObject3D.h>
#include <SpectraForge/Core/Transform3D.h>
#include <SpectraForge/Math/Vector3.h>

using namespace SpectraForge::Core;
using namespace SpectraForge::Math;

class GameObject3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Очищаем реестр перед каждым тестом
        GameObject3D::clearAllObjects();
    }

    void TearDown() override {
        // Очищаем после теста
        GameObject3D::clearAllObjects();
    }
};

// ============================================================================
// Конструктор и базовые операции
// ============================================================================

TEST_F(GameObject3DTest, DefaultConstructor) {
    // Arrange & Act
    GameObject3D obj;
    
    // Assert
    EXPECT_EQ(obj.getName(), "GameObject3D");
    EXPECT_TRUE(obj.isActive());
    EXPECT_FALSE(obj.isStatic());
}

TEST_F(GameObject3DTest, ConstructorWithName) {
    // Arrange & Act
    GameObject3D obj("TestObject");
    
    // Assert
    EXPECT_EQ(obj.getName(), "TestObject");
}

TEST_F(GameObject3DTest, SetName) {
    // Arrange
    GameObject3D obj;
    
    // Act
    obj.setName("NewName");
    
    // Assert
    EXPECT_EQ(obj.getName(), "NewName");
}

TEST_F(GameObject3DTest, SetTag) {
    // Arrange
    GameObject3D obj;
    
    // Act
    obj.setTag("Player");
    
    // Assert
    EXPECT_EQ(obj.getTag(), "Player");
}

// ============================================================================
// Активность и статус
// ============================================================================

TEST_F(GameObject3DTest, SetActive) {
    // Arrange
    GameObject3D obj;
    
    // Act
    obj.setActive(false);
    
    // Assert
    EXPECT_FALSE(obj.isActive());
}

TEST_F(GameObject3DTest, ToggleActive) {
    // Arrange
    GameObject3D obj;
    bool initialState = obj.isActive();
    
    // Act
    obj.setActive(!initialState);
    obj.setActive(!obj.isActive());
    
    // Assert
    EXPECT_EQ(obj.isActive(), initialState);
}

TEST_F(GameObject3DTest, SetStatic) {
    // Arrange
    GameObject3D obj;
    
    // Act
    obj.setStatic(true);
    
    // Assert
    EXPECT_TRUE(obj.isStatic());
}

// ============================================================================
// Transform
// ============================================================================

TEST_F(GameObject3DTest, HasTransform) {
    // Arrange
    GameObject3D obj;
    
    // Act
    Transform3D* transform = obj.getTransform();
    
    // Assert
    EXPECT_NE(transform, nullptr);
}

TEST_F(GameObject3DTest, TransformPosition) {
    // Arrange
    GameObject3D obj;
    Vector3 pos(1.0f, 2.0f, 3.0f);
    
    // Act
    obj.getTransform()->setPosition(pos);
    
    // Assert
    Vector3 result = obj.getTransform()->getPosition();
    EXPECT_FLOAT_EQ(result.x, pos.x);
    EXPECT_FLOAT_EQ(result.y, pos.y);
    EXPECT_FLOAT_EQ(result.z, pos.z);
}

TEST_F(GameObject3DTest, ConstTransformAccess) {
    // Arrange
    const GameObject3D obj;
    
    // Act
    const Transform3D* transform = obj.getTransform();
    
    // Assert
    EXPECT_NE(transform, nullptr);
}

// ============================================================================
// Lifecycle
// ============================================================================

TEST_F(GameObject3DTest, Start) {
    // Arrange
    GameObject3D obj;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.start());
}

TEST_F(GameObject3DTest, Cleanup) {
    // Arrange
    GameObject3D obj;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.cleanup());
}

TEST_F(GameObject3DTest, Update) {
    // Arrange
    GameObject3D obj;
    float deltaTime = 0.016f;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.update(deltaTime));
}

TEST_F(GameObject3DTest, Render) {
    // Arrange
    GameObject3D obj;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.render());
}

// ============================================================================
// Статические методы создания
// ============================================================================

TEST_F(GameObject3DTest, CreateStatic) {
    // Arrange & Act
    GameObject3D* obj = GameObject3D::create("CreatedObject");
    
    // Assert
    EXPECT_NE(obj, nullptr);
    EXPECT_EQ(obj->getName(), "CreatedObject");
}

TEST_F(GameObject3DTest, CreateStaticDefault) {
    // Arrange & Act
    GameObject3D* obj = GameObject3D::create();
    
    // Assert
    EXPECT_NE(obj, nullptr);
}

TEST_F(GameObject3DTest, CreatePrimitive) {
    // Arrange & Act
    GameObject3D* cube = GameObject3D::createPrimitive("Cube");
    
    // Assert
    EXPECT_NE(cube, nullptr);
}

// ============================================================================
// Поиск объектов
// ============================================================================

TEST_F(GameObject3DTest, FindByName) {
    // Arrange
    GameObject3D* obj = GameObject3D::create("FindMe");
    
    // Act
    GameObject3D* found = GameObject3D::find("FindMe");
    
    // Assert
    EXPECT_EQ(found, obj);
}

TEST_F(GameObject3DTest, FindNonExistent) {
    // Arrange & Act
    GameObject3D* found = GameObject3D::find("DoesNotExist");
    
    // Assert
    EXPECT_EQ(found, nullptr);
}

TEST_F(GameObject3DTest, FindWithTag) {
    // Arrange
    GameObject3D* obj = GameObject3D::create("TaggedObject");
    obj->setTag("Enemy");
    
    // Act
    GameObject3D* found = GameObject3D::findWithTag("Enemy");
    
    // Assert
    EXPECT_EQ(found, obj);
}

TEST_F(GameObject3DTest, FindAllWithTag) {
    // Arrange
    GameObject3D* obj1 = GameObject3D::create("Enemy1");
    GameObject3D* obj2 = GameObject3D::create("Enemy2");
    GameObject3D* obj3 = GameObject3D::create("Player");
    
    obj1->setTag("Enemy");
    obj2->setTag("Enemy");
    obj3->setTag("Player");
    
    // Act
    std::vector<GameObject3D*> enemies = GameObject3D::findAllWithTag("Enemy");
    
    // Assert
    EXPECT_EQ(enemies.size(), 2);
}

TEST_F(GameObject3DTest, FindAllWithTagEmpty) {
    // Arrange & Act
    std::vector<GameObject3D*> found = GameObject3D::findAllWithTag("NonExistent");
    
    // Assert
    EXPECT_TRUE(found.empty());
}

// ============================================================================
// Реестр объектов
// ============================================================================

TEST_F(GameObject3DTest, GetAllObjects) {
    // Arrange
    GameObject3D* obj1 = GameObject3D::create("Obj1");
    GameObject3D* obj2 = GameObject3D::create("Obj2");
    GameObject3D* obj3 = GameObject3D::create("Obj3");
    
    // Act
    const std::vector<GameObject3D*>& all = GameObject3D::getAllObjects();
    
    // Assert
    EXPECT_EQ(all.size(), 3);
}

TEST_F(GameObject3DTest, ClearAllObjects) {
    // Arrange
    GameObject3D::create("Obj1");
    GameObject3D::create("Obj2");
    EXPECT_GT(GameObject3D::getAllObjects().size(), 0);
    
    // Act
    GameObject3D::clearAllObjects();
    
    // Assert
    EXPECT_EQ(GameObject3D::getAllObjects().size(), 0);
}

TEST_F(GameObject3DTest, Destroy) {
    // Arrange
    GameObject3D obj("ToDestroy");
    size_t initialCount = GameObject3D::getAllObjects().size();
    
    // Act
    obj.destroy();
    
    // Assert
    size_t finalCount = GameObject3D::getAllObjects().size();
    EXPECT_LT(finalCount, initialCount);
}

// ============================================================================
// Компоненты
// ============================================================================

TEST_F(GameObject3DTest, AddComponent) {
    // Arrange
    GameObject3D obj;
    
    // Act
    Transform3D* component = obj.addComponent<Transform3D>();
    
    // Assert
    EXPECT_NE(component, nullptr);
}

TEST_F(GameObject3DTest, GetComponent) {
    // Arrange
    GameObject3D obj;
    obj.addComponent<Transform3D>();
    
    // Act
    Transform3D* component = obj.getComponent<Transform3D>();
    
    // Assert
    EXPECT_NE(component, nullptr);
}

TEST_F(GameObject3DTest, GetComponentNotFound) {
    // Arrange
    GameObject3D obj;
    
    // Act
    Transform3D* component = obj.getComponent<Transform3D>();
    
    // Assert
    EXPECT_EQ(component, nullptr);
}

TEST_F(GameObject3DTest, GetComponentsMultiple) {
    // Arrange
    GameObject3D obj;
    obj.addComponent<Transform3D>();
    obj.addComponent<Transform3D>();
    
    // Act
    std::vector<Transform3D*> components = obj.getComponents<Transform3D>();
    
    // Assert
    EXPECT_EQ(components.size(), 2);
}

TEST_F(GameObject3DTest, RemoveComponent) {
    // Arrange
    GameObject3D obj;
    Transform3D* component = obj.addComponent<Transform3D>();
    
    // Act
    obj.removeComponent(component);
    
    // Assert
    Transform3D* found = obj.getComponent<Transform3D>();
    EXPECT_EQ(found, nullptr);
}

TEST_F(GameObject3DTest, RemoveComponentByType) {
    // Arrange
    GameObject3D obj;
    obj.addComponent<Transform3D>();
    
    // Act
    obj.removeComponent("Transform3D");
    
    // Assert
    Transform3D* found = obj.getComponent<Transform3D>();
    EXPECT_EQ(found, nullptr);
}

// ============================================================================
// Граничные случаи
// ============================================================================

TEST_F(GameObject3DTest, EmptyName) {
    // Arrange & Act
    GameObject3D obj("");
    
    // Assert
    EXPECT_EQ(obj.getName(), "");
}

TEST_F(GameObject3DTest, LongName) {
    // Arrange
    std::string longName(10000, 'A');
    
    // Act
    GameObject3D obj(longName);
    
    // Assert
    EXPECT_EQ(obj.getName(), longName);
}

TEST_F(GameObject3DTest, SpecialCharactersInName) {
    // Arrange & Act
    GameObject3D obj("Test@#$%^&*()");
    
    // Assert
    EXPECT_EQ(obj.getName(), "Test@#$%^&*()");
}

TEST_F(GameObject3DTest, UpdateWhenInactive) {
    // Arrange
    GameObject3D obj;
    obj.setActive(false);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.update(0.016f));
}

TEST_F(GameObject3DTest, RenderWhenInactive) {
    // Arrange
    GameObject3D obj;
    obj.setActive(false);
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(obj.render());
}

TEST_F(GameObject3DTest, MultipleStartCalls) {
    // Arrange
    GameObject3D obj;
    
    // Act & Assert - множественные вызовы не должны вызывать проблем
    EXPECT_NO_THROW(obj.start());
    EXPECT_NO_THROW(obj.start());
    EXPECT_NO_THROW(obj.start());
}

TEST_F(GameObject3DTest, MultipleCleanupCalls) {
    // Arrange
    GameObject3D obj;
    
    // Act & Assert
    EXPECT_NO_THROW(obj.cleanup());
    EXPECT_NO_THROW(obj.cleanup());
}

TEST_F(GameObject3DTest, LargeNumberOfObjects) {
    // Arrange & Act
    for (int i = 0; i < 1000; ++i) {
        GameObject3D::create("Object_" + std::to_string(i));
    }
    
    // Assert
    EXPECT_EQ(GameObject3D::getAllObjects().size(), 1000);
}

TEST_F(GameObject3DTest, FindPerformance) {
    // Arrange
    for (int i = 0; i < 100; ++i) {
        GameObject3D::create("Object_" + std::to_string(i));
    }
    GameObject3D::create("TargetObject");
    
    // Act
    GameObject3D* found = GameObject3D::find("TargetObject");
    
    // Assert
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->getName(), "TargetObject");
}

TEST_F(GameObject3DTest, TagPerformance) {
    // Arrange
    for (int i = 0; i < 100; ++i) {
        GameObject3D* obj = GameObject3D::create("Tagged_" + std::to_string(i));
        obj->setTag("TestTag");
    }
    
    // Act
    std::vector<GameObject3D*> found = GameObject3D::findAllWithTag("TestTag");
    
    // Assert
    EXPECT_EQ(found.size(), 100);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
