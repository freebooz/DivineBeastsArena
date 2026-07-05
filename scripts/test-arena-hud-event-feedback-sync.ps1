<#
Validates combat announcements, critical-state hints, and objective tracker events flow through the Arena HUD controller.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$announcementHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBACombatAnnouncementWidgetBase.h"
$announcementCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBACombatAnnouncementWidgetBase.cpp"
$criticalHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBACriticalStateHintWidgetBase.h"
$criticalCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBACriticalStateHintWidgetBase.cpp"
$objectiveHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaObjectiveTrackerWidgetBase.h"
$objectiveCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaObjectiveTrackerWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$announcementTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDCombatAnnouncementTests.cpp"
$objectiveTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDObjectiveStateTests.cpp"
$criticalStateTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDCriticalStateTests.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$controllerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerHeaderPath
$controllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerCppPath
$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$announcementHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $announcementHeaderPath
$announcementCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $announcementCppPath
$criticalHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $criticalHeaderPath
$criticalCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $criticalCppPath
$objectiveHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $objectiveHeaderPath
$objectiveCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $objectiveCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$announcementTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $announcementTestPath
$objectiveTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $objectiveTestPath
$criticalStateTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $criticalStateTestPath
$announcementText = -join @([char]0x8FDE, [char]0x9501, [char]0x5DF2, [char]0x5C31, [char]0x7EEA)
$objectiveText = -join @([char]0x5360, [char]0x9886, [char]0x796D, [char]0x575B)

