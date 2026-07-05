<#
Exercises validate-release-input-template.ps1 against valid and invalid templates.

The test proves suggested command placeholders stay backed by template input rows.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-input-template-validation-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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

$validTemplatePath = Join-Path $testRoot "release-input-template-valid.json"
$invalidTemplatePath = Join-Path $testRoot "release-input-template-invalid.json"
$invalidParameterTemplatePath = Join-Path $testRoot "release-input-template-invalid-parameter.json"
$invalidUsesInputTemplatePath = Join-Path $testRoot "release-input-template-invalid-uses-input.json"
$invalidCommandUsesInputTemplatePath = Join-Path $testRoot "release-input-template-invalid-command-uses-input.json"
$invalidCompatibleInputTemplatePath = Join-Path $testRoot "release-input-template-invalid-compatible-input.json"
$invalidAsymmetricCompatibleInputTemplatePath = Join-Path $testRoot "release-input-template-invalid-asymmetric-compatible-input.json"
$invalidInputCountTemplatePath = Join-Path $testRoot "release-input-template-invalid-input-count.json"
$validOutputPath = Join-Path $testRoot "validation-valid.json"
$invalidOutputPath = Join-Path $testRoot "validation-invalid.json"
$invalidParameterOutputPath = Join-Path $testRoot "validation-invalid-parameter.json"
$invalidUsesInputOutputPath = Join-Path $testRoot "validation-invalid-uses-input.json"
$invalidCommandUsesInputOutputPath = Join-Path $testRoot "validation-invalid-command-uses-input.json"
$invalidCompatibleInputOutputPath = Join-Path $testRoot "validation-invalid-compatible-input.json"
$invalidAsymmetricCompatibleInputOutputPath = Join-Path $testRoot "validation-invalid-asymmetric-compatible-input.json"
$invalidInputCountOutputPath = Join-Path $testRoot "validation-invalid-input-count.json"

$validTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 2
    inputs = @(
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") },
        [ordered]@{ input = "local smoke install root"; placeholder = "<local-smoke-install-root>"; primaryParameter = "-InstallRoot"; parameters = @("-InstallRoot"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "run-launcher-cdn-smoke"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            command = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <local-smoke-install-root>"
            usesInputs = @("real HTTPS CDN manifest URL", "local smoke install root")
        }
    )
}

$invalidTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 1
    inputs = @(
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "run-launcher-cdn-smoke"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            command = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <missing-install-root>"
            usesInputs = @("real HTTPS CDN manifest URL")
        }
    )
}

$invalidParameterTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 1
    inputs = @(
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "run-launcher-cdn-smoke"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            command = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -NoSuchParam value"
            usesInputs = @("real HTTPS CDN manifest URL")
        }
    )
}

$invalidUsesInputTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 1
    inputs = @(
        [ordered]@{ input = "release run id"; placeholder = "<release-run-id>"; primaryParameter = "-RunId"; parameters = @("-RunId"); environmentVariables = @(); blockedBy = @() }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "run-ai-showcase-automation"
            script = "scripts\run-ai-showcase-automation.ps1"
            command = ".\scripts\run-ai-showcase-automation.ps1 -EvidenceDir .\Artifacts\ProductionEvidence -RunId <release-run-id>"
            usesInputs = @("release run id", "missing declared input")
        }
    )
}

$invalidCommandUsesInputTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 2
    inputs = @(
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") },
        [ordered]@{ input = "local smoke install root"; placeholder = "<local-smoke-install-root>"; primaryParameter = "-InstallRoot"; parameters = @("-InstallRoot"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") }
    )
    suggestedCommands = @(
        [ordered]@{
            name = "run-launcher-cdn-smoke"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            command = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <local-smoke-install-root>"
            usesInputs = @("real HTTPS CDN manifest URL")
        }
    )
}

$invalidCompatibleInputTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 1
    inputs = @(
        [ordered]@{
            input = "public Shipping package root"
            placeholder = "<public-shipping-package-root>"
            primaryParameter = "-PackageRoot"
            parameters = @("-PackageRoot")
            environmentVariables = @()
            blockedBy = @("client.package_launcher")
            compatibleInputs = @("missing package root input")
        }
    )
    suggestedCommands = @()
}

$invalidAsymmetricCompatibleInputTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 2
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
            input = "public Shipping package root"
            placeholder = "<public-shipping-package-root>"
            primaryParameter = "-PackageRoot"
            parameters = @("-PackageRoot")
            environmentVariables = @()
            blockedBy = @("client.package_launcher")
            compatibleInputs = @()
        }
    )
    suggestedCommands = @()
}

$invalidInputCountTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    releaseId = "fixture-release"
    inputCount = 99
    inputs = @(
        [ordered]@{ input = "real HTTPS CDN manifest URL"; placeholder = "<real-https-cdn-manifest-url>"; primaryParameter = "-ManifestUrl"; parameters = @("-ManifestUrl"); environmentVariables = @(); blockedBy = @("client.cdn_launcher_smoke") }
    )
    suggestedCommands = @()
}

$validTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $validTemplatePath -Encoding UTF8
$invalidTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidTemplatePath -Encoding UTF8
$invalidParameterTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidParameterTemplatePath -Encoding UTF8
$invalidUsesInputTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidUsesInputTemplatePath -Encoding UTF8
$invalidCommandUsesInputTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidCommandUsesInputTemplatePath -Encoding UTF8
$invalidCompatibleInputTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidCompatibleInputTemplatePath -Encoding UTF8
$invalidAsymmetricCompatibleInputTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidAsymmetricCompatibleInputTemplatePath -Encoding UTF8
$invalidInputCountTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $invalidInputCountTemplatePath -Encoding UTF8

& (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
    -TemplatePath $validTemplatePath `
    -OutputJsonPath $validOutputPath `
    -RequireValid

$failedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidTemplatePath `
        -OutputJsonPath $invalidOutputPath `
        -RequireValid
}
catch {
    $failedAsExpected = $true
}

Assert-True $failedAsExpected "Expected invalid template validation to fail under -RequireValid."

$parameterFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidParameterTemplatePath `
        -OutputJsonPath $invalidParameterOutputPath `
        -RequireValid
}
catch {
    $parameterFailedAsExpected = $true
}

Assert-True $parameterFailedAsExpected "Expected invalid parameter validation to fail under -RequireValid."

$usesInputFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidUsesInputTemplatePath `
        -OutputJsonPath $invalidUsesInputOutputPath `
        -RequireValid
}
catch {
    $usesInputFailedAsExpected = $true
}

Assert-True $usesInputFailedAsExpected "Expected invalid usesInputs validation to fail under -RequireValid."

$commandUsesInputFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidCommandUsesInputTemplatePath `
        -OutputJsonPath $invalidCommandUsesInputOutputPath `
        -RequireValid
}
catch {
    $commandUsesInputFailedAsExpected = $true
}

Assert-True $commandUsesInputFailedAsExpected "Expected command placeholder usesInputs validation to fail under -RequireValid."

$compatibleInputFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidCompatibleInputTemplatePath `
        -OutputJsonPath $invalidCompatibleInputOutputPath `
        -RequireValid
}
catch {
    $compatibleInputFailedAsExpected = $true
}

Assert-True $compatibleInputFailedAsExpected "Expected invalid compatibleInputs validation to fail under -RequireValid."

$asymmetricCompatibleInputFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidAsymmetricCompatibleInputTemplatePath `
        -OutputJsonPath $invalidAsymmetricCompatibleInputOutputPath `
        -RequireValid
}
catch {
    $asymmetricCompatibleInputFailedAsExpected = $true
}

Assert-True $asymmetricCompatibleInputFailedAsExpected "Expected asymmetric compatibleInputs validation to fail under -RequireValid."

$inputCountFailedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
        -TemplatePath $invalidInputCountTemplatePath `
        -OutputJsonPath $invalidInputCountOutputPath `
        -RequireValid
}
catch {
    $inputCountFailedAsExpected = $true
}

Assert-True $inputCountFailedAsExpected "Expected invalid inputCount validation to fail under -RequireValid."

$validResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $validOutputPath | ConvertFrom-Json
$invalidResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidOutputPath | ConvertFrom-Json
$invalidParameterResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidParameterOutputPath | ConvertFrom-Json
$invalidUsesInputResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidUsesInputOutputPath | ConvertFrom-Json
$invalidCommandUsesInputResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidCommandUsesInputOutputPath | ConvertFrom-Json
$invalidCompatibleInputResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidCompatibleInputOutputPath | ConvertFrom-Json
$invalidAsymmetricCompatibleInputResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidAsymmetricCompatibleInputOutputPath | ConvertFrom-Json
$invalidInputCountResult = Get-Content -Raw -Encoding UTF8 -LiteralPath $invalidInputCountOutputPath | ConvertFrom-Json

Assert-True ($validResult.kind -eq "release-input-template-validation") "Expected validation kind."
Assert-True ($validResult.isValid -eq $true) "Expected valid template to pass."
Assert-True ($validResult.commandCount -eq 1) "Expected valid command count."
Assert-True ($validResult.placeholderCount -eq 2) "Expected valid placeholder count."
Assert-True ($invalidResult.isValid -eq $false) "Expected invalid template to be marked invalid."
Assert-True (@($invalidResult.missingPlaceholders | Where-Object { $_.placeholder -eq "<missing-install-root>" -and $_.commandName -eq "run-launcher-cdn-smoke" }).Count -eq 1) "Expected missing placeholder detail."
Assert-True ($invalidParameterResult.isValid -eq $false) "Expected invalid parameter template to be marked invalid."
Assert-True (@($invalidParameterResult.invalidParameters | Where-Object { $_.parameter -eq "-NoSuchParam" -and $_.script -eq "scripts\run-launcher-cdn-smoke.ps1" }).Count -eq 1) "Expected invalid parameter detail."
Assert-True ($invalidUsesInputResult.isValid -eq $false) "Expected invalid usesInputs template to be marked invalid."
Assert-True (@($invalidUsesInputResult.missingInputReferences | Where-Object { $_.input -eq "missing declared input" -and $_.commandName -eq "run-ai-showcase-automation" }).Count -eq 1) "Expected missing usesInputs detail."
Assert-True ($invalidCommandUsesInputResult.isValid -eq $false) "Expected command placeholder usesInputs template to be marked invalid."
Assert-True (@($invalidCommandUsesInputResult.missingCommandInputReferences | Where-Object { $_.placeholder -eq "<local-smoke-install-root>" -and $_.input -eq "local smoke install root" -and $_.commandName -eq "run-launcher-cdn-smoke" }).Count -eq 1) "Expected command placeholder usesInputs detail."
Assert-True ($invalidCompatibleInputResult.isValid -eq $false) "Expected invalid compatibleInputs template to be marked invalid."
Assert-True (@($invalidCompatibleInputResult.missingCompatibleInputReferences | Where-Object { $_.input -eq "public Shipping package root" -and $_.compatibleInput -eq "missing package root input" }).Count -eq 1) "Expected missing compatibleInputs detail."
Assert-True ($invalidAsymmetricCompatibleInputResult.isValid -eq $false) "Expected asymmetric compatibleInputs template to be marked invalid."
Assert-True (@($invalidAsymmetricCompatibleInputResult.asymmetricCompatibleInputReferences | Where-Object { $_.input -eq "release package root" -and $_.compatibleInput -eq "public Shipping package root" }).Count -eq 1) "Expected asymmetric compatibleInputs detail."
Assert-True ($invalidInputCountResult.isValid -eq $false) "Expected invalid inputCount template to be marked invalid."
Assert-True ($invalidInputCountResult.inputCountMatches -eq $false) "Expected validation to report inputCountMatches=false."
Assert-True ($invalidInputCountResult.declaredInputCount -eq 99 -and $invalidInputCountResult.actualInputCount -eq 1) "Expected declared and actual input counts in validation output."
Assert-True ($validResult.invalidParameterCount -eq 0) "Expected valid template to have no invalid parameters."
Assert-True ($validResult.scriptCount -eq 1) "Expected valid template to count checked scripts."

Write-Host "PASS: release input template validation fixtures" -ForegroundColor Green
