<#
Runs fixture-based tests for scripts/validate-unreal-source-guardrails.ps1.
Run from the repository root:
  .\scripts\test-unreal-source-guardrails.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$validator = Join-Path -Path $repoRoot -ChildPath "scripts\validate-unreal-source-guardrails.ps1"
$fixtureRoot = Join-Path -Path $repoRoot -ChildPath (".tmp\unreal-source-guardrail-fixtures\{0}" -f [Guid]::NewGuid().ToString("N"))

function New-Fixture {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string]$BuildContent,
    [string]$GameModeContent = "",
    [string]$RuntimeServiceContent = "",
    [string]$SessionServiceContent = "",
    [string]$SessionTypesContent = "",
    [string]$SessionTestsContent = "",
    [string]$UrlOptionsContent = "",
    [string]$UrlOptionsTestsContent = ""
  )

  $root = Join-Path -Path $fixtureRoot -ChildPath $Name
  $sourceRoot = Join-Path -Path $root -ChildPath "DBA_GameClient\Source"
  $arenaRoot = Join-Path -Path $sourceRoot -ChildPath "DivineBeastsArena"
  $pluginRoot = Join-Path -Path $root -ChildPath "DBA_GameClient\Plugins"
  $docsRoot = Join-Path -Path $root -ChildPath "docs\Development"
  $gameModeRoot = Join-Path -Path $arenaRoot -ChildPath "Private\GameDBA\Framework"
  $runtimeServiceRoot = Join-Path -Path $pluginRoot -ChildPath "GameBackendClient\Source\GameBackendClient\Private"
  $backendPublicRoot = Join-Path -Path $pluginRoot -ChildPath "GameBackendClient\Source\GameBackendClient\Public"
  $backendTestsRoot = Join-Path -Path $runtimeServiceRoot -ChildPath "Tests"
  $arenaTestsRoot = Join-Path -Path $arenaRoot -ChildPath "Private\Tests"
  $fixtureAgentsPolicy = @'
C++ Gameplay GAS Blueprint DataAsset VFX SFX UPROPERTY UFUNCTION
PolicyId: `DBA.DataAsset.NoHardcoding`
PrimaryDataAsset DataAsset DataTable DeveloperSettings GameplayTag Asset Manager UI VFX SFX C++
PolicyId: `DBA.UI.EventAsync`
UI Tick Delegate ViewModel FieldNotify MVVM OnRep GameplayCue GameThread
PolicyId: `DBA.Log.ChineseOutput`
UE_LOG ensureMsgf checkf TEXT(" Automation Test MCP CI GameplayTag
'@
  $fixtureControlPromptPolicy = @'
### 2.1.1 C++ Gameplay GAS Blueprint DataAsset UPROPERTY UFUNCTION Subsystem
PolicyId: `DBA.DataAsset.NoHardcoding`
PrimaryDataAsset DataAsset DataTable DeveloperSettings GameplayTag Asset Manager UI VFX SFX C++
PolicyId: `DBA.UI.EventAsync`
UI Tick Delegate ViewModel FieldNotify MVVM OnRep GameplayCue GameThread
PolicyId: `DBA.Log.ChineseOutput`
UE_LOG ensureMsgf checkf TEXT(" Automation Test MCP CI GameplayTag
'@

  New-Item -ItemType Directory -Force -Path $arenaRoot, $pluginRoot, $docsRoot, $gameModeRoot, $runtimeServiceRoot, $backendPublicRoot, $backendTestsRoot, $arenaTestsRoot | Out-Null
  Set-Content -LiteralPath (Join-Path -Path $arenaRoot -ChildPath "DivineBeastsArena.Build.cs") -Value $BuildContent -Encoding UTF8
  Set-Content -LiteralPath (Join-Path -Path $root -ChildPath "AGENTS.md") -Value $fixtureAgentsPolicy -Encoding UTF8
  Set-Content -LiteralPath (Join-Path -Path $docsRoot -ChildPath "ZodiacArena_UE5_8_Codex_fixture.md") -Value $fixtureControlPromptPolicy -Encoding UTF8
  if (-not [string]::IsNullOrWhiteSpace($GameModeContent)) {
    Set-Content -LiteralPath (Join-Path -Path $gameModeRoot -ChildPath "DBAGameModeBase.cpp") -Value $GameModeContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($RuntimeServiceContent)) {
    Set-Content -LiteralPath (Join-Path -Path $runtimeServiceRoot -ChildPath "GameBackendRuntimeService.cpp") -Value $RuntimeServiceContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($SessionServiceContent)) {
    Set-Content -LiteralPath (Join-Path -Path $runtimeServiceRoot -ChildPath "GameBackendSessionService.cpp") -Value $SessionServiceContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($SessionTypesContent)) {
    Set-Content -LiteralPath (Join-Path -Path $backendPublicRoot -ChildPath "GameBackendTypes.h") -Value $SessionTypesContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($SessionTestsContent)) {
    Set-Content -LiteralPath (Join-Path -Path $backendTestsRoot -ChildPath "GameBackendSessionServiceTests.cpp") -Value $SessionTestsContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($UrlOptionsContent)) {
    Set-Content -LiteralPath (Join-Path -Path $gameModeRoot -ChildPath "DBAUrlOptions.cpp") -Value $UrlOptionsContent -Encoding UTF8
  }
  if (-not [string]::IsNullOrWhiteSpace($UrlOptionsTestsContent)) {
    Set-Content -LiteralPath (Join-Path -Path $arenaTestsRoot -ChildPath "DBAUrlOptionsTests.cpp") -Value $UrlOptionsTestsContent -Encoding UTF8
  }
  return $root
}

function Invoke-ExpectFailure {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][scriptblock]$Action,
    [Parameter(Mandatory = $true)][string]$ExpectedMessage
  )

  try {
    & $Action
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

function Invoke-ExpectSuccess {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][scriptblock]$Action
  )

  & $Action
  Write-Host "PASS: $Name" -ForegroundColor Green
}

