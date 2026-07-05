<#
Validates release-input-template.json command placeholders.

Examples:
  .\scripts\validate-release-input-template.ps1
  .\scripts\validate-release-input-template.ps1 -TemplatePath .\Artifacts\ProductionEvidence\release-input-template.json -RequireValid
#>

[CmdletBinding()]
param(
    [string]$TemplatePath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-input-template.json"),
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

function Add-UniqueString {
    param(
        [System.Collections.Generic.List[string]]$Values,
        [string]$Value
    )

    if (-not [string]::IsNullOrWhiteSpace($Value) -and -not $Values.Contains($Value)) {
        $Values.Add($Value)
    }
}

function Get-RepoRoot {
    return (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
}

function Get-ScriptDeclaredParameters {
    param([Parameter(Mandatory = $true)][string]$ScriptPath)

    $tokens = $null
    $parseErrors = $null
    $ast = [System.Management.Automation.Language.Parser]::ParseFile($ScriptPath, [ref]$tokens, [ref]$parseErrors)
    if ($parseErrors.Count -gt 0) {
        throw "PowerShell script could not be parsed for parameter validation: $ScriptPath"
    }

    $parameters = New-Object System.Collections.Generic.List[string]
    foreach ($parameter in @($ast.ParamBlock.Parameters)) {
        Add-UniqueString -Values $parameters -Value ("-" + $parameter.Name.VariablePath.UserPath)
    }

    return $parameters.ToArray()
}

function Get-CommandParameters {
    param([Parameter(Mandatory = $true)][string]$CommandText)

    $parameters = New-Object System.Collections.Generic.List[string]
    foreach ($match in [regex]::Matches($CommandText, "(?<!\S)-[A-Za-z][A-Za-z0-9_]*")) {
        Add-UniqueString -Values $parameters -Value ([string]$match.Value)
    }

    return $parameters.ToArray()
}

$resolvedTemplatePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($TemplatePath)
if (-not (Test-Path -LiteralPath $resolvedTemplatePath)) {
    throw "release-input-template.json was not found: $resolvedTemplatePath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedTemplatePath) -ChildPath "release-input-template-validation.json"
}

$template = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedTemplatePath | ConvertFrom-Json
$declaredPlaceholders = New-Object System.Collections.Generic.List[string]
$declaredInputs = New-Object System.Collections.Generic.List[string]
foreach ($inputRow in @($template.inputs)) {
    Add-UniqueString -Values $declaredPlaceholders -Value ([string]$inputRow.placeholder)
    Add-UniqueString -Values $declaredInputs -Value ([string]$inputRow.input)
}

$usedPlaceholders = New-Object System.Collections.Generic.List[string]
$missingPlaceholders = New-Object System.Collections.Generic.List[object]
$checkedScripts = New-Object System.Collections.Generic.List[string]
$missingScripts = New-Object System.Collections.Generic.List[object]
$invalidParameters = New-Object System.Collections.Generic.List[object]
$missingInputReferences = New-Object System.Collections.Generic.List[object]
$missingCommandInputReferences = New-Object System.Collections.Generic.List[object]
$missingCompatibleInputReferences = New-Object System.Collections.Generic.List[object]
$asymmetricCompatibleInputReferences = New-Object System.Collections.Generic.List[object]
$placeholderPattern = "<[^<>]+>"
$repoRoot = Get-RepoRoot

foreach ($command in @($template.suggestedCommands)) {
    $commandText = [string]$command.command
    $scriptRelativePath = [string]$command.script
    $scriptPath = if ([string]::IsNullOrWhiteSpace($scriptRelativePath)) {
        ""
    }
    else {
        Join-Path $repoRoot $scriptRelativePath
    }

    if ([string]::IsNullOrWhiteSpace($scriptPath) -or -not (Test-Path -LiteralPath $scriptPath)) {
        $missingScripts.Add([ordered]@{
            commandName = [string]$command.name
            script = $scriptRelativePath
            command = $commandText
        })
    }
    else {
        Add-UniqueString -Values $checkedScripts -Value $scriptRelativePath
        $declaredParameters = @(Get-ScriptDeclaredParameters -ScriptPath $scriptPath)
        foreach ($commandParameter in @(Get-CommandParameters -CommandText $commandText)) {
            if ($declaredParameters -notcontains $commandParameter) {
                $invalidParameters.Add([ordered]@{
                    commandName = [string]$command.name
                    script = $scriptRelativePath
                    parameter = $commandParameter
                    command = $commandText
                })
            }
        }
    }

    $commandInputNames = New-Object System.Collections.Generic.List[string]
    $commandInputPlaceholders = New-Object System.Collections.Generic.List[string]
    foreach ($usedInput in @($command.usesInputs)) {
        $inputName = [string]$usedInput
        if (-not [string]::IsNullOrWhiteSpace($inputName)) {
            Add-UniqueString -Values $commandInputNames -Value $inputName
        }
    }

    foreach ($inputRow in @($template.inputs)) {
        $inputName = [string]$inputRow.input
        if ($commandInputNames.Contains($inputName)) {
            Add-UniqueString -Values $commandInputPlaceholders -Value ([string]$inputRow.placeholder)
        }
    }

    foreach ($match in [regex]::Matches($commandText, $placeholderPattern)) {
        $placeholder = [string]$match.Value
        Add-UniqueString -Values $usedPlaceholders -Value $placeholder
        if (-not $declaredPlaceholders.Contains($placeholder)) {
            $missingPlaceholders.Add([ordered]@{
                commandName = [string]$command.name
                script = [string]$command.script
                placeholder = $placeholder
                command = $commandText
            })
        }
        elseif (-not $commandInputPlaceholders.Contains($placeholder)) {
            $matchingInput = @($template.inputs | Where-Object { [string]$_.placeholder -eq $placeholder } | Select-Object -First 1)
            $missingCommandInputReferences.Add([ordered]@{
                commandName = [string]$command.name
                script = [string]$command.script
                input = if ($matchingInput.Count -gt 0) { [string]$matchingInput[0].input } else { "" }
                placeholder = $placeholder
                command = $commandText
            })
        }
    }

    foreach ($usedInput in @($command.usesInputs)) {
        $inputName = [string]$usedInput
        if (-not [string]::IsNullOrWhiteSpace($inputName) -and -not $declaredInputs.Contains($inputName)) {
            $missingInputReferences.Add([ordered]@{
                commandName = [string]$command.name
                script = [string]$command.script
                input = $inputName
                command = $commandText
            })
        }
    }
}

foreach ($inputRow in @($template.inputs)) {
    $inputName = [string]$inputRow.input
    foreach ($compatibleInput in @($inputRow.compatibleInputs)) {
        $compatibleInputName = [string]$compatibleInput
        if (-not [string]::IsNullOrWhiteSpace($compatibleInputName) -and -not $declaredInputs.Contains($compatibleInputName)) {
            $missingCompatibleInputReferences.Add([ordered]@{
                input = $inputName
                compatibleInput = $compatibleInputName
            })
        }
        elseif (-not [string]::IsNullOrWhiteSpace($compatibleInputName)) {
            $compatibleInputRow = @($template.inputs | Where-Object { [string]$_.input -eq $compatibleInputName } | Select-Object -First 1)
            if ($compatibleInputRow.Count -gt 0 -and @($compatibleInputRow[0].compatibleInputs) -notcontains $inputName) {
                $asymmetricCompatibleInputReferences.Add([ordered]@{
                    input = $inputName
                    compatibleInput = $compatibleInputName
                })
            }
        }
    }
}

$actualInputCount = @($template.inputs).Count
$hasDeclaredInputCount = $template.PSObject.Properties.Name -contains "inputCount"
$declaredInputCount = if ($hasDeclaredInputCount) {
    [int]$template.inputCount
}
else {
    -1
}
$inputCountMatches = $hasDeclaredInputCount -and $declaredInputCount -eq $actualInputCount

$isValid = $missingPlaceholders.Count -eq 0 -and $missingScripts.Count -eq 0 -and $invalidParameters.Count -eq 0 -and $missingInputReferences.Count -eq 0 -and $missingCommandInputReferences.Count -eq 0 -and $missingCompatibleInputReferences.Count -eq 0 -and $asymmetricCompatibleInputReferences.Count -eq 0 -and $inputCountMatches
$result = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-input-template-validation"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $template.releaseId
    templatePath = $resolvedTemplatePath
    isValid = $isValid
    inputCount = $actualInputCount
    declaredInputCount = $declaredInputCount
    actualInputCount = $actualInputCount
    inputCountMatches = $inputCountMatches
    commandCount = @($template.suggestedCommands).Count
    scriptCount = $checkedScripts.Count
    placeholderCount = $usedPlaceholders.Count
    declaredPlaceholderCount = $declaredPlaceholders.Count
    missingPlaceholderCount = $missingPlaceholders.Count
    missingScriptCount = $missingScripts.Count
    invalidParameterCount = $invalidParameters.Count
    missingInputReferenceCount = $missingInputReferences.Count
    missingCommandInputReferenceCount = $missingCommandInputReferences.Count
    missingCompatibleInputReferenceCount = $missingCompatibleInputReferences.Count
    asymmetricCompatibleInputReferenceCount = $asymmetricCompatibleInputReferences.Count
    missingPlaceholders = @($missingPlaceholders.ToArray())
    missingScripts = @($missingScripts.ToArray())
    invalidParameters = @($invalidParameters.ToArray())
    missingInputReferences = @($missingInputReferences.ToArray())
    missingCommandInputReferences = @($missingCommandInputReferences.ToArray())
    missingCompatibleInputReferences = @($missingCompatibleInputReferences.ToArray())
    asymmetricCompatibleInputReferences = @($asymmetricCompatibleInputReferences.ToArray())
}

$outputDirectory = Split-Path -Parent $OutputJsonPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

Write-Host ("Release input template validation JSON written: {0}" -f $OutputJsonPath)
if ($isValid) {
    Write-Host "Release input template validation: valid" -ForegroundColor Green
}
else {
    Write-Host ("Release input template validation: invalid ({0} missing placeholder(s), {1} missing script(s), {2} invalid parameter(s), {3} missing input reference(s), {4} missing command input reference(s), {5} missing compatible input reference(s), {6} asymmetric compatible input reference(s), inputCountMatches={7})" -f $missingPlaceholders.Count, $missingScripts.Count, $invalidParameters.Count, $missingInputReferences.Count, $missingCommandInputReferences.Count, $missingCompatibleInputReferences.Count, $asymmetricCompatibleInputReferences.Count, $inputCountMatches) -ForegroundColor Yellow
}

if ($RequireValid -and -not $isValid) {
    throw ("Release input template validation failed with {0} missing placeholder(s), {1} missing script(s), {2} invalid parameter(s), {3} missing input reference(s), {4} missing command input reference(s), {5} missing compatible input reference(s), {6} asymmetric compatible input reference(s), and inputCountMatches={7}." -f $missingPlaceholders.Count, $missingScripts.Count, $invalidParameters.Count, $missingInputReferences.Count, $missingCommandInputReferences.Count, $missingCompatibleInputReferences.Count, $asymmetricCompatibleInputReferences.Count, $inputCountMatches)
}
