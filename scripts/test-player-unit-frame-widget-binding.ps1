<#
Validates PlayerUnitFrame widget binding to its controller broadcasts.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.cpp"

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

Assert-True ($header -match "HandleControllerHPUpdated\s*\(") "Expected PlayerUnitFrame widget to declare HP controller handler."
Assert-True ($header -match "HandleControllerEnergyUpdated\s*\(") "Expected PlayerUnitFrame widget to declare energy controller handler."
Assert-True ($header -match "HandleControllerLevelUpdated\s*\(") "Expected PlayerUnitFrame widget to declare level controller handler."

Assert-True ($cpp.Contains('#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"')) "Expected widget implementation to include controller header."
Assert-True ($cpp -match "CachedCurrentXP\(0\.0f\)") "Expected PlayerUnitFrame constructor to initialize current XP before NativeConstruct replay."
Assert-True ($cpp -match "CachedMaxXP\(100\.0f\)") "Expected PlayerUnitFrame constructor to initialize max XP before NativeConstruct replay."
Assert-True ($cpp -match "NativeConstruct[\s\S]*UpdateHP\(CachedCurrentHP,\s*CachedMaxHP\)[\s\S]*UpdateEnergy\(CachedCurrentEnergy,\s*CachedMaxEnergy\)[\s\S]*UpdateXP\(CachedCurrentXP,\s*CachedMaxXP\)[\s\S]*UpdateUltimateEnergyWithMax\(CachedUltimateEnergy,\s*CachedMaxUltimateEnergy\)[\s\S]*UpdateLevel\(CurrentLevel\)") "Expected NativeConstruct to replay cached PlayerUnitFrame state after widget tree binding."
Assert-True ($cpp -match "OnHPUpdated\.RemoveDynamic") "Expected SetWidgetController to unbind previous HP delegate."
Assert-True ($cpp -match "OnEnergyUpdated\.RemoveDynamic") "Expected SetWidgetController to unbind previous energy delegate."
Assert-True ($cpp -match "OnLevelUpdated\.RemoveDynamic") "Expected SetWidgetController to unbind previous level delegate."
Assert-True ($cpp -match "OnHPUpdated\.AddDynamic") "Expected SetWidgetController to bind HP delegate."
Assert-True ($cpp -match "OnEnergyUpdated\.AddDynamic") "Expected SetWidgetController to bind energy delegate."
Assert-True ($cpp -match "OnLevelUpdated\.AddDynamic") "Expected SetWidgetController to bind level delegate."
Assert-True ($cpp -match "GetCurrentHP\(\)") "Expected SetWidgetController to read initial HP from controller."
Assert-True ($cpp -match "GetCurrentEnergy\(\)") "Expected SetWidgetController to read initial energy from controller."
Assert-True ($cpp -match "GetCurrentLevel\(\)") "Expected SetWidgetController to read initial level from controller."
Assert-True ($cpp -match "void\s+UDBAPlayerUnitFrameWidgetBase::HandleControllerHPUpdated") "Expected HP handler implementation."
Assert-True ($cpp -match "UpdateHP\(CurrentHP,\s*MaxHP\)") "Expected HP handler to call UpdateHP."
Assert-True ($cpp -match "UpdateEnergy\(CurrentEnergy,\s*MaxEnergy\)") "Expected energy handler to call UpdateEnergy."
Assert-True ($cpp -match "UpdateLevel\(Level\)") "Expected level handler to call UpdateLevel."
Assert-True ($cpp -match "UpdateHP[\s\S]*if\s*\(\s*HealthBar\s*\)[\s\S]*HealthBar->SetPercent\(Percentage\)") "Expected UpdateHP to drive the native HealthBar percent."
Assert-True ($cpp -match "UpdateHP[\s\S]*CachedCurrentHP\s*=\s*FMath::Max\(0\.0f,\s*InCachedCurrentHP\)") "Expected UpdateHP to normalize cached current HP before Blueprint/native updates."
Assert-True ($cpp -match "UpdateHP[\s\S]*CachedMaxHP\s*=\s*FMath::Max\(0\.0f,\s*InCachedMaxHP\)") "Expected UpdateHP to normalize cached max HP before percentage calculation."
Assert-True ($cpp -match "UpdateHP[\s\S]*Percentage\s*=\s*FMath::Clamp\([^;]+,\s*0\.0f,\s*1\.0f\)") "Expected UpdateHP percentage to be clamped between 0 and 1."
Assert-True ($cpp -match "BP_OnUpdateHP\(CachedCurrentHP,\s*CachedMaxHP,\s*Percentage\)") "Expected UpdateHP to broadcast normalized HP values."
Assert-True ($cpp -match "UpdateEnergy[\s\S]*CachedCurrentEnergy\s*=\s*FMath::Max\(0\.0f,\s*InCachedCurrentEnergy\)") "Expected UpdateEnergy to normalize cached current energy before Blueprint/native updates."
Assert-True ($cpp -match "UpdateEnergy[\s\S]*CachedMaxEnergy\s*=\s*FMath::Max\(0\.0f,\s*InCachedMaxEnergy\)") "Expected UpdateEnergy to normalize cached max energy before percentage calculation."
Assert-True ($cpp -match "UpdateEnergy[\s\S]*Percentage\s*=\s*FMath::Clamp\([^;]+,\s*0\.0f,\s*1\.0f\)") "Expected UpdateEnergy percentage to be clamped between 0 and 1."
Assert-True ($cpp -match "BP_OnUpdateEnergy\(CachedCurrentEnergy,\s*CachedMaxEnergy,\s*Percentage\)") "Expected UpdateEnergy to broadcast normalized energy values."
Assert-True ($cpp -match "UpdateXP[\s\S]*CachedCurrentXP\s*=\s*FMath::Max\(0\.0f,\s*InCachedCurrentXP\)") "Expected UpdateXP to normalize cached current XP before Blueprint/native updates."
Assert-True ($cpp -match "UpdateXP[\s\S]*CachedMaxXP\s*=\s*FMath::Max\(0\.0f,\s*InCachedMaxXP\)") "Expected UpdateXP to normalize cached max XP before percentage calculation."
Assert-True ($cpp -match "UpdateXP[\s\S]*Percentage\s*=\s*FMath::Clamp\([^;]+,\s*0\.0f,\s*1\.0f\)") "Expected UpdateXP percentage to be clamped between 0 and 1."
Assert-True ($cpp -match "BP_OnUpdateXP\(CachedCurrentXP,\s*CachedMaxXP,\s*Percentage\)") "Expected UpdateXP to broadcast normalized XP values."
Assert-True ($cpp -match "UpdateLevel[\s\S]*CurrentLevel\s*=\s*FMath::Max\(1,\s*Level\)") "Expected UpdateLevel to normalize level to at least 1 before Blueprint updates."
Assert-True ($cpp -match "BP_OnUpdateLevel\(CurrentLevel\)") "Expected UpdateLevel to broadcast normalized level."
Assert-True ($cpp.Contains('#include "GameCore/Types/DBACommonEnums.h"')) "Expected PlayerUnitFrame implementation to include FiveCamp enum definitions."
Assert-True ($cpp -match "ApplyFiveCampTheme[\s\S]*static_cast<uint8>\(EDBAFiveCamp::None\)[\s\S]*static_cast<uint8>\(EDBAFiveCamp::Center\)") "Expected ApplyFiveCampTheme to derive valid FiveCamp bounds from EDBAFiveCamp."
Assert-True ($cpp -match "ApplyFiveCampTheme[\s\S]*NormalizedFiveCamp\s*=\s*FMath::Clamp\(\s*FiveCamp,[\s\S]*static_cast<uint8>\(EDBAFiveCamp::None\),[\s\S]*static_cast<uint8>\(EDBAFiveCamp::Center\)\)") "Expected ApplyFiveCampTheme to clamp direct FiveCamp input to the valid presentation-camp range."
Assert-True ($cpp -match "BP_OnApplyFiveCampTheme\(NormalizedFiveCamp\)") "Expected ApplyFiveCampTheme to broadcast normalized FiveCamp value."

Write-Host "PASS: PlayerUnitFrame widget binding contract" -ForegroundColor Green
