# CMakeLists.txt для unit тестов
cmake_minimum_required(VERSION 3.16)

# Список всех unit тестов для регистрации в родительском CMakeLists
set(ALL_UNIT_TESTS "" PARENT_SCOPE)

# Unit тесты для математической библиотеки
add_unit_test(simple_math_test
    SOURCES
        math_tests.cpp
    LIBS
        HyperEngine_Math
)

# Unit тесты для системы рендеринга
add_unit_test(rendering_tests
    SOURCES 
        rendering/RendererAdapterTest.cpp
        rendering/VulkanRendererTest.cpp
    LIBS
        HyperEngine_Math
        HyperEngine_Core
        HyperEngine_OpenGL
        HyperEngine_Vulkan
)

# Unit тесты для основных компонентов
add_unit_test(optix_tests
    SOURCES
        test_optix_integration.cpp
    LIBS
        # Здесь будут основные библиотеки
)

# Добавляем все тесты в глобальный список
set(ALL_UNIT_TESTS 
    simple_math_test
    PARENT_SCOPE
)
