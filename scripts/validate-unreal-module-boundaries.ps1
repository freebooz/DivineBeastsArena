<#
Validates Unreal module dependency direction and source-level include boundaries.
Run from the repository root:
  .\scripts\validate-unreal-module-boundaries.ps1
#>

[CmdletBinding()]
param(
  [string]$ClientSourceRoot = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$clientSourceRoot = if ([string]::IsNullOrWhiteSpace($ClientSourceRoot)) {
  Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source"
}
else {
  (Resolve-Path -LiteralPath $ClientSourceRoot).ProviderPath
}
$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
  param([Parameter(Mandatory = $true)][string]$Message)
  $failures.Add($Message)
}

function Get-DependencyModules {
  param(
    [Parameter(Mandatory = $true)][string]$BuildFilePath
  )

  $content = Get-Content -LiteralPath $BuildFilePath -Encoding UTF8 -Raw
  $matches = [regex]::Matches(
    $content,
    '(PublicDependencyModuleNames|PrivateDependencyModuleNames)\.AddRange\s*\(\s*new string\[\]\s*\{(?<body>.*?)\}\s*\)',
    [System.Text.RegularExpressions.RegexOptions]::Singleline
  )

  $modules = New-Object System.Collections.Generic.HashSet[string]
  foreach ($match in $matches) {
    $stringMatches = [regex]::Matches($match.Groups["body"].Value, '"(?<name>[^"]+)"')
    foreach ($stringMatch in $stringMatches) {
      [void]$modules.Add($stringMatch.Groups["name"].Value)
    }
  }

  return $modules
}

function Test-BuildDependencies {
  $gameCoreBuild = Join-Path -Path $clientSourceRoot -ChildPath "GameCore\GameCore.Build.cs"
  $gameMobaBuild = Join-Path -Path $clientSourceRoot -ChildPath "GameMoba\GameMoba.Build.cs"
  $arenaBuild = Join-Path -Path $clientSourceRoot -ChildPath "DivineBeastsArena\DivineBeastsArena.Build.cs"

  foreach ($requiredFile in @($gameCoreBuild, $gameMobaBuild, $arenaBuild)) {
    if (-not (Test-Path -LiteralPath $requiredFile)) {
      Add-Failure "Missing Unreal module build file: $requiredFile"
      return
    }
  }

  $gameCoreDeps = Get-DependencyModules -BuildFilePath $gameCoreBuild
  $gameMobaDeps = Get-DependencyModules -BuildFilePath $gameMobaBuild
  $arenaDeps = Get-DependencyModules -BuildFilePath $arenaBuild

  foreach ($forbidden in @("GameMoba", "DivineBeastsArena")) {
    if ($gameCoreDeps.Contains($forbidden)) {
      Add-Failure "GameCore.Build.cs must not depend on $forbidden."
    }
  }

  if (-not $gameMobaDeps.Contains("GameCore")) {
    Add-Failure "GameMoba.Build.cs must depend on GameCore."
  }
  if ($gameMobaDeps.Contains("DivineBeastsArena")) {
    Add-Failure "GameMoba.Build.cs must not depend on DivineBeastsArena."
  }

  foreach ($required in @("GameCore", "GameMoba")) {
    if (-not $arenaDeps.Contains($required)) {
      Add-Failure "DivineBeastsArena.Build.cs should include $required for the current three-layer mapping."
    }
  }
}

function Test-SourceIncludeBoundaries {
  $rules = @(
    @{
      Name = "GameCore"
      Path = (Join-Path -Path $clientSourceRoot -ChildPath "GameCore")
      Pattern = '#include\s+[\x22](GameMoba/|GameDBA/|DivineBeastsArena\.h)'
      Message = "GameCore source must not include GameMoba or DivineBeastsArena headers."
    },
    @{
      Name = "GameMoba"
      Path = (Join-Path -Path $clientSourceRoot -ChildPath "GameMoba")
      Pattern = '#include\s+[\x22](GameDBA/|DivineBeastsArena\.h)'
      Message = "GameMoba source must not include DivineBeastsArena headers."
    }
  )

  foreach ($rule in $rules) {
    if (-not (Test-Path -LiteralPath $rule.Path)) {
      Add-Failure "Missing Unreal module source path: $($rule.Path)"
      continue
    }

    $matches = & rg -n --pcre2 -g "*.h" -g "*.cpp" -e $rule.Pattern -- $rule.Path
    if ($LASTEXITCODE -gt 1) {
      throw "rg include boundary scan failed for $($rule.Name) with code $LASTEXITCODE"
    }

    if ($matches) {
      Add-Failure "$($rule.Message)`n$($matches -join "`n")"
    }
  }
}

function Test-NoParentDirectoryIncludes {
  $sourceRoot = Join-Path -Path $clientSourceRoot -ChildPath "."
  if (-not (Test-Path -LiteralPath $sourceRoot)) {
    Add-Failure "Missing Unreal source path: $sourceRoot"
    return
  }

  $matches = & rg -n -g "*.h" -g "*.cpp" -e '#include\s+[<\x22][^>\x22]*\.\./' -- $sourceRoot
  if ($LASTEXITCODE -gt 1) {
    throw "rg parent include scan failed with code $LASTEXITCODE"
  }

  if ($matches) {
    Add-Failure "Parent-directory includes are not allowed in Unreal source; use module Public headers instead:`n$($matches -join "`n")"
  }
}

function Test-AngleBracketLayerIncludes {
  $rules = @(
    @{
      Name = "GameCore"
      Path = (Join-Path -Path $clientSourceRoot -ChildPath "GameCore")
      Pattern = '#include\s+<((GameMoba|GameDBA)/|DivineBeastsArena\.h)'
      Message = "GameCore source must not include GameMoba or DivineBeastsArena headers through angle brackets."
    },
    @{
      Name = "GameMoba"
      Path = (Join-Path -Path $clientSourceRoot -ChildPath "GameMoba")
      Pattern = '#include\s+<((GameDBA)/|DivineBeastsArena\.h)'
      Message = "GameMoba source must not include DivineBeastsArena headers through angle brackets."
    }
  )

  foreach ($rule in $rules) {
    if (-not (Test-Path -LiteralPath $rule.Path)) {
      Add-Failure "Missing Unreal module source path: $($rule.Path)"
      continue
    }

    $matches = & rg -n --pcre2 -g "*.h" -g "*.cpp" -e $rule.Pattern -- $rule.Path
    if ($LASTEXITCODE -gt 1) {
      throw "rg angle-bracket include boundary scan failed for $($rule.Name) with code $LASTEXITCODE"
    }

    if ($matches) {
      Add-Failure "$($rule.Message)`n$($matches -join "`n")"
    }
  }
}

Set-Location $repoRoot

Test-BuildDependencies
Test-SourceIncludeBoundaries
Test-NoParentDirectoryIncludes
Test-AngleBracketLayerIncludes

if ($failures.Count -gt 0) {
  foreach ($failure in $failures) {
    Write-Host "FAIL: $failure" -ForegroundColor Red
  }
  throw "Unreal module boundary validation failed: $($failures -join '; ')"
}

Write-Host "PASS: Unreal module boundaries" -ForegroundColor Green
