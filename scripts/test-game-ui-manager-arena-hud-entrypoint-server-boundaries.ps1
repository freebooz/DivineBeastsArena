<#
Validates DBAGameUIManager Arena HUD public entrypoints no-op before touching HUD controllers or widgets in server-like runtimes.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath
$header = Get-Content -Raw -Encoding UTF8 -LiteralPath $headerPath

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
    param(
        [Parameter(Mandatory = $true)][string]$FunctionName,
        [Parameter(Mandatory = $true)][string]$SignaturePattern
    )

    $pattern = "$SignaturePattern\s*\{"
    $match = [regex]::Match($cpp, $pattern)
    Assert-True $match.Success "Expected $FunctionName implementation."

    $nextMatch = [regex]::Match($cpp.Substring($match.Index + 1), "`n(?:void|UDBAArenaHUDWidgetController\*|APlayerController\*)\s+UDBAGameUIManager::")
    if ($nextMatch.Success) {
        $end = $match.Index + 1 + $nextMatch.Index
        return $cpp.Substring($match.Index, $end - $match.Index)
    }

    return $cpp.Substring($match.Index)
}

Assert-True ($header -match "APlayerController\*\s+GetArenaHUDLocalPlayerController\s*\(\s*\)\s+const\s*;") `
    "Expected DBAGameUIManager to declare a shared local-player helper for Arena HUD entrypoints."

$helperBody = Get-FunctionBody "GetArenaHUDLocalPlayerController" "APlayerController\*\s+UDBAGameUIManager::GetArenaHUDLocalPlayerController\s*\(\s*\)\s+const"
Assert-True ($helperBody.Contains("IsWorldSafeForWidgetCreation(World)")) "Expected Arena HUD local-player helper to verify the world is safe."
Assert-True ($helperBody.Contains("IsServerLikeRuntime(World)")) "Expected Arena HUD local-player helper to no-op in server-like runtime."
Assert-True ($helperBody.Contains("World->GetFirstPlayerController()")) "Expected Arena HUD local-player helper to own PlayerController lookup."

$showBody = Get-FunctionBody "ShowArenaHUD" "void\s+UDBAGameUIManager::ShowArenaHUD\s*\(\s*\)"
$showWorldGuardIndex = $showBody.IndexOf("IsWorldSafeForWidgetCreation(World)")
$showServerGuardIndex = $showBody.IndexOf("IsServerLikeRuntime(World)")
$showReturnIndex = $showBody.IndexOf("return", [Math]::Max(0, $showServerGuardIndex))
$showViewportIndex = $showBody.IndexOf("AddToViewport")
Assert-True ($showWorldGuardIndex -ge 0) "Expected ShowArenaHUD to verify the world is safe before AddToViewport."
Assert-True ($showServerGuardIndex -ge 0) "Expected ShowArenaHUD to no-op in server-like runtime before AddToViewport."
Assert-True ($showReturnIndex -gt $showServerGuardIndex) "Expected ShowArenaHUD runtime guard to return before AddToViewport."
Assert-True ($showViewportIndex -gt $showReturnIndex) "Expected ShowArenaHUD to AddToViewport only after runtime guards."

$controllerEntrypoints = @(
    "UpdateArenaHUDPlayerVitals",
    "UpdateArenaHUDPlayerLevel",
    "UpdateArenaHUDUltimateEnergy",
    "UpdateArenaHUDCombatState",
    "UpdateArenaHUDMomentum",
    "AddArenaHUDBuff",
    "RemoveArenaHUDBuff",
    "ClearArenaHUDBuffs",
    "AddArenaHUDDebuff",
    "RemoveArenaHUDDebuff",
    "ClearArenaHUDDebuffs",
    "AddArenaHUDCCEffect",
    "RemoveArenaHUDCCEffect",
    "ClearArenaHUDCCEffects",
    "ShowArenaHUDCombatAnnouncement",
    "ClearArenaHUDCombatAnnouncement",
    "UpdateArenaHUDCriticalStateHints",
    "UpdateArenaHUDObjective",
    "CompleteArenaHUDObjective",
    "AddArenaHUDEventFeedEntry",
    "ClearArenaHUDEventFeed",
    "ShowArenaHUDUltimateReadyPrompt",
    "HideArenaHUDUltimateReadyPrompt"
)

foreach ($functionName in $controllerEntrypoints) {
    $body = Get-FunctionBody $functionName "void\s+UDBAGameUIManager::$functionName\s*\([^)]*\)"
    $helperIndex = $body.IndexOf("GetArenaHUDLocalPlayerController()")
    $ensureIndex = $body.IndexOf("EnsureArenaHUDWidgetController(PC)")
    Assert-True ($helperIndex -ge 0) "Expected $functionName to obtain PC through GetArenaHUDLocalPlayerController()."
    Assert-True ($ensureIndex -gt $helperIndex) "Expected $functionName to call EnsureArenaHUDWidgetController only after local-player helper."
    Assert-True (-not $body.Contains("GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr")) `
        "Expected $functionName not to perform direct unguarded PlayerController lookup."
}

$bindBody = Get-FunctionBody "BindArenaHUDToCharacter" "void\s+UDBAGameUIManager::BindArenaHUDToCharacter\s*\(\s*ADBAZodiacCharacterBase\*\s+Character\s*\)"
$bindHelperIndex = $bindBody.IndexOf("GetArenaHUDLocalPlayerController()")
$bindCacheIndex = $bindBody.IndexOf("ArenaHUDCharacter = Character")
$bindWidgetIndex = $bindBody.IndexOf("ArenaHUDWidget->BindArenaHUDToCharacter(Character)")
Assert-True ($bindHelperIndex -ge 0) "Expected BindArenaHUDToCharacter to guard through GetArenaHUDLocalPlayerController()."
Assert-True ($bindCacheIndex -gt $bindHelperIndex) "Expected BindArenaHUDToCharacter to cache only after local client guard."
Assert-True ($bindWidgetIndex -gt $bindHelperIndex) "Expected BindArenaHUDToCharacter to touch widgets only after local client guard."

Write-Host "PASS: Game UI manager Arena HUD entrypoint server boundaries" -ForegroundColor Green
