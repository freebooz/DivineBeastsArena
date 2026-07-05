<#
Validates healing and shield state changes cannot bypass the server-authoritative path.
The spell entrypoints remain BlueprintCallable for configuration/presentation shells,
but AttributeSet health and shield mutation must fail closed outside authority.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$bloomCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBABloomHealingSpell.cpp"
$bloomHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBABloomHealingSpell.h"
$shieldCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAHolyShieldSpell.cpp"
$shieldHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAHolyShieldSpell.h"
$bloomCpp = Get-Content -LiteralPath $bloomCppPath -Encoding UTF8 -Raw
$bloomHeader = Get-Content -LiteralPath $bloomHeaderPath -Encoding UTF8 -Raw
$shieldCpp = Get-Content -LiteralPath $shieldCppPath -Encoding UTF8 -Raw
$shieldHeader = Get-Content -LiteralPath $shieldHeaderPath -Encoding UTF8 -Raw

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

Assert-True ($bloomHeader.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Bloom Healing"")")) `
  "Expected Bloom Healing cast entrypoint to remain BlueprintCallable for configuration/presentation shell usage."

Assert-True ($shieldHeader.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Holy Shield"")")) `
  "Expected Holy Shield cast entrypoint to remain BlueprintCallable for configuration/presentation shell usage."

$bloomCastBody = Get-FunctionBody `
  $bloomCpp `
  "void\s+ADBABloomHealingSpell::CastBloomHealing\s*\(" `
  "`nvoid\s+ADBABloomHealingSpell::ReleaseBloom" `
  "CastBloomHealing"

Assert-True ($bloomCastBody.Contains("if (!HasAuthority() && GetNetMode() != NM_Standalone)")) `
  "Expected CastBloomHealing to reject non-authority network callers before scheduling healing."

$applyHealingBody = Get-FunctionBody `
  $bloomCpp `
  "void\s+ADBABloomHealingSpell::ApplyHealing\s*\(" `
  "`nvoid\s+ADBABloomHealingSpell::SpawnVFX" `
  "ApplyHealing"

foreach ($required in @(
    "if (!HasAuthority())",
    "return;",
    "UAbilitySystemComponent* ASC",
    "AttrSet->SetCurrentHealth"
  )) {
  Assert-True ($applyHealingBody.Contains($required)) "Expected ApplyHealing to contain: $required"
}

$healingAuthorityIndex = $applyHealingBody.IndexOf("if (!HasAuthority())")
$healingAscIndex = $applyHealingBody.IndexOf("UAbilitySystemComponent* ASC")
$healingSetHealthIndex = $applyHealingBody.IndexOf("AttrSet->SetCurrentHealth")
Assert-True ($healingAuthorityIndex -ge 0 -and $healingAscIndex -gt $healingAuthorityIndex -and $healingSetHealthIndex -gt $healingAuthorityIndex) `
  "Expected ApplyHealing to guard authority before AttributeSet health mutation."

$shieldCastBody = Get-FunctionBody `
  $shieldCpp `
  "void\s+ADBAHolyShieldSpell::CastHolyShield\s*\(" `
  "`nvoid\s+ADBAHolyShieldSpell::ApplyShield" `
  "CastHolyShield"

Assert-True ($shieldCastBody.Contains("if (!HasAuthority() && GetNetMode() != NM_Standalone)")) `
  "Expected CastHolyShield to reject non-authority network callers before applying shield."

$applyShieldBody = Get-FunctionBody `
  $shieldCpp `
  "void\s+ADBAHolyShieldSpell::ApplyShield\s*\(" `
  "`nvoid\s+ADBAHolyShieldSpell::ReleaseShield" `
  "ApplyShield"

foreach ($required in @(
    "if (!HasAuthority())",
    "return;",
    "UAbilitySystemComponent* ASC",
    "AttrSet->SetMaxShield",
    "AttrSet->SetCurrentShield"
  )) {
  Assert-True ($applyShieldBody.Contains($required)) "Expected ApplyShield to contain: $required"
}

$shieldAuthorityIndex = $applyShieldBody.IndexOf("if (!HasAuthority())")
$shieldAscIndex = $applyShieldBody.IndexOf("UAbilitySystemComponent* ASC")
$shieldSetMaxIndex = $applyShieldBody.IndexOf("AttrSet->SetMaxShield")
$shieldSetCurrentIndex = $applyShieldBody.IndexOf("AttrSet->SetCurrentShield")
Assert-True ($shieldAuthorityIndex -ge 0 -and $shieldAscIndex -gt $shieldAuthorityIndex -and $shieldSetMaxIndex -gt $shieldAuthorityIndex -and $shieldSetCurrentIndex -gt $shieldAuthorityIndex) `
  "Expected ApplyShield to guard authority before AttributeSet shield mutation."

$releaseShieldBody = Get-FunctionBody `
  $shieldCpp `
  "void\s+ADBAHolyShieldSpell::ReleaseShield\s*\(" `
  "`nvoid\s+ADBAHolyShieldSpell::MulticastPlayShieldStart_Implementation" `
  "ReleaseShield"

foreach ($required in @(
    "if (!HasAuthority())",
    "return;",
    "AttrSet->SetCurrentShield",
    "AttrSet->SetMaxShield",
    "MulticastPlayShieldEnd"
  )) {
  Assert-True ($releaseShieldBody.Contains($required)) "Expected ReleaseShield to contain: $required"
}

$releaseAuthorityIndex = $releaseShieldBody.IndexOf("if (!HasAuthority())")
$releaseSetCurrentIndex = $releaseShieldBody.IndexOf("AttrSet->SetCurrentShield")
$releaseSetMaxIndex = $releaseShieldBody.IndexOf("AttrSet->SetMaxShield")
$releaseMulticastIndex = $releaseShieldBody.IndexOf("MulticastPlayShieldEnd")
Assert-True ($releaseAuthorityIndex -ge 0 -and $releaseSetCurrentIndex -gt $releaseAuthorityIndex -and $releaseSetMaxIndex -gt $releaseAuthorityIndex -and $releaseMulticastIndex -gt $releaseAuthorityIndex) `
  "Expected ReleaseShield to guard authority before shield removal or end multicast."

Write-Host "PASS: Healing and shield authority boundary contract" -ForegroundColor Green
