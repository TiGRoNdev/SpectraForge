#!/bin/bash
# Скрипт проверки заголовков авторских прав для HyperEngine

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Ожидаемый заголовок авторских прав
EXPECTED_COPYRIGHT="/**
 * Copyright (c) 2024 HyperEngine Team
 * 
 * This file is part of HyperEngine, a modern 3D/4D rendering engine.
 * 
 * Licensed under the MIT License. See LICENSE file in the project root
 * for full license information.
 */"

echo -e "${GREEN}Проверка заголовков авторских прав...${NC}"

# Счетчики
files_checked=0
files_missing_copyright=0
files_with_copyright=0

# Функция проверки файла
check_file() {
    local file="$1"
    
    # Пропускаем файлы в определенных директориях
    if [[ "$file" =~ (build|vcpkg|\.git)/ ]]; then
        return 0
    fi
    
    # Пропускаем файлы с определенными расширениями
    if [[ "$file" =~ \.(txt|md|json|yml|yaml|cmake|bat)$ ]]; then
        return 0
    fi
    
    files_checked=$((files_checked + 1))
    
    # Читаем первые 10 строк файла
    head_content=$(head -n 10 "$file" 2>/dev/null || echo "")
    
    # Проверяем наличие заголовка авторских прав
    if [[ "$head_content" =~ "Copyright".*(2024|2023|2025).*"HyperEngine" ]]; then
        files_with_copyright=$((files_with_copyright + 1))
        echo -e "${GREEN}✓${NC} $file"
        return 0
    else
        files_missing_copyright=$((files_missing_copyright + 1))
        echo -e "${RED}✗${NC} $file - отсутствует заголовок авторских прав"
        return 1
    fi
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
echo -e "Файлов с заголовком: ${GREEN}$files_with_copyright${NC}"
echo -e "Файлов без заголовка: ${RED}$files_missing_copyright${NC}"

if [ $exit_code -ne 0 ]; then
    echo
    echo -e "${RED}Некоторые файлы не содержат корректных заголовков авторских прав!${NC}"
    echo -e "${YELLOW}Добавьте следующий заголовок в начало файлов:${NC}"
    echo
    echo "$EXPECTED_COPYRIGHT"
fi

exit $exit_code
