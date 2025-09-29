# CMakeLists.txt для системы тестирования HyperEngine
cmake_minimum_required(VERSION 3.16)

# Настройка тестирования
enable_testing()

# Поиск Google Test и Google Mock (vcpkg style)
find_package(GTest CONFIG REQUIRED)

# Включение поддержки Google Benchmark для тестов производительности
find_package(benchmark QUIET)

# Настройка code coverage
option(ENABLE_CODE_COVERAGE "Enable code coverage for tests" ON)

# Принудительно используем Clang для тестов с coverage
if(ENABLE_CODE_COVERAGE)
    # Находим Clang компилятор
    find_program(CLANG_CXX_EXECUTABLE clang++ PATHS "C:/Program Files/LLVM/bin")
    find_program(CLANG_C_EXECUTABLE clang PATHS "C:/Program Files/LLVM/bin")

    if(CLANG_CXX_EXECUTABLE AND CLANG_C_EXECUTABLE)
        message(STATUS "Используем Clang для тестов с coverage: ${CLANG_CXX_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER ${CLANG_CXX_EXECUTABLE})
        set(CMAKE_C_COMPILER ${CLANG_C_EXECUTABLE})
    else()
        message(WARNING "Clang не найден, coverage может не работать")
    endif()
endif()

# Общие настройки для всех тестов
if(ENABLE_CODE_COVERAGE AND CLANG_CXX_EXECUTABLE)
    set(TEST_COMPILE_FLAGS
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -fprofile-instr-generate
        -fcoverage-mapping
        -g
    )
    set(TEST_LINK_FLAGS "-fprofile-instr-generate")
elseif(MSVC)
    set(TEST_COMPILE_FLAGS
        /W4
    )
    if(ENABLE_CODE_COVERAGE)
        # Для MSVC используем встроенные флаги coverage
        list(APPEND TEST_COMPILE_FLAGS /Zi)
        set(TEST_LINK_FLAGS "/PROFILE")
    endif()
else()
    set(TEST_COMPILE_FLAGS
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
    
    # Включаем code coverage только если явно запрошено
    if(ENABLE_CODE_COVERAGE)
        # Для Clang/GCC используем флаги coverage
        list(APPEND TEST_COMPILE_FLAGS
            -fprofile-instr-generate
            -fcoverage-mapping
            -g
        )
        set(TEST_LINK_FLAGS "-fprofile-instr-generate")
    endif()
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

    if(TEST_LINK_FLAGS)
        target_link_options(${test_name} PRIVATE ${TEST_LINK_FLAGS})
    endif()

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

    # Настройка переменных окружения для coverage
    if(ENABLE_CODE_COVERAGE)
        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT "LLVM_PROFILE_FILE=${test_name}.profraw"
        )
    endif()
endfunction()

# Функция для создания интеграционного теста
function(add_integration_test test_name)
    cmake_parse_arguments(TEST "" "" "SOURCES;LIBS" ${ARGN})

    add_executable(${test_name} ${TEST_SOURCES})

    target_compile_features(${test_name} PRIVATE cxx_std_20)
    target_compile_options(${test_name} PRIVATE ${TEST_COMPILE_FLAGS})

    if(TEST_LINK_FLAGS)
        target_link_options(${test_name} PRIVATE ${TEST_LINK_FLAGS})
    endif()

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

    # Настройка переменных окружения для coverage
    if(ENABLE_CODE_COVERAGE)
        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT "LLVM_PROFILE_FILE=${test_name}.profraw"
        )
    endif()
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
if(ENABLE_CODE_COVERAGE)
    # Находим llvm-cov и llvm-profdata
    find_program(LLVM_COV_EXECUTABLE llvm-cov PATHS "C:/Program Files/LLVM/bin")
    find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata PATHS "C:/Program Files/LLVM/bin")

    if(LLVM_COV_EXECUTABLE AND LLVM_PROFDATA_EXECUTABLE)
        message(STATUS "Найдены инструменты coverage: llvm-cov, llvm-profdata")

        # Создаем директорию для отчетов
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/coverage)

        # Цель для сбора данных coverage
        add_custom_target(coverage_collect
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/coverage/*.profraw
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/*.profraw ${CMAKE_CURRENT_BINARY_DIR}/coverage/ 2>NUL || echo "No profraw files to copy"
            COMMENT "Сбор данных coverage"
            DEPENDS run_all_tests
        )

        # Цель для генерации отчета coverage
        add_custom_target(coverage_report
            COMMAND ${LLVM_PROFDATA_EXECUTABLE} merge -sparse ${CMAKE_CURRENT_BINARY_DIR}/coverage/*.profraw -o ${CMAKE_CURRENT_BINARY_DIR}/coverage/merged.profdata
            COMMAND ${LLVM_COV_EXECUTABLE} show ${CMAKE_CURRENT_BINARY_DIR}/unit/simple_math_test.exe -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/coverage/merged.profdata -format=html -output-dir=${CMAKE_CURRENT_BINARY_DIR}/coverage/html
            COMMAND ${LLVM_COV_EXECUTABLE} report ${CMAKE_CURRENT_BINARY_DIR}/unit/simple_math_test.exe -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/coverage/merged.profdata
            COMMENT "Генерация отчета coverage"
            DEPENDS coverage_collect
        )

        # Общая цель coverage
        add_custom_target(coverage
            DEPENDS coverage_report
            COMMENT "Полный процесс сбора и генерации coverage"
        )

        message(STATUS "Code coverage включен. Используйте 'cmake --build . --target coverage' для генерации отчета")
    else()
        message(WARNING "Инструменты llvm-cov или llvm-profdata не найдены. Code coverage отключен.")
    endif()
else()
    message(STATUS "Code coverage отключен")
endif()

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
if(ENABLE_CODE_COVERAGE)
    message(STATUS "  - Code Coverage: Включено")
else()
    message(STATUS "  - Code Coverage: Отключено")
endif()
