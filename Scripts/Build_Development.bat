@echo off
REM Copyright FreeboozStudio. All Rights Reserved.
REM 使用源代码引擎编译项目

REM 引擎路径
set ENGINE_ROOT=D:\Program Files\Epic Games\UE_5.7
set ENGINE_BUILD=%ENGINE_ROOT%\Engine\Build\BatchFiles
set ENGINE_BINARIES=%ENGINE_ROOT%\Engine\Binaries\Win64

REM 项目路径
set PROJECT_ROOT=%~dp0..
set PROJECT_NAME=DivineBeastsArena

echo ========================================
echo 神兽竞技场 - Development 构建脚本
echo ========================================
echo.

REM 检查引擎路径
if not exist "%ENGINE_BUILD%\Build.bat" (
    echo [错误] 未找到引擎 Build.bat: %ENGINE_BUILD%\Build.bat
    echo 请确认引擎路径配置正确
    pause
    exit /b 1
)

REM 编译参数
set BUILD_CONFIG=Development
set BUILD_TARGET=Editor
set BUILD_PLATFORM=Win64

echo [1/3] 正在编译 %PROJECT_NAME% (%BUILD_CONFIG% | %BUILD_TARGET%)...
echo.

cd /d "%PROJECT_ROOT%"

REM 执行编译
call "%ENGINE_BUILD%\Build.bat" %BUILD_TARGET% %PROJECT_NAME% %BUILD_CONFIG% -Project="%PROJECT_ROOT%\%PROJECT_NAME%.uproject" -BuildMachine -NoEngineChanges -NoHotReloadFromIDE

if %ERRORLEVEL% neq 0 (
    echo.
    echo [错误] 编译失败，错误码: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/3] 编译成功!
echo.

REM 可选：启动编辑器
set /p START_EDITOR="是否启动编辑器? (Y/N): "
if /i "%START_EDITOR%"=="Y" (
    echo [3/3] 正在启动编辑器...
    start "" "%ENGINE_BINARIES%\%BUILD_TARGET%.exe" "%PROJECT_ROOT%\%PROJECT_NAME%.uproject"
)

echo.
echo ========================================
echo 构建完成
echo ========================================
pause
