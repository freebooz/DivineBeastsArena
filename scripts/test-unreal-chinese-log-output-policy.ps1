<#
Validates the global Chinese log and information output policy.
Run from the repository root:
  .\scripts\test-unreal-chinese-log-output-policy.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
if ($null -eq $controlPrompt) {
  throw "Required control prompt file is missing under docs\Development: ZodiacArena_UE5_8_Codex_*.md"
}

function New-CodePointText {
  param(
    [Parameter(Mandatory = $true)][int[]]$CodePoints
  )

  return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function Assert-FileContains {
  param(
    [Parameter(Mandatory = $true)][string]$Path,
    [Parameter(Mandatory = $true)][string[]]$RequiredTokens
  )

  $fullPath = if ([System.IO.Path]::IsPathRooted($Path)) {
    $Path
  }
  else {
    Join-Path -Path $repoRoot -ChildPath $Path
  }

  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $Path"
  }

  $content = Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
  $missingTokens = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missingTokens.Count -gt 0) {
    throw "$Path is missing Chinese log output policy tokens: $($missingTokens -join ', ')"
  }
}

function Assert-FileDoesNotContain {
  param(
    [Parameter(Mandatory = $true)][string]$Path,
    [Parameter(Mandatory = $true)][string[]]$ForbiddenTokens
  )

  $fullPath = if ([System.IO.Path]::IsPathRooted($Path)) {
    $Path
  }
  else {
    Join-Path -Path $repoRoot -ChildPath $Path
  }

  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $Path"
  }

  $content = Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
  $violations = @($ForbiddenTokens | Where-Object { $content.Contains($_) })
  if ($violations.Count -gt 0) {
    throw "$Path still contains English log output tokens: $($violations -join ', ')"
  }
}

$policyTokens = @(
  'PolicyId: `DBA.Log.ChineseOutput`',
  'UE_LOG',
  'ensureMsgf',
  'checkf',
  'TEXT("',
  'Automation Test',
  'MCP',
  'CI',
  'GameplayTag'
)

