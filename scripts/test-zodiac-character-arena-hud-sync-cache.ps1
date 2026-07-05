<#
Validates Zodiac character Arena HUD sync avoids redundant unchanged UI updates.
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

Assert-True ($header -match "SyncArenaHUDFromAttributes\s*\(\s*bool\s+bForce\s*=\s*false\s*\)") "Expected Arena HUD sync to expose optional force flag."
Assert-True ($header -match "bHasSyncedArenaHUDAttributes") "Expected cached Arena HUD sync state flag."
Assert-True ($header -match "LastSyncedArenaHUDCurrentHP") "Expected cached HP value."
Assert-True ($header -match "LastSyncedArenaHUDCurrentEnergy") "Expected cached Energy value."
Assert-True ($header -match "LastSyncedArenaHUDHeroLevel") "Expected cached HeroLevel value."

Assert-True ($cpp -match "SyncArenaHUDFromAttributes\(true\)") "Expected BeginPlay to force initial Arena HUD sync."
Assert-True ($cpp -match "const bool bVitalsChanged") "Expected vitals change detection."
Assert-True ($cpp -match "const bool bLevelChanged") "Expected level change detection."
Assert-True ($cpp -match "FMath::IsNearlyEqual\(CurrentHP,\s*LastSyncedArenaHUDCurrentHP,\s*KINDA_SMALL_NUMBER\)") "Expected HP nearly-equal cache comparison."
Assert-True ($cpp -match "if\s*\(\s*bForce\s*\|\|\s*!bHasSyncedArenaHUDAttributes\s*\|\|\s*bVitalsChanged\s*\)") "Expected vitals update to be gated by force/cache/change detection."
Assert-True ($cpp -match "if\s*\(\s*bForce\s*\|\|\s*!bHasSyncedArenaHUDAttributes\s*\|\|\s*bLevelChanged\s*\)") "Expected level update to be gated by force/cache/change detection."
Assert-True ($cpp -match "LastSyncedArenaHUDCurrentHP\s*=\s*CurrentHP") "Expected HP cache refresh."
Assert-True ($cpp -match "LastSyncedArenaHUDHeroLevel\s*=\s*HeroLevel") "Expected HeroLevel cache refresh."
Assert-True ($cpp -match "bHasSyncedArenaHUDAttributes\s*=\s*true") "Expected sync state to be marked after update."

Write-Host "PASS: Zodiac character Arena HUD sync cache contract" -ForegroundColor Green
