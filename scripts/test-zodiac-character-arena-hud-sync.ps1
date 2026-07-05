<#
Validates local Zodiac character synchronization into Arena HUD runtime entrypoints.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$header = Get-Content -Raw -Encoding UTF8 -LiteralPath $headerPath
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath

Assert-True ($header -match "GetHeroLevel\s*\(\s*\)\s*const") "Expected Zodiac character to expose HeroLevel getter."
Assert-True ($header -match "SyncArenaHUDFromAttributes\s*\(") "Expected Zodiac character to declare Arena HUD attribute sync."

Assert-True ($cpp.Contains('#include "GameDBA/GAS/Attributes/DBAHeroGrowthAttributeSet.h"')) "Expected character implementation to include HeroGrowth attribute set."
Assert-True ($cpp.Contains('#include "GameDBA/UI/DBAGameUIManager.h"')) "Expected character implementation to include UI manager."
Assert-True ($cpp -match "SyncArenaHUDFromAttributes\(\)") "Expected character lifecycle to call Arena HUD sync."
Assert-True ($cpp -match "if\s*\(\s*!IsLocallyControlled\(\)\s*\)") "Expected Arena HUD sync to skip non-local characters."
Assert-True ($cpp -match "GameInstance->GetSubsystem<\s*UDBAGameUIManager\s*>\(\)") "Expected Arena HUD sync to resolve UI manager from GameInstance."
Assert-True ($cpp -match "const float CurrentHP\s*=\s*GetCurrentHealth\(\)") "Expected Arena HUD sync to read current HP."
Assert-True ($cpp -match "const float MaxHP\s*=\s*GetMaxHealth\(\)") "Expected Arena HUD sync to read max HP."
Assert-True ($cpp -match "const float CurrentEnergy\s*=\s*GetCurrentEnergy\(\)") "Expected Arena HUD sync to read current Energy."
Assert-True ($cpp -match "const float MaxEnergy\s*=\s*GetMaxEnergy\(\)") "Expected Arena HUD sync to read max Energy."
Assert-True ($cpp -match "const int32 HeroLevel\s*=\s*GetHeroLevel\(\)") "Expected Arena HUD sync to read HeroLevel."
Assert-True ($cpp -match "UpdateArenaHUDPlayerVitals\(\s*CurrentHP,\s*MaxHP,\s*CurrentEnergy,\s*MaxEnergy\s*\)") "Expected Arena HUD sync to push HP and Energy values."
Assert-True ($cpp -match "UpdateArenaHUDPlayerLevel\(\s*HeroLevel\s*\)") "Expected Arena HUD sync to push HeroLevel."
Assert-True ($cpp -match "GetNumericAttributeBase\(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute\(\)\)") "Expected HeroLevel getter to read the GAS growth attribute."

Write-Host "PASS: Zodiac character Arena HUD sync contract" -ForegroundColor Green
