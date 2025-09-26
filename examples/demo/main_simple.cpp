#include <iostream>
#include <Engine4D/Math/Vector4.h>
#include <Engine4D/Math/Matrix4.h>
#include <Engine4D/Math/Quaternion4D.h>
#include <Engine4D/Core/GameObject4D.h>
#include <chrono>

using namespace Engine4D;
using namespace Engine4D::Math;
using namespace Engine4D::Core;

int main() {
    std::cout << "=== 4D Game Engine Demo (Simple) ===" << std::endl;
    std::cout << "Тестирование основных компонентов движка..." << std::endl;

    // Тестирование Vector4
    std::cout << "\n--- Тестирование Vector4 ---" << std::endl;
    Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 v2(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 v3 = v1 + v2;
    std::cout << "v1: " << v1 << std::endl;
    std::cout << "v2: " << v2 << std::endl;
    std::cout << "v1 + v2: " << v3 << std::endl;
    std::cout << "v1.dot(v2): " << v1.dot(v2) << std::endl;
    std::cout << "v1.magnitude(): " << v1.magnitude() << std::endl;

    // Тестирование Matrix4
    std::cout << "\n--- Тестирование Matrix4 ---" << std::endl;
    Matrix4 m1 = Matrix4::identity();
    Matrix4 m2 = Matrix4::translation(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    Matrix4 m3 = m1 * m2;
    std::cout << "Identity matrix: " << m1 << std::endl;
    std::cout << "Translation matrix: " << m2 << std::endl;
    std::cout << "Identity * Translation: " << m3 << std::endl;

    // Тестирование Quaternion4D
    std::cout << "\n--- Тестирование Quaternion4D ---" << std::endl;
    Quaternion4D q1 = Quaternion4D::identity();
    Quaternion4D q2 = Quaternion4D::rotationXY(1.57f); // 90 градусов
    std::cout << "Identity quaternion: " << q1 << std::endl;
    std::cout << "Rotation quaternion: " << q2 << std::endl;
    std::cout << "q2.magnitude(): " << q2.magnitude() << std::endl;

    // Тестирование GameObject4D
    std::cout << "\n--- Тестирование GameObject4D ---" << std::endl;
    GameObject4D* obj = GameObject4D::create("TestObject");
    std::cout << "Created object: " << obj->name << std::endl;
    std::cout << "Object position: " << obj->transform->position << std::endl;
    std::cout << "Object scale: " << obj->transform->scale << std::endl;

    // Тестирование Transform4D
    std::cout << "\n--- Тестирование Transform4D ---" << std::endl;
    obj->transform->setPosition(Vector4(10.0f, 20.0f, 30.0f, 40.0f));
    obj->transform->setScale(Vector4(2.0f, 2.0f, 2.0f, 2.0f));
    std::cout << "After setting position and scale:" << std::endl;
    std::cout << "Position: " << obj->transform->position << std::endl;
    std::cout << "Scale: " << obj->transform->scale << std::endl;
    std::cout << "Local matrix: " << obj->transform->getLocalMatrix() << std::endl;

    // Очистка
    delete obj;

    std::cout << "\n=== Все тесты завершены успешно! ===" << std::endl;
    return 0;
}