$unguardedBuild = @'
using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
  public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "Core",
      "RenderCore",
      "RHI",
      "AudioMixer",
      "MediaAssets",
    });
  }
}
'@

$guardedBuild = @'
using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
  public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivateDependencyModuleNames.AddRange(new string[]
    {
      "Core",
    });

    if (Target.Type != TargetType.Server)
    {
      PrivateDependencyModuleNames.AddRange(new string[]
      {
        "RenderCore",
        "RHI",
        "AudioMixer",
        "MediaAssets",
      });
    }
  }
}
'@

$completeGameModeRuntimeContract = @'
void ADBAGameModeBase::ReportBackendPlayerJoined()
{
  FDBACharacterBuildSummary AdmissionBuildSummary;
  DBAUrlOptions::TryExtractCharacterBuildSummary(Options, AdmissionBuildSummary);
  BuildSummary.Zodiac = DBACharacterBuild::ToStableZodiacName(AdmissionBuildSummary.Zodiac);
  BuildSummary.PrimaryElement = DBACharacterBuild::ToStableElementName(AdmissionBuildSummary.PrimaryElement);
  BuildSummary.FiveCamp = ToStableFiveCampName(AdmissionBuildSummary.FiveCamp);
  BuildSummary.FixedSkillGroupId = AdmissionBuildSummary.FixedSkillGroupId.ToString();
}

void ADBAGameModeBase::HandleMatchHasStarted()
{
  Super::HandleMatchHasStarted();
  ReportBackendMatchStarted();
}

void ADBAGameModeBase::ReportBackendMatchStarted()
{
  RuntimeService->NotifyMatchStarted(EmptyCallback);
}

void ADBAGameModeBase::HandleMatchHasEnded()
{
  Super::HandleMatchHasEnded();
  RuntimeService->NotifyMatchEnded(EmptyCallback);
  ReportBackendMatchResults();
}

void ADBAGameModeBase::ReportBackendMatchResults()
{
  FDBA_GameBackendRuntimePlayerResult PlayerResult;
  BackendRuntimePlayerIds.FindRef(PlayerController);
  DBAPlayerState->BuildRuntimePlayerResult(PlayerId);
  const FString BackendMatchResultIdempotencyKey = FString::Printf(TEXT("ue-match-result-%s"), *SessionId);
  const FString ResultJson = BuildBackendMatchResultsJson(PlayerResults);
  RuntimeService->NotifyMatchResults(BackendMatchResultIdempotencyKey, ResultJson, PlayerResults, EmptyCallback);
}
'@

