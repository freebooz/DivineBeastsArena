<#
Validates Arena HUD event feed entries flow from UIManager to controller, root
HUD, and a Blueprint-bindable EventFeed widget base.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$eventFeedHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaEventFeedWidgetBase.h"
$eventFeedCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaEventFeedWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$eventFeedTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDEventFeedTests.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

Assert-True (Test-Path -LiteralPath $eventFeedHeaderPath) "Expected UDBAArenaEventFeedWidgetBase.h to exist."
Assert-True (Test-Path -LiteralPath $eventFeedCppPath) "Expected UDBAArenaEventFeedWidgetBase.cpp to exist."
Assert-True (Test-Path -LiteralPath $eventFeedTestPath) "Expected DBAArenaHUDEventFeedTests.cpp to exist."

$controllerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerHeaderPath
$controllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerCppPath
$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$eventFeedHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $eventFeedHeaderPath
$eventFeedCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $eventFeedCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath
$eventFeedTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $eventFeedTestPath
$cachedEventText = [string]::Concat([char]0x6280, [char]0x80FD, [char]0x547D, [char]0x4E2D, [char]0x5DF2, [char]0x786E, [char]0x8BA4)

Assert-True ($eventFeedHeader -match "class\s+DIVINEBEASTSARENA_API\s+UDBAArenaEventFeedWidgetBase") "Expected EventFeed widget base class."
Assert-True ($eventFeedHeader -match "AddEventEntry\s*\(\s*const FText&\s+Text,\s*float\s+Duration\s*\)") "Expected AddEventEntry API."
Assert-True ($eventFeedHeader -match "ClearEventFeed\s*\(") "Expected ClearEventFeed API."
Assert-True ($eventFeedHeader -match "BP_OnEventEntryAdded") "Expected Blueprint event for added event entries."
Assert-True ($eventFeedHeader -match "BP_OnEventFeedCleared") "Expected Blueprint event for clearing event feed."
Assert-True ($eventFeedCpp -match "NormalizeHUDWidgetText[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected EventFeed widget to normalize and reject blank HUD text."
Assert-True ($eventFeedCpp -match "AddEventEntry[\s\S]*FText\s+NormalizedText[\s\S]*if\s*\(\s*!NormalizeHUDWidgetText\(Text,\s*NormalizedText\)\s*\)[\s\S]*return;") "Expected EventFeed widget to no-op blank event entries."
Assert-True ($eventFeedCpp -match "BP_OnEventEntryAdded\(NormalizedText,\s*FMath::Max\(0\.0f,\s*Duration\)\)") "Expected EventFeed widget to clamp entry duration and forward normalized text."
Assert-True ($eventFeedCpp -match "BP_OnEventFeedCleared\(\)") "Expected EventFeed widget to forward clear event."

Assert-True ($controllerHeader -match "AddEventFeedEntry\s*\(\s*const FText&\s+Text,\s*float\s+Duration\s*\)") "Expected controller event-feed entrypoint."
Assert-True ($controllerHeader -match "ClearEventFeed\s*\(") "Expected controller event-feed clear entrypoint."
Assert-True ($controllerHeader -match "FDBAArenaEventFeedEntry") "Expected controller cached event-feed entry type."
Assert-True ($controllerHeader -match "GetLastEventFeedEntry") "Expected controller cached event-feed getter."
Assert-True ($controllerHeader -match "FOnEventFeedEntryAdded") "Expected controller event-feed entry delegate."
Assert-True ($controllerHeader -match "FOnEventFeedCleared") "Expected controller event-feed clear delegate."
Assert-True ($controllerCpp -match "LastEventFeedEntry\.bIsValid\s*=\s*true") "Expected controller to mark cached event-feed entry valid."
Assert-True ($controllerCpp -match "NormalizeHUDFeedbackText[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected controller to normalize and reject blank HUD feedback text."
Assert-True ($controllerCpp -match "AddEventFeedEntry[\s\S]*FText\s+NormalizedText[\s\S]*if\s*\(\s*!NormalizeHUDFeedbackText\(Text,\s*NormalizedText\)\s*\)[\s\S]*return;") "Expected controller to no-op blank event-feed entries."
Assert-True ($controllerCpp -match "LastEventFeedEntry\.Text\s*=\s*NormalizedText") "Expected controller to cache normalized event-feed text."
Assert-True ($controllerCpp -match "LastEventFeedEntry\.Duration\s*=\s*FMath::Max\(0\.0f,\s*Duration\)") "Expected controller to cache clamped event-feed duration."
Assert-True ($controllerCpp -match "OnEventFeedEntryAdded\.Broadcast\(NormalizedText,\s*LastEventFeedEntry\.Duration\)") "Expected controller to broadcast normalized event text and cached clamped event feed duration."
Assert-True ($controllerCpp -match "LastEventFeedEntry\s*=\s*FDBAArenaEventFeedEntry\(\)") "Expected controller clear to reset cached event-feed entry."
Assert-True ($controllerCpp -match "OnEventFeedCleared\.Broadcast\(\)") "Expected controller to broadcast event feed clear."

Assert-True ($rootHeader -match "class\s+UDBAArenaEventFeedWidgetBase") "Expected root HUD to forward declare EventFeed widget."
Assert-True ($rootHeader -match "HandleControllerEventFeedEntryAdded") "Expected root HUD event-feed entry handler."
Assert-True ($rootHeader -match "HandleControllerEventFeedCleared") "Expected root HUD event-feed clear handler."
Assert-True ($rootHeader -match "TObjectPtr<UDBAArenaEventFeedWidgetBase>\s+EventFeed") "Expected root HUD optional EventFeed binding."
Assert-True ($rootCpp -match "UDBAArenaEventFeedWidgetBase\.h") "Expected root HUD to include EventFeed widget type."
Assert-True ($rootCpp -match "OnEventFeedEntryAdded\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded\)") "Expected root HUD to bind event-feed entries."
Assert-True ($rootCpp -match "OnEventFeedEntryAdded\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded\)") "Expected root HUD to unbind event-feed entries."
Assert-True ($rootCpp -match "GetLastEventFeedEntry\(\)") "Expected root HUD to inspect cached event-feed entry after binding."
Assert-True ($rootCpp -match "LastEventFeedEntry\.bIsValid") "Expected root HUD to gate cached event-feed replay."
Assert-True ($rootCpp -match "HandleControllerEventFeedEntryAdded\(LastEventFeedEntry\.Text,\s*LastEventFeedEntry\.Duration\)") "Expected root HUD to replay cached event-feed entry."
Assert-True ($rootCpp -match "EventFeed->AddEventEntry\(Text,\s*Duration\)") "Expected root HUD to forward event-feed entries."
Assert-True ($rootCpp -match "EventFeed->ClearEventFeed\(\)") "Expected root HUD to forward event-feed clear."

Assert-True ($managerHeader -match "AddArenaHUDEventFeedEntry\s*\(\s*const FText&\s+Text,\s*float\s+Duration\s*\)") "Expected UI manager event-feed entrypoint."
Assert-True ($managerHeader -match "ClearArenaHUDEventFeed\s*\(") "Expected UI manager event-feed clear entrypoint."
Assert-True ($managerCpp -match "Controller->AddEventFeedEntry\(Text,\s*Duration\)") "Expected UI manager to push event-feed entries."
Assert-True ($managerCpp -match "Controller->ClearEventFeed\(\)") "Expected UI manager to clear event feed."

Assert-True ($characterCpp -match "HandleArenaHUDSkillCueExecuted[\s\S]*AddArenaHUDEventFeedEntry\(AnnouncementText,\s*ArenaHUDSkillCueAnnouncementDuration\)") "Expected GAS skill cue feedback to append the same text to the Arena HUD EventFeed."
Assert-True ($eventFeedTest -match "EventFeedCachesLatestEntry") "Expected automation coverage for cached event-feed entries."
Assert-True ($eventFeedTest -match "AddEventFeedEntry\(FText::FromString\(TEXT\(`"   `"\)") "Expected automation coverage for blank event-feed entries."
Assert-True ($eventFeedTest -match "GetLastEventFeedEntry") "Expected automation test to exercise cached entry getter."
Assert-True ($eventFeedTest -match [regex]::Escape($cachedEventText)) "Expected automation test to verify cached event text."
Assert-True ($eventFeedTest -match "ClearEventFeed\(\)") "Expected automation test to verify cached entry reset."

Write-Host "PASS: Arena HUD event feed widget sync contract" -ForegroundColor Green
