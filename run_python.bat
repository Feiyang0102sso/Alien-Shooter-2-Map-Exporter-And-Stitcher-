@echo off
REM 关键第一步：确保窗口支持中文显示
chcp 65001 > nul

:menu
cls
echo ========================================
echo  地图拼接工具 (Python 测试模式)
echo  Map Stitching Tool (Python Test Mode)
echo ========================================
echo.
echo  1. English
echo  2. 中文 (Chinese)
echo.
echo ========================================
echo.

REM --- 步骤 1: 使用最稳定的 set /p 获取语言选择 ---
set "lang="
set /p choice="请在此处输入选项 (1 或 2) 然后按 Enter | Enter selection (1 or 2) and press Enter: "

if "%choice%"=="1" set "lang=en"
if "%choice%"=="2" set "lang=zh"

REM 如果输入无效，则返回菜单
if not defined lang (
    echo.
    echo    无效输入! | Invalid input!
    pause
    goto menu
)

REM --- 步骤 2: 根据语言设置所有提示语 ---
if "%lang%"=="zh" (
    set "prompt_path=请输入地图碎片的文件夹路径 (可拖拽文件夹到此窗口): "
    set "prompt_compress=请输入压缩目标大小(MB)，直接回车则不压缩: "
    set "msg_executing=--- 准备执行Python脚本 ---"
    set "msg_finished=--- 脚本执行完毕 ---"
) else (
    set "prompt_path=Please enter the path to the map fragments folder (you can drag the folder here): "
    set "prompt_compress=Enter target size for compression in MB. Press Enter to skip: "
    set "msg_executing=--- Preparing to execute Python script ---"
    set "msg_finished=--- Script execution finished ---"
)

echo.
echo ========================================
echo.

REM --- 步骤 3: 获取用户输入 ---
set /p "map_dir_raw=%prompt_path%"
echo.
set /p "compress_size=%prompt_compress%"

REM --- 步骤 4: 关键！自动清理用户输入的路径中可能包含的引号 ---
set "map_dir=%map_dir_raw:"=%

echo.
echo ==========================================================
REM --- 关键改动：使用语言变量 ---
echo               %msg_executing%
echo ==========================================================
echo.

REM --- 步骤 5: 使用清理过的变量安全地调用 Python ---
python "%~dp0main.py" %lang% "%map_dir%" "%compress_size%"

echo.
echo ==========================================================
REM --- 关键改动：使用语言变量 ---
echo               %msg_finished%
echo ==========================================================
echo.
pause