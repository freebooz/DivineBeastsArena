<#
Validates ChainLevel and ResonanceLevel flow from ASC/character into Arena HUD panels.
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
$chainPanelCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAChainUltimatePanelWidgetBase.cpp"
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
$chainPanelCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $chainPanelCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$characterHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterHeaderPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath

Assert-True ($ascHeader -match "FOnChainLevelChanged") "Expected ASC ChainLevel delegate."
Assert-True ($ascHeader -match "FOnResonanceLevelChanged") "Expected ASC ResonanceLevel delegate."
Assert-True ($ascHeader -match "ReplicatedUsing\s*=\s*OnRep_ChainLevel") "Expected ASC ChainLevel to use OnRep."
Assert-True ($ascHeader -match "ReplicatedUsing\s*=\s*OnRep_ResonanceLevel") "Expected ASC ResonanceLevel to use OnRep."
Assert-True ($ascCpp -match "OnRep_ChainLevel[\s\S]*BroadcastChainLevelChanged\(\)") "Expected ASC ChainLevel OnRep to broadcast."
Assert-True ($ascCpp -match "OnRep_ResonanceLevel[\s\S]*BroadcastResonanceLevelChanged\(\)") "Expected ASC ResonanceLevel OnRep to broadcast."
Assert-True ($ascCpp -match "AddChainLevel[\s\S]*BroadcastChainLevelChanged\(\)") "Expected ASC AddChainLevel to broadcast changed value."
Assert-True ($ascCpp -match "ResetChainLevel[\s\S]*BroadcastChainLevelChanged\(\)") "Expected ASC ResetChainLevel to broadcast changed value."
Assert-True ($ascCpp -match "SetResonanceLevel[\s\S]*BroadcastResonanceLevelChanged\(\)") "Expected ASC SetResonanceLevel to broadcast changed value."

Assert-True ($controllerHeader -match "UpdateChainLevel") "Expected Arena HUD controller ChainLevel update entrypoint."
Assert-True ($controllerHeader -match "UpdateResonanceLevel") "Expected Arena HUD controller ResonanceLevel update entrypoint."
Assert-True ($controllerHeader -match "GetCurrentChainLevel") "Expected Arena HUD controller cached ChainLevel getter."
Assert-True ($controllerHeader -match "GetCurrentResonanceLevel") "Expected Arena HUD controller cached ResonanceLevel getter."
Assert-True ($controllerCpp -match "CurrentChainLevel\s*=\s*FMath::Clamp\(InChainLevel") "Expected controller to clamp and cache ChainLevel."
Assert-True ($controllerCpp -match "CurrentResonanceLevel\s*=\s*FMath::Clamp\(InResonanceLevel") "Expected controller to clamp and cache ResonanceLevel."
Assert-True ($controllerCpp -match "OnChainLevelChanged\.Broadcast\(CurrentChainLevel\)") "Expected controller to broadcast cached ChainLevel."
Assert-True ($controllerCpp -match "OnResonanceLevelChanged\.Broadcast\(CurrentResonanceLevel\)") "Expected controller to broadcast cached ResonanceLevel."

Assert-True ($rootHeader -match "HandleControllerChainLevelUpdated") "Expected root HUD ChainLevel handler."
Assert-True ($rootHeader -match "HandleControllerResonanceLevelUpdated") "Expected root HUD ResonanceLevel handler."
Assert-True ($rootCpp -match "OnChainLevelChanged\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerChainLevelUpdated\)") "Expected root HUD to bind controller ChainLevel event."
Assert-True ($rootCpp -match "OnResonanceLevelChanged\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerResonanceLevelUpdated\)") "Expected root HUD to bind controller ResonanceLevel event."
Assert-True ($rootCpp -match "HandleControllerChainLevelUpdated\(WidgetController->GetCurrentChainLevel\(\)\)") "Expected root HUD to initial-sync cached ChainLevel."
Assert-True ($rootCpp -match "HandleControllerResonanceLevelUpdated\(WidgetController->GetCurrentResonanceLevel\(\)\)") "Expected root HUD to initial-sync cached ResonanceLevel."
Assert-True ($rootCpp -match "ChainUltimatePanel->UpdateChainCount\(ChainLevel\)") "Expected root HUD to forward ChainLevel to chain panel."
Assert-True ($rootCpp -match "PassiveAndResonancePanel->UpdateResonanceLevel\(ResonanceLevel\)") "Expected root HUD to forward ResonanceLevel to passive/resonance panel."
Assert-True ($chainPanelCpp -match "CachedChainCount\s*=\s*NormalizedChainCount") "Expected chain panel to cache normalized ChainCount."
Assert-True ($chainPanelCpp -match "BP_OnChainCountUpdated\(CachedChainCount\)") "Expected chain panel to fire ChainCount BP event with cached normalized count."
Assert-True ($chainPanelCpp -match "BP_OnChainReady\(\)") "Expected chain panel to fire ChainReady BP event."

Assert-True ($managerHeader -match "UpdateArenaHUDCombatState\s*\(\s*int32\s+ChainLevel,\s*int32\s+ResonanceLevel\s*\)") "Expected UI manager combat-state entrypoint."
Assert-True ($managerCpp -match "Controller->UpdateChainLevel\(ChainLevel\)") "Expected UI manager to push ChainLevel into controller."
Assert-True ($managerCpp -match "Controller->UpdateResonanceLevel\(ResonanceLevel\)") "Expected UI manager to push ResonanceLevel into controller."

Assert-True ($characterHeader -match "int32\s+GetChainLevel\s*\(\s*\)\s*const;") "Expected character ChainLevel getter declaration."
Assert-True ($characterHeader -match "int32\s+GetResonanceLevel\s*\(\s*\)\s*const;") "Expected character ResonanceLevel getter declaration."
Assert-True ($characterHeader -match "HandleArenaHUDChainLevelChanged") "Expected character ChainLevel event handler."
Assert-True ($characterHeader -match "HandleArenaHUDResonanceLevelChanged") "Expected character ResonanceLevel event handler."
Assert-True ($characterHeader -match "LastSyncedArenaHUDChainLevel") "Expected character to cache synced ChainLevel."
Assert-True ($characterHeader -match "LastSyncedArenaHUDResonanceLevel") "Expected character to cache synced ResonanceLevel."
Assert-True ($characterCpp -match "return ASC->GetChainLevel\(\)") "Expected character getter to prefer ASC ChainLevel."
Assert-True ($characterCpp -match "return ASC->GetResonanceLevel\(\)") "Expected character getter to prefer ASC ResonanceLevel."
Assert-True ($characterCpp -match "UpdateArenaHUDCombatState\(CurrentChainLevel,\s*CurrentResonanceLevel\)") "Expected character HUD sync to push combat state."
Assert-True ($characterCpp -match "OnChainLevelChanged\.AddDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged\)") "Expected character to bind ASC ChainLevel event."
Assert-True ($characterCpp -match "OnResonanceLevelChanged\.AddDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged\)") "Expected character to bind ASC ResonanceLevel event."
Assert-True ($characterCpp -match "OnChainLevelChanged\.RemoveDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDChainLevelChanged\)") "Expected character to unbind ASC ChainLevel event."
Assert-True ($characterCpp -match "OnResonanceLevelChanged\.RemoveDynamic\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDResonanceLevelChanged\)") "Expected character to unbind ASC ResonanceLevel event."

Write-Host "PASS: Arena HUD Chain/Resonance sync contract" -ForegroundColor Green
