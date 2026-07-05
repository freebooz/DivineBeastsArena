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

$abilityCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacUltimateAbilityBase.cpp"
$abilityHeaderPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacUltimateAbilityBase.h"
$genericHeaderPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacUltimateAbility_Generic.h"
$rpcCppPath = Join-Path $RepoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"

$abilityCpp = Get-Content -Raw -Encoding UTF8 $abilityCppPath
$abilityHeader = Get-Content -Raw -Encoding UTF8 $abilityHeaderPath
$genericHeader = Get-Content -Raw -Encoding UTF8 $genericHeaderPath
$rpcCpp = Get-Content -Raw -Encoding UTF8 $rpcCppPath

Assert-True ($abilityCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected Zodiac ultimate ability implementation to include DBAConstants."
Assert-True ($abilityCpp -match "HasEnoughUltimateEnergy\(DBAConstants::MaxUltimateEnergy\)") "Expected Zodiac ultimate activation check to use DBAConstants::MaxUltimateEnergy."
Assert-True ($abilityCpp -match "ConsumeUltimateEnergy\(DBAConstants::MaxUltimateEnergy\)") "Expected Zodiac ultimate cost commit to use DBAConstants::MaxUltimateEnergy."
Assert-True ($abilityCpp -notmatch "UltimateEnergy[\s\S]{0,180}100\.0f") "Expected Zodiac ultimate ability implementation to avoid hard-coded 100.0f near UltimateEnergy logic."

Assert-True ($rpcCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected RPC handler to include DBAConstants."
Assert-True ($rpcCpp -match "GetUltimateEnergy\(\)\s*>=\s*DBAConstants::MaxUltimateEnergy") "Expected RPC ultimate implementation gate to use DBAConstants::MaxUltimateEnergy."
Assert-True ($rpcCpp -match "GetUltimateEnergy\(\)\s*<\s*DBAConstants::MaxUltimateEnergy") "Expected RPC ultimate validation gate to use DBAConstants::MaxUltimateEnergy."
Assert-True ($rpcCpp -notmatch "GetUltimateEnergy\(\)\s*[<>]=?\s*100\.?f") "Expected RPC ultimate gates to avoid hard-coded 100.f thresholds."

Assert-True ($abilityHeader -notmatch "100\s*点\s*UltimateEnergy|达到\s*100") "Expected Zodiac ultimate ability comments to avoid hard-coded 100-point wording."
Assert-True ($genericHeader -notmatch "100\s*点\s*UltimateEnergy") "Expected generic Zodiac ultimate comments to avoid hard-coded 100-point wording."

Write-Host "PASS: Zodiac ultimate energy cost constants contract" -ForegroundColor Green
