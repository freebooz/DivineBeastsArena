<#
Exercises diagnose-release-blockers.ps1 against a small release readiness fixture.

The test proves every blocking production evidence requirement is translated into
an actionable owner/tool/next-command record with observed evidence reasons.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-blocker-actions-tests-{0}" -f [guid]::NewGuid().ToString("N"))
if (Test-Path -LiteralPath $testRoot) {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $testRoot | Out-Null
$clientEvidenceRoot = Join-Path $testRoot "client"
$unrealEvidenceRoot = Join-Path $testRoot "unreal"
New-Item -ItemType Directory -Force -Path $clientEvidenceRoot, $unrealEvidenceRoot | Out-Null

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$reportPath = Join-Path $testRoot "release-readiness-report.json"
$outputJsonPath = Join-Path $testRoot "release-blocker-actions.json"
$outputMarkdownPath = Join-Path $testRoot "release-blocker-actions.md"

@{
    kind = "client-release-prerequisites"
    readyForReleaseInputs = $false
    blockingIssues = @(
        @{ code = "download_url_example"; message = "DownloadUrl still points at cdn.example.com" },
        @{ code = "manifest_url_example"; message = "ManifestUrl still points at cdn.example.com" }
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "client-release-prerequisites-example-url.json") -Encoding UTF8

@{
    kind = "client-package-launcher"
    releaseReady = $false
    releaseReadinessNotes = @("Old package evidence still used example CDN.")
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "client-package-launcher-old.json") -Encoding UTF8

@{
    kind = "client-package-launcher"
    releaseReady = $false
    releaseReadinessNotes = @("New package evidence still used example CDN.")
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "client-package-launcher-new.json") -Encoding UTF8

(Get-Item -LiteralPath (Join-Path $clientEvidenceRoot "client-package-launcher-old.json")).LastWriteTimeUtc = [datetime]"2026-06-28T00:00:00Z"
(Get-Item -LiteralPath (Join-Path $clientEvidenceRoot "client-package-launcher-new.json")).LastWriteTimeUtc = [datetime]"2026-06-28T01:00:00Z"

@{
    kind = "launcher-manifest"
    version = "0.1.0.0"
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "launcher-manifest.json") -Encoding UTF8

@{
    kind = "launcher-cdn-smoke"
    cdnReady = $false
    manifestUrlIsHttps = $false
    downloadUrlIsHttps = $false
    AllowLocalHttp = $true
    downloadedFileCount = 34
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "launcher-cdn-smoke.json") -Encoding UTF8

@{
    kind = "code-signing"
    signingReady = $false
    unsignedFileCount = 7
    trustedSignedFileCount = 8
    signableFileCount = 15
    signedFileCount = 8
    invalidSignedFileCount = 0
    signingReadinessNotes = @(
        "Unsigned files are present.",
        "Not every signable file has a trusted valid signature.",
        "Fixture extra signing note A.",
        "Fixture extra signing note B.",
        "Fixture extra signing note C."
    )
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $clientEvidenceRoot "code-signing.json") -Encoding UTF8

@{
    kind = "ai-showcase-automation"
    automationReady = $false
    logErrorCount = 16
    logWarningCount = 7
    requestedTestCount = 4
    passedTestCount = 4
    testFilter = "DivineBeastsArena.Showcase.AIShowcase"
} | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $unrealEvidenceRoot "ai-showcase-automation.json") -Encoding UTF8

$report = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-readiness-report"
    generatedAtUtc = "2026-06-28T00:00:00.0000000Z"
    readyForRelease = $false
    releaseId = "fixture-release"
    requirementCount = 5
    presentRequirementCount = 0
    blockingRequirementCount = 5
    blockingRequirements = @(
        [ordered]@{
            key = "unreal.ai_showcase_automation"
            status = "incomplete"
            description = "AI_Showcase regression evidence is missing or failed"
            fileCount = 1
            files = @("unreal/ai-showcase-automation.json")
        },
        [ordered]@{
            key = "client.release_prerequisites"
            status = "incomplete"
            description = "Release inputs still use example CDN or missing signing identity"
            fileCount = 1
            files = @("client/client-release-prerequisites-example-url.json")
        },
        [ordered]@{
            key = "client.package_launcher"
            status = "incomplete"
            description = "Package evidence is not release ready"
            fileCount = 3
            files = @("client/client-package-launcher-old.json", "client/client-package-launcher-new.json", "client/launcher-manifest.json")
        },
        [ordered]@{
            key = "client.cdn_launcher_smoke"
            status = "incomplete"
            description = "No production CDN smoke"
            fileCount = 1
            files = @("client/launcher-cdn-smoke.json")
        },
        [ordered]@{
            key = "client.code_signing"
            status = "incomplete"
            description = "Unsigned public binaries"
            fileCount = 1
            files = @("client/code-signing.json")
        }
    )
}

