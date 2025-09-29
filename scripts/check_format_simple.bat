@echo off
chcp 65001 >nul
REM Простая проверка форматирования без clang-format

echo 🔍 Проверка основных проблем форматирования...
echo ===============================================

set ISSUES=0

REM Проверяем конструкторы с неправильным форматированием
echo Поиск конструкторов с проблемами форматирования...

findstr /R /C:"^.*::.*() *$" src\*.cpp include\*.h tests\*.cpp examples\*.cpp 2>nul
if %errorlevel% equ 0 (
    echo ⚠️ Найдены конструкторы с пробелом перед скобками
    set /a ISSUES+=1
)

findstr /R /C:"^ *: *[a-zA-Z].*" src\*.cpp include\*.h tests\*.cpp examples\*.cpp 2>nul
if %errorlevel% equ 0 (
    echo ⚠️ Найдены списки инициализации с неправильным форматированием
    set /a ISSUES+=1
)

findstr /R /C:"^ *, *[a-zA-Z].*" src\*.cpp include\*.h tests\*.cpp examples\*.cpp 2>nul
if %errorlevel% equ 0 (
    echo ⚠️ Найдены списки инициализации с запятыми в начале строки
    set /a ISSUES+=1
)

echo.
if %ISSUES% equ 0 (
    echo ✅ Основные проблемы форматирования не найдены
    exit /b 0
) else (
    echo ❌ Найдено проблем: %ISSUES%
    exit /b 1
)
