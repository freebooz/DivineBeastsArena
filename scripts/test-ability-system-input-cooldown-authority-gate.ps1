<#
Validates server-authoritative GAS input activation checks the character
cooldown cache before attempting an ability activation by input ID.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$characterCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw
$characterCpp = Get-Content -LiteralPath $characterCppPath -Encoding UTF8 -Raw

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
    [Parameter(Mandatory = $true)][string]$Content,
    [Parameter(Mandatory = $true)][string]$StartPattern,
    [Parameter(Mandatory = $true)][string]$EndPattern,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $match = [regex]::Match($Content, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $Content.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $Content.Substring($match.Index, $match.Length + $nextMatch.Index)
}

$activationBody = Get-FunctionBody `
  $ascCpp `
  "bool\s+UDBAAbilitySystemComponent::TryActivateAbilityByInputID\s*\(\s*int32\s+InputID\s*,\s*AActor\*\s+Target\s*\)\s*\{" `
  "`nbool\s+UDBAAbilitySystemComponent::IsInputAbilityOnCooldown" `
  "TryActivateAbilityByInputID"

$inputCooldownBody = Get-FunctionBody `
  $ascCpp `
  "bool\s+UDBAAbilitySystemComponent::IsInputAbilityOnCooldown\s*\(\s*int32\s+InputID\s*\)\s+const\s*\{" `
  "`nFGameplayAbilitySpecHandle\s+UDBAAbilitySystemComponent::FindAbilitySpecHandleByInputID" `
  "IsInputAbilityOnCooldown"

Assert-True ($ascCpp -match "int32\s+MapAbilityInputIDToCooldownSkillSlot\s*\(\s*int32\s+InputID\s*\)") `
  "Expected a C++ InputID to SkillSlot mapping helper for cooldown authority checks."
Assert-True ($ascCpp -match "MapAbilityInputIDToCooldownSkillSlot[\s\S]*EDBAAbilityInputID::Skill01[\s\S]*return\s+1\s*;") `
  "Expected Skill01 input to map to cooldown SkillSlot 1."
Assert-True ($ascCpp -match "MapAbilityInputIDToCooldownSkillSlot[\s\S]*EDBAAbilityInputID::Ultimate[\s\S]*return\s+DBAConstants::ArenaCombatSkillSlotCount\s*;") `
  "Expected Ultimate input to map after active skill slots."
Assert-True ($activationBody -match "IsInputAbilityOnCooldown\(InputID\)[\s\S]{0,120}return\s+false\s*;") `
  "Expected TryActivateAbilityByInputID to reject active cooldowns before TryActivateAbility."
Assert-True ($inputCooldownBody.Contains("const ADBAZodiacCharacterBase* Character = GetDBAAvatarCharacter();")) `
  "Expected IsInputAbilityOnCooldown to resolve the AvatarActor Zodiac character."
Assert-True (-not ($inputCooldownBody -match "Cast<ADBAZodiacCharacterBase>\s*\(\s*GetOwner\s*\(\s*\)\s*\)")) `
  "Expected IsInputAbilityOnCooldown not to treat PlayerState owner as the Zodiac character."
Assert-True ($inputCooldownBody.Contains("const int32 CooldownSkillSlot = MapAbilityInputIDToCooldownSkillSlot(InputID);")) `
  "Expected IsInputAbilityOnCooldown to derive the cooldown skill slot from InputID."
Assert-True ($inputCooldownBody.Contains("Character->GetPlayableSkillSpecs()")) `
  "Expected IsInputAbilityOnCooldown to inspect runtime playable skill specs."
Assert-True ($inputCooldownBody -match "SkillSpec\.SkillSlot\s*==\s*CooldownSkillSlot") `
  "Expected IsInputAbilityOnCooldown to match runtime specs by cooldown skill slot."
Assert-True ($inputCooldownBody -match "Character->IsAbilityOnCooldown\(SkillSpec\.SkillId\)") `
  "Expected IsInputAbilityOnCooldown to delegate to the character cooldown cache."

$cooldownQueryBody = Get-FunctionBody `
  $characterCpp `
  "bool\s+ADBAZodiacCharacterBase::IsAbilityOnCooldown\s*\(\s*FName\s+SkillId\s*\)\s*const\s*\{" `
  "`nbool\s+ADBAZodiacCharacterBase::HasEnoughEnergy" `
  "IsAbilityOnCooldown"

Assert-True ($cooldownQueryBody.Contains("SkillCooldowns.IsValidIndex(CooldownArrayIndex)")) `
  "Expected character cooldown query to remain array-index guarded."

Write-Host "PASS: AbilitySystem input cooldown authority gate contract" -ForegroundColor Green
