/**
 * @file integration_pipeline_test.cpp
 * @brief Интеграционные тесты для полного конвейера рендеринга
 */

#include <gtest/gtest.h>
#include <SpectraForge/Core/GameObject3D.h>
#include <SpectraForge/Core/Transform3D.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Matrix4.h>
#include <SpectraForge/Math/Quaternion.h>

using namespace SpectraForge::Core;
using namespace SpectraForge::Math;

class IntegrationPipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        GameObject3D::clearAllObjects();
    }

    void TearDown() override {
        GameObject3D::clearAllObjects();
    }
};

// ============================================================================
// Сценарий 1: Создание и манипуляция сценой
// ============================================================================

TEST_F(IntegrationPipelineTest, CreateComplexScene) {
    // Arrange
    GameObject3D* root = GameObject3D::create("Root");
    GameObject3D* camera = GameObject3D::create("Camera");
    GameObject3D* player = GameObject3D::create("Player");
    GameObject3D* enemy1 = GameObject3D::create("Enemy1");
    GameObject3D* enemy2 = GameObject3D::create("Enemy2");
    
    root->setTag("Root");
    camera->setTag("Camera");
    player->setTag("Player");
    enemy1->setTag("Enemy");
    enemy2->setTag("Enemy");
    
    // Act
    camera->getTransform()->setPosition(Vector3(0, 5, 10));
    player->getTransform()->setPosition(Vector3(0, 0, 0));
    enemy1->getTransform()->setPosition(Vector3(5, 0, 0));
    enemy2->getTransform()->setPosition(Vector3(-5, 0, 0));
    
    // Assert
    EXPECT_EQ(GameObject3D::getAllObjects().size(), 5);
    EXPECT_EQ(GameObject3D::findAllWithTag("Enemy").size(), 2);
    EXPECT_NE(GameObject3D::findWithTag("Player"), nullptr);
}

// ============================================================================
// Сценарий 2: Иерархия трансформаций
// ============================================================================

TEST_F(IntegrationPipelineTest, HierarchicalTransforms) {
    // Arrange - создаем иерархию: Parent -> Child -> Grandchild
    GameObject3D* parent = GameObject3D::create("Parent");
    GameObject3D* child = GameObject3D::create("Child");
    GameObject3D* grandchild = GameObject3D::create("Grandchild");
    
    parent->getTransform()->setPosition(Vector3(10, 0, 0));
    child->getTransform()->setPosition(Vector3(5, 0, 0));
    grandchild->getTransform()->setPosition(Vector3(2, 0, 0));
    
    // Act
    parent->getTransform()->addChild(child->getTransform());
    child->getTransform()->addChild(grandchild->getTransform());
    
    // Assert
    Vector3 grandchildWorld = grandchild->getTransform()->getWorldPosition();
    EXPECT_FLOAT_EQ(grandchildWorld.x, 17.0f);  // 10 + 5 + 2
}

// ============================================================================
// Сценарий 3: Применение трансформаций
// ============================================================================

TEST_F(IntegrationPipelineTest, TransformationPipeline) {
    // Arrange
    GameObject3D* obj = GameObject3D::create("TransformObject");
    Transform3D* transform = obj->getTransform();
    
    // Act - применяем последовательность трансформаций
    transform->setPosition(Vector3(1, 2, 3));
    transform->setScale(Vector3(2, 2, 2));
    transform->setRotation(Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f));
    
    Matrix4 worldMatrix = transform->getWorldMatrix();
    Vector3 point(1, 0, 0);
    Vector3 transformed = worldMatrix * point;
    
    // Assert - точка должна быть трансформирована
    EXPECT_FALSE(std::isnan(transformed.x));
    EXPECT_FALSE(std::isnan(transformed.y));
    EXPECT_FALSE(std::isnan(transformed.z));
}

// ============================================================================
// Сценарий 4: Update цикл
// ============================================================================

TEST_F(IntegrationPipelineTest, UpdateLoop) {
    // Arrange
    GameObject3D* obj1 = GameObject3D::create("UpdateTest1");
    GameObject3D* obj2 = GameObject3D::create("UpdateTest2");
    GameObject3D* obj3 = GameObject3D::create("UpdateTest3");
    
    obj1->start();
    obj2->start();
    obj3->start();
    
    // Act - симулируем несколько кадров
    for (int i = 0; i < 60; ++i) {
        float deltaTime = 0.016f;  // ~60 FPS
        obj1->update(deltaTime);
        obj2->update(deltaTime);
        obj3->update(deltaTime);
    }
    
    // Assert - не должно быть ошибок
    EXPECT_TRUE(obj1->isActive());
    EXPECT_TRUE(obj2->isActive());
    EXPECT_TRUE(obj3->isActive());
}

// ============================================================================
// Сценарий 5: Активация/деактивация объектов
// ============================================================================

TEST_F(IntegrationPipelineTest, ObjectActivation) {
    // Arrange
    GameObject3D* obj = GameObject3D::create("ActivationTest");
    obj->start();
    
    // Act
    obj->setActive(false);
    obj->update(0.016f);
    obj->setActive(true);
    obj->update(0.016f);
    
    // Assert
    EXPECT_TRUE(obj->isActive());
}

// ============================================================================
// Сценарий 6: Массовое создание и удаление
// ============================================================================

