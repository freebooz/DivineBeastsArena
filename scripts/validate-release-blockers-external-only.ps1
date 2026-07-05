<#
Validates that current release blockers are blocked only by explicit external inputs.

Examples:
  .\scripts\validate-release-blockers-external-only.ps1
  .\scripts\validate-release-blockers-external-only.ps1 -ActionReportPath .\Artifacts\ProductionEvidence\release-blocker-actions.json -RequireValid
#>

[CmdletBinding()]
param(
    [string]$ActionReportPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-blocker-actions.json"),
    [string]$OutputJsonPath = "",
    [switch]$RequireValid
)

$ErrorActionPreference = "Stop"

function Resolve-DefaultPath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$ChildPath
    )

    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath((Join-Path $BasePath $ChildPath))
}

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    return [string]::Concat([char[]]$CodePoints)
}

$missingActionReportMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 25253, 21578, 19981, 23384, 22312, 65306, 123, 48, 125)
$invalidActionReportKindMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 25253, 21578, 31867, 22411, 19981, 27491, 30830, 65306, 123, 48, 125)
$jsonWrittenMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 22806, 37096, 36755, 20837, 26657, 39564, 32, 74, 83, 79, 78, 32, 24050, 20889, 20837, 65306, 123, 48, 125)
$validMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 22806, 37096, 36755, 20837, 26657, 39564, 65306, 26377, 25928, 65288, 22806, 37096, 38459, 22622, 39033, 61, 123, 48, 125, 65289)
$invalidMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 22806, 37096, 36755, 20837, 26657, 39564, 65306, 26080, 25928, 65288, 26412, 22320, 33258, 21160, 21270, 38459, 22622, 39033, 61, 123, 48, 125, 65292, 31354, 22806, 37096, 36755, 20837, 38459, 22622, 39033, 61, 123, 49, 125, 65292, 37325, 22797, 38459, 22622, 38190, 61, 123, 50, 125, 65289)
$requireValidFailureMessage = New-TextFromCodePoints @(21457, 24067, 38459, 22622, 39033, 19981, 20840, 26159, 22806, 37096, 36755, 20837, 65307, 35831, 26816, 26597, 32, 123, 48, 125, 12290)

$resolvedActionReportPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ActionReportPath)
if (-not (Test-Path -LiteralPath $resolvedActionReportPath)) {
    throw ($missingActionReportMessage -f $resolvedActionReportPath)
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedActionReportPath) -ChildPath "release-blockers-external-only-validation.json"
}

$actionReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedActionReportPath | ConvertFrom-Json
if ([string]$actionReport.kind -ne "release-blocker-actions") {
    throw ($invalidActionReportKindMessage -f $resolvedActionReportPath)
}

$localAutomationBlockers = New-Object System.Collections.Generic.List[string]
$emptyExternalInputBlockers = New-Object System.Collections.Generic.List[string]
$externalBlockers = New-Object System.Collections.Generic.List[string]
$seenActionKeys = @{}
$duplicateActionKeys = New-Object System.Collections.Generic.List[string]
$reportedBlockerCount = [int]$actionReport.blockerCount
$actualBlockerCount = @($actionReport.actions).Count
$blockerCountMatchesActions = $reportedBlockerCount -eq $actualBlockerCount

foreach ($action in @($actionReport.actions)) {
    $key = [string]$action.key
    $blockingInputs = @($action.blockingExternalInputs | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) })

    if (-not [string]::IsNullOrWhiteSpace($key)) {
        if ($seenActionKeys.ContainsKey($key)) {
            if (-not $duplicateActionKeys.Contains($key)) {
                $duplicateActionKeys.Add($key)
            }
        }
        else {
            $seenActionKeys[$key] = $true
        }
    }

    if ($action.automationBlocked -ne $true) {
        $localAutomationBlockers.Add($key)
        continue
    }

    if ($blockingInputs.Count -eq 0) {
        $emptyExternalInputBlockers.Add($key)
        continue
    }

    $externalBlockers.Add($key)
}

$externalOnly = ($localAutomationBlockers.Count -eq 0 -and $emptyExternalInputBlockers.Count -eq 0 -and $duplicateActionKeys.Count -eq 0 -and $blockerCountMatchesActions)

$validation = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blockers-external-only-validation"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    actionReportPath = $resolvedActionReportPath
    releaseId = $actionReport.releaseId
    blockerCount = $actualBlockerCount
    reportedBlockerCount = $reportedBlockerCount
    blockerCountMatchesActions = $blockerCountMatchesActions
    externalOnly = $externalOnly
    externalBlockerCount = $externalBlockers.Count
    externalBlockers = $externalBlockers.ToArray()
    localAutomationBlockerCount = $localAutomationBlockers.Count
    localAutomationBlockers = $localAutomationBlockers.ToArray()
    emptyExternalInputBlockerCount = $emptyExternalInputBlockers.Count
    emptyExternalInputBlockers = $emptyExternalInputBlockers.ToArray()
    duplicateActionKeyCount = $duplicateActionKeys.Count
    duplicateActionKeys = $duplicateActionKeys.ToArray()
}

$outputDirectory = Split-Path -Parent $OutputJsonPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$validation | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

Write-Host ($jsonWrittenMessage -f $OutputJsonPath)
if ($externalOnly) {
    Write-Host ($validMessage -f $externalBlockers.Count) -ForegroundColor Green
}
else {
    Write-Host ($invalidMessage -f $localAutomationBlockers.Count, $emptyExternalInputBlockers.Count, $duplicateActionKeys.Count) -ForegroundColor Yellow
}

if ($RequireValid -and -not $externalOnly) {
    throw ($requireValidFailureMessage -f $OutputJsonPath)
}
