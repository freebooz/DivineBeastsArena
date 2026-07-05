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

$ascCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$ascCpp = Get-Content -Raw -Encoding UTF8 $ascCppPath

Assert-True ($ascCpp -match "PassiveRegenUltimateEnergy[\s\S]*AddUltimateEnergy\s*\(\s*DBAConstants::UltimateEnergy_PassiveRegen\s*\)") "Expected PassiveRegenUltimateEnergy to use DBAConstants::UltimateEnergy_PassiveRegen."
Assert-True ($ascCpp -notmatch "PassiveRegenUltimateEnergy[\s\S]{0,120}AddUltimateEnergy\s*\(\s*1\.0f\s*\)") "Expected PassiveRegenUltimateEnergy to avoid hard-coded passive regen amount."

Write-Host "PASS: AbilitySystem UltimateEnergy passive regen constant contract" -ForegroundColor Green
