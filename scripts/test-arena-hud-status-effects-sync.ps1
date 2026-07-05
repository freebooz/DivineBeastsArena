<#
Validates Buff, Debuff, and CC status-effect HUD events can flow through the Arena HUD controller into the matching panels.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$buffHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBABuffBarWidgetBase.h"
$buffCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBABuffBarWidgetBase.cpp"
$debuffHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBADebuffBarWidgetBase.h"
$debuffCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBADebuffBarWidgetBase.cpp"
$ccHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBACCBarWidgetBase.h"
$ccCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBACCBarWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$statusEffectsTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDStatusEffectsTests.cpp"

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
$buffHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $buffHeaderPath
$buffCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $buffCppPath
$debuffHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $debuffHeaderPath
$debuffCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $debuffCppPath
$ccHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $ccHeaderPath
$ccCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $ccCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$statusEffectsTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $statusEffectsTestPath

Assert-True ($controllerHeader -match "AddStatusBuff\s*\(\s*const FString&\s+BuffId,\s*float\s+Duration\s*\)") "Expected controller AddStatusBuff entrypoint."
Assert-True ($controllerHeader -match "RemoveStatusBuff\s*\(\s*const FString&\s+BuffId\s*\)") "Expected controller RemoveStatusBuff entrypoint."
Assert-True ($controllerHeader -match "ClearStatusBuffs") "Expected controller ClearStatusBuffs entrypoint."
Assert-True ($controllerHeader -match "AddStatusDebuff\s*\(\s*const FString&\s+DebuffId,\s*float\s+Duration\s*\)") "Expected controller AddStatusDebuff entrypoint."
Assert-True ($controllerHeader -match "RemoveStatusDebuff\s*\(\s*const FString&\s+DebuffId\s*\)") "Expected controller RemoveStatusDebuff entrypoint."
Assert-True ($controllerHeader -match "ClearStatusDebuffs") "Expected controller ClearStatusDebuffs entrypoint."
Assert-True ($controllerHeader -match "AddStatusCCEffect\s*\(\s*const FString&\s+CCId,\s*float\s+Duration\s*\)") "Expected controller AddStatusCCEffect entrypoint."
Assert-True ($controllerHeader -match "RemoveStatusCCEffect\s*\(\s*const FString&\s+CCId\s*\)") "Expected controller RemoveStatusCCEffect entrypoint."
Assert-True ($controllerHeader -match "ClearStatusCCEffects") "Expected controller ClearStatusCCEffects entrypoint."
Assert-True ($controllerHeader -match "FOnStatusEffectAdded") "Expected controller status-effect add delegate."
Assert-True ($controllerHeader -match "FOnStatusEffectRemoved") "Expected controller status-effect remove delegate."
Assert-True ($controllerHeader -match "FOnStatusEffectCleared") "Expected controller status-effect clear delegate."
Assert-True ($controllerHeader -match "FDBAArenaStatusEffectEntry") "Expected controller cached status-effect entry type."
Assert-True ($controllerHeader -match "GetActiveStatusBuffs") "Expected controller active Buff cache getter."
Assert-True ($controllerHeader -match "GetActiveStatusDebuffs") "Expected controller active Debuff cache getter."
Assert-True ($controllerHeader -match "GetActiveStatusCCEffects") "Expected controller active CC cache getter."
Assert-True ($controllerCpp -match "NormalizeStatusEffectId[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected controller to trim and reject empty status-effect ids."
Assert-True ($controllerCpp -match "AddStatusBuff[\s\S]*FString\s+NormalizedBuffId[\s\S]*if\s*\(\s*!UpsertStatusEffect\(ActiveStatusBuffs,\s*BuffId,\s*Duration,\s*NormalizedBuffId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid Buff adds."
Assert-True ($controllerCpp -match "RemoveStatusBuff[\s\S]*FString\s+NormalizedBuffId[\s\S]*if\s*\(\s*!RemoveStatusEffect\(ActiveStatusBuffs,\s*BuffId,\s*NormalizedBuffId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid Buff removals."
Assert-True ($controllerCpp -match "ActiveStatusBuffs\.Reset\(\)") "Expected controller to reset active Buff cache."
Assert-True ($controllerCpp -match "AddStatusDebuff[\s\S]*FString\s+NormalizedDebuffId[\s\S]*if\s*\(\s*!UpsertStatusEffect\(ActiveStatusDebuffs,\s*DebuffId,\s*Duration,\s*NormalizedDebuffId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid Debuff adds."
Assert-True ($controllerCpp -match "RemoveStatusDebuff[\s\S]*FString\s+NormalizedDebuffId[\s\S]*if\s*\(\s*!RemoveStatusEffect\(ActiveStatusDebuffs,\s*DebuffId,\s*NormalizedDebuffId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid Debuff removals."
Assert-True ($controllerCpp -match "ActiveStatusDebuffs\.Reset\(\)") "Expected controller to reset active Debuff cache."
Assert-True ($controllerCpp -match "AddStatusCCEffect[\s\S]*FString\s+NormalizedCCId[\s\S]*if\s*\(\s*!UpsertStatusEffect\(ActiveStatusCCEffects,\s*CCId,\s*Duration,\s*NormalizedCCId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid CC adds."
Assert-True ($controllerCpp -match "RemoveStatusCCEffect[\s\S]*FString\s+NormalizedCCId[\s\S]*if\s*\(\s*!RemoveStatusEffect\(ActiveStatusCCEffects,\s*CCId,\s*NormalizedCCId\)\s*\)[\s\S]*return;") "Expected controller to no-op invalid CC removals."
Assert-True ($controllerCpp -match "ActiveStatusCCEffects\.Reset\(\)") "Expected controller to reset active CC cache."
Assert-True ($controllerCpp -match "OnStatusBuffAdded\.Broadcast\(NormalizedBuffId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected controller to broadcast normalized Buff add."
Assert-True ($controllerCpp -match "OnStatusBuffRemoved\.Broadcast\(NormalizedBuffId\)") "Expected controller to broadcast normalized Buff remove."
Assert-True ($controllerCpp -match "OnStatusBuffsCleared\.Broadcast\(\)") "Expected controller to broadcast Buff clear."
Assert-True ($controllerCpp -match "OnStatusDebuffAdded\.Broadcast\(NormalizedDebuffId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected controller to broadcast normalized Debuff add."
Assert-True ($controllerCpp -match "OnStatusDebuffRemoved\.Broadcast\(NormalizedDebuffId\)") "Expected controller to broadcast normalized Debuff remove."
Assert-True ($controllerCpp -match "OnStatusDebuffsCleared\.Broadcast\(\)") "Expected controller to broadcast Debuff clear."
Assert-True ($controllerCpp -match "OnStatusCCEffectAdded\.Broadcast\(NormalizedCCId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected controller to broadcast normalized CC add."
Assert-True ($controllerCpp -match "OnStatusCCEffectRemoved\.Broadcast\(NormalizedCCId\)") "Expected controller to broadcast normalized CC remove."
Assert-True ($controllerCpp -match "OnStatusCCEffectsCleared\.Broadcast\(\)") "Expected controller to broadcast CC clear."

Assert-True ($rootHeader -match "HandleControllerStatusBuffAdded") "Expected root HUD Buff add handler."
Assert-True ($rootHeader -match "HandleControllerStatusBuffRemoved") "Expected root HUD Buff remove handler."
Assert-True ($rootHeader -match "HandleControllerStatusBuffsCleared") "Expected root HUD Buff clear handler."
Assert-True ($rootHeader -match "HandleControllerStatusDebuffAdded") "Expected root HUD Debuff add handler."
Assert-True ($rootHeader -match "HandleControllerStatusCCEffectAdded") "Expected root HUD CC add handler."
Assert-True ($rootCpp -match "UDBABuffBarWidgetBase\.h") "Expected root HUD to include BuffBar type."
Assert-True ($rootCpp -match "UDBADebuffBarWidgetBase\.h") "Expected root HUD to include DebuffBar type."
Assert-True ($rootCpp -match "UDBACCBarWidgetBase\.h") "Expected root HUD to include CCBar type."
Assert-True ($rootCpp -match "OnStatusBuffAdded\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded\)") "Expected root HUD to bind Buff add."
Assert-True ($rootCpp -match "OnStatusBuffAdded\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded\)") "Expected root HUD to unbind Buff add."
Assert-True ($rootCpp -match "GetActiveStatusBuffs\(\)") "Expected root HUD to inspect cached active Buffs after binding."
Assert-True ($rootCpp -match "GetActiveStatusDebuffs\(\)") "Expected root HUD to inspect cached active Debuffs after binding."
Assert-True ($rootCpp -match "GetActiveStatusCCEffects\(\)") "Expected root HUD to inspect cached active CC effects after binding."
Assert-True ($rootCpp -match 'for \(const FDBAArenaStatusEffectEntry& ActiveBuff') "Expected root HUD to replay cached active Buffs."
Assert-True ($rootCpp -match 'for \(const FDBAArenaStatusEffectEntry& ActiveDebuff') "Expected root HUD to replay cached active Debuffs."
Assert-True ($rootCpp -match 'for \(const FDBAArenaStatusEffectEntry& ActiveCCEffect') "Expected root HUD to replay cached active CC effects."
Assert-True ($rootCpp -match "BuffBar->AddBuff\(BuffId,\s*Duration\)") "Expected root HUD to forward Buff add."
Assert-True ($rootCpp -match "BuffBar->RemoveBuff\(BuffId\)") "Expected root HUD to forward Buff remove."
Assert-True ($rootCpp -match "BuffBar->ClearAllBuffs\(\)") "Expected root HUD to forward Buff clear."
Assert-True ($rootCpp -match "DebuffBar->AddDebuff\(DebuffId,\s*Duration\)") "Expected root HUD to forward Debuff add."
Assert-True ($rootCpp -match "DebuffBar->RemoveDebuff\(DebuffId\)") "Expected root HUD to forward Debuff remove."
Assert-True ($rootCpp -match "DebuffBar->ClearAllDebuffs\(\)") "Expected root HUD to forward Debuff clear."
Assert-True ($rootCpp -match "CCBar->AddCCEffect\(CCId,\s*Duration\)") "Expected root HUD to forward CC add."
Assert-True ($rootCpp -match "CCBar->RemoveCCEffect\(CCId\)") "Expected root HUD to forward CC remove."
Assert-True ($rootCpp -match "CCBar->ClearAllCCEffects\(\)") "Expected root HUD to forward CC clear."

Assert-True ($buffHeader -match "BP_OnBuffsCleared") "Expected BuffBar clear BP event."
Assert-True ($buffHeader -match "CachedActiveBuffs") "Expected BuffBar to cache active Buff entries for late widget construction."
Assert-True ($buffCpp -match "NormalizeStatusWidgetId[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected BuffBar widget to normalize and reject blank status-effect ids."
Assert-True ($buffCpp -match "NativeConstruct[\s\S]{0,220}BP_OnBuffsCleared\(\)[\s\S]{0,260}for\s*\(\s*const TPair<FString,\s*float>&\s+CachedBuff") "Expected BuffBar NativeConstruct to clear and replay cached active Buffs."
Assert-True ($buffCpp -match "NativeConstruct[\s\S]{0,520}BP_OnBuffAdded\(CachedBuff\.Key,\s*CachedBuff\.Value\)") "Expected BuffBar NativeConstruct to replay cached Buff ids and durations."
Assert-True ($buffCpp -match "AddBuff[\s\S]*FString\s+NormalizedBuffId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(BuffId,\s*NormalizedBuffId\)\s*\)[\s\S]*return;") "Expected BuffBar AddBuff to no-op blank ids."
Assert-True ($buffCpp -match "RemoveBuff[\s\S]*FString\s+NormalizedBuffId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(BuffId,\s*NormalizedBuffId\)\s*\)[\s\S]*return;") "Expected BuffBar RemoveBuff to no-op blank ids."
Assert-True ($buffCpp -match "CachedActiveBuffs\.Add\(NormalizedBuffId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected BuffBar AddBuff to cache normalized Buff ids and clamped duration."
Assert-True ($buffCpp -match "BP_OnBuffAdded\(NormalizedBuffId,\s*CachedActiveBuffs\[NormalizedBuffId\]\)") "Expected BuffBar to forward cached normalized Buff ids and clamped duration."
Assert-True ($buffCpp -match "CachedActiveBuffs\.Remove\(NormalizedBuffId\)") "Expected BuffBar RemoveBuff to remove cached Buff ids."
Assert-True ($buffCpp -match "BP_OnBuffRemoved\(NormalizedBuffId\)") "Expected BuffBar to forward normalized Buff remove ids."
Assert-True ($buffCpp -match "CachedActiveBuffs\.Reset\(\)") "Expected BuffBar ClearAllBuffs to reset cached active Buffs."
Assert-True ($buffCpp -match "BP_OnBuffsCleared\(\)") "Expected BuffBar ClearAllBuffs to fire BP event."
Assert-True ($debuffHeader -match "BP_OnDebuffsCleared") "Expected DebuffBar clear BP event."
Assert-True ($debuffHeader -match "CachedActiveDebuffs") "Expected DebuffBar to cache active Debuff entries for late widget construction."
Assert-True ($debuffCpp -match "NormalizeStatusWidgetId[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected DebuffBar widget to normalize and reject blank status-effect ids."
Assert-True ($debuffCpp -match "NativeConstruct[\s\S]{0,220}BP_OnDebuffsCleared\(\)[\s\S]{0,260}for\s*\(\s*const TPair<FString,\s*float>&\s+CachedDebuff") "Expected DebuffBar NativeConstruct to clear and replay cached active Debuffs."
Assert-True ($debuffCpp -match "NativeConstruct[\s\S]{0,520}BP_OnDebuffAdded\(CachedDebuff\.Key,\s*CachedDebuff\.Value\)") "Expected DebuffBar NativeConstruct to replay cached Debuff ids and durations."
Assert-True ($debuffCpp -match "AddDebuff[\s\S]*FString\s+NormalizedDebuffId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(DebuffId,\s*NormalizedDebuffId\)\s*\)[\s\S]*return;") "Expected DebuffBar AddDebuff to no-op blank ids."
Assert-True ($debuffCpp -match "RemoveDebuff[\s\S]*FString\s+NormalizedDebuffId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(DebuffId,\s*NormalizedDebuffId\)\s*\)[\s\S]*return;") "Expected DebuffBar RemoveDebuff to no-op blank ids."
Assert-True ($debuffCpp -match "CachedActiveDebuffs\.Add\(NormalizedDebuffId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected DebuffBar AddDebuff to cache normalized Debuff ids and clamped duration."
Assert-True ($debuffCpp -match "BP_OnDebuffAdded\(NormalizedDebuffId,\s*CachedActiveDebuffs\[NormalizedDebuffId\]\)") "Expected DebuffBar to forward cached normalized Debuff ids and clamped duration."
Assert-True ($debuffCpp -match "CachedActiveDebuffs\.Remove\(NormalizedDebuffId\)") "Expected DebuffBar RemoveDebuff to remove cached Debuff ids."
Assert-True ($debuffCpp -match "BP_OnDebuffRemoved\(NormalizedDebuffId\)") "Expected DebuffBar to forward normalized Debuff remove ids."
Assert-True ($debuffCpp -match "CachedActiveDebuffs\.Reset\(\)") "Expected DebuffBar ClearAllDebuffs to reset cached active Debuffs."
Assert-True ($debuffCpp -match "BP_OnDebuffsCleared\(\)") "Expected DebuffBar ClearAllDebuffs to fire BP event."
Assert-True ($ccHeader -match "BP_OnCCEffectsCleared") "Expected CCBar clear BP event."
Assert-True ($ccHeader -match "CachedActiveCCEffects") "Expected CCBar to cache active CC entries for late widget construction."
Assert-True ($ccCpp -match "NormalizeStatusWidgetId[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected CCBar widget to normalize and reject blank status-effect ids."
Assert-True ($ccCpp -match "NativeConstruct[\s\S]{0,220}BP_OnCCEffectsCleared\(\)[\s\S]{0,260}for\s*\(\s*const TPair<FString,\s*float>&\s+CachedCCEffect") "Expected CCBar NativeConstruct to clear and replay cached active CC effects."
Assert-True ($ccCpp -match "NativeConstruct[\s\S]{0,540}BP_OnCCEffectAdded\(CachedCCEffect\.Key,\s*CachedCCEffect\.Value\)") "Expected CCBar NativeConstruct to replay cached CC ids and durations."
Assert-True ($ccCpp -match "AddCCEffect[\s\S]*FString\s+NormalizedCCId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(CCId,\s*NormalizedCCId\)\s*\)[\s\S]*return;") "Expected CCBar AddCCEffect to no-op blank ids."
Assert-True ($ccCpp -match "RemoveCCEffect[\s\S]*FString\s+NormalizedCCId[\s\S]*if\s*\(\s*!NormalizeStatusWidgetId\(CCId,\s*NormalizedCCId\)\s*\)[\s\S]*return;") "Expected CCBar RemoveCCEffect to no-op blank ids."
Assert-True ($ccCpp -match "CachedActiveCCEffects\.Add\(NormalizedCCId,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected CCBar AddCCEffect to cache normalized CC ids and clamped duration."
Assert-True ($ccCpp -match "BP_OnCCEffectAdded\(NormalizedCCId,\s*CachedActiveCCEffects\[NormalizedCCId\]\)") "Expected CCBar to forward cached normalized CC ids and clamped duration."
Assert-True ($ccCpp -match "CachedActiveCCEffects\.Remove\(NormalizedCCId\)") "Expected CCBar RemoveCCEffect to remove cached CC ids."
Assert-True ($ccCpp -match "BP_OnCCEffectRemoved\(NormalizedCCId\)") "Expected CCBar to forward normalized CC remove ids."
Assert-True ($ccCpp -match "CachedActiveCCEffects\.Reset\(\)") "Expected CCBar ClearAllCCEffects to reset cached active CC effects."
Assert-True ($ccCpp -match "BP_OnCCEffectsCleared\(\)") "Expected CCBar ClearAllCCEffects to fire BP event."

Assert-True ($managerHeader -match "AddArenaHUDBuff") "Expected UI manager Buff add entrypoint."
Assert-True ($managerHeader -match "RemoveArenaHUDBuff") "Expected UI manager Buff remove entrypoint."
Assert-True ($managerHeader -match "ClearArenaHUDBuffs") "Expected UI manager Buff clear entrypoint."
Assert-True ($managerHeader -match "AddArenaHUDDebuff") "Expected UI manager Debuff add entrypoint."
Assert-True ($managerHeader -match "AddArenaHUDCCEffect") "Expected UI manager CC add entrypoint."
Assert-True ($managerCpp -match "Controller->AddStatusBuff\(BuffId,\s*Duration\)") "Expected UI manager to push Buff add into controller."
Assert-True ($managerCpp -match "Controller->RemoveStatusBuff\(BuffId\)") "Expected UI manager to push Buff remove into controller."
Assert-True ($managerCpp -match "Controller->ClearStatusBuffs\(\)") "Expected UI manager to push Buff clear into controller."
Assert-True ($managerCpp -match "Controller->AddStatusDebuff\(DebuffId,\s*Duration\)") "Expected UI manager to push Debuff add into controller."
Assert-True ($managerCpp -match "Controller->AddStatusCCEffect\(CCId,\s*Duration\)") "Expected UI manager to push CC add into controller."
Assert-True ($statusEffectsTest -match "StatusEffectsCacheActiveEntries") "Expected automation coverage for active status-effect cache."
Assert-True ($statusEffectsTest -match "GetActiveStatusBuffs") "Expected automation test to exercise active Buff cache getter."
Assert-True ($statusEffectsTest -match "GetActiveStatusDebuffs") "Expected automation test to exercise active Debuff cache getter."
Assert-True ($statusEffectsTest -match "GetActiveStatusCCEffects") "Expected automation test to exercise active CC cache getter."
Assert-True ($statusEffectsTest -match 'AddStatusBuff\(TEXT\("Haste"\),\s*-1\.0f\)') "Expected automation test to verify Buff add/clamp."
Assert-True ($statusEffectsTest -match 'RemoveStatusBuff\(TEXT\("Haste"\)\)') "Expected automation test to verify Buff remove."
Assert-True ($statusEffectsTest -match 'ClearStatusDebuffs\(\)') "Expected automation test to verify Debuff clear."
Assert-True ($statusEffectsTest -match 'RemoveStatusCCEffect\(TEXT\("Stun"\)\)') "Expected automation test to verify CC remove."

Write-Host "PASS: Arena HUD status effects sync contract" -ForegroundColor Green