$gameModeDedicatedServerRuntimeText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x672A, 0x914D, 0x7F6E, 0x8FD0, 0x884C, 0x65F6, 0x53C2, 0x6570)
$oldGameModeDedicatedServerRuntimeText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x672A, 0x914D, 0x7F6E, 0x0020, 0x0052, 0x0075, 0x006E, 0x0074, 0x0069, 0x006D, 0x0065, 0x0020, 0x53C2, 0x6570)
$oldGameModeRuntimeReadySentText = New-CodePointText @(0x0052, 0x0075, 0x006E, 0x0074, 0x0069, 0x006D, 0x0065, 0x0020, 0x6CE8, 0x518C, 0x548C, 0x0020, 0x0052, 0x0065, 0x0061, 0x0064, 0x0079, 0x0020, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimeReadySentText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x6CE8, 0x518C, 0x548C, 0x5C31, 0x7EEA, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimeMatchEndedText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x6BD4, 0x8D5B, 0x7ED3, 0x675F, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimeMatchStartedText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x6BD4, 0x8D5B, 0x5F00, 0x59CB, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimePlayerJoinMissingText = New-CodePointText @(0x65E0, 0x6CD5, 0x4E0A, 0x62A5, 0x8FD0, 0x884C, 0x65F6, 0x73A9, 0x5BB6, 0x52A0, 0x5165)
$gameModeRuntimePlayerJoinRejectedText = New-CodePointText @(0x62D2, 0x7EDD, 0x4E0A, 0x62A5, 0x8FD0, 0x884C, 0x65F6, 0x73A9, 0x5BB6, 0x52A0, 0x5165)
$gameModeRuntimePlayerJoinedSentText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x73A9, 0x5BB6, 0x52A0, 0x5165, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimePlayerLeftSentText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x73A9, 0x5BB6, 0x79BB, 0x5F00, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeRuntimeMatchResultsSkippedText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x6BD4, 0x8D5B, 0x7ED3, 0x679C, 0x8DF3, 0x8FC7)
$gameModeRuntimeMatchResultsSentText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x6BD4, 0x8D5B, 0x7ED3, 0x679C, 0x8BF7, 0x6C42, 0x5DF2, 0x53D1, 0x9001)
$gameModeLaunchOptionsLabelText = New-CodePointText @(0x542F, 0x52A8, 0x53C2, 0x6570, 0x003D, 0x0025, 0x0073)
$gameModePlayerLabelText = New-CodePointText @(0x73A9, 0x5BB6, 0x003D, 0x0025, 0x0073)
$gameModeTeamLabelText = New-CodePointText @(0x961F, 0x4F0D, 0x003D, 0x0025, 0x0073)
$gameModeZodiacLabelText = New-CodePointText @(0x751F, 0x8096, 0x003D, 0x0025, 0x0073)
$gameModeElementLabelText = New-CodePointText @(0x5143, 0x7D20, 0x003D, 0x0025, 0x0073)
$gameModeFixedSkillGroupLabelText = New-CodePointText @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x003D, 0x0025, 0x0073)
$gameModePlayerCountLabelText = New-CodePointText @(0x73A9, 0x5BB6, 0x6570, 0x003D, 0x0025, 0x0064)
$gameModeIdempotencyKeyLabelText = New-CodePointText @(0x5E42, 0x7B49, 0x952E, 0x003D, 0x0025, 0x0073)
$oldRpcUnableGetAscText = New-CodePointText @(0x65E0, 0x6CD5, 0x83B7, 0x53D6, 0x0020, 0x0041, 0x0053, 0x0043)
$oldRpcMissingAscText = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0041, 0x0053, 0x0043)
$oldRpcMissingWorldText = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0057, 0x006F, 0x0072, 0x006C, 0x0064)
$oldRpcMissingOwnerText = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x004F, 0x0077, 0x006E, 0x0065, 0x0072)
$oldRpcInputMismatchText = New-CodePointText @(0x0052, 0x0050, 0x0043, 0x0020, 0x5165, 0x53E3, 0x4E0E, 0x6280, 0x80FD, 0x8F93, 0x5165, 0x7C7B, 0x578B, 0x4E0D, 0x5339, 0x914D)
$oldRpcMissingDbaAbilitySystemText = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0044, 0x0042, 0x0041, 0x0020, 0x0041, 0x0062, 0x0069, 0x006C, 0x0069, 0x0074, 0x0079, 0x0053, 0x0079, 0x0073, 0x0074, 0x0065, 0x006D)
$oldRpcUltimateAscInvalidText = New-CodePointText @(0x80FD, 0x91CF, 0x4E0D, 0x8DB3, 0x6216, 0x0020, 0x0041, 0x0053, 0x0043, 0x0020, 0x65E0, 0x6548)
$rpcUnableGetAbilitySystemText = New-CodePointText @(0x65E0, 0x6CD5, 0x83B7, 0x53D6, 0x80FD, 0x529B, 0x7CFB, 0x7EDF, 0x7EC4, 0x4EF6, 0x0028, 0x0041, 0x0053, 0x0043, 0x0029)
$rpcMissingAbilitySystemText = New-CodePointText @(0x7F3A, 0x5C11, 0x80FD, 0x529B, 0x7CFB, 0x7EDF, 0x7EC4, 0x4EF6, 0x0028, 0x0041, 0x0053, 0x0043, 0x0029)
$rpcMissingWorldObjectText = New-CodePointText @(0x7F3A, 0x5C11, 0x4E16, 0x754C, 0x5BF9, 0x8C61)
$rpcMissingOwnerText = New-CodePointText @(0x7F3A, 0x5C11, 0x62E5, 0x6709, 0x8005)
$rpcInputMismatchText = New-CodePointText @(0x8FDC, 0x7A0B, 0x8C03, 0x7528, 0x5165, 0x53E3, 0x0028, 0x0052, 0x0050, 0x0043, 0x0029, 0x4E0E, 0x6280, 0x80FD, 0x8F93, 0x5165, 0x7C7B, 0x578B, 0x4E0D, 0x5339, 0x914D)
$rpcMissingDbaAbilitySystemText = New-CodePointText @(0x7F3A, 0x5C11, 0x0020, 0x0044, 0x0042, 0x0041, 0x0020, 0x80FD, 0x529B, 0x7CFB, 0x7EDF, 0x7EC4, 0x4EF6)
$rpcUltimateAbilitySystemInvalidText = New-CodePointText @(0x80FD, 0x91CF, 0x4E0D, 0x8DB3, 0x6216, 0x80FD, 0x529B, 0x7CFB, 0x7EDF, 0x7EC4, 0x4EF6, 0x0028, 0x0041, 0x0053, 0x0043, 0x0029, 0x65E0, 0x6548)
$oldRuntimeServiceConfiguredText = New-CodePointText @(0x0052, 0x0075, 0x006E, 0x0074, 0x0069, 0x006D, 0x0065, 0x0020, 0x53C2, 0x6570, 0x8BFB, 0x53D6, 0x5B8C, 0x6210)
$oldRuntimeServiceSessionLabelText = New-CodePointText @(0x0053, 0x0065, 0x0073, 0x0073, 0x0069, 0x006F, 0x006E, 0x0049, 0x0064, 0x003D, 0x0025, 0x0073)
$oldRuntimeServiceServerLabelText = New-CodePointText @(0x0053, 0x0065, 0x0072, 0x0076, 0x0065, 0x0072, 0x0049, 0x0064, 0x003D, 0x0025, 0x0073)
$oldRuntimeServiceTokenLabelText = New-CodePointText @(0x5DF2, 0x914D, 0x7F6E, 0x0054, 0x006F, 0x006B, 0x0065, 0x006E, 0x003D, 0x0025, 0x0073)
$oldRuntimeServiceRequestFailedText = New-CodePointText @(0x0052, 0x0075, 0x006E, 0x0074, 0x0069, 0x006D, 0x0065, 0x0020, 0x8BF7, 0x6C42, 0x5931, 0x8D25)
$runtimeServiceConfiguredText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x53C2, 0x6570, 0x8BFB, 0x53D6, 0x5B8C, 0x6210)
$runtimeServiceSessionLabelText = New-CodePointText @(0x4F1A, 0x8BDD, 0x003D, 0x0025, 0x0073)
$runtimeServiceServerLabelText = New-CodePointText @(0x670D, 0x52A1, 0x5668, 0x003D, 0x0025, 0x0073)
$runtimeServiceTokenLabelText = New-CodePointText @(0x5DF2, 0x914D, 0x7F6E, 0x4EE4, 0x724C, 0x003D, 0x0025, 0x0073)
$runtimeServiceRequestFailedText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x8BF7, 0x6C42, 0x5931, 0x8D25)
$runtimeServiceFalseText = New-CodePointText @(0x0054, 0x0045, 0x0058, 0x0054, 0x0028, 0x0022, 0x5426, 0x0022, 0x0029)
$runtimeServiceTrueText = New-CodePointText @(0x0054, 0x0045, 0x0058, 0x0054, 0x0028, 0x0022, 0x662F, 0x0022, 0x0029)
$gameInstanceNoneText = New-CodePointText @(0x0054, 0x0045, 0x0058, 0x0054, 0x0028, 0x0022, 0x65E0, 0x0022, 0x0029)

