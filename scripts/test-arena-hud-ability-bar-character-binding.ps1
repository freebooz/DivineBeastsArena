<#
Validates Arena HUD binds the local character into AbilityBar through UIManager and root widget handoff.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$abilityBarCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$abilityBarCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilityBarCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath

Assert-True ($rootHeader -match "class\s+ADBAZodiacCharacterBase;") "Expected Arena HUD root to forward declare Zodiac character."
Assert-True ($rootHeader -match "BindArenaHUDToCharacter\s*\(\s*ADBAZodiacCharacterBase\*\s+InCharacter\s*\)") "Expected Arena HUD root character binding entrypoint."
Assert-True ($rootHeader -match "TWeakObjectPtr<\s*ADBAZodiacCharacterBase\s*>\s+BoundArenaHUDCharacter") "Expected Arena HUD root to retain a weak bound character."
Assert-True ($rootCpp.Contains('#include "GameDBA/UI/Arena/UDBAAbilityBarWidgetBase.h"')) "Expected root implementation to include AbilityBar header."
Assert-True ($rootCpp -match "void\s+UDBAArenaHUDRootWidgetBase::BindArenaHUDToCharacter") "Expected root character binding implementation."
Assert-True ($rootCpp -match "BoundArenaHUDCharacter\s*=\s*InCharacter") "Expected root to cache the bound character."
Assert-True ($rootCpp -match "if\s*\(\s*AbilityBar\s*\)") "Expected root to guard optional AbilityBar."
Assert-True ($rootCpp -match "AbilityBar->BindToCharacter\(InCharacter\)") "Expected root to forward character binding to AbilityBar."
Assert-True ($abilityBarCpp -match "NativeConstruct[\s\S]*CacheSkillSlotWidgets\(\)[\s\S]*if\s*\(\s*BoundCharacter\.IsValid\(\)\s*\)[\s\S]*RefreshSkillCatalog\(\)[\s\S]*RefreshCooldowns\(\)") "Expected AbilityBar NativeConstruct to replay cached character binding after slot widgets are cached."
Assert-True ($abilityBarCpp -match "BindToCharacter[\s\S]*if\s*\(\s*BoundCharacter\.Get\(\)\s*==\s*InCharacter\s*\)[\s\S]*RefreshSkillCatalog\(\)[\s\S]*RefreshCooldowns\(\)[\s\S]*return;") "Expected AbilityBar same-character binding to refresh catalog/cooldowns instead of returning stale."

Assert-True ($managerHeader -match "class\s+ADBAZodiacCharacterBase;") "Expected UI manager to forward declare Zodiac character."
Assert-True ($managerHeader -match "BindArenaHUDToCharacter\s*\(\s*ADBAZodiacCharacterBase\*\s+Character\s*\)") "Expected UI manager Arena HUD character binding entrypoint."
Assert-True ($managerHeader -match "TWeakObjectPtr<\s*ADBAZodiacCharacterBase\s*>\s+ArenaHUDCharacter") "Expected UI manager to cache a weak Arena HUD character."
Assert-True ($managerCpp.Contains('#include "GameDBA/Character/DBAZodiacCharacterBase.h"')) "Expected UI manager implementation to include Zodiac character header."
Assert-True ($managerCpp -match "void\s+UDBAGameUIManager::BindArenaHUDToCharacter") "Expected UI manager binding implementation."
Assert-True ($managerCpp -match "ArenaHUDCharacter\s*=\s*Character") "Expected UI manager to cache incoming character."
Assert-True ($managerCpp -match "ArenaHUDWidget->BindArenaHUDToCharacter\(Character\)") "Expected UI manager to forward to active Arena HUD widget."
Assert-True ($managerCpp -match "ArenaHUDWidget->BindArenaHUDToCharacter\(ArenaHUDCharacter\.Get\(\)\)") "Expected Arena HUD creation to replay cached character binding."

Assert-True ($characterCpp -match "UIManager->BindArenaHUDToCharacter\(this\)") "Expected local character HUD sync to bind itself into Arena HUD AbilityBar."

Write-Host "PASS: Arena HUD AbilityBar character binding contract" -ForegroundColor Green
