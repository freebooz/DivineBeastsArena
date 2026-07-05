param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Assert-True {
    param(
        [bool]$Condition,
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$headerPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\Overhead\DBAOverheadWidgetComponent.h"
$cppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\Overhead\DBAOverheadWidgetComponent.cpp"

$header = Get-Content -Raw -Path $headerPath
$cpp = Get-Content -Raw -Path $cppPath

Assert-True ($header -match "FText\s+CachedCharacterName") "Expected OverheadWidgetComponent to cache character name before widget creation."
Assert-True ($header -match "bool\s+bCachedOverheadVisible") "Expected OverheadWidgetComponent to cache visibility before widget creation."
Assert-True ($header -match "void\s+ApplyWidgetConfig\(\)") "Expected OverheadWidgetComponent to expose C++ widget config application."
Assert-True ($header -match "virtual\s+void\s+EndPlay\(\s*const\s+EEndPlayReason::Type\s+EndPlayReason\s*\)\s+override") "Expected OverheadWidgetComponent to clean up viewport widget during EndPlay."
Assert-True ($header -match "virtual\s+void\s+TickComponent\(\s*float\s+DeltaTime,\s*ELevelTick\s+TickType,\s*FActorComponentTickFunction\*\s+ThisTickFunction\s*\)\s+override") "Expected OverheadWidgetComponent to tick so overhead UI tracks moving actors."
Assert-True ($cpp -match "BeginPlay[\s\S]{0,320}GetWorld\(\)[\s\S]{0,260}GetNetMode\(\)\s*==\s*NM_DedicatedServer[\s\S]{0,180}SetComponentTickEnabled\(false\)[\s\S]{0,120}return") "Expected BeginPlay to disable overhead ticking on Dedicated Server."
Assert-True ($cpp -match "CreateOverheadWidget[\s\S]{0,260}GetWorld\(\)[\s\S]{0,260}GetNetMode\(\)\s*==\s*NM_DedicatedServer[\s\S]{0,120}return") "Expected CreateOverheadWidget to skip viewport UI creation on Dedicated Server."
Assert-True ($cpp -match "CreateOverheadWidget[\s\S]{0,900}SetHealthBarPercent\(CachedHealthPercent\)[\s\S]{0,260}SetCharacterName\(CachedCharacterName\)[\s\S]{0,260}SetOverheadVisible\(bCachedOverheadVisible\)[\s\S]{0,260}ApplyWidgetConfig\(\)") "Expected CreateOverheadWidget to replay cached health, name, visibility, and config."
Assert-True ($cpp -match "ApplyWidgetConfig[\s\S]{0,260}HealthBar[\s\S]{0,260}bShowHealthBar\s*\?\s*ESlateVisibility::HitTestInvisible\s*:\s*ESlateVisibility::Collapsed") "Expected ApplyWidgetConfig to apply bShowHealthBar to the HealthBar widget."
Assert-True ($cpp -match "ApplyWidgetConfig[\s\S]{0,520}NameText[\s\S]{0,260}bShowName\s*\?\s*ESlateVisibility::HitTestInvisible\s*:\s*ESlateVisibility::Collapsed") "Expected ApplyWidgetConfig to apply bShowName to the NameText widget."
Assert-True ($cpp -match "EndPlay[\s\S]{0,260}if\s*\(OverheadWidget\)[\s\S]{0,180}OverheadWidget->RemoveFromParent\(\)[\s\S]{0,180}OverheadWidget\s*=\s*nullptr[\s\S]{0,220}Super::EndPlay\(EndPlayReason\)") "Expected EndPlay to remove and clear the viewport widget before calling Super."
Assert-True ($cpp -match "TickComponent[\s\S]{0,260}Super::TickComponent\(DeltaTime,\s*TickType,\s*ThisTickFunction\)[\s\S]{0,180}UpdateWidgetPosition\(\)") "Expected OverheadWidgetComponent TickComponent to update screen position every tick."
Assert-True ($cpp -match "UpdateWidgetPosition[\s\S]{0,620}ProjectWorldLocationToScreen\(WorldPosition,\s*ScreenPosition\)[\s\S]{0,240}SetOverheadVisible\(bCachedOverheadVisible\)[\s\S]{0,220}SetPositionInViewport\(ScreenPosition,\s*false\)") "Expected UpdateWidgetPosition to restore cached visibility when projection succeeds."
Assert-True ($cpp -match "UpdateWidgetPosition[\s\S]{0,900}else[\s\S]{0,180}OverheadWidget->SetVisibility\(ESlateVisibility::Hidden\)") "Expected UpdateWidgetPosition to hide stale overhead UI when projection fails."
Assert-True ($cpp -match "UpdateWidgetPosition[\s\S]{0,320}UWorld\*\s+World\s*=\s*GetWorld\(\)[\s\S]{0,180}if\s*\(!World\)[\s\S]{0,80}return[\s\S]{0,260}World->GetFirstPlayerController\(\)") "Expected UpdateWidgetPosition to guard missing World before querying the first player controller."
Assert-True ($cpp -match "SetCharacterName[\s\S]{0,120}CachedCharacterName\s*=\s*Name") "Expected SetCharacterName to update cached name even when widget is not available."
Assert-True ($cpp -match "SetCharacterName[\s\S]{0,260}if\s*\(!OverheadWidget\)[\s\S]{0,80}return") "Expected SetCharacterName to guard null OverheadWidget before querying child widgets."
Assert-True ($cpp -match "SetOverheadVisible[\s\S]{0,140}bCachedOverheadVisible\s*=\s*bShouldBeVisible") "Expected SetOverheadVisible to update cached visibility even when widget is not available."
Assert-True ($cpp -match "SetOverheadVisible[\s\S]{0,360}bShouldBeVisible\s*\?\s*ESlateVisibility::HitTestInvisible\s*:\s*ESlateVisibility::Hidden") "Expected visible overhead widgets to stay HitTestInvisible instead of intercepting input."

Write-Host "PASS: Arena OverheadWidgetComponent contract" -ForegroundColor Green
