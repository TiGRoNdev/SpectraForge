# CMakeLists.txt для интеграционных тестов
cmake_minimum_required(VERSION 3.16)

# Список всех интеграционных тестов
set(ALL_INTEGRATION_TESTS "" PARENT_SCOPE)

# Интеграционные тесты для rendering pipeline
add_integration_test(rendering_pipeline_test
    SOURCES
        rendering/RenderingPipelineIntegrationTest.cpp
    LIBS
        # Библиотеки рендеринга
)

# Интеграционные тесты для CUDA-Vulkan interop
add_integration_test(cuda_vulkan_interop_test
    SOURCES
        rendering/CudaVulkanInteropTest.cpp
    LIBS
        # CUDA и Vulkan библиотеки
)

# Интеграционные тесты для полного движка
add_integration_test(engine_integration_test
    SOURCES
        EngineIntegrationTest.cpp
    LIBS
        # Все библиотеки движка
)

# Добавляем все тесты в глобальный список
set(ALL_INTEGRATION_TESTS
    rendering_pipeline_test
    cuda_vulkan_interop_test
    engine_integration_test
    PARENT_SCOPE
)
