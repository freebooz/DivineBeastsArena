<#
Validates Zodiac character internal equipped-skill cast helper is server-authoritative even if future code bypasses public entrypoints.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

$cpp = Get-Content -LiteralPath $cppPath -Encoding UTF8 -Raw

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
    [Parameter(Mandatory = $true)][string]$StartPattern,
    [Parameter(Mandatory = $true)][string]$EndPattern,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $match = [regex]::Match($cpp, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $cpp.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $cpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

function Assert-AuthorityGuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $guardIndex = $Body.IndexOf("!HasAuthority()")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($guardIndex -ge 0) "Expected CastEquippedSkillInternal to reject non-authority before $Operation."
  Assert-True ($returnIndex -gt $guardIndex) "Expected CastEquippedSkillInternal authority guard to return."
  Assert-True ($operationIndex -gt $returnIndex) "Expected CastEquippedSkillInternal to run $Operation only after authority guard."
}

$internalCastBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::CastEquippedSkillInternal\s*\(\s*int32\s+SkillSlot,\s*const\s+FVector&\s+AimDirection,\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::MulticastPlayLobbySkillCastFeedback_Implementation" `
  "CastEquippedSkillInternal"

Assert-AuthorityGuardBeforeOperation $internalCastBody "GetWorld()"
Assert-AuthorityGuardBeforeOperation $internalCastBody "TryActivateAbilityByInputID"
Assert-AuthorityGuardBeforeOperation $internalCastBody "SpawnActor<"
Assert-AuthorityGuardBeforeOperation $internalCastBody "SkillCooldowns[CooldownArrayIndex] = Spec.Cooldown"
Assert-AuthorityGuardBeforeOperation $internalCastBody "MulticastPlayLobbySkillCastFeedback"

Write-Host "PASS: Zodiac character internal cast authority boundary" -ForegroundColor Green
