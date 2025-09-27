@echo off
echo =====================================
echo CUDA Kernels Compilation Script
echo =====================================

REM Настройка среды Visual Studio
echo Setting up Visual Studio environment...
call "D:\VSBuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Проверка доступности компилятора
echo Checking cl.exe availability...
where cl.exe
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: cl.exe not found in PATH
    pause
    exit /b 1
)

REM Проверка доступности nvcc
echo Checking nvcc availability...
nvcc --version
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: nvcc not found in PATH
    pause
    exit /b 1
)

REM Создание директории для объектных файлов
if not exist "cuda_build" mkdir cuda_build

REM Настройка параметров компиляции
set CUDA_INCLUDE=-I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.0\include"
set CUDA_FLAGS=-std=c++17 -O2 --gpu-architecture=sm_75 --gpu-architecture=sm_80 --gpu-architecture=sm_86 --gpu-architecture=sm_89 --gpu-architecture=sm_90
set INCLUDE_DIRS=-I"include" -I"include\Engine3D" -I"include\Engine3D\CUDA"

echo =====================================
echo Compiling CUDA kernels...
echo =====================================

REM Компиляция каждого CUDA файла
echo [1/4] Compiling cuda_kernels.cu...
nvcc -c srcVulkan\CUDA\cuda_kernels.cu -o cuda_build\cuda_kernels.obj %CUDA_INCLUDE% %INCLUDE_DIRS% %CUDA_FLAGS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile cuda_kernels.cu
    pause
    exit /b 1
)

echo [2/4] Compiling gaussian_optimization.cu...
nvcc -c srcVulkan\CUDA\gaussian_optimization.cu -o cuda_build\gaussian_optimization.obj %CUDA_INCLUDE% %INCLUDE_DIRS% %CUDA_FLAGS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile gaussian_optimization.cu
    pause
    exit /b 1
)

echo [3/4] Compiling tile_rasterization.cu...
nvcc -c srcVulkan\CUDA\tile_rasterization.cu -o cuda_build\tile_rasterization.obj %CUDA_INCLUDE% %INCLUDE_DIRS% %CUDA_FLAGS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile tile_rasterization.cu
    pause
    exit /b 1
)

echo [4/4] Compiling depth_sorting.cu...
nvcc -c srcVulkan\CUDA\depth_sorting.cu -o cuda_build\depth_sorting.obj %CUDA_INCLUDE% %INCLUDE_DIRS% %CUDA_FLAGS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to compile depth_sorting.cu
    pause
    exit /b 1
)

REM Создание статической библиотеки из объектных файлов
echo =====================================
echo Creating CUDA static library...
echo =====================================
lib /OUT:cuda_build\cuda_kernels.lib cuda_build\*.obj
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to create CUDA library
    pause
    exit /b 1
)

echo =====================================
echo SUCCESS: CUDA kernels compiled!
echo Output: cuda_build\cuda_kernels.lib
echo =====================================

REM Копирование в build директорию для интеграции
if exist "build-vcpkg\Release" (
    echo Copying to build directory...
    copy cuda_build\cuda_kernels.lib build-vcpkg\Release\
    echo CUDA library copied to build directory
)

echo.
echo To integrate with project:
echo 1. Link cuda_kernels.lib in your project
echo 2. Remove cuda_stubs.cpp from compilation
echo 3. Rebuild the project
echo.
pause
