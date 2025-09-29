# CMakeLists.txt для интеграционных тестов
cmake_minimum_required(VERSION 3.16)

# Список всех интеграционных тестов
set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)

# Простой интеграционный тест
add_integration_test(simple_integration_test
    SOURCES
        rendering/CudaVulkanInteropTest.cpp
    LIBS
        HyperEngine_Math
        HyperEngine_Core
        HyperEngine_CUDA
        HyperEngine_Vulkan
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        CUDA::cudart
        CUDA::cuda_driver
        glm::glm
)

# Добавляем все тесты в глобальный список
set(ALL_INTEGRATION_TESTS
    simple_integration_test
    PARENT_SCOPE
)
