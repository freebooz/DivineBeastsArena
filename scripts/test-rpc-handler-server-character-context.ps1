<#
Validates DBARpcHandler server-side RPC paths require a valid character context.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h"
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"

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

function Assert-ContextGuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $guardIndex = $Body.IndexOf("ValidateServerCharacterContext(")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to validate server character context before $Operation."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName context guard to return before $Operation."
  Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after context guard."
}

function Assert-ValidateUsesContext {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $guardIndex = $Body.IndexOf("ValidateServerCharacterContext(")
  $trueIndex = $Body.LastIndexOf("return true")

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to reject missing server character context."
  Assert-True ($trueIndex -gt $guardIndex) "Expected $FunctionName to reach success only after server character context validation."
}

Assert-True ($header -match "ValidateServerCharacterContext\s*\(\s*const\s+TCHAR\*\s+OperationName\s*\)\s*const") `
  "Expected DBARpcHandler to declare a reusable server character context validator."

$tryActivateImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerTryActivateAbility_Implementation\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerTryActivateAbility_Validate" `
  "ServerTryActivateAbility_Implementation"
$tryActivateValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerTryActivateAbility_Validate\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerCancelAbility_Implementation" `
  "ServerTryActivateAbility_Validate"

$cancelImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerCancelAbility_Implementation\s*\(\s*FGameplayAbilitySpecHandle\s+Handle\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerCancelAbility_Validate" `
  "ServerCancelAbility_Implementation"
$cancelValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerCancelAbility_Validate\s*\(\s*FGameplayAbilitySpecHandle\s+Handle\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerLockTarget_Implementation" `
  "ServerCancelAbility_Validate"

$lockTargetImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerLockTarget_Implementation\s*\(\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerLockTarget_Validate" `
  "ServerLockTarget_Implementation"
$lockTargetValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerLockTarget_Validate\s*\(\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerMoveTo_Implementation" `
  "ServerLockTarget_Validate"

$moveToImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerMoveTo_Implementation\s*\(\s*FVector_NetQuantize10\s+Location\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerMoveTo_Validate" `
  "ServerMoveTo_Implementation"
$moveToValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerMoveTo_Validate\s*\(\s*FVector_NetQuantize10\s+Location\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerRequestAttack_Implementation" `
  "ServerMoveTo_Validate"

$requestAttackImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerRequestAttack_Implementation\s*\(\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerRequestAttack_Validate" `
  "ServerRequestAttack_Implementation"
$requestAttackValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerRequestAttack_Validate\s*\(\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerUltimateAbility_Implementation" `
  "ServerRequestAttack_Validate"

$ultimateImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerUltimateAbility_Implementation\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerUltimateAbility_Validate" `
  "ServerUltimateAbility_Implementation"
$ultimateValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerUltimateAbility_Validate\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s*\{" `
  "`n// ==================== IDBARpcClient" `
  "ServerUltimateAbility_Validate"

Assert-ContextGuardBeforeOperation $tryActivateImpl "ServerTryActivateAbility_Implementation" "FindAbilitySpecFromHandle"
Assert-ContextGuardBeforeOperation $cancelImpl "ServerCancelAbility_Implementation" "CancelAbilityHandle"
Assert-ContextGuardBeforeOperation $lockTargetImpl "ServerLockTarget_Implementation" "IsEnemy"
Assert-ContextGuardBeforeOperation $moveToImpl "ServerMoveTo_Implementation" "UE_LOG"
Assert-ContextGuardBeforeOperation $requestAttackImpl "ServerRequestAttack_Implementation" "FindAttackTarget()"
Assert-ContextGuardBeforeOperation $ultimateImpl "ServerUltimateAbility_Implementation" "GetUltimateEnergy()"

Assert-ValidateUsesContext $tryActivateValidate "ServerTryActivateAbility_Validate"
Assert-ValidateUsesContext $cancelValidate "ServerCancelAbility_Validate"
Assert-ValidateUsesContext $lockTargetValidate "ServerLockTarget_Validate"
Assert-ValidateUsesContext $moveToValidate "ServerMoveTo_Validate"
Assert-ValidateUsesContext $requestAttackValidate "ServerRequestAttack_Validate"
Assert-ValidateUsesContext $ultimateValidate "ServerUltimateAbility_Validate"

$energyCostBody = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ValidateEnergyCost\s*\(\s*float\s+Cost\s*\)\s+const\s*\{" `
  "`nbool\s+ADBARpcHandler::ValidateTarget" `
  "ValidateEnergyCost"
$serverContextBody = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ValidateServerCharacterContext\s*\(\s*const\s+TCHAR\*\s+OperationName\s*\)\s+const\s*\{" `
  "`nbool\s+ADBARpcHandler::ValidateEnergyCost" `
  "ValidateServerCharacterContext"

Assert-True ($energyCostBody -match "return\s+false\s*;") "Expected ValidateEnergyCost to fail closed when CharacterRef is missing."
Assert-True ($serverContextBody.Contains("GetCharacterRef()")) "Expected context validator to read CharacterRef."
Assert-True ($serverContextBody.Contains("IsDead()")) "Expected context validator to reject dead characters."
Assert-True ($serverContextBody.Contains("GetAbilitySystemComponent()")) "Expected context validator to require ASC for server ability/attack RPCs."

Write-Host "PASS: RPC handler server character context contract" -ForegroundColor Green
