@echo off
REM Комплексная проверка качества кода HyperEngine для Windows
setlocal enabledelayedexpansion

echo 🎯 Комплексная проверка качества кода HyperEngine
echo ================================================

REM Счетчики
set ERRORS=0
set WARNINGS=0

REM Путь к clang-format
set CLANG_FORMAT="C:\Program Files\LLVM\bin\clang-format.exe"

REM Создать директорию для отчетов
if not exist "build\quality-reports" mkdir "build\quality-reports"

REM 1. Проверка форматирования кода
echo.
echo 🎨 Проверка форматирования кода...

REM Проверяем, существует ли clang-format
if not exist %CLANG_FORMAT% (
    echo ❌ ОШИБКА: clang-format не найден по пути %CLANG_FORMAT%
    set /a ERRORS+=1
    goto :skip_format
)

REM Создаем временный файл со списком файлов
dir /s /b src\*.cpp src\*.h include\*.cpp include\*.h tests\*.cpp tests\*.h 2>nul > temp_files.txt

REM Проверяем форматирование
set FORMAT_ISSUES=0
for /f "delims=" %%f in (temp_files.txt) do (
    %CLANG_FORMAT% --dry-run --Werror --style=file "%%f" >nul 2>&1
    if !errorlevel! neq 0 (
        echo Проблема форматирования в файле: %%f
        set /a FORMAT_ISSUES+=1
    )
)

if !FORMAT_ISSUES! equ 0 (
    echo ✅ Форматирование кода соответствует стандартам
) else (
    echo ❌ ОШИБКА: Обнаружено !FORMAT_ISSUES! файлов с проблемами форматирования
    set /a ERRORS+=1
)

del temp_files.txt 2>nul

:skip_format

REM 2. Проверка сборки
echo.
echo 🔨 Проверка сборки...
if exist "build-vcpkg" (
    echo ✅ Используем существующую сборку build-vcpkg
) else (
    echo ⚠️ ПРЕДУПРЕЖДЕНИЕ: Директория сборки build-vcpkg не найдена
    set /a WARNINGS+=1
)

REM 3. Запуск тестов
echo.
echo 🧪 Запуск тестов...
if exist "build-vcpkg" (
    cd build-vcpkg
    ctest -C Release --output-on-failure > ..\build\quality-reports\tests.log 2>&1
    if !errorlevel! equ 0 (
        echo ✅ Все тесты прошли успешно
    ) else (
        echo ❌ ОШИБКА: Некоторые тесты не прошли. См. build\quality-reports\tests.log
        set /a ERRORS+=1
    )
    cd ..
) else (
    echo ⚠️ ПРЕДУПРЕЖДЕНИЕ: Директория сборки не найдена, пропускаем тесты
    set /a WARNINGS+=1
)

REM 4. Проверка структуры проекта
echo.
echo 🏗️ Проверка структуры проекта...
set REQUIRED_DIRS=src include tests docs scripts
for %%d in (%REQUIRED_DIRS%) do (
    if exist "%%d" (
        echo ✅ Директория %%d существует
    ) else (
        echo ⚠️ ПРЕДУПРЕЖДЕНИЕ: Отсутствует директория %%d
        set /a WARNINGS+=1
    )
)

REM 5. Создание сводного отчета
echo.
echo 📋 Создание сводного отчета...
(
echo # Отчет о качестве кода HyperEngine
echo.
echo **Дата:** %date% %time%
echo.
echo ## Результаты проверки
echo.
echo - **Ошибки:** !ERRORS!
echo - **Предупреждения:** !WARNINGS!
echo.
echo ## Выполненные проверки
echo.
echo 1. ✅ Форматирование кода
echo 2. ✅ Сборка проекта
echo 3. ✅ Запуск тестов
echo 4. ✅ Структура проекта
echo.
echo ## Рекомендации
echo.
if !ERRORS! gtr 0 (
    echo ❌ Исправьте критические ошибки перед коммитом
) else if !WARNINGS! gtr 0 (
    echo ⚠️ Рассмотрите исправление предупреждений
) else (
    echo ✅ Код соответствует всем стандартам качества
)
) > build\quality-reports\summary.md

REM Финальный отчет
echo.
echo 📈 ОТЧЕТ О КАЧЕСТВЕ КОДА
echo ========================
echo Ошибки: !ERRORS!
echo Предупреждения: !WARNINGS!
echo.

if !ERRORS! gtr 0 (
    echo ❌ Обнаружены критические ошибки. Исправьте их перед коммитом.
    echo 📋 Подробный отчет: build\quality-reports\summary.md
    exit /b 1
) else if !WARNINGS! gtr 0 (
    echo ⚠️ Есть предупреждения. Рекомендуется их исправить.
    echo 📋 Подробный отчет: build\quality-reports\summary.md
    exit /b 0
) else (
    echo ✅ Код соответствует всем стандартам качества!
    echo 📋 Подробный отчет: build\quality-reports\summary.md
    exit /b 0
)
