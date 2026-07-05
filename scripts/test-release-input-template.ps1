<#
Exercises write-release-input-template.ps1 against release blocker actions.

The test proves automation-blocked release inputs can be converted into a
machine-readable fill-in template for the next real release run.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\release-input-template-tests-{0}" -f [guid]::NewGuid().ToString("N"))
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

$actionsPath = Join-Path $testRoot "release-blocker-actions.json"
$templateJsonPath = Join-Path $testRoot "release-input-template.json"
$templateMarkdownPath = Join-Path $testRoot "release-input-template.md"

$actions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release"
    readyForRelease = $false
    blockerCount = 4
    actions = @(
        [ordered]@{
            key = "unreal.ai_showcase_automation"
            automationBlocked = $false
            blockingExternalInputs = @()
            inputResolutionHints = @(
                [ordered]@{ input = "release run id"; parameters = @("-RunId"); environmentVariables = @("GITHUB_RUN_ID") }
            )
        },
        [ordered]@{
            key = "client.package_launcher"
            automationBlocked = $true
            blockingExternalInputs = @("public Shipping package root", "real HTTPS CDN download URL")
            inputResolutionHints = @(
                [ordered]@{ input = "public Shipping package root"; parameters = @("-PackageRoot", "-StagedPackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "real HTTPS CDN download URL"; parameters = @("-DownloadUrl"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.cdn_launcher_smoke"
            automationBlocked = $true
            blockingExternalInputs = @("real HTTPS CDN manifest URL", "local smoke install root")
            inputResolutionHints = @(
                [ordered]@{ input = "real HTTPS CDN manifest URL"; parameters = @("-ManifestUrl"); environmentVariables = @() },
                [ordered]@{ input = "local smoke install root"; parameters = @("-InstallRoot"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            blockingExternalInputs = @("trusted Authenticode signing identity", "timestamp URL")
            inputResolutionHints = @(
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "timestamp URL"; parameters = @("-TimestampUrl"); environmentVariables = @() }
            )
        }
    )
}

$actions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $actionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $actionsPath `
    -OutputJsonPath $templateJsonPath `
    -OutputMarkdownPath $templateMarkdownPath

$template = Get-Content -Raw -Encoding UTF8 -LiteralPath $templateJsonPath | ConvertFrom-Json
$markdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $templateMarkdownPath

Assert-True ($template.kind -eq "release-input-template") "Expected release-input-template kind."
Assert-True ($template.releaseId -eq "fixture-release") "Expected release id to be copied from action report."
Assert-True ($template.inputCount -eq 7) "Expected seven unique release input rows."
Assert-True (@($template.suggestedCommands).Count -ge 4) "Expected suggested release commands."
Assert-True (@($template.inputs | Where-Object { $_.input -eq "release run id" -and $_.primaryParameter -eq "-RunId" -and $_.placeholder -eq "<release-run-id>" -and $_.environmentVariables -contains "GITHUB_RUN_ID" }).Count -eq 1) "Expected release run id template row."
Assert-True (@($template.inputs | Where-Object { $_.input -eq "public Shipping package root" -and $_.primaryParameter -eq "-PackageRoot" -and $_.placeholder -eq "<public-shipping-package-root>" }).Count -eq 1) "Expected package root template row."
Assert-True (@($template.inputs | Where-Object { $_.input -eq "real HTTPS CDN manifest URL" -and $_.primaryParameter -eq "-ManifestUrl" -and $_.placeholder -eq "<real-https-cdn-manifest-url>" }).Count -eq 1) "Expected manifest URL template row."
Assert-True (@($template.inputs | Where-Object { $_.input -eq "trusted Authenticode signing identity" -and ($_.parameters -join "`n") -match "-CertificateSubject" }).Count -eq 1) "Expected signing identity template row."
Assert-True (($template.inputs | Where-Object { $_.input -eq "real HTTPS CDN download URL" }).blockedBy -contains "client.package_launcher") "Expected input row to record blocking action."
Assert-True (@($template.inputs | Where-Object { $_.input -eq "public Shipping package root" -and @($_.compatibleInputs).Count -eq 0 }).Count -eq 1) "Expected fixture package root row to have no compatible inputs when no sibling package-root inputs exist."
Assert-True (@($template.suggestedCommands | Where-Object { $_.name -eq "run-client-release-evidence" -and $_.script -eq "scripts\run-client-release-evidence.ps1" -and $_.command -match "-PackageRoot <public-shipping-package-root>" -and $_.command -match "-DownloadUrl <real-https-cdn-download-url>" -and $_.command -match "-ManifestUrl <real-https-cdn-manifest-url>" }).Count -eq 1) "Expected release evidence suggested command with placeholders."
Assert-True (@($template.suggestedCommands | Where-Object { $_.name -eq "run-launcher-cdn-smoke" -and $_.command -match "-ManifestUrl <real-https-cdn-manifest-url>" }).Count -eq 1) "Expected CDN smoke suggested command."
Assert-True (@($template.suggestedCommands | Where-Object { $_.name -eq "sign-client-release-package" -and $_.command -match "-CertificateThumbprint <trusted-authenticode-signing-identity>" -and $_.command -match "-TimestampUrl <timestamp-url>" }).Count -eq 1) "Expected signing suggested command."
Assert-True (@($template.suggestedCommands | Where-Object { $_.name -eq "run-ai-showcase-automation" -and $_.script -eq "scripts\run-ai-showcase-automation.ps1" -and $_.command -match "-EvidenceDir .\\Artifacts\\ProductionEvidence" -and $_.command -match "-RunId <release-run-id>" }).Count -eq 1) "Expected AI_Showcase suggested command."
Assert-True ($markdown -match "Release input template") "Expected markdown title."
Assert-True ($markdown -match "Suggested commands") "Expected markdown suggested commands section."
Assert-True ($markdown -match "Compatible inputs") "Expected markdown compatible inputs column."
Assert-True ($markdown -match "-PackageRoot") "Expected markdown parameter hint."
Assert-True ($markdown -match "run-ai-showcase-automation") "Expected markdown AI_Showcase suggested command."
Assert-True ($markdown -match "<trusted-authenticode-signing-identity>") "Expected markdown placeholder."

Write-Host "PASS: release input template fixtures" -ForegroundColor Green

$compatibleActionsPath = Join-Path $testRoot "release-blocker-actions-compatible.json"
$compatibleTemplateJsonPath = Join-Path $testRoot "release-input-template-compatible.json"
$compatibleTemplateMarkdownPath = Join-Path $testRoot "release-input-template-compatible.md"

$compatibleActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-compatible-inputs"
    readyForRelease = $false
    blockerCount = 3
    actions = @(
        [ordered]@{
            key = "client.release_prerequisites"
            automationBlocked = $true
            inputResolutionHints = @(
                [ordered]@{ input = "release package root"; parameters = @("-PackageRoot"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.package_launcher"
            automationBlocked = $true
            inputResolutionHints = @(
                [ordered]@{ input = "public Shipping package root"; parameters = @("-PackageRoot", "-StagedPackageRoot"); environmentVariables = @() }
            )
        },
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            inputResolutionHints = @(
                [ordered]@{ input = "public package root containing signable binaries"; parameters = @("-PackageRoot"); environmentVariables = @() }
            )
        }
    )
}

$compatibleActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $compatibleActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $compatibleActionsPath `
    -OutputJsonPath $compatibleTemplateJsonPath `
    -OutputMarkdownPath $compatibleTemplateMarkdownPath

$compatibleTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $compatibleTemplateJsonPath | ConvertFrom-Json
$compatibleMarkdown = Get-Content -Raw -Encoding UTF8 -LiteralPath $compatibleTemplateMarkdownPath

Assert-True (($compatibleTemplate.inputs | Where-Object { $_.input -eq "release package root" }).compatibleInputs -contains "public Shipping package root") "Expected release package root to be compatible with public Shipping package root."
Assert-True (($compatibleTemplate.inputs | Where-Object { $_.input -eq "public Shipping package root" }).compatibleInputs -contains "public package root containing signable binaries") "Expected public Shipping package root to be compatible with signable package root."
Assert-True ($compatibleMarkdown -match "public package root containing signable binaries") "Expected compatible input to be rendered in markdown."

Write-Host "PASS: release input template compatible-input fixtures" -ForegroundColor Green

$signingOnlyActionsPath = Join-Path $testRoot "release-blocker-actions-signing-only.json"
$signingOnlyTemplateJsonPath = Join-Path $testRoot "release-input-template-signing-only.json"
$signingOnlyTemplateMarkdownPath = Join-Path $testRoot "release-input-template-signing-only.md"

$signingOnlyActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-signing-only"
    readyForRelease = $false
    blockerCount = 1
    actions = @(
        [ordered]@{
            key = "client.code_signing"
            automationBlocked = $true
            inputResolutionHints = @(
                [ordered]@{ input = "public package root containing signable binaries"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "timestamp URL"; parameters = @("-TimestampUrl"); environmentVariables = @() }
            )
        }
    )
}

$signingOnlyActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $signingOnlyActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $signingOnlyActionsPath `
    -OutputJsonPath $signingOnlyTemplateJsonPath `
    -OutputMarkdownPath $signingOnlyTemplateMarkdownPath

$signingOnlyTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $signingOnlyTemplateJsonPath | ConvertFrom-Json

Assert-True (@($signingOnlyTemplate.suggestedCommands | Where-Object {
    $_.name -eq "sign-client-release-package" -and
    $_.command -match "-PackageRoot <public-package-root-containing-signable-binaries>" -and
    $_.usesInputs -contains "public package root containing signable binaries"
}).Count -eq 1) "Expected signing-only template to suggest signing command using the signable package root input."

Write-Host "PASS: release input template signing-only fixtures" -ForegroundColor Green

$prerequisiteActionsPath = Join-Path $testRoot "release-blocker-actions-prerequisites.json"
$prerequisiteTemplateJsonPath = Join-Path $testRoot "release-input-template-prerequisites.json"
$prerequisiteTemplateMarkdownPath = Join-Path $testRoot "release-input-template-prerequisites.md"

$prerequisiteActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-prerequisites"
    readyForRelease = $false
    blockerCount = 1
    actions = @(
        [ordered]@{
            key = "client.release_prerequisites"
            automationBlocked = $true
            inputResolutionHints = @(
                [ordered]@{ input = "release package root"; parameters = @("-PackageRoot"); environmentVariables = @() },
                [ordered]@{ input = "real HTTPS CDN download URL"; parameters = @("-DownloadUrl"); environmentVariables = @() },
                [ordered]@{ input = "real HTTPS CDN manifest URL"; parameters = @("-ManifestUrl"); environmentVariables = @() },
                [ordered]@{ input = "trusted Authenticode signing identity"; parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath"); environmentVariables = @() },
                [ordered]@{ input = "signtool path"; parameters = @("-SignToolPath"); environmentVariables = @("WindowsSdkDir") }
            )
        }
    )
}

$prerequisiteActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $prerequisiteActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $prerequisiteActionsPath `
    -OutputJsonPath $prerequisiteTemplateJsonPath `
    -OutputMarkdownPath $prerequisiteTemplateMarkdownPath

$prerequisiteTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $prerequisiteTemplateJsonPath | ConvertFrom-Json

Assert-True (@($prerequisiteTemplate.suggestedCommands | Where-Object {
    $_.name -eq "diagnose-client-release-prerequisites" -and
    $_.command -match "-PackageRoot <release-package-root>" -and
    $_.command -match "-DownloadUrl <real-https-cdn-download-url>" -and
    $_.command -match "-ManifestUrl <real-https-cdn-manifest-url>" -and
    $_.command -match "-RequireManifestUrl" -and
    $_.command -match "-RequireSigningIdentity" -and
    $_.command -match "-CertificateThumbprint <trusted-authenticode-signing-identity>" -and
    $_.command -match "-SignToolPath <signtool-path>" -and
    $_.command -match "-RequireSignTool" -and
    $_.usesInputs -contains "release package root" -and
    $_.usesInputs -contains "trusted Authenticode signing identity" -and
    $_.usesInputs -contains "signtool path"
}).Count -eq 1) "Expected prerequisite template to suggest a complete release prerequisite diagnostic command."

Write-Host "PASS: release input template prerequisite fixture" -ForegroundColor Green

$noBlockerActionsPath = Join-Path $testRoot "release-blocker-actions-none.json"
$noBlockerTemplateJsonPath = Join-Path $testRoot "release-input-template-none.json"
$noBlockerTemplateMarkdownPath = Join-Path $testRoot "release-input-template-none.md"
$noBlockerValidationPath = Join-Path $testRoot "release-input-template-none-validation.json"

$noBlockerActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release-ready"
    readyForRelease = $true
    blockerCount = 0
    actions = @()
}

$noBlockerActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $noBlockerActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $noBlockerActionsPath `
    -OutputJsonPath $noBlockerTemplateJsonPath `
    -OutputMarkdownPath $noBlockerTemplateMarkdownPath

$noBlockerTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $noBlockerTemplateJsonPath | ConvertFrom-Json

Assert-True ($noBlockerTemplate.inputCount -eq 0) "Expected no input rows when there are no blockers."
Assert-True (@($noBlockerTemplate.suggestedCommands).Count -eq 0) "Expected no suggested commands when there are no blockers."

& (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
    -TemplatePath $noBlockerTemplateJsonPath `
    -OutputJsonPath $noBlockerValidationPath `
    -RequireValid

Write-Host "PASS: release input template no-blocker fixture" -ForegroundColor Green

$partialActionsPath = Join-Path $testRoot "release-blocker-actions-partial.json"
$partialTemplateJsonPath = Join-Path $testRoot "release-input-template-partial.json"
$partialTemplateMarkdownPath = Join-Path $testRoot "release-input-template-partial.md"
$partialValidationPath = Join-Path $testRoot "release-input-template-partial-validation.json"

$partialActions = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    releaseId = "fixture-release-partial"
    readyForRelease = $false
    blockerCount = 1
    actions = @(
        [ordered]@{
            key = "client.cdn_launcher_smoke"
            automationBlocked = $true
            blockingExternalInputs = @("real HTTPS CDN manifest URL")
            inputResolutionHints = @(
                [ordered]@{ input = "real HTTPS CDN manifest URL"; parameters = @("-ManifestUrl"); environmentVariables = @() }
            )
        }
    )
}

$partialActions | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $partialActionsPath -Encoding UTF8

& (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
    -ActionReportPath $partialActionsPath `
    -OutputJsonPath $partialTemplateJsonPath `
    -OutputMarkdownPath $partialTemplateMarkdownPath

$partialTemplate = Get-Content -Raw -Encoding UTF8 -LiteralPath $partialTemplateJsonPath | ConvertFrom-Json

Assert-True ($partialTemplate.inputCount -eq 1) "Expected partial template to contain only the manifest URL input."
Assert-True (@($partialTemplate.suggestedCommands | Where-Object { $_.name -eq "run-client-release-evidence" }).Count -eq 0) "Expected partial template not to suggest client release evidence without package and download inputs."
Assert-True (@($partialTemplate.suggestedCommands | Where-Object { $_.name -eq "run-launcher-cdn-smoke" }).Count -eq 0) "Expected partial template not to suggest CDN smoke without an install root input."

& (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
    -TemplatePath $partialTemplateJsonPath `
    -OutputJsonPath $partialValidationPath `
    -RequireValid

Write-Host "PASS: release input template partial-input fixture" -ForegroundColor Green
