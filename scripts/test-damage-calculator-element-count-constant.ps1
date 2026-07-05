param()

$ErrorActionPreference = "Stop"

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$constantsPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Core\DBAConstants.h"
$calculatorPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp"

$constants = Get-Content -Raw -Encoding UTF8 $constantsPath
$calculator = Get-Content -Raw -Encoding UTF8 $calculatorPath

Assert-True ($constants -match "constexpr\s+int32\s+ElementCount\s*=\s*5\s*;") `
    "Expected DBAConstants::ElementCount to define the five natural elements."

Assert-True ($calculator.Contains("CounterMap[DBAConstants::ElementCount]")) `
    "Expected DBADamageCalculator counter maps to use DBAConstants::ElementCount."

Assert-True ($calculator.Contains("i < DBAConstants::ElementCount")) `
    "Expected DBADamageCalculator counter loops to use DBAConstants::ElementCount."

Assert-True (-not ($calculator -match "CounterMap\[5\]")) `
    "DBADamageCalculator still hardcodes CounterMap[5]."

Assert-True (-not ($calculator -match "i\s*<\s*5")) `
    "DBADamageCalculator still hardcodes element loop length with i < 5."

Write-Host "PASS: DamageCalculator element count constant contract"
