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

# Дополнительные тесты для математической библиотеки
add_unit_test(math_vector4_test
    SOURCES
        math/Vector4Tests.cpp
    LIBS
        HyperEngine_Math
)

add_unit_test(math_matrix4_test
    SOURCES
        math/Matrix4Tests.cpp
    LIBS
        HyperEngine_Math
)

# Unit тесты для системы рендеринга
set(RENDERING_TEST_LIBS
    HyperEngine_Math
    HyperEngine_Core
    HyperEngine_OpenGL
)

# Добавляем Vulkan только если он включен
if(TARGET HyperEngine_Vulkan)
    list(APPEND RENDERING_TEST_LIBS HyperEngine_Vulkan)
endif()

# Добавляем условную компиляцию для Vulkan тестов
if(TARGET HyperEngine_Vulkan)
    add_compile_definitions(HyperEngine_ENABLE_VULKAN)
endif()

# rendering_tests требует GMock для моков - пропускаем если GMock недоступен
if(GMOCK_FOUND)
    add_unit_test(rendering_tests
        SOURCES
            rendering/RendererAdapterTest.cpp
            rendering/VulkanRendererTest.cpp
        LIBS
            ${RENDERING_TEST_LIBS}
            glfw
            GLEW::GLEW
            glm::glm
    )
    list(APPEND ALL_UNIT_TESTS rendering_tests)
else()
    message(STATUS "Пропуск rendering_tests - требуется GMock")
endif()

# Unit тесты для основных компонентов (только если CUDA доступна)
if(TARGET CUDA::cudart)
    add_unit_test(optix_tests
        SOURCES
            test_optix_integration.cpp
        LIBS
            HyperEngine_Math
            HyperEngine_Core
            CUDA::cudart
            CUDA::cuda_driver
            glm::glm
    )
    list(APPEND ALL_UNIT_TESTS optix_tests)
endif()

# Добавляем все тесты в глобальный список
set(ALL_UNIT_TESTS
    simple_math_test
    math_vector4_test
    math_matrix4_test
    ${ALL_UNIT_TESTS}  # Включаем условные тесты (rendering_tests, optix_tests)
    PARENT_SCOPE
)
