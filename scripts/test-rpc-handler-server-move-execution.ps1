<#
Validates ServerMoveTo performs an authoritative server-side movement update
and sends a correction back to the owning client.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"
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

$moveToImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerMoveTo_Implementation\s*\(\s*FVector_NetQuantize10\s+Location\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerMoveTo_Validate" `
  "ServerMoveTo_Implementation"

$moveToValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerMoveTo_Validate\s*\(\s*FVector_NetQuantize10\s+Location\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerRequestAttack_Implementation" `
  "ServerMoveTo_Validate"

foreach ($required in @(
    "ValidateServerCharacterContext(",
    "UWorld* World = GetWorld()",
    "AActor* OwnerActor = GetOwner()",
    "OwnerActor->SetActorLocation(Location)",
    "World->GetTimeSeconds()",
    "ClientMoveCorrection_Implementation(Location, ServerTime)"
  )) {
  Assert-True ($moveToImpl.Contains($required)) "Expected ServerMoveTo_Implementation to contain: $required"
}

$guardIndex = $moveToImpl.IndexOf("ValidateServerCharacterContext(")
$setLocationIndex = $moveToImpl.IndexOf("OwnerActor->SetActorLocation(Location)")
$correctionIndex = $moveToImpl.IndexOf("ClientMoveCorrection_Implementation(Location, ServerTime)")

Assert-True ($guardIndex -ge 0 -and $setLocationIndex -gt $guardIndex) `
  "Expected ServerMoveTo_Implementation to update location only after character context validation."
Assert-True ($correctionIndex -gt $setLocationIndex) `
  "Expected ServerMoveTo_Implementation to send client correction after updating server location."

Assert-True ($moveToValidate.Contains("UWorld* World = GetWorld()")) `
  "Expected ServerMoveTo_Validate to require a valid World for boundary validation."
Assert-True ($moveToValidate.Contains("if (!World)")) `
  "Expected ServerMoveTo_Validate to fail closed when World is missing."
Assert-True ($moveToValidate.Contains("return false;")) `
  "Expected ServerMoveTo_Validate to reject missing World."

Write-Host "PASS: RPC handler server move execution contract" -ForegroundColor Green
