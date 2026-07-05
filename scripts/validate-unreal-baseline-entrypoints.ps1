<#
Validates baseline Unreal ownership for log channels, data asset base classes,
native gameplay tag registration entrypoints, and character build summaries.
Run from the repository root:
  .\scripts\validate-unreal-baseline-entrypoints.ps1
#>

[CmdletBinding()]
param(
  [string]$ClientSourceRoot = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$clientSourceRoot = if ([string]::IsNullOrWhiteSpace($ClientSourceRoot)) {
  Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source"
}
else {
  (Resolve-Path -LiteralPath $ClientSourceRoot).ProviderPath
}
$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
  param([Parameter(Mandatory = $true)][string]$Message)
  $failures.Add($Message)
}

function Get-FileContent {
  param([Parameter(Mandatory = $true)][string]$Path)

  if (-not (Test-Path -LiteralPath $Path)) {
    Add-Failure "Missing required baseline file: $Path"
    return ""
  }

  return Get-Content -LiteralPath $Path -Encoding UTF8 -Raw
}

function Test-LogBaseline {
  $logHeader = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Public\GameCore\Core\DBALogChannels.h"
  $logCpp = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Private\GameCore\Core\DBALogChannels.cpp"
  $headerContent = Get-FileContent -Path $logHeader
  $cppContent = Get-FileContent -Path $logCpp

  if ([string]::IsNullOrWhiteSpace($headerContent) -or [string]::IsNullOrWhiteSpace($cppContent)) {
    return
  }

  $requiredCategories = @(
    "LogDBACore",
    "LogDBAFrontend",
    "LogDBAMatch",
    "LogDBACombat",
    "LogDBAUI",
    "LogDBAData",
    "LogDBANetwork",
    "LogDBAValidation",
    "LogDBAAI",
    "LogDBATelemetry",
    "LogDBAGameOps"
  )

  foreach ($category in $requiredCategories) {
    if ($headerContent -notmatch [regex]::Escape("DECLARE_LOG_CATEGORY_EXTERN($category")) {
      Add-Failure "GameCore log baseline header is missing category declaration: $category"
    }

    if ($cppContent -notmatch [regex]::Escape("DEFINE_LOG_CATEGORY($category)")) {
      Add-Failure "GameCore log baseline cpp is missing category definition: $category"
    }
  }

  $externalDeclarations = & rg -n --pcre2 -g "*.h" -g "*.cpp" -e 'DECLARE_LOG_CATEGORY_EXTERN\(' -- $clientSourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg log declaration scan failed with code $LASTEXITCODE"
  }
  if ($externalDeclarations) {
    $unexpected = $externalDeclarations | Where-Object { $_ -notmatch 'GameCore\\Public\\GameCore\\Core\\DBALogChannels\.h:\d+:' }
    if ($unexpected) {
      Add-Failure "Only GameCore/Public/GameCore/Core/DBALogChannels.h should declare shared log categories:`n$($unexpected -join "`n")"
    }
  }

  $externalDefinitions = & rg -n --pcre2 -g "*.cpp" -e 'DEFINE_LOG_CATEGORY\(' -- $clientSourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg log definition scan failed with code $LASTEXITCODE"
  }
  if ($externalDefinitions) {
    $unexpected = $externalDefinitions | Where-Object { $_ -notmatch 'GameCore\\Private\\GameCore\\Core\\DBALogChannels\.cpp:\d+:' }
    if ($unexpected) {
      Add-Failure "Only GameCore/Private/GameCore/Core/DBALogChannels.cpp should define shared global log categories:`n$($unexpected -join "`n")"
    }
  }
}

