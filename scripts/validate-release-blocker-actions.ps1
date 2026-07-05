<#
Validates release-blocker-actions.json nextCommand drafts.

Examples:
  .\scripts\validate-release-blocker-actions.ps1
  .\scripts\validate-release-blocker-actions.ps1 -ActionReportPath .\Artifacts\ProductionEvidence\release-blocker-actions.json -RequireValid
#>

[CmdletBinding()]
param(
    [string]$ActionReportPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-blocker-actions.json"),
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

function Get-Placeholder {
    param([string]$InputName)

    $slug = ([string]$InputName).ToLowerInvariant() -replace "[^a-z0-9]+", "-"
    $slug = $slug.Trim("-")
    if ([string]::IsNullOrWhiteSpace($slug)) {
        return "<release-input>"
    }

    return "<$slug>"
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

function Get-CommandScriptPath {
    param([Parameter(Mandatory = $true)][string]$CommandText)

    $match = [regex]::Match($CommandText, "(?i)(?:^|\s)(?:\.\\)?(scripts[\\/][^\s'`"]+?\.ps1)")
    if (-not $match.Success) {
        return ""
    }

    return ([string]$match.Groups[1].Value) -replace "/", "\"
}

function Normalize-RelativePath {
    param([string]$Path)

    return (([string]$Path).Trim() -replace "/", "\").TrimStart(".\")
}

$resolvedActionReportPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ActionReportPath)
if (-not (Test-Path -LiteralPath $resolvedActionReportPath)) {
    throw "release-blocker-actions.json was not found: $resolvedActionReportPath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedActionReportPath) -ChildPath "release-blocker-action-validation.json"
}

$actionReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedActionReportPath | ConvertFrom-Json
$actionReportKind = [string]$actionReport.kind
$kindIsValid = $actionReportKind -eq "release-blocker-actions"
$repoRoot = Get-RepoRoot
$placeholderPattern = "<[^<>]+>"
$declaredPlaceholders = New-Object System.Collections.Generic.List[string]
$usedPlaceholders = New-Object System.Collections.Generic.List[string]
$checkedScripts = New-Object System.Collections.Generic.List[string]
$manualActions = New-Object System.Collections.Generic.List[object]
$missingPlaceholders = New-Object System.Collections.Generic.List[object]
$missingScripts = New-Object System.Collections.Generic.List[object]
$invalidParameters = New-Object System.Collections.Generic.List[object]
$scriptMismatches = New-Object System.Collections.Generic.List[object]
$missingActionKeys = New-Object System.Collections.Generic.List[object]
$missingActionCommands = New-Object System.Collections.Generic.List[object]
$seenActionKeys = @{}
$duplicateActionKeys = New-Object System.Collections.Generic.List[string]
$blockerCountProperty = $actionReport.PSObject.Properties["blockerCount"]
$blockerCountIsPresent = $null -ne $blockerCountProperty -and -not [string]::IsNullOrWhiteSpace([string]$blockerCountProperty.Value)
$reportedBlockerCount = if ($blockerCountIsPresent) { [int]$blockerCountProperty.Value } else { 0 }
$actualActionCount = @($actionReport.actions).Count
$blockerCountMatchesActions = $blockerCountIsPresent -and $reportedBlockerCount -eq $actualActionCount
$linkedReportPathRaw = [string]$actionReport.reportPath
$reportPathIsPresent = -not [string]::IsNullOrWhiteSpace($linkedReportPathRaw)
$resolvedLinkedReportPath = ""
$reportPathExists = $false
$linkedReportKind = ""
$reportKindIsValid = $false
$reportReleaseId = ""
$reportReleaseIdMatches = $false
$reportReadError = ""

if ($reportPathIsPresent) {
    $resolvedLinkedReportPath = if ([System.IO.Path]::IsPathRooted($linkedReportPathRaw)) {
        $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($linkedReportPathRaw)
    }
    else {
        Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedActionReportPath) -ChildPath $linkedReportPathRaw
    }

    $reportPathExists = Test-Path -LiteralPath $resolvedLinkedReportPath
    if ($reportPathExists) {
        try {
            $linkedReport = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedLinkedReportPath | ConvertFrom-Json
            $linkedReportKind = [string]$linkedReport.kind
            $reportKindIsValid = $linkedReportKind -eq "release-readiness-report"
            $reportReleaseId = [string]$linkedReport.releaseId
            $reportReleaseIdMatches = -not [string]::IsNullOrWhiteSpace($reportReleaseId) -and $reportReleaseId -eq [string]$actionReport.releaseId
        }
        catch {
            $reportReadError = $_.Exception.Message
        }
    }
}

foreach ($action in @($actionReport.actions)) {
    foreach ($hint in @($action.inputResolutionHints)) {
        Add-UniqueString -Values $declaredPlaceholders -Value (Get-Placeholder -InputName ([string]$hint.input))
    }
}

