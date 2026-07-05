<#
Validates direct Chain/Resonance panel input boundaries.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$chainPanelHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAChainUltimatePanelWidgetBase.h"
$chainPanelCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAChainUltimatePanelWidgetBase.cpp"
$resonancePanelHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPassiveAndResonancePanelWidgetBase.h"
$resonancePanelCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPassiveAndResonancePanelWidgetBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$chainPanelHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $chainPanelHeaderPath
$chainPanelCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $chainPanelCppPath
$resonancePanelHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $resonancePanelHeaderPath
$resonancePanelCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $resonancePanelCppPath

Assert-True ($chainPanelCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected ChainUltimatePanel implementation to include DBAConstants."
Assert-True ($chainPanelHeader -match "CachedChainCount\s*=\s*0") "Expected ChainUltimatePanel to cache the latest normalized chain count."
Assert-True ($chainPanelCpp -match "NativeConstruct[\s\S]{0,180}BP_OnChainCountUpdated\(CachedChainCount\)") "Expected ChainUltimatePanel NativeConstruct to replay cached chain count."
Assert-True ($chainPanelCpp -match "UpdateChainCount[\s\S]*NormalizedChainCount\s*=\s*FMath::Clamp\(\s*Count,\s*0,\s*DBAConstants::MaxChainLevel\)") "Expected ChainUltimatePanel direct chain count input to clamp to DBAConstants::MaxChainLevel."
Assert-True ($chainPanelCpp -match "CachedChainCount\s*=\s*NormalizedChainCount") "Expected ChainUltimatePanel to cache normalized chain count."
Assert-True ($chainPanelCpp -match "BP_OnChainCountUpdated\(CachedChainCount\)") "Expected ChainUltimatePanel to broadcast cached normalized chain count."

Assert-True ($resonancePanelCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected PassiveAndResonancePanel implementation to include DBAConstants."
Assert-True ($resonancePanelHeader -match "CachedPassiveSkillStates") "Expected PassiveAndResonancePanel to cache passive skill slot states for late widget construction."
Assert-True ($resonancePanelHeader -match "CachedResonanceLevel\s*=\s*0") "Expected PassiveAndResonancePanel to cache the latest normalized resonance level."
Assert-True ($resonancePanelCpp -match "NativeConstruct[\s\S]{0,260}for\s*\(\s*const TPair<int32,\s*bool>&\s+CachedPassiveSkill") "Expected PassiveAndResonancePanel NativeConstruct to replay cached passive skill slots."
Assert-True ($resonancePanelCpp -match "NativeConstruct[\s\S]{0,520}BP_OnPassiveUpdated\(CachedPassiveSkill\.Key,\s*CachedPassiveSkill\.Value\)") "Expected PassiveAndResonancePanel NativeConstruct to replay cached passive skill state."
Assert-True ($resonancePanelCpp -match "NativeConstruct[\s\S]{0,180}BP_OnResonanceLevelUpdated\(CachedResonanceLevel\)") "Expected PassiveAndResonancePanel NativeConstruct to replay cached resonance level."
Assert-True ($resonancePanelCpp -match "UpdatePassiveSkill[\s\S]*NormalizedSlotIndex\s*=\s*FMath::Clamp\(\s*SlotIndex,\s*0,\s*DBAConstants::CoreCombatInputCount\s*-\s*1\)") "Expected PassiveAndResonancePanel direct passive slot input to clamp to combat input slot bounds."
Assert-True ($resonancePanelCpp -match "CachedPassiveSkillStates\.Add\(NormalizedSlotIndex,\s*bActive\)") "Expected PassiveAndResonancePanel to cache normalized passive skill state."
Assert-True ($resonancePanelCpp -match "BP_OnPassiveUpdated\(NormalizedSlotIndex,\s*CachedPassiveSkillStates\[NormalizedSlotIndex\]\)") "Expected PassiveAndResonancePanel to broadcast cached normalized passive skill state."
Assert-True ($resonancePanelCpp -match "UpdateResonanceLevel[\s\S]*NormalizedResonanceLevel\s*=\s*FMath::Clamp\(\s*Level,\s*0,\s*DBAConstants::MaxResonanceLevel\)") "Expected PassiveAndResonancePanel direct resonance level input to clamp to DBAConstants::MaxResonanceLevel."
Assert-True ($resonancePanelCpp -match "CachedResonanceLevel\s*=\s*NormalizedResonanceLevel") "Expected PassiveAndResonancePanel to cache normalized resonance level."
Assert-True ($resonancePanelCpp -match "BP_OnResonanceLevelUpdated\(CachedResonanceLevel\)") "Expected PassiveAndResonancePanel to broadcast cached normalized resonance level."

Write-Host "PASS: Arena HUD Chain/Resonance panel boundary contract" -ForegroundColor Green
