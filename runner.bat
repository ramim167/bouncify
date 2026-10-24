@echo off
setlocal

set "PROJECT_DIR=%~dp0"
set "SOURCE_FILE=%PROJECT_DIR%iMain.cpp"
set "BUILD_ONLY="
if /i "%~1"=="--build-only" (
    set "BUILD_ONLY=1"
) else if not "%~1"=="" (
    set "SOURCE_FILE=%~f1"
)
if /i "%~2"=="--build-only" set "BUILD_ONLY=1"

pushd "%PROJECT_DIR%" || exit /b 1
if not exist "%SOURCE_FILE%" (
    echo Source file not found: "%SOURCE_FILE%"
    goto :failed
)

:: The supplied OpenGL/SDL libraries require a complete 32-bit compiler.
:: Some copies of the bundled MinGW are missing GCC's standard headers.
set "COMPILER="
call :try_compiler "%PROJECT_DIR%MINGW\bin\g++.exe"
if not defined COMPILER call :try_compiler "C:\MinGW\bin\g++.exe"
if not defined COMPILER (
    for /f "delims=" %%C in ('where g++.exe 2^>nul') do call :try_compiler "%%C"
)
if not defined COMPILER (
    echo No complete 32-bit MinGW C++ compiler found. Install it in C:\MinGW or add it to PATH.
    goto :failed
)
for %%C in ("%COMPILER%") do set "PATH=%%~dpC;%PROJECT_DIR%bin;%PATH%"
if not exist obj mkdir obj
if not exist bin mkdir bin

echo Using compiler: "%COMPILER%"
echo Compiling "%SOURCE_FILE%"...
"%COMPILER%" -std=gnu++11 -Wall -fexceptions -g -I. -IOpenGL\include -IOpenGL\include\SDL2 -c "%SOURCE_FILE%" -o obj\opengl.o
if errorlevel 1 (
    echo Compilation failed.
    goto :failed
)

echo Linking bin\opengl.exe...
"%COMPILER%" -static-libgcc -static-libstdc++ -LOpenGL\lib -o bin\opengl.exe obj\opengl.o -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lopengl32 -lfreeglut -lwinmm
if errorlevel 1 (
    echo Linking failed.
    goto :failed
)
echo Finished building.
if defined BUILD_ONLY (
    popd
    exit /b 0
)

bin\opengl.exe
set "RUN_EXIT=%ERRORLEVEL%"
popd
exit /b %RUN_EXIT%

:failed
popd
exit /b 1

:try_compiler
if defined COMPILER exit /b 0
if not exist "%~1" exit /b 0
"%~1" -dumpmachine 2>nul | findstr /x /c:"mingw32" /c:"i686-w64-mingw32" >nul
if errorlevel 1 exit /b 0
"%~1" -x c++ -fsyntax-only -include stddef.h NUL >nul 2>&1
if not errorlevel 1 set "COMPILER=%~1"
exit /b 0
