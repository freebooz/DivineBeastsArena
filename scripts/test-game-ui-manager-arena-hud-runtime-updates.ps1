<#
Validates DBAGameUIManager exposes runtime Arena HUD update entrypoints.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$arenaControllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"

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
$arenaControllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $arenaControllerCppPath

Assert-True ($header -match "UpdateArenaHUDPlayerVitals\s*\(\s*float\s+CurrentHP,\s*float\s+MaxHP,\s*float\s+CurrentEnergy,\s*float\s+MaxEnergy\s*\)") "Expected UI manager to expose Arena HUD vitals update entrypoint."
Assert-True ($header -match "UpdateArenaHUDPlayerLevel\s*\(\s*int32\s+Level\s*\)") "Expected UI manager to expose Arena HUD level update entrypoint."
Assert-True ($header -match "EnsureArenaHUDWidgetController\s*\(\s*class\s+APlayerController\*\s+InPlayerController\s*\)") "Expected UI manager to expose an internal Arena HUD controller factory."
Assert-True ($header -match "GetArenaHUDLocalPlayerController\s*\(\s*\)\s+const") "Expected UI manager to expose an internal local-player helper for Arena HUD runtime updates."

Assert-True ($cpp -match "UDBAGameUIManager::EnsureArenaHUDWidgetController") "Expected EnsureArenaHUDWidgetController implementation."
Assert-True ($cpp -match "GetArenaHUDLocalPlayerController[\s\S]{0,220}IsWorldSafeForWidgetCreation\(World\)[\s\S]{0,80}IsServerLikeRuntime\(World\)[\s\S]{0,120}return\s+nullptr") "Expected Arena HUD runtime updates to no-op before PlayerController lookup in unsafe or server-like runtime."
Assert-True ($cpp -match "EnsureArenaHUDWidgetController[\s\S]{0,260}IsWorldSafeForWidgetCreation\(World\)[\s\S]{0,80}IsServerLikeRuntime\(World\)[\s\S]{0,160}return\s+nullptr") "Expected Arena HUD controller factory to no-op in unsafe or server-like runtime."
Assert-True ($cpp -match "EnsureArenaHUDWidgetController[\s\S]{0,280}!InPlayerController\s*\|\|\s*InPlayerController->GetWorld\(\)\s*!=\s*World[\s\S]{0,120}return\s+nullptr") "Expected Arena HUD runtime updates to no-op without a local PlayerController in the current world."
Assert-True ($cpp -match "NewObject<\s*UDBAArenaHUDWidgetController\s*>\s*\(\s*this\s*\)") "Expected controller factory to create Arena HUD controller with UI manager as outer."
Assert-True ($cpp -match "ArenaHUDWidgetController->InitializeController\(InPlayerController\)") "Expected controller factory to initialize with provided player controller."
Assert-True ($cpp -match "UDBAGameUIManager::UpdateArenaHUDPlayerVitals") "Expected Arena HUD vitals update implementation."
Assert-True ($cpp -match "Controller->UpdatePlayerHP\(CurrentHP,\s*MaxHP\)") "Expected vitals update to push HP into Arena HUD controller."
Assert-True ($cpp -match "Controller->UpdatePlayerEnergy\(CurrentEnergy,\s*MaxEnergy\)") "Expected vitals update to push Energy into Arena HUD controller."
Assert-True ($cpp -match "UDBAGameUIManager::UpdateArenaHUDPlayerLevel") "Expected Arena HUD level update implementation."
Assert-True ($cpp -match "Controller->UpdatePlayerLevel\(Level\)") "Expected level update to push Level into Arena HUD controller."
Assert-True ($cpp -match "ArenaHUDWidget->SetWidgetController\(ArenaHUDWidgetController\)") "Expected runtime update path to keep the HUD widget bound to its controller."
Assert-True ($arenaControllerCpp -match "UpdatePlayerHP[\s\S]*CurrentHP\s*=\s*FMath::Max\(0\.0f,\s*InCurrentHP\)[\s\S]*MaxHP\s*=\s*FMath::Max\(0\.0f,\s*InMaxHP\)[\s\S]*OnPlayerHPChanged\.Broadcast\(CurrentHP,\s*MaxHP\)") "Expected Arena HUD controller to normalize HP before caching and broadcasting."
Assert-True ($arenaControllerCpp -match "UpdatePlayerEnergy[\s\S]*CurrentEnergy\s*=\s*FMath::Max\(0\.0f,\s*InCurrentEnergy\)[\s\S]*MaxEnergy\s*=\s*FMath::Max\(0\.0f,\s*InMaxEnergy\)[\s\S]*OnPlayerEnergyChanged\.Broadcast\(CurrentEnergy,\s*MaxEnergy\)") "Expected Arena HUD controller to normalize Energy before caching and broadcasting."

Write-Host "PASS: Game UI manager Arena HUD runtime updates contract" -ForegroundColor Green
