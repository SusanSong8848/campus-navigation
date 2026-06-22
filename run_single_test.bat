@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set EXE=%~dp0CampusNavigation\campus_nav.exe
set TEST_DIR=%~dp0测试数据\必做

if not "%1"=="" set CASE=%1
if "%CASE%"=="" (
    echo Available test cases:
    echo   Small : case_01  case_02  case_03  case_04
    echo   ECNU  : sample_ecnu
    echo   Medium: chain_flower_100  grid_100  ring_100  star_100  random_sparse_100  mixed_time_100
    echo   Large : chain_1000  random_sparse_10000
    echo.
    set /p CASE=Enter case name: 
)
if "%CASE%"=="" (echo No case name. Exiting. & pause & exit /b)

set CASE_DIR=
for /d %%d in ("%TEST_DIR%\small_cases\%CASE%") do set CASE_DIR=%%d
if not defined CASE_DIR for /d %%d in ("%TEST_DIR%\medium_cases\%CASE%") do set CASE_DIR=%%d
if not defined CASE_DIR for /d %%d in ("%TEST_DIR%\large_cases\%CASE%") do set CASE_DIR=%%d
if "%CASE%"=="sample_ecnu" set CASE_DIR=%TEST_DIR%\sample_ecnu

if not defined CASE_DIR (
    echo [ERROR] Case "%CASE%" not found. Check spelling.
    pause & exit /b
)

pushd "%CASE_DIR%"
echo.
echo === Running: %CASE% ===
"%EXE%" < command.txt > my_answer.txt 2>nul
echo.

fc /w my_answer.txt answer.txt >nul 2>nul
if !errorlevel!==0 (
    echo [PASS] Output matches answer.txt
    echo ==================== Output ====================
    type my_answer.txt
) else (
    echo [FAIL] Output differs from answer.txt
    echo ================= Your Output =================
    type my_answer.txt
    echo ============== Expected Answer ================
    type answer.txt
)
echo ===================================================

popd
echo.
pause
