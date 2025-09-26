@echo off
echo Testing build after fixes...
cd build-vcpkg
cmake --build . --config Release
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed with error code %ERRORLEVEL%
)
pause
