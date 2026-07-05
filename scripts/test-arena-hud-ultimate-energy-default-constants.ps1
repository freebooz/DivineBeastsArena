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

$controllerCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$characterHeaderPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"

$controllerCpp = Get-Content -Raw -Encoding UTF8 $controllerCppPath
$characterHeader = Get-Content -Raw -Encoding UTF8 $characterHeaderPath

Assert-True ($controllerCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Arena HUD controller to include DBAConstants."
Assert-True ($controllerCpp -match "MaxUltimateEnergy\(DBAConstants::MaxUltimateEnergy\)") "Expected Arena HUD controller default MaxUltimateEnergy to use DBAConstants::MaxUltimateEnergy."
Assert-True ($controllerCpp -notmatch "MaxUltimateEnergy\(100\.0f\)") "Expected Arena HUD controller default MaxUltimateEnergy to avoid hard-coded 100.0f."

Assert-True ($characterHeader.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Zodiac character header to include DBAConstants for HUD cache defaults."
Assert-True ($characterHeader -match "LastSyncedArenaHUDMaxUltimateEnergy\s*=\s*DBAConstants::MaxUltimateEnergy") "Expected Zodiac character HUD max ultimate cache default to use DBAConstants::MaxUltimateEnergy."
Assert-True ($characterHeader -notmatch "LastSyncedArenaHUDMaxUltimateEnergy\s*=\s*100\.0f") "Expected Zodiac character HUD max ultimate cache default to avoid hard-coded 100.0f."

Write-Host "PASS: Arena HUD UltimateEnergy default constants contract" -ForegroundColor Green
