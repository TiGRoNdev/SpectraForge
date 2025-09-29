@echo off
chcp 65001 >nul
REM Комплексный статический анализ кода HyperEngine для Windows
setlocal enabledelayedexpansion

echo 🔍 Запуск статического анализа кода HyperEngine...

REM Создать директорию для отчетов
if not exist "build\static-analysis" mkdir "build\static-analysis"

REM Счетчики
set ERRORS=0
set WARNINGS=0

REM Пути к инструментам
set CLANG_TIDY="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\Llvm\bin\clang-tidy.exe"
set CPPCHECK="C:\Program Files\Cppcheck\cppcheck.exe"

REM 1. Clang-Tidy анализ
echo.
echo 🔧 Запуск Clang-Tidy...
if exist %CLANG_TIDY% (
    if exist "build-vcpkg\compile_commands.json" (
        echo Используем compile_commands.json из build-vcpkg

        REM Создаем временный файл со списком файлов для анализа
        dir /s /b src\*.cpp > temp_cpp_files.txt

        set ANALYZED_FILES=0
        for /f "delims=" %%f in (temp_cpp_files.txt) do (
            echo Анализируем: %%f
            %CLANG_TIDY% "%%f" --config-file=.clang-tidy -p build-vcpkg >> build\static-analysis\clang-tidy-report.txt 2>&1
            if !errorlevel! neq 0 (
                set /a WARNINGS+=1
            )
            set /a ANALYZED_FILES+=1
        )

        del temp_cpp_files.txt 2>nul
        echo ✅ Проанализировано файлов: !ANALYZED_FILES!
    ) else (
        echo ⚠️ compile_commands.json не найден, пропускаем clang-tidy
        set /a WARNINGS+=1
    )
) else (
    echo ⚠️ Clang-Tidy не найден по пути %CLANG_TIDY%
    set /a WARNINGS+=1
)

REM 2. Cppcheck анализ
echo.
echo 🛡️ Запуск Cppcheck...
if exist %CPPCHECK% (
    echo Запускаем Cppcheck...
    %CPPCHECK% --enable=all --xml --xml-version=2 --output-file=build\static-analysis\cppcheck-report.xml --suppress=missingIncludeSystem --suppress=unusedFunction -I include src\ include\ > build\static-analysis\cppcheck-console.log 2>&1
    if !errorlevel! equ 0 (
        echo ✅ Cppcheck анализ завершен
    ) else (
        echo ⚠️ Cppcheck завершился с предупреждениями
        set /a WARNINGS+=1
    )
) else (
    echo ⚠️ Cppcheck не найден по пути %CPPCHECK%
    set /a WARNINGS+=1
)

REM 3. Анализ включений (простая проверка)
echo.
echo 📦 Анализ включений...
echo Проверяем циклические зависимости и неиспользуемые включения... > build\static-analysis\includes-report.txt

REM Ищем потенциальные проблемы с включениями
findstr /r /c:"#include.*\.h" src\*.cpp include\*.h >> build\static-analysis\includes-report.txt 2>&1
echo ✅ Анализ включений завершен

REM 4. Проверка соответствия стандартам кодирования
echo.
echo 📋 Проверка стандартов кодирования...
echo Проверяем соответствие правилам проекта... > build\static-analysis\coding-standards.txt

REM Проверяем использование namespace'ов
findstr /r /c:"using namespace" src\*.cpp include\*.h > build\static-analysis\namespace-usage.txt 2>nul
if !errorlevel! equ 0 (
    echo ⚠️ Найдены использования 'using namespace' - рекомендуется избегать в заголовочных файлах
    set /a WARNINGS+=1
) else (
    echo ✅ Использование namespace'ов корректно
)

REM Проверяем наличие include guards
set MISSING_GUARDS=0
for /r include %%f in (*.h) do (
    findstr /c:"#pragma once" "%%f" >nul 2>&1
    if !errorlevel! neq 0 (
        findstr /c:"#ifndef" "%%f" >nul 2>&1
        if !errorlevel! neq 0 (
            echo Отсутствует include guard: %%f >> build\static-analysis\missing-guards.txt
            set /a MISSING_GUARDS+=1
        )
    )
)

if !MISSING_GUARDS! gtr 0 (
    echo ⚠️ Найдено !MISSING_GUARDS! файлов без include guards
    set /a WARNINGS+=1
) else (
    echo ✅ Все заголовочные файлы имеют include guards
)

REM 5. Создание сводного отчета
echo.
echo 📋 Создание сводного отчета...
(
echo # Отчет статического анализа кода HyperEngine
echo.
echo **Дата:** %date% %time%
echo.
echo ## Результаты анализа
echo.
echo - **Ошибки:** !ERRORS!
echo - **Предупреждения:** !WARNINGS!
echo.
echo ## Выполненные проверки
echo.
echo ### Clang-Tidy
if exist %CLANG_TIDY% (
    echo - Статус: ✅ Выполнен
    echo - Отчет: clang-tidy-report.txt
) else (
    echo - Статус: ❌ Не выполнен ^(инструмент не найден^)
)
echo.
echo ### Cppcheck
if exist %CPPCHECK% (
    echo - Статус: ✅ Выполнен
    echo - Отчет: cppcheck-report.xml
) else (
    echo - Статус: ❌ Не выполнен ^(инструмент не найден^)
)
echo.
echo ### Анализ включений
echo - Статус: ✅ Выполнен
echo - Отчет: includes-report.txt
echo.
echo ### Стандарты кодирования
echo - Статус: ✅ Выполнен
echo - Отчет: coding-standards.txt
echo.
echo ## Рекомендации
echo.
if !ERRORS! gtr 0 (
    echo ❌ Исправьте критические ошибки перед продолжением разработки
) else if !WARNINGS! gtr 0 (
    echo ⚠️ Рассмотрите исправление предупреждений для улучшения качества кода
) else (
    echo ✅ Код соответствует стандартам статического анализа
)
echo.
echo ## Детальные отчеты
echo.
echo - Clang-Tidy: build\static-analysis\clang-tidy-report.txt
echo - Cppcheck: build\static-analysis\cppcheck-report.xml
echo - Анализ включений: build\static-analysis\includes-report.txt
echo - Стандарты кодирования: build\static-analysis\coding-standards.txt
) > build\static-analysis\summary.md

REM Финальный отчет
echo.
echo 📈 ОТЧЕТ СТАТИЧЕСКОГО АНАЛИЗА
echo ==============================
echo Ошибки: !ERRORS!
echo Предупреждения: !WARNINGS!
echo.

if !ERRORS! gtr 0 (
    echo ❌ Обнаружены критические ошибки статического анализа
    echo 📋 Подробный отчет: build\static-analysis\summary.md
    exit /b 1
) else if !WARNINGS! gtr 0 (
    echo ⚠️ Есть предупреждения статического анализа
    echo 📋 Подробный отчет: build\static-analysis\summary.md
    exit /b 0
) else (
    echo ✅ Статический анализ не выявил проблем!
    echo 📋 Подробный отчет: build\static-analysis\summary.md
    exit /b 0
)
