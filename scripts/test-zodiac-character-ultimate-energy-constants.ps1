<#
Validates Zodiac character fallback UltimateEnergy logic uses DBAConstants instead of magic 100.0f thresholds.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$headerPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

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

Assert-True ($cpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Zodiac character implementation to include DBAConstants."
Assert-True ($header -match "bool\s+IsUltimateReady\s*\(\s*\)\s+const\s*;") "Expected IsUltimateReady to be implemented in cpp instead of inline hard-coded header logic."
Assert-True ($header -notmatch "IsUltimateReady\s*\(\s*\)\s*const\s*\{[^}]*100\.0f") "Expected header IsUltimateReady to avoid hard-coded 100.0f."
Assert-True ($cpp -match "ADBAZodiacCharacterBase::IsUltimateReady\s*\(\s*\)\s*const") "Expected cpp IsUltimateReady implementation."
Assert-True ($cpp -match "IsUltimateReady[\s\S]*GetUltimateEnergy\(\)\s*>=\s*DBAConstants::MaxUltimateEnergy") "Expected IsUltimateReady to read through GetUltimateEnergy and DBAConstants::MaxUltimateEnergy."
Assert-True ($cpp -match "SetUltimateEnergy[\s\S]*FMath::Clamp\(Value,\s*0\.0f,\s*DBAConstants::MaxUltimateEnergy\)") "Expected SetUltimateEnergy to clamp target with DBAConstants::MaxUltimateEnergy."
Assert-True ($cpp -match "AddUltimateEnergy[\s\S]*ASC->AddUltimateEnergy\(Delta\)") "Expected AddUltimateEnergy to delegate to ASC."
Assert-True ($cpp -notmatch "UltimateEnergy\s*=\s*FMath::Clamp\([^;]*100\.0f") "Expected UltimateEnergy logic to avoid hard-coded 100.0f writes."

Write-Host "PASS: Zodiac character UltimateEnergy constants contract" -ForegroundColor Green
