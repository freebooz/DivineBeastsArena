<#
Exercises validate-release-input-values.ps1 against blank, example, and complete
release input values fixtures.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-input-values-validation-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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

function Write-ValuesFixture {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][array]$Values
    )

    [ordered]@{
        schemaVersion = "1.0"
        kind = "release-input-values"
        releaseId = "fixture-release"
        isTemplate = $false
        inputCount = $Values.Count
        values = $Values
    } | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $Path -Encoding UTF8
}

$blankValuesPath = Join-Path $testRoot "release-input-values-blank.json"
$exampleValuesPath = Join-Path $testRoot "release-input-values-example.json"
$completeValuesPath = Join-Path $testRoot "release-input-values-complete.json"
$blankOutputPath = Join-Path $testRoot "validation-blank.json"
$exampleOutputPath = Join-Path $testRoot "validation-example.json"
$completeOutputPath = Join-Path $testRoot "validation-complete.json"

$baseRows = @(
    [ordered]@{ input = "release package root"; value = ""; placeholder = "<release-package-root>"; primaryParameter = "-PackageRoot"; parameters = @("-PackageRoot"); environmentVariables = @(); blockedBy = @("client.release_prerequisites"); compatibleInputs = @("public Shipping package root") },
    [ordered]@{ input = "real HTTPS CDN download URL"; value = ""; placeholder = "<real-https-cdn-download-url>"; primaryParameter = "-DownloadUrl"; parameters = @("-DownloadUrl"); environmentVariables = @(); blockedBy = @("client.release_prerequisites", "client.package_launcher"); compatibleInputs = @() },
    [ordered]@{ input = "real HTTPS CDN manifest URL"; value = ""; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.release_prerequisites", "client.cdn_launcher_smoke"); compatibleInputs = @() },
    [ordered]@{ input = "trusted Authenticode signing identity"; value = ""; placeholder = "<trusted-authenticode-signing-identity>"; primaryParameter = "-CertificateThumbprint"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @(); blockedBy = @("client.release_prerequisites", "client.code_signing"); compatibleInputs = @() },
    [ordered]@{ input = "signtool path"; value = ""; placeholder = "<signtool-path>"; primaryParameter = "-SignToolPath"; parameters = @("-SignToolPath"); environmentVariables = @("WindowsSdkDir"); blockedBy = @("client.release_prerequisites"); compatibleInputs = @() },
    [ordered]@{ input = "timestamp URL"; value = ""; placeholder = "<timestamp-url>"; primaryParameter = "-TimestampUrl"; parameters = @("-TimestampUrl"); environmentVariables = @(); blockedBy = @("client.code_signing"); compatibleInputs = @() },
    [ordered]@{ input = "local smoke install root"; value = ""; placeholder = "<local-smoke-install-root>"; primaryParameter = "-InstallRoot"; parameters = @("-InstallRoot"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke"); compatibleInputs = @() }
)

Write-ValuesFixture -Path $blankValuesPath -Values $baseRows

$exampleRows = @($baseRows | ForEach-Object { [ordered]@{
    input = $_.input
    value = $_.value
    placeholder = $_.placeholder
    primaryParameter = $_.primaryParameter
    parameters = $_.parameters
    environmentVariables = $_.environmentVariables
    blockedBy = $_.blockedBy
    compatibleInputs = $_.compatibleInputs
} })
$exampleRows[0].value = "<release-package-root>"
$exampleRows[1].value = "https://cdn.example.com/releases/0.1.0.0/"
$exampleRows[2].value = "http://cdn.example.com/releases/0.1.0.0/launcher-manifest.json"
$exampleRows[3].value = "THUMBPRINT"
$exampleRows[4].value = "signtool.exe"
$exampleRows[5].value = "http://timestamp.example.com"
$exampleRows[6].value = "C:\Temp\DBA Smoke"
Write-ValuesFixture -Path $exampleValuesPath -Values $exampleRows

$completeRows = @($baseRows | ForEach-Object { [ordered]@{
    input = $_.input
    value = $_.value
    placeholder = $_.placeholder
    primaryParameter = $_.primaryParameter
    parameters = $_.parameters
    environmentVariables = $_.environmentVariables
    blockedBy = $_.blockedBy
    compatibleInputs = $_.compatibleInputs
} })
$completeRows[0].value = "E:\Builds\DivineBeastsArena\Shipping"
$completeRows[1].value = "https://download.divinebeastsarena.com/releases/1.0.0/"
$completeRows[2].value = "https://download.divinebeastsarena.com/releases/1.0.0/launcher-manifest.json"
$completeRows[3].value = "00112233445566778899AABBCCDDEEFF00112233"
$completeRows[4].value = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"
$completeRows[5].value = "https://timestamp.sectigo.com"
$completeRows[6].value = "C:\Temp\DBA Smoke"
Write-ValuesFixture -Path $completeValuesPath -Values $completeRows

$blankFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-values.ps1") `
        -ValuesPath $blankValuesPath `
        -OutputJsonPath $blankOutputPath `
        -RequireValid
}
catch {
    $blankFailedAsExpected = $true
}

Assert-True $blankFailedAsExpected "Expected blank release input values to fail under -RequireValid."

$exampleFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-values.ps1") `
        -ValuesPath $exampleValuesPath `
        -OutputJsonPath $exampleOutputPath `
        -RequireValid
}
catch {
    $exampleFailedAsExpected = $true
}

Assert-True $exampleFailedAsExpected "Expected example release input values to fail under -RequireValid."

& (Join-Path $repoRoot "scripts\validate-release-input-values.ps1") `
    -ValuesPath $completeValuesPath `
    -OutputJsonPath $completeOutputPath `
    -RequireValid

$blankResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $blankOutputPath | ConvertFrom-Json
$exampleResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $exampleOutputPath | ConvertFrom-Json
$completeResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $completeOutputPath | ConvertFrom-Json

Assert-True ($blankResult.kind -eq "release-input-values-validation") "Expected validation kind."
Assert-True ($blankResult.isValid -eq $false) "Expected blank values to be invalid."
Assert-True ($blankResult.blankValueCount -eq $baseRows.Count) "Expected every blank row to be counted."
Assert-True ($exampleResult.isValid -eq $false) "Expected example values to be invalid."
Assert-True ($exampleResult.placeholderValueCount -eq 1) "Expected placeholder value to be counted."
Assert-True ($exampleResult.exampleUrlCount -eq 3) "Expected example URLs to be counted."
Assert-True ($exampleResult.insecureUrlCount -eq 2) "Expected insecure URLs to be counted."
Assert-True ($completeResult.isValid -eq $true) "Expected complete values to be valid."
Assert-True ($completeResult.inputCount -eq $baseRows.Count) "Expected complete input count."
Assert-True ($completeResult.blankValueCount -eq 0 -and $completeResult.exampleUrlCount -eq 0 -and $completeResult.insecureUrlCount -eq 0) "Expected complete values to have no blocking issues."

Write-Host "PASS: release input values validation fixtures" -ForegroundColor Green
