@echo off
REM 确保窗口支持中文显示
chcp 65001 > nul
cls

echo ==========================================================
echo       多尺寸 ICO 图标转换器 (支持透明背景)
echo ==========================================================
echo.
echo   此脚本会将您拖入的 PNG 文件转换为一个保留了透明
echo   背景的、包含多种标准尺寸的专业 .ico 文件。
echo.
echo ==========================================================
echo.

REM --- 检查 FFmpeg 是否存在 ---
where ffmpeg >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未在系统路径中找到 ffmpeg.exe !
    echo.
    echo 请先下载 FFmpeg 并将其 bin 目录添加到系统环境变量中。
    echo.
    pause
    exit /b
)

REM --- 检查是否有文件拖入 ---
if "%~1"=="" (
    echo [提示] 请将一个带透明背景的 .png 文件拖拽到此脚本图标上。
    echo.
    pause
    exit /b
)

REM --- 设置输入和输出文件名 ---
set "INPUT_FILE=%~1"
set "OUTPUT_FILE=%~dp1%~n1.ico"

echo [信息] 输入文件: "%INPUT_FILE%"
echo [信息] 输出文件: "%OUTPUT_FILE%"
echo.
echo [处理中] 正在生成多尺寸图标 (保留透明度)，请稍候...
echo.

REM --- FFmpeg 核心命令 (已更新) ---
REM 核心改动：我们在处理图片的一开始就加入了 "format=rgba" 这个指令，
REM 强制 FFmpeg 在整个处理流程中都保留 Alpha (透明度) 通道。

ffmpeg -y -i "%INPUT_FILE%" -filter_complex "[0:v]format=rgba,split=5[s1][s2][s3][s4][s5]; [s1]scale=256:256[o1]; [s2]scale=64:64[o2]; [s3]scale=48:48[o3]; [s4]scale=32:32[o4]; [s5]scale=16:16[o5]" -map "[o1]" -map "[o2]" -map "[o3]" -map "[o4]" -map "[o5]" "%OUTPUT_FILE%"

REM --- 检查命令是否执行成功 ---
if %errorlevel% equ 0 (
    echo.
    echo [成功] 转换完成!
    echo 新的图标文件 "%OUTPUT_FILE%" 已生成，并保留了透明背景。
) else (
    echo.
    echo [失败] 转换过程中发生错误。
    echo 请确保您的输入文件是带透明背景的 .png 格式。
)

echo.
pause```

---

### 如何使用

1.  确保您的**源文件**是一个**带透明背景的 `.png` 文件**。如果源文件是 `.jpg` 或其他没有透明通道的格式，那么转换后背景依然会是黑或白色。
2.  将这个 `.png` 文件**拖拽到新的 `ConvertTo-ICO.bat` 脚本图标上**。
3.  脚本会自动运行，这次生成的 `ico.ico` 文件在文件管理器中预览时，背景应该会是**透明的**（或者是文件管理器窗口的背景色），而不是纯黑色。

现在，您就有了一个能制作出专业级、带透明背景图标的完美本地工具了。