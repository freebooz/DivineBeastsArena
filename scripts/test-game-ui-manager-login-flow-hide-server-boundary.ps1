<#
Validates DBAGameUIManager login flow hide entrypoint no-ops before touching flow widgets in server-like runtimes.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-FunctionBody {
    param([Parameter(Mandatory = $true)][string]$FunctionName)

    $pattern = "void\s+UDBAGameUIManager::$FunctionName\s*\([^)]*\)\s*\{"
    $match = [regex]::Match($cpp, $pattern)
    Assert-True $match.Success "Expected $FunctionName implementation."

    $nextMatch = [regex]::Match($cpp.Substring($match.Index + 1), "`nvoid\s+UDBAGameUIManager::")
    if ($nextMatch.Success) {
        $end = $match.Index + 1 + $nextMatch.Index
        return $cpp.Substring($match.Index, $end - $match.Index)
    }

    return $cpp.Substring($match.Index)
}

$body = Get-FunctionBody "HideLoginFlowWidget"
$worldGuardIndex = $body.IndexOf("IsWorldSafeForWidgetCreation(World)")
$serverGuardIndex = $body.IndexOf("IsServerLikeRuntime(World)")
$returnIndex = $body.IndexOf("return", [Math]::Max(0, $serverGuardIndex))
$hideAllIndex = $body.IndexOf("HideAllFlowWidgets()")

Assert-True ($body.Contains("UWorld* World = GetWorld();")) "Expected HideLoginFlowWidget to cache World before runtime checks."
Assert-True ($worldGuardIndex -ge 0) "Expected HideLoginFlowWidget to verify the world is safe before touching widgets."
Assert-True ($serverGuardIndex -ge 0) "Expected HideLoginFlowWidget to no-op in server-like runtime."
Assert-True ($returnIndex -gt $serverGuardIndex) "Expected HideLoginFlowWidget runtime guard to return before touching widgets."
Assert-True ($hideAllIndex -gt $returnIndex) "Expected HideLoginFlowWidget to hide flow widgets only after runtime guards."

Write-Host "PASS: Game UI manager login flow hide server boundary" -ForegroundColor Green
