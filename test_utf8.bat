@echo off
chcp 65001
echo.
echo ========================================
echo         HYPERENGINE UTF-8 ТЕСТ
echo ========================================
echo.
echo Устанавливаем UTF-8 кодировку...
echo Текущая кодовая страница: %ERRORLEVEL%
echo.
echo Запускаем простой тест UTF-8...
echo.
cd build-test\Release
SimpleUTF8Test.exe
echo.
echo ========================================
echo           ТЕСТ ЗАВЕРШЕН
echo ========================================
pause
