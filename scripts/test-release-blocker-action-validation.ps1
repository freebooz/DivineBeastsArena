<#
Exercises validate-release-blocker-actions.ps1 against valid and invalid action reports.

The test proves release blocker nextCommand drafts stay aligned with existing
scripts, declared PowerShell parameters, and inputResolutionHints placeholders.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-blocker-action-validation-tests-{0}" -f [guid]::NewGuid().ToString("N"))
if (Test-Path -LiteralPath $testRoot) {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}
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

function Read-Json {
    param([Parameter(Mandatory = $true)][string]$Path)
    return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

$validActionsPath = Join-Path $testRoot "release-blocker-actions-valid.json"
$validOutputPath = Join-Path $testRoot "validation-valid.json"
$invalidActionsPath = Join-Path $testRoot "release-blocker-actions-invalid.json"
$invalidOutputPath = Join-Path $testRoot "validation-invalid.json"
$countMismatchActionsPath = Join-Path $testRoot "release-blocker-actions-count-mismatch.json"
$countMismatchOutputPath = Join-Path $testRoot "validation-count-mismatch.json"
$wrongKindActionsPath = Join-Path $testRoot "release-blocker-actions-wrong-kind.json"
$wrongKindOutputPath = Join-Path $testRoot "validation-wrong-kind.json"
$missingCountActionsPath = Join-Path $testRoot "release-blocker-actions-missing-count.json"
$missingCountOutputPath = Join-Path $testRoot "validation-missing-count.json"
$malformedActionFieldsPath = Join-Path $testRoot "release-blocker-actions-malformed-fields.json"
$malformedActionFieldsOutputPath = Join-Path $testRoot "validation-malformed-fields.json"
$duplicateKeyActionsPath = Join-Path $testRoot "release-blocker-actions-duplicate-key.json"
$duplicateKeyOutputPath = Join-Path $testRoot "validation-duplicate-key.json"
$validReadinessPath = Join-Path $testRoot "release-readiness-report-valid.json"
$mismatchedReadinessPath = Join-Path $testRoot "release-readiness-report-mismatched.json"
$missingReportPathActionsPath = Join-Path $testRoot "release-blocker-actions-missing-report-path.json"
$missingReportPathOutputPath = Join-Path $testRoot "validation-missing-report-path.json"
$missingReportFileActionsPath = Join-Path $testRoot "release-blocker-actions-missing-report-file.json"
$missingReportFileOutputPath = Join-Path $testRoot "validation-missing-report-file.json"
$releaseIdMismatchActionsPath = Join-Path $testRoot "release-blocker-actions-release-id-mismatch.json"
$releaseIdMismatchOutputPath = Join-Path $testRoot "validation-release-id-mismatch.json"

$validReadinessReport = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-readiness-report"
    releaseId = "fixture-release"
    readyForRelease = $false
}

$mismatchedReadinessReport = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-readiness-report"
    releaseId = "different-fixture-release"
    readyForRelease = $false
}

$validActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 3
    actions = @(
        [ordered]@{
            key = "client.package_launcher"
            script = "scripts\run-client-release-evidence.ps1"
            nextCommand = ".\scripts\run-client-release-evidence.ps1 -PackageRoot <public-shipping-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -BuildConfiguration Shipping"
            inputResolutionHints = @(
                [ordered]@{ input = "public Shipping package root"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "real HTTPS CDN download URL"; parameters = @("-DownloadUrl"); environmentVariables = @() },
                [ordered]@{ input = "real HTTPS CDN manifest URL"; parameters = @("-ManifestUrl"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.code_signing"
            script = "scripts\sign-client-release-package.ps1"
            nextCommand = ".\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned"
            inputResolutionHints = @(
                [ordered]@{ input = "public package root containing signable binaries"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "timestamp URL"; parameters = @("-TimestampUrl"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "security.manual_followup"
            script = ""
            nextCommand = "Inspect release-readiness-report.json and add a blocker-specific release action."
            inputResolutionHints = @()
        }
    )
}

$invalidActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 3
    actions = @(
        [ordered]@{
            key = "client.missing_script"
            script = "scripts\missing-release-script.ps1"
            nextCommand = ".\scripts\missing-release-script.ps1 <undeclared-placeholder>"
            inputResolutionHints = @(
                [ordered]@{ input = "declared placeholder"; parameters = @("-PackageRoot"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.invalid_parameter"
            script = "scripts\run-client-release-evidence.ps1"
            nextCommand = ".\scripts\run-client-release-evidence.ps1 -NoSuchParam <declared-placeholder>"
            inputResolutionHints = @(
                [ordered]@{ input = "declared placeholder"; parameters = @("-PackageRoot"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.script_mismatch"
            script = "scripts\run-client-release-evidence.ps1"
            nextCommand = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <declared-placeholder>"
            inputResolutionHints = @(
                [ordered]@{ input = "declared placeholder"; parameters = @("-ManifestUrl"); environmentVariables = @() }
            )
        }
    )
}

$countMismatchActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = "client.code_signing"
            script = "scripts\sign-client-release-package.ps1"
            nextCommand = ".\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned"
            inputResolutionHints = @(
                [ordered]@{ input = "public package root containing signable binaries"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "timestamp URL"; parameters = @("-TimestampUrl"); environmentVariables = @() }
            )
        }
    )
}

$wrongKindActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-readiness-report"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 0
    actions = @()
}

$missingCountActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    actions = @()
}

$malformedActionFields = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = ""
            script = ""
            nextCommand = "Inspect release-readiness-report.json and add a blocker-specific release action."
            inputResolutionHints = @()
        },
        [ordered]@{
            key = "client.code_signing"
            script = ""
            nextCommand = ""
            inputResolutionHints = @()
        }
    )
}

$duplicateKeyActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $validReadinessPath
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = "client.code_signing"
            script = "scripts\sign-client-release-package.ps1"
            nextCommand = ".\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned"
            inputResolutionHints = @(
                [ordered]@{ input = "public package root containing signable binaries"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "timestamp URL"; parameters = @("-TimestampUrl"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.code_signing"
            script = ""
            nextCommand = "Inspect duplicate signing blocker action."
            inputResolutionHints = @()
        }
    )
}

$missingReportPathActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    blockerCount = 0
    actions = @()
}

$missingReportFileActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = (Join-Path $testRoot "missing-release-readiness-report.json")
    blockerCount = 0
    actions = @()
}

$releaseIdMismatchActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    reportPath = $mismatchedReadinessPath
    blockerCount = 0
    actions = @()
}

$validReadinessReport | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $validReadinessPath -Encoding UTF8
$mismatchedReadinessReport | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $mismatchedReadinessPath -Encoding UTF8
$validActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $validActionsPath -Encoding UTF8
$invalidActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidActionsPath -Encoding UTF8
$countMismatchActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $countMismatchActionsPath -Encoding UTF8
$wrongKindActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $wrongKindActionsPath -Encoding UTF8
$missingCountActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $missingCountActionsPath -Encoding UTF8
$malformedActionFields | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $malformedActionFieldsPath -Encoding UTF8
$duplicateKeyActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $duplicateKeyActionsPath -Encoding UTF8
$missingReportPathActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $missingReportPathActionsPath -Encoding UTF8
$missingReportFileActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $missingReportFileActionsPath -Encoding UTF8
$releaseIdMismatchActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $releaseIdMismatchActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
    -ActionReportPath $validActionsPath `
    -OutputJsonPath $validOutputPath `
    -RequireValid

