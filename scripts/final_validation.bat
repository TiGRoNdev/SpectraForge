@echo off
setlocal enabledelayedexpansion
REM Финальная валидация рефакторинга (Windows версия)

echo 🎯 ФИНАЛЬНАЯ ВАЛИДАЦИЯ РЕФАКТОРИНГА HYPERENGINE
echo ==============================================

set VALIDATION_PASSED=true

REM Функция проверки (эмуляция через метки)
goto :start_validation

:validate
set test_name=%1
set command=%2
echo | set /p="🔍 %test_name%... "

REM Выполнить команду и проверить результат
%command% >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)
goto :eof

:start_validation

echo.
echo 📋 ПРОВЕРКА АРХИТЕКТУРНЫХ ТРЕБОВАНИЙ
echo -----------------------------------

REM 1. Проверка структуры проекта
echo | set /p="🔍 Структура директорий... "
if exist include if exist tests if exist docs (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

REM 2. Проверка namespace'ов
echo | set /p="🔍 Обновление namespace'ов... "
findstr /s /m "namespace HyperEngine" include\*.h >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

REM 3. Проверка миграции include путей
echo | set /p="🔍 Миграция include путей... "
findstr /s /m "#include \"HyperEngine/" examples\*.cpp >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

REM 4. Проверка отсутствия старых namespace'ов
echo | set /p="🔍 Отсутствие старых Engine3D namespace'ов... "
findstr /s /c:"namespace Engine3D" . >nul 2>&1
if %errorlevel% neq 0 (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ (найдены старые namespace'ы)
    set VALIDATION_PASSED=false
)

echo.
echo 🧪 ПРОВЕРКА СИСТЕМЫ ТЕСТИРОВАНИЯ
echo -------------------------------

REM 5. Проверка наличия тестов
echo | set /p="🔍 Наличие тестов... "
if exist tests\unit\*.cpp (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

echo.
echo 🔧 ПРОВЕРКА КАЧЕСТВА КОДА
echo ------------------------

REM 6. Проверка CMakeLists.txt
echo | set /p="🔍 Обновление CMakeLists.txt... "
findstr /m "HyperEngine" CMakeLists.txt >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

echo.
echo 📚 ПРОВЕРКА ДОКУМЕНТАЦИИ
echo -----------------------

REM 7. Наличие ключевых документов
echo | set /p="🔍 Наличие ключевых документов... "
if exist docs\refactoring\FINAL_REPORT.md if exist docs\refactoring\migration_status.md (
    echo ✅ ПРОШЕЛ
) else (
    echo ❌ НЕ ПРОШЕЛ
    set VALIDATION_PASSED=false
)

echo.
echo 🎊 РЕЗУЛЬТАТЫ ВАЛИДАЦИИ
echo ======================

if "!VALIDATION_PASSED!"=="true" (
    echo ✅ ВСЕ ПРОВЕРКИ ПРОШЛИ УСПЕШНО!
    echo.
    echo 🎉 ПОЗДРАВЛЯЕМ! Основная миграция HyperEngine успешно завершена!
    echo.
    echo Проект теперь использует:
    echo   ✅ Namespace HyperEngine вместо Engine3D
    echo   ✅ Обновленные include пути
    echo   ✅ Новую структуру директорий
    echo   ✅ Обновленную систему сборки
    echo.
    echo Следующие шаги:
    echo   1. Физическое перемещение файлов из src3D в src
    echo   2. Тестирование сборки проекта
    echo   3. Обновление документации
    echo.
    
    REM Генерация итогового отчета
    python scripts/generate_final_report.py
    
    exit /b 0
) else (
    echo ❌ НЕКОТОРЫЕ ПРОВЕРКИ НЕ ПРОШЛИ
    echo.
    echo Пожалуйста, исправьте выявленные проблемы перед завершением рефакторинга.
    exit /b 1
)

pause
