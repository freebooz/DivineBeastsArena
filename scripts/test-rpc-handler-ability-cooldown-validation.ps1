<#
Validates AbilityHandle-based RPC activation paths reuse the DBA AbilitySystem
InputID cooldown gate before server-side GAS activation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$rpcHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h"
$rpcCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"
$ascHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

$rpcHeader = Get-Content -LiteralPath $rpcHeaderPath -Encoding UTF8 -Raw
$rpcCpp = Get-Content -LiteralPath $rpcCppPath -Encoding UTF8 -Raw
$ascHeader = Get-Content -LiteralPath $ascHeaderPath -Encoding UTF8 -Raw
$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw

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

Assert-True ($ascHeader -match "bool\s+IsInputAbilityOnCooldown\s*\(\s*int32\s+InputID\s*\)\s+const") `
  "Expected DBA ASC to expose IsInputAbilityOnCooldown(int32 InputID) for RPC validation."
Assert-True ($ascCpp -match "bool\s+UDBAAbilitySystemComponent::IsInputAbilityOnCooldown\s*\(\s*int32\s+InputID\s*\)\s+const") `
  "Expected DBA ASC to implement IsInputAbilityOnCooldown."
Assert-True ($ascCpp -match "IsInputAbilityOnCooldown[\s\S]*MapAbilityInputIDToCooldownSkillSlot\(InputID\)") `
  "Expected ASC cooldown query to reuse InputID to SkillSlot mapping."
Assert-True ($ascCpp -match "IsInputAbilityOnCooldown[\s\S]*Character->IsAbilityOnCooldown\(SkillSpec\.SkillId\)") `
  "Expected ASC cooldown query to delegate to the character SkillId cooldown cache."

Assert-True ($rpcHeader.Contains("bool ValidateAbilityCooldown(const FDBAAbilityRpcParams& Params) const;")) `
  "Expected DBARpcHandler to declare a shared AbilityHandle cooldown validator."
Assert-True ($rpcCpp -match "#include\s+`"GameDBA/GAS/DBAAbilitySystemComponent\.h`"") `
  "Expected DBARpcHandler.cpp to include DBAAbilitySystemComponent."
Assert-True ($rpcCpp -match "bool\s+ADBARpcHandler::ValidateAbilityCooldown\s*\(\s*const\s+FDBAAbilityRpcParams&\s+Params\s*\)\s+const") `
  "Expected DBARpcHandler to implement ValidateAbilityCooldown."
Assert-True ($rpcCpp -match "ValidateAbilityCooldown[\s\S]*Cast<UDBAAbilitySystemComponent>\(CharacterRef->GetAbilitySystemComponent\(\)\)") `
  "Expected RPC cooldown validator to cast the character ASC to UDBAAbilitySystemComponent."
Assert-True ($rpcCpp -match "ValidateAbilityCooldown[\s\S]*FindAbilitySpecFromHandle\(Params\.AbilityHandle\)") `
  "Expected RPC cooldown validator to resolve the ability spec from the requested handle."
Assert-True ($rpcCpp -match "ValidateAbilityCooldown[\s\S]*IsInputAbilityOnCooldown\(Spec->InputID\)") `
  "Expected RPC cooldown validator to query the ASC InputID cooldown gate."

foreach ($bodyAndName in @(
    @{ Body = $tryActivateImpl; Name = "ServerTryActivateAbility_Implementation"; Operation = "TryActivateAbility(Params.AbilityHandle, false)" },
    @{ Body = $tryActivateValidate; Name = "ServerTryActivateAbility_Validate"; Operation = "return true;" },
    @{ Body = $ultimateImpl; Name = "ServerUltimateAbility_Implementation"; Operation = "TryActivateAbility(Params.AbilityHandle, false)" },
    @{ Body = $ultimateValidate; Name = "ServerUltimateAbility_Validate"; Operation = "return true;" }
  )) {
  $cooldownIndex = $bodyAndName.Body.IndexOf("ValidateAbilityCooldown(Params)")
  $operationIndex = $bodyAndName.Body.IndexOf($bodyAndName.Operation)
  Assert-True ($cooldownIndex -ge 0) "Expected $($bodyAndName.Name) to call ValidateAbilityCooldown(Params)."
  Assert-True ($operationIndex -ge 0) "Expected $($bodyAndName.Name) operation marker."
  Assert-True ($cooldownIndex -lt $operationIndex) "Expected $($bodyAndName.Name) to validate cooldown before $($bodyAndName.Operation)."
}

Write-Host "PASS: RPC handler ability cooldown validation contract" -ForegroundColor Green
