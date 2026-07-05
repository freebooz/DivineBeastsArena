<#
Runs fixture-based tests for scripts/validate-unreal-baseline-entrypoints.ps1.
Run from the repository root:
  .\scripts\test-unreal-baseline-entrypoints.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$validator = Join-Path -Path $repoRoot -ChildPath "scripts\validate-unreal-baseline-entrypoints.ps1"
$fixtureRoot = Join-Path -Path $repoRoot -ChildPath (".tmp\unreal-baseline-entrypoint-fixtures\{0}" -f [guid]::NewGuid().ToString("N"))

function Set-FixtureFile {
  param(
    [Parameter(Mandatory = $true)][string]$Root,
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string]$Content
  )

  $path = Join-Path -Path $Root -ChildPath $RelativePath
  New-Item -ItemType Directory -Force -Path (Split-Path -Parent $path) | Out-Null
  Set-Content -LiteralPath $path -Encoding UTF8 -Value $Content
}

function New-BaselineFixture {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [string]$LogHeaderOverride = "",
    [string]$ExtraLogHeaderRelativePath = "",
    [string]$ExtraLogHeaderContent = "",
    [string]$DataAssetHeaderOverride = "",
    [string]$TagsCppOverride = "",
    [string]$CharacterBuildHeaderOverride = "",
    [string]$FrontendSessionCppOverride = "",
    [string]$FixedSkillGroupHeaderOverride = "",
    [string]$SkillGroupGeneratorCppOverride = ""
  )

  $sourceRoot = Join-Path -Path $fixtureRoot -ChildPath "$Name\DBA_GameClient\Source"
  $logCategories = @(
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

  $logHeader = if ([string]::IsNullOrWhiteSpace($LogHeaderOverride)) {
    ($logCategories | ForEach-Object { "DECLARE_LOG_CATEGORY_EXTERN($_, Log, All);" }) -join "`n"
  }
  else {
    $LogHeaderOverride
  }
  $logCpp = ($logCategories | ForEach-Object { "DEFINE_LOG_CATEGORY($_);" }) -join "`n"

  $dataAssetHeader = if ([string]::IsNullOrWhiteSpace($DataAssetHeaderOverride)) {
    "class GAMECORE_API UDBADataAssetBase : public UPrimaryDataAsset {};"
  }
  else {
    $DataAssetHeaderOverride
  }

  $tagsCpp = if ([string]::IsNullOrWhiteSpace($TagsCppOverride)) {
    "void FDBAGameplayTags::InitializeNativeTags(){ AddNativeGameplayTag(); }"
  }
  else {
    $TagsCppOverride
  }

  $characterBuildHeader = if ([string]::IsNullOrWhiteSpace($CharacterBuildHeaderOverride)) {
    @'
struct GAMECORE_API FDBACharacterBuildSummary {};
namespace DBACharacterBuild
{
FName MakeFixedSkillGroupId();
FName ResolveFiveCamp();
FDBACharacterBuildSummary MakeBuildSummary();
}
'@
  }
  else {
    $CharacterBuildHeaderOverride
  }

  $frontendSessionCpp = if ([string]::IsNullOrWhiteSpace($FrontendSessionCppOverride)) {
    "bool UDBAFrontendSessionSubsystem::TrySetCurrentTravelContext(){ Context.HasValidCharacterBuildSummary(); SetState(EDBAFrontendSessionState::Loading); return true; }"
  }
  else {
    $FrontendSessionCppOverride
  }

  $fixedSkillGroupHeader = if ([string]::IsNullOrWhiteSpace($FixedSkillGroupHeaderOverride)) {
    "bool HasValidIdentity(){ return DBACharacterBuild::MakeFixedSkillGroupId() != NAME_None; }"
  }
  else {
    $FixedSkillGroupHeaderOverride
  }

  $skillGroupGeneratorCpp = if ([string]::IsNullOrWhiteSpace($SkillGroupGeneratorCppOverride)) {
    "void Generate(){ DBACharacterBuild::MakeFixedSkillGroupId(); HasValidSkillGroupIdentity(); FoundRow->HasValidIdentity(); }"
  }
  else {
    $SkillGroupGeneratorCppOverride
  }

  Set-FixtureFile $sourceRoot "GameCore\Public\GameCore\Core\DBALogChannels.h" $logHeader
  Set-FixtureFile $sourceRoot "GameCore\Private\GameCore\Core\DBALogChannels.cpp" $logCpp
  Set-FixtureFile $sourceRoot "GameCore\Public\GameCore\Data\DBADataAssetBase.h" $dataAssetHeader
  Set-FixtureFile $sourceRoot "GameCore\Public\GameCore\Character\DBACharacterBuildTypes.h" $characterBuildHeader
  Set-FixtureFile $sourceRoot "GameCore\Private\GameCore\Character\DBACharacterBuildTypes.cpp" "FName MakeFixedSkillGroupId(){} FName ResolveFiveCamp(){} FDBACharacterBuildSummary MakeBuildSummary(){}"
  Set-FixtureFile $sourceRoot "GameCore\Public\GameCore\Session\DBATravelTypes.h" "FName FixedSkillGroupId; FDBACharacterBuildSummary GetCharacterBuildSummary(); bool HasValidCharacterBuildSummary();"
  Set-FixtureFile $sourceRoot "GameCore\Public\GameCore\Session\DBAFrontendSessionSubsystem.h" "bool TrySetCurrentTravelContext();"
  Set-FixtureFile $sourceRoot "GameCore\Private\GameCore\Session\DBAFrontendSessionSubsystem.cpp" $frontendSessionCpp
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Public\GameDBA\Core\DBAGameplayTags.h" "struct DIVINEBEASTSARENA_API FDBAGameplayTags { static void InitializeNativeTags(); };"
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Private\GameDBA\Core\DBAGameplayTags.cpp" $tagsCpp
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Private\DivineBeastsArena.cpp" "void StartupModule(){ FDBAGameplayTags::InitializeNativeTags(); }"
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Public\GameDBA\Data\DBAFixedSkillGroupData.h" $fixedSkillGroupHeader
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Public\GameDBA\Data\DBAZodiacHeroDataAsset.h" "FName BuildFixedSkillGroupRowName();"
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Private\GameDBA\Data\DBAZodiacHeroDataAsset.cpp" "void Lookup(){ BuildFixedSkillGroupRowName(Zodiac, Element); }"
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Private\GameDBA\Services\DBASkillGroupGeneratorSubsystem.cpp" $skillGroupGeneratorCpp
  Set-FixtureFile $sourceRoot "DivineBeastsArena\Private\Tests\DBAFixedSkillGroupDataTests.cpp" "UsesCanonicalBuildSummaryRowName ValidatesRowIdentity GeneratorRejectsInvalidIdentityDimensions GeneratorFallbackUsesCanonicalIdentity AssetRows /Game/DBA/Data/Tables/DT_FixedSkillGroups Rat_Water Snake_Gold Tiger_Fire"

  if (-not [string]::IsNullOrWhiteSpace($ExtraLogHeaderRelativePath)) {
    Set-FixtureFile $sourceRoot $ExtraLogHeaderRelativePath $ExtraLogHeaderContent
  }

  return $sourceRoot
}

