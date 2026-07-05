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
$calculatorCpp = Get-Content -Raw -Encoding UTF8 $calculatorCppPath

Assert-True ($calculatorCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected DamageCalculator to include DBAConstants."
Assert-True ($calculatorCpp -match "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::MaxChainLevel") "Expected GetChainMultiplier final gate to use DBAConstants::MaxChainLevel."
Assert-True ($calculatorCpp -match "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier2Threshold") "Expected GetChainMultiplier tier2 gate to use DBAConstants::ChainTier2Threshold."
Assert-True ($calculatorCpp -match "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold") "Expected GetChainMultiplier tier gate to use DBAConstants::ChainTier1Threshold."
Assert-True ($calculatorCpp -match "GetChainMultiplier[\s\S]*return\s+DBAConstants::ChainTier2DamageBonus") "Expected GetChainMultiplier high-chain multiplier to use DBAConstants::ChainTier2DamageBonus."
Assert-True ($calculatorCpp -match "GetChainMultiplier[\s\S]*return\s+DBAConstants::ChainTier1DamageBonus") "Expected GetChainMultiplier low-chain multiplier to use DBAConstants::ChainTier1DamageBonus."
Assert-True ($calculatorCpp -match "IsChainFinal[\s\S]*ChainLevel\s*>=\s*DBAConstants::MaxChainLevel") "Expected IsChainFinal to use DBAConstants::MaxChainLevel."
Assert-True ($calculatorCpp -match "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::MaxChainLevel") "Expected GetChainBonus final gate to use DBAConstants::MaxChainLevel."
Assert-True ($calculatorCpp -match "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier2Threshold") "Expected GetChainBonus tier2 gate to use DBAConstants::ChainTier2Threshold."
Assert-True ($calculatorCpp -match "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold") "Expected GetChainBonus tier gate to use DBAConstants::ChainTier1Threshold."
Assert-True ($calculatorCpp -match "GetChainBonus[\s\S]*Bonus\.Multiplier\s*=\s*DBAConstants::ChainTier2DamageBonus") "Expected GetChainBonus high-chain multiplier to use DBAConstants::ChainTier2DamageBonus."
Assert-True ($calculatorCpp -match "GetChainBonus[\s\S]*Bonus\.Multiplier\s*=\s*DBAConstants::ChainTier1DamageBonus") "Expected GetChainBonus low-chain multiplier to use DBAConstants::ChainTier1DamageBonus."

Assert-True ($calculatorCpp -notmatch "GetChainMultiplier[\s\S]{0,300}ChainLevel\s*>=\s*10") "Expected GetChainMultiplier to avoid hard-coded final chain threshold."
Assert-True ($calculatorCpp -notmatch "GetChainMultiplier[\s\S]{0,300}ChainLevel\s*>=\s*6") "Expected GetChainMultiplier to avoid hard-coded tier threshold."
Assert-True ($calculatorCpp -notmatch "GetChainBonus[\s\S]{0,360}ChainLevel\s*>=\s*10") "Expected GetChainBonus to avoid hard-coded final chain threshold."
Assert-True ($calculatorCpp -notmatch "GetChainBonus[\s\S]{0,360}ChainLevel\s*>=\s*6") "Expected GetChainBonus to avoid hard-coded tier threshold."

Write-Host "PASS: DamageCalculator chain constants contract" -ForegroundColor Green
