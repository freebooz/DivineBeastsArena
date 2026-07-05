<#
Validates UE UI runtime logs and developer-visible errors use Chinese text.
Chinese strings are built from code points so Windows PowerShell can parse this
script correctly even when the file is read as ANSI.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

function New-CodePointText {
  param(
    [Parameter(Mandatory = $true)][int[]]$CodePoints
  )

  return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function Get-RepoFileContent {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath
  )

  $fullPath = Join-Path -Path $repoRoot -ChildPath $RelativePath
  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $RelativePath"
  }

  return Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
}

function Assert-FileDoesNotContain {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string[]]$ForbiddenTokens
  )

  $content = Get-RepoFileContent -RelativePath $RelativePath
  $violations = @($ForbiddenTokens | Where-Object { $content.Contains($_) })
  if ($violations.Count -gt 0) {
    throw "$RelativePath still contains English runtime output: $($violations -join ', ')"
  }
}

function Assert-FileContains {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string[]]$RequiredTokens
  )

  $content = Get-RepoFileContent -RelativePath $RelativePath
  $missing = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missing.Count -gt 0) {
    throw "$RelativePath is missing Chinese runtime output: $($missing -join ', ')"
  }
}

$splashWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Splash\UDBASplashVideoWidget.cpp"
$startupVideoWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Startup\UDBAStartupVideoWidget.cpp"
$gameUIManagerPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$softwareCursorWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Common\UDBASoftwareCursorWidget.cpp"
$characterPreviewActorPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login\DBACharacterPreviewActor.cpp"
$characterSelectWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login\UDBACharacterSelectFlowWidgetBase.cpp"
$characterCreateWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login\UDBACharacterCreateFlowWidgetBase.cpp"
$loginFlowWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\Login\UDBALoginFlowWidgetBase.cpp"
$loginFlowSubsystemPath = "DBA_GameClient\Source\GameCore\Private\GameCore\Session\DBALoginFlowSubsystem.cpp"
$onlineAccountServicePath = "DBA_GameClient\Source\GameCore\Private\GameCore\Account\DBAOnlineAccountService.cpp"
$onlineAccountJsonPath = "DBA_GameClient\Source\GameCore\Private\GameCore\Account\DBAOnlineAccountJson.cpp"
$accountServiceBasePath = "DBA_GameClient\Source\GameCore\Private\GameCore\Account\DBAAccountServiceBase.cpp"
$mockAccountServicePath = "DBA_GameClient\Source\GameCore\Private\GameCore\Account\DBAMockAccountService.cpp"
$playableSkillCatalogPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAPlayableSkillCatalogDataAsset.cpp"
$playableSkillComponentPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAPlayableSkillComponent.cpp"
$partyServicePath = "DBA_GameClient\Source\GameCore\Private\GameCore\Party\DBAPartyServiceBase.cpp"
$queueServicePath = "DBA_GameClient\Source\GameCore\Private\GameCore\Queue\DBAQueueServiceBase.cpp"
$gameBackendAuthServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendAuthService.cpp"
$gameBackendConfigServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendConfigService.cpp"
$gameBackendMailServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendMailService.cpp"
$gameBackendMatchServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendMatchService.cpp"
$gameBackendPlayerServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendPlayerService.cpp"
$gameBackendRoomServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRoomService.cpp"
$gameBackendRuntimeServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp"
$gameBackendSessionServicePath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendSessionService.cpp"
$gameBackendHttpClientPath = "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendHttpClient.cpp"
$mainLobbyControllerPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp"
$lobbyPlayerHUDWidgetPath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBALobbyPlayerHUDWidgetBase.cpp"
$stagedMovieFallbackText = New-CodePointText @(0x6682, 0x5B58, 0x542F, 0x52A8, 0x89C6, 0x9891, 0x7F3A, 0x5931, 0xFF0C, 0x6539, 0x7528, 0x6E90, 0x7801, 0x76EE, 0x5F55, 0x89C6, 0x9891)
$startupMovieMissingText = New-CodePointText @(0x542F, 0x52A8, 0x89C6, 0x9891, 0x6587, 0x4EF6, 0x7F3A, 0x5931, 0x6216, 0x4E3A, 0x7A7A)
$ticketIdMissingText = New-CodePointText @(0x5339, 0x914D, 0x7968, 0x636E, 0x5DF2, 0x521B, 0x5EFA, 0xFF0C, 0x4F46, 0x54CD, 0x5E94, 0x4E2D, 0x7F3A, 0x5C11, 0x7968, 0x636E, 0x20, 0x49, 0x44, 0x3002)
$garbledFlowStateText = New-CodePointText @(0x6D41, 0x6D6A, 0x72B6, 0x6001, 0x5207, 0x6362)
$garbledEnterLobbyText = New-CodePointText @(0x8FDB, 0x5165, 0x5927, 0x53B3)
$garbledSharedLobbyServerText = New-CodePointText @(0x5171, 0x4EAB, 0x5927, 0x53B3, 0x670D, 0x52A1, 0x5668)
$startupVideoDedicatedServerText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x5FFD, 0x7565, 0x5A92, 0x4F53, 0x64AD, 0x653E, 0x5668, 0x8BBE, 0x7F6E)
$splashDedicatedServerSkipText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x8DF3, 0x8FC7, 0x542F, 0x52A8, 0x89C6, 0x9891, 0x64AD, 0x653E)
$splashDedicatedServerOpenedText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x5FFD, 0x7565, 0x5A92, 0x4F53, 0x6253, 0x5F00, 0x4E8B, 0x4EF6)
$splashDedicatedServerFailedText = New-CodePointText @(0x4E13, 0x7528, 0x670D, 0x52A1, 0x5668, 0x5FFD, 0x7565, 0x5A92, 0x4F53, 0x6253, 0x5F00, 0x5931, 0x8D25, 0x4E8B, 0x4EF6)
$oldWaitingPlayerControllerText = New-CodePointText @(0x6B63, 0x5728, 0x7B49, 0x5F85, 0x0020, 0x0050, 0x006C, 0x0061, 0x0079, 0x0065, 0x0072, 0x0043, 0x006F, 0x006E, 0x0074, 0x0072, 0x006F, 0x006C, 0x006C, 0x0065, 0x0072)
$oldLobbyHudWaitingPlayerControllerText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x6B63, 0x5728, 0x7B49, 0x5F85, 0x0020, 0x0050, 0x006C, 0x0061, 0x0079, 0x0065, 0x0072, 0x0043, 0x006F, 0x006E, 0x0074, 0x0072, 0x006F, 0x006C, 0x006C, 0x0065, 0x0072)
$oldPlayerControllerCountText = New-CodePointText @(0x0050, 0x006C, 0x0061, 0x0079, 0x0065, 0x0072, 0x0043, 0x006F, 0x006E, 0x0074, 0x0072, 0x006F, 0x006C, 0x006C, 0x0065, 0x0072, 0x6570, 0x91CF)
$oldMissingPlayerControllerText = New-CodePointText @(0x4E2D, 0x6CA1, 0x6709, 0x627E, 0x5230, 0x0020, 0x0050, 0x006C, 0x0061, 0x0079, 0x0065, 0x0072, 0x0043, 0x006F, 0x006E, 0x0074, 0x0072, 0x006F, 0x006C, 0x006C, 0x0065, 0x0072)
$oldArenaHudWidgetUnavailableText = New-CodePointText @(0x7ADE, 0x6280, 0x573A, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x63A7, 0x4EF6, 0x84DD, 0x56FE, 0x4E0D, 0x53EF, 0x7528)
$oldLobbyPlayerHudWidgetUnavailableText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x63A7, 0x4EF6, 0x84DD, 0x56FE, 0x4E0D, 0x53EF, 0x7528)
$oldLobbyGameHudShownText = New-CodePointText @(0x5DF2, 0x4E3A, 0x5927, 0x5385, 0x5730, 0x56FE, 0x663E, 0x793A, 0x6E38, 0x620F, 0x0020, 0x0048, 0x0055, 0x0044)
$oldLobbyPlayerHudViewportText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x5DF2, 0x6DFB, 0x52A0, 0x5230, 0x89C6, 0x53E3)
$oldLobbyHudRetryLimitText = New-CodePointText @(0x5927, 0x5385, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x91CD, 0x8BD5, 0x8D85, 0x8FC7, 0x4E0A, 0x9650)
$oldLobbyHudWaitingText = New-CodePointText @(0x5927, 0x5385, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x5C1A, 0x672A, 0x5C31, 0x7EEA)
$oldLobbyPlayerHudCreatedText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x0020, 0x0048, 0x0055, 0x0044, 0x0020, 0x63A7, 0x4EF6, 0x5DF2, 0x521B, 0x5EFA)
$waitingPlayerControllerText = New-CodePointText @(0x6B63, 0x5728, 0x7B49, 0x5F85, 0x73A9, 0x5BB6, 0x63A7, 0x5236, 0x5668)
$lobbyPlayerInterfaceWaitingText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x754C, 0x9762, 0x6B63, 0x5728, 0x7B49, 0x5F85, 0x73A9, 0x5BB6, 0x63A7, 0x5236, 0x5668)
$playerControllerCountText = New-CodePointText @(0x73A9, 0x5BB6, 0x63A7, 0x5236, 0x5668, 0x6570, 0x91CF)
$missingPlayerControllerText = New-CodePointText @(0x4E2D, 0x6CA1, 0x6709, 0x627E, 0x5230, 0x73A9, 0x5BB6, 0x63A7, 0x5236, 0x5668)
$levelAddressText = New-CodePointText @(0x5173, 0x5361, 0x5730, 0x5740)
$arenaInterfaceWidgetUnavailableText = New-CodePointText @(0x7ADE, 0x6280, 0x573A, 0x754C, 0x9762, 0x63A7, 0x4EF6, 0x84DD, 0x56FE, 0x4E0D, 0x53EF, 0x7528)
$lobbyPlayerInterfaceWidgetUnavailableText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x754C, 0x9762, 0x63A7, 0x4EF6, 0x84DD, 0x56FE, 0x4E0D, 0x53EF, 0x7528)
$lobbyGameInterfaceShownText = New-CodePointText @(0x5DF2, 0x4E3A, 0x5927, 0x5385, 0x5730, 0x56FE, 0x663E, 0x793A, 0x6E38, 0x620F, 0x754C, 0x9762)
$lobbyPlayerInterfaceViewportText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x754C, 0x9762, 0x5DF2, 0x6DFB, 0x52A0, 0x5230, 0x89C6, 0x53E3)
$lobbyInterfaceRetryLimitText = New-CodePointText @(0x5927, 0x5385, 0x754C, 0x9762, 0x91CD, 0x8BD5, 0x8D85, 0x8FC7, 0x4E0A, 0x9650)
$lobbyInterfaceWaitingText = New-CodePointText @(0x5927, 0x5385, 0x754C, 0x9762, 0x5C1A, 0x672A, 0x5C31, 0x7EEA)
$lobbyPlayerInterfaceCreatedText = New-CodePointText @(0x5927, 0x5385, 0x73A9, 0x5BB6, 0x754C, 0x9762, 0x63A7, 0x4EF6, 0x5DF2, 0x521B, 0x5EFA)

