<#
Validates DBAGameUIManager creates and injects the Arena HUD WidgetController.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"

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

Assert-True ($header -match "class\s+UDBAArenaHUDWidgetController;") "Expected DBAGameUIManager to forward declare Arena HUD controller."
Assert-True ($header -match "TObjectPtr<\s*UDBAArenaHUDWidgetController\s*>\s+ArenaHUDWidgetController") "Expected DBAGameUIManager to retain Arena HUD controller."

Assert-True ($cpp.Contains('#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"')) "Expected DBAGameUIManager implementation to include Arena HUD controller header."
Assert-True ($cpp -match "ShowArenaHUD[\s\S]{0,220}IsWorldSafeForWidgetCreation\(World\)[\s\S]{0,80}IsServerLikeRuntime\(World\)[\s\S]{0,120}return") "Expected ShowArenaHUD to no-op in unsafe or server-like runtime before creating widgets."
Assert-True ($cpp -match "void\s+UDBAGameUIManager::CreateArenaHUDWidget\s*\(\s*\)[\s\S]{0,260}IsServerLikeRuntime\(World\)[\s\S]{0,140}return") "Expected direct Arena HUD widget factory calls to no-op in server-like runtime."
Assert-True ($cpp -match "NewObject<\s*UDBAArenaHUDWidgetController\s*>\s*\(\s*this\s*\)") "Expected CreateArenaHUDWidget to create Arena HUD controller with UI manager as outer."
Assert-True ($cpp -match "EnsureArenaHUDWidgetController\(PC\)") "Expected Arena HUD widget creation to initialize through the shared controller factory."
Assert-True ($cpp -match "EnsureArenaHUDWidgetController[\s\S]{0,260}IsWorldSafeForWidgetCreation\(World\)[\s\S]{0,80}IsServerLikeRuntime\(World\)[\s\S]{0,160}return\s+nullptr") "Expected Arena HUD controller factory to no-op in unsafe or server-like runtime."
Assert-True ($cpp -match "EnsureArenaHUDWidgetController[\s\S]{0,280}!InPlayerController\s*\|\|\s*InPlayerController->GetWorld\(\)\s*!=\s*World[\s\S]{0,120}return\s+nullptr") "Expected Arena HUD controller factory to no-op without a local PlayerController in the current world."
Assert-True ($cpp -match "ArenaHUDWidgetController->InitializeController\(InPlayerController\)") "Expected Arena HUD controller factory to initialize with the provided player controller."
Assert-True ($cpp -match "ArenaHUDWidget->SetWidgetController\(ArenaHUDWidgetController\)") "Expected Arena HUD widget to receive Arena HUD controller."
Assert-True ($cpp -match "if\s*\(\s*!ArenaHUDWidgetController\s*\)") "Expected UI manager to reuse existing Arena HUD controller instead of recreating it every show."

Write-Host "PASS: Game UI manager Arena HUD controller contract" -ForegroundColor Green
