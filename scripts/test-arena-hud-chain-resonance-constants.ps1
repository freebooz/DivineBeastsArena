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

$controllerCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$characterCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$abilitySystemCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

$controllerCpp = Get-Content -Raw -Encoding UTF8 $controllerCppPath
$rootCpp = Get-Content -Raw -Encoding UTF8 $rootCppPath
$characterCpp = Get-Content -Raw -Encoding UTF8 $characterCppPath
$abilitySystemCpp = Get-Content -Raw -Encoding UTF8 $abilitySystemCppPath

Assert-True ($controllerCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Arena HUD controller to include DBAConstants."
Assert-True ($controllerCpp -match "UpdateChainLevel[\s\S]*FMath::Clamp\(InChainLevel,\s*0,\s*DBAConstants::MaxChainLevel\)") "Expected Arena HUD controller ChainLevel clamp to use DBAConstants::MaxChainLevel."
Assert-True ($controllerCpp -match "UpdateResonanceLevel[\s\S]*FMath::Clamp\(InResonanceLevel,\s*0,\s*DBAConstants::MaxResonanceLevel\)") "Expected Arena HUD controller ResonanceLevel clamp to use DBAConstants::MaxResonanceLevel."
Assert-True ($controllerCpp -notmatch "UpdateChainLevel[\s\S]{0,180}FMath::Clamp\(InChainLevel,\s*0,\s*10\)") "Expected Arena HUD controller ChainLevel clamp to avoid hard-coded 10."
Assert-True ($controllerCpp -notmatch "UpdateResonanceLevel[\s\S]{0,180}FMath::Clamp\(InResonanceLevel,\s*0,\s*4\)") "Expected Arena HUD controller ResonanceLevel clamp to avoid hard-coded 4."

Assert-True ($rootCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Arena HUD root to include DBAConstants."
Assert-True ($rootCpp -match "HandleControllerChainLevelUpdated[\s\S]*ChainLevel\s*>=\s*DBAConstants::MaxChainLevel") "Expected Arena HUD root ChainReady check to use DBAConstants::MaxChainLevel."
Assert-True ($rootCpp -notmatch "HandleControllerChainLevelUpdated[\s\S]{0,220}ChainLevel\s*>=\s*10") "Expected Arena HUD root ChainReady check to avoid hard-coded 10."

Assert-True ($characterCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Zodiac character implementation to include DBAConstants."
Assert-True ($characterCpp -match "AddChainLevel[\s\S]*ASC->AddChainLevel\(Delta\)") "Expected Zodiac character AddChainLevel to delegate to ASC."
Assert-True ($characterCpp -match "GetChainLevel[\s\S]*ASC->GetChainLevel\(\)") "Expected Zodiac character GetChainLevel to read ASC before fallback state."
Assert-True ($characterCpp.Contains("const int32 CurrentChainLevel = GetChainLevel();")) "Expected Zodiac character HUD sync to read ChainLevel through getter."
Assert-True ($characterCpp.Contains("const bool bChainReady = CurrentChainLevel >= DBAConstants::MaxChainLevel;")) "Expected Zodiac character HUD chain readiness to use DBAConstants::MaxChainLevel."
Assert-True ($characterCpp -notmatch "AddChainLevel[\s\S]{0,240}ChainLevel\s*=") "Expected Zodiac character AddChainLevel to avoid direct ChainLevel writes."
Assert-True ($characterCpp -notmatch "AddChainLevel[\s\S]{0,180}FMath::Clamp\(ChainLevel\s*\+\s*Delta,\s*0,\s*10\)") "Expected Zodiac character fallback ChainLevel clamp to avoid hard-coded 10."

Assert-True ($abilitySystemCpp -match "AddChainLevel[\s\S]*FMath::Clamp\(ChainLevel\s*\+\s*Amount,\s*0,\s*DBAConstants::MaxChainLevel\)") "Expected ASC ChainLevel clamp to use DBAConstants::MaxChainLevel."
Assert-True ($abilitySystemCpp -match "SetResonanceLevel[\s\S]*FMath::Clamp\(Level,\s*0,\s*DBAConstants::MaxResonanceLevel\)") "Expected ASC ResonanceLevel clamp to use DBAConstants::MaxResonanceLevel."
Assert-True ($abilitySystemCpp -notmatch "AddChainLevel[\s\S]{0,220}FMath::Clamp\(ChainLevel\s*\+\s*Amount,\s*0,\s*10\)") "Expected ASC ChainLevel clamp to avoid hard-coded 10."
Assert-True ($abilitySystemCpp -notmatch "SetResonanceLevel[\s\S]{0,220}FMath::Clamp\(Level,\s*0,\s*4\)") "Expected ASC ResonanceLevel clamp to avoid hard-coded 4."

Write-Host "PASS: Arena HUD Chain/Resonance constants contract" -ForegroundColor Green
