@echo off
REM Физическое перемещение файлов в новую структуру

echo 🔄 Реструктуризация директорий HyperEngine...
echo ============================================

REM Создать резервную копию
echo 📦 Создание резервной копии...
if not exist backup mkdir backup
xcopy src3D backup\src3D\ /E /I /Y >nul 2>&1
xcopy srcVulkan backup\srcVulkan\ /E /I /Y >nul 2>&1
echo   ✅ Резервная копия создана в backup/

REM Создать новую структуру src/
echo 📁 Создание новой структуры src/...
if not exist src mkdir src
if not exist src\core mkdir src\core
if not exist src\math mkdir src\math
if not exist src\rendering mkdir src\rendering
if not exist src\rendering\common mkdir src\rendering\common
if not exist src\rendering\opengl mkdir src\rendering\opengl
if not exist src\rendering\vulkan mkdir src\rendering\vulkan
if not exist src\physics mkdir src\physics
if not exist src\input mkdir src\input
if not exist src\cuda mkdir src\cuda
if not exist src\optix mkdir src\optix
if not exist src\upscaling mkdir src\upscaling
echo   ✅ Структура src/ создана

REM Перемещение файлов из src3D/
echo 🚚 Перемещение файлов из src3D/...

REM Core компоненты
if exist src3D\Core (
    echo   📦 Core компоненты...
    copy src3D\Core\*.cpp src\core\ >nul 2>&1
    copy src3D\Core\*.h src\core\ >nul 2>&1
)

REM Math библиотека
if exist src3D\Math (
    echo   🔢 Math библиотека...
    copy src3D\Math\*.cpp src\math\ >nul 2>&1
    copy src3D\Math\*.h src\math\ >nul 2>&1
)

REM Rendering система (OpenGL)
if exist src3D\Rendering (
    echo   🎨 Rendering система...
    copy src3D\Rendering\*.cpp src\rendering\opengl\ >nul 2>&1
    copy src3D\Rendering\*.h src\rendering\opengl\ >nul 2>&1
)

REM Physics система
if exist src3D\Physics (
    echo   ⚡ Physics система...
    copy src3D\Physics\*.cpp src\physics\ >nul 2>&1
    copy src3D\Physics\*.h src\physics\ >nul 2>&1
)

REM Input система
if exist src3D\Input (
    echo   🎮 Input система...
    copy src3D\Input\*.cpp src\input\ >nul 2>&1
    copy src3D\Input\*.h src\input\ >nul 2>&1
)

REM Перемещение файлов из srcVulkan/
echo 🚚 Перемещение файлов из srcVulkan/...

REM Vulkan рендерер
if exist srcVulkan\Vulkan (
    echo   🌋 Vulkan рендерер...
    copy srcVulkan\Vulkan\*.cpp src\rendering\vulkan\ >nul 2>&1
    copy srcVulkan\Vulkan\*.h src\rendering\vulkan\ >nul 2>&1
)

REM CUDA интеграция
if exist srcVulkan\CUDA (
    echo   🚀 CUDA интеграция...
    copy srcVulkan\CUDA\*.cpp src\cuda\ >nul 2>&1
    copy srcVulkan\CUDA\*.cu src\cuda\ >nul 2>&1
    copy srcVulkan\CUDA\*.h src\cuda\ >nul 2>&1
)

REM OptiX ray tracing
if exist srcVulkan\OptiX (
    echo   ✨ OptiX ray tracing...
    copy srcVulkan\OptiX\*.cpp src\optix\ >nul 2>&1
    copy srcVulkan\OptiX\*.h src\optix\ >nul 2>&1
)

REM Upscaling системы
if exist srcVulkan\Upscaling (
    echo   📈 Upscaling системы...
    copy srcVulkan\Upscaling\*.cpp src\upscaling\ >nul 2>&1
    copy srcVulkan\Upscaling\*.h src\upscaling\ >nul 2>&1
)

REM CMakeLists.txt уже создан отдельно
echo ⚙️ CMakeLists.txt для src/ уже создан
echo   ✅ CMakeLists.txt готов

REM Создать отчет о перемещении
echo 📊 Создание отчета о перемещении...
echo # Отчет о реструктуризации директорий > docs\refactoring\restructure_report.md
echo. >> docs\refactoring\restructure_report.md
echo **Дата:** %date% %time% >> docs\refactoring\restructure_report.md
echo. >> docs\refactoring\restructure_report.md
echo ## Перемещенные файлы >> docs\refactoring\restructure_report.md
echo. >> docs\refactoring\restructure_report.md
echo ### src3D/ → src/ >> docs\refactoring\restructure_report.md
echo - Core: src3D/Core/ → src/core/ >> docs\refactoring\restructure_report.md
echo - Math: src3D/Math/ → src/math/ >> docs\refactoring\restructure_report.md
echo - Rendering: src3D/Rendering/ → src/rendering/opengl/ >> docs\refactoring\restructure_report.md
echo - Physics: src3D/Physics/ → src/physics/ >> docs\refactoring\restructure_report.md
echo - Input: src3D/Input/ → src/input/ >> docs\refactoring\restructure_report.md
echo. >> docs\refactoring\restructure_report.md
echo ### srcVulkan/ → src/ >> docs\refactoring\restructure_report.md
echo - Vulkan: srcVulkan/Vulkan/ → src/rendering/vulkan/ >> docs\refactoring\restructure_report.md
echo - CUDA: srcVulkan/CUDA/ → src/cuda/ >> docs\refactoring\restructure_report.md
echo - OptiX: srcVulkan/OptiX/ → src/optix/ >> docs\refactoring\restructure_report.md
echo - Upscaling: srcVulkan/Upscaling/ → src/upscaling/ >> docs\refactoring\restructure_report.md

echo.
echo 🎉 РЕСТРУКТУРИЗАЦИЯ ЗАВЕРШЕНА!
echo =============================
echo.
echo ✅ Файлы перемещены в новую структуру src/
echo ✅ Создана резервная копия в backup/
echo ✅ Создан новый CMakeLists.txt
echo ✅ Создан отчет о перемещении
echo.
echo 📋 Следующие шаги:
echo   1. Проверить сборку: cmake --build build-vcpkg
echo   2. Запустить тесты
echo   3. Обновить основной CMakeLists.txt
echo.

pause
