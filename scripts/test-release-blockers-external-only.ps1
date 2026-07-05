<#
Exercises validate-release-blockers-external-only.ps1 against release blocker fixtures.

The test proves the current development-phase release posture can distinguish
external-input-only release blockers from local automation blockers that should
not be skipped.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-blockers-external-only-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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

$validatorPath = Join-Path $repoRoot "scripts\validate-release-blockers-external-only.ps1"
$validatorSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $validatorPath
$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 21457, 24067, 38459, 22622, 39033, 22806, 37096, 36755, 20837, 26657, 39564, 22865, 32422)
$expectedJsonMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 32, 74, 83, 79, 78, 32, 20889, 20837, 28040, 24687, 12290)
$expectedValidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 26377, 25928, 28040, 24687, 12290)
$expectedInvalidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 26080, 25928, 28040, 24687, 12290)
$expectedRequireValidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 32, 82, 101, 113, 117, 105, 114, 101, 86, 97, 108, 105, 100, 32, 22833, 36133, 28040, 24687, 12290)
$forbiddenOldJsonMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 32, 74, 83, 79, 78, 32, 20889, 20837, 28040, 24687, 12290)
$forbiddenOldValidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 26377, 25928, 28040, 24687, 12290)
$forbiddenOldInvalidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 26080, 25928, 28040, 24687, 12290)
$forbiddenOldRequireValidMessageContract = New-TextFromCodePoints @(26657, 39564, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 32, 82, 101, 113, 117, 105, 114, 101, 86, 97, 108, 105, 100, 32, 22833, 36133, 28040, 24687, 12290)

Assert-True ($validatorSource.Contains('$jsonWrittenMessage')) $expectedJsonMessageContract
Assert-True ($validatorSource.Contains('$validMessage')) $expectedValidMessageContract
Assert-True ($validatorSource.Contains('$invalidMessage')) $expectedInvalidMessageContract
Assert-True ($validatorSource.Contains('$requireValidFailureMessage')) $expectedRequireValidMessageContract
Assert-True (-not $validatorSource.Contains("Release blockers external-only validation JSON written")) $forbiddenOldJsonMessageContract
Assert-True (-not $validatorSource.Contains("Release blockers external-only validation: valid")) $forbiddenOldValidMessageContract
Assert-True (-not $validatorSource.Contains("Release blockers external-only validation: invalid")) $forbiddenOldInvalidMessageContract
Assert-True (-not $validatorSource.Contains("Release blockers are not external-input-only")) $forbiddenOldRequireValidMessageContract

$validActionsPath = Join-Path $testRoot "release-blocker-actions-external-only.json"
$invalidAutomationActionsPath = Join-Path $testRoot "release-blocker-actions-local-automation.json"
$invalidEmptyInputsActionsPath = Join-Path $testRoot "release-blocker-actions-empty-inputs.json"
$invalidCountActionsPath = Join-Path $testRoot "release-blocker-actions-count-mismatch.json"
$invalidDuplicateActionsPath = Join-Path $testRoot "release-blocker-actions-duplicate-key.json"

[ordered]@{
    kind = "release-blocker-actions"
    releaseId = "fixture-external-only"
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = "client.package_launcher"
            automationBlocked = $true
            blockingExternalInputs = @("public Shipping package root", "real HTTPS CDN download URL")
        },
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            blockingExternalInputs = @("trusted Authenticode signing identity", "timestamp URL")
        }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $validActionsPath -Encoding UTF8

