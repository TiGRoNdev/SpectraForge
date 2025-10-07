/**
 * @file input_system_test.cpp
 * @brief Комплексные тесты для системы ввода Input3D
 */

#include <gtest/gtest.h>
#include <SpectraForge/Input/Input3D.h>
#include <SpectraForge/Math/Vector3.h>
#include <SpectraForge/Math/Quaternion.h>

using namespace SpectraForge::Input;
using namespace SpectraForge::Math;

// ============================================================================
// InputAction3D Tests
// ============================================================================

class InputAction3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        action = std::make_unique<InputAction3D>("TestAction");
    }

    std::unique_ptr<InputAction3D> action;
};

TEST_F(InputAction3DTest, DefaultConstructor) {
    // Arrange & Act
    InputAction3D act;
    
    // Assert
    EXPECT_TRUE(act.isEnabled());
}

TEST_F(InputAction3DTest, NamedConstructor) {
    // Arrange & Act
    InputAction3D act("MyAction");
    
    // Assert
    EXPECT_EQ(act.getName(), "MyAction");
}

TEST_F(InputAction3DTest, GetName) {
    // Arrange & Act & Assert
    EXPECT_EQ(action->getName(), "TestAction");
}

TEST_F(InputAction3DTest, AddKey) {
    // Arrange & Act & Assert - не должно падать
    EXPECT_NO_THROW(action->addKey(Key3D::W));
    EXPECT_NO_THROW(action->addKey(Key3D::A));
    EXPECT_NO_THROW(action->addKey(Key3D::S));
    EXPECT_NO_THROW(action->addKey(Key3D::D));
}

TEST_F(InputAction3DTest, AddMouseButton) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(action->addMouseButton(MouseButton3D::Left));
    EXPECT_NO_THROW(action->addMouseButton(MouseButton3D::Right));
}

TEST_F(InputAction3DTest, SetOnPressed) {
    // Arrange
    bool called = false;
    auto callback = [&called]() { called = true; };
    
    // Act
    action->setOnPressed(callback);
    action->executePressed();
    
    // Assert
    EXPECT_TRUE(called);
}

TEST_F(InputAction3DTest, SetOnReleased) {
    // Arrange
    bool called = false;
    auto callback = [&called]() { called = true; };
    
    // Act
    action->setOnReleased(callback);
    action->executeReleased();
    
    // Assert
    EXPECT_TRUE(called);
}

TEST_F(InputAction3DTest, SetOnHeld) {
    // Arrange
    bool called = false;
    auto callback = [&called]() { called = true; };
    
    // Act
    action->setOnHeld(callback);
    action->executeHeld();
    
    // Assert
    EXPECT_TRUE(called);
}

TEST_F(InputAction3DTest, SetEnabled) {
    // Arrange & Act
    action->setEnabled(false);
    
    // Assert
    EXPECT_FALSE(action->isEnabled());
}

TEST_F(InputAction3DTest, IsEnabled) {
    // Arrange & Act & Assert
    EXPECT_TRUE(action->isEnabled());
    
    action->setEnabled(false);
    EXPECT_FALSE(action->isEnabled());
}

TEST_F(InputAction3DTest, IsTriggered) {
    // Arrange
    InputState3D state;
    action->addKey(Key3D::W);
    state.keyStates[Key3D::W] = true;
    
    // Act
    bool triggered = action->isTriggered(state);
    
    // Assert
    EXPECT_TRUE(triggered);
}

TEST_F(InputAction3DTest, WasJustPressed) {
    // Arrange
    InputState3D state;
    action->addKey(Key3D::Space);
    state.keyJustPressed[Key3D::Space] = true;
    
    // Act
    bool justPressed = action->wasJustPressed(state);
    
    // Assert
    EXPECT_TRUE(justPressed);
}

TEST_F(InputAction3DTest, WasJustReleased) {
    // Arrange
    InputState3D state;
    action->addKey(Key3D::Enter);
    state.keyJustReleased[Key3D::Enter] = true;
    
    // Act
    bool justReleased = action->wasJustReleased(state);
    
    // Assert
    EXPECT_TRUE(justReleased);
}

TEST_F(InputAction3DTest, DisabledActionNotTriggered) {
    // Arrange
    InputState3D state;
    action->addKey(Key3D::W);
    action->setEnabled(false);
    state.keyStates[Key3D::W] = true;
    
    // Act
    bool triggered = action->isTriggered(state);
    
    // Assert
    EXPECT_FALSE(triggered);
}

// ============================================================================
// Controller3D Tests
// ============================================================================

class Controller3DTest : public ::testing::Test {
protected:
    void SetUp() override {
        controller = std::make_unique<Controller3D>();
        epsilon = 0.001f;
    }

