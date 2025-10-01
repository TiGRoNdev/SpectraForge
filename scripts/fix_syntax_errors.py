#!/usr/bin/env python3
"""
Скрипт для исправления синтаксических ошибок в тестах
"""

import re
import os

def fix_syntax_errors():
    """Исправляет синтаксические ошибки в RendererAdapterTest.cpp"""
    file_path = "tests/unit/rendering/RendererAdapterTest.cpp"

    if not os.path.exists(file_path):
        print(f"Файл {file_path} не найден")
        return False

    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Исправляем неправильные скобки в ASSERT_TRUE
    content = re.sub(r'ASSERT_TRUE\(([^;]+);([^)]*)\);', r'ASSERT_TRUE(\1);\2', content)

    # Исправляем оставшиеся проблемы с initialize
    content = re.sub(r'adapter->initialize\(RenderBackend::(\w+),\s*(\d+),\s*(\d+)\)',
                     r'adapter->setBackend(RenderBackend::\1); adapter->initialize(\2, \3)', content)

    # Исправляем двойные точки с запятой
    content = re.sub(r';;', ';', content)

    # Исправляем неправильные переносы строк в ASSERT_TRUE
    content = re.sub(r'ASSERT_TRUE\(([^)]+)\n\s*([^)]+)\);', r'ASSERT_TRUE(\1 \2);', content)

    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Исправлены синтаксические ошибки в {file_path}")
    return True

if __name__ == "__main__":
    print("Исправление синтаксических ошибок...")

    if fix_syntax_errors():
        print("Исправления применены успешно!")
    else:
        print("Ошибка при исправлении")
