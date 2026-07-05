<#
Validates DBA AbilitySystemComponent uses AvatarActor context when PlayerState owns ASC.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$ascHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

$ascHeader = Get-Content -LiteralPath $ascHeaderPath -Encoding UTF8 -Raw
$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw

function New-TextFromCodePoints {
  param([int[]]$CodePoints)
  return -join ($CodePoints | ForEach-Object { [char]$_ })
}

$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 65, 83, 67, 32, 35282, 33394, 19978, 19979, 25991, 20351, 29992, 32, 65, 118, 97, 116, 97, 114, 65, 99, 116, 111, 114, 32, 35821, 20041)

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
  Assert-True $match.Success "Missing function implementation: $FunctionName"

  $remaining = $Content.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Missing function end boundary: $FunctionName"

  return $Content.Substring($match.Index, $match.Length + $nextMatch.Index)
}

Assert-True -Condition ($ascHeader -match 'class\s+ADBAZodiacCharacterBase\s*;') -Message 'ASC header must forward-declare ADBAZodiacCharacterBase.'
Assert-True -Condition ($ascHeader -match 'ADBAZodiacCharacterBase\*\s+GetDBAAvatarCharacter\s*\(\s*\)\s+const\s*;') -Message 'ASC must declare GetDBAAvatarCharacter.'
Assert-True -Condition ($ascCpp -match 'ADBAZodiacCharacterBase\*\s+UDBAAbilitySystemComponent::GetDBAAvatarCharacter\s*\(\s*\)\s+const') -Message 'ASC must implement GetDBAAvatarCharacter.'
Assert-True -Condition ($ascCpp -match 'GetDBAAvatarCharacter[\s\S]*AbilityActorInfo[\s\S]*AvatarActor') -Message 'GetDBAAvatarCharacter must resolve AbilityActorInfo AvatarActor first.'

$inputCooldownBody = Get-FunctionBody `
  $ascCpp `
  "bool\s+UDBAAbilitySystemComponent::IsInputAbilityOnCooldown\s*\(\s*int32\s+InputID\s*\)\s+const\s*\{" `
  "`nFGameplayAbilitySpecHandle\s+UDBAAbilitySystemComponent::FindAbilitySpecHandleByInputID" `
  "IsInputAbilityOnCooldown"

Assert-True -Condition ($inputCooldownBody -match 'GetDBAAvatarCharacter\s*\(\s*\)') -Message 'Input cooldown gate must resolve the avatar character.'
Assert-True -Condition (-not ($inputCooldownBody -match 'Cast<ADBAZodiacCharacterBase>\s*\(\s*GetOwner\s*\(\s*\)\s*\)')) -Message 'Input cooldown gate must not cast component owner to Character.'

$targetBody = Get-FunctionBody `
  $ascCpp `
  "bool\s+UDBAAbilitySystemComponent::IsValidTarget\s*\(\s*AActor\*\s+Target\s*,\s*bool\s+bRequireEnemy\s*\)\s+const\s*\{" `
  "`nvoid\s+UDBAAbilitySystemComponent::TriggerGameplayCue" `
  "IsValidTarget"

Assert-True -Condition ($targetBody -match 'GetDBAAvatarCharacter\s*\(\s*\)') -Message 'Target validation must resolve source from avatar character.'
Assert-True -Condition (-not ($targetBody -match 'AActor\*\s+SourceActor\s*=\s*GetOwner\s*\(\s*\)\s*;')) -Message 'Target validation must not use PlayerState owner as source actor.'

$cueBody = Get-FunctionBody `
  $ascCpp `
  "void\s+UDBAAbilitySystemComponent::TriggerGameplayCue\s*\(\s*const\s+FGameplayTag&\s+CueTag\s*,\s*AActor\*\s+Target\s*\)\s*\{" `
  "`nvoid\s+UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy" `
  "TriggerGameplayCue"

Assert-True -Condition ($cueBody -match 'GetDBAAvatarCharacter\s*\(\s*\)') -Message 'GameplayCue must prefer avatar character as Instigator.'
Assert-True -Condition (-not ($cueBody -match 'CueParams\.Instigator\s*=\s*GetOwner\s*\(\s*\)\s*;')) -Message 'GameplayCue must not use PlayerState owner as Instigator.'

$syncBody = Get-FunctionBody `
  $ascCpp `
  "void\s+UDBAAbilitySystemComponent::SyncCooldownsToCharacter\s*\(\s*\)\s*\{" `
  "`nvoid\s+UDBAAbilitySystemComponent::GetLifetimeReplicatedProps" `
  "SyncCooldownsToCharacter"

Assert-True -Condition ($syncBody -match 'GetDBAAvatarCharacter\s*\(\s*\)') -Message 'Cooldown mirror sync must resolve the avatar character.'
Assert-True -Condition (-not ($syncBody -match 'Cast<ADBAZodiacCharacterBase>\s*\(\s*GetOwner\s*\(\s*\)\s*\)')) -Message 'Cooldown mirror sync must not cast component owner to Character.'

Write-Host $successMessage -ForegroundColor Green
