<#
Exercises write-release-readiness-report.ps1 against small manifest fixtures.

The test proves JSON counters, blocking requirement projection, markdown table
content, and -RequireReady failure behavior stay aligned with the manifest.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-readiness-report-tests-{0}" -f [guid]::NewGuid().ToString("N"))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 21457, 24067, 23601, 32490, 25253, 21578, 22841, 20855, 22865, 32422)
$expectedUniqueTestRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 24517, 39035, 20351, 29992, 21807, 19968, 20020, 26102, 30446, 24405, 65292, 36991, 20813, 22797, 29992, 26087, 22841, 20855, 30446, 24405, 12290)
$forbiddenFixedTestRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 22797, 29992, 22266, 23450, 21457, 24067, 23601, 32490, 25253, 21578, 22841, 20855, 30446, 24405, 12290)
$expectedChineseSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenOldSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenFixedTestRootLiteral = New-TextFromCodePoints @(36, 116, 101, 115, 116, 82, 111, 111, 116, 32, 61, 32, 74, 111, 105, 110, 45, 80, 97, 116, 104, 32, 36, 114, 101, 112, 111, 82, 111, 111, 116, 32, 34, 46, 116, 109, 112, 92, 114, 101, 108, 101, 97, 115, 101, 45, 114, 101, 97, 100, 105, 110, 101, 115, 115, 45, 114, 101, 112, 111, 114, 116, 45, 116, 101, 115, 116, 115, 34)
$forbiddenOldSuccessLiteral = New-TextFromCodePoints @(80, 65, 83, 83, 58, 32, 114, 101, 108, 101, 97, 115, 101, 32, 114, 101, 97, 100, 105, 110, 101, 115, 115, 32, 114, 101, 112, 111, 114, 116, 32, 102, 105, 120, 116, 117, 114, 101, 115)

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$testSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $PSCommandPath
Assert-True ($testSource.Contains("release-readiness-report-tests-{0}")) $expectedUniqueTestRootMessage
Assert-True (-not $testSource.Contains($forbiddenFixedTestRootLiteral)) $forbiddenFixedTestRootMessage
Assert-True ($testSource.Contains('$successMessage')) $expectedChineseSuccessMessage
Assert-True (-not $testSource.Contains($forbiddenOldSuccessLiteral)) $forbiddenOldSuccessMessage

New-Item -ItemType Directory -Force -Path $testRoot | Out-Null

$manifestPath = Join-Path $testRoot "production-evidence-manifest.json"
$reportJsonPath = Join-Path $testRoot "release-readiness-report.json"
$reportMarkdownPath = Join-Path $testRoot "release-readiness-report.md"
$externalOnlyValidationPath = Join-Path $testRoot "release-blockers-external-only-validation.json"
$stalePostureReportJsonPath = Join-Path $testRoot "release-readiness-report-stale-posture.json"
$stalePostureReportMarkdownPath = Join-Path $testRoot "release-readiness-report-stale-posture.md"
$countMismatchReportJsonPath = Join-Path $testRoot "release-readiness-report-count-mismatch.json"
$countMismatchReportMarkdownPath = Join-Path $testRoot "release-readiness-report-count-mismatch.md"

$manifest = [ordered]@{
    schemaVersion = "1.0"
    releaseId = "fixture-release"
    generatedAtUtc = "2026-06-28T00:00:00.0000000Z"
    repoRoot = $repoRoot
    gitCommit = "fixture-commit"
    gitIsDirty = $true
    evidenceRoot = $testRoot
    requirements = @(
        [ordered]@{
            key = "security.nuget"
            description = "NuGet audit is present"
            status = "present"
            fileCount = 1
            files = @("security/vulnerability-report.txt")
        },
        [ordered]@{
            key = "client.code_signing"
            description = "Unsigned binaries keep release blocked"
            status = "incomplete"
            fileCount = 1
            files = @("client/code-signing-fixture.json")
        },
        [ordered]@{
            key = "client.cdn_launcher_smoke"
            description = "CDN smoke missing | needs real URL"
            status = "missing"
            fileCount = 0
            files = @()
        }
    )
    files = @(
        [ordered]@{
            path = "security/vulnerability-report.txt"
            sizeBytes = 12
            modifiedAtUtc = "2026-06-28T00:00:00.0000000Z"
            sha256 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        }
    )
}

$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

$externalOnlyValidation = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blockers-external-only-validation"
    releaseId = "fixture-release"
    blockerCount = 2
    externalOnly = $true
    externalBlockerCount = 2
    externalBlockers = @("client.code_signing", "client.cdn_launcher_smoke")
    localAutomationBlockerCount = 0
    localAutomationBlockers = @()
    emptyExternalInputBlockerCount = 0
    emptyExternalInputBlockers = @()
}

