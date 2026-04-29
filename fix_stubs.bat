@echo off
REM Stub replacer for DivineBeastsArena build - handles GameplayAbilities temp file conflict
echo Starting build preparation...

REM Clean up any stale Intermediate\Source temp files that conflict with real Target.cs
if exist "D:\DivineBeastsArena\Intermediate\Source" (
    echo Cleaning stale Intermediate/Source...
    rd /s /q "D:\DivineBeastsArena\Intermediate\Source" 2>nul
)

REM Clean build intermediates to force clean rebuild
if exist "D:\DivineBeastsArena\Intermediate\Build\Win64\DivineBeastsArena" (
    echo Cleaning build cache...
    rd /s /q "D:\DivineBeastsArena\Intermediate\Build\Win64\DivineBeastsArena" 2>nul
)

echo Build preparation complete