    bool vectorsEqual(const Vector3& a, const Vector3& b) {
        return std::abs(a.x - b.x) < epsilon &&
               std::abs(a.y - b.y) < epsilon &&
               std::abs(a.z - b.z) < epsilon;
    }

    std::unique_ptr<Controller3D> controller;
    float epsilon;
};

TEST_F(Controller3DTest, Constructor) {
    // Arrange & Act
    Controller3D ctrl;
    
    // Assert
    EXPECT_TRUE(ctrl.isMovementEnabled());
    EXPECT_TRUE(ctrl.isRotationEnabled());
}

TEST_F(Controller3DTest, GetPosition) {
    // Arrange & Act
    Vector3 pos = controller->getPosition();
    
    // Assert
    EXPECT_TRUE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Controller3DTest, SetPosition) {
    // Arrange
    Vector3 pos(1, 2, 3);
    
    // Act
    controller->setPosition(pos);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(controller->getPosition(), pos));
}

TEST_F(Controller3DTest, GetRotation) {
    // Arrange & Act
    Quaternion rot = controller->getRotation();
    
    // Assert
    EXPECT_TRUE(rot.isIdentity());
}

TEST_F(Controller3DTest, SetRotation) {
    // Arrange
    Quaternion rot = Quaternion::fromAxisAngle(Vector3::unitY(), M_PI / 4.0f);
    
    // Act
    controller->setRotation(rot);
    
    // Assert
    Quaternion result = controller->getRotation();
    EXPECT_FLOAT_EQ(result.w, rot.w);
}

TEST_F(Controller3DTest, Move) {
    // Arrange
    Vector3 movement(1, 0, 0);
    
    // Act
    controller->move(movement);
    
    // Assert
    Vector3 pos = controller->getPosition();
    EXPECT_TRUE(vectorsEqual(pos, movement));
}

TEST_F(Controller3DTest, MoveForward) {
    // Arrange
    controller->setPosition(Vector3::zero());
    
    // Act
    controller->moveForward(5.0f);
    
    // Assert
    Vector3 pos = controller->getPosition();
    EXPECT_FALSE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Controller3DTest, MoveRight) {
    // Arrange
    controller->setPosition(Vector3::zero());
    
    // Act
    controller->moveRight(3.0f);
    
    // Assert
    Vector3 pos = controller->getPosition();
    EXPECT_FALSE(vectorsEqual(pos, Vector3::zero()));
}

TEST_F(Controller3DTest, MoveUp) {
    // Arrange & Act
    controller->moveUp(2.0f);
    
    // Assert
    Vector3 pos = controller->getPosition();
    EXPECT_GT(pos.y, 0.0f);
}

TEST_F(Controller3DTest, RotateX) {
    // Arrange & Act
    controller->rotateX(M_PI / 4.0f);
    
    // Assert
    Quaternion rot = controller->getRotation();
    EXPECT_FALSE(rot.isIdentity());
}

TEST_F(Controller3DTest, RotateY) {
    // Arrange & Act
    controller->rotateY(M_PI / 4.0f);
    
    // Assert
    Quaternion rot = controller->getRotation();
    EXPECT_FALSE(rot.isIdentity());
}

TEST_F(Controller3DTest, RotateZ) {
    // Arrange & Act
    controller->rotateZ(M_PI / 4.0f);
    
    // Assert
    Quaternion rot = controller->getRotation();
    EXPECT_FALSE(rot.isIdentity());
}

TEST_F(Controller3DTest, Rotate) {
    // Arrange & Act
    controller->rotate(M_PI / 6.0f, M_PI / 4.0f, 0.0f);
    
    // Assert
    Quaternion rot = controller->getRotation();
    EXPECT_FALSE(rot.isIdentity());
}

TEST_F(Controller3DTest, LookAt) {
    // Arrange
    Vector3 target(0, 0, -10);
    
    // Act
    controller->lookAt(target);
    
    // Assert
    Vector3 forward = controller->getForward();
    EXPECT_FALSE(std::isnan(forward.z));
}

TEST_F(Controller3DTest, GetForward) {
    // Arrange & Act
    Vector3 forward = controller->getForward();
    
    // Assert
    EXPECT_FALSE(std::isnan(forward.x));
}

TEST_F(Controller3DTest, GetRight) {
    // Arrange & Act
    Vector3 right = controller->getRight();
    
    // Assert
    EXPECT_FALSE(std::isnan(right.x));
}

TEST_F(Controller3DTest, GetUp) {
    // Arrange & Act
    Vector3 up = controller->getUp();
    
    // Assert
    EXPECT_FALSE(std::isnan(up.y));
}

