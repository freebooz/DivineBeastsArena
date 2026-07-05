<#
Exercises resolve-release-input-template.ps1 against complete and incomplete
release input value fixtures.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\resolve-release-input-template-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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
$valuesPath = Join-Path $testRoot "release-input-values.json"
$incompleteValuesPath = Join-Path $testRoot "release-input-values-incomplete.json"
$invalidValuesPath = Join-Path $testRoot "release-input-values-invalid.json"
$outputJsonPath = Join-Path $testRoot "release-command-plan.json"
$outputMarkdownPath = Join-Path $testRoot "release-command-plan.md"
$incompleteOutputJsonPath = Join-Path $testRoot "release-command-plan-incomplete.json"
$incompleteOutputMarkdownPath = Join-Path $testRoot "release-command-plan-incomplete.md"
$invalidOutputJsonPath = Join-Path $testRoot "release-command-plan-invalid.json"
$invalidOutputMarkdownPath = Join-Path $testRoot "release-command-plan-invalid.md"

$template = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 5
    inputs = @(
        [ordered]@{ input = "release package root"; placeholder = "<release-package-root>" },
        [ordered]@{ input = "real HTTPS CDN download URL"; placeholder = "<real-https-cdn-download-url>" },
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>" },
        [ordered]@{ input = "trusted Authenticode signing identity"; placeholder = "<trusted-authenticode-signing-identity>" },
        [ordered]@{ input = "signtool path"; placeholder = "<signtool-path>" }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "diagnose-client-release-prerequisites"
            script = "scripts\diagnose-client-release-prerequisites.ps1"
            command = ".\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot <release-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint <trusted-authenticode-signing-identity> -SignToolPath <signtool-path> -RequireSignTool -FailOnBlockingIssues"
            usesInputs = @("release package root", "real HTTPS CDN download URL", "real HTTPS CDN manifest URL", "trusted Authenticode signing identity", "signtool path")
        },
        [ordered]@{
            name = "run-launcher-cdn-smoke"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            command = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot C:\DBA\InstallSmoke"
            usesInputs = @("real HTTPS CDN manifest URL")
        }
    )
}

$values = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-values"
    releaseId = "fixture-release"
    values = @(
        [ordered]@{ input = "release package root"; value = "C:\DBA\Release" },
        [ordered]@{ input = "real HTTPS CDN download URL"; value = "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/" },
        [ordered]@{ input = "real HTTPS CDN manifest URL"; value = "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json" },
        [ordered]@{ input = "trusted Authenticode signing identity"; value = "ABCDEF1234567890" },
        [ordered]@{ input = "signtool path"; value = "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" }
    )
}

$incompleteValues = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-values"
    releaseId = "fixture-release"
    values = @(
        [ordered]@{ input = "release package root"; value = "C:\DBA\Release" },
        [ordered]@{ input = "real HTTPS CDN download URL"; value = "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/" },
        [ordered]@{ input = "trusted Authenticode signing identity"; value = "ABCDEF1234567890" },
        [ordered]@{ input = "signtool path"; value = "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" }
    )
}

$invalidValues = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-values"
    releaseId = "fixture-release"
    values = @(
        [ordered]@{ input = "release package root"; value = "C:\DBA\Release" },
        [ordered]@{ input = "real HTTPS CDN download URL"; value = "https://cdn.example.com/releases/0.1.0.0/" },
        [ordered]@{ input = "real HTTPS CDN manifest URL"; value = "http://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" },
        [ordered]@{ input = "trusted Authenticode signing identity"; value = "<trusted-authenticode-signing-identity>" },
        [ordered]@{ input = "signtool path"; value = "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" }
    )
}

$template | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $templatePath -Encoding UTF8
$values | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $valuesPath -Encoding UTF8
$incompleteValues | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $incompleteValuesPath -Encoding UTF8
$invalidValues | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidValuesPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\resolve-release-input-template.ps1") `
    -TemplatePath $templatePath `
    -ValuesPath $valuesPath `
    -OutputJsonPath $outputJsonPath `
    -OutputMarkdownPath $outputMarkdownPath `
    -RequireComplete

$plan = Get-Content -Raw -Encoding UTF8 -LiteralPath $outputJsonPath | ConvertFrom-Json
$markdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $outputMarkdownPath

Assert-True ($plan.kind -eq "release-command-plan") "Expected release-command-plan kind."
Assert-True ($plan.isComplete -eq $true) "Expected complete values to produce a complete command plan."
Assert-True ($plan.commandCount -eq 2) "Expected command count."
Assert-True ($plan.unresolvedPlaceholderCount -eq 0) "Expected no unresolved placeholders."
Assert-True (@($plan.commands | Where-Object { $_.name -eq "diagnose-client-release-prerequisites" -and $_.command -match "C:\\DBA\\Release" -and $_.command -match "ABCDEF1234567890" }).Count -eq 1) "Expected prerequisite command to be resolved."
Assert-True ($markdown -match "release-command-plan") "Expected markdown title."
Assert-True ($markdown -match "diagnose-client-release-prerequisites") "Expected markdown command name."
Assert-True ($markdown -notmatch "<real-https-cdn-manifest-url>") "Expected markdown to contain no unresolved manifest placeholder."

$failedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\resolve-release-input-template.ps1") `
        -TemplatePath $templatePath `
        -ValuesPath $incompleteValuesPath `
        -OutputJsonPath $incompleteOutputJsonPath `
        -OutputMarkdownPath $incompleteOutputMarkdownPath `
        -RequireComplete
}
catch {
    $failedAsExpected = $true
}

Assert-True $failedAsExpected "Expected incomplete values to fail under -RequireComplete."

$incompletePlan = Get-Content -Raw -Encoding UTF8 -LiteralPath $incompleteOutputJsonPath | ConvertFrom-Json
Assert-True ($incompletePlan.isComplete -eq $false) "Expected incomplete values to produce an incomplete command plan."
Assert-True (@($incompletePlan.missingInputs | Where-Object { $_.input -eq "real HTTPS CDN manifest URL" -and $_.placeholder -eq "<real-https-cdn-manifest-url>" }).Count -eq 1) "Expected missing manifest input detail."
Assert-True ($incompletePlan.unresolvedPlaceholderCount -ge 1) "Expected unresolved placeholder count."

$invalidFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\resolve-release-input-template.ps1") `
        -TemplatePath $templatePath `
        -ValuesPath $invalidValuesPath `
        -OutputJsonPath $invalidOutputJsonPath `
        -OutputMarkdownPath $invalidOutputMarkdownPath `
        -RequireComplete
}
catch {
    $invalidFailedAsExpected = $true
}

Assert-True $invalidFailedAsExpected "Expected invalid values to fail command plan completion under -RequireComplete."

$invalidPlan = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidOutputJsonPath | ConvertFrom-Json
Assert-True ($invalidPlan.isComplete -eq $false) "Expected invalid values to produce an incomplete command plan."
Assert-True ($invalidPlan.valuesValidation.isValid -eq $false) "Expected command plan to include invalid values validation."
Assert-True ($invalidPlan.valuesValidation.exampleUrlCount -eq 2) "Expected command plan to report example URL values."
Assert-True ($invalidPlan.valuesValidation.insecureUrlCount -eq 1) "Expected command plan to report insecure URL values."
Assert-True ($invalidPlan.valuesValidation.placeholderValueCount -eq 1) "Expected command plan to report placeholder values."

Write-Host "PASS: resolve release input template fixtures" -ForegroundColor Green