Assert-FileDoesNotContain $splashWidgetPath @(
  "Press ESC to skip",
  "[UDBASplashVideoWidget] NativeConstruct",
  "[UDBASplashVideoWidget] VideoImage:",
  "SkipHintText is NULL - Blueprint binding may have failed",
  "SkipButton is NULL - Blueprint binding may have failed",
  "Timed out, continuing to login",
  "[UDBASplashVideoWidget] PlayVideo",
  "Failed to create MediaPlayer",
  "Failed to create FileMediaSource",
  "Staged movie missing, using source path",
  "Startup movie file missing or empty",
  "Startup movie path",
  "Failed to create MediaTexture",
  "Video brush set",
  "VideoImage is NULL - Blueprint binding may have failed",
  "MediaSoundComponent registered",
  "Failed to create MediaSoundComponent",
  "Failed to request media open",
  "Media open requested",
  "Media opened",
  "Media sound after open",
  "Media open failed",
  "Media end reached",
  "Playback started",
  "Media was ready but not playing, retrying playback",
  "Playback state after delay",
  "Sound state after delay",
  "Failed to create fallback audio wave",
  "Fallback Startup.wav audio playing",
  "Failed to spawn fallback Startup.wav audio",
  "Native Windows Startup.wav audio playing",
  "Native audio fallback is only available on Windows",
  "Unsupported WAV format",
  "Loaded fallback WAV",
  "[UDBASplashVideoWidget] SkipVideo",
  "[UDBASplashVideoWidget] OnVideoFinished",
  "[UDBASplashVideoWidget] TransitionToLogin",
  "Unable to resolve DBA game instance when transitioning to login.",
  "Dedicated Server"
)