$validReport = Read-Json -Path $validOutputPath
Assert-True ($validReport.kind -eq "release-blocker-action-validation") "Expected valid report kind."
Assert-True ($validReport.isValid -eq $true) "Expected valid blocker action report."
Assert-True ($validReport.invalidParameterCount -eq 0) "Expected no invalid parameters in valid blocker action report."
Assert-True ($validReport.missingPlaceholderCount -eq 0) "Expected no missing placeholders in valid blocker action report."
Assert-True ($validReport.manualActionCount -eq 1) "Expected manual blocker actions to be counted but not fail validation."
Assert-True ($validReport.reportedBlockerCount -eq 3) "Expected valid report to preserve reported blocker count."
Assert-True ($validReport.blockerCountMatchesActions -eq $true) "Expected valid report blocker count to match actions."
Assert-True ($validReport.reportPathIsPresent -eq $true) "Expected valid report path to be present."
Assert-True ($validReport.reportPathExists -eq $true) "Expected valid readiness report path to exist."
Assert-True ($validReport.reportKindIsValid -eq $true) "Expected linked readiness report kind to be valid."
Assert-True ($validReport.reportReleaseIdMatches -eq $true) "Expected linked readiness report releaseId to match action report."

$invalidSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $invalidActionsPath `
        -OutputJsonPath $invalidOutputPath `
        -RequireValid
}
catch {
    $invalidSucceeded = $false
}

Assert-True (-not $invalidSucceeded) "Expected invalid blocker action validation to fail."
$invalidReport = Read-Json -Path $invalidOutputPath
Assert-True ($invalidReport.isValid -eq $false) "Expected invalid blocker action report."
Assert-True ($invalidReport.missingScriptCount -eq 1) "Expected missing script validation detail."
Assert-True ($invalidReport.missingPlaceholderCount -eq 1) "Expected missing placeholder validation detail."
Assert-True ($invalidReport.invalidParameterCount -eq 1) "Expected invalid parameter validation detail."
Assert-True ($invalidReport.scriptMismatchCount -eq 1) "Expected script mismatch validation detail."

$countMismatchSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $countMismatchActionsPath `
        -OutputJsonPath $countMismatchOutputPath `
        -RequireValid
}
catch {
    $countMismatchSucceeded = $false
}

Assert-True (-not $countMismatchSucceeded) "Expected blocker count mismatch validation to fail."
$countMismatchReport = Read-Json -Path $countMismatchOutputPath
Assert-True ($countMismatchReport.isValid -eq $false) "Expected count mismatch action report to be invalid."
Assert-True ($countMismatchReport.reportedBlockerCount -eq 2) "Expected count mismatch report to preserve reported blocker count."
Assert-True ($countMismatchReport.actionCount -eq 1) "Expected count mismatch report to preserve actual action count."
Assert-True ($countMismatchReport.blockerCountMatchesActions -eq $false) "Expected blocker count mismatch to be recorded."

$wrongKindSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $wrongKindActionsPath `
        -OutputJsonPath $wrongKindOutputPath `
        -RequireValid
}
catch {
    $wrongKindSucceeded = $false
}

Assert-True (-not $wrongKindSucceeded) "Expected wrong-kind action validation to fail."
$wrongKindReport = Read-Json -Path $wrongKindOutputPath
Assert-True ($wrongKindReport.isValid -eq $false) "Expected wrong-kind action report to be invalid."
Assert-True ($wrongKindReport.reportKind -eq "release-readiness-report") "Expected wrong-kind validation to preserve observed report kind."
Assert-True ($wrongKindReport.kindIsValid -eq $false) "Expected wrong-kind validation to record invalid kind."

$missingCountSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $missingCountActionsPath `
        -OutputJsonPath $missingCountOutputPath `
        -RequireValid
}
catch {
    $missingCountSucceeded = $false
}

Assert-True (-not $missingCountSucceeded) "Expected missing blocker count validation to fail."
$missingCountReport = Read-Json -Path $missingCountOutputPath
Assert-True ($missingCountReport.isValid -eq $false) "Expected missing-count action report to be invalid."
Assert-True ($missingCountReport.blockerCountIsPresent -eq $false) "Expected missing-count validation to record absent blockerCount."
Assert-True ($missingCountReport.blockerCountMatchesActions -eq $false) "Expected missing blockerCount not to match actions even when action list is empty."

$malformedActionFieldsSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $malformedActionFieldsPath `
        -OutputJsonPath $malformedActionFieldsOutputPath `
        -RequireValid
}
catch {
    $malformedActionFieldsSucceeded = $false
}

Assert-True (-not $malformedActionFieldsSucceeded) "Expected malformed action fields validation to fail."
$malformedActionFieldsReport = Read-Json -Path $malformedActionFieldsOutputPath
Assert-True ($malformedActionFieldsReport.isValid -eq $false) "Expected malformed action fields report to be invalid."
Assert-True ($malformedActionFieldsReport.missingActionKeyCount -eq 1) "Expected missing action key count to be recorded."
Assert-True ($malformedActionFieldsReport.missingActionCommandCount -eq 1) "Expected missing action command count to be recorded."

$duplicateKeySucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $duplicateKeyActionsPath `
        -OutputJsonPath $duplicateKeyOutputPath `
        -RequireValid
}
catch {
    $duplicateKeySucceeded = $false
}

Assert-True (-not $duplicateKeySucceeded) "Expected duplicate action key validation to fail."
$duplicateKeyReport = Read-Json -Path $duplicateKeyOutputPath
Assert-True ($duplicateKeyReport.isValid -eq $false) "Expected duplicate action key report to be invalid."
Assert-True ($duplicateKeyReport.duplicateActionKeyCount -eq 1) "Expected duplicate action key count to be recorded."
Assert-True (($duplicateKeyReport.duplicateActionKeys -join "`n") -match "client.code_signing") "Expected duplicate action key detail to include client.code_signing."

$missingReportPathSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $missingReportPathActionsPath `
        -OutputJsonPath $missingReportPathOutputPath `
        -RequireValid
}
catch {
    $missingReportPathSucceeded = $false
}

Assert-True (-not $missingReportPathSucceeded) "Expected missing reportPath validation to fail."
$missingReportPathReport = Read-Json -Path $missingReportPathOutputPath
Assert-True ($missingReportPathReport.isValid -eq $false) "Expected missing reportPath action report to be invalid."
Assert-True ($missingReportPathReport.reportPathIsPresent -eq $false) "Expected missing reportPath to be recorded."

$missingReportFileSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $missingReportFileActionsPath `
        -OutputJsonPath $missingReportFileOutputPath `
        -RequireValid
}
catch {
    $missingReportFileSucceeded = $false
}

Assert-True (-not $missingReportFileSucceeded) "Expected missing linked readiness report file validation to fail."
$missingReportFileReport = Read-Json -Path $missingReportFileOutputPath
Assert-True ($missingReportFileReport.isValid -eq $false) "Expected missing linked readiness report file action report to be invalid."
Assert-True ($missingReportFileReport.reportPathIsPresent -eq $true) "Expected reportPath presence to be recorded."
Assert-True ($missingReportFileReport.reportPathExists -eq $false) "Expected missing linked readiness report file to be recorded."

$releaseIdMismatchSucceeded = $true
try {
    & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
        -ActionReportPath $releaseIdMismatchActionsPath `
        -OutputJsonPath $releaseIdMismatchOutputPath `
        -RequireValid
}
catch {
    $releaseIdMismatchSucceeded = $false
}

Assert-True (-not $releaseIdMismatchSucceeded) "Expected linked readiness report releaseId mismatch validation to fail."
$releaseIdMismatchReport = Read-Json -Path $releaseIdMismatchOutputPath
Assert-True ($releaseIdMismatchReport.isValid -eq $false) "Expected releaseId mismatch action report to be invalid."
Assert-True ($releaseIdMismatchReport.reportReleaseId -eq "different-fixture-release") "Expected linked readiness report releaseId to be recorded."
Assert-True ($releaseIdMismatchReport.reportReleaseIdMatches -eq $false) "Expected releaseId mismatch to be recorded."

Write-Host "PASS: release blocker action validation fixtures" -ForegroundColor Green
