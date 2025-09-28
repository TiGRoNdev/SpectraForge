@echo off
REM Комплексный статический анализ кода HyperEngine для Windows
echo 🔍 Запуск статического анализа кода HyperEngine...

REM Создать директорию для отчетов
if not exist "build\static-analysis" mkdir "build\static-analysis"

REM Проверка наличия Visual Studio Code Analysis
echo 📊 Проверка доступности инструментов анализа...

REM 1. Visual Studio Code Analysis (если доступен)
echo 🔧 Запуск Visual Studio Code Analysis...
if exist "D:\VSBuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe" (
    echo ✅ Visual Studio Build Tools найдены
    REM Запуск анализа через MSBuild с включенным Code Analysis
    if exist "build-vcpkg\HyperEngine.sln" (
        echo 📊 Запуск MSBuild с Code Analysis...
        msbuild "build-vcpkg\HyperEngine.sln" /p:Configuration=Debug /p:Platform=x64 /p:RunCodeAnalysis=true /p:CodeAnalysisRuleSet=AllRules.ruleset /flp:logfile=build\static-analysis\msbuild-analysis.log;verbosity=diagnostic
    ) else (
        echo ⚠️ Файл решения не найден, пропускаем MSBuild анализ
    )
) else (
    echo ⚠️ Visual Studio Build Tools не найдены
)

REM 2. Clang-Tidy анализ (если доступен через vcpkg или LLVM)
echo 🔧 Поиск Clang-Tidy...
where clang-tidy >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ Clang-Tidy найден
    REM Создаем простой compile_commands.json для анализа
    echo 📝 Создание compile_commands.json...
    echo [ > build\static-analysis\compile_commands.json
    echo   { >> build\static-analysis\compile_commands.json
    echo     "directory": "%CD%", >> build\static-analysis\compile_commands.json
    echo     "command": "cl.exe /I include /I vcpkg_installed\\x64-windows\\include /std:c++17 /EHsc src\\core\\Engine.cpp", >> build\static-analysis\compile_commands.json
    echo     "file": "src\\core\\Engine.cpp" >> build\static-analysis\compile_commands.json
    echo   } >> build\static-analysis\compile_commands.json
    echo ] >> build\static-analysis\compile_commands.json
    
    REM Запуск clang-tidy на ключевых файлах
    echo 🔍 Анализ ключевых файлов с Clang-Tidy...
    for %%f in (src\core\*.cpp src\rendering\*.cpp) do (
        if exist "%%f" (
            echo Анализ: %%f
            clang-tidy "%%f" --config-file=.clang-tidy -p build\static-analysis >> build\static-analysis\clang-tidy-report.txt 2>&1
        )
    )
) else (
    echo ⚠️ Clang-Tidy не найден в PATH
)

REM 3. Cppcheck анализ (если установлен)
echo 🛡️ Поиск Cppcheck...
where cppcheck >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ Cppcheck найден
    echo 🔍 Запуск Cppcheck анализа...
    cppcheck --enable=all --xml --xml-version=2 --output-file=build\static-analysis\cppcheck-report.xml --suppress=missingIncludeSystem --suppress=unusedFunction -I include -I vcpkg_installed\x64-windows\include src\ include\ 2>build\static-analysis\cppcheck-errors.txt
) else (
    echo ⚠️ Cppcheck не установлен
    echo 💡 Для установки Cppcheck: winget install Cppcheck.Cppcheck
)

REM 4. PVS-Studio анализ (если доступен)
echo 🔬 Поиск PVS-Studio...
where "PVS-Studio_Cmd.exe" >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ PVS-Studio найден
    echo 🔍 Запуск PVS-Studio анализа...
    if exist "build-vcpkg\HyperEngine.sln" (
        "PVS-Studio_Cmd.exe" --target "build-vcpkg\HyperEngine.sln" --configuration Debug --platform x64 --output build\static-analysis\pvs-studio-report.plog
    )
) else (
    echo ⚠️ PVS-Studio не найден
)

REM 5. Простой анализ с помощью findstr для поиска потенциальных проблем
echo 🔍 Запуск простого анализа паттернов...
echo Поиск потенциальных проблем в коде... > build\static-analysis\pattern-analysis.txt
echo. >> build\static-analysis\pattern-analysis.txt