Assert-FileContains $splashWidgetPath @(
  $stagedMovieFallbackText,
  $startupMovieMissingText,
  $splashDedicatedServerSkipText,
  $splashDedicatedServerOpenedText,
  $splashDedicatedServerFailedText
)

Assert-FileDoesNotContain $startupVideoWidgetPath @(
  "Dedicated Server"
)

Assert-FileContains $startupVideoWidgetPath @(
  $startupVideoDedicatedServerText
)

Assert-FileDoesNotContain $gameUIManagerPath @(
  "Party panel widget blueprint is unavailable.",
  "Invite panel widget blueprint is unavailable.",
  "Queue mode select widget blueprint is unavailable.",
  "Queue status widget blueprint is unavailable.",
  "Ready check widget blueprint is unavailable.",
  "Match found widget blueprint is unavailable.",
  "Portal confirm widget blueprint is unavailable.",
  "Interaction prompt widget blueprint is unavailable.",
  "Newbie village main widget blueprint is unavailable.",
  "Newbie task tracker widget blueprint is unavailable.",
  "Ready check completed:",
  "accepted",
  "declined",
  "Portal confirmed:",
  "Portal cancelled.",
  $oldWaitingPlayerControllerText,
  $oldLobbyHudWaitingPlayerControllerText,
  $oldPlayerControllerCountText,
  $oldMissingPlayerControllerText,
  $oldArenaHudWidgetUnavailableText,
  $oldLobbyPlayerHudWidgetUnavailableText,
  $oldLobbyGameHudShownText,
  $oldLobbyPlayerHudViewportText,
  $oldLobbyHudRetryLimitText,
  $oldLobbyHudWaitingText,
  $oldLobbyPlayerHudCreatedText,
  "URL=%s"
)

