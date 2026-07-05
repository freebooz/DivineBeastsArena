<#
Validates Momentum state can flow through the Arena HUD controller into the Momentum panel.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$momentumHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAMomentumPanelWidgetBase.h"
$momentumCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAMomentumPanelWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$controllerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerHeaderPath
$controllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerCppPath
$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$momentumHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $momentumHeaderPath
$momentumCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $momentumCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath

Assert-True ($controllerHeader -match "UpdateMomentum\s*\(\s*int32\s+InMomentumLevel,\s*float\s+InMomentumProgress\s*\)") "Expected Arena HUD controller Momentum update entrypoint."
Assert-True ($controllerHeader -match "GetCurrentMomentumLevel") "Expected Arena HUD controller cached MomentumLevel getter."
Assert-True ($controllerHeader -match "GetCurrentMomentumProgress") "Expected Arena HUD controller cached MomentumProgress getter."
Assert-True ($controllerHeader -match "FOnMomentumChanged") "Expected Arena HUD controller Momentum delegate."
Assert-True ($controllerHeader -match "CurrentMomentumLevel") "Expected Arena HUD controller cached MomentumLevel field."
Assert-True ($controllerHeader -match "CurrentMomentumProgress") "Expected Arena HUD controller cached MomentumProgress field."
Assert-True ($controllerCpp -match "CurrentMomentumLevel\s*=\s*FMath::Max\(0,\s*InMomentumLevel\)") "Expected controller to clamp and cache MomentumLevel."
Assert-True ($controllerCpp -match "CurrentMomentumProgress\s*=\s*FMath::Clamp\(InMomentumProgress,\s*0\.0f,\s*1\.0f\)") "Expected controller to clamp and cache MomentumProgress."
Assert-True ($controllerCpp -match "OnMomentumChanged\.Broadcast\(CurrentMomentumLevel,\s*CurrentMomentumProgress\)") "Expected controller to broadcast cached Momentum."

Assert-True ($rootHeader -match "HandleControllerMomentumUpdated") "Expected root HUD Momentum handler."
Assert-True ($rootCpp -match "UDBAMomentumPanelWidgetBase\.h") "Expected root HUD to include Momentum panel type."
Assert-True ($rootCpp -match "OnMomentumChanged\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated\)") "Expected root HUD to bind controller Momentum event."
Assert-True ($rootCpp -match "OnMomentumChanged\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated\)") "Expected root HUD to unbind controller Momentum event."
Assert-True ($rootCpp -match "HandleControllerMomentumUpdated\(WidgetController->GetCurrentMomentumLevel\(\),\s*WidgetController->GetCurrentMomentumProgress\(\)\)") "Expected root HUD to initial-sync cached Momentum."
Assert-True ($rootCpp -match "MomentumPanel->UpdateMomentumLevel\(MomentumLevel\)") "Expected root HUD to forward MomentumLevel to MomentumPanel."
Assert-True ($rootCpp -match "MomentumPanel->UpdateMomentumProgress\(MomentumProgress\)") "Expected root HUD to forward MomentumProgress to MomentumPanel."

Assert-True ($momentumHeader -match "CachedMomentumLevel") "Expected Momentum panel to cache level."
Assert-True ($momentumHeader -match "CachedMomentumProgress") "Expected Momentum panel to cache progress."
Assert-True ($momentumCpp -match "NativeConstruct[\s\S]{0,180}BP_OnMomentumUpdated\(CachedMomentumLevel,\s*CachedMomentumProgress\)") "Expected Momentum panel NativeConstruct to replay cached state after widget construction."
Assert-True ($momentumCpp -match "CachedMomentumLevel\s*=\s*FMath::Max\(0,\s*Level\)") "Expected Momentum panel to clamp and cache level."
Assert-True ($momentumCpp -match "CachedMomentumProgress\s*=\s*FMath::Clamp\(Progress,\s*0\.0f,\s*1\.0f\)") "Expected Momentum panel to clamp and cache progress."
Assert-True ($momentumCpp -match "BP_OnMomentumUpdated\(CachedMomentumLevel,\s*CachedMomentumProgress\)") "Expected Momentum panel to fire BP event with cached state."

Assert-True ($managerHeader -match "UpdateArenaHUDMomentum\s*\(\s*int32\s+MomentumLevel,\s*float\s+MomentumProgress\s*\)") "Expected UI manager Momentum entrypoint."
Assert-True ($managerCpp -match "UDBAGameUIManager::UpdateArenaHUDMomentum") "Expected UI manager Momentum implementation."
Assert-True ($managerCpp -match "Controller->UpdateMomentum\(MomentumLevel,\s*MomentumProgress\)") "Expected UI manager to push Momentum into controller."

Write-Host "PASS: Arena HUD Momentum sync contract" -ForegroundColor Green
