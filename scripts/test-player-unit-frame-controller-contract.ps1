<#
Validates the Arena PlayerUnitFrame controller does not expose placeholder
hard-coded HUD values.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetController.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetController.cpp"

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

Assert-True ($header -match "SetVitals\s*\(") "Expected PlayerUnitFrame controller to expose SetVitals."
Assert-True ($header -match "SetCurrentLevel\s*\(") "Expected PlayerUnitFrame controller to expose SetCurrentLevel."
Assert-True ($header -match "SetOwningPlayerController\s*\(\s*APlayerController\*\s+InPlayerController\s*\)") "Expected PlayerUnitFrame controller to expose SetOwningPlayerController(APlayerController* InPlayerController)."
Assert-True ($header -match "GetOwningPlayerController\s*\(\s*\)\s*const") "Expected PlayerUnitFrame controller to expose GetOwningPlayerController() const."
Assert-True ($header -match "CurrentHP") "Expected PlayerUnitFrame controller to store CurrentHP."
Assert-True ($header -match "MaxHP") "Expected PlayerUnitFrame controller to store MaxHP."
Assert-True ($header -match "CurrentEnergy") "Expected PlayerUnitFrame controller to store CurrentEnergy."
Assert-True ($header -match "MaxEnergy") "Expected PlayerUnitFrame controller to store MaxEnergy."
Assert-True ($header -match "CurrentLevel") "Expected PlayerUnitFrame controller to store CurrentLevel."
Assert-True ($header -match "TWeakObjectPtr<class APlayerController>\s+OwningPlayerController") "Expected PlayerUnitFrame controller to retain OwningPlayerController as a weak pointer."

$forbiddenLiterals = @(
    "return 850.0f;",
    "return 1000.0f;",
    "return 70.0f;",
    "return 100.0f;",
    "return 12;"
)

foreach ($literal in $forbiddenLiterals) {
    Assert-True (-not $cpp.Contains($literal)) "PlayerUnitFrame controller still returns placeholder literal: $literal"
}

Assert-True ($cpp -match "CurrentHP\s*=.*InCurrentHP") "Expected SetVitals to update CurrentHP."
Assert-True ($cpp -match "MaxHP\s*=.*InMaxHP") "Expected SetVitals to update MaxHP."
Assert-True ($cpp -match "CurrentEnergy\s*=.*InCurrentEnergy") "Expected SetVitals to update CurrentEnergy."
Assert-True ($cpp -match "MaxEnergy\s*=.*InMaxEnergy") "Expected SetVitals to update MaxEnergy."
Assert-True ($cpp -match "CurrentLevel\s*=.*InLevel") "Expected SetCurrentLevel to update CurrentLevel."
Assert-True ($cpp -match "void\s+UDBAPlayerUnitFrameWidgetController::SetOwningPlayerController\s*\(\s*APlayerController\*\s+InPlayerController\s*\)") "Expected SetOwningPlayerController implementation."
Assert-True ($cpp -match "OwningPlayerController\s*=\s*InPlayerController") "Expected SetOwningPlayerController to store the weak pointer."
Assert-True ($cpp -match "OnHPUpdated\.Broadcast") "Expected SetVitals to broadcast HP updates."
Assert-True ($cpp -match "OnEnergyUpdated\.Broadcast") "Expected SetVitals to broadcast energy updates."
Assert-True ($cpp -match "OnLevelUpdated\.Broadcast") "Expected SetCurrentLevel to broadcast level updates."

Write-Host "PASS: PlayerUnitFrame controller contract" -ForegroundColor Green