Assert-FileContains $gameUIManagerPath @(
  $waitingPlayerControllerText,
  $lobbyPlayerInterfaceWaitingText,
  $playerControllerCountText,
  $missingPlayerControllerText,
  $levelAddressText,
  $arenaInterfaceWidgetUnavailableText,
  $lobbyPlayerInterfaceWidgetUnavailableText,
  $lobbyGameInterfaceShownText,
  $lobbyPlayerInterfaceViewportText,
  $lobbyInterfaceRetryLimitText,
  $lobbyInterfaceWaitingText,
  $lobbyPlayerInterfaceCreatedText
)

Assert-FileDoesNotContain $softwareCursorWidgetPath @(
  "Loaded cursor texture asset:",
  "Loaded cursor texture:",
  "Failed to load cursor PNG:"
)

Assert-FileDoesNotContain $characterPreviewActorPath @(
  "Failed to load any preview skeletal mesh.",
  "Loaded mesh:",
  "Skeleton=",
  'TEXT("Valid")',
  'TEXT("None")',
  "Mesh has no skeleton, skip idle animation."
)

Assert-FileDoesNotContain $characterSelectWidgetPath @(
  "Characters updated:",
  "Selected character:",
  "Native fallback layout created",
  "Using world 3D character presentation stage.",
  "Button click sound not found.",
  "BGM asset not found.",
  "Failed to spawn BGM component."
)

Assert-FileDoesNotContain $characterCreateWidgetPath @(
  "Submitted character creation:",
  "Native fallback layout created",
  "Using world 3D character presentation stage for zodiac",
  "Failed to spawn world 3D character presentation stage.",
  "Button click sound not found.",
  "BGM asset not found.",
  "Failed to spawn BGM component."
)

Assert-FileDoesNotContain $loginFlowWidgetPath @(
  "组件绑定: LoginButton=",
  "GuestLoginButton=",
  "DebugLoginButton=",
  "EmailInput=",
  "PasswordInput=",
  "LoginButton \u4e3a\u7a7a",
  "GuestLoginButton \u4e3a\u7a7a",
  "DebugLoginButton \u4e3a\u7a7a"
)

Assert-FileDoesNotContain $loginFlowSubsystemPath @(
  $garbledFlowStateText,
  $garbledEnterLobbyText,
  $garbledSharedLobbyServerText,
  "Account service unavailable",
  "Character selection failed",
  "Character selection is not available in current state",
  "Character creation is not available in current state"
)

Assert-FileDoesNotContain $onlineAccountServicePath @(
  "Online account service initialized:",
  "GuestLogin request failed:",
  "Online login failed",
  "Online registration failed",
  "Auto login failed",
  "Online character list request failed:",
  "Online character creation failed",
  "Online character selection request failed:",
  "Online character selection failed",
  "Mock fallback unavailable",
  "Online login unavailable, fallback to mock guest login"
)

