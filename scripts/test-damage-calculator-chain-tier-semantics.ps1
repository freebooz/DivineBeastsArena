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
$calculatorPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp"
$calculator = Get-Content -Raw -Encoding UTF8 $calculatorPath

Assert-True ($calculator -match "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier2Threshold[\s\S]*return\s+DBAConstants::ChainTier2DamageBonus") `
    "Expected GetChainMultiplier to use ChainTier2Threshold for ChainTier2DamageBonus."
Assert-True ($calculator -match "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold[\s\S]*return\s+DBAConstants::ChainTier1DamageBonus") `
    "Expected GetChainMultiplier to use ChainTier1Threshold for ChainTier1DamageBonus."
Assert-True ($calculator -notmatch "GetChainMultiplier[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold\s*\)\s*\{[\s\S]{0,180}return\s+DBAConstants::ChainTier2DamageBonus") `
    "GetChainMultiplier maps ChainTier1Threshold to ChainTier2DamageBonus."

Assert-True ($calculator -match "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier2Threshold[\s\S]*Bonus\.Multiplier\s*=\s*DBAConstants::ChainTier2DamageBonus") `
    "Expected GetChainBonus to use ChainTier2Threshold for ChainTier2DamageBonus."
Assert-True ($calculator -match "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold[\s\S]*Bonus\.Multiplier\s*=\s*DBAConstants::ChainTier1DamageBonus") `
    "Expected GetChainBonus to use ChainTier1Threshold for ChainTier1DamageBonus."
Assert-True ($calculator -notmatch "GetChainBonus[\s\S]*ChainLevel\s*>=\s*DBAConstants::ChainTier1Threshold\s*\)\s*\{[\s\S]{0,220}Bonus\.Multiplier\s*=\s*DBAConstants::ChainTier2DamageBonus") `
    "GetChainBonus maps ChainTier1Threshold to ChainTier2DamageBonus."

Write-Host "PASS: DamageCalculator chain tier semantics contract"
