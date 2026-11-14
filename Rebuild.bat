@echo off
rem TAKE 4471 - rebuild the C++ module and open the editor, without the "missing modules" dance.
rem Close Unreal first, then double-click this file.
setlocal
set UE=C:\Program Files\Epic Games\UE_5.8
set PROJ=%~dp0DirectorsCut_58.uproject
set LOG=%~dp0Saved\Logs\Rebuild.log

tasklist /FI "IMAGENAME eq UnrealEditor.exe" | find /I "UnrealEditor.exe" >nul
if not errorlevel 1 (
    echo Unreal is still open. Close it first ^(check Task Manager for UnrealEditor.exe^), then run this again.
    pause
    exit /b 1
)
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"
echo Building DirectorsCut_58Editor (Win64 Development)... log: %LOG%
call "%UE%\Engine\Build\BatchFiles\Build.bat" DirectorsCut_58Editor Win64 Development -Project="%PROJ%" -WaitMutex -NoHotReload > "%LOG%" 2>&1
if errorlevel 1 (
    echo.
    echo BUILD FAILED. The errors:
    findstr /C:": error" "%LOG%"
    echo.
    echo Full log: %LOG%
    pause
    exit /b 1
)
echo Build OK. Opening the editor...
start "" "%UE%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJ%"
endlocal
