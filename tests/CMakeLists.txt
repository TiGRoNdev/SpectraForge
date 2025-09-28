# CMakeLists.txt для системы тестирования HyperEngine
cmake_minimum_required(VERSION 3.16)

# Настройка тестирования
enable_testing()

# Поиск Google Test и Google Mock (vcpkg style)
find_package(GTest CONFIG REQUIRED)

# Включение поддержки Google Benchmark для тестов производительности
find_package(benchmark QUIET)

# Общие настройки для всех тестов
if(MSVC)
    set(TEST_COMPILE_FLAGS
        /W4
    )
else()
    set(TEST_COMPILE_FLAGS
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fprofile-arcs -ftest-coverage  # Для code coverage
    )
endif()

set(TEST_LINK_LIBRARIES
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

# Добавление include директорий
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Функция для создания unit теста
function(add_unit_test test_name)
    cmake_parse_arguments(TEST "" "" "SOURCES;LIBS" ${ARGN})
    
    add_executable(${test_name} ${TEST_SOURCES})
    
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_compile_options(${test_name} PRIVATE ${TEST_COMPILE_FLAGS})
    
    target_link_libraries(${test_name} 
        PRIVATE 
        ${TEST_LINK_LIBRARIES}
        ${TEST_LIBS}
    )
    
    # Регистрация теста в CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Настройка рабочей директории для теста
    set_tests_properties(${test_name} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# Функция для создания интеграционного теста
function(add_integration_test test_name)
    cmake_parse_arguments(TEST "" "" "SOURCES;LIBS" ${ARGN})
    
    add_executable(${test_name} ${TEST_SOURCES})
    
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_compile_options(${test_name} PRIVATE ${TEST_COMPILE_FLAGS})
    
    target_link_libraries(${test_name} 
        PRIVATE 
        ${TEST_LINK_LIBRARIES}
        ${TEST_LIBS}
    )
    
    # Интеграционные тесты требуют больше времени
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        LABELS "integration"
    )
endfunction()

# Функция для создания теста производительности
function(add_performance_test test_name)
    if(NOT benchmark_FOUND)
        message(WARNING "Google Benchmark не найден, пропуск теста производительности ${test_name}")
        return()
    endif()
    
    cmake_parse_arguments(TEST "" "" "SOURCES;LIBS" ${ARGN})
    
    add_executable(${test_name} ${TEST_SOURCES})
    
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_compile_options(${test_name} PRIVATE ${TEST_COMPILE_FLAGS} -O3)
    
    target_link_libraries(${test_name} 
        PRIVATE 
        benchmark::benchmark
        ${TEST_LIBS}
    )
    
    # Тесты производительности не регистрируются в CTest автоматически
    # Их можно запускать отдельно для профилирования
    set_target_properties(${test_name} PROPERTIES
        FOLDER "Performance Tests"
    )
endfunction()

# Unit тесты
add_subdirectory(unit)

# Интеграционные тесты
add_subdirectory(integration)

# Тесты производительности
add_subdirectory(performance)

# Создание мета-цели для запуска всех тестов
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel 4
    COMMENT "Запуск всех тестов"
    DEPENDS ${ALL_TEST_TARGETS}
)

# Создание цели для сбора code coverage
add_custom_target(coverage
    COMMAND gcov ${CMAKE_CURRENT_BINARY_DIR}/**/*.gcno
    COMMAND lcov --capture --directory ${CMAKE_CURRENT_BINARY_DIR} --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
    COMMAND lcov --list coverage.info
    COMMAND genhtml coverage.info --output-directory coverage_html
    COMMENT "Сбор данных о покрытии кода"
    DEPENDS run_all_tests
)

# Настройка валидации кода
find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
if(CLANG_TIDY_EXECUTABLE)
    add_custom_target(tidy_tests
        COMMAND ${CLANG_TIDY_EXECUTABLE} 
            ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/**/*.h
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        COMMENT "Статический анализ тестов"
    )
endif()

# Вывод информации о настроенных тестах
message(STATUS "Система тестирования настроена:")
message(STATUS "  - Google Test: ${GTest_FOUND}")
message(STATUS "  - Google Mock: ${GMock_FOUND}")
message(STATUS "  - Google Benchmark: ${benchmark_FOUND}")
message(STATUS "  - Clang-Tidy: ${CLANG_TIDY_EXECUTABLE}")
message(STATUS "  - Code Coverage: Включено")
