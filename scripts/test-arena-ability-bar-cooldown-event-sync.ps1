<#
Validates Arena AbilityBar cooldown UI is driven by character cooldown events
instead of relying on per-frame polling by default.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$characterHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$abilityBarHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.h"
$abilityBarCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$characterHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterHeaderPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath
$abilityBarHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilityBarHeaderPath
$abilityBarCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilityBarCppPath

Assert-True ($characterHeader -match "DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam\(\s*FOnSkillCooldownsChanged") "Expected character to declare FOnSkillCooldownsChanged delegate."
Assert-True ($characterHeader -match 'UPROPERTY\(BlueprintAssignable,\s*Category\s*=\s*"DBA\|Character\|Spectator"\)\s*FOnSkillCooldownsChanged\s+OnSkillCooldownsChanged') "Expected character to expose OnSkillCooldownsChanged."
Assert-True ($characterHeader -match "UFUNCTION\(\)\s*void\s+OnRep_SkillCooldowns\s*\(") "Expected character to declare OnRep_SkillCooldowns."
Assert-True ($characterHeader -match 'UPROPERTY\(ReplicatedUsing\s*=\s*OnRep_SkillCooldowns,\s*BlueprintReadOnly,\s*Category\s*=\s*"DBA\|Spectator"\)\s*TArray<float>\s+SkillCooldowns') "Expected SkillCooldowns to replicate through OnRep_SkillCooldowns."

Assert-True ($characterCpp -match "void\s+ADBAZodiacCharacterBase::OnRep_SkillCooldowns\s*\(\s*\)[\s\S]*OnSkillCooldownsChanged\.Broadcast\(SkillCooldowns\)") "Expected OnRep_SkillCooldowns to broadcast cooldown changes."
Assert-True ($characterCpp -match "UpdateSkillCooldowns[\s\S]*OnSkillCooldownsChanged\.Broadcast\(SkillCooldowns\)") "Expected server-side UpdateSkillCooldowns to broadcast cooldown changes."
Assert-True ($characterCpp -match "DOREPLIFETIME\(ADBAZodiacCharacterBase,\s*SkillCooldowns\)") "Expected SkillCooldowns to remain replicated."

Assert-True ($abilityBarHeader -match "bRefreshCooldownsEveryTick\s*=\s*false") "Expected AbilityBar to disable every-tick cooldown refresh by default."
Assert-True ($abilityBarHeader -match "UFUNCTION\(\)\s*void\s+HandleSkillCooldownsChanged\s*\(\s*const\s+TArray<float>&\s+Cooldowns\s*\)") "Expected AbilityBar to declare cooldown event handler."
Assert-True ($abilityBarHeader -match "void\s+UnbindFromCooldownEvents\s*\(") "Expected AbilityBar to declare cooldown event unbind helper."
Assert-True ($abilityBarHeader -match "TWeakObjectPtr<ADBAZodiacCharacterBase>\s+CooldownEventCharacter") "Expected AbilityBar to track the character bound for cooldown events."

Assert-True ($abilityBarCpp -match "NativeDestruct[\s\S]*UnbindFromCooldownEvents\(\)") "Expected AbilityBar to unbind cooldown events during NativeDestruct."
Assert-True ($abilityBarCpp -match "BindToCharacter[\s\S]*UnbindFromCooldownEvents\(\)[\s\S]*OnSkillCooldownsChanged\.AddDynamic\(this,\s*&UDBAAbilityBarWidgetBase::HandleSkillCooldownsChanged\)") "Expected BindToCharacter to bind the character cooldown event."
Assert-True ($abilityBarCpp -match "HandleSkillCooldownsChanged[\s\S]*UpdateAbility\(SkillSlot,\s*Remaining,\s*0\.0f\)") "Expected cooldown event handler to update ability slots."
Assert-True ($abilityBarCpp -match "UnbindFromCooldownEvents[\s\S]*OnSkillCooldownsChanged\.RemoveDynamic\(this,\s*&UDBAAbilityBarWidgetBase::HandleSkillCooldownsChanged\)") "Expected unbind helper to remove the dynamic cooldown binding."

Write-Host "PASS: Arena AbilityBar cooldown event sync contract" -ForegroundColor Green
