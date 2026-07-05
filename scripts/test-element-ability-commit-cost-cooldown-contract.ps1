<#
Validates that normal element abilities submit GAS cost/cooldown through C++.
Blueprints may configure ability classes and data, but runtime commit logic must stay native.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

function Assert-True {
  param(
    [bool]$Condition,
    [string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Get-FileContent {
  param([string]$RelativePath)

  $path = Join-Path -Path $repoRoot -ChildPath $RelativePath
  if (-not (Test-Path -LiteralPath $path)) {
    throw "Required file is missing: $RelativePath"
  }

  return Get-Content -LiteralPath $path -Encoding UTF8 -Raw
}

$genericSkillCpp = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementSkillAbility_Generic.cpp"
$elementAbilityHeader = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAElementAbilityBase.h"
$elementAbilityCpp = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementAbilityBase.cpp"
$commitFailureText = -join ([char[]](0x63D0, 0x4EA4, 0x6D88, 0x8017, 0x6216, 0x51B7, 0x5374, 0x5931, 0x8D25))
$energyText = -join ([char[]](0x80FD, 0x91CF))

Assert-True ($genericSkillCpp -match "CommitAbility\s*\(\s*Handle\s*,\s*ActorInfo\s*,\s*ActivationInfo\s*\)") `
  "Generic element skill activation must call CommitAbility so GAS can submit cost and cooldown."

Assert-True ($genericSkillCpp -match "if\s*\(\s*!\s*CommitAbility\s*\(") `
  "Generic element skill activation must fail closed when CommitAbility fails."

Assert-True ($genericSkillCpp -match "EndAbility\s*\(\s*Handle\s*,\s*ActorInfo\s*,\s*ActivationInfo\s*,\s*true\s*,\s*true\s*\)") `
  "Generic element skill activation must cancel/end the ability when commit fails."

Assert-True ($genericSkillCpp.Contains($commitFailureText)) `
  "Generic element skill commit failure must emit a Chinese diagnostic log."

Assert-True ($elementAbilityCpp -match "Super::CommitAbilityCost\s*\(\s*Handle\s*,\s*ActorInfo\s*,\s*ActivationInfo\s*,\s*OptionalRelevantTags\s*\)") `
  "Element ability cost commit must preserve parent GAS cost behavior."

Assert-True ($elementAbilityHeader -match "CheckCost\s*\(") `
  "Element ability must override CheckCost because CommitAbility validates cost through CheckCost."

Assert-True ($elementAbilityHeader -match "ApplyCost\s*\(") `
  "Element ability must override ApplyCost because CommitAbility spends resources through ApplyCost."

Assert-True ($elementAbilityCpp -match "FMath::Max\s*\(\s*AbilityEnergyCost\s*,\s*EnergyCost\s*\)") `
  "Element ability cost commit must use the configured ability/data energy cost."

Assert-True ($elementAbilityCpp -match "bool\s+UDBAElementAbilityBase::CheckCost[\s\S]*GetCurrentEnergy\s*\(\s*\)[\s\S]*Cost") `
  "Element ability CheckCost must validate CurrentEnergy before CommitAbility succeeds."

Assert-True ($elementAbilityCpp -match "void\s+UDBAElementAbilityBase::ApplyCost[\s\S]*SetCurrentEnergy\s*\(") `
  "Element ability ApplyCost must deduct CurrentEnergy on the CommitAbility execution path."

Assert-True ($elementAbilityCpp -match "ActorInfo->IsNetAuthority\s*\(\s*\)") `
  "Element ability cost commit must mutate CurrentEnergy only on the network authority."

Assert-True ($elementAbilityCpp -match "SetCurrentEnergy\s*\(") `
  "Element ability cost commit must deduct CurrentEnergy from DBABattleAttributeSet."

Assert-True ($elementAbilityCpp -match "FMath::Clamp\s*\(") `
  "Element ability cost commit must clamp CurrentEnergy after deduction."

Assert-True ($elementAbilityCpp.Contains($energyText)) `
  "Element ability cost commit must use Chinese energy diagnostics."

Write-Host "PASS: Element ability commit cost/cooldown contract" -ForegroundColor Green
