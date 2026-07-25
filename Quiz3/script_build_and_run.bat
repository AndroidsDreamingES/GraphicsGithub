@echo off
REM Desktop build and run for VertexArrayObject-Modern.
REM Requires CMake 3.15+ and Visual Studio 2022 in PATH.

cmake --version >nul 2>&1
IF %ERRORLEVEL% NEQ 0 ( echo CMake not found. & pause & exit /b 1 )

echo [1/3] Configuring ...
IF EXIST build_desktop RMDIR /S /Q build_desktop
MKDIR build_desktop
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
IF %ERRORLEVEL% NEQ 0 ( echo Config failed. & pause & exit /b 1 )

echo [2/3] Building ...
cmake --build build_desktop --config Debug
IF %ERRORLEVEL% NEQ 0 ( echo Build failed. & pause & exit /b 1 )

echo [3/3] Running ...
echo Rotating cube (VAO) + spinning grid (VAO). Click to pause cube. ESC to quit.
pushd build_desktop\Debug
VertexArrayObject.exe
popd
pause
