<#
Exercises validate-development-continuation-readiness.ps1 against release
readiness fixtures.

The test proves development-stage automation can continue when production
release is blocked only by external release inputs, while still failing when a
local automation blocker remains.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\development-continuation-readiness-tests-{0}" -f [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $testRoot | Out-Null

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Assert-Fails {
    param(
        [Parameter(Mandatory = $true)][scriptblock]$Script,
        [Parameter(Mandatory = $true)][string]$Message
    )

    $failed = $false
    try {
        & $Script
    }
    catch {
        $failed = $true
    }

    if (-not $failed) {
        throw $Message
    }
}

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    return [string]::Concat([char[]]$CodePoints)
}

function Write-ReadinessFixture {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$ReleaseId,
        [Parameter(Mandatory = $true)][bool]$ReadyForRelease,
        [Parameter(Mandatory = $true)][bool]$DevelopmentContinuationReady,
        [Parameter(Mandatory = $true)][int]$BlockingRequirementCount,
        [Parameter(Mandatory = $true)][bool]$ExternalOnly,
        [Parameter(Mandatory = $true)][int]$ExternalBlockerCount,
        [Parameter(Mandatory = $true)][int]$LocalAutomationBlockerCount,
        [string]$ManifestPath = "",
        [string]$PostureReleaseId = "",
        [int]$PostureBlockerCount = -1
    )

    if ([string]::IsNullOrWhiteSpace($PostureReleaseId)) {
        $PostureReleaseId = $ReleaseId
    }
    if ($PostureBlockerCount -lt 0) {
        $PostureBlockerCount = $BlockingRequirementCount
    }

    $fixture = [ordered]@{
        schemaVersion = "1.0"
        kind = "release-readiness-report"
        releaseId = $ReleaseId
        readyForRelease = $ReadyForRelease
        developmentContinuationReady = $DevelopmentContinuationReady
        blockingRequirementCount = $BlockingRequirementCount
        releaseBlockerPosture = [ordered]@{
            kind = "release-blockers-external-only-validation"
            releaseId = $PostureReleaseId
            blockerCount = $PostureBlockerCount
            externalOnly = $ExternalOnly
            externalBlockerCount = $ExternalBlockerCount
            localAutomationBlockerCount = $LocalAutomationBlockerCount
            emptyExternalInputBlockerCount = 0
        }
    }

    if (-not [string]::IsNullOrWhiteSpace($ManifestPath)) {
        $fixture.manifestPath = $ManifestPath
    }

    $fixture | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $Path -Encoding UTF8
}

$validatorPath = Join-Path $repoRoot "scripts\validate-development-continuation-readiness.ps1"
$validatorSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $validatorPath
$jsonWrittenMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 32, 74, 83, 79, 78, 32, 24050, 20889, 20837)
$readyMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 65306, 21487, 32487, 32493)
$blockedMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 65306, 38459, 22622)
$blockedRequireReadyMessage = New-TextFromCodePoints @(32487, 32493, 24320, 21457, 34987, 38459, 22622, 65307, 35831, 26816, 26597)
$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 32487, 32493, 24320, 21457, 23601, 32490, 29366, 24577, 22865, 32422)
$expectedJsonMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 32, 74, 83, 79, 78, 32, 20889, 20837, 28040, 24687, 12290)
$expectedReadyMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 21487, 32487, 32493, 28040, 24687, 12290)
$expectedBlockedMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 38459, 22622, 28040, 24687, 12290)
$expectedBlockedRequireReadyContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 32, 82, 101, 113, 117, 105, 114, 101, 82, 101, 97, 100, 121, 32, 22833, 36133, 28040, 24687, 12290)
$forbiddenOldJsonMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 32, 74, 83, 79, 78, 32, 20889, 20837, 28040, 24687, 12290)
$forbiddenOldReadyMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 21487, 32487, 32493, 28040, 24687, 12290)
$forbiddenOldBlockedMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 38459, 22622, 28040, 24687, 12290)
$forbiddenOldRequireReadyMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 32, 82, 101, 113, 117, 105, 114, 101, 82, 101, 97, 100, 121, 32, 22833, 36133, 28040, 24687, 12290)

