# CMakeLists.txt для интеграционных тестов
cmake_minimum_required(VERSION 3.16)

# Список всех интеграционных тестов
set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)

# Простой интеграционный тест
set(INTEGRATION_TEST_LIBS
    SpectraForge_Math
    SpectraForge_Core
    glm::glm
)

# Добавляем Vulkan зависимости только если Vulkan доступен
if(TARGET SpectraForge_Vulkan AND TARGET Vulkan::Vulkan)
    list(APPEND INTEGRATION_TEST_LIBS
        SpectraForge_Vulkan
        Vulkan::Vulkan
    )
    
    # Добавляем VMA если доступен
    if(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
        list(APPEND INTEGRATION_TEST_LIBS VulkanMemoryAllocator::VulkanMemoryAllocator)
    elseif(TARGET GPUOpen::VulkanMemoryAllocator)
        list(APPEND INTEGRATION_TEST_LIBS GPUOpen::VulkanMemoryAllocator)
    endif()
endif()

# Добавляем CUDA зависимости только если CUDA доступна
if(BUILD_WITH_CUDA AND TARGET SpectraForge_CUDA)
    list(APPEND INTEGRATION_TEST_LIBS
        SpectraForge_CUDA
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
    # simple_integration_test использует SpectraForge_Core (окна/инициализация Vulkan через GLFW)
    if(TARGET glfw)
        target_link_libraries(simple_integration_test PRIVATE glfw)
    endif()
    # Гарантируем порядок разрешения статических библиотек
    target_link_options(simple_integration_test PRIVATE -Wl,--start-group -Wl,--end-group)
    
    # NEW: Full Pipeline Integration Test (CRITICAL for 80% coverage)
    add_integration_test(full_pipeline_integration_test
        SOURCES
            rendering/FullPipelineIntegrationTest.cpp
        LIBS
            ${INTEGRATION_TEST_LIBS}
    )
    if(TARGET glfw)
        target_link_libraries(full_pipeline_integration_test PRIVATE glfw)
    endif()
    target_link_options(full_pipeline_integration_test PRIVATE -Wl,--start-group -Wl,--end-group)
    
    set(ALL_INTEGRATION_TESTS 
        simple_integration_test
        full_pipeline_integration_test
        PARENT_SCOPE)
else()
    message(STATUS "Пропуск integration tests - требуется GMock")
    set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)
endif()
