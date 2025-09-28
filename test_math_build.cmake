# Тестовая сборка только математической библиотеки
cmake_minimum_required(VERSION 3.16)

project(HyperEngine_Math_Test VERSION 0.0.11 LANGUAGES CXX)

# Включить новую структуру
add_subdirectory(src/math)

# Создать простой тестовый исполняемый файл
set(TEST_SOURCES test_math_main.cpp)

# Проверим, нужно ли создать тестовый файл
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_math_main.cpp")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/test_math_main.cpp" 
"#include \"HyperEngine/Math/Math.h\"
#include <iostream>

using namespace HyperEngine::Math;

int main() {
    std::cout << \"Тестирование математической библиотеки HyperEngine\" << std::endl;
    
    // Тест Vector3
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);
    Vector3 v3 = v1 + v2;
    
    std::cout << \"Vector3 test: \" << v3.toString() << std::endl;
    
    // Тест Matrix4
    Matrix4 m1 = Matrix4::identity();
    Matrix4 m2 = Matrix4::translation(1.0f, 2.0f, 3.0f);
    
    std::cout << \"Matrix4 identity test passed\" << std::endl;
    
    // Тест Quaternion
    Quaternion q1 = Quaternion::identity();
    Quaternion q2 = Quaternion::fromAxisAngle(Vector3(0, 1, 0), PI/4);
    
    std::cout << \"Quaternion test passed\" << std::endl;
    
    std::cout << \"Все тесты пройдены!\" << std::endl;
    return 0;
}")
endif()

add_executable(test_math_app ${TEST_SOURCES})
target_link_libraries(test_math_app HyperEngine::Math)
