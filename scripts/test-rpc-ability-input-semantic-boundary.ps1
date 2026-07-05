<#
Validates AbilityHandle RPC entrypoints cannot be cross-used between normal
skill activation and Ultimate activation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$rpcHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h"
$rpcCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"
$predictionCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAClientPredictionComponent.cpp"

$rpcHeader = Get-Content -LiteralPath $rpcHeaderPath -Encoding UTF8 -Raw
$rpcCpp = Get-Content -LiteralPath $rpcCppPath -Encoding UTF8 -Raw
$predictionCpp = Get-Content -LiteralPath $predictionCppPath -Encoding UTF8 -Raw

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

$tryActivateImpl = Get-FunctionBody `
  $rpcCpp `
  "void\s+ADBARpcHandler::ServerTryActivateAbility_Implementation\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerTryActivateAbility_Validate" `
  "ServerTryActivateAbility_Implementation"

$tryActivateValidate = Get-FunctionBody `
  $rpcCpp `
  "bool\s+ADBARpcHandler::ServerTryActivateAbility_Validate\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerCancelAbility_Implementation" `
  "ServerTryActivateAbility_Validate"

$ultimateImpl = Get-FunctionBody `
  $rpcCpp `
  "void\s+ADBARpcHandler::ServerUltimateAbility_Implementation\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerUltimateAbility_Validate" `
  "ServerUltimateAbility_Implementation"

$ultimateValidate = Get-FunctionBody `
  $rpcCpp `
  "bool\s+ADBARpcHandler::ServerUltimateAbility_Validate\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`n// ==================== IDBARpcClient" `
  "ServerUltimateAbility_Validate"

$predictionAbilityBody = Get-FunctionBody `
  $predictionCpp `
  "void\s+UDBAClientPredictionComponent::TryPredictAbility\s*\(\s*FName\s+SkillId,\s*AActor\*\s+Target,\s*FVector\s+TargetLocation\s*\)\s*\{" `
  "`nvoid\s+UDBAClientPredictionComponent::TryPredictMove" `
  "TryPredictAbility"

Assert-True ($rpcHeader.Contains("bool ValidateAbilityInputSemantics(const FDBAAbilityRpcParams& Params, bool bRequireUltimate) const;")) `
  "Expected DBARpcHandler to declare a shared AbilityHandle input semantic validator."
Assert-True ($rpcCpp -match "ValidateAbilityInputSemantics[\s\S]*FindAbilitySpecFromHandle\(Params\.AbilityHandle\)") `
  "Expected input semantic validator to resolve the requested AbilityHandle to a spec."
Assert-True ($rpcCpp -match "ValidateAbilityInputSemantics[\s\S]*Spec->InputID\s*==\s*static_cast<int32>\(EDBAAbilityInputID::Ultimate\)") `
  "Expected input semantic validator to identify Ultimate specs."
Assert-True ($rpcCpp -match "ValidateAbilityInputSemantics[\s\S]*bRequireUltimate") `
  "Expected input semantic validator to support normal-vs-Ultimate RPC modes."

foreach ($bodyAndName in @(
    @{ Body = $tryActivateImpl; Name = "ServerTryActivateAbility_Implementation"; RequireUltimate = "false"; Operation = "ValidateAbilityCooldown(Params)" },
    @{ Body = $tryActivateValidate; Name = "ServerTryActivateAbility_Validate"; RequireUltimate = "false"; Operation = "ValidateAbilityCooldown(Params)" },
    @{ Body = $ultimateImpl; Name = "ServerUltimateAbility_Implementation"; RequireUltimate = "true"; Operation = "ValidateAbilityCooldown(Params)" },
    @{ Body = $ultimateValidate; Name = "ServerUltimateAbility_Validate"; RequireUltimate = "true"; Operation = "ValidateAbilityCooldown(Params)" }
  )) {
  $semanticCall = "ValidateAbilityInputSemantics(Params, $($bodyAndName.RequireUltimate))"
  $semanticIndex = $bodyAndName.Body.IndexOf($semanticCall)
  $operationIndex = $bodyAndName.Body.IndexOf($bodyAndName.Operation)
  Assert-True ($semanticIndex -ge 0) "Expected $($bodyAndName.Name) to call $semanticCall."
  Assert-True ($operationIndex -ge 0) "Expected $($bodyAndName.Name) operation marker."
  Assert-True ($semanticIndex -lt $operationIndex) "Expected $($bodyAndName.Name) to validate input semantics before cooldown/activation."
}

Assert-True ($predictionAbilityBody.Contains("AbilityInputID == static_cast<int32>(EDBAAbilityInputID::Ultimate)")) `
  "Expected client prediction to branch Ultimate input separately."
Assert-True ($predictionAbilityBody.Contains("RpcHandler->ServerUltimateAbility(Params);")) `
  "Expected client prediction to send Ultimate input through ServerUltimateAbility."
Assert-True ($predictionAbilityBody.Contains("RpcHandler->ServerTryActivateAbility(Params);")) `
  "Expected client prediction to keep normal skills on ServerTryActivateAbility."

Write-Host "PASS: RPC ability input semantic boundary" -ForegroundColor Green
