param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

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

$calculatorCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp"
$battleAttributeSetCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Attributes\DBABattleAttributeSet.cpp"

$calculatorCpp = Get-Content -Raw -Encoding UTF8 $calculatorCppPath
$battleAttributeSetCpp = Get-Content -Raw -Encoding UTF8 $battleAttributeSetCppPath

Assert-True ($calculatorCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected DamageCalculator to include DBAConstants."
Assert-True ($battleAttributeSetCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected BattleAttributeSet to include DBAConstants."

Assert-True ($calculatorCpp -match "CalculateFinalDamage[\s\S]*Defense\s*/\s*\(Defense\s*\+\s*DBAConstants::DefenseReductionConstant\)") "Expected CalculateFinalDamage to use DBAConstants::DefenseReductionConstant."
Assert-True ($calculatorCpp -match "CalculateFinalDamageWithObject[\s\S]*Params\.Defense\s*/\s*\(Params\.Defense\s*\+\s*DBAConstants::DefenseReductionConstant\)") "Expected CalculateFinalDamageWithObject to use DBAConstants::DefenseReductionConstant."
Assert-True ($battleAttributeSetCpp -match "CalculatePhysicalDamageReduction[\s\S]*DefenseValue\s*/\s*\(DefenseValue\s*\+\s*DBAConstants::DefenseReductionConstant\)") "Expected CalculatePhysicalDamageReduction to use DBAConstants::DefenseReductionConstant."

Assert-True ($calculatorCpp -notmatch "CalculateFinalDamage[\s\S]{0,500}Defense\s*\+\s*100\.0f") "Expected CalculateFinalDamage to avoid hard-coded defense reduction constant."
Assert-True ($calculatorCpp -notmatch "CalculateFinalDamageWithObject[\s\S]{0,500}Params\.Defense\s*\+\s*100\.0f") "Expected CalculateFinalDamageWithObject to avoid hard-coded defense reduction constant."
Assert-True ($battleAttributeSetCpp -notmatch "CalculatePhysicalDamageReduction[\s\S]{0,240}DefenseValue\s*\+\s*100\.0f") "Expected CalculatePhysicalDamageReduction to avoid hard-coded defense reduction constant."

Write-Host "PASS: Defense reduction constant contract" -ForegroundColor Green
