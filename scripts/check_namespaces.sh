#!/bin/bash
# Скрипт проверки консистентности namespaces для HyperEngine

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Проверка консистентности namespaces...${NC}"

# Счетчики
files_checked=0
files_with_correct_ns=0
files_with_wrong_ns=0
files_without_ns=0

# Карта путей к ожидаемым namespace'ам
declare -A path_to_namespace
path_to_namespace["src/math/"]="HyperEngine::Math"
path_to_namespace["include/HyperEngine/Math/"]="HyperEngine::Math"
path_to_namespace["src/rendering/"]="HyperEngine::Rendering"
path_to_namespace["include/HyperEngine/Rendering/"]="HyperEngine::Rendering"
path_to_namespace["src/physics/"]="HyperEngine::Physics"
path_to_namespace["include/HyperEngine/Physics/"]="HyperEngine::Physics"
path_to_namespace["src/input/"]="HyperEngine::Input"
path_to_namespace["include/HyperEngine/Input/"]="HyperEngine::Input"
path_to_namespace["src/core/"]="HyperEngine::Core"
path_to_namespace["include/HyperEngine/Core/"]="HyperEngine::Core"
path_to_namespace["src/testing/"]="HyperEngine::Testing"
path_to_namespace["include/HyperEngine/Testing/"]="HyperEngine::Testing"

# Функция определения ожидаемого namespace
get_expected_namespace() {
    local file="$1"
    local dir=$(dirname "$file")/
    
    # Проходим по всем известным путям
    for path in "${!path_to_namespace[@]}"; do
        if [[ "$dir" =~ ^${path} ]]; then
            echo "${path_to_namespace[$path]}"
            return 0
        fi
    done
    
    # Если путь не найден, возвращаем базовый namespace
    echo "HyperEngine"
}

# Функция извлечения namespace из файла
extract_namespace() {
    local file="$1"
    local content=$(cat "$file" 2>/dev/null || echo "")
    
    # Ищем объявления namespace
    local namespaces=$(echo "$content" | grep -E "^namespace[[:space:]]+[^[:space:]]+.*\{" | \
                      sed 's/namespace[[:space:]]*\([^[:space:]]*\).*/\1/' | \
                      grep -v "^$" || echo "")
    
    # Ищем вложенные namespace
    local nested_ns=$(echo "$content" | grep -E "namespace.*::" | \
                     sed 's/.*namespace[[:space:]]*\([^[:space:]]*::[^[:space:]]*\).*/\1/' | \
                     head -n 1 || echo "")
    
    if [ -n "$nested_ns" ]; then
        echo "$nested_ns"
    elif [ -n "$namespaces" ]; then
        # Объединяем namespace'ы через ::
        echo "$namespaces" | tr '\n' ':' | sed 's/:$//' | sed 's/:/::/g'
    else
        echo ""
    fi
}

# Функция проверки файла
check_file() {
    local file="$1"
    
    # Пропускаем файлы в определенных директориях
    if [[ "$file" =~ (build|vcpkg|\.git|tests|examples)/ ]]; then
        return 0
    fi
    
    # Только .cpp, .h и .hpp файлы
    if [[ ! "$file" =~ \.(cpp|h|hpp)$ ]]; then
        return 0
    fi
    
    files_checked=$((files_checked + 1))
    
    local expected_ns=$(get_expected_namespace "$file")
    local actual_ns=$(extract_namespace "$file")
    
    if [ -z "$actual_ns" ]; then
        files_without_ns=$((files_without_ns + 1))
        echo -e "${YELLOW}⚠${NC} $file - отсутствует namespace"
        echo "    Ожидается: $expected_ns"
        return 1
    fi
    
    # Нормализуем namespace'ы для сравнения
    local normalized_expected=$(echo "$expected_ns" | sed 's/[[:space:]]//g')
    local normalized_actual=$(echo "$actual_ns" | sed 's/[[:space:]]//g')
    
    if [ "$normalized_actual" != "$normalized_expected" ]; then
        files_with_wrong_ns=$((files_with_wrong_ns + 1))
        echo -e "${YELLOW}⚠${NC} $file - неправильный namespace"
        echo "    Ожидается: $expected_ns"
        echo "    Найден: $actual_ns"
        return 1
    fi
    
    files_with_correct_ns=$((files_with_correct_ns + 1))
    echo -e "${GREEN}✓${NC} $file"
    return 0
}

# Проверяем все переданные файлы
exit_code=0
for file in "$@"; do
    if [ -f "$file" ]; then
        if ! check_file "$file"; then
            exit_code=1
        fi
    fi
done

# Если файлы не переданы, проверяем все исходники
if [ $# -eq 0 ]; then
    while IFS= read -r -d '' file; do
        if ! check_file "$file"; then
            exit_code=1
        fi
    done < <(find src include -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print0 2>/dev/null)
fi

# Выводим статистику
echo
echo -e "${YELLOW}Статистика:${NC}"
echo "Проверено файлов: $files_checked"
echo -e "Файлов с корректными namespace: ${GREEN}$files_with_correct_ns${NC}"
echo -e "Файлов с неправильными namespace: ${YELLOW}$files_with_wrong_ns${NC}"
echo -e "Файлов без namespace: ${YELLOW}$files_without_ns${NC}"

if [ $exit_code -ne 0 ]; then
    echo
    echo -e "${RED}Некоторые файлы имеют проблемы с namespace!${NC}"
    echo -e "${YELLOW}Структура namespace'ов:${NC}"
    echo "  src/math/ -> HyperEngine::Math"
    echo "  src/rendering/ -> HyperEngine::Rendering"
    echo "  src/physics/ -> HyperEngine::Physics"
    echo "  src/input/ -> HyperEngine::Input"
    echo "  src/core/ -> HyperEngine::Core"
fi

exit $exit_code