Assert-FileContains "AGENTS.md" $policyTokens
Assert-FileContains $controlPrompt.FullName $policyTokens

Assert-FileContains "scripts\validate-unreal-source-guardrails.ps1" @(
  'Test-ChineseLogOutputPolicy',
  'PolicyId: `DBA.Log.ChineseOutput`',
  'UE_LOG',
  'ensureMsgf',
  'checkf',
  'Automation Test'
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
  "test-unreal-chinese-log-output-policy.ps1",
  "Unreal Chinese log output policy contract"
)

Assert-FileContains "scripts\validate-production-evidence-contracts.ps1" @(
  "test-unreal-chinese-log-output-policy.ps1",
  "PASS: Unreal Chinese log output policy contract"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GameInstance\DBAGameInstance.cpp" @(
	"zodiac=%d",
	"队伍创建成功 %s, 成员=%d",
	"success=%s error=%s members=%d",
	'TEXT("None")'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GameInstance\DBAGameInstance.cpp" @(
	$gameInstanceNoneText
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\DivineBeastsArena.cpp" @(
  "Dedicated Server",
  " Client ",
  " Editor ",
  "运行在 Dedicated Server 模式",
  "运行在 Client 模式",
  "运行在 Editor 模式"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
  'Dedicated Server',
  $oldGameModeDedicatedServerRuntimeText,
  $oldGameModeRuntimeReadySentText,
  'Runtime match-ended',
  'Runtime match-started',
  'Runtime player-joined',
  'Runtime player-left',
  'Runtime match-results',
  'PlayerId=%s',
  'Team=%s',
  'Zodiac=%s',
  'Element=%s',
  'FixedSkillGroupId=%s',
  'Players=%d',
  'IdempotencyKey=%s',
  'Options=%s'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
  $gameModeDedicatedServerRuntimeText,
  $gameModeRuntimeReadySentText,
  $gameModeRuntimeMatchEndedText,
  $gameModeRuntimeMatchStartedText,
  $gameModeRuntimePlayerJoinMissingText,
  $gameModeRuntimePlayerJoinRejectedText,
  $gameModeRuntimePlayerJoinedSentText,
  $gameModeRuntimePlayerLeftSentText,
  $gameModeRuntimeMatchResultsSkippedText,
  $gameModeRuntimeMatchResultsSentText,
  $gameModeLaunchOptionsLabelText,
  $gameModePlayerLabelText,
  $gameModeTeamLabelText,
  $gameModeZodiacLabelText,
  $gameModeElementLabelText,
  $gameModeFixedSkillGroupLabelText,
  $gameModePlayerCountLabelText,
  $gameModeIdempotencyKeyLabelText
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
  $oldRpcUnableGetAscText,
  $oldRpcMissingAscText,
  $oldRpcMissingWorldText,
  $oldRpcMissingOwnerText,
  $oldRpcInputMismatchText,
  $oldRpcMissingDbaAbilitySystemText,
  $oldRpcUltimateAscInvalidText
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
  $rpcUnableGetAbilitySystemText,
  $rpcMissingAbilitySystemText,
  $rpcMissingWorldObjectText,
  $rpcMissingOwnerText,
  $rpcInputMismatchText,
  $rpcMissingDbaAbilitySystemText,
  $rpcUltimateAbilitySystemInvalidText
)

