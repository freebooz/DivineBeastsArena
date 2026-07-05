<#
Validates that Zodiac character cooldown mirrors are updated by ASC event
sync entrypoints instead of per-frame Character Tick countdown logic.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

$cpp = Get-Content -LiteralPath $cppPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Get-FunctionBody {
  param(
    [Parameter(Mandatory = $true)][string]$StartPattern,
    [Parameter(Mandatory = $true)][string]$EndPattern,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $match = [regex]::Match($cpp, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $cpp.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $cpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

$tickBody = Get-FunctionBody `
  'void\s+ADBAZodiacCharacterBase::Tick\s*\(\s*float\s+DeltaSeconds\s*\)\s*\{' `
  "`nvoid\s+ADBAZodiacCharacterBase::ApplyLobbyVisuals" `
  "Tick"

$updateCooldownsBody = Get-FunctionBody `
  'void\s+ADBAZodiacCharacterBase::UpdateSkillCooldowns\s*\(\s*const\s+TArray<float>&\s+NewCooldowns\s*\)\s*\{' `
  "`nvoid\s+ADBAZodiacCharacterBase::OnRep_SkillCooldowns" `
  "UpdateSkillCooldowns"

$onRepCooldownsBody = Get-FunctionBody `
  'void\s+ADBAZodiacCharacterBase::OnRep_SkillCooldowns\s*\(\s*\)\s*\{' `
  "`nvoid\s+ADBAZodiacCharacterBase::GetSpectatorData" `
  "OnRep_SkillCooldowns"

Assert-True ($tickBody.Contains("Super::Tick(DeltaSeconds);")) `
  "Expected Tick to keep the parent Tick call."
Assert-True (-not ($tickBody -match 'for\s*\(\s*float&\s+Cooldown\s*:\s*SkillCooldowns\s*\)')) `
  "Expected Character Tick not to iterate SkillCooldowns every frame."
Assert-True (-not ($tickBody -match 'SkillCooldowns\[[^\]]+\]\s*=\s*FMath::Max\s*\(\s*0\.0f\s*,\s*SkillCooldowns\[[^\]]+\]\s*-\s*DeltaSeconds\s*\)')) `
  "Expected Character Tick not to decrement SkillCooldowns by DeltaSeconds."
Assert-True (-not ($tickBody -match 'Cooldown\s*=\s*FMath::Max\s*\(\s*0\.0f\s*,\s*Cooldown\s*-\s*DeltaSeconds\s*\)')) `
  "Expected Character Tick not to decrement SkillCooldowns through a local Cooldown reference."
Assert-True (-not ($tickBody.Contains("SkillMaxCooldowns.SetNumZeroed"))) `
  "Expected Character Tick not to maintain SkillMaxCooldowns fallback capacity."
Assert-True (-not ($tickBody.Contains("ResolvePlayableSkillSpec(this, SkillSlot"))) `
  "Expected Character Tick not to derive default cooldowns from skill config."

Assert-True ($updateCooldownsBody.Contains("if (!HasAuthority())")) `
  "Expected UpdateSkillCooldowns to stay server-authoritative."
Assert-True ($updateCooldownsBody.Contains("SkillCooldowns = NewCooldowns;")) `
  "Expected UpdateSkillCooldowns to accept the ASC cooldown mirror."
Assert-True ($updateCooldownsBody.Contains("OnSkillCooldownsChanged.Broadcast(SkillCooldowns);")) `
  "Expected UpdateSkillCooldowns to broadcast cooldown mirror changes."
Assert-True ($onRepCooldownsBody.Contains("OnSkillCooldownsChanged.Broadcast(SkillCooldowns);")) `
  "Expected OnRep_SkillCooldowns to broadcast replicated cooldown mirror changes."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 29983, 32918, 35282, 33394, 20919, 21364, 20107, 20214, 38236, 20687, 22865, 32422))
Write-Host $successMessage -ForegroundColor Green
