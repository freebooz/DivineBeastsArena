<#
验证 GAS 输入激活成功后会为 HUD/VFX 监听者暴露稳定反馈，并同步冷却状态。
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$ascHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$ascHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascHeaderPath
$ascCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascCppPath

$resolveCueHeaderPattern = 'ResolveSkillCueNameForInputID\s*\(\s*int32\s+InputID\s*\)\s*const'
$resolveCueCppPattern = 'FName\s+UDBAAbilitySystemComponent::ResolveSkillCueNameForInputID\s*\(\s*int32\s+InputID\s*\)\s*const'
$skill01CuePattern = 'ResolveSkillCueNameForInputID[\s\S]*EDBAAbilityInputID::Skill01[\s\S]*TEXT\("Skill01"\)'
$ultimateCuePattern = 'ResolveSkillCueNameForInputID[\s\S]*EDBAAbilityInputID::Ultimate[\s\S]*TEXT\("Ultimate"\)'
$inputCueResolvePattern = 'TryActivateAbilityByInputID[\s\S]*const\s+FName\s+SkillCueName\s*=\s*ResolveSkillCueNameForInputID\(InputID\)'
$cooldownSyncPattern = 'TryActivateAbilityByInputID[\s\S]*SyncCooldownsToCharacter\(\)'

Assert-True -Condition ($ascHeader -match $resolveCueHeaderPattern) -Message "ASC 必须声明 ResolveSkillCueNameForInputID(int32 InputID) const。"
Assert-True -Condition ($ascCpp -match $resolveCueCppPattern) -Message "ASC 必须实现 ResolveSkillCueNameForInputID。"
Assert-True -Condition ($ascCpp -match $skill01CuePattern) -Message "Skill01 输入必须有稳定反馈名称。"
Assert-True -Condition ($ascCpp -match $ultimateCuePattern) -Message "Ultimate 输入必须有稳定反馈名称。"
Assert-True -Condition ($ascCpp -match $inputCueResolvePattern) -Message "输入激活必须解析稳定 Cue 名称。"
$inputFeedbackBroadcastPattern = 'TryActivateAbilityByInputID[\s\S]*OnSkillCueExecuted\.Broadcast\(\s*SkillCueName\s*,\s*Target\s*\?\s*Target\s*:\s*GetDBAAvatarCharacter\(\)\s*\)'
Assert-True -Condition ($ascCpp -match $inputFeedbackBroadcastPattern) -Message "输入激活成功后必须广播 SkillCueName，并优先使用目标，否则使用 AvatarActor 角色上下文。"
Assert-True -Condition ($ascCpp -match $cooldownSyncPattern) -Message "输入激活成功后必须同步冷却状态。"

Write-Host "通过：AbilitySystem 输入激活反馈契约" -ForegroundColor Green
