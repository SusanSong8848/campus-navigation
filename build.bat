@echo off
echo ========================================
echo   CampusNavigation - Build Script
echo ========================================
echo.
echo Select build method:
echo   1. g++ direct compile (recommended, no CMake needed)
echo   2. CMake build (requires CMake installed)
echo.
set /p MODE=Enter 1 or 2: 

if "%MODE:~0,1%"=="1" goto gpp
if "%MODE:~0,1%"=="2" goto cmake
echo Invalid input. Please run again.
pause
exit /b

:gpp
echo.
echo [g++] Compiling...
cd /d "%~dp0CampusNavigation"
g++ -std=c++17 -o campus_nav.exe main.cpp LGraph.cpp LocationInfo.cpp Algorithm.cpp CsvIO.cpp CommandProcessor.cpp -I.
if %errorlevel%==0 (
    echo [OK] Build successful! campus_nav.exe generated.
) else (
    echo [ERROR] Build failed. Check error messages above.
    echo Hint: Did you forget to include CommandProcessor.cpp?
)
cd /d "%~dp0"
echo.
echo Press any key to exit...
pause >nul
exit /b

:cmake
echo.
echo [CMake] Building...
cd /d "%~dp0CampusNavigation"

if exist build (
    echo Removing old build directory...
    rmdir /s /q build 2>nul
    if exist build (
        echo [WARNING] build directory is locked, skipping removal
    )
)
mkdir build 2>nul
cd build

cmake ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    cd /d "%~dp0"
    pause
    exit /b
)

cmake --build .
if %errorlevel% equ 0 (
    echo [OK] Build successful!
    echo exe location: CampusNavigation\build\Debug\CampusNavigation.exe
) else (
    echo [ERROR] Build failed!
    echo Hint: If you see C4819/C2059 encoding errors,
    echo        make sure CMakeLists.txt has /utf-8 option.
)
cd /d "%~dp0"
echo.
echo Press any key to exit...
pause >nul
exit /b