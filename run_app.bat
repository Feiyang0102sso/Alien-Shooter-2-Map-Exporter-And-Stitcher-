@echo off
REM --- 这个 .bat 文件是为最终用户准备的 ---
chcp 65001 > nul

:menu
cls
echo ========================================
echo  地图拼接工具
echo  Map Stitching Tool
echo ========================================
echo.
echo  1. English
echo  2. 中文 (Chinese)
echo.
echo ========================================
echo.

set "lang="
set /p choice="请在此处输入选项 (1 或 2) 然后按 Enter | Enter selection (1 or 2) and press Enter: "

if "%choice%"=="1" set "lang=en"
if "%choice%"=="2" set "lang=zh"

if not defined lang (
    echo. & echo    无效输入! | Invalid input! & pause & goto menu
)

if "%lang%"=="zh" (
    set "prompt_path=请输入地图碎片的文件夹路径 (可拖拽文件夹到此窗口): "
    set "prompt_compress=请输入压缩目标大小(MB)，直接回车则不压缩: "
) else (
    set "prompt_path=Please enter the path to the map fragments folder (you can drag the folder here): "
    set "prompt_compress=Enter target size for compression in MB. Press Enter to skip: "
)

echo. & echo ======================================== & echo.
set /p "map_dir_raw=%prompt_path%"
echo.
set /p "compress_size=%prompt_compress%"
set "map_dir=%map_dir_raw:"=%"
echo.

REM --- 调用与 .bat 文件在同一个目录下的 AS2MapStitcher.exe ---
"%~dp0AS2MapStitcher.exe" %lang% "%map_dir%" "%compress_size%"

echo.
echo ==========================================================
if "%lang%"=="zh" (echo               --- 程序执行完毕 ---) else (echo               --- Program finished ---)
echo ==========================================================
echo.
pause