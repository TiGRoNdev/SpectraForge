@echo off
echo Компиляция шейдеров для 4D Engine Vulkan Forward+...

set VULKAN_SDK_DIR=%VULKAN_SDK%

if not defined VULKAN_SDK (
    echo ОШИБКА: Переменная среды VULKAN_SDK не установлена!
    echo Убедитесь, что Vulkan SDK установлен и переменная среды настроена.
    pause
    exit /b 1
)

set GLSLC=%VULKAN_SDK%\Bin\glslc.exe

if not exist "%GLSLC%" (
    echo ОШИБКА: glslc.exe не найден в %VULKAN_SDK%\Bin\
    echo Убедитесь, что Vulkan SDK установлен корректно.
    pause
    exit /b 1
)

mkdir shaders\compiled 2>nul

echo Компиляция depth prepass шейдеров...
"%GLSLC%" shaders\depth_prepass.vert -o shaders\compiled\depth_prepass.vert.spv
if %ERRORLEVEL% neq 0 (
    echo ОШИБКА: Не удалось скомпилировать depth_prepass.vert
    pause
    exit /b 1
)

"%GLSLC%" shaders\depth_prepass.frag -o shaders\compiled\depth_prepass.frag.spv
if %ERRORLEVEL% neq 0 (
    echo ОШИБКА: Не удалось скомпилировать depth_prepass.frag
    pause
    exit /b 1
)

echo Компиляция light culling compute шейдера...
"%GLSLC%" shaders\light_culling.comp -o shaders\compiled\light_culling.comp.spv
if %ERRORLEVEL% neq 0 (
    echo ОШИБКА: Не удалось скомпилировать light_culling.comp
    pause
    exit /b 1
)

echo Компиляция forward+ shading шейдеров...
"%GLSLC%" shaders\forward_plus.vert -o shaders\compiled\forward_plus.vert.spv
if %ERRORLEVEL% neq 0 (
    echo ОШИБКА: Не удалось скомпилировать forward_plus.vert
    pause
    exit /b 1
)

"%GLSLC%" shaders\forward_plus.frag -o shaders\compiled\forward_plus.frag.spv
if %ERRORLEVEL% neq 0 (
    echo ОШИБКА: Не удалось скомпилировать forward_plus.frag
    pause
    exit /b 1
)

echo.
echo Все шейдеры успешно скомпилированы!
echo Скомпилированные файлы находятся в папке shaders\compiled\
echo.
pause
