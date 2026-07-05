<#
Validates that the AI_Showcase automation suite checks the generated menu and
HUD widget trees, not just asset existence.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAAIShowcaseTests.cpp"
$runnerFixturePath = Join-Path $repoRoot "scripts\test-ai-showcase-automation-runner.ps1"

function New-CodePointText {
    param(
        [Parameter(Mandatory = $true)][int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

$missingWidgetTreeTokenPrefix = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0041, 0x0049, 0x005F, 0x0053, 0x0068, 0x006F, 0x0077, 0x0063, 0x0061, 0x0073, 0x0065, 0x0020, 0x63A7, 0x4EF6, 0x6811, 0x5951, 0x7EA6, 0x4EE4, 0x724C, 0xFF1A)
$missingRunnerCountTokenPrefix = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0041, 0x0049, 0x005F, 0x0053, 0x0068, 0x006F, 0x0077, 0x0063, 0x0061, 0x0073, 0x0065, 0x0020, 0x81EA, 0x52A8, 0x5316, 0x8BA1, 0x6570, 0x5951, 0x7EA6, 0x4EE4, 0x724C, 0xFF1A)
$passText = New-CodePointText @(0x901A, 0x8FC7, 0xFF1A, 0x0041, 0x0049, 0x005F, 0x0053, 0x0068, 0x006F, 0x0077, 0x0063, 0x0061, 0x0073, 0x0065, 0x0020, 0x63A7, 0x4EF6, 0x6811, 0x5951, 0x7EA6)

function Assert-ContainsToken {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$Token,
        [Parameter(Mandatory = $true)][string]$MissingPrefix
    )

    if (-not $Content.Contains($Token)) {
        throw "$MissingPrefix$Token"
    }
}

function Assert-MatchesToken {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$Pattern,
        [Parameter(Mandatory = $true)][string]$MissingPrefix
    )

    if ($Content -notmatch $Pattern) {
        throw "$MissingPrefix$Pattern"
    }
}

$testCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $testCppPath
$runnerFixture = Get-Content -Raw -Encoding UTF8 -LiteralPath $runnerFixturePath

Assert-ContainsToken $testCpp "AIShowcaseWidgetTreeContract" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseMenu_TitleText" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseMenu_StartButton" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseMenu_OptionsButton" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseMenu_QuitButton" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseHUD_HealthBar" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseHUD_EnergyBar" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseHUD_ScoreText" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseHUD_EventFeedBox" $missingWidgetTreeTokenPrefix
Assert-ContainsToken $testCpp "AIShowcaseHUD_SkillButton_0" $missingWidgetTreeTokenPrefix
Assert-MatchesToken $runnerFixture '\$evidence\.requestedTestCount -ne 5' $missingRunnerCountTokenPrefix
Assert-MatchesToken $runnerFixture '\$evidence\.passedTestCount -ne 5' $missingRunnerCountTokenPrefix

Write-Host $passText -ForegroundColor Green
