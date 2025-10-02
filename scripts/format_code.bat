@echo off
chcp 65001 >nul
REM Скрипт для форматирования кода с помощью clang-format

echo 🎨 Форматирование кода SpectraForge...
echo ===================================

REM Проверяем наличие clang-format
where clang-format >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ clang-format не найден в системе
    echo 📥 Варианты установки:
    echo    1. Visual Studio Installer (компонент "Clang-format")
    echo    2. LLVM: https://releases.llvm.org/
    echo    3. Chocolatey: choco install llvm
    if exist "vcpkg\vcpkg.exe" (
        echo    4. vcpkg: vcpkg install llvm[clang-format]:x64-windows
    )
    exit /b 1
)

REM Создаем директорию для отчетов
if not exist "..\build\quality-reports" mkdir "..\build\quality-reports"

echo 🔍 Поиск файлов C++...
set FILES_FOUND=0

REM Форматируем файлы в src
for /r ..\src %%f in (*.cpp *.h) do (
    echo Форматирование: %%f
    clang-format -i --style=file "%%f"
    if %errorlevel% neq 0 (
        echo ❌ Ошибка форматирования: %%f
    ) else (
        set /a FILES_FOUND+=1
    )
)

REM Форматируем файлы в include
for /r ..\include %%f in (*.cpp *.h) do (
    echo Форматирование: %%f
    clang-format -i --style=file "%%f"
    if %errorlevel% neq 0 (
        echo ❌ Ошибка форматирования: %%f
    ) else (
        set /a FILES_FOUND+=1
    )
)

REM Форматируем файлы в tests
for /r ..\tests %%f in (*.cpp *.h) do (
    echo Форматирование: %%f
    clang-format -i --style=file "%%f"
    if %errorlevel% neq 0 (
        echo ❌ Ошибка форматирования: %%f
    ) else (
        set /a FILES_FOUND+=1
    )
)

REM Форматируем файлы в examples
for /r ..\examples %%f in (*.cpp *.h) do (
    echo Форматирование: %%f
    clang-format -i --style=file "%%f"
    if %errorlevel% neq 0 (
        echo ❌ Ошибка форматирования: %%f
    ) else (
        set /a FILES_FOUND+=1
    )
)

echo.
echo ✅ Форматирование завершено!
echo 📊 Обработано файлов: %FILES_FOUND%
echo.

REM Проверяем результат
echo 🔍 Проверка результата форматирования...
clang-format --dry-run --Werror --style=file ..\src\**\*.cpp ..\src\**\*.h ..\include\**\*.cpp ..\include\**\*.h ..\tests\**\*.cpp ..\tests\**\*.h ..\examples\**\*.cpp ..\examples\**\*.h > ..\build\quality-reports\format-check-after.log 2>&1

if %errorlevel% equ 0 (
    echo ✅ Все файлы соответствуют стандартам форматирования!
) else (
    echo ⚠️ Некоторые файлы все еще требуют внимания. См. build\quality-reports\format-check-after.log
)

echo.
echo 📋 Отчет сохранен в build\quality-reports\
