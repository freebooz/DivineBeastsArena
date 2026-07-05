<#
Writes a fill-in release input template from release-blocker-actions.json.

Examples:
  .\scripts\write-release-input-template.ps1
  .\scripts\write-release-input-template.ps1 -ActionReportPath .\Artifacts\ProductionEvidence\release-blocker-actions.json
#>

[CmdletBinding()]
param(
    [string]$ActionReportPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-blocker-actions.json"),
    [string]$OutputJsonPath = "",
    [string]$OutputMarkdownPath = ""
)

$ErrorActionPreference = "Stop"

function Resolve-DefaultPath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$ChildPath
    )

    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath((Join-Path $BasePath $ChildPath))
}

function Escape-MarkdownCell {
    param([string]$Value)

    if ($null -eq $Value) {
        return ""
    }

    return ($Value -replace "\|", "\|") -replace "`r?`n", " "
}

function Get-Placeholder {
    param([string]$InputName)

    $slug = ([string]$InputName).ToLowerInvariant() -replace "[^a-z0-9]+", "-"
    $slug = $slug.Trim("-")
    if ([string]::IsNullOrWhiteSpace($slug)) {
        return "<release-input>"
    }

    return "<$slug>"
}

function Add-UniqueString {
    param(
        [System.Collections.Generic.List[string]]$Values,
        [string]$Value
    )

    if (-not [string]::IsNullOrWhiteSpace($Value) -and -not $Values.Contains($Value)) {
        $Values.Add($Value)
    }
}

function Find-InputPlaceholder {
    param(
        [object[]]$Inputs,
        [string]$InputName,
        [string]$Fallback
    )

    $match = @($Inputs | Where-Object { $_.input -eq $InputName } | Select-Object -First 1)
    if ($match.Count -gt 0 -and -not [string]::IsNullOrWhiteSpace([string]$match[0].placeholder)) {
        return [string]$match[0].placeholder
    }

    return $Fallback
}

function New-SuggestedCommand {
    param(
        [string]$Name,
        [string]$Script,
        [string]$Command,
        [string[]]$UsesInputs
    )

    return [ordered]@{
        name = $Name
        script = $Script
        command = $Command
        usesInputs = @($UsesInputs)
    }
}

function Get-CompatibleInputs {
    param(
        [string]$InputName,
        [string[]]$AvailableInputNames
    )

    $packageRootInputs = @(
        "release package root",
        "public Shipping package root",
        "public package root containing signable binaries"
    )

    if ($packageRootInputs -contains $InputName) {
        return @($packageRootInputs | Where-Object { $_ -ne $InputName -and $AvailableInputNames -contains $_ })
    }

    return @()
}

