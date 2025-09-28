@echo off
REM Миграция устаревшего кода в новую архитектуру (Windows версия)

echo 🔄 Миграция устаревшего кода в новую архитектуру...

REM 1. Массовая замена namespace'ов во всех файлах
echo 📝 Этап 1: Замена namespace'ов Engine3D → HyperEngine...

REM Создать PowerShell скрипт для замены
echo $files = Get-ChildItem -Path . -Include *.cpp,*.h -Recurse ^| Where-Object { $_.FullName -notmatch "\\build" -and $_.FullName -notmatch "\\vcpkg" } > temp_migrate.ps1
echo foreach ($file in $files) { >> temp_migrate.ps1
echo     if (Select-String -Path $file.FullName -Pattern "Engine3D" -Quiet) { >> temp_migrate.ps1
echo         Write-Host "  🔄 Обрабатывается: $($file.FullName)" >> temp_migrate.ps1
echo         $content = Get-Content $file.FullName -Raw >> temp_migrate.ps1
echo         $content = $content -replace "namespace Engine3D", "namespace HyperEngine" >> temp_migrate.ps1
echo         $content = $content -replace "Engine3D::", "HyperEngine::" >> temp_migrate.ps1
echo         $content = $content -replace "using namespace Engine3D", "using namespace HyperEngine" >> temp_migrate.ps1
echo         $content = $content -replace "#include `"Engine3D/", "#include `"HyperEngine/" >> temp_migrate.ps1
echo         $content = $content -replace "#include ^<Engine3D/", "#include ^<HyperEngine/" >> temp_migrate.ps1
echo         Set-Content -Path $file.FullName -Value $content >> temp_migrate.ps1
echo         Write-Host "  ✅ Обновлен: $($file.FullName)" >> temp_migrate.ps1
echo     } >> temp_migrate.ps1
echo } >> temp_migrate.ps1

REM Выполнить PowerShell скрипт
powershell -ExecutionPolicy Bypass -File temp_migrate.ps1

REM Удалить временный скрипт
del temp_migrate.ps1

REM 2. Создание новой структуры директорий
echo 📁 Этап 2: Создание новой структуры директорий...

mkdir src\core 2>nul
mkdir src\math 2>nul
mkdir src\rendering\common 2>nul
mkdir src\rendering\opengl 2>nul
mkdir src\rendering\vulkan 2>nul
mkdir src\physics 2>nul
mkdir src\input 2>nul
mkdir src\cuda 2>nul
mkdir src\optix 2>nul

mkdir include\HyperEngine\Core 2>nul
mkdir include\HyperEngine\Math 2>nul
mkdir include\HyperEngine\Rendering 2>nul
mkdir include\HyperEngine\Physics 2>nul
mkdir include\HyperEngine\Input 2>nul
mkdir include\HyperEngine\Vulkan 2>nul
mkdir include\HyperEngine\CUDA 2>nul
mkdir include\HyperEngine\OptiX 2>nul

echo   ✅ Структура директорий создана

REM 3. Переименование директории include/Engine3D в include/HyperEngine
echo 📂 Этап 3: Переименование директорий...

if exist "include\Engine3D" (
    if not exist "include\HyperEngine" (
        echo   🔄 Переименование include\Engine3D → include\HyperEngine
        move "include\Engine3D" "include\HyperEngine"
        echo   ✅ Директория переименована
    ) else (
        echo   ℹ️ Директория include\HyperEngine уже существует
    )
) else (
    echo   ℹ️ Директория include\Engine3D не найдена
)

REM 4. Обновление CMakeLists.txt
echo ⚙️ Этап 4: Обновление системы сборки...

if exist "CMakeLists.txt" (
    echo   🔄 Обновление CMakeLists.txt
    
    REM Создать резервную копию
    copy "CMakeLists.txt" "CMakeLists.txt.backup" >nul
    
    REM Создать PowerShell скрипт для замены в CMakeLists.txt
    echo $content = Get-Content "CMakeLists.txt" -Raw > temp_cmake.ps1
    echo $content = $content -replace "Engine3D", "HyperEngine" >> temp_cmake.ps1
    echo $content = $content -replace "src3D", "src" >> temp_cmake.ps1
    echo $content = $content -replace "srcVulkan", "src" >> temp_cmake.ps1
    echo Set-Content -Path "CMakeLists.txt" -Value $content >> temp_cmake.ps1
    
    powershell -ExecutionPolicy Bypass -File temp_cmake.ps1
    del temp_cmake.ps1
    
    echo   ✅ CMakeLists.txt обновлен
)

REM 5. Генерация отчета о миграции
echo 📊 Этап 5: Генерация отчета о миграции...

REM Создать отчет о прогрессе
echo # Прогресс миграции кода > docs\refactoring\migration_progress.md
echo. >> docs\refactoring\migration_progress.md
echo **Дата последнего обновления:** %date% %time% >> docs\refactoring\migration_progress.md
echo. >> docs\refactoring\migration_progress.md
echo ## 🔄 Выполненные операции >> docs\refactoring\migration_progress.md
echo 1. ✅ Замена namespace'ов Engine3D → HyperEngine >> docs\refactoring\migration_progress.md
echo 2. ✅ Обновление include путей >> docs\refactoring\migration_progress.md
echo 3. ✅ Создание новой структуры директорий >> docs\refactoring\migration_progress.md
echo 4. ✅ Переименование include/Engine3D → include/HyperEngine >> docs\refactoring\migration_progress.md
echo 5. ✅ Обновление CMakeLists.txt >> docs\refactoring\migration_progress.md
echo. >> docs\refactoring\migration_progress.md
echo ## 📋 Следующие шаги >> docs\refactoring\migration_progress.md
echo - Проверить сборку проекта >> docs\refactoring\migration_progress.md
echo - Запустить тесты >> docs\refactoring\migration_progress.md
echo - Обновить документацию >> docs\refactoring\migration_progress.md
echo - Создать PR с изменениями >> docs\refactoring\migration_progress.md

echo   ✅ Отчет о миграции создан: docs\refactoring\migration_progress.md

echo.
echo 🎉 МИГРАЦИЯ ЗАВЕРШЕНА!
echo 📋 Следующие шаги:
echo   1. Проверьте сборку: cmake --build build-vcpkg
echo   2. Запустите тесты: ctest
echo   3. Проверьте отчет: docs\refactoring\migration_progress.md
echo   4. Создайте коммит с изменениями

pause