function Test-DataAssetBaseline {
  $dataAssetBaseHeader = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Public\GameCore\Data\DBADataAssetBase.h"
  $headerContent = Get-FileContent -Path $dataAssetBaseHeader

  if ([string]::IsNullOrWhiteSpace($headerContent)) {
    return
  }

  if ($headerContent -notmatch 'class\s+GAMECORE_API\s+UDBADataAssetBase\s*:\s*public\s+UPrimaryDataAsset') {
    Add-Failure "UDBADataAssetBase must remain the GameCore-owned UPrimaryDataAsset baseline."
  }

  $otherBaseDeclarations = & rg -n --pcre2 -g "*.h" -e 'class\s+\w+_API\s+U\w*DataAssetBase\s*:\s*public' -- $clientSourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg DataAssetBase scan failed with code $LASTEXITCODE"
  }
  if ($otherBaseDeclarations) {
    $unexpected = $otherBaseDeclarations | Where-Object { $_ -notmatch 'GameCore\\Public\\GameCore\\Data\\DBADataAssetBase\.h:\d+:' }
    if ($unexpected) {
      Add-Failure "Only GameCore should define the shared *DataAssetBase baseline class:`n$($unexpected -join "`n")"
    }
  }
}

function Test-GameplayTagBaseline {
  $tagsHeader = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Public\GameDBA\Core\DBAGameplayTags.h"
  $tagsCpp = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Private\GameDBA\Core\DBAGameplayTags.cpp"
  $moduleCpp = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Private\DivineBeastsArena.cpp"

  $tagsHeaderContent = Get-FileContent -Path $tagsHeader
  $tagsCppContent = Get-FileContent -Path $tagsCpp
  $moduleCppContent = Get-FileContent -Path $moduleCpp

  if ([string]::IsNullOrWhiteSpace($tagsHeaderContent) -or [string]::IsNullOrWhiteSpace($tagsCppContent) -or [string]::IsNullOrWhiteSpace($moduleCppContent)) {
    return
  }

  if ($tagsHeaderContent -notmatch 'struct\s+DIVINEBEASTSARENA_API\s+FDBAGameplayTags') {
    Add-Failure "FDBAGameplayTags must remain declared in DivineBeastsArena/Public/GameDBA/Core/DBAGameplayTags.h."
  }

  if ($tagsCppContent -notmatch 'AddNativeGameplayTag') {
    Add-Failure "DivineBeastsArena gameplay tag baseline must own native tag registration via AddNativeGameplayTag."
  }

  if ($moduleCppContent -notmatch 'FDBAGameplayTags::InitializeNativeTags\(\)') {
    Add-Failure "DivineBeastsArena module startup must initialize native gameplay tags."
  }

  $nativeTagRegistrations = & rg -n --pcre2 -g "*.cpp" -g "*.h" -e 'AddNativeGameplayTag' -- $clientSourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg native tag registration scan failed with code $LASTEXITCODE"
  }
  if ($nativeTagRegistrations) {
    $unexpected = $nativeTagRegistrations | Where-Object { $_ -notmatch 'DivineBeastsArena\\Private\\GameDBA\\Core\\DBAGameplayTags\.cpp:\d+:' }
    if ($unexpected) {
      Add-Failure "Only DivineBeastsArena gameplay tag registry should call AddNativeGameplayTag:`n$($unexpected -join "`n")"
    }
  }
}

