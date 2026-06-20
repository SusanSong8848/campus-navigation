@echo off
setlocal enabledelayedexpansion
set EXE=d:\C_CPP\CPP\test\DataStructure_EOJ\校园导航\campus-navigation\CampusNavigation\campus_nav.exe
set TEST_DIR=d:\C_CPP\CPP\test\DataStructure_EOJ\校园导航\campus-navigation\测试数据\必做

echo ========================================
echo   校园导航系统 — 全量集成测试
echo ========================================
echo.

set PASS=0
set FAIL=0
set TOTAL=0

:: ================ small_cases ================
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

:: ================ sample_ecnu ================
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

:: ================ medium_cases ================
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

:: ================ large_cases ================
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
echo   测试结果: %PASS% 通过, %FAIL% 失败 (共 %TOTAL% 个)
echo ========================================
endlocal