<#
Validates Android touch input bridge does not broadcast UI/GAS input events from unsafe/server-like runtimes.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Input\DBAAndroidTouchInputBridge.cpp"
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

function Assert-GuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $guardIndex = $Body.IndexOf("IsTouchInputRuntimeAllowed(")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to call IsTouchInputRuntimeAllowed before $Operation."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName runtime guard to return before $Operation."
  Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after runtime guard."
}

function Assert-BeginPlayRuntimeDisable {
  param(
    [Parameter(Mandatory = $true)][string]$Body
  )

  $guardIndex = $Body.IndexOf("IsTouchInputRuntimeAllowed(")
  $disableIndex = $Body.IndexOf("SetComponentTickEnabled(false);", [Math]::Max(0, $guardIndex))
  $deactivateIndex = $Body.IndexOf("Deactivate();", [Math]::Max(0, $guardIndex))
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))

  Assert-True ($guardIndex -ge 0) "Expected BeginPlay to call IsTouchInputRuntimeAllowed before component activation decisions."
  Assert-True ($disableIndex -gt $guardIndex) "Expected BeginPlay to disable the component after runtime guard fails."
  Assert-True ($deactivateIndex -gt $guardIndex) "Expected BeginPlay to deactivate the component after runtime guard fails."
  Assert-True ($returnIndex -gt $deactivateIndex) "Expected BeginPlay to return after disabling and deactivating the component."
}

Assert-True ($cpp.Contains("IsTouchInputRuntimeAllowed")) "Expected a shared Android touch input runtime guard."
Assert-True ($cpp.Contains("GetWorld()")) "Expected runtime guard to inspect World."
Assert-True ($cpp.Contains("NM_DedicatedServer")) "Expected runtime guard to reject Dedicated Server runtime."

$beginPlayBody = Get-FunctionBody `
  "void\s+UDBAAndroidTouchInputBridge::BeginPlay\s*\(\s*\)\s*\{" `
  "`nvoid\s+UDBAAndroidTouchInputBridge::OnSkillButtonLongPressStart" `
  "BeginPlay"
Assert-BeginPlayRuntimeDisable $beginPlayBody

$longPressBody = Get-FunctionBody `
  "void\s+UDBAAndroidTouchInputBridge::OnSkillButtonLongPressStart\s*\(\s*int32\s+SkillIndex,\s*FVector2D\s+TouchLocation\s*\)\s*\{" `
  "`nvoid\s+UDBAAndroidTouchInputBridge::OnSkillButtonDrag" `
  "OnSkillButtonLongPressStart"
Assert-GuardBeforeOperation $longPressBody "OnSkillButtonLongPressStart" "CurrentLongPressSkillIndex = SkillIndex;"
Assert-GuardBeforeOperation $longPressBody "OnSkillButtonLongPressStart" "OnSkillWheelShowEvent.Broadcast(SkillIndex);"

$dragBody = Get-FunctionBody `
  "void\s+UDBAAndroidTouchInputBridge::OnSkillButtonDrag\s*\(\s*int32\s+SkillIndex,\s*FVector2D\s+DragDelta\s*\)\s*\{" `
  "`nvoid\s+UDBAAndroidTouchInputBridge::OnSkillButtonRelease" `
  "OnSkillButtonDrag"
Assert-GuardBeforeOperation $dragBody "OnSkillButtonDrag" "bIsDragging = true;"
Assert-GuardBeforeOperation $dragBody "OnSkillButtonDrag" "OnSkillDirectionUpdateEvent.Broadcast(CurrentSkillDirection);"

$releaseBody = Get-FunctionBody `
  "void\s+UDBAAndroidTouchInputBridge::OnSkillButtonRelease\s*\(\s*int32\s+SkillIndex\s*\)\s*\{" `
  "`nvoid\s+UDBAAndroidTouchInputBridge::UpdateUltimateButtonState" `
  "OnSkillButtonRelease"
Assert-GuardBeforeOperation $releaseBody "OnSkillButtonRelease" "OnSkillReleasedEvent.Broadcast(SkillIndex);"
Assert-GuardBeforeOperation $releaseBody "OnSkillButtonRelease" "OnSkillWheelHideEvent.Broadcast();"

$ultimateBody = Get-FunctionBody `
  "void\s+UDBAAndroidTouchInputBridge::UpdateUltimateButtonState\s*\(\s*float\s+UltimateEnergy,\s*bool\s+bIsReady\s*\)\s*\{" `
  "\z" `
  "UpdateUltimateButtonState"
Assert-GuardBeforeOperation $ultimateBody "UpdateUltimateButtonState" "static_cast<void>(UltimateEnergy);"

Write-Host "PASS: Android touch input bridge server boundary" -ForegroundColor Green