TEST_F(IntegrationPipelineTest, MassObjectCreation) {
    // Arrange & Act
    std::vector<GameObject3D*> objects;
    for (int i = 0; i < 100; ++i) {
        GameObject3D* obj = GameObject3D::create("Object_" + std::to_string(i));
        obj->getTransform()->setPosition(Vector3(i, i, i));
        objects.push_back(obj);
    }
    
    // Assert
    EXPECT_EQ(GameObject3D::getAllObjects().size(), 100);
    
    // Cleanup
    GameObject3D::clearAllObjects();
    EXPECT_EQ(GameObject3D::getAllObjects().size(), 0);
}

// ============================================================================
// Сценарий 7: Комплексная сцена с анимацией
// ============================================================================

TEST_F(IntegrationPipelineTest, AnimatedScene) {
    // Arrange
    GameObject3D* rotatingObj = GameObject3D::create("Rotating");
    GameObject3D* movingObj = GameObject3D::create("Moving");
    GameObject3D* scalingObj = GameObject3D::create("Scaling");
    
    rotatingObj->start();
    movingObj->start();
    scalingObj->start();
    
    // Act - симулируем анимацию
    for (int frame = 0; frame < 100; ++frame) {
        float t = frame / 100.0f;
        
        // Вращение
        Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), t * 2 * M_PI);
        rotatingObj->getTransform()->setRotation(rot);
        
        // Движение
        Vector3 pos = Vector3(t * 10, 0, 0);
        movingObj->getTransform()->setPosition(pos);
        
        // Масштабирование
        float scale = 1.0f + std::sin(t * M_PI) * 0.5f;
        scalingObj->getTransform()->setScale(Vector3(scale, scale, scale));
        
        // Update
        rotatingObj->update(0.016f);
        movingObj->update(0.016f);
        scalingObj->update(0.016f);
    }
    
    // Assert
    Vector3 finalPos = movingObj->getTransform()->getPosition();
    EXPECT_GT(finalPos.x, 9.0f);
}

// ============================================================================
// Сценарий 8: Тест производительности поиска
// ============================================================================

TEST_F(IntegrationPipelineTest, FindPerformance) {
    // Arrange - создаем много объектов
    for (int i = 0; i < 1000; ++i) {
        GameObject3D* obj = GameObject3D::create("Object_" + std::to_string(i));
        if (i % 10 == 0) {
            obj->setTag("Special");
        }
    }
    
    // Act - поиск объектов
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<GameObject3D*> specialObjects = GameObject3D::findAllWithTag("Special");
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Assert
    EXPECT_EQ(specialObjects.size(), 100);
    EXPECT_LT(duration.count(), 10000);  // Должно быть быстрее 10ms
}

// ============================================================================
// Сценарий 9: Матрицы камеры и проекции
// ============================================================================

TEST_F(IntegrationPipelineTest, CameraMatrices) {
    // Arrange
    GameObject3D* camera = GameObject3D::create("Camera");
    camera->getTransform()->setPosition(Vector3(0, 5, 10));
    camera->getTransform()->setRotation(
        Quaternion::lookRotation(Vector3(0, 0, -1), Vector3(0, 1, 0))
    );
    
    // Act - создаем матрицы view и projection
    Matrix4 viewMatrix = Matrix4::lookAt(
        camera->getTransform()->getPosition(),
        Vector3(0, 0, 0),
        Vector3(0, 1, 0)
    );
    
    Matrix4 projMatrix = Matrix4::perspective(
        M_PI / 4.0f,  // 45 градусов FOV
        16.0f / 9.0f,  // Aspect ratio
        0.1f,  // Near plane
        100.0f  // Far plane
    );
    
    Matrix4 viewProj = projMatrix * viewMatrix;
    
    // Assert
    EXPECT_FALSE(std::isnan(viewProj.m[0][0]));
}

// ============================================================================
// Сценарий 10: Стресс-тест трансформаций
// ============================================================================

TEST_F(IntegrationPipelineTest, TransformStressTest) {
    // Arrange
    GameObject3D* root = GameObject3D::create("Root");
    Transform3D* current = root->getTransform();
    
    // Act - создаем глубокую иерархию
    for (int i = 0; i < 10; ++i) {
        GameObject3D* child = GameObject3D::create("Level_" + std::to_string(i));
        child->getTransform()->setPosition(Vector3(1, 0, 0));
        current->addChild(child->getTransform());
        current = child->getTransform();
    }
    
    // Assert - самый глубокий узел должен быть на расстоянии 10 единиц от корня
    Vector3 deepestWorld = current->getWorldPosition();
    EXPECT_FLOAT_EQ(deepestWorld.x, 10.0f);
}

// ============================================================================
// Сценарий 11: Комбинированные операции
// ============================================================================

TEST_F(IntegrationPipelineTest, CombinedOperations) {
    // Arrange
    GameObject3D* obj = GameObject3D::create("Combined");
    
    // Act - выполняем множество операций подряд
    obj->setTag("TestTag");
    obj->start();
    obj->getTransform()->setPosition(Vector3(1, 2, 3));
    obj->getTransform()->translate(Vector3(1, 1, 1));
    obj->getTransform()->setScale(Vector3(2, 2, 2));
    obj->getTransform()->rotate(Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 2.0f));
    obj->update(0.016f);
    obj->render();
    
    // Assert
    Vector3 pos = obj->getTransform()->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 2.0f);
    EXPECT_FLOAT_EQ(pos.y, 3.0f);
    EXPECT_FLOAT_EQ(pos.z, 4.0f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