TEST_F(Controller3DTest, GetTransformMatrix) {
    // Arrange
    controller->setPosition(Vector3(1, 2, 3));
    
    // Act
    Matrix4 transform = controller->getTransformMatrix();
    
    // Assert
    EXPECT_FALSE(std::isnan(transform.m[0][0]));
}

TEST_F(Controller3DTest, SetMoveSpeed) {
    // Arrange & Act
    controller->setMoveSpeed(10.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getMoveSpeed(), 10.0f);
}

TEST_F(Controller3DTest, SetRotationSpeed) {
    // Arrange & Act
    controller->setRotationSpeed(2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getRotationSpeed(), 2.0f);
}

TEST_F(Controller3DTest, SetMouseSensitivity) {
    // Arrange & Act
    controller->setMouseSensitivity(0.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getMouseSensitivity(), 0.5f);
}

TEST_F(Controller3DTest, SetMouseLookEnabled) {
    // Arrange & Act
    controller->setMouseLookEnabled(false);
    
    // Assert
    EXPECT_FALSE(controller->isMouseLookEnabled());
}

TEST_F(Controller3DTest, SetMovementEnabled) {
    // Arrange & Act
    controller->setMovementEnabled(false);
    
    // Assert
    EXPECT_FALSE(controller->isMovementEnabled());
}

TEST_F(Controller3DTest, SetRotationEnabled) {
    // Arrange & Act
    controller->setRotationEnabled(false);
    
    // Assert
    EXPECT_FALSE(controller->isRotationEnabled());
}

TEST_F(Controller3DTest, Update) {
    // Arrange
    float deltaTime = 0.016f;
    
    // Act & Assert - не должно падать
    EXPECT_NO_THROW(controller->update(deltaTime));
}

TEST_F(Controller3DTest, HandleInput) {
    // Arrange
    InputState3D state;
    
    // Act & Assert
    EXPECT_NO_THROW(controller->handleInput(state));
}

TEST_F(Controller3DTest, SetVelocity) {
    // Arrange
    Vector3 vel(1, 2, 3);
    
    // Act
    controller->setVelocity(vel);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(controller->getVelocity(), vel));
}

// ============================================================================
// FirstPersonController Tests
// ============================================================================

class FirstPersonControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        controller = std::make_unique<FirstPersonController>();
    }

    std::unique_ptr<FirstPersonController> controller;
};

TEST_F(FirstPersonControllerTest, Constructor) {
    // Arrange & Act
    FirstPersonController ctrl;
    
    // Assert
    EXPECT_TRUE(ctrl.isMovementEnabled());
}

TEST_F(FirstPersonControllerTest, SetPitchLimit) {
    // Arrange & Act & Assert - не должно падать
    EXPECT_NO_THROW(controller->setPitchLimit(-M_PI / 2.0f, M_PI / 2.0f));
}

TEST_F(FirstPersonControllerTest, Jump) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(controller->jump());
}

TEST_F(FirstPersonControllerTest, SetJumpHeight) {
    // Arrange & Act
    controller->setJumpHeight(3.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getJumpHeight(), 3.0f);
}

TEST_F(FirstPersonControllerTest, SetGravity) {
    // Arrange & Act
    controller->setGravity(-20.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getGravity(), -20.0f);
}

TEST_F(FirstPersonControllerTest, IsGrounded) {
    // Arrange & Act & Assert
    EXPECT_TRUE(controller->isGrounded());
}

TEST_F(FirstPersonControllerTest, SetGrounded) {
    // Arrange & Act
    controller->setGrounded(false);
    
    // Assert
    EXPECT_FALSE(controller->isGrounded());
}

TEST_F(FirstPersonControllerTest, HandleInput) {
    // Arrange
    InputState3D state;
    
    // Act & Assert
    EXPECT_NO_THROW(controller->handleInput(state));
}

// ============================================================================
// OrbitController Tests
// ============================================================================

class OrbitControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        controller = std::make_unique<OrbitController>();
        epsilon = 0.001f;
    }

    bool vectorsEqual(const Vector3& a, const Vector3& b) {
        return std::abs(a.x - b.x) < epsilon &&
               std::abs(a.y - b.y) < epsilon &&
               std::abs(a.z - b.z) < epsilon;
    }

    std::unique_ptr<OrbitController> controller;
    float epsilon;
};

TEST_F(OrbitControllerTest, Constructor) {
    // Arrange & Act
    OrbitController ctrl;
    
    // Assert
    EXPECT_TRUE(ctrl.isMovementEnabled());
}

