[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$characterHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function New-CodePointText {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function Assert-Contains {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$Token,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Content.Contains($Token)) {
        throw $Message
    }
}

function Assert-NotContains {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$Token,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if ($Content.Contains($Token)) {
        throw $Message
    }
}

$characterHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterHeaderPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath
$combinedContent = $characterHeader + "`n" + $characterCpp
$missingProjectileClassText = New-CodePointText @(0x6295, 0x5C04, 0x7269, 0x6280, 0x80FD, 0x7F3A, 0x5C11, 0x6570, 0x636E, 0x8D44, 0x4EA7, 0x914D, 0x7F6E, 0x7C7B)

Assert-Contains -Content $characterCpp -Token "ResolvePlayableSkillSpec(Character, SkillSlot, PlayableSpec)" -Message "ResolveEquippedLobbySkillId must read playable skill specs first."
Assert-Contains -Content $characterCpp -Token "ProjectileClass = Spec.ProjectileClass" -Message "Projectile skill class must come from skill spec."
Assert-Contains -Content $characterCpp -Token "BloomClass = Spec.BloomHealingClass" -Message "Bloom healing class must come from skill spec."
Assert-Contains -Content $characterCpp -Token "ChainClass = Spec.ChainLightningClass" -Message "Chain lightning class must come from skill spec."
Assert-Contains -Content $characterCpp -Token "ShieldClass = Spec.HolyShieldClass" -Message "Holy shield class must come from skill spec."
Assert-Contains -Content $characterCpp -Token $missingProjectileClassText -Message "Missing projectile class must produce Chinese diagnostics and stop."

$forbiddenTokens = @(
    "FLobbyEquippedSkillCastSpec",
    "GetDefaultLobbySkillSpec",
    "SetSoftNiagaraAsset",
    "SetSoftSoundAsset",
    "ResolveOptionalGameplayCueTag",
    "LobbyFireballProjectileClass",
    "LobbyFrostShardProjectileClass",
    "LobbyBloomHealingSpellClass",
    "LobbyChainLightningSpellClass",
    "LobbyHolyShieldSpellClass",
    "LobbyShadowBoltProjectileClass",
    "LobbyFireballSpeed",
    "LobbyFireballRadius",
    "LobbyFireballDamage",
    "LobbyFireballCooldown",
    "ADBAFireballProjectile::StaticClass()",
    "ADBAFrostShardProjectile::StaticClass()",
    "ADBAShadowBoltProjectile::StaticClass()",
    "Lobby.Skill01",
    "Lobby.Skill02",
    "Lobby.Skill03",
    "Lobby.Skill04",
    "Lobby.Skill05",
    "Lobby.Skill06",
    "/Game/DBA/VFX",
    "/Game/ProjectileHitVFX",
    "/Game/DBA/Audio/SFX",
    "SFX_MageFireball",
    "SFX_FrostShard",
    "SFX_BloomHealing",
    "SFX_ChainLightning",
    "SFX_PriestShield",
    "SFX_ShadowBolt"
)

foreach ($token in $forbiddenTokens) {
    Assert-NotContains -Content $combinedContent -Token $token -Message "Zodiac character lobby skill boundary forbids hardcoded token: $token"
}

$successMessage = New-CodePointText @(0x901A, 0x8FC7, 0xFF1A, 0x751F, 0x8096, 0x89D2, 0x8272, 0x5927, 0x5385, 0x88C5, 0x914D, 0x6280, 0x80FD, 0x5DF2, 0x9650, 0x5236, 0x4E3A, 0x6570, 0x636E, 0x8D44, 0x4EA7, 0x9A71, 0x52A8)
Write-Host $successMessage -ForegroundColor Green