[ordered]@{
    kind = "release-blocker-actions"
    releaseId = "fixture-local-automation"
    blockerCount = 1
    actions = @(
        [ordered]@{
            key = "unreal.ai_showcase_automation"
            automationBlocked = $false
            blockingExternalInputs = @()
        }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $invalidAutomationActionsPath -Encoding UTF8

[ordered]@{
    kind = "release-blocker-actions"
    releaseId = "fixture-empty-inputs"
    blockerCount = 1
    actions = @(
        [ordered]@{
            key = "client.package_launcher"
            automationBlocked = $true
            blockingExternalInputs = @()
        }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $invalidEmptyInputsActionsPath -Encoding UTF8

[ordered]@{
    kind = "release-blocker-actions"
    releaseId = "fixture-count-mismatch"
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            blockingExternalInputs = @("trusted Authenticode signing identity")
        }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $invalidCountActionsPath -Encoding UTF8

[ordered]@{
    kind = "release-blocker-actions"
    releaseId = "fixture-duplicate-key"
    blockerCount = 2
    actions = @(
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            blockingExternalInputs = @("trusted Authenticode signing identity")
        },
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            blockingExternalInputs = @("timestamp URL")
        }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $invalidDuplicateActionsPath -Encoding UTF8

& $validatorPath `
    -ActionReportPath $validActionsPath `
    -OutputJsonPath (Join-Path $testRoot "external-only-valid.json") `
    -RequireValid

$validReport = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "external-only-valid.json") | ConvertFrom-Json
Assert-True ($validReport.kind -eq "release-blockers-external-only-validation") "Expected external-only validation report kind."
Assert-True ($validReport.externalOnly -eq $true) "Expected valid fixture to be external-only."
Assert-True ($validReport.externalBlockerCount -eq 2) "Expected valid fixture external blocker count."
Assert-True ($validReport.reportedBlockerCount -eq 2) "Expected reported blocker count to be preserved."
Assert-True ($validReport.blockerCountMatchesActions -eq $true) "Expected valid fixture blocker count to match actions."
Assert-True ($validReport.localAutomationBlockerCount -eq 0) "Expected no local automation blockers."
Assert-True ($validReport.emptyExternalInputBlockerCount -eq 0) "Expected no empty external-input blockers."

Assert-Fails -Message "Expected local automation blocker fixture to fail under -RequireValid." -Script {
    & $validatorPath `
        -ActionReportPath $invalidAutomationActionsPath `
        -OutputJsonPath (Join-Path $testRoot "external-only-invalid-automation.json") `
        -RequireValid
}
$invalidAutomationReport = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "external-only-invalid-automation.json") | ConvertFrom-Json
Assert-True ($invalidAutomationReport.externalOnly -eq $false) "Expected local automation blocker fixture to be invalid."
Assert-True ($invalidAutomationReport.localAutomationBlockerCount -eq 1) "Expected one local automation blocker."
Assert-True (($invalidAutomationReport.localAutomationBlockers -join "`n") -match "unreal.ai_showcase_automation") "Expected AI_Showcase local automation blocker detail."

Assert-Fails -Message "Expected empty external-input blocker fixture to fail under -RequireValid." -Script {
    & $validatorPath `
        -ActionReportPath $invalidEmptyInputsActionsPath `
        -OutputJsonPath (Join-Path $testRoot "external-only-invalid-empty-inputs.json") `
        -RequireValid
}
$invalidInputsReport = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "external-only-invalid-empty-inputs.json") | ConvertFrom-Json
Assert-True ($invalidInputsReport.externalOnly -eq $false) "Expected empty input fixture to be invalid."
Assert-True ($invalidInputsReport.emptyExternalInputBlockerCount -eq 1) "Expected one blocker with empty external inputs."
Assert-True (($invalidInputsReport.emptyExternalInputBlockers -join "`n") -match "client.package_launcher") "Expected empty external-input blocker detail."

Assert-Fails -Message "Expected blocker count mismatch fixture to fail under -RequireValid." -Script {
    & $validatorPath `
        -ActionReportPath $invalidCountActionsPath `
        -OutputJsonPath (Join-Path $testRoot "external-only-invalid-count.json") `
        -RequireValid
}
$invalidCountReport = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "external-only-invalid-count.json") | ConvertFrom-Json
Assert-True ($invalidCountReport.externalOnly -eq $false) "Expected count mismatch fixture to be invalid."
Assert-True ($invalidCountReport.reportedBlockerCount -eq 2) "Expected reported blocker count to be preserved for diagnostics."
Assert-True ($invalidCountReport.blockerCount -eq 1) "Expected actual action count to be preserved for diagnostics."
Assert-True ($invalidCountReport.blockerCountMatchesActions -eq $false) "Expected count mismatch to be recorded."

Assert-Fails -Message "Expected duplicate blocker key fixture to fail under -RequireValid." -Script {
    & $validatorPath `
        -ActionReportPath $invalidDuplicateActionsPath `
        -OutputJsonPath (Join-Path $testRoot "external-only-invalid-duplicate-key.json") `
        -RequireValid
}
$invalidDuplicateReport = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $testRoot "external-only-invalid-duplicate-key.json") | ConvertFrom-Json
Assert-True ($invalidDuplicateReport.externalOnly -eq $false) "Expected duplicate blocker key fixture to be invalid."
Assert-True ($invalidDuplicateReport.duplicateActionKeyCount -eq 1) "Expected duplicate blocker key count to be recorded."
Assert-True (($invalidDuplicateReport.duplicateActionKeys -join "`n") -match "client.code_signing") "Expected duplicate blocker key detail."

Write-Host $successMessage -ForegroundColor Green
