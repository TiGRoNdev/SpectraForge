# CMakeLists.txt для интеграционных тестов
cmake_minimum_required(VERSION 3.16)

# Список всех интеграционных тестов
set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)

# Простой интеграционный тест
set(INTEGRATION_TEST_LIBS
    HyperEngine_Math
    HyperEngine_Core
    HyperEngine_Vulkan
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    glm::glm
)

# Добавляем CUDA зависимости только если CUDA доступна
if(BUILD_WITH_CUDA AND TARGET HyperEngine_CUDA)
    list(APPEND INTEGRATION_TEST_LIBS
        HyperEngine_CUDA
        CUDA::cudart
        CUDA::cuda_driver
    )
endif()

add_integration_test(simple_integration_test
    SOURCES
        rendering/CudaVulkanInteropTest.cpp
    LIBS
        ${INTEGRATION_TEST_LIBS}
)

# Добавляем все тесты в глобальный список
set(ALL_INTEGRATION_TESTS
    simple_integration_test
    PARENT_SCOPE
)
