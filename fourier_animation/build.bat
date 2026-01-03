@echo off
REM ============================================
REM Fourier Epicycles - Build Script
REM ============================================

echo.
echo Configurando entorno de Visual Studio...
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

if not exist build mkdir build
cd build

echo.
echo Configurando CMake...
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release

echo.
echo Compilando...
ninja

echo.
echo ==========================================
echo   Build completado!
echo ==========================================
echo.
echo Para generar un video, usa:
echo   generate_video.bat ^<imagen^>
echo.