function Invoke-ExpectFailure {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string]$SourceRoot,
    [Parameter(Mandatory = $true)][string]$ExpectedMessage
  )

  try {
    & $validator -ClientSourceRoot $SourceRoot
  }
  catch {
    if ($_.Exception.Message -notlike "*$ExpectedMessage*") {
      throw "$Name failed for the wrong reason: $($_.Exception.Message)"
    }

    Write-Host "PASS: $Name failed as expected" -ForegroundColor Green
    return
  }

  throw "$Name should have failed."
}

if (Test-Path -LiteralPath $fixtureRoot) {
  Remove-Item -LiteralPath $fixtureRoot -Recurse -Force
}

$validRoot = New-BaselineFixture -Name "valid"
& $validator -ClientSourceRoot $validRoot
Write-Host "PASS: valid baseline entrypoint fixture" -ForegroundColor Green

$missingLogRoot = New-BaselineFixture -Name "missing-log-category" -LogHeaderOverride "DECLARE_LOG_CATEGORY_EXTERN(LogDBACore, Log, All);"
Invoke-ExpectFailure `
  -Name "missing log category fixture" `
  -SourceRoot $missingLogRoot `
  -ExpectedMessage "GameCore log baseline header is missing category declaration"

$duplicateLogRoot = New-BaselineFixture `
  -Name "duplicate-log-category" `
  -ExtraLogHeaderRelativePath "GameMoba\Public\GameMoba\Core\BadLog.h" `
  -ExtraLogHeaderContent "DECLARE_LOG_CATEGORY_EXTERN(LogDBAUI, Log, All);"
Invoke-ExpectFailure `
  -Name "duplicate log category fixture" `
  -SourceRoot $duplicateLogRoot `
  -ExpectedMessage "Only GameCore/Public/GameCore/Core/DBALogChannels.h should declare shared log categories"

$missingTravelValidationRoot = New-BaselineFixture `
  -Name "missing-travel-validation" `
  -FrontendSessionCppOverride "bool UDBAFrontendSessionSubsystem::TrySetCurrentTravelContext(){ SetState(EDBAFrontendSessionState::Loading); return true; }"
Invoke-ExpectFailure `
  -Name "missing travel validation fixture" `
  -SourceRoot $missingTravelValidationRoot `
  -ExpectedMessage "FrontendSession TrySetCurrentTravelContext must validate the frozen character build summary"

$legacyFixedSkillGroupRoot = New-BaselineFixture `
  -Name "legacy-fixed-skill-group-row-name" `
  -FixedSkillGroupHeaderOverride "bool HasValidIdentity(){ return true; }"
Invoke-ExpectFailure `
  -Name "fixed skill group identity fixture" `
  -SourceRoot $legacyFixedSkillGroupRoot `
  -ExpectedMessage "ArenaGame fixed skill group row identity must reuse GameCore DBACharacterBuild::MakeFixedSkillGroupId"

Write-Host "PASS: Unreal baseline entrypoint fixtures" -ForegroundColor Green
