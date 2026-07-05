<#
Runs the AI_Showcase Unreal automation regression suite through UnrealEditor-Cmd.

This keeps the MCP-generated showcase sample wired into a repeatable repository
entrypoint instead of relying on ad-hoc shell history. It intentionally avoids
the Gauntlet wrapper because UE 5.8 source builds can emit engine low-level
smoke-test errors before the requested project automation filter starts.
#>

[CmdletBinding()]
param(
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$ProjectPath = "",
    [string]$TestFilter = "DivineBeastsArena.Showcase.AIShowcase",
    [string]$Platform = "Win64",
    [string]$Configuration = "Development",
    [string]$EvidenceDir = "",
    [string]$RunId = "",
    [string]$LogPath = "",
    [string]$EditorCmdPath = "",
    [switch]$CommandOnly
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[ai-showcase-automation] {0}" -f $Message) -ForegroundColor Cyan
}

function Get-LastRegexInt {
    param(
        [AllowEmptyString()]
        [string[]]$Lines,
        [Parameter(Mandatory = $true)][string]$Pattern
    )

    $value = 0
    foreach ($line in $Lines) {
        if ([string]::IsNullOrWhiteSpace([string]$line)) {
            continue
        }

        $match = [regex]::Match([string]$line, $Pattern)
        if ($match.Success) {
            $value = [int]$match.Groups[1].Value
        }
    }

    return $value
}

function Get-MatchCount {
    param(
        [AllowEmptyString()]
        [string[]]$Lines,
        [Parameter(Mandatory = $true)][string]$Pattern
    )

    $count = 0
    foreach ($line in $Lines) {
        if ([string]::IsNullOrWhiteSpace([string]$line)) {
            continue
        }

        if ([regex]::IsMatch([string]$line, $Pattern)) {
            $count++
        }
    }

    return $count
}

function Select-AutomationSessionLines {
    param(
        [AllowEmptyString()]
        [string[]]$Lines
    )

    $startIndex = -1
    $endIndex = $Lines.Count - 1
    for ($i = 0; $i -lt $Lines.Count; $i++) {
        if ([regex]::IsMatch([string]$Lines[$i], "Found\s+\d+\s+automation tests based on")) {
            $startIndex = $i
            break
        }
    }

    if ($startIndex -lt 0) {
        return $Lines
    }

    for ($i = $startIndex; $i -lt $Lines.Count; $i++) {
        if ([regex]::IsMatch([string]$Lines[$i], "\*\*\*\*\s+TEST COMPLETE\.\s+EXIT CODE:")) {
            $endIndex = $i
            break
        }
    }

    return $Lines[$startIndex..$endIndex]
}

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject"
}

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-ai-showcase-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

$resolvedProjectPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ProjectPath)
if (-not (Test-Path -LiteralPath $resolvedProjectPath)) {
    throw "Project file was not found: $resolvedProjectPath"
}

if ([string]::IsNullOrWhiteSpace($EditorCmdPath)) {
    $editorCmd = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
}
else {
    $editorCmd = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EditorCmdPath)
}

if (-not (Test-Path -LiteralPath $editorCmd)) {
    throw "UnrealEditor-Cmd.exe was not found: $editorCmd"
}

if ([string]::IsNullOrWhiteSpace($LogPath)) {
    $LogPath = Join-Path $repoRoot ("Artifacts\Logs\ai-showcase-automation-{0}.log" -f $RunId)
}

$arguments = @(
    $resolvedProjectPath,
    "-unattended",
    "-nullrhi",
    "-nosplash",
    "-log",
    "-abslog=$LogPath",
    "-ExecCmds=Automation RunTests $TestFilter; Quit",
    "-TestExit=Automation Test Queue Empty"
)

$command = [PSCustomObject]@{
    FilePath = $editorCmd
    WorkingDirectory = $repoRoot
    Arguments = $arguments
    LogPath = $LogPath
    EvidencePath = if ([string]::IsNullOrWhiteSpace($EvidenceDir)) { "" } else { Join-Path $EvidenceDir ("unreal\ai-showcase-automation-{0}.json" -f $RunId) }
}

if ($CommandOnly) {
    return $command
}

Write-Step ("running {0}" -f $TestFilter)
Write-Step ("UnrealEditor-Cmd: {0}" -f $editorCmd)

$logDirectory = Split-Path -Parent $LogPath
if (-not (Test-Path -LiteralPath $logDirectory)) {
    New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
}

$editorOutput = @()
Push-Location $repoRoot
try {
    $editorOutput = @(& $editorCmd @arguments 2>&1)
    $editorSucceeded = $?
    $editorOutput | ForEach-Object { Write-Host $_ }
    if (-not $editorSucceeded) {
        $exitCode = if ($null -ne $LASTEXITCODE) { [string]$LASTEXITCODE } else { "unknown" }
        throw "UnrealEditor-Cmd failed with exit code $exitCode"
    }
}
finally {
    Pop-Location
}

if (-not (Test-Path -LiteralPath $LogPath)) {
    throw "Expected UnrealEditor-Cmd log was not found: $LogPath"
}

$logLines = @(Get-Content -LiteralPath $LogPath -ErrorAction Stop)
$automationSessionLines = @(Select-AutomationSessionLines -Lines $logLines)
$logErrorCount = Get-MatchCount -Lines $automationSessionLines -Pattern "Log[^:]+:\s+Error:"
$logWarningCount = Get-MatchCount -Lines $automationSessionLines -Pattern "Log[^:]+:\s+Warning:"
$requestedTestCount = Get-LastRegexInt -Lines $logLines -Pattern "Found\s+(\d+)\s+automation tests based on"
if ($requestedTestCount -eq 0) {
    $requestedTestCount = Get-LastRegexInt -Lines $logLines -Pattern "(\d+)\s+Test\(s\)\s+Requested"
}

$passedTestCount = Get-MatchCount -Lines $logLines -Pattern "Test Completed\.\s+Result=\{Success\}"
if ($passedTestCount -eq 0) {
    $passedTestCount = Get-LastRegexInt -Lines $logLines -Pattern "(\d+)\s+Test\(s\)\s+Passed"
}

$automationReady = $logErrorCount -eq 0 -and $requestedTestCount -gt 0 -and $requestedTestCount -eq $passedTestCount

if (-not [string]::IsNullOrWhiteSpace($EvidenceDir)) {
    $evidencePath = $command.EvidencePath
    $evidenceDirectory = Split-Path -Parent $evidencePath
    if (-not (Test-Path -LiteralPath $evidenceDirectory)) {
        New-Item -ItemType Directory -Force -Path $evidenceDirectory | Out-Null
    }

    [ordered]@{
        kind = "ai-showcase-automation"
        runId = $RunId
        generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
        automationReady = $automationReady
        logErrorCount = $logErrorCount
        logWarningCount = $logWarningCount
        requestedTestCount = $requestedTestCount
        passedTestCount = $passedTestCount
        testFilter = $TestFilter
        platform = $Platform
        configuration = $Configuration
        projectPath = $resolvedProjectPath
        command = [ordered]@{
            filePath = $editorCmd
            workingDirectory = $repoRoot
            arguments = $arguments
            logPath = $LogPath
        }
    } | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8

    Write-Step ("evidence written: {0}" -f $evidencePath)
}

Write-Step ("completed {0}" -f $TestFilter)
return $command