function Test-CharacterBuildBaseline {
  $buildHeader = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Public\GameCore\Character\DBACharacterBuildTypes.h"
  $buildCpp = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Private\GameCore\Character\DBACharacterBuildTypes.cpp"
  $travelHeader = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Public\GameCore\Session\DBATravelTypes.h"
  $frontendSessionHeader = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Public\GameCore\Session\DBAFrontendSessionSubsystem.h"
  $frontendSessionCpp = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\Private\GameCore\Session\DBAFrontendSessionSubsystem.cpp"
  $headerContent = Get-FileContent -Path $buildHeader
  $cppContent = Get-FileContent -Path $buildCpp
  $travelContent = Get-FileContent -Path $travelHeader
  $frontendSessionHeaderContent = Get-FileContent -Path $frontendSessionHeader
  $frontendSessionCppContent = Get-FileContent -Path $frontendSessionCpp

  if ([string]::IsNullOrWhiteSpace($headerContent) -or [string]::IsNullOrWhiteSpace($cppContent) -or [string]::IsNullOrWhiteSpace($travelContent) -or [string]::IsNullOrWhiteSpace($frontendSessionHeaderContent) -or [string]::IsNullOrWhiteSpace($frontendSessionCppContent)) {
    return
  }

  if ($headerContent -notmatch 'struct\s+GAMECORE_API\s+FDBACharacterBuildSummary') {
    Add-Failure "FDBACharacterBuildSummary must remain a GameCore-owned build summary contract."
  }

  $requiredSymbols = @(
    "MakeFixedSkillGroupId",
    "ResolveFiveCamp",
    "MakeBuildSummary"
  )

  foreach ($symbol in $requiredSymbols) {
    if ($headerContent -notmatch [regex]::Escape($symbol)) {
      Add-Failure "GameCore character build header is missing symbol declaration: $symbol"
    }

    if ($cppContent -notmatch [regex]::Escape($symbol)) {
      Add-Failure "GameCore character build implementation is missing symbol definition: $symbol"
    }
  }

  $requiredTravelSymbols = @(
    "FixedSkillGroupId",
    "GetCharacterBuildSummary",
    "HasValidCharacterBuildSummary"
  )

  foreach ($symbol in $requiredTravelSymbols) {
    if ($travelContent -notmatch [regex]::Escape($symbol)) {
      Add-Failure "GameCore travel context is missing character build validation symbol: $symbol"
    }
  }

  if ($frontendSessionHeaderContent -notmatch 'bool\s+TrySetCurrentTravelContext\s*\(') {
    Add-Failure "FrontendSession must expose TrySetCurrentTravelContext for validated travel admission."
  }

  if ($frontendSessionCppContent -notmatch 'Context\.HasValidCharacterBuildSummary\(\)') {
    Add-Failure "FrontendSession TrySetCurrentTravelContext must validate the frozen character build summary."
  }

  if ($frontendSessionCppContent -notmatch 'SetState\(EDBAFrontendSessionState::Loading\)') {
    Add-Failure "FrontendSession must enter Loading only after accepting a valid travel context."
  }

  $duplicateFixedSkillGroupHelpers = & rg -n --pcre2 -g "*.cpp" -g "*.h" -e 'FName\s+MakeFixedSkillGroupId\s*\(' -- $clientSourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg FixedSkillGroup helper scan failed with code $LASTEXITCODE"
  }
  if ($duplicateFixedSkillGroupHelpers) {
    $unexpected = $duplicateFixedSkillGroupHelpers | Where-Object { $_ -notmatch 'GameCore\\(Public|Private)\\GameCore\\Character\\DBACharacterBuildTypes\.(h|cpp):\d+:' }
    if ($unexpected) {
      Add-Failure "Only GameCore/Character/DBACharacterBuildTypes should define MakeFixedSkillGroupId:`n$($unexpected -join "`n")"
    }
  }
}

function Test-ArenaGameFixedSkillGroupBaseline {
  $fixedSkillGroupHeader = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Public\GameDBA\Data\DBAFixedSkillGroupData.h"
  $zodiacHeroDataAssetHeader = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Public\GameDBA\Data\DBAZodiacHeroDataAsset.h"
  $zodiacHeroDataAssetCpp = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Private\GameDBA\Data\DBAZodiacHeroDataAsset.cpp"
  $skillGroupGeneratorCpp = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Private\GameDBA\Services\DBASkillGroupGeneratorSubsystem.cpp"
  $fixedSkillGroupTests = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\Private\Tests\DBAFixedSkillGroupDataTests.cpp"

  $fixedSkillGroupHeaderContent = Get-FileContent -Path $fixedSkillGroupHeader
  $zodiacHeroDataAssetHeaderContent = Get-FileContent -Path $zodiacHeroDataAssetHeader
  $zodiacHeroDataAssetCppContent = Get-FileContent -Path $zodiacHeroDataAssetCpp
  $skillGroupGeneratorCppContent = Get-FileContent -Path $skillGroupGeneratorCpp
  $fixedSkillGroupTestsContent = Get-FileContent -Path $fixedSkillGroupTests

  if ([string]::IsNullOrWhiteSpace($fixedSkillGroupHeaderContent) -or [string]::IsNullOrWhiteSpace($zodiacHeroDataAssetHeaderContent) -or [string]::IsNullOrWhiteSpace($zodiacHeroDataAssetCppContent) -or [string]::IsNullOrWhiteSpace($skillGroupGeneratorCppContent) -or [string]::IsNullOrWhiteSpace($fixedSkillGroupTestsContent)) {
    return
  }

  if ($fixedSkillGroupHeaderContent -notmatch 'HasValidIdentity') {
    Add-Failure "ArenaGame fixed skill group row must expose HasValidIdentity."
  }

  if ($fixedSkillGroupHeaderContent -notmatch 'DBACharacterBuild::MakeFixedSkillGroupId') {
    Add-Failure "ArenaGame fixed skill group row identity must reuse GameCore DBACharacterBuild::MakeFixedSkillGroupId."
  }

  if ($zodiacHeroDataAssetHeaderContent -notmatch 'BuildFixedSkillGroupRowName') {
    Add-Failure "ZodiacHeroDataAsset must expose canonical BuildFixedSkillGroupRowName."
  }

  if ($zodiacHeroDataAssetCppContent -notmatch 'BuildFixedSkillGroupRowName\(Zodiac, Element\)') {
    Add-Failure "ZodiacHeroDataAsset data-table lookup must use the canonical fixed skill group row name."
  }

  if ($zodiacHeroDataAssetCppContent -match 'Zodiac_%s_Element_%s') {
    Add-Failure "ZodiacHeroDataAsset must not use legacy Zodiac_*_Element_* fixed skill group row names."
  }

  if ($skillGroupGeneratorCppContent -notmatch 'DBACharacterBuild::MakeFixedSkillGroupId') {
    Add-Failure "SkillGroupGeneratorSubsystem must reuse GameCore DBACharacterBuild::MakeFixedSkillGroupId for fallback row names."
  }

  if ($skillGroupGeneratorCppContent -notmatch 'HasValidSkillGroupIdentity') {
    Add-Failure "SkillGroupGeneratorSubsystem must reject missing Zodiac / Element before generating fallback skill groups."
  }

  if ($skillGroupGeneratorCppContent -notmatch 'FoundRow->HasValidIdentity\(\)') {
    Add-Failure "SkillGroupGeneratorSubsystem must reject fixed skill group data-table rows with invalid identity fields."
  }

  $requiredTestSymbols = @(
    "UsesCanonicalBuildSummaryRowName",
    "ValidatesRowIdentity",
    "GeneratorRejectsInvalidIdentityDimensions",
    "GeneratorFallbackUsesCanonicalIdentity",
    "AssetRows",
    "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    "Rat_Water",
    "Snake_Gold",
    "Tiger_Fire"
  )

  foreach ($symbol in $requiredTestSymbols) {
    if ($fixedSkillGroupTestsContent -notmatch [regex]::Escape($symbol)) {
      Add-Failure "Fixed skill group data tests are missing coverage token: $symbol"
    }
  }
}

Set-Location $repoRoot

Test-LogBaseline
Test-DataAssetBaseline
Test-GameplayTagBaseline
Test-CharacterBuildBaseline
Test-ArenaGameFixedSkillGroupBaseline

if ($failures.Count -gt 0) {
  foreach ($failure in $failures) {
    Write-Host "FAIL: $failure" -ForegroundColor Red
  }
  throw "Unreal baseline entrypoint validation failed: $($failures -join '; ')"
}

Write-Host "PASS: Unreal baseline entrypoints" -ForegroundColor Green
