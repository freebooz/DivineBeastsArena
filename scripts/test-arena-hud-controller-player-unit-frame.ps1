<#
Validates Arena HUD controller ownership and synchronization of PlayerUnitFrame controller.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$header = Get-Content -Raw -Encoding UTF8 -LiteralPath $headerPath
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath

Assert-True ($header -match "class\s+UDBAPlayerUnitFrameWidgetController;") "Expected Arena HUD controller to forward declare PlayerUnitFrame controller."
Assert-True ($header -match "GetPlayerUnitFrameWidgetController\s*\(") "Expected Arena HUD controller to expose PlayerUnitFrame controller getter."
Assert-True ($header -match "SetPlayerUnitFrameWidgetController\s*\(") "Expected Arena HUD controller to expose PlayerUnitFrame controller setter."
Assert-True ($header -match "InitializeController\s*\(\s*APlayerController\*\s+InPlayerController\s*\)") "Expected Arena HUD controller to override InitializeController(APlayerController* InPlayerController)."
Assert-True ($header -match "UpdatePlayerLevel\s*\(") "Expected Arena HUD controller to expose PlayerUnitFrame level update."
Assert-True ($header -match "TObjectPtr<\s*UDBAPlayerUnitFrameWidgetController\s*>\s+PlayerUnitFrameWidgetController") "Expected Arena HUD controller to retain PlayerUnitFrame controller."
Assert-True ($header -match "int32\s+CurrentLevel") "Expected Arena HUD controller to retain current level."

Assert-True ($cpp.Contains('#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"')) "Expected Arena HUD controller implementation to include PlayerUnitFrame controller header."
Assert-True ($cpp -match "NewObject<\s*UDBAPlayerUnitFrameWidgetController\s*>\s*\(\s*this\s*\)") "Expected getter to create PlayerUnitFrame controller with Arena HUD controller as outer."
Assert-True ($cpp -match "SetPlayerUnitFrameWidgetController\s*\(") "Expected setter implementation."
Assert-True ($cpp -match "void\s+UDBAArenaHUDWidgetController::InitializeController\s*\(\s*APlayerController\*\s+InPlayerController\s*\)") "Expected Arena HUD controller InitializeController override."
Assert-True ($cpp -match "Super::InitializeController\(InPlayerController\)") "Expected Arena HUD controller to preserve base InitializeController."
Assert-True ($cpp -match "PlayerUnitFrameWidgetController->SetOwningPlayerController\(GetPlayerController\(\)\)") "Expected Arena HUD controller to propagate owning player controller to PlayerUnitFrame controller."
Assert-True ($cpp -match "PlayerUnitFrameWidgetController->InitializeController\(\)") "Expected setter to initialize PlayerUnitFrame controller."
Assert-True ($cpp -match "PlayerUnitFrameWidgetController->SetVitals\(CurrentHP,\s*MaxHP,\s*CurrentEnergy,\s*MaxEnergy\)") "Expected setter and updates to sync vitals."
Assert-True ($cpp -match "PlayerUnitFrameWidgetController->SetCurrentLevel\(CurrentLevel\)") "Expected setter and updates to sync level."
Assert-True ($cpp -match "void\s+UDBAArenaHUDWidgetController::UpdatePlayerLevel") "Expected UpdatePlayerLevel implementation."

Assert-True ($rootCpp.Contains('#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"')) "Expected Arena HUD root implementation to include Arena HUD controller header."
Assert-True ($rootCpp -match "SetPlayerUnitFrameWidgetController\(WidgetController\s*\?\s*WidgetController->GetPlayerUnitFrameWidgetController\(\)\s*:\s*nullptr\)") "Expected Arena HUD root SetWidgetController to forward main controller child controller."

Write-Host "PASS: Arena HUD controller PlayerUnitFrame contract" -ForegroundColor Green
