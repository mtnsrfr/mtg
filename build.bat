@echo off
setlocal

echo ============================================================================
echo Building MTG Retro Spielebox (Windows / MSVC x64 + SFML 3.0.0)
echo ============================================================================

cd /d "%~dp0"

:: 1. Locate Visual Studio 2022 vcvars64.bat
set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VCVARS%"=="" (
    echo [ERROR] Could not find Visual Studio 2022 vcvars64.bat.
    exit /b 1
)

call "%VCVARS%" >nul
if errorlevel 1 (
    echo [ERROR] Could not initialize Visual Studio 2022 x64 environment.
    exit /b 1
)

:: 2. Download SFML 3.0.0 if not already present in deps\SFML-3.0.0
if not exist "deps\SFML-3.0.0\include\SFML\Graphics.hpp" (
    echo [0/2] Downloading SFML 3.0.0 for Visual C++ 17 ^(x64^)...
    if not exist "deps" mkdir "deps"
    curl.exe -fL "https://github.com/SFML/SFML/releases/download/3.0.0/SFML-3.0.0-windows-vc17-64-bit.zip" -o "deps\sfml.zip"
    if errorlevel 1 (
        echo [ERROR] Failed to download SFML 3.0.0.
        exit /b 1
    )
    echo Extracting SFML 3.0.0...
    tar.exe -xf "deps\sfml.zip" -C "deps"
    del /q "deps\sfml.zip"
    copy /y "deps\SFML-3.0.0\bin\sfml-graphics-3.dll" . >nul
    copy /y "deps\SFML-3.0.0\bin\sfml-window-3.dll" . >nul
    copy /y "deps\SFML-3.0.0\bin\sfml-audio-3.dll" . >nul
    copy /y "deps\SFML-3.0.0\bin\sfml-system-3.dll" . >nul
)

echo [1/2] Compiling dynamic executable (run_the_games.exe)...
cl /nologo /std:c++17 /EHsc /O2 /utf-8 /D_USE_MATH_DEFINES ^
   /I"deps\SFML-3.0.0\include" main.cc /Fe:run_the_games.exe ^
   /link /LIBPATH:"deps\SFML-3.0.0\lib" ^
   sfml-graphics.lib sfml-window.lib sfml-audio.lib sfml-system.lib
if errorlevel 1 exit /b 1

echo [2/2] Compiling standalone static executable (run_the_games_static.exe)...
cl /nologo /std:c++17 /EHsc /O2 /MD /utf-8 /D_USE_MATH_DEFINES /DSFML_STATIC ^
   /I"deps\SFML-3.0.0\include" main.cc /Fe:run_the_games_static.exe ^
   /link /LIBPATH:"deps\SFML-3.0.0\lib" ^
   sfml-graphics-s.lib sfml-window-s.lib sfml-audio-s.lib sfml-system-s.lib ^
   freetype.lib FLAC.lib vorbisenc.lib vorbisfile.lib vorbis.lib ogg.lib ^
   opengl32.lib winmm.lib gdi32.lib user32.lib advapi32.lib
if errorlevel 1 exit /b 1

echo.
echo Build succeeded!
echo   - Dynamic binary: run_the_games.exe
echo   - Static binary:  run_the_games_static.exe (zero-DLL standalone)
