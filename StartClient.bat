@echo off
REM 神兽竞技场 - 客户端启动脚本

cd /d "%~dp0Binaries\Win64\"

set UA_PROJECTDIR=%~dp0

echo ============================================
echo 神兽竞技场 客户端
echo ============================================
echo.
echo 正在启动客户端，请稍候...
echo.

DivineBeastsArena.exe -Windowed -GameDefaultMap=/Game/Maps/Lobby/LobbyMap.LobbyMap -Log -UserSettingsFolder="%~dp0Saved\Config\WindowsClient"

pause