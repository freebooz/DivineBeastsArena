<#
Validates FixedSkillGroup DataAsset exposes data-driven runtime GAS config
for active abilities: cost, cooldown GE/tag/duration and Chinese validation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySetLibrary.h"
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySetLibrary.cpp"

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
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

Assert-True ($header -match "struct\s+DIVINEBEASTSARENA_API\s+FDBAAbilityRuntimeConfig") `
  "Expected FixedSkillGroup runtime config struct for data-driven GAS ability config."
Assert-True ($header -match "TSubclassOf<UGameplayEffect>\s+CostGameplayEffectClass") `
  "Expected runtime config to expose CostGameplayEffectClass."
Assert-True ($header -match "TSubclassOf<UGameplayEffect>\s+CooldownGameplayEffectClass") `
  "Expected runtime config to expose CooldownGameplayEffectClass."
Assert-True ($header -match "FGameplayTag\s+CooldownTag") `
  "Expected runtime config to expose CooldownTag."
Assert-True ($header -match "float\s+CooldownDuration") `
  "Expected runtime config to expose CooldownDuration."
Assert-True ($header -match "float\s+EnergyCost") `
  "Expected runtime config to expose EnergyCost."
Assert-True ($header -match "FText\s+DisplayName") `
  "Expected runtime config to expose DisplayName for UI text data."
Assert-True ($header -match "TSoftObjectPtr<UTexture2D>\s+Icon") `
  "Expected runtime config to expose Icon for UI resource data."
Assert-True ($header -match "bool\s+Validate\s*\(\s*FStringView\s+SlotName\s*,\s*TArray<FString>&\s+OutErrors\s*\)\s+const") `
  "Expected runtime config to provide a C++ validation function."

Assert-True ($header -match "FDBAAbilityRuntimeConfig\s+Skill01RuntimeConfig") `
  "Expected Skill01 runtime config on FixedSkillGroup DataAsset."
Assert-True ($header -match "FDBAAbilityRuntimeConfig\s+Skill02RuntimeConfig") `
  "Expected Skill02 runtime config on FixedSkillGroup DataAsset."
Assert-True ($header -match "FDBAAbilityRuntimeConfig\s+Skill03RuntimeConfig") `
  "Expected Skill03 runtime config on FixedSkillGroup DataAsset."
Assert-True ($header -match "FDBAAbilityRuntimeConfig\s+Skill04RuntimeConfig") `
  "Expected Skill04 runtime config on FixedSkillGroup DataAsset."
Assert-True ($header -match "bool\s+ValidateRuntimeAbilityConfigs\s*\(\s*TArray<FString>&\s+OutErrors\s*\)\s+const") `
  "Expected FixedSkillGroup DataAsset to validate all runtime ability configs."

Assert-True ($cpp -match "bool\s+FDBAAbilityRuntimeConfig::Validate\s*\(") `
  "Expected runtime config validation implementation."
Assert-True ($cpp -match "bool\s+UDBAFixedSkillGroupDataAsset::ValidateRuntimeAbilityConfigs\s*\(") `
  "Expected FixedSkillGroup runtime config validation implementation."
Assert-True ($cpp -match "Skill01RuntimeConfig\.Validate") `
  "Expected validation to include Skill01RuntimeConfig."
Assert-True ($cpp -match "Skill02RuntimeConfig\.Validate") `
  "Expected validation to include Skill02RuntimeConfig."
Assert-True ($cpp -match "Skill03RuntimeConfig\.Validate") `
  "Expected validation to include Skill03RuntimeConfig."
Assert-True ($cpp -match "Skill04RuntimeConfig\.Validate") `
  "Expected validation to include Skill04RuntimeConfig."
Assert-True ($cpp -match "OutErrors\.Add\(FString::Printf\(TEXT\(") `
  "Expected validation failures to use Chinese FString diagnostics."
Assert-True ($cpp -match "ValidateRuntimeAbilityConfigs\(ValidationErrors\)") `
  "Expected grant path to validate FixedSkillGroup runtime ability configs."
Assert-True ($cpp -match "UE_LOG\(LogDBAData,\s*Warning,\s*TEXT\(") `
  "Expected validation failures to be reported through Chinese data logs."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 22266, 23450, 25216, 33021, 32452, 36816, 34892, 37197, 32622, 22865, 32422))
Write-Host $successMessage -ForegroundColor Green