TEST_F(OrbitControllerTest, SetTarget) {
    // Arrange
    Vector3 target(5, 0, 0);
    
    // Act
    controller->setTarget(target);
    
    // Assert
    EXPECT_TRUE(vectorsEqual(controller->getTarget(), target));
}

TEST_F(OrbitControllerTest, GetTarget) {
    // Arrange & Act
    Vector3 target = controller->getTarget();
    
    // Assert
    EXPECT_FALSE(std::isnan(target.x));
}

TEST_F(OrbitControllerTest, SetDistance) {
    // Arrange & Act
    controller->setDistance(10.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getDistance(), 10.0f);
}

TEST_F(OrbitControllerTest, SetDistanceRange) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(controller->setDistanceRange(1.0f, 100.0f));
}

TEST_F(OrbitControllerTest, SetPitchRange) {
    // Arrange & Act & Assert
    EXPECT_NO_THROW(controller->setPitchRange(-M_PI / 3.0f, M_PI / 3.0f));
}

TEST_F(OrbitControllerTest, SetZoomSpeed) {
    // Arrange & Act
    controller->setZoomSpeed(2.0f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getZoomSpeed(), 2.0f);
}

TEST_F(OrbitControllerTest, SetOrbitSpeed) {
    // Arrange & Act
    controller->setOrbitSpeed(1.5f);
    
    // Assert
    EXPECT_FLOAT_EQ(controller->getOrbitSpeed(), 1.5f);
}

TEST_F(OrbitControllerTest, HandleInput) {
    // Arrange
    InputState3D state;
    
    // Act & Assert
    EXPECT_NO_THROW(controller->handleInput(state));
}

// ============================================================================
// Key and Button Enums Tests
// ============================================================================

TEST(InputEnumsTest, Key3DValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(Key3D::A), 65);
    EXPECT_EQ(static_cast<int>(Key3D::W), 87);
    EXPECT_EQ(static_cast<int>(Key3D::Space), 32);
    EXPECT_EQ(static_cast<int>(Key3D::Enter), 257);
}

TEST(InputEnumsTest, MouseButton3DValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(MouseButton3D::Left), 0);
    EXPECT_EQ(static_cast<int>(MouseButton3D::Right), 1);
    EXPECT_EQ(static_cast<int>(MouseButton3D::Middle), 2);
}

TEST(InputEnumsTest, KeyState3DValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(KeyState3D::Released), 0);
    EXPECT_EQ(static_cast<int>(KeyState3D::Pressed), 1);
    EXPECT_EQ(static_cast<int>(KeyState3D::Held), 2);
}

// ============================================================================
// InputState3D Tests
// ============================================================================

TEST(InputState3DTest, DefaultConstruction) {
    // Arrange & Act
    InputState3D state;
    
    // Assert
    EXPECT_TRUE(state.keyStates.empty());
    EXPECT_TRUE(state.mouseStates.empty());
}

TEST(InputState3DTest, SetKeyState) {
    // Arrange
    InputState3D state;
    
    // Act
    state.keyStates[Key3D::W] = true;
    state.keyJustPressed[Key3D::W] = true;
    
    // Assert
    EXPECT_TRUE(state.keyStates[Key3D::W]);
    EXPECT_TRUE(state.keyJustPressed[Key3D::W]);
}

TEST(InputState3DTest, SetMouseState) {
    // Arrange
    InputState3D state;
    
    // Act
    state.mouseStates[MouseButton3D::Left] = true;
    state.mouseJustPressed[MouseButton3D::Left] = true;
    
    // Assert
    EXPECT_TRUE(state.mouseStates[MouseButton3D::Left]);
    EXPECT_TRUE(state.mouseJustPressed[MouseButton3D::Left]);
}

TEST(InputState3DTest, MousePosition) {
    // Arrange
    InputState3D state;
    
    // Act
    state.mousePosition = Vector3(100, 200, 0);
    
    // Assert
    EXPECT_FLOAT_EQ(state.mousePosition.x, 100.0f);
    EXPECT_FLOAT_EQ(state.mousePosition.y, 200.0f);
}

TEST(InputState3DTest, MouseDelta) {
    // Arrange
    InputState3D state;
    
    // Act
    state.mouseDelta = Vector3(10, -5, 0);
    
    // Assert
    EXPECT_FLOAT_EQ(state.mouseDelta.x, 10.0f);
    EXPECT_FLOAT_EQ(state.mouseDelta.y, -5.0f);
}

TEST(InputState3DTest, ScrollDelta) {
    // Arrange
    InputState3D state;
    
    // Act
    state.scrollDelta = Vector3(0, 1, 0);
    
    // Assert
    EXPECT_FLOAT_EQ(state.scrollDelta.y, 1.0f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
