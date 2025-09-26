@echo off
echo Building 4D Engine (Simple Version)...

REM Create build directory
if not exist build_simple mkdir build_simple
cd build_simple

REM Configure with CMake
cmake .. -f ../CMakeLists_simple.txt -G "Visual Studio 17 2022" -A x64

REM Build
cmake --build . --config Release

echo Build complete!
pause
