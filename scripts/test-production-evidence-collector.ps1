<#
Exercises collect-production-evidence.ps1 against small file-index fixtures.

The test proves generated release readiness and blocker action reports are
treated as derived outputs and are not re-ingested into production evidence.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\production-evidence-collector-tests-{0}" -f [guid]::NewGuid().ToString("N"))
if (Test-Path -LiteralPath $testRoot) {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 29983, 20135, 35777, 25454, 25910, 38598, 22120, 22841, 20855, 22865, 32422)
$expectedChineseSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenOldSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25104, 21151, 28040, 24687, 12290)
$expectedCollectorManifestMessageContract = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 28165, 21333, 20889, 20837, 28040, 24687, 12290)
$expectedCollectorMissingTitleContract = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 32570, 22833, 20998, 31867, 26631, 39064, 12290)
$expectedCollectorReadyMessageContract = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 40784, 22791, 28040, 24687, 12290)
$forbiddenOldCollectorManifestMessage = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 28165, 21333, 20889, 20837, 28040, 24687, 12290)
$forbiddenOldCollectorMissingTitle = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 32570, 22833, 20998, 31867, 26631, 39064, 12290)
$forbiddenOldCollectorReadyMessage = New-TextFromCodePoints @(25910, 38598, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 40784, 22791, 28040, 24687, 12290)
$forbiddenOldSuccessLiteral = New-TextFromCodePoints @(80, 65, 83, 83, 58, 32, 112, 114, 111, 100, 117, 99, 116, 105, 111, 110, 32, 101, 118, 105, 100, 101, 110, 99, 101, 32, 99, 111, 108, 108, 101, 99, 116, 111, 114, 32, 102, 105, 120, 116, 117, 114, 101, 115)

$testSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $PSCommandPath
if (-not $testSource.Contains('$successMessage')) {
    throw $expectedChineseSuccessMessage
}
if ($testSource.Contains($forbiddenOldSuccessLiteral)) {
    throw $forbiddenOldSuccessMessage
}

$collectorScriptPath = Join-Path $repoRoot "scripts\collect-production-evidence.ps1"
$collectorSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $collectorScriptPath
if (-not $collectorSource.Contains('$manifestWrittenMessage')) {
    throw $expectedCollectorManifestMessageContract
}
if (-not $collectorSource.Contains('$missingEvidenceTitleMessage')) {
    throw $expectedCollectorMissingTitleContract
}
if (-not $collectorSource.Contains('$allEvidencePresentMessage')) {
    throw $expectedCollectorReadyMessageContract
}
if ($collectorSource.Contains("Production evidence manifest written")) {
    throw $forbiddenOldCollectorManifestMessage
}
if ($collectorSource.Contains("Missing or incomplete production evidence categories")) {
    throw $forbiddenOldCollectorMissingTitle
}
if ($collectorSource.Contains("All production evidence categories are present")) {
    throw $forbiddenOldCollectorReadyMessage
}

$evidenceRoot = Join-Path $testRoot "evidence"
$securityRoot = Join-Path $evidenceRoot "security"
$unrealRoot = Join-Path $evidenceRoot "unreal"
$reportsRoot = Join-Path $evidenceRoot "reports"
New-Item -ItemType Directory -Force -Path $securityRoot, $unrealRoot | Out-Null
New-Item -ItemType Directory -Force -Path $reportsRoot | Out-Null

