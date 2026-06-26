@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

pushd "%~dp0CampusNavigation"
set EXE=%cd%\campus_nav.exe
popd

pushd "%~dp0测试数据\必做"
set TEST_DIR=%cd%
popd

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

rem use if exist + FOUND flag — no preset of CASE_DIR needed
set FOUND=0
if exist "%TEST_DIR%\small_cases\%CASE%\" (
    set "CASE_DIR=%TEST_DIR%\small_cases\%CASE%"
    set FOUND=1
)
if !FOUND!==0 if exist "%TEST_DIR%\medium_cases\%CASE%\" (
    set "CASE_DIR=%TEST_DIR%\medium_cases\%CASE%"
    set FOUND=1
)
if !FOUND!==0 if exist "%TEST_DIR%\large_cases\%CASE%\" (
    set "CASE_DIR=%TEST_DIR%\large_cases\%CASE%"
    set FOUND=1
)
if "%CASE%"=="sample_ecnu" (
    set "CASE_DIR=%TEST_DIR%\sample_ecnu"
    set FOUND=1
)

if !FOUND!==0 (
    echo [ERROR] Case "%CASE%" not found. Check spelling.
    pause & exit /b
)

pushd "!CASE_DIR!"
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
endlocal