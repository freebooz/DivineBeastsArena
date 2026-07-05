<#
Validates Arena HUD root handoff of the PlayerUnitFrame controller.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"

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

Assert-True ($header -match "class\s+UDBAPlayerUnitFrameWidgetController;") "Expected Arena HUD root to forward declare PlayerUnitFrame controller."
Assert-True ($header -match "SetPlayerUnitFrameWidgetController\s*\(\s*UDBAPlayerUnitFrameWidgetController\*\s+InController\s*\)") "Expected Arena HUD root to expose PlayerUnitFrame controller handoff."
Assert-True ($header -match "TObjectPtr<\s*UDBAPlayerUnitFrameWidgetController\s*>\s+PlayerUnitFrameWidgetController") "Expected Arena HUD root to retain PlayerUnitFrame controller."

Assert-True ($cpp.Contains('#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetBase.h"')) "Expected Arena HUD root implementation to include PlayerUnitFrame widget header."
Assert-True ($cpp.Contains('#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"')) "Expected Arena HUD root implementation to include PlayerUnitFrame controller header."
Assert-True ($cpp -match "void\s+UDBAArenaHUDRootWidgetBase::SetPlayerUnitFrameWidgetController") "Expected SetPlayerUnitFrameWidgetController implementation."
Assert-True ($cpp -match "PlayerUnitFrameWidgetController\s*=\s*InController") "Expected Arena HUD root to retain incoming PlayerUnitFrame controller."
Assert-True ($cpp -match "if\s*\(\s*PlayerUnitFrame\s*\)") "Expected Arena HUD root to guard optional PlayerUnitFrame widget."
Assert-True ($cpp -match "PlayerUnitFrame->SetWidgetController\(PlayerUnitFrameWidgetController\)") "Expected Arena HUD root to forward controller to PlayerUnitFrame."

Write-Host "PASS: Arena HUD root PlayerUnitFrame handoff contract" -ForegroundColor Green
