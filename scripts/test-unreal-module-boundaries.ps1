<#
Runs fixture-based tests for scripts/validate-unreal-module-boundaries.ps1.
Run from the repository root:
  .\scripts\test-unreal-module-boundaries.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$validator = Join-Path -Path $repoRoot -ChildPath "scripts\validate-unreal-module-boundaries.ps1"
$fixtureRoot = Join-Path -Path $repoRoot -ChildPath (".tmp\unreal-module-boundary-fixtures\{0}" -f [guid]::NewGuid().ToString("N"))

function New-ModuleFixture {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [string]$GameCoreBuildExtra = "",
    [string]$GameMobaBuildExtra = "",
    [string]$GameCoreSource = '#include "GameCore/Account/DBAAccountSaveGame.h"',
    [string]$GameMobaSource = '#include "GameCore/Account/DBAAccountSaveGame.h"'
  )

  $sourceRoot = Join-Path -Path $fixtureRoot -ChildPath "$Name\DBA_GameClient\Source"
  $gameCoreRoot = Join-Path -Path $sourceRoot -ChildPath "GameCore"
  $gameMobaRoot = Join-Path -Path $sourceRoot -ChildPath "GameMoba"
  $arenaRoot = Join-Path -Path $sourceRoot -ChildPath "DivineBeastsArena"

  New-Item -ItemType Directory -Force -Path `
    (Join-Path -Path $gameCoreRoot -ChildPath "Private"), `
    (Join-Path -Path $gameMobaRoot -ChildPath "Private"), `
    (Join-Path -Path $arenaRoot -ChildPath "Private") | Out-Null

  Set-Content -LiteralPath (Join-Path -Path $gameCoreRoot -ChildPath "GameCore.Build.cs") -Encoding UTF8 -Value @"
using UnrealBuildTool;

public class GameCore : ModuleRules
{
  public GameCore(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivateDependencyModuleNames.AddRange(new string[] { "Core"$GameCoreBuildExtra });
  }
}
"@

  Set-Content -LiteralPath (Join-Path -Path $gameMobaRoot -ChildPath "GameMoba.Build.cs") -Encoding UTF8 -Value @"
using UnrealBuildTool;

public class GameMoba : ModuleRules
{
  public GameMoba(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivateDependencyModuleNames.AddRange(new string[] { "Core", "GameCore"$GameMobaBuildExtra });
  }
}
"@

  Set-Content -LiteralPath (Join-Path -Path $arenaRoot -ChildPath "DivineBeastsArena.Build.cs") -Encoding UTF8 -Value @"
using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
  public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
  {
    PrivateDependencyModuleNames.AddRange(new string[] { "Core", "GameCore", "GameMoba" });
  }
}
"@

  Set-Content -LiteralPath (Join-Path -Path $gameCoreRoot -ChildPath "Private\Fixture.cpp") -Encoding UTF8 -Value $GameCoreSource
  Set-Content -LiteralPath (Join-Path -Path $gameMobaRoot -ChildPath "Private\Fixture.cpp") -Encoding UTF8 -Value $GameMobaSource

  return $sourceRoot
}

function Invoke-ExpectFailure {
  param(
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string]$SourceRoot,
    [Parameter(Mandatory = $true)][string]$ExpectedMessage
  )

  try {
    & $validator -ClientSourceRoot $SourceRoot
  }
  catch {
    if ($_.Exception.Message -notlike "*$ExpectedMessage*") {
      throw "$Name failed for the wrong reason: $($_.Exception.Message)"
    }

    Write-Host "PASS: $Name failed as expected" -ForegroundColor Green
    return
  }

  throw "$Name should have failed."
}

if (Test-Path -LiteralPath $fixtureRoot) {
  Remove-Item -LiteralPath $fixtureRoot -Recurse -Force
}

$validRoot = New-ModuleFixture -Name "valid"
& $validator -ClientSourceRoot $validRoot
Write-Host "PASS: valid module boundary fixture" -ForegroundColor Green

$badGameCoreDependencyRoot = New-ModuleFixture -Name "bad-gamecore-dependency" -GameCoreBuildExtra ', "GameMoba"'
Invoke-ExpectFailure `
  -Name "GameCore reverse dependency fixture" `
  -SourceRoot $badGameCoreDependencyRoot `
  -ExpectedMessage "GameCore.Build.cs must not depend on GameMoba"

$badGameMobaDependencyRoot = New-ModuleFixture -Name "bad-gamemoba-dependency" -GameMobaBuildExtra ', "DivineBeastsArena"'
Invoke-ExpectFailure `
  -Name "GameMoba reverse dependency fixture" `
  -SourceRoot $badGameMobaDependencyRoot `
  -ExpectedMessage "GameMoba.Build.cs must not depend on DivineBeastsArena"

$badQuotedIncludeRoot = New-ModuleFixture -Name "bad-quoted-include" -GameCoreSource '#include "GameMoba/Combat/DBACombatRules.h"'
Invoke-ExpectFailure `
  -Name "quoted include boundary fixture" `
  -SourceRoot $badQuotedIncludeRoot `
  -ExpectedMessage "GameCore source must not include GameMoba"

$badAngleIncludeRoot = New-ModuleFixture -Name "bad-angle-include" -GameMobaSource '#include <GameDBA/UI/DBAGameUIManager.h>'
Invoke-ExpectFailure `
  -Name "angle include boundary fixture" `
  -SourceRoot $badAngleIncludeRoot `
  -ExpectedMessage "GameMoba source must not include DivineBeastsArena headers through angle brackets"

$badParentIncludeRoot = New-ModuleFixture -Name "bad-parent-include" -GameCoreSource '#include "../Private/Hidden.h"'
Invoke-ExpectFailure `
  -Name "parent include fixture" `
  -SourceRoot $badParentIncludeRoot `
  -ExpectedMessage "Parent-directory includes are not allowed"

Write-Host "PASS: Unreal module boundary fixtures" -ForegroundColor Green
