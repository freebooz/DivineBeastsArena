<#
Validates DBAClientPredictionComponent only runs prediction and correction on locally controlled client instances.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAClientPredictionComponent.h"
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAClientPredictionComponent.cpp"

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
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

function Assert-RuntimeGuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $guardIndex = $Body.IndexOf("!IsPredictionRuntimeAllowed()")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to reject unsafe prediction runtime before $Operation."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName unsafe-runtime guard to return before $Operation."
  Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after prediction runtime guard."
}

Assert-True ($header -match "bool\s+IsPredictionRuntimeAllowed\s*\(\s*\)\s+const\s*;") `
  "Expected ClientPredictionComponent to declare a reusable local client runtime guard."
Assert-True ($cpp.Contains('#include "Engine/World.h"')) "Expected implementation to include Engine/World.h for NetMode checks."
Assert-True ($cpp.Contains('#include "GameDBA/GAS/DBAAbilitySystemComponent.h"')) "Expected implementation to include DBAAbilitySystemComponent for handle resolution."
Assert-True ($cpp.Contains("PrimaryComponentTick.bCanEverTick = false")) "Expected prediction component to keep ticking disabled until a TickComponent implementation exists."
Assert-True ($cpp.Contains("PrimaryComponentTick.bStartWithTickEnabled = false")) "Expected prediction component to start with ticking disabled."

$guardBody = Get-FunctionBody `
  "bool\s+UDBAClientPredictionComponent::IsPredictionRuntimeAllowed\s*\(\s*\)\s+const\s*\{" `
  "`nvoid\s+UDBAClientPredictionComponent::TryPredictAbility" `
  "IsPredictionRuntimeAllowed"

Assert-True ($guardBody.Contains("GetWorld()")) "Expected prediction runtime guard to require a World."
Assert-True ($guardBody.Contains("NM_DedicatedServer")) "Expected prediction runtime guard to reject Dedicated Server."
Assert-True ($guardBody.Contains("ADBAZodiacCharacterBase")) "Expected prediction runtime guard to require a Zodiac character owner."
Assert-True ($guardBody.Contains("IsLocallyControlled()")) "Expected prediction runtime guard to require local control."

$resolveBody = Get-FunctionBody `
  "int32\s+ResolvePredictionAbilityInputID\s*\(\s*FName\s+SkillId\s*\)\s*\{" `
  "`n\}`r?\n\r?\nUDBAClientPredictionComponent::UDBAClientPredictionComponent" `
  "ResolvePredictionAbilityInputID"

Assert-True ($resolveBody.Contains("EDBAAbilityInputID::Skill01")) "Expected prediction skill id resolver to support Skill01."
Assert-True ($resolveBody.Contains("EDBAAbilityInputID::Skill02")) "Expected prediction skill id resolver to support Skill02."
Assert-True ($resolveBody.Contains("EDBAAbilityInputID::Skill03")) "Expected prediction skill id resolver to support Skill03."
Assert-True ($resolveBody.Contains("EDBAAbilityInputID::Skill04")) "Expected prediction skill id resolver to support Skill04."
Assert-True ($resolveBody.Contains("EDBAAbilityInputID::Ultimate")) "Expected prediction skill id resolver to support Ultimate."

$abilityBody = Get-FunctionBody `
  "void\s+UDBAClientPredictionComponent::TryPredictAbility\s*\(\s*FName\s+SkillId,\s*AActor\*\s+Target,\s*FVector\s+TargetLocation\s*\)\s*\{" `
  "`nvoid\s+UDBAClientPredictionComponent::TryPredictMove" `
  "TryPredictAbility"
$moveBody = Get-FunctionBody `
  "void\s+UDBAClientPredictionComponent::TryPredictMove\s*\(\s*FVector\s+TargetLocation\s*\)\s*\{" `
  "`nvoid\s+UDBAClientPredictionComponent::ApplyServerCorrection" `
  "TryPredictMove"
$correctionBody = Get-FunctionBody `
  "void\s+UDBAClientPredictionComponent::ApplyServerCorrection\s*\(\s*FVector\s+ServerLocation,\s*float\s+ServerTime\s*\)\s*\{" `
  "`nvoid\s+UDBAClientPredictionComponent::OnAbilityActivated" `
  "ApplyServerCorrection"
$moveCorrectedBody = Get-FunctionBody `
  "void\s+UDBAClientPredictionComponent::OnMoveCorrected\s*\(\s*FVector\s+CorrectedLocation\s*\)\s*\{" `
  "\z" `
  "OnMoveCorrected"

Assert-RuntimeGuardBeforeOperation $abilityBody "TryPredictAbility" "ServerTryActivateAbility"
Assert-RuntimeGuardBeforeOperation $moveBody "TryPredictMove" "PredictedLocation"
Assert-RuntimeGuardBeforeOperation $moveBody "TryPredictMove" "ServerMoveTo"
Assert-RuntimeGuardBeforeOperation $correctionBody "ApplyServerCorrection" "PredictionError"
Assert-RuntimeGuardBeforeOperation $correctionBody "ApplyServerCorrection" "OnMoveCorrected"
Assert-RuntimeGuardBeforeOperation $moveCorrectedBody "OnMoveCorrected" "SetActorLocation"

Assert-True ($abilityBody.Contains("ResolvePredictionAbilityInputID(SkillId)")) "Expected TryPredictAbility to resolve SkillId into a GAS input ID."
Assert-True ($abilityBody.Contains("FindAbilitySpecHandleByInputID(AbilityInputID)")) "Expected TryPredictAbility to resolve a valid AbilitySpecHandle by input ID."
Assert-True ($abilityBody.Contains("if (!AbilityHandle.IsValid())")) "Expected TryPredictAbility to reject unresolved ability handles."
Assert-True ($abilityBody.Contains("Params.AbilityHandle = AbilityHandle;")) "Expected TryPredictAbility to send a resolved AbilityHandle."
Assert-True (-not $abilityBody.Contains("Params.AbilityHandle = FGameplayAbilitySpecHandle();")) "Expected TryPredictAbility to avoid sending an empty AbilityHandle."
Assert-True (-not $abilityBody.Contains("static_cast<void>(SkillId);")) "Expected TryPredictAbility to use SkillId instead of discarding it."
Assert-True ($correctionBody.Contains("static_cast<void>(ServerTime);")) "Expected ApplyServerCorrection to consume ServerTime until smoothing is implemented."

Write-Host "PASS: Client prediction local runtime boundary" -ForegroundColor Green
