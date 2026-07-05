<#
Writes a fill-in release input values template from release-input-template.json.

This script does not run release commands. It creates a local values file that
can be filled with real release inputs, then checked with
resolve-release-input-template.ps1 before execution.

Examples:
  .\scripts\write-release-input-values-template.ps1
  .\scripts\write-release-input-values-template.ps1 -TemplatePath .\Artifacts\ProductionEvidence\release-input-template.json
#>

[CmdletBinding()]
param(
    [string]$TemplatePath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-input-template.json"),
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

$resolvedTemplatePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($TemplatePath)
if (-not (Test-Path -LiteralPath $resolvedTemplatePath)) {
    throw "release-input-template.json was not found: $resolvedTemplatePath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedTemplatePath) -ChildPath "release-input-values.template.json"
}
if ([string]::IsNullOrWhiteSpace($OutputMarkdownPath)) {
    $OutputMarkdownPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedTemplatePath) -ChildPath "release-input-values.template.md"
}

$template = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedTemplatePath | ConvertFrom-Json
$valueRows = @(
    foreach ($inputRow in @($template.inputs)) {
        [ordered]@{
            input = [string]$inputRow.input
            value = ""
            placeholder = [string]$inputRow.placeholder
            primaryParameter = [string]$inputRow.primaryParameter
            parameters = @($inputRow.parameters)
            environmentVariables = @($inputRow.environmentVariables)
            blockedBy = @($inputRow.blockedBy)
            compatibleInputs = @($inputRow.compatibleInputs)
        }
    }
)

$valuesTemplate = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-values"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $template.releaseId
    isTemplate = $true
    templatePath = $resolvedTemplatePath
    inputCount = $valueRows.Count
    values = $valueRows
}

$jsonDirectory = Split-Path -Parent $OutputJsonPath
if ($jsonDirectory -and -not (Test-Path -LiteralPath $jsonDirectory)) {
    New-Item -ItemType Directory -Force -Path $jsonDirectory | Out-Null
}
$markdownDirectory = Split-Path -Parent $OutputMarkdownPath
if ($markdownDirectory -and -not (Test-Path -LiteralPath $markdownDirectory)) {
    New-Item -ItemType Directory -Force -Path $markdownDirectory | Out-Null
}

$valuesTemplate | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("# release-input-values")
$lines.Add("")
$lines.Add("- kind: release-input-values")
$lines.Add("- releaseId: $($valuesTemplate.releaseId)")
$lines.Add("- isTemplate: true")
$lines.Add("- inputCount: $($valuesTemplate.inputCount)")
$lines.Add("")

if ($valueRows.Count -gt 0) {
    $lines.Add("| Input | Value | Placeholder | Primary parameter | Parameters | Env | Blocked by | Compatible inputs |")
    $lines.Add("| --- | --- | --- | --- | --- | --- | --- | --- |")
    foreach ($valueRow in $valueRows) {
        $parameters = if (@($valueRow.parameters).Count -gt 0) { @($valueRow.parameters) -join ", " } else { "" }
        $environmentVariables = if (@($valueRow.environmentVariables).Count -gt 0) { @($valueRow.environmentVariables) -join ", " } else { "" }
        $blockedBy = if (@($valueRow.blockedBy).Count -gt 0) { @($valueRow.blockedBy) -join ", " } else { "" }
        $compatibleInputs = if (@($valueRow.compatibleInputs).Count -gt 0) { @($valueRow.compatibleInputs) -join ", " } else { "" }
        $lines.Add("| $(Escape-MarkdownCell $valueRow.input) |  | ``$(Escape-MarkdownCell $valueRow.placeholder)`` | ``$(Escape-MarkdownCell $valueRow.primaryParameter)`` | $(Escape-MarkdownCell $parameters) | $(Escape-MarkdownCell $environmentVariables) | $(Escape-MarkdownCell $blockedBy) | $(Escape-MarkdownCell $compatibleInputs) |")
    }
}
else {
    $lines.Add("No release inputs are required.")
}

$lines | Set-Content -LiteralPath $OutputMarkdownPath -Encoding UTF8

Write-Host ("Release input values template JSON written: {0}" -f $OutputJsonPath)
Write-Host ("Release input values template report written: {0}" -f $OutputMarkdownPath)
Write-Host ("Release input values template rows: {0}" -f $valueRows.Count)
