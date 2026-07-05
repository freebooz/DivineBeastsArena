<#
Validates SelfCastBar native-to-Blueprint progress behavior.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBASelfCastBarWidgetBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBASelfCastBarWidgetBase.cpp"

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

Assert-True ($header -match "ShowSelfCastProgress\s*\(\s*float\s+Duration\s*\)") "Expected SelfCastBar to expose ShowSelfCastProgress."
Assert-True ($header -match "HideSelfCastProgress\s*\(\s*\)") "Expected SelfCastBar to expose HideSelfCastProgress."
Assert-True ($header -match "BP_OnSelfCastProgress\s*\(\s*float\s+Duration,\s*float\s+RemainingTime\s*\)") "Expected SelfCastBar to declare self-cast progress BP event."
Assert-True ($header -match "CachedSelfCastDuration") "Expected SelfCastBar to cache self-cast duration."
Assert-True ($header -match "bCachedSelfCastVisible") "Expected SelfCastBar to cache self-cast visibility."

Assert-True ($cpp -match "NativeConstruct[\s\S]{0,260}if\s*\(\s*bCachedSelfCastVisible\s*\)[\s\S]{0,180}BP_OnSelfCastProgress\(CachedSelfCastDuration,\s*CachedSelfCastDuration\)") "Expected NativeConstruct to replay visible cached self-cast progress after widget construction."
Assert-True ($cpp -match "NativeConstruct[\s\S]{0,440}else[\s\S]{0,160}BP_OnSelfCastProgress\(0\.0f,\s*0\.0f\)") "Expected NativeConstruct to replay hidden self-cast state after widget construction."
Assert-True ($cpp -match "ShowSelfCastProgress[\s\S]*NormalizedDuration\s*=\s*FMath::Max\(\s*0\.0f,\s*Duration\s*\)") "Expected ShowSelfCastProgress to normalize duration to at least 0."
Assert-True ($cpp -match "CachedSelfCastDuration\s*=\s*NormalizedDuration") "Expected ShowSelfCastProgress to cache normalized duration."
Assert-True ($cpp -match "bCachedSelfCastVisible\s*=\s*true") "Expected ShowSelfCastProgress to cache visible state."
Assert-True ($cpp -match "ShowSelfCastProgress[\s\S]*BP_OnSelfCastProgress\(CachedSelfCastDuration,\s*CachedSelfCastDuration\)") "Expected ShowSelfCastProgress to broadcast cached duration and remaining time."
Assert-True ($cpp -match "CachedSelfCastDuration\s*=\s*0\.0f") "Expected HideSelfCastProgress to reset cached duration."
Assert-True ($cpp -match "bCachedSelfCastVisible\s*=\s*false") "Expected HideSelfCastProgress to cache hidden state."
Assert-True ($cpp -match "HideSelfCastProgress[\s\S]*BP_OnSelfCastProgress\(0\.0f,\s*0\.0f\)") "Expected HideSelfCastProgress to clear progress through the BP event."

Write-Host "PASS: Arena HUD SelfCastBar contract" -ForegroundColor Green
