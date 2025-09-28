# CMakeLists.txt для unit тестов
cmake_minimum_required(VERSION 3.16)

# Список всех unit тестов для регистрации в родительском CMakeLists
set(ALL_UNIT_TESTS "" PARENT_SCOPE)

# Unit тесты для математической библиотеки
add_unit_test(math_tests
    SOURCES
        math/Vector3Test.cpp
        math/Matrix4Test.cpp
        math/QuaternionTest.cpp
    LIBS
        # Здесь будет HyperEngine_Math когда он будет создан
)

# Unit тесты для системы рендеринга
add_unit_test(rendering_tests
    SOURCES 
        rendering/RendererAdapterTest.cpp
        rendering/VulkanRendererTest.cpp
        rendering/UpscalingTest.cpp
    LIBS
        # Здесь будут библиотеки рендеринга
)

# Unit тесты для основных компонентов
add_unit_test(core_tests
    SOURCES
        core/InterfacesTest.cpp
        core/GameObjectTest.cpp
        core/ComponentTest.cpp
    LIBS
        # Здесь будут основные библиотеки
)

# Добавляем все тесты в глобальный список
set(ALL_UNIT_TESTS 
    math_tests
    rendering_tests
    core_tests
    PARENT_SCOPE
)
