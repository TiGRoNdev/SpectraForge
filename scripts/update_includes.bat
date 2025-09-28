@echo off
echo Обновление include путей с Engine3D на HyperEngine...

REM Обновление в исходных файлах
for /r src %%f in (*.cpp *.h *.cu) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

REM Обновление в примерах
for /r examples %%f in (*.cpp *.h) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

REM Обновление в тестах
for /r tests %%f in (*.cpp *.h) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

REM Обновление в backup директориях
for /r backup %%f in (*.cpp *.h *.cu) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

REM Обновление в srcVulkan
for /r srcVulkan %%f in (*.cpp *.h *.cu) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

REM Обновление в src3D
for /r src3D %%f in (*.cpp *.h) do (
    if exist "%%f" (
        powershell -Command "(Get-Content '%%f') -replace '#include.*<Engine3D/', '#include <HyperEngine/' | Set-Content '%%f'"
        powershell -Command "(Get-Content '%%f') -replace '#include.*\"Engine3D/', '#include \"HyperEngine/' | Set-Content '%%f'"
    )
)

echo Обновление include путей завершено!
