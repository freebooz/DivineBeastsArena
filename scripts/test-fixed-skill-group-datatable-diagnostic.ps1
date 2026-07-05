<#
Exercises scripts/diagnose-fixed-skill-group-datatable.ps1 against lightweight fixtures.

The test keeps the real FixedSkillGroups DataTable release blocker reproducible
without requiring a live Unreal Editor for every script edit.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$diagnostic = Join-Path $repoRoot "scripts\diagnose-fixed-skill-group-datatable.ps1"
$testRoot = Join-Path $repoRoot (".tmp\fixed-skill-group-datatable-diagnostic-tests-{0}" -f ([System.Guid]::NewGuid().ToString("N")))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

$missingProjectMessage = New-TextFromCodePoints -CodePoints @(0x9879, 0x76EE, 0x6587, 0x4EF6, 0x4E0D, 0x5B58, 0x5728)
$missingEditorMessage = "UnrealEditor-Cmd.exe " + (New-TextFromCodePoints -CodePoints @(0x4E0D, 0x5B58, 0x5728))
$nonGamePackageMessage = New-TextFromCodePoints -CodePoints @(0x4EC5, 0x652F, 0x6301, 0x20, 0x2F, 0x47, 0x61, 0x6D, 0x65, 0x20, 0x5305, 0x8DEF, 0x5F84)

function Invoke-ExpectFailure {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action,
        [Parameter(Mandatory = $true)][string]$ExpectedMessage
    )

    try {
        & $Action
    }
    catch {
        if ($_.Exception.Message -notlike "*$ExpectedMessage*") {
            throw "$Name failed for the wrong reason: $($_.Exception.Message)"
        }

        Write-Host "PASS: $Name failed as expected" -ForegroundColor Green
        return
    }

    throw "$Name should have failed."
}

function Invoke-ExpectSuccess {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action
    )

    & $Action
    Write-Host "PASS: $Name" -ForegroundColor Green
}

New-Item -ItemType Directory -Force -Path $testRoot | Out-Null

Invoke-ExpectSuccess `
    -Name "diagnostic script exists" `
    -Action {
        if (-not (Test-Path -LiteralPath $diagnostic)) {
            throw "Missing required diagnostic script: $diagnostic"
        }
    }

Invoke-ExpectSuccess `
    -Name "diagnostic script defines Chinese runtime message fragments" `
    -Action {
        $diagnosticContent = Get-Content -LiteralPath $diagnostic -Raw -Encoding UTF8

        $requiredMessageTokens = @(
            "projectFileMissingMessage",
            "editorCmdMissingMessage",
            "assetFileMissingMessage",
            "automationFailedMessage",
            "completedMessage"
        )

        foreach ($token in $requiredMessageTokens) {
            if ($diagnosticContent -notlike "*$token*") {
                throw "Expected diagnostic script to define Chinese runtime message token '$token'."
            }
        }

        $forbiddenEnglishDiagnostics = @(
            "Only /Game package paths are supported by this diagnostic",
            "Project file was not found",
            "UnrealEditor-Cmd.exe was not found",
            "checking asset file",
            "FixedSkillGroups DataTable asset was not found",
            "FixedSkillGroups DataTable automation failed with exit code",
            "completed {0}"
        )

        foreach ($diagnosticMessage in $forbiddenEnglishDiagnostics) {
            if ($diagnosticContent -like "*$diagnosticMessage*") {
                throw "Diagnostic script should not expose English message: $diagnosticMessage"
            }
        }
    }

$fixtureRoot = Join-Path $testRoot "valid-fixture"
$unrealRoot = Join-Path $fixtureRoot "Unreal"
$projectRoot = Join-Path $fixtureRoot "Project"
$editorCmd = Join-Path $unrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$projectPath = Join-Path $projectRoot "DBA_GameClient\DivineBeastsArena.uproject"

New-Item -ItemType Directory -Force -Path (Split-Path $editorCmd -Parent), (Split-Path $projectPath -Parent) | Out-Null
Set-Content -LiteralPath $editorCmd -Encoding ASCII -Value "fixture"
Set-Content -LiteralPath $projectPath -Encoding UTF8 -Value "{}"

Invoke-ExpectFailure `
    -Name "non game package path fails in Chinese" `
    -ExpectedMessage $nonGamePackageMessage `
    -Action {
        & $diagnostic `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -AssetPackagePath "/Engine/Invalid" `
            -CommandOnly | Out-Null
    }

Invoke-ExpectSuccess `
    -Name "command-only output includes the expected editor automation command" `
    -Action {
        $result = & $diagnostic `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -CommandOnly

        if ($result.FilePath -ne $editorCmd) {
            throw "Expected FilePath '$editorCmd' but got '$($result.FilePath)'."
        }

        if ($result.WorkingDirectory -ne $repoRoot) {
            throw "Expected WorkingDirectory '$repoRoot' but got '$($result.WorkingDirectory)'."
        }

        if ($result.AssetPackagePath -ne "/Game/DBA/Data/Tables/DT_FixedSkillGroups") {
            throw "Unexpected asset package path: $($result.AssetPackagePath)"
        }

        $expectedArgs = @(
            $projectPath,
            "-Unattended",
            "-NullRHI",
            "-NoSound",
            "-NoSplash",
            "-ExecCmds=Automation RunTests DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows; Quit",
            "-TestExit=Automation Test Queue Empty"
        )

        if (@($result.Arguments).Count -ne $expectedArgs.Count) {
            throw "Expected $($expectedArgs.Count) arguments but got $(@($result.Arguments).Count)."
        }

        for ($i = 0; $i -lt $expectedArgs.Count; $i++) {
            if ($result.Arguments[$i] -ne $expectedArgs[$i]) {
                throw "Argument[$i] expected '$($expectedArgs[$i])' but got '$($result.Arguments[$i])'."
            }
        }
    }

Invoke-ExpectFailure `
    -Name "missing project path fails fast" `
    -ExpectedMessage $missingProjectMessage `
    -Action {
        & $diagnostic `
            -UnrealRoot $unrealRoot `
            -ProjectPath (Join-Path $projectRoot "Missing\DivineBeastsArena.uproject") `
            -CommandOnly | Out-Null
    }

Invoke-ExpectFailure `
    -Name "missing UnrealEditor-Cmd fails fast" `
    -ExpectedMessage $missingEditorMessage `
    -Action {
        & $diagnostic `
            -UnrealRoot (Join-Path $fixtureRoot "MissingUnreal") `
            -ProjectPath $projectPath `
            -CommandOnly | Out-Null
    }
