<#
Validates built-in playable skill fallback display names are localized Chinese
FText values, so missing DataAssets do not leak English placeholders into HUD.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$componentCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAPlayableSkillComponent.cpp"

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

$componentCpp = Get-Content -LiteralPath $componentCppPath -Encoding UTF8 -Raw

Assert-True ($componentCpp -match "FText\s+DisplayName") `
  "Expected MakeSkill to accept an FText display name."
Assert-True ($componentCpp -notmatch "FText::FromString\s*\(\s*DisplayName\s*\)") `
  "Expected built-in display names not to be created from raw strings."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultSkill01DisplayName"') `
  "Expected default skill 1 display name to use localized FText."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultSkill02DisplayName"') `
  "Expected default skill 2 display name to use localized FText."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultSkill03DisplayName"') `
  "Expected default skill 3 display name to use localized FText."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultSkill04DisplayName"') `
  "Expected default skill 4 display name to use localized FText."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultUltimateDisplayName"') `
  "Expected default ultimate display name to use localized FText."
Assert-True ($componentCpp -match 'NSLOCTEXT\(\s*"DBAPlayableSkill",\s*"DefaultSkill06DisplayName"') `
  "Expected default skill 6 display name to use localized FText."

foreach ($englishName in @(
  'Mage Fireball',
  'Frost Shard',
  'Bloom Healing',
  'Chain Lightning',
  'Priest Shield',
  'Shadow Bolt'
)) {
  Assert-True (-not $componentCpp.Contains(('TEXT("' + $englishName + '")'))) `
    ("Expected built-in display name not to use English placeholder: " + $englishName)
}

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 21487, 29992, 25216, 33021, 20869, 32622, 23637, 31034, 21517, 24050, 20351, 29992, 20013, 25991, 26412, 22320, 21270))
Write-Host $successMessage -ForegroundColor Green
