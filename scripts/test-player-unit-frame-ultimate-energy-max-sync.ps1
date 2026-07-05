param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$frameHeaderPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.h"
$frameCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.cpp"
$rootCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"

$frameHeader = Get-Content -Raw -Encoding UTF8 $frameHeaderPath
$frameCpp = Get-Content -Raw -Encoding UTF8 $frameCppPath
$rootCpp = Get-Content -Raw -Encoding UTF8 $rootCppPath

Assert-True ($frameHeader -match "UpdateUltimateEnergyWithMax\s*\(\s*float\s+Energy,\s*float\s+MaxEnergy\s*\)") "Expected PlayerUnitFrame to expose a max-aware UltimateEnergy update entrypoint."
Assert-True ($frameHeader -match "CachedMaxUltimateEnergy") "Expected PlayerUnitFrame to cache max UltimateEnergy for percentage calculation."
Assert-True ($frameCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected PlayerUnitFrame implementation to include DBAConstants for the legacy default path."
Assert-True ($frameCpp -match "UpdateUltimateEnergy\s*\(\s*float\s+Energy\s*\)[\s\S]*UpdateUltimateEnergyWithMax\(Energy,\s*DBAConstants::MaxUltimateEnergy\)") "Expected legacy UltimateEnergy update to delegate through the constant-backed max-aware path."
Assert-True ($frameCpp -match "UpdateUltimateEnergyWithMax\s*\(\s*float\s+Energy,\s*float\s+MaxEnergy\s*\)") "Expected max-aware UltimateEnergy update implementation."
Assert-True ($frameCpp -match "CachedMaxUltimateEnergy\s*=\s*FMath::Max\(1\.0f,\s*MaxEnergy\)") "Expected max UltimateEnergy to be guarded against zero before percentage calculation."
Assert-True ($frameCpp -match "CachedUltimateEnergy\s*=\s*FMath::Clamp\(Energy,\s*0\.0f,\s*CachedMaxUltimateEnergy\)") "Expected UltimateEnergy to clamp against the dynamic max."
Assert-True ($frameCpp -match "Percentage\s*=\s*CachedUltimateEnergy\s*/\s*CachedMaxUltimateEnergy") "Expected UltimateEnergy percentage to use the dynamic max."
Assert-True ($frameCpp -notmatch "CachedUltimateEnergy[\s\S]{0,140}100\.0f") "Expected PlayerUnitFrame UltimateEnergy logic to avoid hard-coded 100.0f."
Assert-True ($rootCpp -match "PlayerUnitFrame->UpdateUltimateEnergyWithMax\(CurrentEnergy,\s*MaxEnergy\)") "Expected Arena HUD root to forward current and max UltimateEnergy to PlayerUnitFrame."

Write-Host "PASS: PlayerUnitFrame UltimateEnergy max sync contract" -ForegroundColor Green
