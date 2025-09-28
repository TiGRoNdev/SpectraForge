#!/usr/bin/env python3
"""
Скрипт для исправления API вызовов в тестах HyperEngine
"""

import re
import os

def fix_renderer_adapter_test():
    """Исправляет вызовы initialize в RendererAdapterTest.cpp"""
    file_path = "tests/unit/rendering/RendererAdapterTest.cpp"
    
    if not os.path.exists(file_path):
        print(f"Файл {file_path} не найден")
        return False
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Паттерн для поиска вызовов initialize с RenderBackend
    pattern = r'(\s*)([A-Z_]+\()?adapter->initialize\(RenderBackend::(\w+),\s*(\d+),\s*(\d+)\);?'
    
    def replace_initialize(match):
        indent = match.group(1)
        prefix = match.group(2) or ""
        backend = match.group(3)
        width = match.group(4)
        height = match.group(5)
        
        return f"{indent}{prefix}adapter->setBackend(RenderBackend::{backend});\n{indent}adapter->initialize({width}, {height});"
    
    # Заменяем все вхождения
    new_content = re.sub(pattern, replace_initialize, content)
    
    # Также исправляем случаи с bool result =
    pattern2 = r'(\s*)bool result = adapter->initialize\(RenderBackend::(\w+),\s*(\d+),\s*(\d+)\);'
    
    def replace_bool_initialize(match):
        indent = match.group(1)
        backend = match.group(2)
        width = match.group(3)
        height = match.group(4)
        
        return f"{indent}adapter->setBackend(RenderBackend::{backend});\n{indent}bool result = adapter->initialize({width}, {height});"
    
    new_content = re.sub(pattern2, replace_bool_initialize, new_content)
    
    # Записываем обратно
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"Исправлен файл {file_path}")
    return True

def fix_missing_methods():
    """Исправляет отсутствующие методы в тестах"""
    file_path = "tests/unit/rendering/RendererAdapterTest.cpp"
    
    if not os.path.exists(file_path):
        return False
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Удаляем вызовы несуществующих методов
    content = re.sub(r'\s*adapter->switchBackend\([^)]+\);\s*', '', content)
    
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"Удалены вызовы несуществующих методов из {file_path}")
    return True

if __name__ == "__main__":
    print("Исправление API вызовов в тестах...")
    
    success = True
    success &= fix_renderer_adapter_test()
    success &= fix_missing_methods()
    
    if success:
        print("Все исправления применены успешно!")
    else:
        print("Некоторые исправления не удались")
