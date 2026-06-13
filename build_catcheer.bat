@echo off
setlocal enabledelayedexpansion

:: colors
for /f %%a in ('powershell -Command "[char]27"') do set "ESC=%%a"
set "ORANGE=%ESC%[38;5;214m"
set "RESET=%ESC%[0m"

echo %ORANGE%=== Catcheer Windows Build ===%RESET%

:: env setup
where nmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo %ORANGE%=== MSVC env not found, searching... ===%RESET%
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set "VS_PATH=%%i"
        )
        if exist "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat" (
            echo %ORANGE%=== Loading vcvars64.bat... ===%RESET%
            call "!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat" >nul
        )
    )
)

:: verify again
where nmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo %ESC%[31m[ERROR] nmake not found. Install VS with C++ support.%RESET%
    pause
    exit /b 1
)

if not exist build mkdir build
cd build

:: clean
if exist CMakeCache.txt del /f /q CMakeCache.txt

echo %ORANGE%=== generating build files con MSVC (NMake) ===%RESET%
cmake .. -G "NMake Makefiles"

echo %ORANGE%=== compile Catcheer ===%RESET%
nmake

echo %ORANGE%=== compile completed ===%RESET%
pause
