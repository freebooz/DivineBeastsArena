<#
Validates DBAGameUIManager main lobby hide entrypoint no-ops before touching lobby widgets in server-like runtimes.
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

$body = Get-FunctionBody "HideMainLobby"
$worldGuardIndex = $body.IndexOf("IsWorldSafeForWidgetCreation(World)")
$serverGuardIndex = $body.IndexOf("IsServerLikeRuntime(World)")
$returnIndex = $body.IndexOf("return", [Math]::Max(0, $serverGuardIndex))
$removeIndex = $body.IndexOf("MainLobbyWidget->RemoveFromParent()")
$visibilityIndex = $body.IndexOf("bMainLobbyVisible = false")
$hideLobbyHUDIndex = $body.IndexOf("HideLobbyPlayerHUD()")

Assert-True ($body.Contains("UWorld* World = GetWorld();")) "Expected HideMainLobby to cache World before runtime checks."
Assert-True ($worldGuardIndex -ge 0) "Expected HideMainLobby to verify the world is safe before touching widgets."
Assert-True ($serverGuardIndex -ge 0) "Expected HideMainLobby to no-op in server-like runtime."
Assert-True ($returnIndex -gt $serverGuardIndex) "Expected HideMainLobby runtime guard to return before touching widgets."
Assert-True ($removeIndex -gt $returnIndex) "Expected HideMainLobby to remove main lobby widgets only after runtime guards."
Assert-True ($visibilityIndex -gt $returnIndex) "Expected HideMainLobby to update visibility state only after runtime guards."
Assert-True ($hideLobbyHUDIndex -gt $returnIndex) "Expected HideMainLobby to hide lobby HUD only after runtime guards."

Write-Host "PASS: Game UI manager MainLobby hide server boundary" -ForegroundColor Green
