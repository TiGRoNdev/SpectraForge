@echo off
REM setup_sdk.bat - Автоматическая настройка SDK для SpectraForge
REM Этот скрипт проверяет наличие необходимых SDK и помогает с их настройкой

echo ========================================
echo SpectraForge SDK Setup Script
echo ========================================
echo.

REM Проверка CUDA Toolkit
echo [1/4] Проверка CUDA Toolkit...
where nvcc >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo ✓ CUDA Toolkit найден
    nvcc --version | findstr "release"
) else (
    echo ✗ CUDA Toolkit не найден
    echo   Скачайте с: https://developer.nvidia.com/cuda-downloads
    echo   Рекомендуемая версия: 11.8 или новее
)
echo.

REM Проверка OptiX SDK
echo [2/4] Проверка OptiX SDK...
if exist "%OPTIX_ROOT%\include\optix.h" (
    echo ✓ OptiX SDK найден: %OPTIX_ROOT%
) else (
    echo ✗ OptiX SDK не найден
    echo   Установите переменную окружения OPTIX_ROOT
    echo   Пример: set OPTIX_ROOT=C:\ProgramData\NVIDIA Corporation\OptiX SDK 7.7.0
    echo   Скачайте с: https://developer.nvidia.com/optix
)
echo.

REM Проверка DLSS SDK (Streamline)
echo [3/4] Проверка DLSS SDK (опционально)...
if exist "%STREAMLINE_ROOT%\include\sl.h" (
    echo ✓ DLSS SDK найден: %STREAMLINE_ROOT%
) else (
    echo ⚠ DLSS SDK не найден (опционально)
    echo   Установите переменную окружения STREAMLINE_ROOT
    echo   Пример: set STREAMLINE_ROOT=C:\Program Files\NVIDIA Corporation\Streamline
    echo   Скачайте с: https://developer.nvidia.com/rtx/streamline
)
echo.

REM Проверка FidelityFX SDK
echo [4/4] Проверка FidelityFX SDK...
if exist "%FIDELITYFX_ROOT%\sdk\include\FidelityFX\host\ffx_fsr2.h" (
    echo ✓ FidelityFX SDK найден: %FIDELITYFX_ROOT%
) else (
    echo ⚠ FidelityFX SDK не найден
    echo   Установите переменную окружения FIDELITYFX_ROOT
    echo   Или клонируйте: git clone https://github.com/GPUOpen-Effects/FidelityFX-FSR2.git
)
echo.

REM Проверка Vulkan SDK
echo Дополнительно: Проверка Vulkan SDK...
where vulkaninfo >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo ✓ Vulkan SDK найден
    vulkaninfo --summary 2>nul | findstr "Vulkan Instance Version"
) else (
    echo ✗ Vulkan SDK не найден
    echo   Скачайте с: https://vulkan.lunarg.com/
)
echo.

REM Проверка vcpkg
echo Дополнительно: Проверка vcpkg...
if exist "vcpkg\vcpkg.exe" (
    echo ✓ vcpkg найден
) else (
    echo ⚠ vcpkg не найден в текущей директории
    echo   Выполните: git clone https://github.com/Microsoft/vcpkg.git
    echo   Затем: vcpkg\bootstrap-vcpkg.bat
)
echo.

echo ========================================
echo Рекомендации по сборке:
echo ========================================
echo.
echo Минимальная конфигурация (без дополнительных SDK):
echo cmake .. -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake ^
echo          -DBUILD_VULKAN_RENDERER=ON ^
echo          -DBUILD_WITH_CUDA=OFF ^
echo          -DBUILD_WITH_OPTIX=OFF ^
echo          -DBUILD_WITH_DLSS=OFF ^
echo          -DBUILD_WITH_FSR=OFF
echo.
echo Полная конфигурация (все SDK):
echo cmake .. -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake ^
echo          -DBUILD_VULKAN_RENDERER=ON ^
echo          -DBUILD_WITH_CUDA=ON ^
echo          -DBUILD_WITH_OPTIX=ON ^
echo          -DBUILD_WITH_DLSS=ON ^
echo          -DBUILD_WITH_FSR=ON
echo.
echo Для получения подробной информации см. docs/SDK_SETUP_GUIDE.md
echo.

pause
