<#
Validates DBARpcHandler public server RPC wrappers run their Validate function before Implementation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h"
$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Assert-WrapperValidates {
  param(
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$ValidateCall,
    [Parameter(Mandatory = $true)][string]$ImplementationCall
  )

  $startPattern = "virtual\s+void\s+$FunctionName\s*\("
  $match = [regex]::Match($header, $startPattern)
  Assert-True $match.Success "Expected $FunctionName wrapper body."

  $remaining = $header.Substring($match.Index)
  $endMatch = [regex]::Match($remaining.Substring($match.Length), "\n\tvirtual\s+void|\n\t// ====================")
  Assert-True $endMatch.Success "Expected end boundary after $FunctionName wrapper."

  $body = $remaining.Substring(0, $match.Length + $endMatch.Index)
  $validateIndex = $body.IndexOf($ValidateCall)
  $implementationIndex = $body.IndexOf($ImplementationCall)

  Assert-True ($validateIndex -ge 0) "Expected $FunctionName wrapper to call $ValidateCall before implementation."
  Assert-True ($implementationIndex -gt $validateIndex) "Expected $FunctionName wrapper to call $ImplementationCall only after validation."
}

Assert-WrapperValidates "ServerTryActivateAbility" "ServerTryActivateAbility_Validate(Params)" "ServerTryActivateAbility_Implementation(Params)"
Assert-WrapperValidates "ServerCancelAbility" "ServerCancelAbility_Validate(Handle)" "ServerCancelAbility_Implementation(Handle)"
Assert-WrapperValidates "ServerLockTarget" "ServerLockTarget_Validate(TargetActor)" "ServerLockTarget_Implementation(TargetActor)"
Assert-WrapperValidates "ServerMoveTo" "ServerMoveTo_Validate(Location)" "ServerMoveTo_Implementation(Location)"
Assert-WrapperValidates "ServerRequestAttack" "ServerRequestAttack_Validate()" "ServerRequestAttack_Implementation()"
Assert-WrapperValidates "ServerUltimateAbility" "ServerUltimateAbility_Validate(Params)" "ServerUltimateAbility_Implementation(Params)"

Write-Host "PASS: RPC handler wrapper validation contract" -ForegroundColor Green
