@echo off
echo Building HyperEngine with vcpkg...

REM Install dependencies
echo Installing dependencies with vcpkg...
vcpkg install

REM Create build directory
if not exist "build-vcpkg" mkdir build-vcpkg
cd build-vcpkg

REM Configure with vcpkg
echo Configuring with CMake and vcpkg...
cmake .. -DCMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64

REM Build
echo Building...
cmake --build . --config Release

echo Build complete!
pause
