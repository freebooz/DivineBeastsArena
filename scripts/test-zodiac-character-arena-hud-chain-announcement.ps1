<#
Validates local Zodiac character chain-ready state triggers Arena HUD combat announcements once per ready transition.
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
$chainReadyText = [string]::Concat([char[]](36830, 38145, 23601, 32490))

Assert-True ($header -match "ArenaHUDChainReadyAnnouncementDuration") "Expected configurable chain-ready announcement duration."
Assert-True ($header -match "bLastSyncedArenaHUDChainReady") "Expected cached chain-ready HUD announcement state."

Assert-True ($cpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected character implementation to use DBAConstants for chain threshold."
Assert-True ($cpp -match "const bool bChainReady\s*=\s*CurrentChainLevel\s*>=\s*DBAConstants::MaxChainLevel") "Expected chain-ready state to use DBAConstants::MaxChainLevel."
Assert-True ($cpp -match "if\s*\(\s*bChainReady\s*&&\s*!bLastSyncedArenaHUDChainReady\s*\)") "Expected chain-ready announcement only on false-to-true transition."
Assert-True ($cpp.Contains(('ShowArenaHUDCombatAnnouncement(NSLOCTEXT("DBAArenaHUD", "ChainReadyAnnouncement", "' + $chainReadyText + '"), ArenaHUDChainReadyAnnouncementDuration)'))) "Expected chain-ready announcement text and duration to route through UI manager."
Assert-True ($cpp -notmatch "Chain Ready") "Expected chain-ready announcement not to use English placeholder text."
Assert-True ($cpp -match "bLastSyncedArenaHUDChainReady\s*=\s*bChainReady") "Expected chain-ready cache refresh."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 36830, 38145, 23601, 32490, 25112, 26007, 25552, 31034, 24050, 25509, 20837, 20013, 25991, 25991, 26412))
Write-Host $successMessage -ForegroundColor Green