Assert-FileDoesNotContain "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp" @(
  $oldRuntimeServiceConfiguredText,
  $oldRuntimeServiceSessionLabelText,
  $oldRuntimeServiceServerLabelText,
  $oldRuntimeServiceTokenLabelText,
  $oldRuntimeServiceRequestFailedText,
  'TEXT("false")',
  'TEXT("true")'
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp" @(
	$runtimeServiceConfiguredText,
	$runtimeServiceSessionLabelText,
	$runtimeServiceServerLabelText,
	$runtimeServiceTokenLabelText,
	$runtimeServiceRequestFailedText,
	$runtimeServiceFalseText,
	$runtimeServiceTrueText
)

Assert-FileDoesNotContain "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendRuntimeServiceTests.cpp" @(
	"Payload should parse as JSON",
	"Payload root should be valid",
	"serverId should be serialized",
	"sessionId should be serialized",
	"runtimeToken should be serialized",
	"idempotencyKey should be serialized",
	"resultJson should be serialized",
	"players array should exist",
	"players array should contain one player",
	"player entry should be an object",
	"playerId should be serialized",
	"team should be serialized",
	"result should be serialized",
	"kills should be serialized",
	"deaths should be serialized",
	"assists should be serialized",
	"score should be serialized",
	"expDelta should be serialized",
	"rewards should be serialized as an object",
	"coin reward should be serialized",
	"honor reward should be serialized"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\GameCore\Private\Tests\DBAOnlineAccountServiceTests.cpp" @(
	"NetworkUnavailable can fallback",
	"Timeout can fallback",
	"EndpointMissing should expose backend contract drift instead of falling back",
	"ServiceUnavailable can fallback",
	"InvalidCredentials cannot fallback",
	"AccountUnavailable cannot fallback",
	"ValidationFailed cannot fallback",
	"Account slot should include command line suffix",
	"Profile slot should include command line suffix",
	"Account id should come from command line",
	"Display name should come from command line",
	"Login type should remain Guest",
	"Command line guest account should be valid"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\GameCore\Private\Tests\DBAFrontendFlowTests.cpp" @(
	"GameInstance should exist",
	"LoginFlow subsystem should exist",
	"AccountService subsystem should exist",
	"FrontendSession subsystem should exist",
	"PartyService subsystem should exist",
	"QueueService subsystem should exist",
	"Guest login should enter CharacterCreate on empty role list",
	"Character creation flow should enter MainLobby",
	"Frontend state should be MainLobby",
	"CreateParty should produce a valid party",
	"Frontend state should be InParty",
	"StartQueue should produce a valid queue",
	"Frontend state should be InQueue",
	"ReadyCheck should be available",
	"ConfirmReady should succeed",
	"Frontend state should switch to Loading (entering arena)",
	"Match session should switch to Loading",
	"Valid travel context should be accepted",
	"Valid travel context should be stored",
	"Accepted travel context should enter Loading",
	"Tampered travel context should be rejected",
	"Rejected travel context must not replace stored FixedSkillGroupId",
	"Rejected travel context must not replace stored MatchSessionId",
	"Rejected travel context should keep session in Loading for the accepted context"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\GameCore\Private\Tests\DBAOnlineAccountJsonTests.cpp" @(
	"Login response should parse",
	"Login should succeed",
	"Token should parse",
	"AccountId should parse",
	"DisplayName should parse",
	"LoginType should parse",
	"Status should parse",
	"Level should parse",
	"Experience should parse",
	"Guest request JSON should parse",
	"Guest request object should be valid",
	"Guest request should include deviceId",
	"Guest request should include deviceName",
	"Guest request should include platform",
	"Guest request should not send account login type",
	"Guest login response should parse",
	"Guest login should succeed",
	"Guest token should parse",
	"Guest refresh token should parse",
	"Guest player id should parse",
	"Guest display name should use backend displayName",
	"Guest account id should parse",
	"Wrapped token response should not infer an endpoint-specific login type",
	"Refresh token response should parse",
	"Refresh token response should succeed",
	"Refresh token response should parse player id",
	"Refresh token response should parse display name",
	"Refresh token response should not be classified as guest by JSON shape alone",
	"Characters should parse",
	"Wrapped characters should parse",
	"Wrapped character count",
	"Wrapped character id",
	"Wrapped character zodiac",
	"Wrapped character element"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAPlayableSkillCatalogTests.cpp" @(
	"Default skill count",
	"Default skill catalog validates",
	"Default skill catalog validation error count",
	"Default summary source",
	"Default summary is valid",
	"Fireball has projectile class",
	"Frost has impact VFX",
	"Bloom has spell class",
	"Chain has cast VFX",
	"Shield has spell class",
	"Shadow has impact SFX",
	"Broken catalog fails validation",
	"Broken catalog reports errors",
	"Missing catalog id is reported",
	"Missing projectile class is reported",
	"Duplicate slot is reported",
	"SkillSlot is duplicated",
	"Missing cast VFX is reported",
	"Missing cast SFX is reported",
	"Missing impact VFX is reported",
	"Catalog ignores invalid slots",
	"Catalog override keeps default fallback count",
	"Override summary appends defaults",
	"Slot 1 override keeps projectile class",
	"Slot 2 remains default frost skill",
	"Catalog-only mode does not append defaults",
	"Catalog-only mode has no slot 2"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDEventFeedTests.cpp" @(
	"Controller should be created",
	"Event feed should start without cached entry",
	"Skill hit confirmed",
	"Event feed should cache latest entry",
	"Cached event text should match",
	"Cached event duration should clamp",
	"BlankEventFeedEntryIsIgnored",
	"Blank event feed entry should preserve cached text",
	"Blank event feed entry should preserve cached duration",
	"Event feed clear should reset cached entry"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDCombatAnnouncementTests.cpp" @(
	"Controller should be created",
	"Combat announcement should start without cached entry",
	"Chain Ready",
	"Combat announcement should cache latest entry",
	"Cached announcement text should match",
	"Cached announcement duration should clamp",
	"BlankCombatAnnouncementIsIgnored",
	"Blank combat announcement should preserve cached text",
	"Blank combat announcement should preserve cached duration",
	"Combat announcement clear should reset cached entry"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDObjectiveStateTests.cpp" @(
	"Controller should be created",
	"Objective state should start invalid",
	"Capture Shrine",
	"Objective state should cache latest update",
	"Objective state should not complete on update",
	"Objective text should match",
	"Objective progress should clamp",
	"BlankObjectiveUpdateIsIgnored",
	"Blank objective update should preserve completion flag",
	"Blank objective update should preserve text",
	"Blank objective update should preserve progress",
	"Objective state should remain valid after completion",
	"Objective state should cache completion",
	"Objective completion should force full progress"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDCriticalStateTests.cpp" @(
	"Controller should be created",
	"Critical state should start invalid",
	"Critical state should cache latest update",
	"Critical state should cache low HP",
	"Critical state should cache normal energy",
	"Critical state reset should still be valid",
	"Critical state should clear low HP",
	"Critical state should clear low energy"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDUltimateReadyPromptTests.cpp" @(
	"Controller should be created",
	"Ultimate-ready prompt should start invalid",
	"Ultimate-ready prompt should cache latest shown state",
	"Ultimate-ready prompt should cache visible state",
	"Ultimate-ready prompt hidden state should still be valid",
	"Ultimate-ready prompt should cache hidden state"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDStatusEffectsTests.cpp" @(
	"Controller should be created",
	"Buff cache should start empty",
	"Debuff cache should start empty",
	"CC cache should start empty",
	"Buff add should cache one entry",
	"Buff id should cache",
	"Buff duration should clamp",
	"Buff add should upsert matching id",
	"Buff duration should update",
	"Buff remove should update cache",
	"Debuff add should cache one entry",
	"Debuff id should cache",
	"Debuff duration should cache",
	"Debuff clear should update cache",
	"CC add should cache one entry",
	"CC id should cache",
	"CC duration should cache",
	"CC remove should update cache"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBALoginVisualLayoutTests.cpp" @(
	"Panel should be centered like the approved login art",
	"Panel should use the Chinese title from the reference",
	"Primary CTA should match the reference",
	"Tool entries should match the reference",
	"First tool is announcements",
	"Second tool is support",
	"Third tool is repair"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAMainLobbyMatchHistoryTests.cpp" @(
	"Controller should be created",
	"Match history JSON should update recent summary",
	"Recent match summary should be valid",
	"Recent session id should parse",
	"Recent result should parse",
	"Recent winner team should parse",
	"Recent map should parse",
	"Recent score should parse",
	"Recent kills should parse",
	"Recent deaths should parse",
	"Recent assists should parse",
	"Recent duration should parse",
	"Recent combat summary should format",
	"Recent played at should parse",
	"Recent exp delta should parse",
	"Recent coin reward should parse",
	"Recent honor reward should parse",
	"Recent reward summary should include all numeric rewards"
)