$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $reportPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\diagnose-release-blockers.ps1") `
    -ReportPath $reportPath `
    -OutputJsonPath $outputJsonPath `
    -OutputMarkdownPath $outputMarkdownPath

$actions = Get-Content -Raw -Encoding UTF8 -LiteralPath $outputJsonPath | ConvertFrom-Json
$markdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $outputMarkdownPath

Assert-True ($actions.kind -eq "release-blocker-actions") "Expected release-blocker-actions kind."
Assert-True ($actions.readyForRelease -eq $false) "Expected readyForRelease=false."
Assert-True ($actions.blockerCount -eq 5) "Expected 5 blocker actions."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and $_.script -match "run-ai-showcase-automation.ps1" -and $_.nextCommand -match "-EvidenceDir" -and $_.nextCommand -match "-RunId" }).Count -eq 1) "Expected AI_Showcase automation action."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and $_.automationBlocked -eq $false }).Count -eq 1) "Expected AI_Showcase action to be automatically executable."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and ($_.observedReasons -join "`n") -match "automationReady=False" }).Count -eq 1) "Expected AI_Showcase observed reason."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and ($_.observedReasons -join "`n") -match "logErrorCount=16" }).Count -eq 1) "Expected AI_Showcase log error observed reason."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and ($_.observedReasons -join "`n") -match "requestedTestCount=4" -and ($_.observedReasons -join "`n") -match "passedTestCount=4" }).Count -eq 1) "Expected AI_Showcase stale 4/4 test count observed reasons."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and $_.unblockCriteria -match "logErrorCount=0" }).Count -eq 1) "Expected AI_Showcase unblock criteria to require zero log errors."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "unreal.ai_showcase_automation" -and $_.unblockCriteria -match "requestedTestCount=5" -and $_.unblockCriteria -match "passedTestCount=5" }).Count -eq 1) "Expected AI_Showcase unblock criteria to require the current 5/5 automation suite."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.release_prerequisites" -and $_.script -match "diagnose-client-release-prerequisites.ps1" }).Count -eq 1) "Expected prerequisite diagnostic action."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.release_prerequisites" -and $_.nextCommand -match "-PackageRoot <release-package-root>" -and $_.nextCommand -match "-DownloadUrl <real-https-cdn-download-url>" -and $_.nextCommand -match "-ManifestUrl <real-https-cdn-manifest-url>" -and $_.nextCommand -match "-RequireManifestUrl" -and $_.nextCommand -match "-RequireSigningIdentity" -and $_.nextCommand -match "-CertificateThumbprint <trusted-authenticode-signing-identity>" -and $_.nextCommand -match "-SignToolPath <signtool-path>" -and $_.nextCommand -match "-RequireSignTool" }).Count -eq 1) "Expected prerequisite diagnostic action to include executable release input placeholders."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.package_launcher" -and $_.script -match "run-client-release-evidence.ps1" -and $_.nextCommand -match "-PackageRoot <public-shipping-package-root>" -and $_.nextCommand -match "-DownloadUrl <real-https-cdn-download-url>" -and $_.nextCommand -match "-ManifestUrl <real-https-cdn-manifest-url>" }).Count -eq 1) "Expected package action to route through release evidence bundle with valid placeholders."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.cdn_launcher_smoke" -and $_.script -match "run-launcher-cdn-smoke.ps1" -and $_.nextCommand -match "-ManifestUrl <real-https-cdn-manifest-url>" -and $_.nextCommand -match "-InstallRoot <local-smoke-install-root>" }).Count -eq 1) "Expected CDN smoke action with valid placeholders."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.code_signing" -and $_.script -match "sign-client-release-package.ps1" }).Count -eq 1) "Expected code signing action."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.release_prerequisites" -and ($_.observedReasons -join "`n") -match "download_url_example" }).Count -eq 1) "Expected prerequisite observed reason."
$packageAction = @($actions.actions | Where-Object { $_.key -eq "client.package_launcher" })[0]
Assert-True (($packageAction.observedReasons[0] -match "client-package-launcher-new.json") -and ($packageAction.observedReasons[0] -match "releaseReady=False")) "Expected newest package evidence to be reported first."
Assert-True ($packageAction.latestEvidencePath -eq "client/client-package-launcher-new.json") "Expected latest package evidence path to be exposed."
Assert-True ($packageAction.automationBlocked -eq $true) "Expected package action to be marked blocked on external inputs."
Assert-True (($packageAction.blockingExternalInputs -join "`n") -match "public Shipping package root") "Expected package action to expose blocking external inputs."
Assert-True (@($packageAction.inputResolutionHints | Where-Object { $_.input -eq "public Shipping package root" -and $_.parameters -contains "-PackageRoot" }).Count -eq 1) "Expected package input hint to include -PackageRoot."
Assert-True (($packageAction.observedReasons -join "`n") -match "New package evidence still used example CDN") "Expected package observed reason."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.package_launcher" -and ($_.observedReasons -join "`n") -match "launcher-manifest.json:" }).Count -eq 0) "Expected empty package observed reasons to be omitted."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.cdn_launcher_smoke" -and ($_.observedReasons -join "`n") -match "manifestUrlIsHttps=false" }).Count -eq 1) "Expected CDN observed reason."
$cdnAction = @($actions.actions | Where-Object { $_.key -eq "client.cdn_launcher_smoke" })[0]
Assert-True (($cdnAction.missingExternalInputs -join "`n") -match "real HTTPS CDN manifest URL") "Expected CDN action to name the missing production manifest input."
Assert-True ($cdnAction.automationBlocked -eq $true) "Expected CDN smoke action to be marked blocked on external inputs."
Assert-True (@($cdnAction.inputResolutionHints | Where-Object { $_.input -eq "real HTTPS CDN manifest URL" -and $_.parameters -contains "-ManifestUrl" }).Count -eq 1) "Expected CDN input hint to include -ManifestUrl."
Assert-True (@($actions.actions | Where-Object { $_.key -eq "client.code_signing" -and ($_.observedReasons -join "`n") -match "unsignedFileCount=7" }).Count -eq 1) "Expected signing observed reason."
$signingAction = @($actions.actions | Where-Object { $_.key -eq "client.code_signing" })[0]
Assert-True (($signingAction.missingExternalInputs -join "`n") -match "trusted Authenticode signing identity") "Expected signing action to name the missing signing identity."
Assert-True (($signingAction.blockingExternalInputs -join "`n") -match "timestamp URL") "Expected signing action to expose blocking signing inputs."
Assert-True (@($signingAction.inputResolutionHints | Where-Object { $_.input -eq "trusted Authenticode signing identity" -and $_.parameters -contains "-CertificateThumbprint" -and $_.parameters -contains "-CertificateSubject" }).Count -eq 1) "Expected signing input hint to include certificate parameters."
Assert-True ($signingAction.observedReasonCount -gt 5) "Expected full signing observed reason count to preserve omitted detail count."
Assert-True (@($signingAction.observedReasons).Count -le 5) "Expected displayed signing observed reasons to be capped."
Assert-True ($markdown -match "client.release_prerequisites") "Expected prerequisite blocker in markdown."
Assert-True ($markdown -match "unreal.ai_showcase_automation") "Expected AI_Showcase blocker in markdown."
Assert-True ($markdown -match "run-ai-showcase-automation.ps1") "Expected AI_Showcase command in markdown."
Assert-True ($markdown -match "diagnose-client-release-prerequisites.ps1") "Expected prerequisite command in markdown."
Assert-True ($markdown -match "run-launcher-cdn-smoke.ps1") "Expected CDN smoke command in markdown."
Assert-True ($markdown -match "Observed reasons") "Expected observed reasons section in markdown."
Assert-True ($markdown -match "showing 5 of") "Expected markdown to show observed reason truncation count."
Assert-True ($markdown -match "Automation status") "Expected automation status section in markdown."
Assert-True ($markdown -match "automationBlocked: True") "Expected markdown to show automation is blocked."
Assert-True ($markdown -match "Input resolution hints") "Expected markdown to include input resolution hints."
Assert-True ($markdown -match "parameters: -CertificateThumbprint, -CertificateSubject, -PfxPath") "Expected markdown to include signing parameter hints."
Assert-True ($markdown -match "Missing external inputs") "Expected missing external inputs section in markdown."
Assert-True ($markdown -match "latest evidence: client/client-package-launcher-new.json") "Expected latest evidence path in markdown."
Assert-True ($markdown -match "download_url_example") "Expected observed prerequisite reason in markdown."
Assert-True ($markdown -match "unsignedFileCount=7") "Expected observed signing reason in markdown."

Write-Host "PASS: release blocker action fixtures" -ForegroundColor Green
