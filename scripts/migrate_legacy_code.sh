#!/bin/bash
# Миграция устаревшего кода в новую архитектуру

echo "🔄 Миграция устаревшего кода в новую архитектуру..."

# Функция для безопасной замены в файлах
safe_replace() {
    local file="$1"
    local old_pattern="$2"
    local new_pattern="$3"
    
    if [ -f "$file" ]; then
        # Создать резервную копию
        cp "$file" "$file.backup"
        
        # Выполнить замену
        sed -i "s|$old_pattern|$new_pattern|g" "$file"
        
        echo "  ✅ Обновлен: $file"
    fi
}

# 1. Массовая замена namespace'ов во всех файлах
echo "📝 Этап 1: Замена namespace'ов Engine3D → HyperEngine..."

# Найти все .cpp и .h файлы
find . -name "*.cpp" -o -name "*.h" | while read file; do
    if [ -f "$file" ]; then
        # Пропустить файлы в build директориях
        if [[ "$file" == *"/build"* ]] || [[ "$file" == *"/vcpkg"* ]]; then
            continue
        fi
        
        # Проверить, содержит ли файл Engine3D
        if grep -q "Engine3D" "$file"; then
            echo "  🔄 Обрабатывается: $file"
            
            # Замены namespace'ов
            safe_replace "$file" "namespace Engine3D" "namespace HyperEngine"
            safe_replace "$file" "Engine3D::" "HyperEngine::"
            safe_replace "$file" "using namespace Engine3D" "using namespace HyperEngine"
            
            # Замены include путей
            safe_replace "$file" "#include \"Engine3D/" "#include \"HyperEngine/"
            safe_replace "$file" "#include <Engine3D/" "#include <HyperEngine/"
        fi
    fi
done

# 2. Создание новой структуры директорий
echo "📁 Этап 2: Создание новой структуры директорий..."

# Создать новую структуру если не существует
mkdir -p src/core
mkdir -p src/math  
mkdir -p src/rendering/common
mkdir -p src/rendering/opengl
mkdir -p src/rendering/vulkan
mkdir -p src/physics
mkdir -p src/input
mkdir -p src/cuda
mkdir -p src/optix

mkdir -p include/HyperEngine/Core
mkdir -p include/HyperEngine/Math
mkdir -p include/HyperEngine/Rendering
mkdir -p include/HyperEngine/Physics
mkdir -p include/HyperEngine/Input
mkdir -p include/HyperEngine/Vulkan
mkdir -p include/HyperEngine/CUDA
mkdir -p include/HyperEngine/OptiX

echo "  ✅ Структура директорий создана"

# 3. Переименование директории include/Engine3D в include/HyperEngine
echo "📂 Этап 3: Переименование директорий..."

if [ -d "include/Engine3D" ] && [ ! -d "include/HyperEngine" ]; then
    echo "  🔄 Переименование include/Engine3D → include/HyperEngine"
    mv "include/Engine3D" "include/HyperEngine"
    echo "  ✅ Директория переименована"
else
    echo "  ℹ️ Директория include/HyperEngine уже существует или Engine3D не найдена"
fi

# 4. Обновление CMakeLists.txt
echo "⚙️ Этап 4: Обновление системы сборки..."

if [ -f "CMakeLists.txt" ]; then
    echo "  🔄 Обновление CMakeLists.txt"
    
    # Создать резервную копию
    cp "CMakeLists.txt" "CMakeLists.txt.backup"
    
    # Замены в CMakeLists.txt
    safe_replace "CMakeLists.txt" "Engine3D" "HyperEngine"
    safe_replace "CMakeLists.txt" "src3D" "src"
    safe_replace "CMakeLists.txt" "srcVulkan" "src"
    
    echo "  ✅ CMakeLists.txt обновлен"
fi

# 5. Генерация отчета о миграции
echo "📊 Этап 5: Генерация отчета о миграции..."

# Подсчитать количество обработанных файлов
PROCESSED_FILES=$(find . -name "*.backup" | wc -l)
REMAINING_ENGINE3D=$(grep -r "Engine3D" --include="*.cpp" --include="*.h" . | grep -v ".backup" | wc -l)

echo "  📈 Статистика миграции:"
echo "    - Обработано файлов: $PROCESSED_FILES"
echo "    - Осталось упоминаний Engine3D: $REMAINING_ENGINE3D"

# Обновить статус миграции
cat > docs/refactoring/migration_progress.md << EOF
# Прогресс миграции кода

**Дата последнего обновления:** $(date)

## 📊 Статистика
- **Обработано файлов:** $PROCESSED_FILES
- **Осталось упоминаний Engine3D:** $REMAINING_ENGINE3D
- **Статус:** $(if [ $REMAINING_ENGINE3D -lt 10 ]; then echo "Почти завершено ✅"; else echo "В процессе 🔄"; fi)

## 🔄 Выполненные операции
1. ✅ Замена namespace'ов Engine3D → HyperEngine
2. ✅ Обновление include путей
3. ✅ Создание новой структуры директорий
4. ✅ Переименование include/Engine3D → include/HyperEngine
5. ✅ Обновление CMakeLists.txt

## 📋 Следующие шаги
- Проверить сборку проекта
- Запустить тесты
- Обновить документацию
- Создать PR с изменениями

EOF

echo "  ✅ Отчет о миграции создан: docs/refactoring/migration_progress.md"

echo ""
echo "🎉 МИГРАЦИЯ ЗАВЕРШЕНА!"
echo "📋 Следующие шаги:"
echo "  1. Проверьте сборку: cmake --build build-vcpkg"
echo "  2. Запустите тесты: ctest"
echo "  3. Проверьте отчет: docs/refactoring/migration_progress.md"
echo "  4. Создайте коммит с изменениями"
