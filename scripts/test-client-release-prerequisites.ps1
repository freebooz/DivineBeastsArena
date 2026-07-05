<#
使用临时夹具验证客户端发布前置条件诊断链路。
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$scriptPath = Join-Path $repoRoot "scripts\diagnose-client-release-prerequisites.ps1"
$fixtureRoot = Join-Path $repoRoot (".tmp\client-release-prerequisites-tests-{0}" -f [guid]::NewGuid().ToString("N"))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 23458, 25143, 31471, 21457, 24067, 21069, 32622, 26465, 20214, 35786, 26029, 22865, 32422)
$expectedUniqueFixtureRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 24517, 39035, 20351, 29992, 21807, 19968, 20020, 26102, 30446, 24405, 65292, 36991, 20813, 22797, 29992, 26087, 22841, 20855, 30446, 24405, 12290)
$forbiddenFixedFixtureRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 22797, 29992, 22266, 23450, 23458, 25143, 31471, 21457, 24067, 21069, 32622, 26465, 20214, 22841, 20855, 30446, 24405, 12290)
$expectedChineseSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenOldSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenFixedFixtureRootLiteral = New-TextFromCodePoints @(36, 102, 105, 120, 116, 117, 114, 101, 82, 111, 111, 116, 32, 61, 32, 74, 111, 105, 110, 45, 80, 97, 116, 104, 32, 36, 114, 101, 112, 111, 82, 111, 111, 116, 32, 34, 46, 116, 109, 112, 92, 99, 108, 105, 101, 110, 116, 45, 114, 101, 108, 101, 97, 115, 101, 45, 112, 114, 101, 114, 101, 113, 117, 105, 115, 105, 116, 101, 115, 45, 116, 101, 115, 116, 115, 34)
$forbiddenOldSuccessLiteral = New-TextFromCodePoints @(80, 65, 83, 83, 58, 32, 99, 108, 105, 101, 110, 116, 32, 114, 101, 108, 101, 97, 115, 101, 32, 112, 114, 101, 114, 101, 113, 117, 105, 115, 105, 116, 101, 32, 100, 105, 97, 103, 110, 111, 115, 116, 105, 99, 115)
$expectedExampleCdnFailureMessage = New-TextFromCodePoints @(39044, 26399, 31034, 20363, 32, 67, 68, 78, 32, 22320, 22336, 21069, 32622, 26465, 20214, 26816, 26597, 22833, 36133, 12290)
$expectedExampleReadyFalseMessage = New-TextFromCodePoints @(39044, 26399, 31034, 20363, 22320, 22336, 25253, 21578, 23558, 32, 114, 101, 97, 100, 121, 70, 111, 114, 82, 101, 108, 101, 97, 115, 101, 73, 110, 112, 117, 116, 115, 32, 35774, 20026, 32, 102, 97, 108, 115, 101, 12290)
$expectedExampleDownloadIssueMessage = New-TextFromCodePoints @(39044, 26399, 31034, 20363, 22320, 22336, 25253, 21578, 21253, 21547, 32, 100, 111, 119, 110, 108, 111, 97, 100, 95, 117, 114, 108, 95, 101, 120, 97, 109, 112, 108, 101, 12290)
$expectedExampleManifestIssueMessage = New-TextFromCodePoints @(39044, 26399, 31034, 20363, 22320, 22336, 25253, 21578, 21253, 21547, 32, 109, 97, 110, 105, 102, 101, 115, 116, 95, 117, 114, 108, 95, 101, 120, 97, 109, 112, 108, 101, 12290)
$expectedHostlessFailureMessage = New-TextFromCodePoints @(39044, 26399, 32570, 23569, 20027, 26426, 21517, 30340, 19979, 36733, 22320, 22336, 21069, 32622, 26465, 20214, 26816, 26597, 22833, 36133, 12290)
$expectedHostlessIssueMessage = New-TextFromCodePoints @(39044, 26399, 32570, 23569, 20027, 26426, 21517, 30340, 22320, 22336, 25253, 21578, 21253, 21547, 32, 100, 111, 119, 110, 108, 111, 97, 100, 95, 117, 114, 108, 95, 105, 110, 118, 97, 108, 105, 100, 12290)
$expectedHostlessMessageText = New-TextFromCodePoints @(39044, 26399, 32570, 23569, 20027, 26426, 21517, 30340, 22320, 22336, 25253, 21578, 20351, 29992, 28165, 26224, 30340, 32477, 23545, 22320, 22336, 19982, 20027, 26426, 21517, 26657, 39564, 28040, 24687, 12290)
$expectedValidInputsReadyMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 21457, 24067, 36755, 20837, 36890, 36807, 21069, 32622, 26465, 20214, 26816, 26597, 12290)
$expectedValidInputsNoBlockersMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 21457, 24067, 36755, 20837, 27809, 26377, 38459, 22622, 39033, 12290)
$expectedValidInputsPackageCountMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 21457, 24067, 36755, 20837, 25253, 21578, 21253, 20307, 25991, 20214, 25968, 37327, 12290)
$expectedBundleExampleFailureMessage = New-TextFromCodePoints @(39044, 26399, 23458, 25143, 31471, 21457, 24067, 35777, 25454, 21253, 22312, 31034, 20363, 32, 67, 68, 78, 32, 22320, 22336, 19978, 25552, 21069, 22833, 36133, 12290)
$expectedBundleReadyFalseMessage = New-TextFromCodePoints @(39044, 26399, 35777, 25454, 21253, 21069, 32622, 26465, 20214, 25253, 21578, 23558, 32, 114, 101, 97, 100, 121, 70, 111, 114, 82, 101, 108, 101, 97, 115, 101, 73, 110, 112, 117, 116, 115, 32, 35774, 20026, 32, 102, 97, 108, 115, 101, 12290)
$expectedBundleDownloadIssueMessage = New-TextFromCodePoints @(39044, 26399, 35777, 25454, 21253, 21069, 32622, 26465, 20214, 25253, 21578, 21253, 21547, 32, 100, 111, 119, 110, 108, 111, 97, 100, 95, 117, 114, 108, 95, 101, 120, 97, 109, 112, 108, 101, 12290)
$expectedBundleManifestIssueMessage = New-TextFromCodePoints @(39044, 26399, 35777, 25454, 21253, 21069, 32622, 26465, 20214, 25253, 21578, 21253, 21547, 32, 109, 97, 110, 105, 102, 101, 115, 116, 95, 117, 114, 108, 95, 101, 120, 97, 109, 112, 108, 101, 12290)
$expectedManifestRequirementMessage = New-TextFromCodePoints @(39044, 26399, 29983, 20135, 35777, 25454, 28165, 21333, 21253, 21547, 32, 99, 108, 105, 101, 110, 116, 46, 114, 101, 108, 101, 97, 115, 101, 95, 112, 114, 101, 114, 101, 113, 117, 105, 115, 105, 116, 101, 115, 12290)
$expectedManifestPresentMessage = New-TextFromCodePoints @(39044, 26399, 23601, 32490, 30340, 21069, 32622, 26465, 20214, 35777, 25454, 23558, 32, 99, 108, 105, 101, 110, 116, 46, 114, 101, 108, 101, 97, 115, 101, 95, 112, 114, 101, 114, 101, 113, 117, 105, 115, 105, 116, 101, 115, 32, 26631, 35760, 20026, 32, 112, 114, 101, 115, 101, 110, 116, 12290)
$expectedDiagnosticReportMessageContract = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25253, 21578, 20889, 20837, 28040, 24687, 12290)
$expectedDiagnosticBlockingTitleContract = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 38459, 22622, 39033, 26631, 39064, 12290)
$expectedDiagnosticSuccessContract = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenOldDiagnosticReportMessage = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25253, 21578, 20889, 20837, 28040, 24687, 12290)
$forbiddenOldDiagnosticBlockingTitle = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 38459, 22622, 39033, 26631, 39064, 12290)
$forbiddenOldDiagnosticSuccessMessage = New-TextFromCodePoints @(35786, 26029, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25104, 21151, 28040, 24687, 12290)
$expectedChineseUrlHostMessage = New-TextFromCodePoints @(21253, 21547, 20027, 26426, 21517)

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function New-FixturePackage {
    param([Parameter(Mandatory = $true)][string]$Root)

    if (Test-Path -LiteralPath $Root) {
        Remove-Item -LiteralPath $Root -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $Root | Out-Null
    Set-Content -LiteralPath (Join-Path $Root "DivineBeastsArena.exe") -Value "fixture exe" -Encoding ASCII
}

function Read-Json {
    param([Parameter(Mandatory = $true)][string]$Path)
    Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

$testSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $PSCommandPath
Assert-True ($testSource.Contains("client-release-prerequisites-tests-{0}")) $expectedUniqueFixtureRootMessage
Assert-True (-not $testSource.Contains($forbiddenFixedFixtureRootLiteral)) $forbiddenFixedFixtureRootMessage
Assert-True ($testSource.Contains('$successMessage')) $expectedChineseSuccessMessage
Assert-True (-not $testSource.Contains($forbiddenOldSuccessLiteral)) $forbiddenOldSuccessMessage

$diagnosticSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $scriptPath
Assert-True ($diagnosticSource.Contains('$reportWrittenMessage')) $expectedDiagnosticReportMessageContract
Assert-True ($diagnosticSource.Contains('$blockingIssuesTitleMessage')) $expectedDiagnosticBlockingTitleContract
Assert-True ($diagnosticSource.Contains('$successReadyMessage')) $expectedDiagnosticSuccessContract
Assert-True (-not $diagnosticSource.Contains("Client release prerequisite report written")) $forbiddenOldDiagnosticReportMessage
Assert-True (-not $diagnosticSource.Contains("Blocking client release input issues")) $forbiddenOldDiagnosticBlockingTitle
Assert-True (-not $diagnosticSource.Contains("PASS: client release prerequisite inputs are ready")) $forbiddenOldDiagnosticSuccessMessage

New-Item -ItemType Directory -Force -Path $fixtureRoot | Out-Null
$packageRoot = Join-Path $fixtureRoot "package"
New-FixturePackage -Root $packageRoot

$exampleOutput = Join-Path $fixtureRoot "example-url.json"
$failedAsExpected = $false
try {
    & $scriptPath `
        -PackageRoot $packageRoot `
        -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/" `
        -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" `
        -RequireManifestUrl `
        -OutputJsonPath $exampleOutput `
        -FailOnBlockingIssues | Out-Null
}
catch {
    $failedAsExpected = $true
}

if (-not $failedAsExpected) {
    throw $expectedExampleCdnFailureMessage
}

$exampleReport = Read-Json -Path $exampleOutput
if ($exampleReport.readyForReleaseInputs -ne $false) {
    throw $expectedExampleReadyFalseMessage
}
if (@($exampleReport.blockingIssues | Where-Object { $_.code -eq "download_url_example" }).Count -ne 1) {
    throw $expectedExampleDownloadIssueMessage
}
if (@($exampleReport.blockingIssues | Where-Object { $_.code -eq "manifest_url_example" }).Count -ne 1) {
    throw $expectedExampleManifestIssueMessage
}

$hostlessOutput = Join-Path $fixtureRoot "hostless-url.json"
$hostlessFailedAsExpected = $false
try {
    & $scriptPath `
        -PackageRoot $packageRoot `
        -DownloadUrl "https://" `
        -ManifestUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json" `
        -RequireManifestUrl `
        -OutputJsonPath $hostlessOutput `
        -FailOnBlockingIssues | Out-Null
}
catch {
    $hostlessFailedAsExpected = $true
}

if (-not $hostlessFailedAsExpected) {
    throw $expectedHostlessFailureMessage
}

$hostlessReport = Read-Json -Path $hostlessOutput
$hostlessIssue = @($hostlessReport.blockingIssues | Where-Object { $_.code -eq "download_url_invalid" })
if ($hostlessIssue.Count -ne 1) {
    throw $expectedHostlessIssueMessage
}
if ($hostlessIssue[0].message -notmatch $expectedChineseUrlHostMessage) {
    throw $expectedHostlessMessageText
}

$validOutput = Join-Path $fixtureRoot "valid-inputs.json"
& $scriptPath `
    -PackageRoot $packageRoot `
    -DownloadUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/" `
    -ManifestUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json" `
    -RequireManifestUrl `
    -OutputJsonPath $validOutput `
    -FailOnBlockingIssues | Out-Null

$validReport = Read-Json -Path $validOutput
if ($validReport.readyForReleaseInputs -ne $true) {
    throw $expectedValidInputsReadyMessage
}
if ($validReport.blockingIssueCount -ne 0) {
    throw $expectedValidInputsNoBlockersMessage
}
if ($validReport.package.fileCount -lt 1) {
    throw $expectedValidInputsPackageCountMessage
}

$bundleEvidenceRoot = Join-Path $fixtureRoot "bundle-evidence"
$bundleRunId = "bundle-example-url"
$bundleFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\run-client-release-evidence.ps1") `
        -PackageRoot $packageRoot `
        -EvidenceRoot $bundleEvidenceRoot `
        -RunId $bundleRunId `
        -BuildConfiguration Shipping `
        -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/" `
        -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" `
        -SkipLauncherInstallUpdate `
        -SkipCdnSmoke | Out-Null
}
catch {
    $bundleFailedAsExpected = $true
}

if (-not $bundleFailedAsExpected) {
    throw $expectedBundleExampleFailureMessage
}

$bundlePrerequisiteReportPath = Join-Path $bundleEvidenceRoot "client\client-release-prerequisites-$bundleRunId.json"
$bundlePrerequisiteReport = Read-Json -Path $bundlePrerequisiteReportPath
if ($bundlePrerequisiteReport.readyForReleaseInputs -ne $false) {
    throw $expectedBundleReadyFalseMessage
}
if (@($bundlePrerequisiteReport.blockingIssues | Where-Object { $_.code -eq "download_url_example" }).Count -ne 1) {
    throw $expectedBundleDownloadIssueMessage
}
if (@($bundlePrerequisiteReport.blockingIssues | Where-Object { $_.code -eq "manifest_url_example" }).Count -ne 1) {
    throw $expectedBundleManifestIssueMessage
}

$manifestFixtureRoot = Join-Path $fixtureRoot "manifest-ready-evidence"
$manifestFixtureClientDir = Join-Path $manifestFixtureRoot "client"
New-Item -ItemType Directory -Force -Path $manifestFixtureClientDir | Out-Null
Copy-Item -LiteralPath $validOutput -Destination (Join-Path $manifestFixtureClientDir "client-release-prerequisites-valid-inputs.json") -Force

& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") -EvidenceRoot $manifestFixtureRoot | Out-Null
$manifest = Read-Json -Path (Join-Path $manifestFixtureRoot "production-evidence-manifest.json")
$prerequisiteRequirement = @($manifest.requirements | Where-Object { $_.key -eq "client.release_prerequisites" })
if ($prerequisiteRequirement.Count -ne 1) {
    throw $expectedManifestRequirementMessage
}
if ($prerequisiteRequirement[0].status -ne "present") {
    throw $expectedManifestPresentMessage
}

Write-Host $successMessage -ForegroundColor Green
