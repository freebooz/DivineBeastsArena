<#
Validates DBAGameUIManager keeps Lobby Player HUD creation on client-like runtimes only.
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

Assert-True ($header -match "ShowLobbyPlayerHUD\s*\(\s*\)") "Expected UI manager to expose Lobby Player HUD show entrypoint."
Assert-True ($header -match "CreateLobbyPlayerHUDWidget\s*\(\s*\)") "Expected UI manager to expose Lobby Player HUD widget factory."
Assert-True ($cpp -match "void\s+UDBAGameUIManager::ShowLobbyPlayerHUD\s*\(\s*\)[\s\S]{0,260}IsServerLikeRuntime\(World\)[\s\S]{0,140}return") "Expected ShowLobbyPlayerHUD to no-op in server-like runtime."
Assert-True ($cpp -match "void\s+UDBAGameUIManager::CreateLobbyPlayerHUDWidget\s*\(\s*\)[\s\S]{0,260}IsServerLikeRuntime\(World\)[\s\S]{0,140}return") "Expected direct Lobby Player HUD factory calls to no-op in server-like runtime."
Assert-True ($cpp -match "ShowLobbyPlayerHUD[\s\S]{0,360}ResetLobbyHUDRefreshRetry\(\)") "Expected Lobby Player HUD success path to reset retry state."
Assert-True ($cpp -match "CreateLobbyPlayerHUDWidget[\s\S]{0,360}CreateWidget<\s*UDBALobbyPlayerHUDWidgetBase\s*>") "Expected Lobby Player HUD factory to create the native widget only after runtime guards."

Write-Host "PASS: Game UI manager Lobby HUD server boundary contract" -ForegroundColor Green