Set-Content -LiteralPath (Join-Path $securityRoot "vulnerability-report.txt") -Encoding UTF8 -Value "fixture vulnerability report"
Set-Content -LiteralPath (Join-Path $unrealRoot "ai-showcase-automation-fixture.json") -Encoding UTF8 -Value '{"kind":"ai-showcase-automation","automationReady":true,"logErrorCount":0,"requestedTestCount":5,"passedTestCount":5,"testFilter":"DivineBeastsArena.Showcase.AIShowcase"}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-readiness-report.json") -Encoding UTF8 -Value '{"kind":"release-readiness-report","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-readiness-report.md") -Encoding UTF8 -Value "# Derived report"
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-blocker-actions.json") -Encoding UTF8 -Value '{"kind":"release-blocker-actions","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-blocker-actions.md") -Encoding UTF8 -Value "# Derived blocker actions"
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-blocker-action-validation.json") -Encoding UTF8 -Value '{"kind":"release-blocker-action-validation","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-blockers-external-only-validation.json") -Encoding UTF8 -Value '{"kind":"release-blockers-external-only-validation","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "development-continuation-readiness-validation.json") -Encoding UTF8 -Value '{"kind":"development-continuation-readiness-validation","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-template.json") -Encoding UTF8 -Value '{"kind":"release-input-template","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-template.md") -Encoding UTF8 -Value "# Derived release input template"
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-template-validation.json") -Encoding UTF8 -Value '{"kind":"release-input-template-validation","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-values.template.json") -Encoding UTF8 -Value '{"kind":"release-input-values-template","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-values.template.md") -Encoding UTF8 -Value "# Derived release input values template"
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-input-values-validation.json") -Encoding UTF8 -Value '{"kind":"release-input-values-validation","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-command-plan.template-check.json") -Encoding UTF8 -Value '{"kind":"release-command-plan","derived":true}'
Set-Content -LiteralPath (Join-Path $evidenceRoot "release-command-plan.template-check.md") -Encoding UTF8 -Value "# Derived release command plan template check"
Set-Content -LiteralPath (Join-Path $reportsRoot "release-readiness-report.json") -Encoding UTF8 -Value '{"kind":"release-readiness-report","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-readiness-report.md") -Encoding UTF8 -Value "# Nested derived report"
Set-Content -LiteralPath (Join-Path $reportsRoot "release-blocker-actions.json") -Encoding UTF8 -Value '{"kind":"release-blocker-actions","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-blocker-actions.md") -Encoding UTF8 -Value "# Nested derived blocker actions"
Set-Content -LiteralPath (Join-Path $reportsRoot "release-blocker-action-validation.json") -Encoding UTF8 -Value '{"kind":"release-blocker-action-validation","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-blockers-external-only-validation.json") -Encoding UTF8 -Value '{"kind":"release-blockers-external-only-validation","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "development-continuation-readiness-validation.json") -Encoding UTF8 -Value '{"kind":"development-continuation-readiness-validation","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-template.json") -Encoding UTF8 -Value '{"kind":"release-input-template","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-template.md") -Encoding UTF8 -Value "# Nested derived release input template"
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-template-validation.json") -Encoding UTF8 -Value '{"kind":"release-input-template-validation","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-values.template.json") -Encoding UTF8 -Value '{"kind":"release-input-values-template","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-values.template.md") -Encoding UTF8 -Value "# Nested derived release input values template"
Set-Content -LiteralPath (Join-Path $reportsRoot "release-input-values-validation.json") -Encoding UTF8 -Value '{"kind":"release-input-values-validation","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-command-plan.template-check.json") -Encoding UTF8 -Value '{"kind":"release-command-plan","derived":true,"nested":true}'
Set-Content -LiteralPath (Join-Path $reportsRoot "release-command-plan.template-check.md") -Encoding UTF8 -Value "# Nested derived release command plan template check"

$manifestPath = Join-Path $evidenceRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $evidenceRoot `
    -OutputPath $manifestPath `
    -ReleaseId "collector-fixture"

$manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath | ConvertFrom-Json
$indexedPaths = @($manifest.files | ForEach-Object { [string]$_.path })

if ($indexedPaths -contains "release-readiness-report.json") {
    throw "Derived release-readiness-report.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-readiness-report.md") {
    throw "Derived release-readiness-report.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-blocker-actions.json") {
    throw "Derived release-blocker-actions.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-blocker-actions.md") {
    throw "Derived release-blocker-actions.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-blocker-action-validation.json") {
    throw "Derived release-blocker-action-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-blockers-external-only-validation.json") {
    throw "Derived release-blockers-external-only-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "development-continuation-readiness-validation.json") {
    throw "Derived development-continuation-readiness-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-template.json") {
    throw "Derived release-input-template.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-template.md") {
    throw "Derived release-input-template.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-template-validation.json") {
    throw "Derived release-input-template-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-values.template.json") {
    throw "Derived release-input-values.template.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-values.template.md") {
    throw "Derived release-input-values.template.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-input-values-validation.json") {
    throw "Derived release-input-values-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-command-plan.template-check.json") {
    throw "Derived release-command-plan.template-check.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "release-command-plan.template-check.md") {
    throw "Derived release-command-plan.template-check.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-readiness-report.json") {
    throw "Nested derived release-readiness-report.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-readiness-report.md") {
    throw "Nested derived release-readiness-report.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-blocker-actions.json") {
    throw "Nested derived release-blocker-actions.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-blocker-actions.md") {
    throw "Nested derived release-blocker-actions.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-blocker-action-validation.json") {
    throw "Nested derived release-blocker-action-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-blockers-external-only-validation.json") {
    throw "Nested derived release-blockers-external-only-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/development-continuation-readiness-validation.json") {
    throw "Nested derived development-continuation-readiness-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-template.json") {
    throw "Nested derived release-input-template.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-template.md") {
    throw "Nested derived release-input-template.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-template-validation.json") {
    throw "Nested derived release-input-template-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-values.template.json") {
    throw "Nested derived release-input-values.template.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-values.template.md") {
    throw "Nested derived release-input-values.template.md must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-input-values-validation.json") {
    throw "Nested derived release-input-values-validation.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-command-plan.template-check.json") {
    throw "Nested derived release-command-plan.template-check.json must not be indexed as production evidence."
}
if ($indexedPaths -contains "reports/release-command-plan.template-check.md") {
    throw "Nested derived release-command-plan.template-check.md must not be indexed as production evidence."
}
if ($indexedPaths -notcontains "security/vulnerability-report.txt") {
    throw "Expected real security evidence to remain indexed."
}
if ($indexedPaths -notcontains "unreal/ai-showcase-automation-fixture.json") {
    throw "Expected AI_Showcase automation evidence to remain indexed."
}

$aiShowcaseRequirement = @($manifest.requirements | Where-Object { $_.key -eq "unreal.ai_showcase_automation" } | Select-Object -First 1)
if (-not $aiShowcaseRequirement) {
    throw "Expected unreal.ai_showcase_automation requirement to be present."
}
if ($aiShowcaseRequirement.status -ne "present") {
    throw "Expected unreal.ai_showcase_automation to be present but got '$($aiShowcaseRequirement.status)'."
}

$failedEvidenceRoot = Join-Path $testRoot "failed-evidence"
$failedUnrealRoot = Join-Path $failedEvidenceRoot "unreal"
New-Item -ItemType Directory -Force -Path $failedUnrealRoot | Out-Null
Set-Content -LiteralPath (Join-Path $failedUnrealRoot "ai-showcase-automation-failed.json") -Encoding UTF8 -Value '{"kind":"ai-showcase-automation","automationReady":false,"testFilter":"DivineBeastsArena.Showcase.AIShowcase"}'
Set-Content -LiteralPath (Join-Path $failedUnrealRoot "ai-showcase-automation-legacy-no-log-summary.json") -Encoding UTF8 -Value '{"kind":"ai-showcase-automation","automationReady":true,"testFilter":"DivineBeastsArena.Showcase.AIShowcase"}'
Set-Content -LiteralPath (Join-Path $failedUnrealRoot "ai-showcase-automation-log-errors.json") -Encoding UTF8 -Value '{"kind":"ai-showcase-automation","automationReady":true,"logErrorCount":2,"logWarningCount":0,"testFilter":"DivineBeastsArena.Showcase.AIShowcase"}'
Set-Content -LiteralPath (Join-Path $failedUnrealRoot "ue-online-validation-failed.json") -Encoding UTF8 -Value '{"kind":"ue-online-validation","status":"failed","safeLogEvidence":{"runtimePlayerJoined":[],"runtimePlayerJoinedOk":[],"clientATravelCompleted":[],"clientBTravelCompleted":[]}}'

$failedManifestPath = Join-Path $failedEvidenceRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $failedEvidenceRoot `
    -OutputPath $failedManifestPath `
    -ReleaseId "collector-failed-fixture"

$failedManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $failedManifestPath | ConvertFrom-Json
$failedAiShowcaseRequirement = @($failedManifest.requirements | Where-Object { $_.key -eq "unreal.ai_showcase_automation" } | Select-Object -First 1)
if ($failedAiShowcaseRequirement.status -ne "incomplete") {
    throw "Expected failed AI_Showcase automation evidence to be incomplete but got '$($failedAiShowcaseRequirement.status)'."
}

$failedOnlineRequirement = @($failedManifest.requirements | Where-Object { $_.key -eq "unreal.online_validation" } | Select-Object -First 1)
if ($failedOnlineRequirement.status -ne "incomplete") {
    throw "Expected failed UE online validation evidence to be incomplete but got '$($failedOnlineRequirement.status)'."
}

$staleAiShowcaseRoot = Join-Path $testRoot "stale-ai-showcase-evidence"
$staleAiShowcaseUnrealRoot = Join-Path $staleAiShowcaseRoot "unreal"
New-Item -ItemType Directory -Force -Path $staleAiShowcaseUnrealRoot | Out-Null
Set-Content -LiteralPath (Join-Path $staleAiShowcaseUnrealRoot "ai-showcase-automation-stale-four-tests.json") -Encoding UTF8 -Value '{"kind":"ai-showcase-automation","automationReady":true,"logErrorCount":0,"requestedTestCount":4,"passedTestCount":4,"testFilter":"DivineBeastsArena.Showcase.AIShowcase"}'

$staleAiShowcaseManifestPath = Join-Path $staleAiShowcaseRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $staleAiShowcaseRoot `
    -OutputPath $staleAiShowcaseManifestPath `
    -ReleaseId "collector-stale-ai-showcase-fixture"

$staleAiShowcaseManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $staleAiShowcaseManifestPath | ConvertFrom-Json
$staleAiShowcaseRequirement = @($staleAiShowcaseManifest.requirements | Where-Object { $_.key -eq "unreal.ai_showcase_automation" } | Select-Object -First 1)
if ($staleAiShowcaseRequirement.status -ne "incomplete") {
    throw "Expected stale 4/4 AI_Showcase automation evidence to be incomplete but got '$($staleAiShowcaseRequirement.status)'."
}

$vulnerableNpmAuditRoot = Join-Path $testRoot "vulnerable-npm-audit-evidence"
$vulnerableNpmAuditSecurityRoot = Join-Path $vulnerableNpmAuditRoot "security"
New-Item -ItemType Directory -Force -Path $vulnerableNpmAuditSecurityRoot | Out-Null
Set-Content -LiteralPath (Join-Path $vulnerableNpmAuditSecurityRoot "npm-audit-admin-vulnerable.json") -Encoding UTF8 -Value '{"auditReportVersion":2,"metadata":{"vulnerabilities":{"info":0,"low":0,"moderate":0,"high":1,"critical":0,"total":1}},"vulnerabilities":{"fixture":{"severity":"high"}}}'

$vulnerableNpmAuditManifestPath = Join-Path $vulnerableNpmAuditRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $vulnerableNpmAuditRoot `
    -OutputPath $vulnerableNpmAuditManifestPath `
    -ReleaseId "collector-vulnerable-npm-audit-fixture"

$vulnerableNpmAuditManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $vulnerableNpmAuditManifestPath | ConvertFrom-Json
$vulnerableNpmAuditRequirement = @($vulnerableNpmAuditManifest.requirements | Where-Object { $_.key -eq "security.npm" } | Select-Object -First 1)
if ($vulnerableNpmAuditRequirement.status -ne "incomplete") {
    throw "Expected vulnerable npm audit evidence to be incomplete but got '$($vulnerableNpmAuditRequirement.status)'."
}

$cleanNpmAuditRoot = Join-Path $testRoot "clean-npm-audit-evidence"
$cleanNpmAuditSecurityRoot = Join-Path $cleanNpmAuditRoot "security"
New-Item -ItemType Directory -Force -Path $cleanNpmAuditSecurityRoot | Out-Null
$cleanNpmAuditJson = '{"auditReportVersion":2,"metadata":{"vulnerabilities":{"info":0,"low":0,"moderate":0,"high":0,"critical":0,"total":0}},"vulnerabilities":{}}'
Set-Content -LiteralPath (Join-Path $cleanNpmAuditSecurityRoot "npm-audit-admin-clean.json") -Encoding UTF8 -Value $cleanNpmAuditJson
Set-Content -LiteralPath (Join-Path $cleanNpmAuditSecurityRoot "npm-audit-website-clean.json") -Encoding UTF8 -Value $cleanNpmAuditJson
Set-Content -LiteralPath (Join-Path $cleanNpmAuditSecurityRoot "npm-audit-launcher-clean.json") -Encoding UTF8 -Value $cleanNpmAuditJson

$cleanNpmAuditManifestPath = Join-Path $cleanNpmAuditRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanNpmAuditRoot `
    -OutputPath $cleanNpmAuditManifestPath `
    -ReleaseId "collector-clean-npm-audit-fixture"

$cleanNpmAuditManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanNpmAuditManifestPath | ConvertFrom-Json
$cleanNpmAuditRequirement = @($cleanNpmAuditManifest.requirements | Where-Object { $_.key -eq "security.npm" } | Select-Object -First 1)
if ($cleanNpmAuditRequirement.status -ne "present") {
    throw "Expected clean Admin/Website/Launcher npm audit evidence to be present but got '$($cleanNpmAuditRequirement.status)'."
}

$vulnerableTrivyRoot = Join-Path $testRoot "vulnerable-trivy-evidence"
$vulnerableTrivySecurityRoot = Join-Path $vulnerableTrivyRoot "security"
New-Item -ItemType Directory -Force -Path $vulnerableTrivySecurityRoot | Out-Null
Set-Content -LiteralPath (Join-Path $vulnerableTrivySecurityRoot "trivy-api-vulnerable.sarif") -Encoding UTF8 -Value '{"version":"2.1.0","runs":[{"tool":{"driver":{"name":"Trivy"}},"results":[{"ruleId":"CVE-TEST-HIGH","level":"error","message":{"text":"Fixture HIGH vulnerability"}}]}]}'

$vulnerableTrivyManifestPath = Join-Path $vulnerableTrivyRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $vulnerableTrivyRoot `
    -OutputPath $vulnerableTrivyManifestPath `
    -ReleaseId "collector-vulnerable-trivy-fixture"

$vulnerableTrivyManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $vulnerableTrivyManifestPath | ConvertFrom-Json
$vulnerableTrivyRequirement = @($vulnerableTrivyManifest.requirements | Where-Object { $_.key -eq "security.trivy" } | Select-Object -First 1)
if ($vulnerableTrivyRequirement.status -ne "incomplete") {
    throw "Expected vulnerable Trivy evidence to be incomplete but got '$($vulnerableTrivyRequirement.status)'."
}

$cleanTrivyRoot = Join-Path $testRoot "clean-trivy-evidence"
$cleanTrivySecurityRoot = Join-Path $cleanTrivyRoot "security"
New-Item -ItemType Directory -Force -Path $cleanTrivySecurityRoot | Out-Null
$cleanTrivySarif = '{"version":"2.1.0","runs":[{"tool":{"driver":{"name":"Trivy"}},"results":[]}]}'
Set-Content -LiteralPath (Join-Path $cleanTrivySecurityRoot "trivy-api-clean.sarif") -Encoding UTF8 -Value $cleanTrivySarif
Set-Content -LiteralPath (Join-Path $cleanTrivySecurityRoot "trivy-worker-clean.sarif") -Encoding UTF8 -Value $cleanTrivySarif

$cleanTrivyManifestPath = Join-Path $cleanTrivyRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanTrivyRoot `
    -OutputPath $cleanTrivyManifestPath `
    -ReleaseId "collector-clean-trivy-fixture"

$cleanTrivyManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanTrivyManifestPath | ConvertFrom-Json
$cleanTrivyRequirement = @($cleanTrivyManifest.requirements | Where-Object { $_.key -eq "security.trivy" } | Select-Object -First 1)
if ($cleanTrivyRequirement.status -ne "present") {
    throw "Expected clean API/Worker Trivy evidence to be present but got '$($cleanTrivyRequirement.status)'."
}

$vulnerableNuGetRoot = Join-Path $testRoot "vulnerable-nuget-evidence"
$vulnerableNuGetSecurityRoot = Join-Path $vulnerableNuGetRoot "security"
New-Item -ItemType Directory -Force -Path $vulnerableNuGetSecurityRoot | Out-Null
Set-Content -LiteralPath (Join-Path $vulnerableNuGetSecurityRoot "vulnerability-report.txt") -Encoding UTF8 -Value 'Project `Game.Api` has the following vulnerable packages. Newtonsoft.Json 1.0.0 high'

$vulnerableNuGetManifestPath = Join-Path $vulnerableNuGetRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $vulnerableNuGetRoot `
    -OutputPath $vulnerableNuGetManifestPath `
    -ReleaseId "collector-vulnerable-nuget-fixture"

$vulnerableNuGetManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $vulnerableNuGetManifestPath | ConvertFrom-Json
$vulnerableNuGetRequirement = @($vulnerableNuGetManifest.requirements | Where-Object { $_.key -eq "security.nuget" } | Select-Object -First 1)
if ($vulnerableNuGetRequirement.status -ne "incomplete") {
    throw "Expected vulnerable NuGet evidence to be incomplete but got '$($vulnerableNuGetRequirement.status)'."
}

$cleanNuGetRoot = Join-Path $testRoot "clean-nuget-evidence"
$cleanNuGetSecurityRoot = Join-Path $cleanNuGetRoot "security"
New-Item -ItemType Directory -Force -Path $cleanNuGetSecurityRoot | Out-Null
Set-Content -LiteralPath (Join-Path $cleanNuGetSecurityRoot "vulnerability-report.txt") -Encoding UTF8 -Value 'Project `Game.Api` has no vulnerable packages. PASS: NuGet vulnerable package audit'

$cleanNuGetManifestPath = Join-Path $cleanNuGetRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanNuGetRoot `
    -OutputPath $cleanNuGetManifestPath `
    -ReleaseId "collector-clean-nuget-fixture"

$cleanNuGetManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanNuGetManifestPath | ConvertFrom-Json
$cleanNuGetRequirement = @($cleanNuGetManifest.requirements | Where-Object { $_.key -eq "security.nuget" } | Select-Object -First 1)
if ($cleanNuGetRequirement.status -ne "present") {
    throw "Expected clean NuGet evidence to be present but got '$($cleanNuGetRequirement.status)'."
}

$failedK6Root = Join-Path $testRoot "failed-k6-evidence"
$failedK6LoadRoot = Join-Path $failedK6Root "load"
New-Item -ItemType Directory -Force -Path $failedK6LoadRoot | Out-Null
Set-Content -LiteralPath (Join-Path $failedK6LoadRoot "k6-login-failed.json") -Encoding UTF8 -Value '{"metrics":{"checks":{"passes":0,"fails":1,"value":0},"http_req_failed":{"value":0.5},"http_reqs":{"count":2},"iterations":{"count":1}}}'

$failedK6ManifestPath = Join-Path $failedK6Root "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $failedK6Root `
    -OutputPath $failedK6ManifestPath `
    -ReleaseId "collector-failed-k6-fixture"

$failedK6Manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $failedK6ManifestPath | ConvertFrom-Json
$failedK6Requirement = @($failedK6Manifest.requirements | Where-Object { $_.key -eq "load.k6" } | Select-Object -First 1)
if ($failedK6Requirement.status -ne "incomplete") {
    throw "Expected failed k6 evidence to be incomplete but got '$($failedK6Requirement.status)'."
}

$cleanK6Root = Join-Path $testRoot "clean-k6-evidence"
$cleanK6LoadRoot = Join-Path $cleanK6Root "load"
New-Item -ItemType Directory -Force -Path $cleanK6LoadRoot | Out-Null
$cleanK6SummaryJson = '{"metrics":{"checks":{"passes":4,"fails":0,"value":1},"http_req_failed":{"value":0},"http_reqs":{"count":2},"iterations":{"count":1}}}'
Set-Content -LiteralPath (Join-Path $cleanK6LoadRoot "k6-login-clean.json") -Encoding UTF8 -Value $cleanK6SummaryJson
Set-Content -LiteralPath (Join-Path $cleanK6LoadRoot "k6-matchmaking-clean.json") -Encoding UTF8 -Value $cleanK6SummaryJson

$cleanK6ManifestPath = Join-Path $cleanK6Root "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanK6Root `
    -OutputPath $cleanK6ManifestPath `
    -ReleaseId "collector-clean-k6-fixture"

$cleanK6Manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanK6ManifestPath | ConvertFrom-Json
$cleanK6Requirement = @($cleanK6Manifest.requirements | Where-Object { $_.key -eq "load.k6" } | Select-Object -First 1)
if ($cleanK6Requirement.status -ne "present") {
    throw "Expected clean login/matchmaking k6 evidence to be present but got '$($cleanK6Requirement.status)'."
}

$clientDownloadOnlyRoot = Join-Path $testRoot "client-download-only-evidence"
$clientDownloadOnlyClientRoot = Join-Path $clientDownloadOnlyRoot "client"
New-Item -ItemType Directory -Force -Path $clientDownloadOnlyClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $clientDownloadOnlyClientRoot "download-manifest.json") -Encoding UTF8 -Value '{"kind":"client-download-metadata","downloadUrl":"https://cdn.divinebeastsarena.invalid/releases/1.0.0.0/DivineBeastsArena.zip"}'

$clientDownloadOnlyManifestPath = Join-Path $clientDownloadOnlyRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $clientDownloadOnlyRoot `
    -OutputPath $clientDownloadOnlyManifestPath `
    -ReleaseId "collector-client-download-only-fixture"

$clientDownloadOnlyManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $clientDownloadOnlyManifestPath | ConvertFrom-Json
$clientDownloadOnlyK6Requirement = @($clientDownloadOnlyManifest.requirements | Where-Object { $_.key -eq "load.k6" } | Select-Object -First 1)
if ($clientDownloadOnlyK6Requirement.status -ne "missing") {
    throw "Expected client download metadata not to match load.k6 but got '$($clientDownloadOnlyK6Requirement.status)'."
}
if ($null -ne $clientDownloadOnlyK6Requirement.files -and @($clientDownloadOnlyK6Requirement.files).Count -ne 0) {
    throw "Expected client download metadata not to be listed under load.k6 files."
}

$mixedK6Root = Join-Path $testRoot "mixed-k6-evidence"
$mixedK6LoadRoot = Join-Path $mixedK6Root "load"
New-Item -ItemType Directory -Force -Path $mixedK6LoadRoot | Out-Null
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-login-failed.json") -Encoding UTF8 -Value '{"metrics":{"checks":{"passes":0,"fails":1,"value":0},"http_req_failed":{"value":0.25},"http_reqs":{"count":2},"iterations":{"count":1}}}'
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-login-failed.log") -Encoding UTF8 -Value 'failed login k6 fixture'
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-login-clean.json") -Encoding UTF8 -Value $cleanK6SummaryJson
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-login-clean.log") -Encoding UTF8 -Value 'clean login k6 fixture'
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-login-clean.meta.txt") -Encoding UTF8 -Value 'metadata should not support present load.k6'
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-matchmaking-clean.json") -Encoding UTF8 -Value $cleanK6SummaryJson
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "k6-matchmaking-clean.log") -Encoding UTF8 -Value 'clean matchmaking k6 fixture'
Set-Content -LiteralPath (Join-Path $mixedK6LoadRoot "dedicated-server-orchestration-skipped.txt") -Encoding UTF8 -Value 'skipped dedicated server orchestration should not support present load.k6'

$mixedK6ManifestPath = Join-Path $mixedK6Root "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $mixedK6Root `
    -OutputPath $mixedK6ManifestPath `
    -ReleaseId "collector-mixed-k6-fixture"

$mixedK6Manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $mixedK6ManifestPath | ConvertFrom-Json
$mixedK6Requirement = @($mixedK6Manifest.requirements | Where-Object { $_.key -eq "load.k6" } | Select-Object -First 1)
if ($mixedK6Requirement.status -ne "present") {
    throw "Expected mixed k6 evidence to be present because clean login and matchmaking summaries exist but got '$($mixedK6Requirement.status)'."
}
if (@($mixedK6Requirement.files | Where-Object { $_ -match "failed|skipped|\\.meta\\.txt" }).Count -ne 0) {
    throw "Expected failed, skipped, and metadata k6 files to be excluded from present load.k6 support files."
}
if (@($mixedK6Requirement.files | Where-Object { $_ -match "k6-login-clean|k6-matchmaking-clean" }).Count -ne 4) {
    throw "Expected clean login/matchmaking k6 JSON and log files to support present load.k6."
}

$failedBackupRestoreRoot = Join-Path $testRoot "failed-backup-restore-evidence"
$failedBackupRestoreOpsRoot = Join-Path $failedBackupRestoreRoot "ops"
New-Item -ItemType Directory -Force -Path $failedBackupRestoreOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $failedBackupRestoreOpsRoot "backup-restore-rehearsal-failed.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"failed-fixture","status":"failed","exitCode":1,"backupFile":"./backups/postgres-game_platform-failed.sql.gz","restoreDatabase":"","publicTableCount":"","logFile":"backup-restore-rehearsal-failed.log"}'

$failedBackupRestoreManifestPath = Join-Path $failedBackupRestoreRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $failedBackupRestoreRoot `
    -OutputPath $failedBackupRestoreManifestPath `
    -ReleaseId "collector-failed-backup-restore-fixture"

$failedBackupRestoreManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $failedBackupRestoreManifestPath | ConvertFrom-Json
$failedBackupRestoreRequirement = @($failedBackupRestoreManifest.requirements | Where-Object { $_.key -eq "ops.backup_restore" } | Select-Object -First 1)
if ($failedBackupRestoreRequirement.status -ne "incomplete") {
    throw "Expected failed backup restore rehearsal evidence to be incomplete but got '$($failedBackupRestoreRequirement.status)'."
}

$incompleteBackupRestoreRoot = Join-Path $testRoot "incomplete-backup-restore-evidence"
$incompleteBackupRestoreOpsRoot = Join-Path $incompleteBackupRestoreRoot "ops"
New-Item -ItemType Directory -Force -Path $incompleteBackupRestoreOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $incompleteBackupRestoreOpsRoot "backup-restore-rehearsal-no-restore-db.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"incomplete-fixture","status":"passed","exitCode":0,"backupFile":"./backups/postgres-game_platform-incomplete.sql.gz","restoreDatabase":"","publicTableCount":"51","logFile":"backup-restore-rehearsal-no-restore-db.log"}'

$incompleteBackupRestoreManifestPath = Join-Path $incompleteBackupRestoreRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $incompleteBackupRestoreRoot `
    -OutputPath $incompleteBackupRestoreManifestPath `
    -ReleaseId "collector-incomplete-backup-restore-fixture"

$incompleteBackupRestoreManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $incompleteBackupRestoreManifestPath | ConvertFrom-Json
$incompleteBackupRestoreRequirement = @($incompleteBackupRestoreManifest.requirements | Where-Object { $_.key -eq "ops.backup_restore" } | Select-Object -First 1)
if ($incompleteBackupRestoreRequirement.status -ne "incomplete") {
    throw "Expected backup restore evidence without a restore database to be incomplete but got '$($incompleteBackupRestoreRequirement.status)'."
}

$cleanBackupRestoreRoot = Join-Path $testRoot "clean-backup-restore-evidence"
$cleanBackupRestoreOpsRoot = Join-Path $cleanBackupRestoreRoot "ops"
New-Item -ItemType Directory -Force -Path $cleanBackupRestoreOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $cleanBackupRestoreOpsRoot "backup-restore-rehearsal-clean.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"clean-fixture","status":"passed","exitCode":0,"backupFile":"./backups/postgres-game_platform-clean.sql.gz","restoreDatabase":"game_platform_restore_rehearsal_20260630000000","publicTableCount":"51","logFile":"backup-restore-rehearsal-clean.log"}'

$cleanBackupRestoreManifestPath = Join-Path $cleanBackupRestoreRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanBackupRestoreRoot `
    -OutputPath $cleanBackupRestoreManifestPath `
    -ReleaseId "collector-clean-backup-restore-fixture"

$cleanBackupRestoreManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanBackupRestoreManifestPath | ConvertFrom-Json
$cleanBackupRestoreRequirement = @($cleanBackupRestoreManifest.requirements | Where-Object { $_.key -eq "ops.backup_restore" } | Select-Object -First 1)
if ($cleanBackupRestoreRequirement.status -ne "present") {
    throw "Expected clean backup restore rehearsal evidence to be present but got '$($cleanBackupRestoreRequirement.status)'."
}

$mixedBackupRestoreRoot = Join-Path $testRoot "mixed-backup-restore-evidence"
$mixedBackupRestoreOpsRoot = Join-Path $mixedBackupRestoreRoot "ops"
New-Item -ItemType Directory -Force -Path $mixedBackupRestoreOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $mixedBackupRestoreOpsRoot "backup-restore-rehearsal-incomplete.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"mixed-incomplete","status":"passed","exitCode":0,"backupFile":"./backups/postgres-game_platform-incomplete.sql.gz","restoreDatabase":"","publicTableCount":"51","logFile":"backup-restore-rehearsal-incomplete.log"}'
Set-Content -LiteralPath (Join-Path $mixedBackupRestoreOpsRoot "backup-restore-rehearsal-incomplete.log") -Encoding UTF8 -Value 'incomplete backup restore fixture'
Set-Content -LiteralPath (Join-Path $mixedBackupRestoreOpsRoot "backup-restore-rehearsal-clean.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"mixed-clean","status":"passed","exitCode":0,"backupFile":"./backups/postgres-game_platform-clean.sql.gz","restoreDatabase":"game_platform_restore_rehearsal_20260630010101","publicTableCount":"51","logFile":"backup-restore-rehearsal-clean.log"}'
Set-Content -LiteralPath (Join-Path $mixedBackupRestoreOpsRoot "backup-restore-rehearsal-clean.log") -Encoding UTF8 -Value 'clean backup restore fixture'

$mixedBackupRestoreManifestPath = Join-Path $mixedBackupRestoreRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $mixedBackupRestoreRoot `
    -OutputPath $mixedBackupRestoreManifestPath `
    -ReleaseId "collector-mixed-backup-restore-fixture"

$mixedBackupRestoreManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $mixedBackupRestoreManifestPath | ConvertFrom-Json
$mixedBackupRestoreRequirement = @($mixedBackupRestoreManifest.requirements | Where-Object { $_.key -eq "ops.backup_restore" } | Select-Object -First 1)
if ($mixedBackupRestoreRequirement.status -ne "present") {
    throw "Expected mixed backup restore evidence to be present because a clean rehearsal exists but got '$($mixedBackupRestoreRequirement.status)'."
}
if (@($mixedBackupRestoreRequirement.files | Where-Object { $_ -match "incomplete" }).Count -ne 0) {
    throw "Expected incomplete backup restore files to be excluded from present ops.backup_restore support files."
}
if (@($mixedBackupRestoreRequirement.files | Where-Object { $_ -match "backup-restore-rehearsal-clean" }).Count -ne 2) {
    throw "Expected clean backup restore JSON and log to support present ops.backup_restore."
}

$failedDeployRollbackRoot = Join-Path $testRoot "failed-deploy-rollback-evidence"
$failedDeployRollbackOpsRoot = Join-Path $failedDeployRollbackRoot "ops"
New-Item -ItemType Directory -Force -Path $failedDeployRollbackOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $failedDeployRollbackOpsRoot "production-smoke-backend-failed.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"failed-smoke","status":"failed","exitCode":1,"baseUrl":"http://localhost:8080","guestLogin":true,"logFile":"production-smoke-backend-failed.log","checks":[{"name":"live health","status":"passed"},{"name":"ready health","status":"failed"}]}'

$failedDeployRollbackManifestPath = Join-Path $failedDeployRollbackRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $failedDeployRollbackRoot `
    -OutputPath $failedDeployRollbackManifestPath `
    -ReleaseId "collector-failed-deploy-rollback-fixture"

$failedDeployRollbackManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $failedDeployRollbackManifestPath | ConvertFrom-Json
$failedDeployRollbackRequirement = @($failedDeployRollbackManifest.requirements | Where-Object { $_.key -eq "ops.deploy_rollback" } | Select-Object -First 1)
if ($failedDeployRollbackRequirement.status -ne "incomplete") {
    throw "Expected failed deploy rollback smoke evidence to be incomplete but got '$($failedDeployRollbackRequirement.status)'."
}

$partialDeployRollbackRoot = Join-Path $testRoot "partial-deploy-rollback-evidence"
$partialDeployRollbackOpsRoot = Join-Path $partialDeployRollbackRoot "ops"
New-Item -ItemType Directory -Force -Path $partialDeployRollbackOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $partialDeployRollbackOpsRoot "production-smoke-backend-missing-metrics.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"partial-smoke","status":"passed","exitCode":0,"baseUrl":"http://localhost:8080","guestLogin":true,"logFile":"production-smoke-backend-missing-metrics.log","checks":[{"name":"live health","status":"passed"},{"name":"ready health","status":"passed"},{"name":"version api","status":"passed"},{"name":"launcher manifest","status":"passed"},{"name":"guest login","status":"passed"}]}'

$partialDeployRollbackManifestPath = Join-Path $partialDeployRollbackRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $partialDeployRollbackRoot `
    -OutputPath $partialDeployRollbackManifestPath `
    -ReleaseId "collector-partial-deploy-rollback-fixture"

$partialDeployRollbackManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $partialDeployRollbackManifestPath | ConvertFrom-Json
$partialDeployRollbackRequirement = @($partialDeployRollbackManifest.requirements | Where-Object { $_.key -eq "ops.deploy_rollback" } | Select-Object -First 1)
if ($partialDeployRollbackRequirement.status -ne "incomplete") {
    throw "Expected deploy rollback smoke evidence without metrics check to be incomplete but got '$($partialDeployRollbackRequirement.status)'."
}

$cleanDeployRollbackRoot = Join-Path $testRoot "clean-deploy-rollback-evidence"
$cleanDeployRollbackOpsRoot = Join-Path $cleanDeployRollbackRoot "ops"
New-Item -ItemType Directory -Force -Path $cleanDeployRollbackOpsRoot | Out-Null
Set-Content -LiteralPath (Join-Path $cleanDeployRollbackOpsRoot "production-smoke-backend-clean.json") -Encoding UTF8 -Value '{"schemaVersion":"1.0","runId":"clean-smoke","status":"passed","exitCode":0,"baseUrl":"http://localhost:8080","guestLogin":true,"logFile":"production-smoke-backend-clean.log","checks":[{"name":"live health","status":"passed"},{"name":"ready health","status":"passed"},{"name":"version api","status":"passed"},{"name":"launcher manifest","status":"passed"},{"name":"metrics endpoint","status":"passed"},{"name":"guest login","status":"passed"}]}'

$cleanDeployRollbackManifestPath = Join-Path $cleanDeployRollbackRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $cleanDeployRollbackRoot `
    -OutputPath $cleanDeployRollbackManifestPath `
    -ReleaseId "collector-clean-deploy-rollback-fixture"

$cleanDeployRollbackManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $cleanDeployRollbackManifestPath | ConvertFrom-Json
$cleanDeployRollbackRequirement = @($cleanDeployRollbackManifest.requirements | Where-Object { $_.key -eq "ops.deploy_rollback" } | Select-Object -First 1)
if ($cleanDeployRollbackRequirement.status -ne "present") {
    throw "Expected clean deploy rollback smoke evidence to be present but got '$($cleanDeployRollbackRequirement.status)'."
}

$inconsistentReleasePrerequisiteRoot = Join-Path $testRoot "inconsistent-release-prerequisites-evidence"
$inconsistentReleasePrerequisiteClientRoot = Join-Path $inconsistentReleasePrerequisiteRoot "client"
New-Item -ItemType Directory -Force -Path $inconsistentReleasePrerequisiteClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $inconsistentReleasePrerequisiteClientRoot "client-release-prerequisites-inconsistent.json") -Encoding UTF8 -Value '{"kind":"client-release-prerequisites","readyForReleaseInputs":true,"blockingIssueCount":1,"blockingIssues":[{"code":"signing_identity_missing","message":"Signing identity is required."}],"package":{"clientExePath":"C:/tmp/DivineBeastsArena.exe","fileCount":1},"urls":{"downloadUrl":{"value":"https://","valid":true,"isHttps":true,"isExample":false,"isAllowedLocalHttp":false},"manifestUrl":{"value":"https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json","valid":true,"isHttps":true,"isExample":false,"isAllowedLocalHttp":false}},"signing":{"required":true,"certificateThumbprintProvided":true,"certificateSubjectProvided":false,"pfxPath":"","certificateFound":false,"signToolPath":""}}'

$inconsistentReleasePrerequisiteManifestPath = Join-Path $inconsistentReleasePrerequisiteRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $inconsistentReleasePrerequisiteRoot `
    -OutputPath $inconsistentReleasePrerequisiteManifestPath `
    -ReleaseId "collector-inconsistent-release-prerequisites-fixture"

$inconsistentReleasePrerequisiteManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $inconsistentReleasePrerequisiteManifestPath | ConvertFrom-Json
$inconsistentReleasePrerequisiteRequirement = @($inconsistentReleasePrerequisiteManifest.requirements | Where-Object { $_.key -eq "client.release_prerequisites" } | Select-Object -First 1)
if ($inconsistentReleasePrerequisiteRequirement.status -ne "incomplete") {
    throw "Expected inconsistent release prerequisite evidence to be incomplete but got '$($inconsistentReleasePrerequisiteRequirement.status)'."
}

$hostlessPackageRoot = Join-Path $testRoot "hostless-package-evidence"
$hostlessPackageClientRoot = Join-Path $hostlessPackageRoot "client"
New-Item -ItemType Directory -Force -Path $hostlessPackageClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $hostlessPackageClientRoot "client-package-launcher-hostless.json") -Encoding UTF8 -Value '{"kind":"client-package-launcher","releaseReady":true,"downloadUrl":"https://","downloadUrlIsHttps":true,"downloadUrlIsExample":false}'

$hostlessPackageManifestPath = Join-Path $hostlessPackageRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $hostlessPackageRoot `
    -OutputPath $hostlessPackageManifestPath `
    -ReleaseId "collector-hostless-package-fixture"

$hostlessPackageManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $hostlessPackageManifestPath | ConvertFrom-Json
$hostlessPackageRequirement = @($hostlessPackageManifest.requirements | Where-Object { $_.key -eq "client.package_launcher" } | Select-Object -First 1)
if ($hostlessPackageRequirement.status -ne "incomplete") {
    throw "Expected hostless package launcher evidence to be incomplete but got '$($hostlessPackageRequirement.status)'."
}

$hostlessCdnSmokeRoot = Join-Path $testRoot "hostless-cdn-smoke-evidence"
$hostlessCdnSmokeClientRoot = Join-Path $hostlessCdnSmokeRoot "client"
New-Item -ItemType Directory -Force -Path $hostlessCdnSmokeClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $hostlessCdnSmokeClientRoot "launcher-cdn-smoke-hostless.json") -Encoding UTF8 -Value '{"kind":"launcher-cdn-smoke","cdnReady":true,"ManifestUrl":"https://","downloadUrl":"https://cdn.divinebeastsarena.invalid/releases/1.0.0.0/","manifestUrlIsHttps":true,"downloadUrlIsHttps":true,"manifestUrlIsExample":false,"downloadUrlIsExample":false}'

$hostlessCdnSmokeManifestPath = Join-Path $hostlessCdnSmokeRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $hostlessCdnSmokeRoot `
    -OutputPath $hostlessCdnSmokeManifestPath `
    -ReleaseId "collector-hostless-cdn-smoke-fixture"

$hostlessCdnSmokeManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $hostlessCdnSmokeManifestPath | ConvertFrom-Json
$hostlessCdnSmokeRequirement = @($hostlessCdnSmokeManifest.requirements | Where-Object { $_.key -eq "client.cdn_launcher_smoke" } | Select-Object -First 1)
if ($hostlessCdnSmokeRequirement.status -ne "incomplete") {
    throw "Expected hostless launcher CDN smoke evidence to be incomplete but got '$($hostlessCdnSmokeRequirement.status)'."
}

$inconsistentSigningRoot = Join-Path $testRoot "inconsistent-code-signing-evidence"
$inconsistentSigningClientRoot = Join-Path $inconsistentSigningRoot "client"
New-Item -ItemType Directory -Force -Path $inconsistentSigningClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $inconsistentSigningClientRoot "code-signing-inconsistent.json") -Encoding UTF8 -Value '{"kind":"code-signing","signingReady":true,"signableFileCount":2,"signedFileCount":1,"trustedSignedFileCount":1,"unsignedFileCount":1,"invalidSignedFileCount":0}'

$inconsistentSigningManifestPath = Join-Path $inconsistentSigningRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $inconsistentSigningRoot `
    -OutputPath $inconsistentSigningManifestPath `
    -ReleaseId "collector-inconsistent-code-signing-fixture"

$inconsistentSigningManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $inconsistentSigningManifestPath | ConvertFrom-Json
$inconsistentSigningRequirement = @($inconsistentSigningManifest.requirements | Where-Object { $_.key -eq "client.code_signing" } | Select-Object -First 1)
if ($inconsistentSigningRequirement.status -ne "incomplete") {
    throw "Expected inconsistent code signing evidence to be incomplete but got '$($inconsistentSigningRequirement.status)'."
}

$inconsistentLauncherSmokeRoot = Join-Path $testRoot "inconsistent-launcher-install-update-evidence"
$inconsistentLauncherSmokeClientRoot = Join-Path $inconsistentLauncherSmokeRoot "client"
New-Item -ItemType Directory -Force -Path $inconsistentLauncherSmokeClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $inconsistentLauncherSmokeClientRoot "launcher-install-update-smoke-inconsistent.json") -Encoding UTF8 -Value '{"kind":"launcher-install-update-smoke","installUpdateReady":true,"hashVerified":true,"versionPersisted":true,"exitCode":1,"testName":"repair_game_downloads_local_package_and_persists_version"}'

$inconsistentLauncherSmokeManifestPath = Join-Path $inconsistentLauncherSmokeRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $inconsistentLauncherSmokeRoot `
    -OutputPath $inconsistentLauncherSmokeManifestPath `
    -ReleaseId "collector-inconsistent-launcher-install-update-fixture"

$inconsistentLauncherSmokeManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $inconsistentLauncherSmokeManifestPath | ConvertFrom-Json
$inconsistentLauncherSmokeRequirement = @($inconsistentLauncherSmokeManifest.requirements | Where-Object { $_.key -eq "client.launcher_install_update" } | Select-Object -First 1)
if ($inconsistentLauncherSmokeRequirement.status -ne "incomplete") {
    throw "Expected inconsistent launcher install/update evidence to be incomplete but got '$($inconsistentLauncherSmokeRequirement.status)'."
}

$mixedLauncherSmokeRoot = Join-Path $testRoot "mixed-launcher-install-update-evidence"
$mixedLauncherSmokeClientRoot = Join-Path $mixedLauncherSmokeRoot "client"
New-Item -ItemType Directory -Force -Path $mixedLauncherSmokeClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-failed.json") -Encoding UTF8 -Value '{"kind":"launcher-install-update-smoke","installUpdateReady":true,"hashVerified":true,"versionPersisted":true,"exitCode":1,"testName":"repair_game_downloads_local_package_and_persists_version"}'
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-failed.log") -Encoding UTF8 -Value "launcher install update failed"
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-failed.stderr.log") -Encoding UTF8 -Value "error"
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-clean.json") -Encoding UTF8 -Value '{"kind":"launcher-install-update-smoke","installUpdateReady":true,"hashVerified":true,"versionPersisted":true,"exitCode":0,"testName":"repair_game_downloads_local_package_and_persists_version"}'
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-clean.log") -Encoding UTF8 -Value "launcher install update passed"
Set-Content -LiteralPath (Join-Path $mixedLauncherSmokeClientRoot "launcher-install-update-smoke-clean.stderr.log") -Encoding UTF8 -Value ""

$mixedLauncherSmokeManifestPath = Join-Path $mixedLauncherSmokeRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $mixedLauncherSmokeRoot `
    -OutputPath $mixedLauncherSmokeManifestPath `
    -ReleaseId "collector-mixed-launcher-install-update-fixture"

$mixedLauncherSmokeManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $mixedLauncherSmokeManifestPath | ConvertFrom-Json
$mixedLauncherSmokeRequirement = @($mixedLauncherSmokeManifest.requirements | Where-Object { $_.key -eq "client.launcher_install_update" } | Select-Object -First 1)
if ($mixedLauncherSmokeRequirement.status -ne "present") {
    throw "Expected mixed launcher install/update evidence to be present because one clean smoke result exists but got '$($mixedLauncherSmokeRequirement.status)'."
}

$mixedLauncherSmokeFiles = @($mixedLauncherSmokeRequirement.files)
if (@($mixedLauncherSmokeFiles | Where-Object { $_ -match "failed|stderr" }).Count -gt 0) {
    throw "Expected failed and stderr launcher install/update support files to be excluded from present evidence files."
}

if (@($mixedLauncherSmokeFiles | Where-Object { $_ -match "launcher-install-update-smoke-clean\.(json|log)$" }).Count -ne 2) {
    throw "Expected clean launcher install/update JSON and stdout log to be retained as evidence files."
}

$inconsistentLauncherUiRoot = Join-Path $testRoot "inconsistent-launcher-ui-visual-evidence"
$inconsistentLauncherUiClientRoot = Join-Path $inconsistentLauncherUiRoot "client"
New-Item -ItemType Directory -Force -Path $inconsistentLauncherUiClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $inconsistentLauncherUiClientRoot "launcher-ui-visual-evidence-inconsistent.json") -Encoding UTF8 -Value '{"kind":"launcher-ui-visual-evidence","uiEvidenceReady":true,"screenshotReady":true,"uiMarkersReady":true,"buildExitCode":0,"previewStarted":true,"screenshotExitCode":1,"domExitCode":0,"requiredMarkers":["Divine Beasts Arena","launcher-shell","action-bar","primary"],"missingMarkers":[]}'

$inconsistentLauncherUiManifestPath = Join-Path $inconsistentLauncherUiRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $inconsistentLauncherUiRoot `
    -OutputPath $inconsistentLauncherUiManifestPath `
    -ReleaseId "collector-inconsistent-launcher-ui-visual-fixture"

$inconsistentLauncherUiManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $inconsistentLauncherUiManifestPath | ConvertFrom-Json
$inconsistentLauncherUiRequirement = @($inconsistentLauncherUiManifest.requirements | Where-Object { $_.key -eq "client.launcher_ui_visual" } | Select-Object -First 1)
if ($inconsistentLauncherUiRequirement.status -ne "incomplete") {
    throw "Expected inconsistent launcher UI visual evidence to be incomplete but got '$($inconsistentLauncherUiRequirement.status)'."
}

$mixedLauncherUiRoot = Join-Path $testRoot "mixed-launcher-ui-visual-evidence"
$mixedLauncherUiClientRoot = Join-Path $mixedLauncherUiRoot "client"
New-Item -ItemType Directory -Force -Path $mixedLauncherUiClientRoot | Out-Null
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-failed.json") -Encoding UTF8 -Value '{"kind":"launcher-ui-visual-evidence","uiEvidenceReady":true,"screenshotReady":true,"uiMarkersReady":true,"buildExitCode":0,"previewStarted":true,"screenshotExitCode":1,"domExitCode":0,"requiredMarkers":["Divine Beasts Arena","launcher-shell"],"missingMarkers":[]}'
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-failed.png") -Encoding UTF8 -Value "failed screenshot"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-failed.browser.log") -Encoding UTF8 -Value "browser failed"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-failed.browser.stderr.log") -Encoding UTF8 -Value "browser stderr"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.json") -Encoding UTF8 -Value '{"kind":"launcher-ui-visual-evidence","uiEvidenceReady":true,"screenshotReady":true,"uiMarkersReady":true,"buildExitCode":0,"previewStarted":true,"screenshotExitCode":0,"domExitCode":0,"requiredMarkers":["Divine Beasts Arena","launcher-shell"],"missingMarkers":[]}'
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.png") -Encoding UTF8 -Value "clean screenshot"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.browser.log") -Encoding UTF8 -Value "browser passed"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.build.log") -Encoding UTF8 -Value "build passed"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.preview.log") -Encoding UTF8 -Value "preview passed"
Set-Content -LiteralPath (Join-Path $mixedLauncherUiClientRoot "launcher-ui-visual-evidence-clean.browser.stderr.log") -Encoding UTF8 -Value ""

$mixedLauncherUiManifestPath = Join-Path $mixedLauncherUiRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $mixedLauncherUiRoot `
    -OutputPath $mixedLauncherUiManifestPath `
    -ReleaseId "collector-mixed-launcher-ui-visual-fixture"

$mixedLauncherUiManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $mixedLauncherUiManifestPath | ConvertFrom-Json
$mixedLauncherUiRequirement = @($mixedLauncherUiManifest.requirements | Where-Object { $_.key -eq "client.launcher_ui_visual" } | Select-Object -First 1)
if ($mixedLauncherUiRequirement.status -ne "present") {
    throw "Expected mixed launcher UI visual evidence to be present because one clean UI report exists but got '$($mixedLauncherUiRequirement.status)'."
}

$mixedLauncherUiFiles = @($mixedLauncherUiRequirement.files)
if (@($mixedLauncherUiFiles | Where-Object { $_ -match "failed|stderr" }).Count -gt 0) {
    throw "Expected failed and stderr launcher UI visual support files to be excluded from present evidence files."
}

if (@($mixedLauncherUiFiles | Where-Object { $_ -match "launcher-ui-visual-evidence-clean\.(json|png|browser\.log|build\.log|preview\.log)$" }).Count -ne 5) {
    throw "Expected clean launcher UI visual JSON, screenshot, and non-stderr logs to be retained as evidence files."
}

$inconsistentUeOnlineRoot = Join-Path $testRoot "inconsistent-ue-online-evidence"
$inconsistentUeOnlineUnrealRoot = Join-Path $inconsistentUeOnlineRoot "unreal"
New-Item -ItemType Directory -Force -Path $inconsistentUeOnlineUnrealRoot | Out-Null
Set-Content -LiteralPath (Join-Path $inconsistentUeOnlineUnrealRoot "ue-online-validation-inconsistent.json") -Encoding UTF8 -Value '{"kind":"ue-online-validation","status":"passed","skipClientLaunch":false,"roomId":"","sessionId":"","serverId":"","allocatedPort":0,"clientConnectPort":0,"processIds":{"server":0,"clientA":0,"clientB":0},"safeLogEvidence":{"runtimePlayerJoinedOk":["runtime/servers/player-joined A=OK","runtime/servers/player-joined B=OK"],"clientATravelCompleted":["TravelCompleted Pending net game travel completed"],"clientBTravelCompleted":["TravelCompleted Pending net game travel completed"]}}'

$inconsistentUeOnlineManifestPath = Join-Path $inconsistentUeOnlineRoot "production-evidence-manifest.json"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $inconsistentUeOnlineRoot `
    -OutputPath $inconsistentUeOnlineManifestPath `
    -ReleaseId "collector-inconsistent-ue-online-fixture"

$inconsistentUeOnlineManifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $inconsistentUeOnlineManifestPath | ConvertFrom-Json
$inconsistentUeOnlineRequirement = @($inconsistentUeOnlineManifest.requirements | Where-Object { $_.key -eq "unreal.online_validation" } | Select-Object -First 1)
if ($inconsistentUeOnlineRequirement.status -ne "incomplete") {
    throw "Expected inconsistent UE online validation evidence to be incomplete but got '$($inconsistentUeOnlineRequirement.status)'."
}

Write-Host $successMessage -ForegroundColor Green
