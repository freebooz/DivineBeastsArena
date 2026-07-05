<#
Validates ConnectionWarning native-to-Blueprint update behavior.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAConnectionWarningWidgetBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAConnectionWarningWidgetBase.cpp"

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

Assert-True ($header -match "ShowWarning\s*\(\s*const\s+FText&\s+Message\s*\)") "Expected ConnectionWarning to expose ShowWarning."
Assert-True ($header -match "HideWarning\s*\(\s*\)") "Expected ConnectionWarning to expose HideWarning."
Assert-True ($header -match "BP_OnWarningShown\s*\(\s*const\s+FText&\s+Message\s*\)") "Expected ConnectionWarning to declare warning shown BP event."
Assert-True ($header -match "BP_OnWarningHidden\s*\(\s*\)") "Expected ConnectionWarning to declare warning hidden BP event."
Assert-True ($header -match "CachedWarningMessage") "Expected ConnectionWarning to cache warning message."
Assert-True ($header -match "bCachedWarningVisible") "Expected ConnectionWarning to cache warning visibility."

Assert-True ($cpp -match "NormalizeConnectionWarningText") "Expected ConnectionWarning implementation to normalize warning text."
Assert-True ($cpp -match "TrimStartAndEnd\(\)") "Expected ConnectionWarning to trim warning text."
Assert-True ($cpp -match "NormalizedText\.IsEmpty\(\)") "Expected ConnectionWarning to ignore blank warning text."
Assert-True ($cpp -match "NativeConstruct[\s\S]{0,260}if\s*\(\s*bCachedWarningVisible\s*&&\s*!CachedWarningMessage\.IsEmpty\(\)\s*\)[\s\S]{0,180}BP_OnWarningShown\(CachedWarningMessage\)") "Expected NativeConstruct to replay visible cached warning after widget construction."
Assert-True ($cpp -match "NativeConstruct[\s\S]{0,440}else[\s\S]{0,160}BP_OnWarningHidden\(\)") "Expected NativeConstruct to replay hidden warning state after widget construction."
Assert-True ($cpp -match "ShowWarning[\s\S]*FText\s+NormalizedMessage[\s\S]*if\s*\(\s*!NormalizeConnectionWarningText\(Message,\s*NormalizedMessage\)\s*\)[\s\S]*return;") "Expected ShowWarning to ignore blank warning text."
Assert-True ($cpp -match "CachedWarningMessage\s*=\s*NormalizedMessage") "Expected ShowWarning to cache normalized warning text."
Assert-True ($cpp -match "bCachedWarningVisible\s*=\s*true") "Expected ShowWarning to cache visible state."
Assert-True ($cpp -match "BP_OnWarningShown\(CachedWarningMessage\)") "Expected ShowWarning to broadcast cached normalized warning text."
Assert-True ($cpp -match "CachedWarningMessage\s*=\s*FText::GetEmpty\(\)") "Expected HideWarning to reset cached warning text."
Assert-True ($cpp -match "bCachedWarningVisible\s*=\s*false") "Expected HideWarning to cache hidden state."
Assert-True ($cpp -match "HideWarning[\s\S]*BP_OnWarningHidden\(\)") "Expected HideWarning to broadcast warning hidden event."

Write-Host "PASS: Arena HUD ConnectionWarning contract" -ForegroundColor Green
