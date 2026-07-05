<#
Validates AuraSummaryPanel native-to-Blueprint update behavior.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAAuraSummaryPanelWidgetBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAuraSummaryPanelWidgetBase.cpp"

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

Assert-True ($header -match "UpdateAuraCount\s*\(\s*int32\s+Count\s*\)") "Expected AuraSummaryPanel to expose UpdateAuraCount."
Assert-True ($header -match "BP_OnAuraCountUpdated\s*\(\s*int32\s+Count\s*\)") "Expected AuraSummaryPanel to declare AuraCount BP event."
Assert-True ($header -match "CachedAuraCount\s*=\s*0") "Expected AuraSummaryPanel to cache the latest normalized aura count."
Assert-True ($header -match "ShowAuraDetails\s*\(\s*\)") "Expected AuraSummaryPanel to expose ShowAuraDetails."
Assert-True ($header -match "BP_OnAuraDetailsRequested\s*\(\s*\)") "Expected AuraSummaryPanel to declare AuraDetails BP event."
Assert-True ($cpp -match "NativeConstruct[\s\S]{0,180}BP_OnAuraCountUpdated\(CachedAuraCount\)") "Expected AuraSummaryPanel NativeConstruct to replay cached aura count."
Assert-True ($cpp -match "UpdateAuraCount[\s\S]*NormalizedAuraCount\s*=\s*FMath::Max\(\s*0,\s*Count\s*\)") "Expected AuraSummaryPanel to normalize AuraCount to at least 0."
Assert-True ($cpp -match "CachedAuraCount\s*=\s*NormalizedAuraCount") "Expected AuraSummaryPanel UpdateAuraCount to cache normalized count."
Assert-True ($cpp -match "BP_OnAuraCountUpdated\(CachedAuraCount\)") "Expected AuraSummaryPanel UpdateAuraCount to broadcast cached normalized count."
Assert-True ($cpp -match "ShowAuraDetails[\s\S]*BP_OnAuraDetailsRequested\(\)") "Expected AuraSummaryPanel ShowAuraDetails to forward detail requests to Blueprint."

Write-Host "PASS: Arena HUD AuraSummaryPanel contract" -ForegroundColor Green
