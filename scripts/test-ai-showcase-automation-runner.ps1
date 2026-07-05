<#
Exercises scripts/run-ai-showcase-automation.ps1 against lightweight fixtures.

The test keeps AI_Showcase automation wiring verifiable without requiring a
live Unreal session for every edit.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$runner = Join-Path $repoRoot "scripts\run-ai-showcase-automation.ps1"
$testRoot = Join-Path $repoRoot (".tmp\ai-showcase-automation-runner-tests-{0}" -f [guid]::NewGuid().ToString("N"))

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

if (Test-Path -LiteralPath $testRoot) {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $testRoot | Out-Null

Invoke-ExpectSuccess `
    -Name "runner script exists" `
    -Action {
        if (-not (Test-Path -LiteralPath $runner)) {
            throw "Missing required runner script: $runner"
        }
    }

$fixtureRoot = Join-Path $testRoot "valid-fixture"
$unrealRoot = Join-Path $fixtureRoot "Unreal"
$projectRoot = Join-Path $fixtureRoot "Project"
$editorCmd = Join-Path $unrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$fixtureEditorCmd = Join-Path $fixtureRoot "fake-unreal-editor.ps1"
$projectPath = Join-Path $projectRoot "DBA_GameClient\DivineBeastsArena.uproject"

New-Item -ItemType Directory -Force -Path (Split-Path $editorCmd -Parent), (Split-Path $projectPath -Parent) | Out-Null
Set-Content -LiteralPath $editorCmd -Encoding ASCII -Value "@echo off`necho fixture"
Set-Content -LiteralPath $projectPath -Encoding UTF8 -Value "{}"

Invoke-ExpectSuccess `
    -Name "command-only output includes the expected UnrealEditor-Cmd command" `
    -Action {
        $result = & $runner `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -CommandOnly

        if ($result.FilePath -ne $editorCmd) {
            throw "Expected FilePath '$editorCmd' but got '$($result.FilePath)'."
        }

        if ($result.WorkingDirectory -ne $repoRoot) {
            throw "Expected WorkingDirectory '$repoRoot' but got '$($result.WorkingDirectory)'."
        }

        $expectedArgs = @(
            $projectPath,
            '-unattended',
            '-nullrhi',
            '-nosplash',
            '-log',
            $result.Arguments[5],
            '-ExecCmds=Automation RunTests DivineBeastsArena.Showcase.AIShowcase; Quit',
            '-TestExit=Automation Test Queue Empty'
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

$evidenceDir = Join-Path $testRoot "evidence"
Invoke-ExpectSuccess `
    -Name "runner evidence records editor log error and warning counts" `
    -Action {
        $runId = "fixture-runner-log-summary"
        $fixtureEditorOutput = @(
            '$ErrorActionPreference = "Stop"',
            '$log = ""',
            'foreach ($arg in $args) {',
            '    if ($arg -like "-abslog=*") { $log = $arg.Substring(8) }',
            '}',
            'if ([string]::IsNullOrWhiteSpace($log)) { throw "Missing -abslog argument." }',
            'Write-Host "fixture editor run"',
            '@(',
            '"[fixture]LogAutomationCommandLine: Display: Found 5 automation tests based on ''DivineBeastsArena.Showcase.AIShowcase''",',
            '"[fixture]LogFixture: Error: first fixture error",',
            '"[fixture]LogFixture: Error: second fixture error",',
            '"[fixture]LogFixture: Error: third fixture error",',
            '"[fixture]LogFixture: Warning: first fixture warning",',
            '"[fixture]LogFixture: Warning: second fixture warning",',
            '"[fixture]LogFixture: Warning: third fixture warning",',
            '"[fixture]LogFixture: Warning: fourth fixture warning",',
            '"[fixture]LogFixture: Warning: fifth fixture warning",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={AssetsExist}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={WidgetTreeContract}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={InteractionContract}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={InteractivePropDefaults}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={MapPlacement}"',
            ') | Set-Content -LiteralPath $log -Encoding UTF8'
        ) -join "`r`n"
        Set-Content -LiteralPath $fixtureEditorCmd -Encoding UTF8 -Value $fixtureEditorOutput

        & $runner `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -EvidenceDir $evidenceDir `
            -RunId $runId `
            -EditorCmdPath $fixtureEditorCmd | Out-Null

        $evidencePath = Join-Path $evidenceDir "unreal\ai-showcase-automation-$runId.json"
        if (-not (Test-Path -LiteralPath $evidencePath)) {
            throw "Expected evidence JSON to be written: $evidencePath"
        }

        $evidence = Get-Content -Raw -Encoding UTF8 -LiteralPath $evidencePath | ConvertFrom-Json
        if ($evidence.logErrorCount -ne 3) {
            throw "Expected logErrorCount=3 but got '$($evidence.logErrorCount)'."
        }
        if ($evidence.logWarningCount -ne 5) {
            throw "Expected logWarningCount=5 but got '$($evidence.logWarningCount)'."
        }
        if ($evidence.requestedTestCount -ne 5 -or $evidence.passedTestCount -ne 5) {
            throw "Expected requested/passed test counts to be 5/5 but got '$($evidence.requestedTestCount)/$($evidence.passedTestCount)'."
        }
        if ($evidence.automationReady) {
            throw "Evidence with logErrorCount > 0 must not be automationReady."
        }
    }

Invoke-ExpectSuccess `
    -Name "runner ignores pre-session engine startup smoke errors" `
    -Action {
        $runId = "fixture-pre-session-smoke-errors"
        $fixtureEditorOutput = @(
            '$ErrorActionPreference = "Stop"',
            '$log = ""',
            'foreach ($arg in $args) {',
            '    if ($arg -like "-abslog=*") { $log = $arg.Substring(8) }',
            '}',
            'if ([string]::IsNullOrWhiteSpace($log)) { throw "Missing -abslog argument." }',
            '@(',
            '"[fixture]LogTemp: Error test: UE::UnifiedErrorTest::Empty: [empty error]",',
            '"[fixture]LogAutomationTest: Error: Condition failed",',
            '"[fixture]LogAutomationCommandLine: Display: Found 5 automation tests based on ''DivineBeastsArena.Showcase.AIShowcase''",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={AssetsExist}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={WidgetTreeContract}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={InteractionContract}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={InteractivePropDefaults}",',
            '"[fixture]LogAutomationController: Display: Test Completed. Result={Success} Name={MapPlacement}",',
            '"[fixture]LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: 0 ****"',
            ') | Set-Content -LiteralPath $log -Encoding UTF8'
        ) -join "`r`n"
        Set-Content -LiteralPath $fixtureEditorCmd -Encoding UTF8 -Value $fixtureEditorOutput

        & $runner `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -EvidenceDir $evidenceDir `
            -RunId $runId `
            -EditorCmdPath $fixtureEditorCmd | Out-Null

        $evidencePath = Join-Path $evidenceDir "unreal\ai-showcase-automation-$runId.json"
        $evidence = Get-Content -Raw -Encoding UTF8 -LiteralPath $evidencePath | ConvertFrom-Json
        if ($evidence.logErrorCount -ne 0) {
            throw "Expected pre-session smoke errors to be ignored but got logErrorCount='$($evidence.logErrorCount)'."
        }
        if (-not $evidence.automationReady) {
            throw "Expected pre-session smoke errors not to block automationReady."
        }
    }

Invoke-ExpectFailure `
    -Name "missing project path fails fast" `
    -ExpectedMessage "Project file was not found" `
    -Action {
        & $runner `
            -UnrealRoot $unrealRoot `
            -ProjectPath (Join-Path $projectRoot "Missing\DivineBeastsArena.uproject") `
            -CommandOnly | Out-Null
    }

Invoke-ExpectFailure `
    -Name "missing UnrealEditor-Cmd fails fast" `
    -ExpectedMessage "UnrealEditor-Cmd.exe was not found" `
    -Action {
        & $runner `
            -UnrealRoot (Join-Path $fixtureRoot "MissingUnreal") `
            -ProjectPath $projectPath `
            -CommandOnly | Out-Null
    }
