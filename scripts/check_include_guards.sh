#!/bin/bash
# Скрипт проверки include guards для HyperEngine

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Проверка include guards...${NC}"

# Счетчики
files_checked=0
files_with_guards=0
files_missing_guards=0
files_with_wrong_guards=0

# Функция генерации ожидаемого guard'а
generate_expected_guard() {
    local file="$1"
    
    # Убираем префикс include/ если есть
    local relative_path="${file#include/}"
    
    # Преобразуем путь в guard: path/to/file.h -> PATH_TO_FILE_H_
    local guard=$(echo "${relative_path}" | tr '[:lower:]/' '[:upper:]_' | sed 's/\./_/g')
    
    # Добавляем префикс HYPERENGINE_
    echo "HYPERENGINE_${guard}_"
}

# Функция проверки файла
check_file() {
    local file="$1"
    
    # Пропускаем файлы не в include/
    if [[ ! "$file" =~ ^include/ ]]; then
        return 0
    fi
    
    # Только .h и .hpp файлы
    if [[ ! "$file" =~ \.(h|hpp)$ ]]; then
        return 0
    fi
    
    files_checked=$((files_checked + 1))
    
    # Читаем содержимое файла
    content=$(cat "$file" 2>/dev/null || echo "")
    
    # Генерируем ожидаемый guard
    expected_guard=$(generate_expected_guard "$file")
    
    # Проверяем наличие #ifndef и #define
    ifndef_line=$(echo "$content" | grep -n "#ifndef" | head -n 1 || echo "")
    define_line=$(echo "$content" | grep -n "#define" | head -n 1 || echo "")
    endif_line=$(echo "$content" | grep -n "#endif" | tail -n 1 || echo "")
    
    if [ -z "$ifndef_line" ] || [ -z "$define_line" ] || [ -z "$endif_line" ]; then
        files_missing_guards=$((files_missing_guards + 1))
        echo -e "${RED}✗${NC} $file - отсутствует include guard"
        return 1
    fi
    
    # Извлекаем guard из #ifndef
    actual_guard=$(echo "$ifndef_line" | sed 's/.*#ifndef[[:space:]]*\([^[:space:]]*\).*/\1/')
    
    # Проверяем соответствие ожидаемому guard'у
    if [ "$actual_guard" != "$expected_guard" ]; then
        files_with_wrong_guards=$((files_with_wrong_guards + 1))
        echo -e "${YELLOW}⚠${NC} $file - неправильный include guard"
        echo "    Ожидается: $expected_guard"
        echo "    Найден: $actual_guard"
        return 1
    fi
    
    # Проверяем, что #define соответствует #ifndef
    define_guard=$(echo "$define_line" | sed 's/.*#define[[:space:]]*\([^[:space:]]*\).*/\1/')
    if [ "$define_guard" != "$actual_guard" ]; then
        files_with_wrong_guards=$((files_with_wrong_guards + 1))
        echo -e "${YELLOW}⚠${NC} $file - несоответствие #ifndef и #define"
        echo "    #ifndef: $actual_guard"
        echo "    #define: $define_guard"
        return 1
    fi
    
    files_with_guards=$((files_with_guards + 1))
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

# Если файлы не переданы, проверяем все заголовки
if [ $# -eq 0 ]; then
    while IFS= read -r -d '' file; do
        if ! check_file "$file"; then
            exit_code=1
        fi
    done < <(find include -type f \( -name "*.h" -o -name "*.hpp" \) -print0 2>/dev/null)
fi

# Выводим статистику
echo
echo -e "${YELLOW}Статистика:${NC}"
echo "Проверено файлов: $files_checked"
echo -e "Файлов с корректными guards: ${GREEN}$files_with_guards${NC}"
echo -e "Файлов без guards: ${RED}$files_missing_guards${NC}"
echo -e "Файлов с неправильными guards: ${YELLOW}$files_with_wrong_guards${NC}"

if [ $exit_code -ne 0 ]; then
    echo
    echo -e "${RED}Некоторые файлы имеют проблемы с include guards!${NC}"
    echo -e "${YELLOW}Include guard должен иметь формат: HYPERENGINE_PATH_TO_FILE_H_${NC}"
fi

exit $exit_code
