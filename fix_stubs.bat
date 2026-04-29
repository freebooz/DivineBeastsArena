@echo off
REM Quick stub replacer for DivineBeastsArena build
echo Starting build with stub fix...

:retry
REM Wait for Intermediate\Source to be created
if not exist "D:\DivineBeastsArena\Intermediate\Source" (
    timeout /t 1 /nobreak >nul
    goto retry
)

REM Copy real files over stubs
copy /Y "D:\DivineBeastsArena\Source\DivineBeastsArena\DivineBeastsArena.Build.cs" "D:\DivineBeastsArena\Intermediate\Source\DivineBeastsArena.Build.cs" >nul 2>&1
copy /Y "D:\DivineBeastsArena\Source\DivineBeastsArena\DivineBeastsArena.Target.cs" "D:\DivineBeastsArena\Intermediate\Source\DivineBeastsArena.Target.cs" >nul 2>&1
copy /Y "D:\DivineBeastsArena\Source\DivineBeastsArena\DivineBeastsArenaEditor.Target.cs" "D:\DivineBeastsArena\Intermediate\Source\DivineBeastsArenaEditor.Target.cs" >nul 2>&1
copy /Y "D:\DivineBeastsArena\Source\DivineBeastsArena\DivineBeastsArenaServer.Target.cs" "D:\DivineBeastsArena\Intermediate\Source\DivineBeastsArenaServer.Target.cs" >nul 2>&1

echo Stub files replaced with real files