foreach ($action in @($actionReport.actions)) {
    $actionKey = [string]$action.key
    $commandText = [string]$action.nextCommand
    $scriptRelativePath = [string]$action.script
    $scriptPath = if ([string]::IsNullOrWhiteSpace($scriptRelativePath)) {
        ""
    }
    else {
        Join-Path $repoRoot $scriptRelativePath
    }

    if ([string]::IsNullOrWhiteSpace($actionKey)) {
        $missingActionKeys.Add([ordered]@{
            command = $commandText
            script = $scriptRelativePath
        })
    }
    elseif ($seenActionKeys.ContainsKey($actionKey)) {
        Add-UniqueString -Values $duplicateActionKeys -Value $actionKey
    }
    else {
        $seenActionKeys[$actionKey] = $true
    }

    if ([string]::IsNullOrWhiteSpace($commandText)) {
        $missingActionCommands.Add([ordered]@{
            key = $actionKey
            script = $scriptRelativePath
        })
    }

    if ([string]::IsNullOrWhiteSpace($scriptRelativePath)) {
        $manualActions.Add([ordered]@{
            key = $actionKey
            command = $commandText
        })
    }
    elseif (-not (Test-Path -LiteralPath $scriptPath)) {
        $missingScripts.Add([ordered]@{
            key = $actionKey
            script = $scriptRelativePath
            command = $commandText
        })
    }
    else {
        Add-UniqueString -Values $checkedScripts -Value $scriptRelativePath
        $commandScriptPath = Get-CommandScriptPath -CommandText $commandText
        if (-not [string]::IsNullOrWhiteSpace($commandScriptPath) -and
            (Normalize-RelativePath $commandScriptPath) -ne (Normalize-RelativePath $scriptRelativePath)) {
            $scriptMismatches.Add([ordered]@{
                key = $actionKey
                script = $scriptRelativePath
                commandScript = $commandScriptPath
                command = $commandText
            })
        }

        $declaredParameters = @(Get-ScriptDeclaredParameters -ScriptPath $scriptPath)
        foreach ($commandParameter in @(Get-CommandParameters -CommandText $commandText)) {
            if ($declaredParameters -notcontains $commandParameter) {
                $invalidParameters.Add([ordered]@{
                    key = $actionKey
                    script = $scriptRelativePath
                    parameter = $commandParameter
                    command = $commandText
                })
            }
        }
    }

    foreach ($match in [regex]::Matches($commandText, $placeholderPattern)) {
        $placeholder = [string]$match.Value
        Add-UniqueString -Values $usedPlaceholders -Value $placeholder
        if (-not $declaredPlaceholders.Contains($placeholder)) {
            $missingPlaceholders.Add([ordered]@{
                key = $actionKey
                script = $scriptRelativePath
                placeholder = $placeholder
                command = $commandText
            })
        }
    }
}

$isValid = $kindIsValid -and $reportPathIsPresent -and $reportPathExists -and $reportKindIsValid -and $reportReleaseIdMatches -and $missingActionKeys.Count -eq 0 -and $missingActionCommands.Count -eq 0 -and $duplicateActionKeys.Count -eq 0 -and $missingPlaceholders.Count -eq 0 -and $missingScripts.Count -eq 0 -and $invalidParameters.Count -eq 0 -and $scriptMismatches.Count -eq 0 -and $blockerCountMatchesActions
$result = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-action-validation"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $actionReport.releaseId
    actionReportPath = $resolvedActionReportPath
    reportKind = $actionReportKind
    kindIsValid = $kindIsValid
    linkedReportPath = $resolvedLinkedReportPath
    reportPathIsPresent = $reportPathIsPresent
    reportPathExists = $reportPathExists
    linkedReportKind = $linkedReportKind
    reportKindIsValid = $reportKindIsValid
    reportReleaseId = $reportReleaseId
    reportReleaseIdMatches = $reportReleaseIdMatches
    reportReadError = $reportReadError
    isValid = $isValid
    actionCount = $actualActionCount
    blockerCountIsPresent = $blockerCountIsPresent
    reportedBlockerCount = $reportedBlockerCount
    blockerCountMatchesActions = $blockerCountMatchesActions
    scriptCount = $checkedScripts.Count
    manualActionCount = $manualActions.Count
    placeholderCount = $usedPlaceholders.Count
    declaredPlaceholderCount = $declaredPlaceholders.Count
    missingActionKeyCount = $missingActionKeys.Count
    missingActionCommandCount = $missingActionCommands.Count
    duplicateActionKeyCount = $duplicateActionKeys.Count
    missingPlaceholderCount = $missingPlaceholders.Count
    missingScriptCount = $missingScripts.Count
    invalidParameterCount = $invalidParameters.Count
    scriptMismatchCount = $scriptMismatches.Count
    manualActions = @($manualActions.ToArray())
    missingActionKeys = @($missingActionKeys.ToArray())
    missingActionCommands = @($missingActionCommands.ToArray())
    duplicateActionKeys = @($duplicateActionKeys.ToArray())
    missingPlaceholders = @($missingPlaceholders.ToArray())
    missingScripts = @($missingScripts.ToArray())
    invalidParameters = @($invalidParameters.ToArray())
    scriptMismatches = @($scriptMismatches.ToArray())
}

$outputDirectory = Split-Path -Parent $OutputJsonPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

Write-Host ("Release blocker action validation JSON written: {0}" -f $OutputJsonPath)
if ($isValid) {
    Write-Host "Release blocker action validation: valid" -ForegroundColor Green
}
else {
    Write-Host ("Release blocker action validation: invalid ({0} missing placeholder(s), {1} missing script(s), {2} invalid parameter(s), {3} script mismatch(es))" -f $missingPlaceholders.Count, $missingScripts.Count, $invalidParameters.Count, $scriptMismatches.Count) -ForegroundColor Yellow
}

if ($RequireValid -and -not $isValid) {
    throw ("Release blocker action validation failed with {0} missing placeholder(s), {1} missing script(s), {2} invalid parameter(s), and {3} script mismatch(es)." -f $missingPlaceholders.Count, $missingScripts.Count, $invalidParameters.Count, $scriptMismatches.Count)
}
