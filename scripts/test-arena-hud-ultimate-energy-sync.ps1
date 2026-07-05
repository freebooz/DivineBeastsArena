<#
Validates UltimateEnergy flows from ASC/character into Arena HUD controller and PlayerUnitFrame.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$ascHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$characterHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$ascHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascHeaderPath
$ascCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascCppPath
$controllerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerHeaderPath
$controllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerCppPath
$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$characterHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterHeaderPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath

Assert-True ($ascHeader -match "FOnUltimateEnergyChanged") "Expected ASC UltimateEnergy change delegate."
Assert-True ($ascHeader -match "ReplicatedUsing\s*=\s*OnRep_UltimateEnergy") "Expected ASC UltimateEnergy to use OnRep."
Assert-True ($ascHeader -match "OnUltimateEnergyChanged") "Expected ASC to expose UltimateEnergy change event."
Assert-True ($ascCpp -match "BroadcastUltimateEnergyChanged\(\)") "Expected ASC to centralize UltimateEnergy broadcasts."
Assert-True ($ascCpp -match "OnRep_UltimateEnergy[\s\S]*BroadcastUltimateEnergyChanged\(\)") "Expected ASC OnRep to broadcast UltimateEnergy."
Assert-True ($ascCpp -match "AddUltimateEnergy[\s\S]*BroadcastUltimateEnergyChanged\(\)") "Expected ASC AddUltimateEnergy to broadcast changed value."
Assert-True ($ascCpp -match "ConsumeUltimateEnergy[\s\S]*BroadcastUltimateEnergyChanged\(\)") "Expected ASC ConsumeUltimateEnergy to broadcast changed value."

Assert-True ($controllerHeader -match "GetCurrentUltimateEnergy") "Expected Arena HUD controller to expose current UltimateEnergy."
Assert-True ($controllerHeader -match "GetMaxUltimateEnergy") "Expected Arena HUD controller to expose max UltimateEnergy."
Assert-True ($controllerHeader -match "CurrentUltimateEnergy") "Expected Arena HUD controller to cache current UltimateEnergy."
Assert-True ($controllerHeader -match "MaxUltimateEnergy") "Expected Arena HUD controller to cache max UltimateEnergy."
Assert-True ($controllerCpp -match "MaxUltimateEnergy\s*=\s*FMath::Max\(1\.0f,\s*InMaxEnergy\)[\s\S]*CurrentUltimateEnergy\s*=\s*FMath::Clamp\(InCurrentEnergy,\s*0\.0f,\s*MaxUltimateEnergy\)") "Expected controller to clamp UltimateEnergy against the cached max before broadcasting."
Assert-True ($controllerCpp -match "OnUltimateEnergyChanged\.Broadcast\(CurrentUltimateEnergy,\s*MaxUltimateEnergy\)") "Expected controller to broadcast cached UltimateEnergy."

Assert-True ($rootHeader -match "HandleControllerUltimateEnergyUpdated") "Expected root HUD to handle controller UltimateEnergy updates."
Assert-True ($rootCpp -match "OnUltimateEnergyChanged\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated\)") "Expected root HUD to bind controller UltimateEnergy event."
Assert-True ($rootCpp -match "OnUltimateEnergyChanged\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated\)") "Expected root HUD to unbind controller UltimateEnergy event."
Assert-True ($rootCpp -match "HandleControllerUltimateEnergyUpdated\(WidgetController->GetCurrentUltimateEnergy\(\),\s*WidgetController->GetMaxUltimateEnergy\(\)\)") "Expected root HUD to initial-sync cached UltimateEnergy."
Assert-True ($rootCpp -match "PlayerUnitFrame->UpdateUltimateEnergyWithMax\(CurrentEnergy,\s*MaxEnergy\)") "Expected root HUD to forward current and max UltimateEnergy to PlayerUnitFrame."

Assert-True ($managerHeader -match "UpdateArenaHUDUltimateEnergy\s*\(\s*float\s+CurrentEnergy,\s*float\s+MaxEnergy\s*\)") "Expected UI manager UltimateEnergy entrypoint."
Assert-True ($managerCpp -match "UDBAGameUIManager::UpdateArenaHUDUltimateEnergy") "Expected UI manager UltimateEnergy implementation."
Assert-True ($managerCpp -match "Controller->UpdateUltimateEnergy\(CurrentEnergy,\s*MaxEnergy\)") "Expected UI manager to push UltimateEnergy into Arena HUD controller."

Assert-True ($characterHeader -match "GetUltimateEnergy\s*\(\s*\)\s*const") "Expected character UltimateEnergy getter declaration."
Assert-True ($characterHeader -match "HandleArenaHUDUltimateEnergyChanged") "Expected character to handle ASC UltimateEnergy changes."
Assert-True ($characterHeader -match "LastSyncedArenaHUDUltimateEnergy") "Expected character to cache synced UltimateEnergy."
Assert-True ($characterCpp -match "return ASC->GetUltimateEnergy\(\)") "Expected character getter to prefer ASC UltimateEnergy."
Assert-True ($characterCpp -match "UpdateArenaHUDUltimateEnergy\(CurrentUltimateEnergy,\s*MaxUltimateEnergy\)") "Expected character HUD sync to push UltimateEnergy."
Assert-True ($characterCpp -match "OnUltimateEnergyChanged\.AddDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged\)") "Expected character to bind ASC UltimateEnergy event."
Assert-True ($characterCpp -match "OnUltimateEnergyChanged\.RemoveDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDUltimateEnergyChanged\)") "Expected character to unbind ASC UltimateEnergy event."

Write-Host "PASS: Arena HUD UltimateEnergy sync contract" -ForegroundColor Green