Assert-True ($validatorSource.Contains('$jsonWrittenMessage')) $expectedJsonMessageContract
Assert-True ($validatorSource.Contains('$readyMessage')) $expectedReadyMessageContract
Assert-True ($validatorSource.Contains('$blockedMessage')) $expectedBlockedMessageContract
Assert-True ($validatorSource.Contains('$blockedRequireReadyMessage')) $expectedBlockedRequireReadyContract
Assert-True (-not $validatorSource.Contains("Development continuation readiness JSON written")) $forbiddenOldJsonMessageContract
Assert-True (-not $validatorSource.Contains("Development continuation readiness: ready")) $forbiddenOldReadyMessageContract
Assert-True (-not $validatorSource.Contains("Development continuation readiness: blocked")) $forbiddenOldBlockedMessageContract
Assert-True (-not $validatorSource.Contains("Development continuation is blocked")) $forbiddenOldRequireReadyMessageContract

$readyPath = Join-Path $testRoot "release-ready.json"
$developmentReadyPath = Join-Path $testRoot "development-ready.json"
$blockedPath = Join-Path $testRoot "local-automation-blocked.json"
$inconsistentReadyPath = Join-Path $testRoot "inconsistent-development-ready.json"
$staleReportManifestPath = Join-Path $testRoot "current-production-evidence-manifest.json"
$staleReportPath = Join-Path $testRoot "stale-development-ready.json"
$stalePosturePath = Join-Path $testRoot "stale-posture-development-ready.json"
$postureCountMismatchPath = Join-Path $testRoot "posture-count-mismatch-development-ready.json"

Write-ReadinessFixture `
    -Path $readyPath `
    -ReleaseId "fixture-ready" `
    -ReadyForRelease $true `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 0 `
    -ExternalOnly $true `
    -ExternalBlockerCount 0 `
    -LocalAutomationBlockerCount 0

Write-ReadinessFixture `
    -Path $developmentReadyPath `
    -ReleaseId "fixture-development-ready" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 2 `
    -ExternalOnly $true `
    -ExternalBlockerCount 2 `
    -LocalAutomationBlockerCount 0

Write-ReadinessFixture `
    -Path $blockedPath `
    -ReleaseId "fixture-local-automation-blocked" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $false `
    -BlockingRequirementCount 1 `
    -ExternalOnly $false `
    -ExternalBlockerCount 0 `
    -LocalAutomationBlockerCount 1

Write-ReadinessFixture `
    -Path $inconsistentReadyPath `
    -ReleaseId "fixture-inconsistent-development-ready" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 2 `
    -ExternalOnly $false `
    -ExternalBlockerCount 1 `
    -LocalAutomationBlockerCount 1

[ordered]@{
    schemaVersion = "1.0"
    kind = "production-evidence-manifest"
    releaseId = "fixture-current-manifest"
    requirements = @()
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $staleReportManifestPath -Encoding UTF8

Write-ReadinessFixture `
    -Path $staleReportPath `
    -ReleaseId "fixture-stale-report" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 2 `
    -ExternalOnly $true `
    -ExternalBlockerCount 2 `
    -LocalAutomationBlockerCount 0 `
    -ManifestPath $staleReportManifestPath

Write-ReadinessFixture `
    -Path $stalePosturePath `
    -ReleaseId "fixture-stale-posture" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 2 `
    -ExternalOnly $true `
    -ExternalBlockerCount 2 `
    -LocalAutomationBlockerCount 0 `
    -PostureReleaseId "different-posture-release"

Write-ReadinessFixture `
    -Path $postureCountMismatchPath `
    -ReleaseId "fixture-posture-count-mismatch" `
    -ReadyForRelease $false `
    -DevelopmentContinuationReady $true `
    -BlockingRequirementCount 2 `
    -ExternalOnly $true `
    -ExternalBlockerCount 2 `
    -LocalAutomationBlockerCount 0 `
    -PostureBlockerCount 1

& $validatorPath `
    -ReadinessReportPath $readyPath `
    -OutputJsonPath (Join-Path $testRoot "release-ready-validation.json") `
    -RequireReady

$releaseReadyValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "release-ready-validation.json") | ConvertFrom-Json
Assert-True ($releaseReadyValidation.kind -eq "development-continuation-readiness-validation") "Expected validation kind."
Assert-True ($releaseReadyValidation.developmentContinuationReady -eq $true) "Expected release-ready fixture to pass development continuation."
Assert-True ($releaseReadyValidation.readyForRelease -eq $true) "Expected release-ready fixture to preserve release readiness."

& $validatorPath `
    -ReadinessReportPath $developmentReadyPath `
    -OutputJsonPath (Join-Path $testRoot "development-ready-validation.json") `
    -RequireReady

$developmentReadyValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "development-ready-validation.json") | ConvertFrom-Json
Assert-True ($developmentReadyValidation.developmentContinuationReady -eq $true) "Expected external-only fixture to pass development continuation."
Assert-True ($developmentReadyValidation.readyForRelease -eq $false) "Expected external-only fixture to remain not release-ready."
Assert-True ($developmentReadyValidation.externalOnly -eq $true) "Expected external-only posture."
Assert-True ($developmentReadyValidation.localAutomationBlockerCount -eq 0) "Expected no local automation blockers."

Assert-Fails -Message "Expected local automation blocker fixture to fail under -RequireReady." -Script {
    & $validatorPath `
        -ReadinessReportPath $blockedPath `
        -OutputJsonPath (Join-Path $testRoot "blocked-validation.json") `
        -RequireReady
}

$blockedValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "blocked-validation.json") | ConvertFrom-Json
Assert-True ($blockedValidation.developmentContinuationReady -eq $false) "Expected blocked fixture to fail development continuation."
Assert-True ($blockedValidation.localAutomationBlockerCount -eq 1) "Expected local automation blocker count to be preserved."

Assert-Fails -Message "Expected inconsistent development-ready fixture to fail under -RequireReady." -Script {
    & $validatorPath `
        -ReadinessReportPath $inconsistentReadyPath `
        -OutputJsonPath (Join-Path $testRoot "inconsistent-development-ready-validation.json") `
        -RequireReady
}

$inconsistentReadyValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "inconsistent-development-ready-validation.json") | ConvertFrom-Json
Assert-True ($inconsistentReadyValidation.developmentContinuationReady -eq $false) "Expected validator to recompute and reject inconsistent development continuation readiness."
Assert-True ($inconsistentReadyValidation.reportedDevelopmentContinuationReady -eq $true) "Expected validation to preserve the originally reported readiness value for diagnostics."
Assert-True ($inconsistentReadyValidation.localAutomationBlockerCount -eq 1) "Expected inconsistent local automation blocker count to be preserved."

Assert-Fails -Message "Expected stale development-ready report to fail under -RequireReady when manifest releaseId differs." -Script {
    & $validatorPath `
        -ReadinessReportPath $staleReportPath `
        -OutputJsonPath (Join-Path $testRoot "stale-development-ready-validation.json") `
        -RequireReady
}

$staleValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "stale-development-ready-validation.json") | ConvertFrom-Json
Assert-True ($staleValidation.developmentContinuationReady -eq $false) "Expected stale report to fail development continuation readiness."
Assert-True ($staleValidation.reportMatchesManifest -eq $false) "Expected stale report validation to record manifest mismatch."
Assert-True ($staleValidation.manifestReleaseId -eq "fixture-current-manifest") "Expected current manifest releaseId to be preserved for diagnostics."

Assert-Fails -Message "Expected stale release blocker posture to fail under -RequireReady when posture releaseId differs." -Script {
    & $validatorPath `
        -ReadinessReportPath $stalePosturePath `
        -OutputJsonPath (Join-Path $testRoot "stale-posture-development-ready-validation.json") `
        -RequireReady
}

$stalePostureValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "stale-posture-development-ready-validation.json") | ConvertFrom-Json
Assert-True ($stalePostureValidation.developmentContinuationReady -eq $false) "Expected stale release blocker posture to fail development continuation readiness."
Assert-True ($stalePostureValidation.postureReleaseIdMatchesReport -eq $false) "Expected stale release blocker posture releaseId mismatch to be recorded."

Assert-Fails -Message "Expected posture blocker count mismatch to fail under -RequireReady." -Script {
    & $validatorPath `
        -ReadinessReportPath $postureCountMismatchPath `
        -OutputJsonPath (Join-Path $testRoot "posture-count-mismatch-development-ready-validation.json") `
        -RequireReady
}

$postureCountMismatchValidation = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "posture-count-mismatch-development-ready-validation.json") | ConvertFrom-Json
Assert-True ($postureCountMismatchValidation.developmentContinuationReady -eq $false) "Expected release blocker posture count mismatch to fail development continuation readiness."
Assert-True ($postureCountMismatchValidation.postureBlockerCountMatchesBlockingRequirements -eq $false) "Expected release blocker posture count mismatch to be recorded."

Write-Host $successMessage -ForegroundColor Green