$externalOnlyValidation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $externalOnlyValidationPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1") `
    -EvidenceRoot $testRoot `
    -ManifestPath $manifestPath `
    -OutputJsonPath $reportJsonPath `
    -OutputMarkdownPath $reportMarkdownPath

$report = Get-Content -Raw -Encoding UTF8 -LiteralPath $reportJsonPath | ConvertFrom-Json
$markdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $reportMarkdownPath

Assert-True ($report.kind -eq "release-readiness-report") "Expected release-readiness-report kind."
Assert-True ($report.readyForRelease -eq $false) "Expected readyForRelease=false."
Assert-True ($report.requirementCount -eq 3) "Expected 3 requirements."
Assert-True ($report.presentRequirementCount -eq 1) "Expected 1 present requirement."
Assert-True ($report.blockingRequirementCount -eq 2) "Expected 2 blocking requirements."
Assert-True ($report.developmentContinuationReady -eq $true) "Expected development continuation to be ready when all blockers are external release inputs."
Assert-True ($report.releaseBlockerPosture.kind -eq "release-blockers-external-only-validation") "Expected release blocker posture to be projected."
Assert-True ($report.releaseBlockerPosture.externalOnly -eq $true) "Expected release blocker posture to be external-only."
Assert-True ($report.releaseBlockerPosture.releaseId -eq "fixture-release") "Expected release blocker posture releaseId to be projected."
Assert-True ($report.releaseBlockerPosture.releaseIdMatchesManifest -eq $true) "Expected release blocker posture releaseId to match manifest."
Assert-True ($report.releaseBlockerPosture.blockerCountMatchesManifest -eq $true) "Expected release blocker posture blocker count to match manifest blocking requirements."
Assert-True ($report.releaseBlockerPosture.externalBlockerCount -eq 2) "Expected 2 external release blockers."
Assert-True ($report.releaseBlockerPosture.localAutomationBlockerCount -eq 0) "Expected no local automation blockers."
Assert-True (@($report.blockingRequirements | Where-Object { $_.key -eq "client.code_signing" }).Count -eq 1) "Expected client.code_signing to block release."
Assert-True (@($report.blockingRequirements | Where-Object { $_.key -eq "client.cdn_launcher_smoke" }).Count -eq 1) "Expected client.cdn_launcher_smoke to block release."
Assert-True ($markdown -match "presentRequirements: 1 / 3") "Expected markdown present requirement count."
Assert-True ($markdown -match "blockingRequirements: 2") "Expected markdown blocking requirement count."
Assert-True ($markdown -match "developmentContinuationReady: True") "Expected markdown development continuation readiness."
Assert-True ($markdown -match "externalOnlyReleaseBlockers: True") "Expected markdown external-only blocker posture."
Assert-True ($markdown -match "localAutomationBlockers: 0") "Expected markdown local automation blocker count."
Assert-True ($markdown -match "CDN smoke missing \\| needs real URL") "Expected markdown table pipe escaping."

$failedRequireReady = $false
try {
    & (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1") `
        -EvidenceRoot $testRoot `
        -ManifestPath $manifestPath `
        -OutputJsonPath (Join-Path $testRoot "require-ready.json") `
        -OutputMarkdownPath (Join-Path $testRoot "require-ready.md") `
        -RequireReady
}
catch {
    $failedRequireReady = ($_.Exception.Message -match "2 blockingRequirements")
}

Assert-True $failedRequireReady "Expected -RequireReady to fail with the blocking requirement count."

$staleExternalOnlyValidation = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blockers-external-only-validation"
    releaseId = "stale-fixture-release"
    blockerCount = 2
    externalOnly = $true
    externalBlockerCount = 2
    externalBlockers = @("client.code_signing", "client.cdn_launcher_smoke")
    localAutomationBlockerCount = 0
    localAutomationBlockers = @()
    emptyExternalInputBlockerCount = 0
    emptyExternalInputBlockers = @()
}
$staleExternalOnlyValidation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $externalOnlyValidationPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1") `
    -EvidenceRoot $testRoot `
    -ManifestPath $manifestPath `
    -OutputJsonPath $stalePostureReportJsonPath `
    -OutputMarkdownPath $stalePostureReportMarkdownPath

$stalePostureReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $stalePostureReportJsonPath | ConvertFrom-Json
Assert-True ($stalePostureReport.readyForRelease -eq $false) "Expected stale posture fixture not to be release-ready."
Assert-True ($stalePostureReport.developmentContinuationReady -eq $false) "Expected stale posture releaseId to block development continuation readiness."
Assert-True ($stalePostureReport.releaseBlockerPosture.releaseIdMatchesManifest -eq $false) "Expected stale posture releaseId mismatch to be recorded."

$countMismatchExternalOnlyValidation = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blockers-external-only-validation"
    releaseId = "fixture-release"
    blockerCount = 1
    externalOnly = $true
    externalBlockerCount = 1
    externalBlockers = @("client.code_signing")
    localAutomationBlockerCount = 0
    localAutomationBlockers = @()
    emptyExternalInputBlockerCount = 0
    emptyExternalInputBlockers = @()
}
$countMismatchExternalOnlyValidation | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $externalOnlyValidationPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1") `
    -EvidenceRoot $testRoot `
    -ManifestPath $manifestPath `
    -OutputJsonPath $countMismatchReportJsonPath `
    -OutputMarkdownPath $countMismatchReportMarkdownPath

$countMismatchReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $countMismatchReportJsonPath | ConvertFrom-Json
Assert-True ($countMismatchReport.readyForRelease -eq $false) "Expected count mismatch posture fixture not to be release-ready."
Assert-True ($countMismatchReport.developmentContinuationReady -eq $false) "Expected posture blocker count mismatch to block development continuation readiness."
Assert-True ($countMismatchReport.releaseBlockerPosture.blockerCountMatchesManifest -eq $false) "Expected posture blocker count mismatch to be recorded."

Write-Host $successMessage -ForegroundColor Green