function Get-SuggestedCommands {
    param([object[]]$Inputs)

    $inputNames = @($Inputs | ForEach-Object { [string]$_.input })
    function Test-HasInputs {
        param([string[]]$RequiredInputs)

        foreach ($requiredInput in $RequiredInputs) {
            if ($inputNames -notcontains $requiredInput) {
                return $false
            }
        }

        return $true
    }

    $packageRoot = Find-InputPlaceholder -Inputs $Inputs -InputName "public Shipping package root" -Fallback "<public-shipping-package-root>"
    $signablePackageRoot = Find-InputPlaceholder -Inputs $Inputs -InputName "public package root containing signable binaries" -Fallback $packageRoot
    $downloadUrl = Find-InputPlaceholder -Inputs $Inputs -InputName "real HTTPS CDN download URL" -Fallback "<real-https-cdn-download-url>"
    $manifestUrl = Find-InputPlaceholder -Inputs $Inputs -InputName "real HTTPS CDN manifest URL" -Fallback "<real-https-cdn-manifest-url>"
    $installRoot = Find-InputPlaceholder -Inputs $Inputs -InputName "local smoke install root" -Fallback "<local-smoke-install-root>"
    $certificateIdentity = Find-InputPlaceholder -Inputs $Inputs -InputName "trusted Authenticode signing identity" -Fallback "<trusted-authenticode-signing-identity>"
    $signToolPath = Find-InputPlaceholder -Inputs $Inputs -InputName "signtool path" -Fallback "<signtool-path>"
    $timestampUrl = Find-InputPlaceholder -Inputs $Inputs -InputName "timestamp URL" -Fallback "<timestamp-url>"
    $releaseRunId = Find-InputPlaceholder -Inputs $Inputs -InputName "release run id" -Fallback "<release-run-id>"

    $commands = New-Object System.Collections.Generic.List[object]
    if (Test-HasInputs @("release package root", "real HTTPS CDN download URL", "real HTTPS CDN manifest URL", "trusted Authenticode signing identity", "signtool path")) {
        $releasePackageRoot = Find-InputPlaceholder -Inputs $Inputs -InputName "release package root" -Fallback "<release-package-root>"
        $commands.Add((New-SuggestedCommand `
            -Name "diagnose-client-release-prerequisites" `
            -Script "scripts\diagnose-client-release-prerequisites.ps1" `
            -Command ".\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot $releasePackageRoot -DownloadUrl $downloadUrl -ManifestUrl $manifestUrl -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint $certificateIdentity -SignToolPath $signToolPath -RequireSignTool -FailOnBlockingIssues" `
            -UsesInputs @("release package root", "real HTTPS CDN download URL", "real HTTPS CDN manifest URL", "trusted Authenticode signing identity", "signtool path")))
    }
    if (Test-HasInputs @("release run id")) {
        $commands.Add((New-SuggestedCommand `
            -Name "run-ai-showcase-automation" `
            -Script "scripts\run-ai-showcase-automation.ps1" `
            -Command ".\scripts\run-ai-showcase-automation.ps1 -EvidenceDir .\Artifacts\ProductionEvidence -RunId $releaseRunId" `
            -UsesInputs @("release run id")))
    }
    if (Test-HasInputs @("public Shipping package root", "real HTTPS CDN download URL", "real HTTPS CDN manifest URL")) {
        $commands.Add((New-SuggestedCommand `
            -Name "run-client-release-evidence" `
            -Script "scripts\run-client-release-evidence.ps1" `
            -Command ".\scripts\run-client-release-evidence.ps1 -PackageRoot $packageRoot -DownloadUrl $downloadUrl -ManifestUrl $manifestUrl -BuildConfiguration Shipping" `
            -UsesInputs @("public Shipping package root", "real HTTPS CDN download URL", "real HTTPS CDN manifest URL")))
    }
    if (Test-HasInputs @("real HTTPS CDN manifest URL", "local smoke install root")) {
        $commands.Add((New-SuggestedCommand `
            -Name "run-launcher-cdn-smoke" `
            -Script "scripts\run-launcher-cdn-smoke.ps1" `
            -Command ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl $manifestUrl -InstallRoot $installRoot" `
            -UsesInputs @("real HTTPS CDN manifest URL", "local smoke install root")))
    }
    $signingPackageRootInput = if ($inputNames -contains "public package root containing signable binaries") {
        "public package root containing signable binaries"
    }
    elseif ($inputNames -contains "public Shipping package root") {
        "public Shipping package root"
    }
    else {
        ""
    }

    if (-not [string]::IsNullOrWhiteSpace($signingPackageRootInput) -and (Test-HasInputs @("trusted Authenticode signing identity", "timestamp URL"))) {
        $commands.Add((New-SuggestedCommand `
            -Name "sign-client-release-package" `
            -Script "scripts\sign-client-release-package.ps1" `
            -Command ".\scripts\sign-client-release-package.ps1 -PackageRoot $signablePackageRoot -CertificateThumbprint $certificateIdentity -TimestampUrl $timestampUrl -RequireSigned" `
            -UsesInputs @($signingPackageRootInput, "trusted Authenticode signing identity", "timestamp URL")))
    }

    return @($commands.ToArray())
}

$resolvedActionReportPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ActionReportPath)
if (-not (Test-Path -LiteralPath $resolvedActionReportPath)) {
    throw "release-blocker-actions.json was not found: $resolvedActionReportPath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedActionReportPath) -ChildPath "release-input-template.json"
}
if ([string]::IsNullOrWhiteSpace($OutputMarkdownPath)) {
    $OutputMarkdownPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedActionReportPath) -ChildPath "release-input-template.md"
}

$actionReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedActionReportPath | ConvertFrom-Json
$inputRows = [ordered]@{}

foreach ($action in @($actionReport.actions)) {
    foreach ($hint in @($action.inputResolutionHints)) {
        $inputName = [string]$hint.input
        if ([string]::IsNullOrWhiteSpace($inputName)) {
            continue
        }

        if (-not $inputRows.Contains($inputName)) {
            $inputRows[$inputName] = [ordered]@{
                input = $inputName
                placeholder = Get-Placeholder -InputName $inputName
                primaryParameter = ""
                parameters = New-Object System.Collections.Generic.List[string]
                environmentVariables = New-Object System.Collections.Generic.List[string]
                blockedBy = New-Object System.Collections.Generic.List[string]
            }
        }

        $row = $inputRows[$inputName]
        foreach ($parameter in @($hint.parameters)) {
            Add-UniqueString -Values $row.parameters -Value ([string]$parameter)
        }
        foreach ($environmentVariable in @($hint.environmentVariables)) {
            Add-UniqueString -Values $row.environmentVariables -Value ([string]$environmentVariable)
        }
        if ($action.automationBlocked -eq $true) {
            Add-UniqueString -Values $row.blockedBy -Value ([string]$action.key)
        }
    }
}