echo === Поиск потенциальных утечек памяти === >> build\static-analysis\pattern-analysis.txt
findstr /s /n /i "new\|malloc\|calloc" src\*.cpp include\*.h >> build\static-analysis\pattern-analysis.txt 2>nul
echo. >> build\static-analysis\pattern-analysis.txt

echo === Поиск использования raw pointers === >> build\static-analysis\pattern-analysis.txt
findstr /s /n /i "\*.*=" src\*.cpp include\*.h >> build\static-analysis\pattern-analysis.txt 2>nul
echo. >> build\static-analysis\pattern-analysis.txt

echo === Поиск TODO и FIXME === >> build\static-analysis\pattern-analysis.txt
findstr /s /n /i "TODO\|FIXME\|HACK\|XXX" src\*.cpp include\*.h >> build\static-analysis\pattern-analysis.txt 2>nul
echo. >> build\static-analysis\pattern-analysis.txt

echo === Поиск потенциально небезопасных функций === >> build\static-analysis\pattern-analysis.txt
findstr /s /n /i "strcpy\|strcat\|sprintf\|gets" src\*.cpp include\*.h >> build\static-analysis\pattern-analysis.txt 2>nul

REM 6. Создание сводного отчета
echo 📋 Создание сводного отчета...
(
echo # Отчет статического анализа кода HyperEngine
echo.
echo Дата анализа: %date% %time%
echo.
echo ## Выполненные проверки
echo.
if exist "build\static-analysis\msbuild-analysis.log" (
    echo ### Visual Studio Code Analysis
    echo - Статус: ✅ Выполнен
    echo - Отчет: build\static-analysis\msbuild-analysis.log
    echo.
) else (
    echo ### Visual Studio Code Analysis
    echo - Статус: ❌ Не выполнен ^(инструменты не найдены^)
    echo.
)

if exist "build\static-analysis\clang-tidy-report.txt" (
    echo ### Clang-Tidy
    echo - Статус: ✅ Выполнен
    echo - Отчет: build\static-analysis\clang-tidy-report.txt
    echo.
) else (
    echo ### Clang-Tidy
    echo - Статус: ❌ Не выполнен ^(инструмент не найден^)
    echo.
)

if exist "build\static-analysis\cppcheck-report.xml" (
    echo ### Cppcheck
    echo - Статус: ✅ Выполнен
    echo - Отчет: build\static-analysis\cppcheck-report.xml
    echo.
) else (
    echo ### Cppcheck
    echo - Статус: ❌ Не выполнен ^(инструмент не установлен^)
    echo - Установка: `winget install Cppcheck.Cppcheck`
    echo.
)

if exist "build\static-analysis\pvs-studio-report.plog" (
    echo ### PVS-Studio
    echo - Статус: ✅ Выполнен
    echo - Отчет: build\static-analysis\pvs-studio-report.plog
    echo.
) else (
    echo ### PVS-Studio
    echo - Статус: ❌ Не выполнен ^(инструмент не найден^)
    echo.
)

echo ### Анализ паттернов
echo - Статус: ✅ Выполнен
echo - Отчет: build\static-analysis\pattern-analysis.txt
echo.
echo ## Рекомендации по установке инструментов
echo.
echo 1. **Cppcheck**: `winget install Cppcheck.Cppcheck`
echo 2. **LLVM ^(включает clang-tidy^)**: `winget install LLVM.LLVM`
echo 3. **PVS-Studio**: Скачать с официального сайта
echo.
echo ## Следующие шаги
echo.
echo 1. Просмотрите все найденные предупреждения
echo 2. Исправьте критические ошибки
echo 3. Рассмотрите предложения по улучшению
echo 4. Установите недостающие инструменты для более полного анализа
) > build\static-analysis\summary.md

echo.
echo ✅ Статический анализ завершен. Отчеты в build\static-analysis\
echo 📋 Сводный отчет: build\static-analysis\summary.md
echo.
echo 💡 Для установки дополнительных инструментов анализа:
echo    - Cppcheck: winget install Cppcheck.Cppcheck
echo    - LLVM: winget install LLVM.LLVM
echo    - PVS-Studio: https://pvs-studio.com/
