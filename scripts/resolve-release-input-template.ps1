<#
Resolves release-input-template suggested commands with explicit input values.

This script does not run release commands. It writes a reviewable command plan
and fails under -RequireComplete if any required command placeholder remains.

Examples:
  .\scripts\resolve-release-input-template.ps1 -ValuesPath .\release-input-values.json
  .\scripts\resolve-release-input-template.ps1 -TemplatePath .\Artifacts\ProductionEvidence\release-input-template.json -ValuesPath .\release-input-values.json -RequireComplete
#>

[CmdletBinding()]
param(
    [string]$TemplatePath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-input-template.json"),
    [Parameter(Mandatory = $true)][string]$ValuesPath,
    [string]$OutputJsonPath = "",
    [string]$OutputMarkdownPath = "",
    [switch]$RequireComplete
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

function Add-UniqueObject {
    param(
        [System.Collections.Generic.List[object]]$Values,
        [object]$Value,
        [string]$Key
    )

    foreach ($existing in $Values) {
        if ([string]$existing.$Key -eq [string]$Value.$Key) {
            return
        }
    }

    $Values.Add($Value) | Out-Null
}

function Add-ValueIssue {
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
    }) | Out-Null
}

function Test-ReleaseInputValues {
    param([Parameter(Mandatory = $true)][object]$ValuesDocument)

    $blankValues = [System.Collections.Generic.List[object]]::new()
    $placeholderValues = [System.Collections.Generic.List[object]]::new()
    $exampleUrls = [System.Collections.Generic.List[object]]::new()
    $insecureUrls = [System.Collections.Generic.List[object]]::new()
    $rows = @($ValuesDocument.values)

    foreach ($row in $rows) {
        $value = [string]$row.value
        $placeholder = [string]$row.placeholder

        if ([string]::IsNullOrWhiteSpace($value)) {
            Add-ValueIssue -Issues $blankValues -Row $row -Reason "value is blank"
            continue
        }

        if ($value -eq $placeholder -or $value -match "^<[^<>]+>$") {
            Add-ValueIssue -Issues $placeholderValues -Row $row -Reason "value is still a placeholder"
        }

        if ($value -match "(?i)\bexample\.(com|net|org)\b" -or $value -match "(?i)\bcdn\.example\.com\b") {
            Add-ValueIssue -Issues $exampleUrls -Row $row -Reason "value points at an example URL"
        }

        $looksLikeUrlInput = ([string]$row.input) -match "(?i)\bURL\b" -or ([string]$row.primaryParameter) -match "(?i)Url$"
        if ($looksLikeUrlInput -and $value -match "^(?i:http)://") {
            Add-ValueIssue -Issues $insecureUrls -Row $row -Reason "URL value must use HTTPS"
        }
    }

    $actualInputCount = $rows.Count
    $declaredInputCount = if ($ValuesDocument.PSObject.Properties.Name -contains "inputCount") {
        [int]$ValuesDocument.inputCount
    }
    else {
        $actualInputCount
    }
    $inputCountMatches = $declaredInputCount -eq $actualInputCount
    $isValid = $blankValues.Count -eq 0 -and $placeholderValues.Count -eq 0 -and $exampleUrls.Count -eq 0 -and $insecureUrls.Count -eq 0 -and $inputCountMatches

    return [ordered]@{
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
}

$resolvedTemplatePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($TemplatePath)
$resolvedValuesPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ValuesPath)
if (-not (Test-Path -LiteralPath $resolvedTemplatePath)) {
    throw "release-input-template.json was not found: $resolvedTemplatePath"
}
if (-not (Test-Path -LiteralPath $resolvedValuesPath)) {
    throw "release input values JSON was not found: $resolvedValuesPath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedValuesPath) -ChildPath "release-command-plan.json"
}
if ([string]::IsNullOrWhiteSpace($OutputMarkdownPath)) {
    $OutputMarkdownPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedValuesPath) -ChildPath "release-command-plan.md"
}

$template = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedTemplatePath | ConvertFrom-Json
$valuesDocument = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedValuesPath | ConvertFrom-Json
$valuesValidation = Test-ReleaseInputValues -ValuesDocument $valuesDocument
$valueByInput = @{}
foreach ($valueRow in @($valuesDocument.values)) {
    $inputName = [string]$valueRow.input
    if (-not [string]::IsNullOrWhiteSpace($inputName)) {
        $valueByInput[$inputName] = [string]$valueRow.value
    }
}

$placeholderByInput = @{}
$inputByPlaceholder = @{}
foreach ($inputRow in @($template.inputs)) {
    $inputName = [string]$inputRow.input
    $placeholder = [string]$inputRow.placeholder
    if (-not [string]::IsNullOrWhiteSpace($inputName) -and -not [string]::IsNullOrWhiteSpace($placeholder)) {
        $placeholderByInput[$inputName] = $placeholder
        $inputByPlaceholder[$placeholder] = $inputName
    }
}

$missingInputs = [System.Collections.Generic.List[object]]::new()
$allUnresolvedPlaceholders = [System.Collections.Generic.List[object]]::new()
$resolvedCommands = [System.Collections.Generic.List[object]]::new()
$placeholderPattern = "<[^<>]+>"

