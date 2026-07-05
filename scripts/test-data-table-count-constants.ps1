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
$zodiacAssetPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Data\DBAZodiacHeroDataAsset.cpp"
$abilitySetPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Data\DBAAbilitySetDataAsset.cpp"
$staticDataPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Data\DBAStaticDataAsset.cpp"

$constants = Get-Content -Raw -Encoding UTF8 $constantsPath
$zodiacAsset = Get-Content -Raw -Encoding UTF8 $zodiacAssetPath
$abilitySet = Get-Content -Raw -Encoding UTF8 $abilitySetPath
$staticData = Get-Content -Raw -Encoding UTF8 $staticDataPath

Assert-True ($constants -match "constexpr\s+int32\s+ZodiacCount\s*=\s*12\s*;") `
    "Expected DBAConstants::ZodiacCount to define the twelve zodiac heroes."
Assert-True ($constants -match "constexpr\s+int32\s+ElementAbilityPositionCount\s*=\s*DBAConstants::ActiveSkillCount\s*\+\s*1\s*;") `
    "Expected DBAConstants::ElementAbilityPositionCount to derive element active rows per element."
Assert-True ($constants -match "constexpr\s+int32\s+ElementActiveAbilityRowCount\s*=\s*DBAConstants::ElementCount\s*\*\s*DBAConstants::ElementAbilityPositionCount\s*;") `
    "Expected DBAConstants::ElementActiveAbilityRowCount to derive active ability table rows."
Assert-True ($constants -match "constexpr\s+int32\s+ElementResonanceRowCount\s*=\s*DBAConstants::ElementCount\s*\*\s*\(DBAConstants::MaxResonanceLevel\s*\+\s*1\)\s*;") `
    "Expected DBAConstants::ElementResonanceRowCount to derive resonance table rows."
Assert-True ($constants -match "constexpr\s+int32\s+FixedSkillGroupRowCount\s*=\s*DBAConstants::ZodiacCount\s*\*\s*DBAConstants::ElementCount\s*;") `
    "Expected DBAConstants::FixedSkillGroupRowCount to derive Zodiac x Element rows."

Assert-True ($zodiacAsset.Contains("DBAConstants::ZodiacCount")) `
    "Expected DBAZodiacHeroDataAsset to use DBAConstants::ZodiacCount."
Assert-True ($zodiacAsset.Contains("DBAConstants::FixedSkillGroupRowCount")) `
    "Expected DBAZodiacHeroDataAsset to use DBAConstants::FixedSkillGroupRowCount."
Assert-True ($abilitySet.Contains("DBAConstants::ElementCount")) `
    "Expected DBAAbilitySetDataAsset to use DBAConstants::ElementCount."
Assert-True ($abilitySet.Contains("DBAConstants::ElementActiveAbilityRowCount")) `
    "Expected DBAAbilitySetDataAsset to use DBAConstants::ElementActiveAbilityRowCount."
Assert-True ($abilitySet.Contains("DBAConstants::ElementResonanceRowCount")) `
    "Expected DBAAbilitySetDataAsset to use DBAConstants::ElementResonanceRowCount."
Assert-True ($abilitySet.Contains("DBAConstants::ZodiacCount")) `
    "Expected DBAAbilitySetDataAsset to use DBAConstants::ZodiacCount."
Assert-True ($staticData.Contains("DBAConstants::ElementCount")) `
    "Expected DBAStaticDataAsset to use DBAConstants::ElementCount for Element and FiveCamp row validation."

Assert-True (-not ($zodiacAsset -match "RowNames\.Num\(\)\s*!=\s*12")) `
    "DBAZodiacHeroDataAsset still hardcodes zodiac row count 12."
Assert-True (-not ($zodiacAsset -match "RowNames\.Num\(\)\s*!=\s*60")) `
    "DBAZodiacHeroDataAsset still hardcodes fixed skill group row count 60."
Assert-True (-not ($abilitySet -match "RowNames\.Num\(\)\s*!=\s*5")) `
    "DBAAbilitySetDataAsset still hardcodes element/zodiac table row count 5."
Assert-True (-not ($abilitySet -match "RowNames\.Num\(\)\s*!=\s*25")) `
    "DBAAbilitySetDataAsset still hardcodes derived element table row count 25."
Assert-True (-not ($abilitySet -match "RowNames\.Num\(\)\s*!=\s*12")) `
    "DBAAbilitySetDataAsset still hardcodes zodiac table row count 12."
Assert-True (-not ($staticData -match "GetRowNames\(\)\.Num\(\)\s*!=\s*5")) `
    "DBAStaticDataAsset still hardcodes element/five-camp table row count 5."

Write-Host "PASS: Data table count constants contract"
