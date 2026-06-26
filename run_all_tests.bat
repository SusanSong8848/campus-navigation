@echo off
chcp 65001 >nul 2>nul
setlocal enabledelayedexpansion

pushd "%~dp0CampusNavigation"
set EXE=%cd%\campus_nav.exe
popd

pushd "%~dp0测试数据\必做"
set TEST_DIR=%cd%
popd

echo ========================================
echo   CampusNavigation - Full Integration Test
echo ========================================
echo.

set PASS=0
set FAIL=0
set TOTAL=0

echo --- Small Cases ---
for /d %%d in ("%TEST_DIR%\small_cases\*") do (
    set /a TOTAL+=1
    pushd "%%d"
    "%EXE%" < command.txt > my_answer.txt 2>nul
    fc /w my_answer.txt answer.txt >nul 2>nul
    if !errorlevel!==0 (
        echo [PASS] %%~nxd
        set /a PASS+=1
    ) else (
        echo [FAIL] %%~nxd
        set /a FAIL+=1
    )
    popd
)

echo --- Sample ECNU ---
set /a TOTAL+=1
pushd "%TEST_DIR%\sample_ecnu"
"%EXE%" < command.txt > my_answer.txt 2>nul
fc /w my_answer.txt answer.txt >nul 2>nul
if !errorlevel!==0 (
    echo [PASS] sample_ecnu
    set /a PASS+=1
) else (
    echo [FAIL] sample_ecnu
    set /a FAIL+=1
)
popd

echo --- Medium Cases ---
for /d %%d in ("%TEST_DIR%\medium_cases\*") do (
    set /a TOTAL+=1
    pushd "%%d"
    "%EXE%" < command.txt > my_answer.txt 2>nul
    fc /w my_answer.txt answer.txt >nul 2>nul
    if !errorlevel!==0 (
        echo [PASS] %%~nxd
        set /a PASS+=1
    ) else (
        echo [FAIL] %%~nxd
        set /a FAIL+=1
    )
    popd
)

echo --- Large Cases ---
for /d %%d in ("%TEST_DIR%\large_cases\*") do (
    set /a TOTAL+=1
    pushd "%%d"
    "%EXE%" < command.txt > my_answer.txt 2>nul
    fc /w my_answer.txt answer.txt >nul 2>nul
    if !errorlevel!==0 (
        echo [PASS] %%~nxd
        set /a PASS+=1
    ) else (
        echo [FAIL] %%~nxd
        set /a FAIL+=1
    )
    popd
)

echo.
echo ========================================
echo   Result: %PASS% passed, %FAIL% failed (total %TOTAL%)
echo ========================================
pause
endlocal