$missingGameModeLifecycleContract = @'
void ADBAGameModeBase::ReportBackendPlayerJoined()
{
  FDBACharacterBuildSummary AdmissionBuildSummary;
  DBAUrlOptions::TryExtractCharacterBuildSummary(Options, AdmissionBuildSummary);
  BuildSummary.Zodiac = DBACharacterBuild::ToStableZodiacName(AdmissionBuildSummary.Zodiac);
  BuildSummary.PrimaryElement = DBACharacterBuild::ToStableElementName(AdmissionBuildSummary.PrimaryElement);
  BuildSummary.FiveCamp = ToStableFiveCampName(AdmissionBuildSummary.FiveCamp);
  BuildSummary.FixedSkillGroupId = AdmissionBuildSummary.FixedSkillGroupId.ToString();
}
'@

$completeRuntimeServiceContract = @'
void UDBA_GameBackendRuntimeService::NotifyPlayerJoined()
{
  Json->SetStringField(TEXT("zodiac"), BuildSummary.Zodiac);
  Json->SetStringField(TEXT("primaryElement"), BuildSummary.PrimaryElement);
  Json->SetStringField(TEXT("fiveCamp"), BuildSummary.FiveCamp);
  Json->SetStringField(TEXT("fixedSkillGroupId"), BuildSummary.FixedSkillGroupId);
}
'@

$missingRuntimeServiceContract = @'
void UDBA_GameBackendRuntimeService::NotifyPlayerJoined()
{
  Json->SetStringField(TEXT("zodiac"), BuildSummary.Zodiac);
  Json->SetStringField(TEXT("primaryElement"), BuildSummary.PrimaryElement);
}
'@

$missingSessionServiceContract = @'
void UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData()
{
  TEXT("serverIp");
  TEXT("serverPort");
  TEXT("sessionToken");
}
'@

$missingSessionTeamContract = @'
void UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData()
{
  TEXT("characterBuildSummary");
  TEXT("DBAZodiac");
  TEXT("DBAElement");
  TEXT("DBAFiveCamp");
  TEXT("DBAFixedSkillGroupId");
  Connection.Zodiac;
  Connection.PrimaryElement;
  Connection.FiveCamp;
  Connection.FixedSkillGroupId;
  TryBuildTravelUrlFromConnectionData();
  TryGetObjectField(TEXT("data"));
  NestedBuildSummaryObj;
  TEXT("serverIp");
  TEXT("serverPort");
  TEXT("sessionToken");
}
'@

$completeSessionServiceContract = @'
void UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData()
{
  TEXT("characterBuildSummary");
  TEXT("DBAZodiac");
  TEXT("DBAElement");
  TEXT("DBAFiveCamp");
  TEXT("DBAFixedSkillGroupId");
  TEXT("DBATeamId");
  TEXT("teamId");
  Connection.Zodiac;
  Connection.PrimaryElement;
  Connection.FiveCamp;
  Connection.FixedSkillGroupId;
  Connection.TeamId;
  TryBuildTravelUrlFromConnectionData();
  TryGetObjectField(TEXT("data"));
  NestedBuildSummaryObj;
  TEXT("serverIp");
  TEXT("serverPort");
  TEXT("sessionToken");
}
'@

$completeSessionTypesContract = @'
struct FDBA_GameBackendSessionConnection
{
  FString Zodiac;
  FString PrimaryElement;
  FString FiveCamp;
  FString FixedSkillGroupId;
  int32 TeamId;
};
'@

$completeSessionTestsContract = @'
FDBA_GameBackendSessionTravelUrlBuildSummaryTest
FDBA_GameBackendSessionConnectionJsonBuildSummaryTest
FDBA_GameBackendSessionConnectionAliasJsonTest
FDBA_GameBackendSessionEnvelopeJsonTest
BuildTravelUrlIncludesFrozenBuildSummary
ConnectionJsonBuildsTravelUrlWithNestedBuildSummary
ConnectionJsonAcceptsNestedServerAliases
ConnectionJsonAcceptsResponseEnvelopeData
DBAZodiac=Rat
DBAElement=Water
DBAFiveCamp=East
DBAFixedSkillGroupId=Rat_Water
DBAZodiac=Tiger
DBAElement=Fire
DBAFiveCamp=South
DBAFixedSkillGroupId=Tiger_Fire
PlayerSessionToken=alias-token
DBAFixedSkillGroupId=Dragon_Wood
PlayerSessionToken=envelope-token
DBAZodiac=Snake
DBAElement=Gold
DBAFixedSkillGroupId=Snake_Gold
DBATeamId=1
DBATeamId=2
'@

