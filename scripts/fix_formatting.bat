@echo off
REM Автоматическое исправление форматирования кода HyperEngine
setlocal enabledelayedexpansion

echo 🎨 Исправление форматирования кода HyperEngine
echo =============================================

REM Путь к clang-format
set CLANG_FORMAT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\Llvm\bin\clang-format.exe"

REM Проверяем, существует ли clang-format
if not exist %CLANG_FORMAT% (
    echo ❌ ОШИБКА: clang-format не найден по пути %CLANG_FORMAT%
    exit /b 1
)

echo ℹ️ Используем clang-format: %CLANG_FORMAT%
echo.

REM Создаем временный файл со списком файлов
dir /s /b src\*.cpp src\*.h include\*.cpp include\*.h tests\*.cpp tests\*.h 2>nul > temp_files.txt

set FIXED_COUNT=0
set TOTAL_COUNT=0

echo 🔧 Исправляем форматирование файлов...
for /f "delims=" %%f in (temp_files.txt) do (
    set /a TOTAL_COUNT+=1
    echo Обрабатываем: %%f

    REM Применяем форматирование
    %CLANG_FORMAT% -i --style=file "%%f"
    if !errorlevel! equ 0 (
        set /a FIXED_COUNT+=1
        echo   ✅ Исправлено
    ) else (
        echo   ❌ Ошибка при обработке
    )
)

del temp_files.txt 2>nul

echo.
echo 📊 РЕЗУЛЬТАТЫ:
echo ===============
echo Всего файлов обработано: !TOTAL_COUNT!
echo Успешно исправлено: !FIXED_COUNT!
echo.

if !FIXED_COUNT! equ !TOTAL_COUNT! (
    echo ✅ Все файлы успешно отформатированы!
    exit /b 0
) else (
    echo ⚠️ Некоторые файлы не удалось отформатировать
    exit /b 1
)
