<#
Validates DBALobbyPlayerController binds client input only for local controllers.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBALobbyPlayerController.cpp"
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

function Assert-LocalGuardBeforeBinding {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$BindingCall
  )

  $guardIndex = $Body.IndexOf("!IsLocalController()")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $bindingIndex = $Body.IndexOf($BindingCall)

  Assert-True ($guardIndex -ge 0) "Expected SetupInputComponent to reject non-local controllers before $BindingCall."
  Assert-True ($returnIndex -gt $guardIndex) "Expected SetupInputComponent non-local guard to return before $BindingCall."
  Assert-True ($bindingIndex -gt $returnIndex) "Expected SetupInputComponent to run $BindingCall only after local-controller guard."
}

$setupBody = Get-FunctionBody `
  "void\s+ADBALobbyPlayerController::SetupInputComponent\s*\(\s*\)\s*\{" `
  "`nvoid\s+ADBALobbyPlayerController::PlayerTick" `
  "SetupInputComponent"

Assert-LocalGuardBeforeBinding $setupBody "InputComponent->BindAxis("
Assert-LocalGuardBeforeBinding $setupBody "InputComponent->BindAction("
Assert-LocalGuardBeforeBinding $setupBody "InputComponent->BindKey("

Assert-True ($setupBody.Contains("InputComponent->BindAction(TEXT(""Skill01"")")) `
  "Expected SetupInputComponent contract to cover skill action bindings."
Assert-True ($setupBody.Contains("InputComponent->BindKey(EKeys::LeftMouseButton")) `
  "Expected SetupInputComponent contract to cover mouse button bindings."

Write-Host "PASS: Lobby PlayerController local input binding contract" -ForegroundColor Green
