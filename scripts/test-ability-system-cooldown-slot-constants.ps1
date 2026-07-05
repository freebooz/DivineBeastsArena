<#
Validates AbilitySystem cooldown arrays share the same named slot-count constants
as Character/HUD instead of re-deriving local counts.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$ascCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$ascCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascCppPath

Assert-True ($ascCpp -match "GetSkillCooldowns[\s\S]*constexpr\s+int32\s+CooldownSlotCount\s*=\s*DBAConstants::ArenaCombatSkillSlotCount") "Expected GetSkillCooldowns to use DBAConstants::ArenaCombatSkillSlotCount."
Assert-True ($ascCpp -match "GetSkillCooldowns[\s\S]*OutCooldowns\.Init\(0\.0f,\s*CooldownSlotCount\)") "Expected GetSkillCooldowns to initialize cooldowns with CooldownSlotCount."
Assert-True ($ascCpp -match "GetSkillCooldowns[\s\S]*SlotIndex\s*=\s*DBAConstants::ActiveSkillCount") "Expected Ultimate cooldown slot index to use DBAConstants::ActiveSkillCount."
Assert-True ($ascCpp -match "NormalizeSkillCooldowns[\s\S]*constexpr\s+int32\s+CooldownSlotCount\s*=\s*DBAConstants::ArenaCombatSkillSlotCount") "Expected NormalizeSkillCooldowns to use DBAConstants::ArenaCombatSkillSlotCount."
Assert-True ($ascCpp -match "NormalizeSkillCooldowns[\s\S]*OutNormalized\.Init\(0\.0f,\s*CooldownSlotCount\)") "Expected NormalizeSkillCooldowns to initialize normalized cooldowns with CooldownSlotCount."
Assert-True ($ascCpp -match "NormalizeSkillCooldowns[\s\S]*Index\s*<\s*CooldownSlotCount\s*&&\s*Index\s*<\s*InCooldowns\.Num\(\)") "Expected NormalizeSkillCooldowns copy loop to use CooldownSlotCount."

Assert-True ($ascCpp -notmatch "DBAConstants::ActiveSkillCount\s*\+\s*1") "Expected AbilitySystem cooldown code to avoid re-deriving the arena combat slot count."
Assert-True ($ascCpp -notmatch "GetSkillCooldowns[\s\S]{0,120}ExpectedSlots\s*=\s*5") "Expected GetSkillCooldowns to avoid hard-coded ExpectedSlots = 5."
Assert-True ($ascCpp -notmatch "NormalizeSkillCooldowns[\s\S]{0,120}ExpectedSlots\s*=\s*5") "Expected NormalizeSkillCooldowns to avoid hard-coded ExpectedSlots = 5."
Assert-True ($ascCpp -notmatch "GetSkillCooldowns[\s\S]{0,520}SlotIndex\s*=\s*4") "Expected GetSkillCooldowns to avoid hard-coded Ultimate cooldown index 4."

Write-Host "PASS: AbilitySystem cooldown slot constants contract" -ForegroundColor Green
