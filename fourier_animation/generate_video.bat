@echo off
REM ============================================
REM Fourier Epicycles - Video Generator
REM ============================================

setlocal enabledelayedexpansion

REM Configurar entorno de Visual Studio
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

REM Directorio del script
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%build"

REM Valores por defecto
set "IMAGE="
set "OUTPUT=fourier_output.mp4"
set "CIRCLES=150"
set "FRAMES=1800"
set "FPS=60"
set "WIDTH=1920"
set "HEIGHT=1080"

REM Procesar argumentos
:parse_args
if "%~1"=="" goto check_args
if /i "%~1"=="-i" set "IMAGE=%~2" & shift & shift & goto parse_args
if /i "%~1"=="-o" set "OUTPUT=%~2" & shift & shift & goto parse_args
if /i "%~1"=="-n" set "CIRCLES=%~2" & shift & shift & goto parse_args
if /i "%~1"=="-f" set "FRAMES=%~2" & shift & shift & goto parse_args
if /i "%~1"=="--fps" set "FPS=%~2" & shift & shift & goto parse_args
if /i "%~1"=="-w" set "WIDTH=%~2" & shift & shift & goto parse_args
if /i "%~1"=="-h" set "HEIGHT=%~2" & shift & shift & goto parse_args
if /i "%~1"=="--help" goto show_help
REM Si no hay flag, asumir que es la imagen
if "!IMAGE!"=="" set "IMAGE=%~1"
shift
goto parse_args

:check_args
if "!IMAGE!"=="" goto show_help

echo.
echo ==========================================
echo   Fourier Epicycles Video Generator
echo ==========================================
echo.
echo  Imagen:     !IMAGE!
echo  Salida:     !OUTPUT!
echo  Epiciclos:  !CIRCLES!
echo  Frames:     !FRAMES! (duracion: !FRAMES!/!FPS! segundos)
echo  FPS:        !FPS!
echo  Resolucion: !WIDTH!x!HEIGHT!
echo.
echo ==========================================
echo.

fourier_animation.exe "!IMAGE!" -o "!OUTPUT!" -n !CIRCLES! -f !FRAMES! --fps !FPS! -w !WIDTH! -h !HEIGHT!

echo.
echo Video generado: !OUTPUT!
goto end

:show_help
echo.
echo USO: generate_video.bat ^<imagen^> [opciones]
echo.
echo OPCIONES:
echo   -i ^<imagen^>    Ruta a la imagen de entrada
echo   -o ^<salida^>    Ruta del video de salida (default: fourier_output.mp4)
echo   -n ^<num^>       Numero de epiciclos (default: 150)
echo   -f ^<num^>       Numero de frames (default: 1800, ~30 segundos)
echo   --fps ^<num^>    Frames por segundo (default: 60)
echo   -w ^<num^>       Ancho del video (default: 1920)
echo   -h ^<num^>       Alto del video (default: 1080)
echo   --help         Mostrar esta ayuda
echo.
echo EJEMPLOS:
echo   generate_video.bat mi_imagen.png
echo   generate_video.bat logo.png -o mi_video.mp4 -n 200 -f 3600
echo   generate_video.bat silueta.png -f 7200 -n 300
echo.
echo DURACIONES TIPICAS:
echo   -f 600   = 10 segundos
echo   -f 1800  = 30 segundos
echo   -f 3600  = 1 minuto
echo   -f 7200  = 2 minutos
echo.

:end
endlocal
