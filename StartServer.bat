@echo off
REM 神兽竞技场 - 服务端启动脚本

cd /d "%~dp0Binaries\Win64\"

set UA_PROJECTDIR=%~dp0
set LOGFILE=%~dp0Saved\Logs\Server.log

echo ============================================
echo 神兽竞技场 服务端
echo ============================================
echo.

DivineBeastsArenaServer.exe -Windowed -log -UserSettingsFolder="%~dp0Saved\Config\WindowsServer" -GameDefaultMap=/Game/Maps/Lobby/LobbyMap.LobbyMap -ServerDefaultMap=/Game/Maps/Lobby/LobbyMap.LobbyMap -Port=7777 -QueryPort=27017

pause