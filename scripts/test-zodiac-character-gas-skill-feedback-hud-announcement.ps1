<#
Validates successful GAS skill cue feedback reaches the local Arena HUD combat
announcement bridge with Chinese display text sourced from runtime skill config.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$characterHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
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

$characterHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterHeaderPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath
$releaseText = [string]::Concat([char[]](123, 48, 125, 32, 24050, 37322, 25918))

Assert-True ($characterHeader -match "HandleArenaHUDSkillCueExecuted\s*\(\s*FName\s+SkillId\s*,\s*AActor\*\s+Target\s*\)") "Expected Zodiac character to declare HandleArenaHUDSkillCueExecuted(FName SkillId, AActor* Target)."
Assert-True ($characterHeader -match "ResolveArenaHUDSkillCueDisplayName\s*\(\s*FName\s+SkillId\s*\)\s+const") "Expected Zodiac character to declare runtime skill cue display name resolver."
Assert-True ($characterHeader -match "ArenaHUDSkillCueAnnouncementDuration") "Expected configurable ArenaHUDSkillCueAnnouncementDuration."

Assert-True ($characterCpp -match "BindArenaHUDAttributeDelegates[\s\S]*OnSkillCueExecuted\.RemoveDynamic\(\s*this\s*,\s*&ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted\s*\)") "Expected BindArenaHUDAttributeDelegates to remove stale skill cue bindings first."
Assert-True ($characterCpp -match "BindArenaHUDAttributeDelegates[\s\S]*OnSkillCueExecuted\.AddDynamic\(\s*this\s*,\s*&ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted\s*\)") "Expected BindArenaHUDAttributeDelegates to bind skill cue feedback."
Assert-True ($characterCpp -match "UnbindArenaHUDAttributeDelegates[\s\S]*OnSkillCueExecuted\.RemoveDynamic\(\s*this\s*,\s*&ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted\s*\)") "Expected UnbindArenaHUDAttributeDelegates to remove skill cue feedback binding."

Assert-True ($characterCpp -match "void\s+ADBAZodiacCharacterBase::HandleArenaHUDSkillCueExecuted\s*\(\s*FName\s+SkillId\s*,\s*AActor\*\s+Target\s*\)") "Expected HandleArenaHUDSkillCueExecuted implementation."
Assert-True ($characterCpp -match "ResolveArenaHUDSkillCueDisplayName[\s\S]*FindAbilityRuntimeConfigByInputID") "Expected skill cue display names to prefer FixedSkillGroup runtime config from ASC."
Assert-True ($characterCpp -match "ResolveArenaHUDSkillCueDisplayName[\s\S]*RuntimeConfig->DisplayName") "Expected skill cue display names to use DataAsset DisplayName."
Assert-True ($characterCpp -match "ResolveArenaHUDSkillCueDisplayName[\s\S]*GetPlayableSkillSpecs\(\)") "Expected skill cue display names to fall back through playable skill specs before localized fallback text."
Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*FText::Format") "Expected skill cue HUD feedback to build localized announcement text."
Assert-True ($characterCpp.Contains(('NSLOCTEXT("DBAArenaHUD", "SkillCueAnnouncement", "' + $releaseText + '")'))) "Expected skill cue HUD feedback to use Chinese localized release text."
Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*ResolveArenaHUDSkillCueDisplayName\s*\(\s*SkillId\s*\)") "Expected skill cue HUD feedback to use resolved display name."
Assert-True ($characterCpp -notmatch "HandleArenaHUDSkillCueExecuted[\s\S]*FText::FromString\(\s*SkillId\.ToString\(\)\s*\)") "Expected skill cue HUD feedback not to expose internal SkillId."
Assert-True ($characterCpp -notmatch "\{0\} Activated") "Expected skill cue HUD feedback not to use English placeholder text."
Assert-True ($characterCpp -notmatch "HandleArenaHUDSkillCueExecuted[\s\S]*UDBAGameUIManager::Get\(") "Expected skill cue HUD feedback to use the existing GameInstance subsystem access pattern, not a nonexistent static Get helper."
Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*GetGameInstance\(\)[\s\S]*GetSubsystem<UDBAGameUIManager>\(\)") "Expected skill cue HUD feedback to resolve UDBAGameUIManager through GetGameInstance()->GetSubsystem<UDBAGameUIManager>()."
Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*ShowArenaHUDCombatAnnouncement\(") "Expected skill cue HUD feedback to use the Arena HUD combat announcement bridge."
Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*ArenaHUDSkillCueAnnouncementDuration") "Expected skill cue HUD feedback to use its configured announcement duration."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 71, 65, 83, 25216, 33021, 21453, 39304, 24050, 20351, 29992, 20013, 25991, 23637, 31034, 25991, 26412))
Write-Host $successMessage -ForegroundColor Green
