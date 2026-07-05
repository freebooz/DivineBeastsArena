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

$expectedConstants = @(
    "DBAConstants::ResonanceLevel1_DamageBonus",
    "DBAConstants::ResonanceLevel2_DamageBonus",
    "DBAConstants::ResonanceLevel3_DamageBonus",
    "DBAConstants::ResonanceLevel4_DamageBonus"
)

foreach ($constant in $expectedConstants) {
    $constantName = $constant.Replace("DBAConstants::", "")
    Assert-True ($constants -match ("constexpr\s+float\s+{0}\s*=" -f [regex]::Escape($constantName))) `
        "Expected $constant to define resonance damage bonus."
    Assert-True ($calculator.Contains($constant)) `
        "Expected DBADamageCalculator to use $constant."
}

Assert-True (-not ($calculator -match "case\s+1:\s*return\s+0\.05f")) `
    "GetResonanceBonus still hardcodes Level1 damage bonus."
Assert-True (-not ($calculator -match "case\s+2:\s*return\s+0\.10f")) `
    "GetResonanceBonus still hardcodes Level2 damage bonus."
Assert-True (-not ($calculator -match "case\s+3:\s*return\s+0\.15f")) `
    "GetResonanceBonus still hardcodes Level3 damage bonus."
Assert-True (-not ($calculator -match "case\s+4:\s*return\s+0\.20f")) `
    "GetResonanceBonus still hardcodes Level4 damage bonus."
Assert-True (-not ($calculator -match "EDBAResonanceLevel::Level1:\s*return\s+0\.05f")) `
    "GetResonanceBonusForElement still hardcodes Level1 damage bonus."
Assert-True (-not ($calculator -match "EDBAResonanceLevel::Level2:\s*return\s+0\.10f")) `
    "GetResonanceBonusForElement still hardcodes Level2 damage bonus."
Assert-True (-not ($calculator -match "EDBAResonanceLevel::Level3:\s*return\s+0\.15f")) `
    "GetResonanceBonusForElement still hardcodes Level3 damage bonus."
Assert-True (-not ($calculator -match "EDBAResonanceLevel::Level4:\s*return\s+0\.20f")) `
    "GetResonanceBonusForElement still hardcodes Level4 damage bonus."

Write-Host "PASS: DamageCalculator resonance damage constants contract"