foreach ($suggestedCommand in @($template.suggestedCommands)) {
    $resolvedCommandText = [string]$suggestedCommand.command
    foreach ($usedInput in @($suggestedCommand.usesInputs)) {
        $inputName = [string]$usedInput
        if (-not $placeholderByInput.ContainsKey($inputName)) {
            continue
        }
        $placeholder = [string]$placeholderByInput[$inputName]
        if ($valueByInput.ContainsKey($inputName) -and -not [string]::IsNullOrWhiteSpace([string]$valueByInput[$inputName])) {
            $resolvedCommandText = $resolvedCommandText.Replace($placeholder, [string]$valueByInput[$inputName])
        }
        else {
            Add-UniqueObject -Values $missingInputs -Key "placeholder" -Value ([ordered]@{
                input = $inputName
                placeholder = $placeholder
            })
        }
    }

    $commandUnresolvedPlaceholders = [System.Collections.Generic.List[object]]::new()
    foreach ($match in [regex]::Matches($resolvedCommandText, $placeholderPattern)) {
        $placeholder = [string]$match.Value
        $inputName = if ($inputByPlaceholder.ContainsKey($placeholder)) { [string]$inputByPlaceholder[$placeholder] } else { "" }
        $detail = [ordered]@{
            commandName = [string]$suggestedCommand.name
            input = $inputName
            placeholder = $placeholder
        }
        Add-UniqueObject -Values $commandUnresolvedPlaceholders -Key "placeholder" -Value $detail
        Add-UniqueObject -Values $allUnresolvedPlaceholders -Key "placeholder" -Value $detail
    }

    $resolvedCommands.Add([ordered]@{
        name = [string]$suggestedCommand.name
        script = [string]$suggestedCommand.script
        command = $resolvedCommandText
        usesInputs = @($suggestedCommand.usesInputs)
        unresolvedPlaceholderCount = $commandUnresolvedPlaceholders.Count
        unresolvedPlaceholders = @($commandUnresolvedPlaceholders.ToArray())
    }) | Out-Null
}

$isComplete = $missingInputs.Count -eq 0 -and $allUnresolvedPlaceholders.Count -eq 0 -and $valuesValidation.isValid
$plan = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-command-plan"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $template.releaseId
    templatePath = $resolvedTemplatePath
    valuesPath = $resolvedValuesPath
    isComplete = $isComplete
    inputCount = @($template.inputs).Count
    providedInputCount = $valueByInput.Count
    missingInputCount = $missingInputs.Count
    unresolvedPlaceholderCount = $allUnresolvedPlaceholders.Count
    valuesValidation = $valuesValidation
    commandCount = $resolvedCommands.Count
    missingInputs = @($missingInputs.ToArray())
    unresolvedPlaceholders = @($allUnresolvedPlaceholders.ToArray())
    commands = @($resolvedCommands.ToArray())
}

$jsonDirectory = Split-Path -Parent $OutputJsonPath
if ($jsonDirectory -and -not (Test-Path -LiteralPath $jsonDirectory)) {
    New-Item -ItemType Directory -Force -Path $jsonDirectory | Out-Null
}
$markdownDirectory = Split-Path -Parent $OutputMarkdownPath
if ($markdownDirectory -and -not (Test-Path -LiteralPath $markdownDirectory)) {
    New-Item -ItemType Directory -Force -Path $markdownDirectory | Out-Null
}

$plan | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("# release-command-plan")
$lines.Add("")
$lines.Add("- kind: release-command-plan")
$lines.Add("- releaseId: $($plan.releaseId)")
$lines.Add("- isComplete: $($plan.isComplete)")
$lines.Add("- commandCount: $($plan.commandCount)")
$lines.Add("- missingInputCount: $($plan.missingInputCount)")
$lines.Add("- unresolvedPlaceholderCount: $($plan.unresolvedPlaceholderCount)")
$lines.Add("- valuesValid: $($plan.valuesValidation.isValid)")
$lines.Add("")

if (-not $valuesValidation.isValid) {
    $lines.Add("## Invalid values")
    $lines.Add("")
    $lines.Add("- blankValueCount: $($valuesValidation.blankValueCount)")
    $lines.Add("- placeholderValueCount: $($valuesValidation.placeholderValueCount)")
    $lines.Add("- exampleUrlCount: $($valuesValidation.exampleUrlCount)")
    $lines.Add("- insecureUrlCount: $($valuesValidation.insecureUrlCount)")
    $lines.Add("- inputCountMatches: $($valuesValidation.inputCountMatches)")
    $lines.Add("")
}

if ($missingInputs.Count -gt 0) {
    $lines.Add("## Missing inputs")
    $lines.Add("")
    foreach ($missingInput in @($missingInputs.ToArray())) {
        $lines.Add("- $($missingInput.input): ``$($missingInput.placeholder)``")
    }
    $lines.Add("")
}

$lines.Add("## Commands")
$lines.Add("")
foreach ($command in @($resolvedCommands.ToArray())) {
    $lines.Add("### $($command.name)")
    $lines.Add("")
    $lines.Add("- script: $(Escape-MarkdownCell $command.script)")
    $lines.Add("- unresolvedPlaceholderCount: $($command.unresolvedPlaceholderCount)")
    $lines.Add("")
    $lines.Add('```powershell')
    $lines.Add($command.command)
    $lines.Add('```')
    $lines.Add("")
}

$lines | Set-Content -LiteralPath $OutputMarkdownPath -Encoding UTF8

Write-Host ("Release command plan JSON written: {0}" -f $OutputJsonPath)
Write-Host ("Release command plan report written: {0}" -f $OutputMarkdownPath)
if ($isComplete) {
    Write-Host "Release command plan: complete" -ForegroundColor Green
}
else {
    Write-Host ("Release command plan: incomplete ({0} missing input(s), {1} unresolved placeholder(s), valuesValid={2})" -f $missingInputs.Count, $allUnresolvedPlaceholders.Count, $valuesValidation.isValid) -ForegroundColor Yellow
}

if ($RequireComplete -and -not $isComplete) {
    throw ("Release command plan is incomplete with {0} missing input(s), {1} unresolved placeholder(s), and valuesValid={2}." -f $missingInputs.Count, $allUnresolvedPlaceholders.Count, $valuesValidation.isValid)
}
