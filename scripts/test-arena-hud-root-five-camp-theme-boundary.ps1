<#
Validates Arena HUD root FiveCamp theme boundary handling.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
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

$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath

Assert-True ($cpp.Contains('#include "GameCore/Types/DBACommonEnums.h"')) "Expected Arena HUD root implementation to include FiveCamp enum definitions."
Assert-True ($cpp -match "ApplyFiveCampTheme[\s\S]*static_cast<uint8>\(EDBAFiveCamp::None\)[\s\S]*static_cast<uint8>\(EDBAFiveCamp::Center\)") "Expected Arena HUD root ApplyFiveCampTheme to derive valid FiveCamp bounds from EDBAFiveCamp."
Assert-True ($cpp -match "ApplyFiveCampTheme[\s\S]*NormalizedFiveCamp\s*=\s*FMath::Clamp\(\s*FiveCamp,[\s\S]*static_cast<uint8>\(EDBAFiveCamp::None\),[\s\S]*static_cast<uint8>\(EDBAFiveCamp::Center\)\)") "Expected Arena HUD root ApplyFiveCampTheme to clamp direct FiveCamp input to the valid presentation-camp range."
Assert-True ($cpp -match "BP_OnApplyFiveCampTheme\(NormalizedFiveCamp\)") "Expected Arena HUD root ApplyFiveCampTheme to broadcast normalized FiveCamp value."

Write-Host "PASS: Arena HUD root FiveCamp theme boundary contract" -ForegroundColor Green
