@echo off
setlocal

REM Build LFOz Tool VST3 on Windows 11 with Visual Studio 2022.
REM Requirements:
REM   - Git
REM   - CMake 3.22+
REM   - Visual Studio 2022 with "Desktop development with C++"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Release --target LFOzTool_VST3
if errorlevel 1 exit /b %errorlevel%

echo.
echo Build finished.
echo VST3 output is usually here:
echo   build\LFOzTool_artefacts\Release\VST3\LFOz Tool.vst3
echo.
echo Copy "LFOz Tool.vst3" to:
echo   C:\Program Files\Common Files\VST3\
echo then rescan plugins in FL Studio.
