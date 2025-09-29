@echo off
echo Building HyperEngine with vcpkg and Ninja...

REM Add ninja to PATH for this session
set PATH=%~dp0tools;%PATH%

REM Check if ninja is available
ninja --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Ninja not found! Please ensure ninja.exe is in the tools directory.
    pause
    exit /b 1
)

REM Install dependencies
echo Installing dependencies with vcpkg...
cd vcpkg
call .\vcpkg.exe install
cd ..

REM Create build directory
if not exist "build-vcpkg" mkdir build-vcpkg
cd build-vcpkg

REM Configure with vcpkg and Ninja
echo Configuring with CMake, vcpkg and Ninja...
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

REM Build
echo Building with Ninja...
ninja

echo Build complete!
echo Compile commands exported to: build-vcpkg\compile_commands.json
