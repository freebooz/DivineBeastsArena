<#
Validates local Zodiac character critical HP/Energy hints feed the Arena HUD event feedback path.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$header = Get-Content -Raw -Encoding UTF8 -LiteralPath $headerPath
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath

Assert-True ($header -match "ArenaHUDCriticalHealthRatioThreshold") "Expected configurable critical HP ratio threshold."
Assert-True ($header -match "ArenaHUDCriticalEnergyRatioThreshold") "Expected configurable critical Energy ratio threshold."
Assert-True ($header -match "LastSyncedArenaHUDBLowHP") "Expected cached low HP HUD state."
Assert-True ($header -match "LastSyncedArenaHUDBLowEnergy") "Expected cached low Energy HUD state."
Assert-True ($header -match "bHasSyncedArenaHUDCriticalState") "Expected critical state sync flag."

Assert-True ($cpp -match "const float HealthRatio\s*=\s*MaxHP\s*>\s*0\.0f\s*\?\s*CurrentHP\s*/\s*MaxHP\s*:\s*0\.0f") "Expected HealthRatio calculation from current and max HP."
Assert-True ($cpp -match "const float EnergyRatio\s*=\s*MaxEnergy\s*>\s*0\.0f\s*\?\s*CurrentEnergy\s*/\s*MaxEnergy\s*:\s*0\.0f") "Expected EnergyRatio calculation from current and max Energy."
Assert-True ($cpp -match "const bool bLowHP\s*=\s*HealthRatio\s*<=\s*ArenaHUDCriticalHealthRatioThreshold") "Expected low HP state to use threshold."
Assert-True ($cpp -match "const bool bLowEnergy\s*=\s*EnergyRatio\s*<=\s*ArenaHUDCriticalEnergyRatioThreshold") "Expected low Energy state to use threshold."
Assert-True ($cpp -match "const bool bCriticalStateChanged") "Expected critical state change detection."
Assert-True ($cpp -match "bLowHP\s*!=\s*LastSyncedArenaHUDBLowHP") "Expected low HP cache comparison."
Assert-True ($cpp -match "bLowEnergy\s*!=\s*LastSyncedArenaHUDBLowEnergy") "Expected low Energy cache comparison."
Assert-True ($cpp -match "if\s*\(\s*bForce\s*\|\|\s*!bHasSyncedArenaHUDCriticalState\s*\|\|\s*bCriticalStateChanged\s*\)") "Expected critical state update to be gated by force/cache/change detection."
Assert-True ($cpp -match "UpdateArenaHUDCriticalStateHints\(\s*bLowHP,\s*bLowEnergy\s*\)") "Expected Zodiac character to push critical state hints through UI manager."
Assert-True ($cpp -match "LastSyncedArenaHUDBLowHP\s*=\s*bLowHP") "Expected low HP cache refresh."
Assert-True ($cpp -match "LastSyncedArenaHUDBLowEnergy\s*=\s*bLowEnergy") "Expected low Energy cache refresh."
Assert-True ($cpp -match "bHasSyncedArenaHUDCriticalState\s*=\s*true") "Expected critical state sync flag refresh."

Write-Host "PASS: Zodiac character Arena HUD critical state contract" -ForegroundColor Green
