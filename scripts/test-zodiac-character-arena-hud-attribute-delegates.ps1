<#
Validates Zodiac character Arena HUD sync uses GAS attribute change delegates with Tick fallback only before binding.
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

Assert-True ($header -match "EndPlay\s*\(\s*const\s+EEndPlayReason::Type\s+EndPlayReason\s*\)\s+override") "Expected EndPlay override for delegate cleanup."
Assert-True ($header -match "BindArenaHUDAttributeDelegates") "Expected Arena HUD attribute delegate binding helper."
Assert-True ($header -match "UnbindArenaHUDAttributeDelegates") "Expected Arena HUD attribute delegate cleanup helper."
Assert-True ($header -match "HandleArenaHUDAttributeChanged") "Expected Arena HUD attribute change handler."
Assert-True ($header -match "bHasBoundArenaHUDAttributeDelegates") "Expected delegate-bound state flag."
Assert-True ($header -match "ArenaHUDAttributeDelegateASC") "Expected weak ASC owner for delegate cleanup."
Assert-True ($header -match "ArenaHUDCurrentHealthChangedHandle") "Expected current health delegate handle."
Assert-True ($header -match "ArenaHUDMaxHealthChangedHandle") "Expected max health delegate handle."
Assert-True ($header -match "ArenaHUDCurrentEnergyChangedHandle") "Expected current energy delegate handle."
Assert-True ($header -match "ArenaHUDMaxEnergyChangedHandle") "Expected max energy delegate handle."
Assert-True ($header -match "ArenaHUDHeroLevelChangedHandle") "Expected hero level delegate handle."

Assert-True ($cpp -match "BindArenaHUDAttributeDelegates\(\);\s*\r?\n\s*SyncArenaHUDFromAttributes\(true\)") "Expected BeginPlay to bind delegates before forced initial HUD sync."
Assert-True ($cpp -match "void\s+ADBAZodiacCharacterBase::EndPlay") "Expected EndPlay implementation."
Assert-True ($cpp -match "UnbindArenaHUDAttributeDelegates\(\);\s*\r?\n\s*Super::EndPlay\(EndPlayReason\)") "Expected EndPlay to unbind before Super."
Assert-True ($cpp -match "BindArenaHUDAttributeDelegates\(\);\s*\r?\n\s*if\s*\(\s*!bHasBoundArenaHUDAttributeDelegates\s*\)") "Expected Tick fallback to run only when delegates are not bound."
Assert-True ($cpp -match "GetGameplayAttributeValueChangeDelegate\(UDBABattleAttributeSet::GetCurrentHealthAttribute\(\)\)") "Expected CurrentHealth delegate binding."
Assert-True ($cpp -match "GetGameplayAttributeValueChangeDelegate\(UDBABattleAttributeSet::GetMaxHealthAttribute\(\)\)") "Expected MaxHealth delegate binding."
Assert-True ($cpp -match "GetGameplayAttributeValueChangeDelegate\(UDBABattleAttributeSet::GetCurrentEnergyAttribute\(\)\)") "Expected CurrentEnergy delegate binding."
Assert-True ($cpp -match "GetGameplayAttributeValueChangeDelegate\(UDBABattleAttributeSet::GetMaxEnergyAttribute\(\)\)") "Expected MaxEnergy delegate binding."
Assert-True ($cpp -match "GetGameplayAttributeValueChangeDelegate\(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute\(\)\)") "Expected HeroLevel delegate binding."
Assert-True ($cpp -match "AddUObject\(this,\s*&ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged\)") "Expected delegates to call the HUD sync handler."
Assert-True ($cpp -match "Remove\(ArenaHUDCurrentHealthChangedHandle\)") "Expected CurrentHealth delegate cleanup."
Assert-True ($cpp -match "Remove\(ArenaHUDHeroLevelChangedHandle\)") "Expected HeroLevel delegate cleanup."
Assert-True ($cpp -match "void\s+ADBAZodiacCharacterBase::HandleArenaHUDAttributeChanged\s*\(\s*const\s+FOnAttributeChangeData&\s+ChangeData\s*\)") "Expected attribute change handler signature."
Assert-True ($cpp -match "HandleArenaHUDAttributeChanged[\s\S]*SyncArenaHUDFromAttributes\(\)") "Expected attribute changes to push through cached HUD sync."

Write-Host "PASS: Zodiac character Arena HUD attribute delegate contract" -ForegroundColor Green