$completeUrlOptionsContract = @'
void DBAUrlOptions::TryExtractCharacterBuildSummary()
{
  TryExtractCharacterBuildSummary();
  TryExtractTeamId();
  DBACharacterBuild::MakeFixedSkillGroupId();
}
'@

$missingUrlOptionsContract = @'
void DBAUrlOptions::TryExtractCharacterBuildSummary()
{
  TryExtractCharacterBuildSummary();
}
'@

$completeUrlOptionsTestsContract = @'
TryExtractCharacterBuildSummary
TryExtractTeamId
ValidatesDedicatedServerBuildSummary
Rat_Fire
DBATeamId=1
TeamId=2
MissingTeamId
NonPositiveTeamId
'@

$unguardedRoot = New-Fixture `
  -Name "unguarded-client-deps" `
  -BuildContent $unguardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $completeSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$guardedRoot = New-Fixture `
  -Name "guarded-client-deps" `
  -BuildContent $guardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $completeSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$missingRuntimeContractRoot = New-Fixture `
  -Name "missing-runtime-player-build-summary" `
  -BuildContent $guardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $missingRuntimeServiceContract `
  -SessionServiceContent $completeSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$missingSessionTravelContractRoot = New-Fixture `
  -Name "missing-session-travel-build-summary" `
  -BuildContent $guardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $missingSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$missingGameModeLifecycleContractRoot = New-Fixture `
  -Name "missing-runtime-match-lifecycle-handoff" `
  -BuildContent $guardedBuild `
  -GameModeContent $missingGameModeLifecycleContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $completeSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$missingSessionTeamContractRoot = New-Fixture `
  -Name "missing-session-travel-team-id" `
  -BuildContent $guardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $missingSessionTeamContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $completeUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract
$missingUrlAdmissionContractRoot = New-Fixture `
  -Name "missing-url-build-summary-admission" `
  -BuildContent $guardedBuild `
  -GameModeContent $completeGameModeRuntimeContract `
  -RuntimeServiceContent $completeRuntimeServiceContract `
  -SessionServiceContent $completeSessionServiceContract `
  -SessionTypesContent $completeSessionTypesContract `
  -SessionTestsContent $completeSessionTestsContract `
  -UrlOptionsContent $missingUrlOptionsContract `
  -UrlOptionsTestsContent $completeUrlOptionsTestsContract

Invoke-ExpectFailure `
  -Name "unguarded client-only dependencies" `
  -ExpectedMessage "client-only Unreal modules must be guarded" `
  -Action { & $validator -RepoRoot $unguardedRoot }

Invoke-ExpectFailure `
  -Name "missing runtime player build summary contract" `
  -ExpectedMessage "Runtime player-joined build summary contract is incomplete" `
  -Action { & $validator -RepoRoot $missingRuntimeContractRoot }

Invoke-ExpectFailure `
  -Name "missing session travel build summary contract" `
  -ExpectedMessage "Session travel build summary contract is incomplete" `
  -Action { & $validator -RepoRoot $missingSessionTravelContractRoot }

Invoke-ExpectFailure `
  -Name "missing Runtime match lifecycle handoff" `
  -ExpectedMessage "Runtime match lifecycle handoff contract is incomplete" `
  -Action { & $validator -RepoRoot $missingGameModeLifecycleContractRoot }

Invoke-ExpectFailure `
  -Name "missing session travel TeamId handoff" `
  -ExpectedMessage "Session travel build summary contract is incomplete" `
  -Action { & $validator -RepoRoot $missingSessionTeamContractRoot }

Invoke-ExpectFailure `
  -Name "missing Dedicated Server URL build summary admission" `
  -ExpectedMessage "Dedicated Server URL build summary admission implementation is incomplete" `
  -Action { & $validator -RepoRoot $missingUrlAdmissionContractRoot }

Invoke-ExpectSuccess `
  -Name "guarded client-only dependencies" `
  -Action { & $validator -RepoRoot $guardedRoot }
