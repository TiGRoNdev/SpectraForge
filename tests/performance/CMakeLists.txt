# CMakeLists.txt для тестов производительности
cmake_minimum_required(VERSION 3.16)

# Проверяем наличие Google Benchmark
if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark не найден, пропуск тестов производительности")
    return()
endif()

# Список всех тестов производительности
set(ALL_PERFORMANCE_TESTS "" PARENT_SCOPE)

# Тесты производительности для математической библиотеки
add_performance_test(math_performance_tests
    SOURCES
        math/MathPerformanceTest.cpp
    LIBS
        # Здесь будет SpectraForge_Math когда он будет создан
)

# Тесты производительности для системы рендеринга
add_performance_test(rendering_performance_tests
    SOURCES
        rendering/RenderingPerformanceTest.cpp
        rendering/VulkanPerformanceTest.cpp
    LIBS
        # Библиотеки рендеринга
)

# Тесты производительности FreqVox (не требует Google Benchmark)
if(ENABLE_FREQVOX AND TARGET SpectraForge_FreqVox)
    add_executable(freqvox_benchmark
        FreqVoxBenchmark.cpp
    )
    target_link_libraries(freqvox_benchmark PRIVATE
        SpectraForge_FreqVox
        SpectraForge_Core
        SpectraForge_Math
    )
    target_include_directories(freqvox_benchmark PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    target_compile_features(freqvox_benchmark PRIVATE cxx_std_20)
    
    message(STATUS "FreqVox benchmark добавлен (не требует Google Benchmark)")
endif()

# Добавляем все тесты в глобальный список
set(ALL_PERFORMANCE_TESTS
    math_performance_tests
    rendering_performance_tests
    PARENT_SCOPE
)

# Создание цели для запуска всех бенчмарков
add_custom_target(run_benchmarks
    COMMENT "Запуск всех тестов производительности"
)

# Добавляем зависимости
if(TARGET math_performance_tests)
    add_dependencies(run_benchmarks math_performance_tests)
    add_custom_command(TARGET run_benchmarks POST_BUILD
        COMMAND math_performance_tests --benchmark_format=json --benchmark_out=math_benchmark.json
        COMMENT "Запуск бенчмарков математической библиотеки"
    )
endif()

if(TARGET rendering_performance_tests)
    add_dependencies(run_benchmarks rendering_performance_tests)
    add_custom_command(TARGET run_benchmarks POST_BUILD
        COMMAND rendering_performance_tests --benchmark_format=json --benchmark_out=rendering_benchmark.json
        COMMENT "Запуск бенчмарков рендеринга"
    )
endif()

# Создание отчета о производительности
add_custom_target(performance_report
    COMMAND ${CMAKE_COMMAND} -E echo "Генерация отчета о производительности..."
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_performance_report.cmake
    DEPENDS run_benchmarks
    COMMENT "Генерация отчета о производительности"
)

message(STATUS "Тесты производительности настроены с Google Benchmark")
