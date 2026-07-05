<#
Exercises write-release-input-values-template.ps1 against a small release input
template fixture.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-input-values-template-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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

$templatePath = Join-Path $testRoot "release-input-template.json"
$valuesTemplatePath = Join-Path $testRoot "release-input-values.template.json"
$valuesTemplateMarkdownPath = Join-Path $testRoot "release-input-values.template.md"
$commandPlanPath = Join-Path $testRoot "release-command-plan.json"
$commandPlanMarkdownPath = Join-Path $testRoot "release-command-plan.md"

$template = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 3
    inputs = @(
        [ordered]@{
            input = "release package root"
            placeholder = "<release-package-root>"
            primaryParameter = "-PackageRoot"
            parameters = @("-PackageRoot")
            environmentVariables = @()
            blockedBy = @("client.release_prerequisites")
            compatibleInputs = @("public Shipping package root")
        },
        [ordered]@{
            input = "real HTTPS CDN manifest URL"
            placeholder = "<real-https-cdn-manifest-url>"
            primaryParameter = "-ManifestUrl"
            parameters = @("-ManifestUrl")
            environmentVariables = @()
            blockedBy = @("client.release_prerequisites", "client.cdn_launcher_smoke")
            compatibleInputs = @()
        },
        [ordered]@{
            input = "trusted Authenticode signing identity"
            placeholder = "<trusted-authenticode-signing-identity>"
            primaryParameter = "-CertificateThumbprint"
            parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath")
            environmentVariables = @()
            blockedBy = @("client.code_signing")
            compatibleInputs = @()
        }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "diagnose-client-release-prerequisites"
            script = "scripts\diagnose-client-release-prerequisites.ps1"
            command = ".\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot <release-package-root> -ManifestUrl <real-https-cdn-manifest-url> -CertificateThumbprint <trusted-authenticode-signing-identity>"
            usesInputs = @("release package root", "real HTTPS CDN manifest URL", "trusted Authenticode signing identity")
        }
    )
}

$template | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $templatePath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-values-template.ps1") `
    -TemplatePath $templatePath `
    -OutputJsonPath $valuesTemplatePath `
    -OutputMarkdownPath $valuesTemplateMarkdownPath

$valuesTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $valuesTemplatePath | ConvertFrom-Json
$markdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $valuesTemplateMarkdownPath

Assert-True ($valuesTemplate.kind -eq "release-input-values") "Expected release-input-values kind."
Assert-True ($valuesTemplate.releaseId -eq "fixture-release") "Expected release id to match the input template."
Assert-True ($valuesTemplate.isTemplate -eq $true) "Expected values file to be marked as a template."
Assert-True ($valuesTemplate.inputCount -eq 3) "Expected input count."
Assert-True (@($valuesTemplate.values | Where-Object { $_.input -eq "release package root" -and $_.value -eq "" -and $_.placeholder -eq "<release-package-root>" -and $_.primaryParameter -eq "-PackageRoot" }).Count -eq 1) "Expected release package root value row."
Assert-True (@($valuesTemplate.values | Where-Object { $_.input -eq "trusted Authenticode signing identity" -and ($_.parameters -join "`n") -match "-PfxPath" }).Count -eq 1) "Expected signing identity parameter hints."
Assert-True ($markdown -match "release-input-values") "Expected markdown title."
Assert-True ($markdown -match "<trusted-authenticode-signing-identity>") "Expected markdown placeholder."
Assert-True ($markdown -match "client.code_signing") "Expected markdown blocker hint."

$failedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\resolve-release-input-template.ps1") `
        -TemplatePath $templatePath `
        -ValuesPath $valuesTemplatePath `
        -OutputJsonPath $commandPlanPath `
        -OutputMarkdownPath $commandPlanMarkdownPath `
        -RequireComplete
}
catch {
    $failedAsExpected = $true
}

Assert-True $failedAsExpected "Expected blank values template to fail command plan completion."

$commandPlan = Get-Content -Raw -Encoding UTF8 -LiteralPath $commandPlanPath | ConvertFrom-Json
Assert-True ($commandPlan.isComplete -eq $false) "Expected blank values template to produce an incomplete command plan."
Assert-True ($commandPlan.missingInputCount -eq 3) "Expected all fixture inputs to be missing."

Write-Host "PASS: release input values template fixtures" -ForegroundColor Green
