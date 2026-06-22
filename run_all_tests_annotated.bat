@echo off
REM ============================================================
REM  @echo off — 禁止显示每条命令本身，只显示命令的输出，让界面干净
REM  @ 前缀也作用于这一行自身，连 "echo off" 这条命令也不会显示
REM ============================================================

REM setlocal enabledelayedexpansion — 开启"延迟环境变量展开"
REM 这样在 for 循环内部可以用 !变量名! 读取实时更新的值（而不是循环前的旧值）
setlocal enabledelayedexpansion

REM ============================================================
REM 定义 EXE 变量：指向编译好的校园导航程序
REM 注意：如果 exe 路径变了，只需要改这一行
REM ============================================================
set EXE=d:\C_CPP\CPP\test\DataStructure_EOJ\校园导航\campus-navigation\CampusNavigation\campus_nav.exe

REM ============================================================
REM 定义 TEST_DIR 变量：指向必做测试数据的根目录
REM ============================================================
set TEST_DIR=d:\C_CPP\CPP\test\DataStructure_EOJ\校园导航\campus-navigation\测试数据\必做

echo ========================================
echo   校园导航系统 — 全量集成测试
echo ========================================
echo.  REM echo. 输出一个空行

REM 初始化计数器
set PASS=0    REM 通过的测试数
set FAIL=0    REM 失败的测试数
set TOTAL=0   REM 总测试数

REM ================ small_cases ================
echo --- Small Cases ---

REM for /d — 只匹配目录（文件夹），不匹配普通文件
REM %%d — for 循环的迭代变量，每次循环中代表当前找到的文件夹完整路径
REM       注意：在 .bat 文件中必须用 %%d，在命令行直接敲时用一个 %d
for /d %%d in ("%TEST_DIR%\small_cases\*") do (
    set /a TOTAL+=1
    REM set /a — /a 表示算术运算（Arithmetic），把 TOTAL 的值加 1
    REM 如果没有 /a，set TOTAL=%TOTAL%+1 只会赋值为字符串 "0+1"，不会算出 1

    pushd "%%d"
    REM pushd — 保存当前目录，然后切换到 "%%d"（当前测试用例文件夹）
    REM 比 cd 更安全：不用记住从哪来的，popd 自动恢复。用栈结构保存路径

    REM 下面这行做了三件事：
    REM   < command.txt  — 把 command.txt 的内容作为程序的键盘输入（stdin）
    REM   > my_answer.txt — 把程序的屏幕输出保存到 my_answer.txt（覆盖原有文件）
    REM   2>nul           — 把标准错误输出流（stderr，文件描述符=2）丢进空设备 nul
    REM                     nul = Windows 的黑洞，写入即消失，不显示
    "%EXE%" < command.txt > my_answer.txt 2>nul

    REM fc — File Compare，比较两个文件内容
    REM /w — 忽略空白字符（空格、制表符等），只比较实质性内容
    REM >nul 2>nul — 把 fc 的比较结果和错误都隐藏，只需要它的退出码 errorlevel
    fc /w my_answer.txt answer.txt >nul 2>nul

    REM errorlevel — 上一条命令（fc）的退出码
    REM   0：两文件完全相同
    REM   1：文件不同
    REM   >1：比较过程出错（如文件不存在）
    REM !errorlevel! — 用 ! 包裹才能在循环内读到实时值（受益于 setlocal enabledelayedexpansion）
    if !errorlevel!==0 (
        REM %%~nxd — 扩展语法，只取文件夹名（去掉路径前缀）
        REM 显示为 case_01 而非完整路径
        echo [PASS] %%~nxd
        set /a PASS+=1
    ) else (
        echo [FAIL] %%~nxd
        set /a FAIL+=1
    )

    popd
    REM popd — 回到 pushd 之前保存的目录，准备处理下一个用例
)

REM ================ sample_ecnu ================
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

REM ================ medium_cases ================
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

REM ================ large_cases ================
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
REM %PASS% — 循环结束后用 % 包裹即可（此时已不在循环体内部，不需要延迟展开）
echo ========================================

endlocal
REM endlocal — 结束本地变量作用域，恢复 setlocal 之前的变量状态
REM 防止脚本中的临时变量（PASS/FAIL/TOTAL）污染命令行后续操作