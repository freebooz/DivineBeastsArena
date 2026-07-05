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

Assert-True ($ascCpp -match "CalculateResonanceLevel[\s\S]*DBAConstants::ResonanceLevel4_SkillCount") "Expected CalculateResonanceLevel to use DBAConstants::ResonanceLevel4_SkillCount."
Assert-True ($ascCpp -match "CalculateResonanceLevel[\s\S]*DBAConstants::ResonanceLevel3_SkillCount") "Expected CalculateResonanceLevel to use DBAConstants::ResonanceLevel3_SkillCount."
Assert-True ($ascCpp -match "CalculateResonanceLevel[\s\S]*DBAConstants::ResonanceLevel2_SkillCount") "Expected CalculateResonanceLevel to use DBAConstants::ResonanceLevel2_SkillCount."
Assert-True ($ascCpp -match "CalculateResonanceLevel[\s\S]*DBAConstants::ResonanceLevel1_SkillCount") "Expected CalculateResonanceLevel to use DBAConstants::ResonanceLevel1_SkillCount."
Assert-True ($ascCpp -match "CalculateResonanceLevel[\s\S]*return\s+DBAConstants::MaxResonanceLevel") "Expected CalculateResonanceLevel max branch to return DBAConstants::MaxResonanceLevel."

Assert-True ($ascCpp -notmatch "CalculateResonanceLevel[\s\S]{0,80}SameElementCount\s*>=\s*5") "Expected CalculateResonanceLevel to avoid hard-coded level 4 skill-count threshold."
Assert-True ($ascCpp -notmatch "CalculateResonanceLevel[\s\S]{0,160}SameElementCount\s*>=\s*4") "Expected CalculateResonanceLevel to avoid hard-coded level 3 skill-count threshold."
Assert-True ($ascCpp -notmatch "CalculateResonanceLevel[\s\S]{0,240}SameElementCount\s*>=\s*3") "Expected CalculateResonanceLevel to avoid hard-coded level 2 skill-count threshold."
Assert-True ($ascCpp -notmatch "CalculateResonanceLevel[\s\S]{0,320}SameElementCount\s*>=\s*2") "Expected CalculateResonanceLevel to avoid hard-coded level 1 skill-count threshold."

Write-Host "PASS: AbilitySystem resonance constants contract" -ForegroundColor Green