Assert-FileDoesNotContain "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendPlayerServiceTests.cpp" @(
	"Match history envelope should parse",
	"Total count should parse",
	"Page should parse",
	"Page size should parse",
	"Exactly one match should parse",
	"SessionId should parse",
	"Mode should parse",
	"MapId should parse",
	"Team should parse",
	"Result should parse",
	"Kills should parse",
	"Deaths should parse",
	"Assists should parse",
	"Score should parse",
	"ResultJson should preserve winnerTeam",
	"WinnerTeam should parse",
	"DurationSeconds should parse",
	"ExpDelta should parse",
	"Coin reward should parse",
	"Honor reward should parse",
	"PlayedAt should parse"
)

Assert-FileDoesNotContain "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendSessionServiceTests.cpp" @(
	"Travel URL should contain SessionId",
	"Travel URL should URL-encode PlayerSessionToken",
	"Travel URL should contain DBATeamId",
	"Travel URL should contain DBAZodiac",
	"Travel URL should contain DBAElement",
	"Travel URL should contain DBAFiveCamp",
	"Travel URL should contain DBAFixedSkillGroupId",
	"Nested connection JSON should build a travel URL",
	"Override SessionId should win",
	"Nested token should be included",
	"Nested teamId should be included",
	"Nested DBAZodiac should be included",
	"Nested DBAElement should be included",
	"Nested DBAFiveCamp should be included",
	"Nested DBAFixedSkillGroupId should be included",
	"Nested connection aliases should build a travel URL",
	"Nested serverIp should be used",
	"Nested sessionId should be included",
	"Nested sessionToken alias should be included",
	"Nested alias teamId should be included",
	"Nested alias DBAZodiac should be included",
	"Nested alias DBAElement should be included",
	"Nested alias DBAFixedSkillGroupId should be included",
	"Response envelope data should build a travel URL",
	"Envelope serverIp should be used",
	"Envelope sessionId should be included",
	"Envelope sessionToken alias should be included",
	"Envelope teamId should be included",
	"Envelope DBAZodiac should be included",
	"Envelope DBAElement should be included",
	"Envelope DBAFixedSkillGroupId should be included"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\GameCore\Private\Tests\DBACharacterBuildTypesTests.cpp" @(
	"Rat + Water + East should be a valid build summary",
	"Rat + Water + North should be a valid build summary",
	"FixedSkillGroupId should depend on Zodiac + Element",
	"Changing FiveCamp should not change FixedSkillGroupId",
	"FiveCamp should still be preserved as presentation choice",
	"Missing Zodiac should make the build summary invalid",
	"Missing Element should make the build summary invalid",
	"A missing FiveCamp should be resolved to a stable presentation camp",
	"Resolved FiveCamp should not remain None",
	"Auto FiveCamp must not affect FixedSkillGroupId",
	"Travel context should accept a matching frozen build summary",
	"Travel context summary should preserve Zodiac",
	"Travel context summary should preserve Element",
	"Travel context summary should preserve FiveCamp",
	"Travel context summary should preserve FixedSkillGroupId",
	"Changing FiveCamp should not invalidate the same Zodiac + Element skill group",
	"Travel context should reject a tampered FixedSkillGroupId",
	"Travel context should reject missing Element even when FixedSkillGroupId is present"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAUrlOptionsTests.cpp" @(
	"Escaped player session token is URL-decoded",
	"Unescaped build fields are preserved",
	"Missing option returns empty string",
	"Dedicated Server options should accept a matching frozen build summary",
	"Zodiac should be parsed from URL options",
	"Element should be parsed from URL options",
	"FiveCamp should be preserved as presentation-only identity",
	"FixedSkillGroupId should be preserved from URL options",
	"Dedicated Server options should accept DBATeamId",
	"DBATeamId should parse as a positive authority team id",
	"Dedicated Server options should accept TeamId alias from backend travel URLs",
	"TeamId alias should parse as a positive authority team id",
	"Dedicated Server options should accept mixed-case stable names",
	"Accepted mixed-case FixedSkillGroupId should be normalized for backend reporting",
	"Dedicated Server options should reject tampered FixedSkillGroupId",
	"Dedicated Server options should reject missing frozen identity fields",
	"Dedicated Server options should reject missing TeamId",
	"Missing TeamId rejection should clear the output team id",
	"Dedicated Server options should reject non-positive TeamId",
	"Non-positive TeamId rejection should clear the output team id"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAFixedSkillGroupDataTests.cpp" @(
	"DataAsset fixed skill group row name should match backend/runtime FixedSkillGroupId",
	"Snake + Gold should use the same canonical identity as Dedicated Server admission",
	"Missing identity dimensions should not produce a valid data-table row name",
	"Matching row identity should be accepted",
	"Mismatched row id should be rejected",
	"Missing element should be rejected",
	"Skill group generator can be constructed for validation",
	"Missing zodiac must not produce a fallback fixed skill group",
	"Missing element must not produce a fallback fixed skill group",
	"Invalid identity dimensions must not be reported as configured",
	"Missing zodiac must not produce a skill group summary",
	"Valid identity dimensions should produce a fallback fixed skill group when the table is unavailable",
	"Fallback row id should use canonical FixedSkillGroupId",
	"Fallback fixed skill group should satisfy row identity validation",
	"Valid zodiac should produce fallback summary row ids",
	"Summary water row id should use canonical FixedSkillGroupId",
	"FixedSkillGroups DataTable package should exist for release validation",
	"Missing required FixedSkillGroups DataTable asset",
	"FixedSkillGroups DataTable should load",
	"FixedSkillGroups DataTable should use the fixed skill group row struct",
	"FixedSkillGroups test should cover every Zodiac dimension",
	"FixedSkillGroups test should cover every Element dimension",
	"FixedSkillGroups DataTable should contain 60 Zodiac x Element rows",
	"DataTable row should exist",
	"RowId should match row name",
	"Row identity should be valid"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAAIShowcaseTests.cpp" @(
	"BP_InteractiveProp class should exist",
	"WBP_MainMenu class should exist",
	"WBP_GameHUD class should exist",
	"WBP_InteractionPrompt class should exist",
	"MI_InteractiveGlow should exist",
	"NS_InteractionBurst_V2 should exist",
	"NS_InteractionBurst_V3 should exist",
	"NSE_InteractionSpark asset should exist",
	"NSE_InteractionSpark should stay authored as a NiagaraStatelessEmitter",
	"L_AI_Showcase_Test map should exist",
	"WBP_MainMenu class should load for widget tree validation",
	"WBP_GameHUD class should load for widget tree validation",
	"WBP_MainMenu generated class should expose a widget tree",
	"WBP_GameHUD generated class should expose a widget tree",
	"WBP_MainMenu should expose AIShowcaseMenu_TitleText",
	"WBP_MainMenu should expose AIShowcaseMenu_StartButton",
	"WBP_MainMenu should expose AIShowcaseMenu_OptionsButton",
	"WBP_MainMenu should expose AIShowcaseMenu_QuitButton",
	"WBP_GameHUD should expose AIShowcaseHUD_HealthBar",
	"WBP_GameHUD should expose AIShowcaseHUD_EnergyBar",
	"WBP_GameHUD should expose AIShowcaseHUD_ScoreText",
	"WBP_GameHUD should expose AIShowcaseHUD_MinimapRoot",
	"WBP_GameHUD should expose AIShowcaseHUD_EventFeedBox",
	"WBP_GameHUD should expose AIShowcaseHUD_SkillButton_0",
	"BP_InteractiveProp class should load",
	"BP_InteractiveProp blueprint asset should load",
	"BP_InteractiveProp CDO should exist",
	"Interactive prop should default to active",
	"Interactive prop should not start on cooldown",
	"GlowIntensity default should stay at the documented value",
	"Cooldown default should stay at the documented value",
	"InteractionText default should keep the current prompt",
	"NS_InteractionBurst_V3 should load",
	"FX_Interact should default to the validated V3 Niagara system",
	"FX_Interact should not auto-activate before overlap or interact",
	"AC_Interact should not auto-play before interact",
	"Interaction collision should generate overlap events",
	"Prompt widget class should load",
	"Prompt widget component should use WBP_InteractionPrompt",
	"Prompt widget should start hidden in game",
	"BP_InteractiveProp class should load for interaction contract",
	"Interact should be callable from automation without parameters",
	"ResetInteractionCooldown should be callable from automation without parameters",
	"Interaction contract test world should be created",
	"BP_InteractiveProp should spawn in a transient automation world",
	"Spawned interactive prop should not start on cooldown",
	"Interact should be safe to invoke in a transient automation world",
	"ResetInteractionCooldown should be safe to invoke in a transient automation world",
	"AI_Showcase test map should load",
	"BP_InteractiveProp class should load for map validation",
	"AI_Showcase map should expose a persistent level",
	"AI_Showcase map should contain a BP_InteractiveProp instance",
	"AI_Showcase interactive prop should stay near the documented placement"
)

Write-Host "PASS: Unreal Chinese log output policy contract" -ForegroundColor Green