Assert-True ($controllerHeader -match "ShowCombatAnnouncement\s*\(\s*const FText&\s+Text,\s*float\s+Duration\s*\)") "Expected controller combat announcement entrypoint."
Assert-True ($controllerHeader -match "ClearCombatAnnouncement") "Expected controller clear announcement entrypoint."
Assert-True ($controllerHeader -match "FDBAArenaCombatAnnouncementEntry") "Expected controller cached combat announcement type."
Assert-True ($controllerHeader -match "GetLastCombatAnnouncement") "Expected controller cached combat announcement getter."
Assert-True ($controllerHeader -match "UpdateCriticalStateHints\s*\(\s*bool\s+bLowHP,\s*bool\s+bLowEnergy\s*\)") "Expected controller critical-state entrypoint."
Assert-True ($controllerHeader -match "FDBAArenaCriticalStateHintState") "Expected controller cached critical-state type."
Assert-True ($controllerHeader -match "GetLastCriticalStateHints") "Expected controller cached critical-state getter."
Assert-True ($controllerHeader -match "UpdateArenaObjective\s*\(\s*const FText&\s+ObjectiveText,\s*float\s+Progress\s*\)") "Expected controller objective update entrypoint."
Assert-True ($controllerHeader -match "CompleteArenaObjective") "Expected controller objective completion entrypoint."
Assert-True ($controllerHeader -match "FDBAArenaObjectiveState") "Expected controller cached objective state type."
Assert-True ($controllerHeader -match "GetLastArenaObjectiveState") "Expected controller cached objective state getter."
Assert-True ($controllerHeader -match "FOnCombatAnnouncementShown") "Expected controller announcement delegate."
Assert-True ($controllerHeader -match "FOnCriticalStateHintsChanged") "Expected controller critical-state delegate."
Assert-True ($controllerHeader -match "FOnArenaObjectiveUpdated") "Expected controller objective delegate."
Assert-True ($controllerCpp -match "LastCombatAnnouncement\.bIsValid\s*=\s*true") "Expected controller to mark cached combat announcement valid."
Assert-True ($controllerCpp -match "NormalizeHUDFeedbackText[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected controller to normalize and reject blank HUD feedback text."
Assert-True ($controllerCpp -match "ShowCombatAnnouncement[\s\S]*FText\s+NormalizedText[\s\S]*if\s*\(\s*!NormalizeHUDFeedbackText\(Text,\s*NormalizedText\)\s*\)[\s\S]*return;") "Expected controller to no-op blank combat announcements."
Assert-True ($controllerCpp -match "LastCombatAnnouncement\.Text\s*=\s*NormalizedText") "Expected controller to cache normalized combat announcement text."
Assert-True ($controllerCpp -match "LastCombatAnnouncement\.Duration\s*=\s*FMath::Max\(0\.0f,\s*Duration\)") "Expected controller to cache clamped combat announcement duration."
Assert-True ($controllerCpp -match "OnCombatAnnouncementShown\.Broadcast\(NormalizedText,\s*LastCombatAnnouncement\.Duration\)") "Expected controller to broadcast normalized announcement text and cached clamped duration."
Assert-True ($controllerCpp -match "LastCombatAnnouncement\s*=\s*FDBAArenaCombatAnnouncementEntry\(\)") "Expected controller clear to reset cached combat announcement."
Assert-True ($controllerCpp -match "OnCombatAnnouncementCleared\.Broadcast\(\)") "Expected controller to broadcast announcement clear."
Assert-True ($controllerCpp -match "LastCriticalStateHints\.bIsValid\s*=\s*true") "Expected controller to mark cached critical-state valid."
Assert-True ($controllerCpp -match "LastCriticalStateHints\.bLowHP\s*=\s*bLowHP") "Expected controller to cache low-HP state."
Assert-True ($controllerCpp -match "LastCriticalStateHints\.bLowEnergy\s*=\s*bLowEnergy") "Expected controller to cache low-energy state."
Assert-True ($controllerCpp -match "OnCriticalStateHintsChanged\.Broadcast\(LastCriticalStateHints\.bLowHP,\s*LastCriticalStateHints\.bLowEnergy\)") "Expected controller to broadcast cached critical-state hints."
Assert-True ($controllerCpp -match "LastArenaObjectiveState\.bIsValid\s*=\s*true") "Expected controller to mark cached objective state valid."
Assert-True ($controllerCpp -match "UpdateArenaObjective[\s\S]*FText\s+NormalizedObjectiveText[\s\S]*if\s*\(\s*!NormalizeHUDFeedbackText\(ObjectiveText,\s*NormalizedObjectiveText\)\s*\)[\s\S]*return;") "Expected controller to no-op blank objective updates."
Assert-True ($controllerCpp -match "LastArenaObjectiveState\.ObjectiveText\s*=\s*NormalizedObjectiveText") "Expected controller to cache normalized objective text."
Assert-True ($controllerCpp -match "LastArenaObjectiveState\.Progress\s*=\s*FMath::Clamp\(Progress,\s*0\.0f,\s*1\.0f\)") "Expected controller to cache clamped objective progress."
Assert-True ($controllerCpp -match "LastArenaObjectiveState\.bIsCompleted\s*=\s*false") "Expected objective update to clear completion flag."
Assert-True ($controllerCpp -match "OnArenaObjectiveUpdated\.Broadcast\(NormalizedObjectiveText,\s*LastArenaObjectiveState\.Progress\)") "Expected controller to broadcast normalized objective text and cached clamped objective progress."
Assert-True ($controllerCpp -match "LastArenaObjectiveState\.bIsCompleted\s*=\s*true") "Expected objective completion to cache completion flag."
Assert-True ($controllerCpp -match "OnArenaObjectiveCompleted\.Broadcast\(\)") "Expected controller to broadcast objective completion."

Assert-True ($rootHeader -match "HandleControllerCombatAnnouncementShown") "Expected root HUD announcement shown handler."
Assert-True ($rootHeader -match "HandleControllerCriticalStateHintsChanged") "Expected root HUD critical-state handler."
Assert-True ($rootHeader -match "HandleControllerArenaObjectiveUpdated") "Expected root HUD objective update handler."
Assert-True ($rootCpp -match "UDBACombatAnnouncementWidgetBase\.h") "Expected root HUD to include announcement panel type."
Assert-True ($rootCpp -match "UDBACriticalStateHintWidgetBase\.h") "Expected root HUD to include critical-state panel type."
Assert-True ($rootCpp -match "UDBAArenaObjectiveTrackerWidgetBase\.h") "Expected root HUD to include objective tracker type."
Assert-True ($rootCpp -match "OnCombatAnnouncementShown\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown\)") "Expected root HUD to bind announcement shown."
Assert-True ($rootCpp -match "OnCombatAnnouncementShown\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown\)") "Expected root HUD to unbind announcement shown."
Assert-True ($rootCpp -match "GetLastCombatAnnouncement\(\)") "Expected root HUD to inspect cached combat announcement after binding."
Assert-True ($rootCpp -match "LastCombatAnnouncement\.bIsValid") "Expected root HUD to gate cached combat announcement replay."
Assert-True ($rootCpp -match "HandleControllerCombatAnnouncementShown\(LastCombatAnnouncement\.Text,\s*LastCombatAnnouncement\.Duration\)") "Expected root HUD to replay cached combat announcement."
Assert-True ($rootCpp -match "CombatAnnouncement->ShowAnnouncement\(Text,\s*Duration\)") "Expected root HUD to forward announcement shown."
Assert-True ($rootCpp -match "CombatAnnouncement->ClearAnnouncement\(\)") "Expected root HUD to forward announcement clear."
Assert-True ($rootCpp -match "CriticalStateHint->ShowCriticalHP\(bLowHP\)") "Expected root HUD to forward low-HP state."
Assert-True ($rootCpp -match "CriticalStateHint->ShowCriticalEnergy\(bLowEnergy\)") "Expected root HUD to forward low-energy state."
Assert-True ($rootCpp -match "GetLastCriticalStateHints\(\)") "Expected root HUD to inspect cached critical state after binding."
Assert-True ($rootCpp -match "LastCriticalStateHints\.bIsValid") "Expected root HUD to gate cached critical-state replay."
Assert-True ($rootCpp -match "HandleControllerCriticalStateHintsChanged\(LastCriticalStateHints\.bLowHP,\s*LastCriticalStateHints\.bLowEnergy\)") "Expected root HUD to replay cached critical-state hints."
Assert-True ($rootCpp -match "ObjectiveTracker->UpdateObjective\(ObjectiveText,\s*Progress\)") "Expected root HUD to forward objective update."
Assert-True ($rootCpp -match "ObjectiveTracker->CompleteObjective\(\)") "Expected root HUD to forward objective completion."
Assert-True ($rootCpp -match "GetLastArenaObjectiveState\(\)") "Expected root HUD to inspect cached objective state after binding."
Assert-True ($rootCpp -match "LastArenaObjectiveState\.bIsValid") "Expected root HUD to gate cached objective replay."
Assert-True ($rootCpp -match "HandleControllerArenaObjectiveUpdated\(LastArenaObjectiveState\.ObjectiveText,\s*LastArenaObjectiveState\.Progress\)") "Expected root HUD to replay cached objective update."
Assert-True ($rootCpp -match "LastArenaObjectiveState\.bIsCompleted") "Expected root HUD to replay cached objective completion."

Assert-True ($announcementHeader -match "BP_OnAnnouncementCleared") "Expected announcement clear BP event."
Assert-True ($announcementCpp -match "NormalizeHUDWidgetText[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected announcement widget to normalize and reject blank HUD text."
Assert-True ($announcementCpp -match "ShowAnnouncement[\s\S]*FText\s+NormalizedText[\s\S]*if\s*\(\s*!NormalizeHUDWidgetText\(Text,\s*NormalizedText\)\s*\)[\s\S]*return;") "Expected announcement widget to no-op blank announcements."
Assert-True ($announcementHeader -match "CachedAnnouncementText") "Expected announcement widget to cache announcement text."
Assert-True ($announcementHeader -match "CachedAnnouncementDuration") "Expected announcement widget to cache announcement duration."
Assert-True ($announcementHeader -match "bCachedAnnouncementVisible") "Expected announcement widget to cache announcement visibility."
Assert-True ($announcementCpp -match "NativeConstruct[\s\S]{0,300}if\s*\(\s*bCachedAnnouncementVisible\s*&&\s*!CachedAnnouncementText\.IsEmpty\(\)\s*\)[\s\S]{0,220}BP_OnAnnouncementShown\(CachedAnnouncementText,\s*CachedAnnouncementDuration\)") "Expected announcement widget NativeConstruct to replay visible cached announcement after widget construction."
Assert-True ($announcementCpp -match "NativeConstruct[\s\S]{0,520}else[\s\S]{0,180}BP_OnAnnouncementCleared\(\)") "Expected announcement widget NativeConstruct to replay cleared announcement state after widget construction."
Assert-True ($announcementCpp -match "CachedAnnouncementText\s*=\s*NormalizedText") "Expected announcement widget to cache normalized announcement text."
Assert-True ($announcementCpp -match "CachedAnnouncementDuration\s*=\s*FMath::Max\(0\.0f,\s*Duration\)") "Expected announcement widget to cache non-negative announcement duration."
Assert-True ($announcementCpp -match "bCachedAnnouncementVisible\s*=\s*true") "Expected announcement widget to cache visible announcement state."
Assert-True ($announcementCpp -match "BP_OnAnnouncementShown\(CachedAnnouncementText,\s*CachedAnnouncementDuration\)") "Expected announcement widget to fire shown BP event with cached text and duration."
Assert-True ($announcementCpp -match "CachedAnnouncementText\s*=\s*FText::GetEmpty\(\)") "Expected announcement clear to reset cached text."
Assert-True ($announcementCpp -match "CachedAnnouncementDuration\s*=\s*0\.0f") "Expected announcement clear to reset cached duration."
Assert-True ($announcementCpp -match "bCachedAnnouncementVisible\s*=\s*false") "Expected announcement clear to cache hidden state."
Assert-True ($announcementCpp -match "BP_OnAnnouncementCleared\(\)") "Expected announcement widget to fire clear BP event."
Assert-True ($criticalHeader -match "CachedLowHP") "Expected critical-state widget to cache low HP."
Assert-True ($criticalHeader -match "CachedLowEnergy") "Expected critical-state widget to cache low energy."
Assert-True ($criticalCpp -match "NativeConstruct[\s\S]{0,180}BP_OnCriticalStateChanged\(CachedLowHP,\s*CachedLowEnergy\)") "Expected critical-state widget NativeConstruct to replay cached state after widget construction."
Assert-True ($criticalCpp -match "CachedLowHP\s*=\s*bLowHP") "Expected critical-state widget to cache HP state."
Assert-True ($criticalCpp -match "CachedLowEnergy\s*=\s*bLowEnergy") "Expected critical-state widget to cache energy state."
Assert-True ($criticalCpp -match "BP_OnCriticalStateChanged\(CachedLowHP,\s*CachedLowEnergy\)") "Expected critical-state widget to fire cached BP event."
Assert-True ($objectiveCpp -match "NormalizeHUDWidgetText[\s\S]*TrimStartAndEnd\(\)[\s\S]*IsEmpty\(\)") "Expected objective tracker widget to normalize and reject blank HUD text."
Assert-True ($objectiveCpp -match "UpdateObjective[\s\S]*FText\s+NormalizedObjectiveText[\s\S]*if\s*\(\s*!NormalizeHUDWidgetText\(ObjectiveText,\s*NormalizedObjectiveText\)\s*\)[\s\S]*return;") "Expected objective tracker widget to no-op blank objective updates."
Assert-True ($objectiveHeader -match "CachedObjectiveText") "Expected objective tracker widget to cache objective text."
Assert-True ($objectiveHeader -match "CachedObjectiveProgress") "Expected objective tracker widget to cache objective progress."
Assert-True ($objectiveHeader -match "bCachedObjectiveCompleted") "Expected objective tracker widget to cache objective completion state."
Assert-True ($objectiveCpp -match "NativeConstruct[\s\S]{0,260}if\s*\(\s*!CachedObjectiveText\.IsEmpty\(\)\s*\)[\s\S]{0,260}BP_OnObjectiveUpdated\(CachedObjectiveText,\s*CachedObjectiveProgress\)") "Expected objective tracker NativeConstruct to replay cached objective update after widget construction."
Assert-True ($objectiveCpp -match "NativeConstruct[\s\S]{0,420}if\s*\(\s*bCachedObjectiveCompleted\s*\)[\s\S]{0,160}BP_OnObjectiveCompleted\(\)") "Expected objective tracker NativeConstruct to replay cached objective completion after widget construction."
Assert-True ($objectiveCpp -match "CachedObjectiveText\s*=\s*NormalizedObjectiveText") "Expected objective tracker to cache normalized objective text."
Assert-True ($objectiveCpp -match "CachedObjectiveProgress\s*=\s*FMath::Clamp\(Progress,\s*0\.0f,\s*1\.0f\)") "Expected objective tracker to cache clamped objective progress."
Assert-True ($objectiveCpp -match "bCachedObjectiveCompleted\s*=\s*false") "Expected objective update to clear cached completion state."
Assert-True ($objectiveCpp -match "BP_OnObjectiveUpdated\(CachedObjectiveText,\s*CachedObjectiveProgress\)") "Expected objective tracker to fire cached update BP event."
Assert-True ($objectiveCpp -match "bCachedObjectiveCompleted\s*=\s*true") "Expected objective completion to cache completion state."
Assert-True ($objectiveCpp -match "BP_OnObjectiveCompleted\(\)") "Expected objective tracker to fire completion BP event."

Assert-True ($managerHeader -match "ShowArenaHUDCombatAnnouncement") "Expected UI manager announcement entrypoint."
Assert-True ($managerHeader -match "UpdateArenaHUDCriticalStateHints") "Expected UI manager critical-state entrypoint."
Assert-True ($managerHeader -match "UpdateArenaHUDObjective") "Expected UI manager objective update entrypoint."
Assert-True ($managerCpp -match "Controller->ShowCombatAnnouncement\(Text,\s*Duration\)") "Expected UI manager to push announcement."
Assert-True ($managerCpp -match "Controller->UpdateCriticalStateHints\(bLowHP,\s*bLowEnergy\)") "Expected UI manager to push critical-state hints."
Assert-True ($managerCpp -match "Controller->UpdateArenaObjective\(ObjectiveText,\s*Progress\)") "Expected UI manager to push objective update."
Assert-True ($managerCpp -match "Controller->CompleteArenaObjective\(\)") "Expected UI manager to push objective completion."
Assert-True ($announcementTest -match "CombatAnnouncementCachesLatestEntry") "Expected automation coverage for cached combat announcements."
Assert-True ($announcementTest -match "ShowCombatAnnouncement\(FText::FromString\(TEXT\(`"   `"\)") "Expected automation coverage for blank combat announcements."
Assert-True ($announcementTest -match "GetLastCombatAnnouncement") "Expected automation test to exercise cached announcement getter."
Assert-True ($announcementTest -match ([regex]::Escape($announcementText))) "Expected automation test to verify cached announcement text."
Assert-True ($announcementTest -match "ClearCombatAnnouncement\(\)") "Expected automation test to verify cached announcement reset."
Assert-True ($objectiveTest -match "ObjectiveCachesLatestState") "Expected automation coverage for cached objective state."
Assert-True ($objectiveTest -match "UpdateArenaObjective\(FText::FromString\(TEXT\(`"   `"\)") "Expected automation coverage for blank objective updates."
Assert-True ($objectiveTest -match "GetLastArenaObjectiveState") "Expected automation test to exercise cached objective getter."
Assert-True ($objectiveTest -match ([regex]::Escape($objectiveText))) "Expected automation test to verify cached objective text."
Assert-True ($objectiveTest -match "CompleteArenaObjective\(\)") "Expected automation test to verify cached objective completion."
Assert-True ($criticalStateTest -match "CriticalStateCachesLatestState") "Expected automation coverage for cached critical state."
Assert-True ($criticalStateTest -match "GetLastCriticalStateHints") "Expected automation test to exercise cached critical-state getter."
Assert-True ($criticalStateTest -match "UpdateCriticalStateHints\(true,\s*false\)") "Expected automation test to verify cached critical-state update."
Assert-True ($criticalStateTest -match "UpdateCriticalStateHints\(false,\s*false\)") "Expected automation test to verify cached critical-state reset."

Write-Host "PASS: Arena HUD event feedback sync contract" -ForegroundColor Green
