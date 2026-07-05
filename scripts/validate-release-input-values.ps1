<#
Validates release-input-values JSON before resolving release commands.

Examples:
  .\scripts\validate-release-input-values.ps1
  .\scripts\validate-release-input-values.ps1 -ValuesPath .\Artifacts\ProductionEvidence\release-input-values.json -RequireValid
#>

[CmdletBinding()]
param(
    [string]$ValuesPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-input-values.json"),
    [string]$OutputJsonPath = "",
    [switch]$RequireValid
)

$ErrorActionPreference = "Stop"

function Resolve-DefaultPath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$ChildPath
    )

    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath((Join-Path $BasePath $ChildPath))
}

function Add-Issue {
    param(
        [System.Collections.Generic.List[object]]$Issues,
        [object]$Row,
        [string]$Reason
    )

    $Issues.Add([ordered]@{
        input = [string]$Row.input
        placeholder = [string]$Row.placeholder
        value = [string]$Row.value
        primaryParameter = [string]$Row.primaryParameter
        reason = $Reason
    })
}

$resolvedValuesPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ValuesPath)
if (-not (Test-Path -LiteralPath $resolvedValuesPath)) {
    throw "release input values file was not found: $resolvedValuesPath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedValuesPath) -ChildPath "release-input-values-validation.json"
}

$valuesDocument = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedValuesPath | ConvertFrom-Json
$blankValues = New-Object System.Collections.Generic.List[object]
$placeholderValues = New-Object System.Collections.Generic.List[object]
$exampleUrls = New-Object System.Collections.Generic.List[object]
$insecureUrls = New-Object System.Collections.Generic.List[object]
$rows = @($valuesDocument.values)

foreach ($row in $rows) {
    $value = [string]$row.value
    $placeholder = [string]$row.placeholder

    if ([string]::IsNullOrWhiteSpace($value)) {
        Add-Issue -Issues $blankValues -Row $row -Reason "value is blank"
        continue
    }

    if ($value -eq $placeholder -or $value -match "^<[^<>]+>$") {
        Add-Issue -Issues $placeholderValues -Row $row -Reason "value is still a placeholder"
    }

    if ($value -match "(?i)\bexample\.(com|net|org)\b" -or $value -match "(?i)\bcdn\.example\.com\b") {
        Add-Issue -Issues $exampleUrls -Row $row -Reason "value points at an example URL"
    }

    $looksLikeUrlInput = ([string]$row.input) -match "(?i)\bURL\b" -or ([string]$row.primaryParameter) -match "(?i)Url$"
    if ($looksLikeUrlInput -and $value -match "^(?i:http)://" ) {
        Add-Issue -Issues $insecureUrls -Row $row -Reason "URL value must use HTTPS"
    }
}

$actualInputCount = $rows.Count
$declaredInputCount = if ($valuesDocument.PSObject.Properties.Name -contains "inputCount") {
    [int]$valuesDocument.inputCount
}
else {
    -1
}
$inputCountMatches = $declaredInputCount -eq $actualInputCount
$isValid = $blankValues.Count -eq 0 -and $placeholderValues.Count -eq 0 -and $exampleUrls.Count -eq 0 -and $insecureUrls.Count -eq 0 -and $inputCountMatches

$result = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-values-validation"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $valuesDocument.releaseId
    valuesPath = $resolvedValuesPath
    isValid = $isValid
    inputCount = $actualInputCount
    declaredInputCount = $declaredInputCount
    actualInputCount = $actualInputCount
    inputCountMatches = $inputCountMatches
    blankValueCount = $blankValues.Count
    placeholderValueCount = $placeholderValues.Count
    exampleUrlCount = $exampleUrls.Count
    insecureUrlCount = $insecureUrls.Count
    blankValues = @($blankValues.ToArray())
    placeholderValues = @($placeholderValues.ToArray())
    exampleUrls = @($exampleUrls.ToArray())
    insecureUrls = @($insecureUrls.ToArray())
}

$outputDirectory = Split-Path -Parent $OutputJsonPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

Write-Host ("Release input values validation JSON written: {0}" -f $OutputJsonPath)
if ($isValid) {
    Write-Host "Release input values validation: valid" -ForegroundColor Green
}
else {
    Write-Host ("Release input values validation: invalid ({0} blank value(s), {1} placeholder value(s), {2} example URL(s), {3} insecure URL(s), inputCountMatches={4})" -f $blankValues.Count, $placeholderValues.Count, $exampleUrls.Count, $insecureUrls.Count, $inputCountMatches) -ForegroundColor Yellow
}

if ($RequireValid -and -not $isValid) {
    throw ("Release input values validation failed with {0} blank value(s), {1} placeholder value(s), {2} example URL(s), {3} insecure URL(s), and inputCountMatches={4}." -f $blankValues.Count, $placeholderValues.Count, $exampleUrls.Count, $insecureUrls.Count, $inputCountMatches)
}