Assert-FileDoesNotContain $onlineAccountJsonPath @(
  "Malformed JSON response",
  "Missing characters array"
)

Assert-FileDoesNotContain $accountServiceBasePath @(
  "Login -",
  "Register -",
  "GuestLogin -",
  "AutoLogin -",
  "GetCharacterList -",
  "GetCharacterProfile -",
  "CreateCharacter -",
  "DeleteCharacter -",
  "SaveProfile -",
  "LoadAccountSaveGame -",
  "SaveAccountSaveGame -",
  "LoadProfileSaveGame -",
  "SaveProfileSaveGame -",
  "CreateDefaultAccountSaveGame -",
  "CreateDefaultProfileSaveGame -",
  "HandleCorruptedSaveGame -"
)

Assert-FileDoesNotContain $mockAccountServicePath @(
  "Login -",
  "Register -",
  "GuestLogin -",
  "AutoLogin -",
  "Logout -",
  "GetCharacterList -",
  "GetCharacterProfile -",
  "CreateCharacter -",
  "DeleteCharacter -",
  "SelectCharacter -",
  "PerformGuestLogin -",
  "SaveCurrentAccount -"
)

Assert-FileDoesNotContain $playableSkillCatalogPath @(
  "SkillSlot=%d SkillId=%s",
  "ProjectileClass is not configured",
  "ChainLightningClass is not configured",
  "BloomHealingClass is not configured",
  "HolyShieldClass is not configured",
  "EffectShape is invalid",
  "ProjectileSpeed must be greater than 0",
  "ProjectileRadius must be greater than 0",
  "CastNiagaraVFXAsset is not configured",
  "ProjectileNiagaraVFXAsset is not configured",
  "ImpactNiagaraVFXAsset is not configured",
  "CastSFXAsset is not configured",
  "FlySFXAsset is not configured",
  "ImpactSFXAsset is not configured",
  "CatalogId is not configured",
  "SkillSpecs is empty",
  "SkillSlot must be greater than 0",
  "SkillSlot is duplicated",
  "SkillId is not configured",
  "DisplayName is not configured",
  "Magnitude must be greater than 0",
  "Cooldown must be greater than 0",
  "CastVFXScale must be greater than 0"
)

Assert-FileDoesNotContain $playableSkillComponentPath @(
  "Playable skill catalog validation failed:"
)

Assert-FileDoesNotContain $partyServicePath @(
  "Party service initialized",
  "Party service deinitialized",
  "CreateParty failed: account not logged in",
  "Party not created",
  "Party is full",
  "Invalid invite",
  "LeaveParty succeeded",
  "Member not found or is leader",
  "Member not found"
)

Assert-FileDoesNotContain $queueServicePath @(
  "Queue service initialized",
  "Queue service deinitialized",
  "StartQueue failed: missing logged-in account or party",
  "CancelQueue succeeded"
)

Assert-FileDoesNotContain $gameBackendAuthServicePath @(
  "Request failed.",
  "Auth request failed."
)

Assert-FileDoesNotContain $gameBackendConfigServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendMailServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendMatchServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendPlayerServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendRoomServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendRuntimeServicePath @(
  "Runtime request failed."
)

Assert-FileDoesNotContain $gameBackendSessionServicePath @(
  "Request failed."
)

Assert-FileDoesNotContain $gameBackendHttpClientPath @(
  "Backend subsystem invalid."
)

Assert-FileDoesNotContain $mainLobbyControllerPath @(
  "Switch camp theme request",
  "Navigate to newbie village requested.",
  "Navigate to practice requested.",
  "Exit game requested.",
  "Player service unavailable.",
  "Room service unavailable.",
  "RoomId is empty.",
  "Not in room.",
  "Match service unavailable.",
  "Session service unavailable.",
  "Match history response could not be parsed.",
  "Backend state switched",
  "Backend request failed.",
  "Backend error:",
  "World context is invalid, cannot start ticket polling.",
  "Ticket created but ticket id is missing."
)

Assert-FileContains $mainLobbyControllerPath @(
  $ticketIdMissingText
)

Assert-FileDoesNotContain $lobbyPlayerHUDWidgetPath @(
  "[LobbyPlayerHUD] Constructed:",
  "[LobbyPlayerHUD] Loaded skill hotkeys from FixedSkillGroup asset:"
)

Write-Host "PASS: UE UI runtime Chinese output contract" -ForegroundColor Green
