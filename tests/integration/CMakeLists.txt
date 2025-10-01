# CMakeLists.txt для интеграционных тестов
cmake_minimum_required(VERSION 3.16)

# Список всех интеграционных тестов
set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)

# Простой интеграционный тест
set(INTEGRATION_TEST_LIBS
    HyperEngine_Math
    HyperEngine_Core
    glm::glm
)

# Добавляем Vulkan зависимости только если Vulkan доступен
if(TARGET HyperEngine_Vulkan AND TARGET Vulkan::Vulkan)
    list(APPEND INTEGRATION_TEST_LIBS
        HyperEngine_Vulkan
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
    )
endif()

# Добавляем CUDA зависимости только если CUDA доступна
if(BUILD_WITH_CUDA AND TARGET HyperEngine_CUDA)
    list(APPEND INTEGRATION_TEST_LIBS
        HyperEngine_CUDA
        CUDA::cudart
        CUDA::cuda_driver
    )
endif()

# simple_integration_test требует GMock для моков - пропускаем если GMock недоступен
if(GMOCK_FOUND)
    add_integration_test(simple_integration_test
        SOURCES
            rendering/CudaVulkanInteropTest.cpp
        LIBS
            ${INTEGRATION_TEST_LIBS}
    )
    
    # NEW: Full Pipeline Integration Test (CRITICAL for 80% coverage)
    add_integration_test(full_pipeline_integration_test
        SOURCES
            rendering/FullPipelineIntegrationTest.cpp
        LIBS
            ${INTEGRATION_TEST_LIBS}
    )
    
    set(ALL_INTEGRATION_TESTS 
        simple_integration_test
        full_pipeline_integration_test
        PARENT_SCOPE)
else()
    message(STATUS "Пропуск integration tests - требуется GMock")
    set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)
endif()
