<#
Validates local Zodiac character UltimateEnergy readiness drives the Arena HUD UltimateReadyPrompt.
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

Assert-True ($header -match "bHasSyncedArenaHUDUltimateReadyPrompt") "Expected UltimateReadyPrompt sync state flag."
Assert-True ($header -match "bLastSyncedArenaHUDUltimateReady") "Expected cached ultimate-ready prompt state."

Assert-True ($cpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected character implementation to use DBAConstants for ultimate threshold."
Assert-True ($cpp -match "const float MaxUltimateEnergy\s*=\s*DBAConstants::MaxUltimateEnergy") "Expected max ultimate energy to use DBAConstants."
Assert-True ($cpp -match "const bool bUltimateReady\s*=\s*CurrentUltimateEnergy\s*>=\s*MaxUltimateEnergy") "Expected ultimate-ready state from current/max ultimate energy."
Assert-True ($cpp -match "const bool bUltimateReadyPromptChanged") "Expected ultimate-ready prompt change detection."
Assert-True ($cpp -match "bUltimateReady\s*!=\s*bLastSyncedArenaHUDUltimateReady") "Expected ultimate-ready cache comparison."
Assert-True ($cpp -match "if\s*\(\s*bForce\s*\|\|\s*!bHasSyncedArenaHUDUltimateReadyPrompt\s*\|\|\s*bUltimateReadyPromptChanged\s*\)") "Expected prompt update gated by force/cache/change detection."
Assert-True ($cpp -match "if\s*\(\s*bUltimateReady\s*\)[\s\S]*ShowArenaHUDUltimateReadyPrompt\(\)") "Expected ready state to show prompt."
Assert-True ($cpp -match "else[\s\S]*HideArenaHUDUltimateReadyPrompt\(\)") "Expected non-ready state to hide prompt."
Assert-True ($cpp -match "bLastSyncedArenaHUDUltimateReady\s*=\s*bUltimateReady") "Expected ultimate-ready cache refresh."
Assert-True ($cpp -match "bHasSyncedArenaHUDUltimateReadyPrompt\s*=\s*true") "Expected prompt sync flag refresh."

Write-Host "PASS: Zodiac character Arena HUD UltimateReadyPrompt contract" -ForegroundColor Green
