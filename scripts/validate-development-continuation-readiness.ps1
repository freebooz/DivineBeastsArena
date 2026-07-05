<#
Validates whether development-stage automation may continue.

This is intentionally distinct from production release readiness. A report may
have readyForRelease=false while still allowing development continuation when
all remaining blockers are validated as external release-input gaps.

Examples:
  .\scripts\validate-development-continuation-readiness.ps1
  .\scripts\validate-development-continuation-readiness.ps1 -RequireReady
#>

[CmdletBinding()]
param(
    [string]$ReadinessReportPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-readiness-report.json"),
    [string]$OutputJsonPath = "",
    [switch]$RequireReady
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

$missingReportMessage = New-TextFromCodePoints @(21457, 24067, 23601, 32490, 25253, 21578, 19981, 23384, 22312, 65306, 123, 48, 125)
$invalidReportKindMessage = New-TextFromCodePoints @(21457, 24067, 23601, 32490, 25253, 21578, 31867, 22411, 19981, 27491, 30830, 65306, 123, 48, 125)
$jsonWrittenMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 32, 74, 83, 79, 78, 32, 24050, 20889, 20837, 65306, 123, 48, 125)
$readyMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 65306, 21487, 32487, 32493, 65288, 21457, 24067, 24050, 23601, 32490, 61, 123, 48, 125, 65292, 22806, 37096, 38459, 22622, 39033, 61, 123, 49, 125, 65289)
$blockedMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 65306, 38459, 22622, 65288, 26412, 22320, 33258, 21160, 21270, 38459, 22622, 39033, 61, 123, 48, 125, 65292, 31354, 22806, 37096, 36755, 20837, 38459, 22622, 39033, 61, 123, 49, 125, 65289)
$blockedRequireReadyMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 34987, 38459, 22622, 65307, 35831, 26816, 26597, 32, 123, 48, 125, 12290)

$resolvedReadinessReportPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ReadinessReportPath)
if (-not (Test-Path -LiteralPath $resolvedReadinessReportPath)) {
    throw ($missingReportMessage -f $resolvedReadinessReportPath)
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedReadinessReportPath) -ChildPath "development-continuation-readiness-validation.json"
}

$readinessReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedReadinessReportPath | ConvertFrom-Json
if ([string]$readinessReport.kind -ne "release-readiness-report") {
    throw ($invalidReportKindMessage -f $resolvedReadinessReportPath)
}

$posture = $readinessReport.releaseBlockerPosture
$externalOnly = $false
$externalBlockerCount = 0
$postureBlockerCount = 0
$localAutomationBlockerCount = 0
$emptyExternalInputBlockerCount = 0
$postureKind = ""
$postureKindIsValid = $false
$postureReleaseId = ""
$postureReleaseIdMatchesReport = $false
$postureBlockerCountMatchesBlockingRequirements = $false
if ($null -ne $posture) {
    $postureKind = [string]$posture.kind
    $postureKindIsValid = $postureKind -eq "release-blockers-external-only-validation"
    $postureReleaseId = [string]$posture.releaseId
    $externalOnly = [bool]$posture.externalOnly
    $postureBlockerCount = [int]$posture.blockerCount
    $externalBlockerCount = [int]$posture.externalBlockerCount
    $localAutomationBlockerCount = [int]$posture.localAutomationBlockerCount
    $emptyExternalInputBlockerCount = [int]$posture.emptyExternalInputBlockerCount
}

$reportReleaseId = [string]$readinessReport.releaseId
$manifestReleaseId = ""
$reportMatchesManifest = $true
$reportManifestPath = [string]$readinessReport.manifestPath
if (-not [string]::IsNullOrWhiteSpace($reportManifestPath) -and (Test-Path -LiteralPath $reportManifestPath)) {
    $reportManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $reportManifestPath | ConvertFrom-Json
    $manifestReleaseId = [string]$reportManifest.releaseId
    $reportMatchesManifest = -not [string]::IsNullOrWhiteSpace($manifestReleaseId) -and $reportReleaseId -eq $manifestReleaseId
}

$readyForRelease = [bool]$readinessReport.readyForRelease
$reportedDevelopmentContinuationReady = [bool]$readinessReport.developmentContinuationReady
$blockingRequirementCount = [int]$readinessReport.blockingRequirementCount
$postureReleaseIdMatchesReport = -not [string]::IsNullOrWhiteSpace($postureReleaseId) -and $postureReleaseId -eq $reportReleaseId
$postureBlockerCountMatchesBlockingRequirements = $postureBlockerCount -eq $blockingRequirementCount -and $externalBlockerCount -eq $blockingRequirementCount
$computedDevelopmentContinuationReady = $readyForRelease -or (
    $postureKindIsValid -and
    $postureReleaseIdMatchesReport -and
    $postureBlockerCountMatchesBlockingRequirements -and
    $externalOnly -and
    $localAutomationBlockerCount -eq 0 -and
    $emptyExternalInputBlockerCount -eq 0
)
$developmentContinuationReady = $computedDevelopmentContinuationReady -and $reportMatchesManifest

$validation = [ordered]@{
    schemaVersion = "1.0"
    kind = "development-continuation-readiness-validation"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    readinessReportPath = $resolvedReadinessReportPath
    releaseId = $reportReleaseId
    manifestReleaseId = $manifestReleaseId
    reportMatchesManifest = $reportMatchesManifest
    readyForRelease = $readyForRelease
    reportedDevelopmentContinuationReady = $reportedDevelopmentContinuationReady
    computedDevelopmentContinuationReady = $computedDevelopmentContinuationReady
    developmentContinuationReady = $developmentContinuationReady
    blockingRequirementCount = $blockingRequirementCount
    postureKind = $postureKind
    postureKindIsValid = $postureKindIsValid
    postureReleaseId = $postureReleaseId
    postureReleaseIdMatchesReport = $postureReleaseIdMatchesReport
    postureBlockerCount = $postureBlockerCount
    postureBlockerCountMatchesBlockingRequirements = $postureBlockerCountMatchesBlockingRequirements
    externalOnly = $externalOnly
    externalBlockerCount = $externalBlockerCount
    localAutomationBlockerCount = $localAutomationBlockerCount
    emptyExternalInputBlockerCount = $emptyExternalInputBlockerCount
}

$outputDirectory = Split-Path -Parent $OutputJsonPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$validation | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

Write-Host ($jsonWrittenMessage -f $OutputJsonPath)
if ($developmentContinuationReady) {
    Write-Host ($readyMessage -f $readyForRelease, $externalBlockerCount) -ForegroundColor Green
}
else {
    Write-Host ($blockedMessage -f $localAutomationBlockerCount, $emptyExternalInputBlockerCount) -ForegroundColor Yellow
}

if ($RequireReady -and -not $developmentContinuationReady) {
    throw ($blockedRequireReadyMessage -f $OutputJsonPath)
}