$inputs = @(
    $availableInputNames = @($inputRows.Keys)
    foreach ($row in $inputRows.Values) {
        $parameters = @($row.parameters.ToArray())
        $environmentVariables = @($row.environmentVariables.ToArray())
        $blockedBy = @($row.blockedBy.ToArray())
        $primaryParameter = if ($parameters.Count -gt 0) { [string]$parameters[0] } else { "" }
        $compatibleInputs = @(Get-CompatibleInputs -InputName ([string]$row.input) -AvailableInputNames $availableInputNames)

        [ordered]@{
            input = [string]$row.input
            placeholder = [string]$row.placeholder
            primaryParameter = $primaryParameter
            parameters = $parameters
            environmentVariables = $environmentVariables
            blockedBy = $blockedBy
            compatibleInputs = $compatibleInputs
        }
    }
)
$suggestedCommands = @(Get-SuggestedCommands -Inputs $inputs)

$template = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $actionReport.releaseId
    actionReportPath = $resolvedActionReportPath
    inputCount = $inputs.Count
    inputs = $inputs
    suggestedCommands = $suggestedCommands
}

$jsonDirectory = Split-Path -Parent $OutputJsonPath
if ($jsonDirectory -and -not (Test-Path -LiteralPath $jsonDirectory)) {
    New-Item -ItemType Directory -Force -Path $jsonDirectory | Out-Null
}
$markdownDirectory = Split-Path -Parent $OutputMarkdownPath
if ($markdownDirectory -and -not (Test-Path -LiteralPath $markdownDirectory)) {
    New-Item -ItemType Directory -Force -Path $markdownDirectory | Out-Null
}

$template | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Release input template")
$lines.Add("")
$lines.Add("- kind: release-input-template")
$lines.Add("- releaseId: $($template.releaseId)")
$lines.Add("- generatedAtUtc: $($template.generatedAtUtc)")
$lines.Add("- inputCount: $($template.inputCount)")
$lines.Add("")

if ($inputs.Count -gt 0) {
    $lines.Add("| Input | Placeholder | Primary parameter | Parameters | Env | Blocked by | Compatible inputs |")
    $lines.Add("| --- | --- | --- | --- | --- | --- | --- |")
    foreach ($inputRow in $inputs) {
        $parameters = if (@($inputRow.parameters).Count -gt 0) { @($inputRow.parameters) -join ", " } else { "" }
        $environmentVariables = if (@($inputRow.environmentVariables).Count -gt 0) { @($inputRow.environmentVariables) -join ", " } else { "" }
        $blockedBy = if (@($inputRow.blockedBy).Count -gt 0) { @($inputRow.blockedBy) -join ", " } else { "" }
        $compatibleInputs = if (@($inputRow.compatibleInputs).Count -gt 0) { @($inputRow.compatibleInputs) -join ", " } else { "" }
        $lines.Add("| $(Escape-MarkdownCell $inputRow.input) | ``$(Escape-MarkdownCell $inputRow.placeholder)`` | ``$(Escape-MarkdownCell $inputRow.primaryParameter)`` | $(Escape-MarkdownCell $parameters) | $(Escape-MarkdownCell $environmentVariables) | $(Escape-MarkdownCell $blockedBy) | $(Escape-MarkdownCell $compatibleInputs) |")
    }
}
else {
    $lines.Add("No automation-blocking release inputs were found.")
}

if ($suggestedCommands.Count -gt 0) {
    $lines.Add("")
    $lines.Add("## Suggested commands")
    $lines.Add("")
    foreach ($suggestedCommand in $suggestedCommands) {
        $lines.Add("### $($suggestedCommand.name)")
        $lines.Add("")
        $lines.Add('```powershell')
        $lines.Add($suggestedCommand.command)
        $lines.Add('```')
        $lines.Add("")
    }
}

$lines | Set-Content -LiteralPath $OutputMarkdownPath -Encoding UTF8

Write-Host ("Release input template JSON written: {0}" -f $OutputJsonPath)
Write-Host ("Release input template report written: {0}" -f $OutputMarkdownPath)
Write-Host ("Release input template rows: {0}" -f $inputs.Count)
