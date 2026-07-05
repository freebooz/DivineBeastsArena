<#
Validates DBAGameUIManager main lobby public entrypoint no-ops before touching widgets or input mode in server-like runtimes.
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

    $pattern = "void\s+UDBAGameUIManager::$FunctionName\s*\(\s*\)\s*\{"
    $match = [regex]::Match($cpp, $pattern)
    Assert-True $match.Success "Expected $FunctionName implementation."

    $nextMatch = [regex]::Match($cpp.Substring($match.Index + 1), "`nvoid\s+UDBAGameUIManager::")
    if ($nextMatch.Success) {
        $end = $match.Index + 1 + $nextMatch.Index
        return $cpp.Substring($match.Index, $end - $match.Index)
    }

    return $cpp.Substring($match.Index)
}

$body = Get-FunctionBody "ShowMainLobby"
$worldGuardIndex = $body.IndexOf("IsWorldSafeForWidgetCreation(World)")
$serverGuardIndex = $body.IndexOf("IsServerLikeRuntime(World)")
$returnIndex = $body.IndexOf("return", [Math]::Max(0, $serverGuardIndex))
$hideFlowIndex = $body.IndexOf("HideAllFlowWidgets()")
$addViewportIndex = $body.IndexOf("AddToViewport")
$inputModeIndex = $body.IndexOf("ApplyLobbyGameplayInputMode(World)")
$legacyInputModeIndex = $body.IndexOf("ApplyLobbyGameplayInputMode(GetWorld())")

Assert-True ($body.Contains("UWorld* World = GetWorld();")) "Expected ShowMainLobby to cache World before runtime checks."
Assert-True ($worldGuardIndex -ge 0) "Expected ShowMainLobby to verify the world is safe before touching widgets."
Assert-True ($serverGuardIndex -ge 0) "Expected ShowMainLobby to no-op in server-like runtime."
Assert-True ($returnIndex -gt $serverGuardIndex) "Expected ShowMainLobby runtime guard to return before touching widgets."
Assert-True ($hideFlowIndex -gt $returnIndex) "Expected ShowMainLobby to hide flow widgets only after runtime guards."
Assert-True ($addViewportIndex -gt $returnIndex) "Expected ShowMainLobby to AddToViewport only after runtime guards."
Assert-True ($inputModeIndex -gt $returnIndex) "Expected ShowMainLobby to apply lobby input mode only through the guarded World."
Assert-True ($legacyInputModeIndex -lt 0) "Expected ShowMainLobby not to call ApplyLobbyGameplayInputMode(GetWorld()) after caching guarded World."
Assert-True ($body.Contains("IsLobbyGameplayWorldForUIManager(World)")) "Expected ShowMainLobby to use the guarded World for lobby gameplay detection."

Write-Host "PASS: Game UI manager MainLobby entrypoint server boundary" -ForegroundColor Green
