#!/bin/bash
# Динамический анализ памяти и производительности HyperEngine

echo "🏃 Запуск динамического анализа HyperEngine..."

# Создать директорию для отчетов
mkdir -p build/dynamic-analysis

# 1. AddressSanitizer
echo "🚫 Сборка с AddressSanitizer..."
cmake -B build/asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTING=ON

if cmake --build build/asan --parallel; then
    echo "🧪 Запуск тестов с AddressSanitizer..."
    cd build/asan
    ASAN_OPTIONS="verbosity=1:halt_on_error=1:abort_on_error=1" \
    ctest --output-on-failure > ../dynamic-analysis/asan-report.txt 2>&1
    ASAN_EXIT_CODE=$?
    cd ../..
    
    if [ $ASAN_EXIT_CODE -eq 0 ]; then
        echo "✅ AddressSanitizer: Ошибки памяти не обнаружены"
    else
        echo "❌ AddressSanitizer: Обнаружены ошибки памяти"
    fi
else
    echo "❌ Ошибка сборки с AddressSanitizer"
fi

# 2. MemorySanitizer (Linux only)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "🧠 Сборка с MemorySanitizer..."
    cmake -B build/msan \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="-fsanitize=memory -fno-omit-frame-pointer -g" \
        -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DBUILD_TESTING=ON
    
    if cmake --build build/msan --parallel; then
        echo "🧪 Запуск тестов с MemorySanitizer..."
        cd build/msan
        MSAN_OPTIONS="print_stats=1" \
        ctest --output-on-failure > ../dynamic-analysis/msan-report.txt 2>&1
        cd ../..
        echo "✅ MemorySanitizer завершен"
    else
        echo "❌ Ошибка сборки с MemorySanitizer"
    fi
else
    echo "⚠️ MemorySanitizer доступен только на Linux"
fi

# 3. UndefinedBehaviorSanitizer
echo "❓ Сборка с UBSanitizer..."
cmake -B build/ubsan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer -g" \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTING=ON

if cmake --build build/ubsan --parallel; then
    echo "🧪 Запуск тестов с UBSanitizer..."
    cd build/ubsan
    UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1" \
    ctest --output-on-failure > ../dynamic-analysis/ubsan-report.txt 2>&1
    UBSAN_EXIT_CODE=$?
    cd ../..
    
    if [ $UBSAN_EXIT_CODE -eq 0 ]; then
        echo "✅ UBSanitizer: Неопределенное поведение не обнаружено"
    else
        echo "❌ UBSanitizer: Обнаружено неопределенное поведение"
    fi
else
    echo "❌ Ошибка сборки с UBSanitizer"
fi

# 4. ThreadSanitizer
echo "🧵 Сборка с ThreadSanitizer..."
cmake -B build/tsan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -g" \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTING=ON

if cmake --build build/tsan --parallel; then
    echo "🧪 Запуск тестов с ThreadSanitizer..."
    cd build/tsan
    TSAN_OPTIONS="halt_on_error=1" \
    ctest --output-on-failure > ../dynamic-analysis/tsan-report.txt 2>&1
    TSAN_EXIT_CODE=$?
    cd ../..
    
    if [ $TSAN_EXIT_CODE -eq 0 ]; then
        echo "✅ ThreadSanitizer: Гонки потоков не обнаружены"
    else
        echo "❌ ThreadSanitizer: Обнаружены гонки потоков"
    fi
else
    echo "❌ Ошибка сборки с ThreadSanitizer"
fi

# 5. Valgrind (Linux only)
if [[ "$OSTYPE" == "linux-gnu"* ]] && command -v valgrind &> /dev/null; then
    echo "🔍 Запуск Valgrind..."
    cmake -B build/valgrind \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DBUILD_TESTING=ON
    
    if cmake --build build/valgrind --parallel; then
        cd build/valgrind
        valgrind --tool=memcheck \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --xml=yes \
            --xml-file=../dynamic-analysis/valgrind-report.xml \
            ctest --output-on-failure > ../dynamic-analysis/valgrind-stdout.txt 2>&1
        cd ../..
        echo "✅ Valgrind анализ завершен"
    else
        echo "❌ Ошибка сборки для Valgrind"
    fi
else
    echo "⚠️ Valgrind недоступен (требуется Linux)"
fi

# 6. Создание сводного отчета
echo "📋 Создание сводного отчета динамического анализа..."
cat > build/dynamic-analysis/summary.md << 'EOF'
# Отчет динамического анализа

## Выполненные проверки

### AddressSanitizer (ASan)
- Назначение: Обнаружение ошибок памяти
- Отчет: asan-report.txt

### MemorySanitizer (MSan) 
- Назначение: Обнаружение использования неинициализированной памяти
- Отчет: msan-report.txt (только Linux)

### UndefinedBehaviorSanitizer (UBSan)
- Назначение: Обнаружение неопределенного поведения
- Отчет: ubsan-report.txt

### ThreadSanitizer (TSan)
- Назначение: Обнаружение гонок потоков
- Отчет: tsan-report.txt

### Valgrind
- Назначение: Детальный анализ памяти
- Отчет: valgrind-report.xml (только Linux)

## Рекомендации
1. Исправьте все обнаруженные ошибки памяти
2. Устраните неопределенное поведение
3. Решите проблемы с многопоточностью
4. Проверьте утечки памяти
EOF

echo "✅ Динамический анализ завершен. Отчеты в build/dynamic-analysis/"
echo "📋 Сводный отчет: build/dynamic-analysis/summary.md"
