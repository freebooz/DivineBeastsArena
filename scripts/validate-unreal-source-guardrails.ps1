<#
Validates Unreal source guardrails that keep the client, server, and shared modules aligned with project rules.
Run from the repository root:
  .\scripts\validate-unreal-source-guardrails.ps1
#>

[CmdletBinding()]
param(
  [string]$RepoRoot
)

$ErrorActionPreference = "Stop"
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
  (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
}
else {
  (Resolve-Path -LiteralPath $RepoRoot).ProviderPath
}
$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
  param([Parameter(Mandatory = $true)][string]$Message)
  $failures.Add($Message)
}

function Test-LogTempUsage {
  $searchRoots = @(
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source"),
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Plugins")
  )

  $matches = & rg -n "LogTemp" @searchRoots -g "*.h" -g "*.cpp" -g "*.cs"
  if ($LASTEXITCODE -gt 1) {
    throw "rg LogTemp scan failed with code $LASTEXITCODE"
  }

  if ($matches) {
    Add-Failure "LogTemp is not allowed in committed Unreal source:`n$($matches -join "`n")"
  }
}

function Test-NoPrivateInclude {
  $searchRoots = @(
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source"),
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Plugins")
  )

  $matches = & rg -n '#include\s+.*Private' @searchRoots -g "*.h" -g "*.cpp"
  if ($LASTEXITCODE -gt 1) {
    throw "rg Private include scan failed with code $LASTEXITCODE"
  }

  if ($matches) {
    Add-Failure "Including Private directories across modules is not allowed:`n$($matches -join "`n")"
  }
}

function Get-BuildFiles {
  $roots = @(
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source"),
    (Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Plugins")
  )

  foreach ($root in $roots) {
    if (Test-Path -LiteralPath $root) {
      Get-ChildItem -LiteralPath $root -Recurse -Filter "*.Build.cs" -File
    }
  }
}

function Get-NonServerGuardRanges {
  param([Parameter(Mandatory = $true)][string]$Content)

  $ranges = New-Object System.Collections.Generic.List[object]
  $guardMatches = [regex]::Matches($Content, 'if\s*\(\s*Target\.Type\s*!=\s*TargetType\.Server\s*\)')

  foreach ($guardMatch in $guardMatches) {
    $blockStart = $Content.IndexOf("{", $guardMatch.Index)
    if ($blockStart -lt 0) {
      continue
    }

    $depth = 0
    $blockEnd = -1
    for ($i = $blockStart; $i -lt $Content.Length; $i++) {
      if ($Content[$i] -eq "{") {
        $depth++
      }
      elseif ($Content[$i] -eq "}") {
        $depth--
        if ($depth -eq 0) {
          $blockEnd = $i
          break
        }
      }
    }

    if ($blockEnd -ge 0) {
      $ranges.Add([pscustomobject]@{
        Start = $blockStart
        End = $blockEnd
      })
    }
  }

  return $ranges
}

function Test-IsInsideRange {
  param(
    [Parameter(Mandatory = $true)][int]$Index,
    $Ranges
  )

  if ($null -eq $Ranges) {
    return $false
  }

  foreach ($range in $Ranges) {
    if ($Index -ge $range.Start -and $Index -le $range.End) {
      return $true
    }
  }

  return $false
}

function Test-ClientOnlyModuleServerGuards {
  $clientOnlyModules = @("RenderCore", "RHI", "AudioMixer", "MediaAssets")

  foreach ($buildFile in Get-BuildFiles) {
    $content = Get-Content -LiteralPath $buildFile.FullName -Encoding UTF8 -Raw
    $guardRanges = Get-NonServerGuardRanges -Content $content
    $forbiddenOccurrences = New-Object System.Collections.Generic.List[string]

    foreach ($moduleName in $clientOnlyModules) {
      $matches = [regex]::Matches($content, '"' + [regex]::Escape($moduleName) + '"')
      foreach ($match in $matches) {
        if (-not (Test-IsInsideRange -Index $match.Index -Ranges $guardRanges)) {
          $forbiddenOccurrences.Add($moduleName)
        }
      }
    }

    if ($forbiddenOccurrences.Count -gt 0) {
      $relativePath = $buildFile.FullName
      if ($relativePath.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        $relativePath = $relativePath.Substring($repoRoot.Length).TrimStart('\', '/')
      }
      $modules = ($forbiddenOccurrences | Sort-Object -Unique) -join ", "
      Add-Failure "$relativePath client-only Unreal modules must be guarded by Target.Type != TargetType.Server: $modules"
    }
  }
}

function Test-FileContainsAllTokens {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string[]]$Tokens,
    [Parameter(Mandatory = $true)][string]$FailurePrefix
  )

  $fullPath = if ([System.IO.Path]::IsPathRooted($RelativePath)) {
    $RelativePath
  }
  else {
    Join-Path -Path $repoRoot -ChildPath $RelativePath
  }
  if (-not (Test-Path -LiteralPath $fullPath)) {
    Add-Failure "$FailurePrefix`: missing file $RelativePath"
    return
  }

  $content = Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
  $missingTokens = New-Object System.Collections.Generic.List[string]
  foreach ($token in $Tokens) {
    if (-not $content.Contains($token)) {
      $missingTokens.Add($token)
    }
  }

  if ($missingTokens.Count -gt 0) {
    Add-Failure "$FailurePrefix`: missing $($missingTokens -join ', ') in $RelativePath"
  }
}

function Test-CppLogicBlueprintBoundaryPolicy {
  $controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
  if ($null -eq $controlPrompt) {
    Add-Failure "C++ logic / Blueprint boundary policy is incomplete: missing docs\Development\ZodiacArena_UE5_8_Codex_*.md"
    return
  }

  $agentsTokens = @(
    "C++",
    "Gameplay",
    "GAS",
    "Blueprint",
    "DataAsset",
    "VFX",
    "SFX",
    "UPROPERTY",
    "UFUNCTION"
  )
  $controlPromptTokens = @(
    "### 2.1.1",
    "C++",
    "Gameplay",
    "GAS",
    "Blueprint",
    "DataAsset",
    "UPROPERTY",
    "UFUNCTION",
    "Subsystem"
  )

  Test-FileContainsAllTokens `
    -RelativePath "AGENTS.md" `
    -Tokens $agentsTokens `
    -FailurePrefix "C++ logic / Blueprint boundary policy is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath $controlPrompt.FullName `
    -Tokens $controlPromptTokens `
    -FailurePrefix "C++ logic / Blueprint boundary policy is incomplete"
}

function Test-DataAssetNoHardcodingPolicy {
  $controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
  if ($null -eq $controlPrompt) {
    Add-Failure "DataAsset / no-hardcoding policy is incomplete: missing docs\Development\ZodiacArena_UE5_8_Codex_*.md"
    return
  }

  $policyTokens = @(
    'PolicyId: `DBA.DataAsset.NoHardcoding`',
    'PrimaryDataAsset',
    'DataAsset',
    'DataTable',
    'DeveloperSettings',
    'GameplayTag',
    'Asset Manager',
    'UI',
    'VFX',
    'SFX',
    'C++'
  )

  Test-FileContainsAllTokens `
    -RelativePath "AGENTS.md" `
    -Tokens $policyTokens `
    -FailurePrefix "DataAsset / no-hardcoding policy is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath $controlPrompt.FullName `
    -Tokens $policyTokens `
    -FailurePrefix "DataAsset / no-hardcoding policy is incomplete"
}

function Test-EventDrivenUiAsyncInterfacePolicy {
  $controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
  if ($null -eq $controlPrompt) {
    Add-Failure "UI event / async interface policy is incomplete: missing docs\Development\ZodiacArena_UE5_8_Codex_*.md"
    return
  }

  $policyTokens = @(
    'PolicyId: `DBA.UI.EventAsync`',
    'UI',
    'Tick',
    'Delegate',
    'ViewModel',
    'FieldNotify',
    'MVVM',
    'OnRep',
    'GameplayCue',
    'GameThread'
  )

  Test-FileContainsAllTokens `
    -RelativePath "AGENTS.md" `
    -Tokens $policyTokens `
    -FailurePrefix "UI event / async interface policy is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath $controlPrompt.FullName `
    -Tokens $policyTokens `
    -FailurePrefix "UI event / async interface policy is incomplete"
}

function Test-ChineseLogOutputPolicy {
  $controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
  if ($null -eq $controlPrompt) {
    Add-Failure "Chinese log output policy is incomplete: missing docs\Development\ZodiacArena_UE5_8_Codex_*.md"
    return
  }

  $policyTokens = @(
    'PolicyId: `DBA.Log.ChineseOutput`',
    'UE_LOG',
    'ensureMsgf',
    'checkf',
    'TEXT("',
    'Automation Test',
    'MCP',
    'CI',
    'GameplayTag'
  )

  Test-FileContainsAllTokens `
    -RelativePath "AGENTS.md" `
    -Tokens $policyTokens `
    -FailurePrefix "Chinese log output policy is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath $controlPrompt.FullName `
    -Tokens $policyTokens `
    -FailurePrefix "Chinese log output policy is incomplete"
}

function Test-RuntimePlayerJoinedBuildSummaryContract {
  $gameModeTokens = @(
    'ToStableZodiacName',
    'ToStableElementName',
    'ToStableFiveCampName',
    'FixedSkillGroupId',
    'TryExtractCharacterBuildSummary',
    'AdmissionBuildSummary'
  )
  $runtimePayloadTokens = @(
    'TEXT("zodiac")',
    'TEXT("primaryElement")',
    'TEXT("fiveCamp")',
    'TEXT("fixedSkillGroupId")'
  )

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" `
    -Tokens $gameModeTokens `
    -FailurePrefix "Runtime player-joined build summary contract is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp" `
    -Tokens $runtimePayloadTokens `
    -FailurePrefix "Runtime player-joined build summary contract is incomplete"
}

function Test-SessionTravelBuildSummaryContract {
  $sessionServiceTokens = @(
    'TEXT("characterBuildSummary")',
    'TEXT("DBAZodiac")',
    'TEXT("DBAElement")',
    'TEXT("DBAFiveCamp")',
    'TEXT("DBAFixedSkillGroupId")',
    'TEXT("DBATeamId")',
    'TEXT("teamId")',
    'Connection.Zodiac',
    'Connection.PrimaryElement',
    'Connection.FiveCamp',
    'Connection.FixedSkillGroupId',
    'Connection.TeamId',
    'TryBuildTravelUrlFromConnectionData',
    'TryGetObjectField(TEXT("data")',
    'NestedBuildSummaryObj',
    'TEXT("serverIp")',
    'TEXT("serverPort")',
    'TEXT("sessionToken")'
  )
  $sessionTypesTokens = @(
    'FString Zodiac',
    'FString PrimaryElement',
    'FString FiveCamp',
    'FString FixedSkillGroupId',
    'int32 TeamId'
  )
  $sessionTestTokens = @(
    'FDBA_GameBackendSessionTravelUrlBuildSummaryTest',
    'FDBA_GameBackendSessionConnectionJsonBuildSummaryTest',
    'FDBA_GameBackendSessionConnectionAliasJsonTest',
    'FDBA_GameBackendSessionEnvelopeJsonTest',
    'BuildTravelUrlIncludesFrozenBuildSummary',
    'ConnectionJsonBuildsTravelUrlWithNestedBuildSummary',
    'ConnectionJsonAcceptsNestedServerAliases',
    'ConnectionJsonAcceptsResponseEnvelopeData',
    'DBAZodiac=Rat',
    'DBAElement=Water',
    'DBAFiveCamp=East',
    'DBAFixedSkillGroupId=Rat_Water',
    'DBAZodiac=Tiger',
    'DBAElement=Fire',
    'DBAFiveCamp=South',
    'DBAFixedSkillGroupId=Tiger_Fire',
    'PlayerSessionToken=alias-token',
    'DBAFixedSkillGroupId=Dragon_Wood',
    'PlayerSessionToken=envelope-token',
    'DBAZodiac=Snake',
    'DBAElement=Gold',
    'DBAFixedSkillGroupId=Snake_Gold',
    'DBATeamId=1',
    'DBATeamId=2'
  )
  $urlOptionsServiceTokens = @(
    'TryExtractCharacterBuildSummary',
    'TryExtractTeamId',
    'DBACharacterBuild::MakeFixedSkillGroupId'
  )
  $urlOptionsTestTokens = @(
    'TryExtractCharacterBuildSummary',
    'TryExtractTeamId',
    'ValidatesDedicatedServerBuildSummary',
    'Rat_Fire',
    'DBATeamId=1',
    'TeamId=2',
    'MissingTeamId',
    'NonPositiveTeamId'
  )

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendSessionService.cpp" `
    -Tokens $sessionServiceTokens `
    -FailurePrefix "Session travel build summary contract is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendTypes.h" `
    -Tokens $sessionTypesTokens `
    -FailurePrefix "Session travel build summary contract is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendSessionServiceTests.cpp" `
    -Tokens $sessionTestTokens `
    -FailurePrefix "Session travel build summary contract is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAUrlOptions.cpp" `
    -Tokens $urlOptionsServiceTokens `
    -FailurePrefix "Dedicated Server URL build summary admission implementation is incomplete"

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAUrlOptionsTests.cpp" `
    -Tokens $urlOptionsTestTokens `
    -FailurePrefix "Dedicated Server URL build summary admission coverage is incomplete"
}

function Test-RuntimeMatchLifecycleHandoffContract {
  $gameModeLifecycleTokens = @(
    'void ADBAGameModeBase::HandleMatchHasStarted()',
    'Super::HandleMatchHasStarted()',
    'ReportBackendMatchStarted',
    'void ADBAGameModeBase::ReportBackendMatchStarted()',
    'RuntimeService->NotifyMatchStarted',
    'void ADBAGameModeBase::HandleMatchHasEnded()',
    'Super::HandleMatchHasEnded()',
    'RuntimeService->NotifyMatchEnded',
    'ReportBackendMatchResults',
    'void ADBAGameModeBase::ReportBackendMatchResults()',
    'FDBA_GameBackendRuntimePlayerResult',
    'BackendRuntimePlayerIds',
    'DBAPlayerState->BuildRuntimePlayerResult',
    'FString::Printf(TEXT("ue-match-result-%s")',
    'BuildBackendMatchResultsJson',
    'RuntimeService->NotifyMatchResults'
  )

  Test-FileContainsAllTokens `
    -RelativePath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" `
    -Tokens $gameModeLifecycleTokens `
    -FailurePrefix "Runtime match lifecycle handoff contract is incomplete"
}

Push-Location $repoRoot
try {
  Test-LogTempUsage
  Test-NoPrivateInclude
  Test-ClientOnlyModuleServerGuards
  Test-CppLogicBlueprintBoundaryPolicy
  Test-DataAssetNoHardcodingPolicy
  Test-EventDrivenUiAsyncInterfacePolicy
  Test-ChineseLogOutputPolicy
  Test-RuntimePlayerJoinedBuildSummaryContract
  Test-SessionTravelBuildSummaryContract
  Test-RuntimeMatchLifecycleHandoffContract
}
finally {
  Pop-Location
}

if ($failures.Count -gt 0) {
  foreach ($failure in $failures) {
    Write-Host "FAIL: $failure" -ForegroundColor Red
  }
  throw "Unreal source guardrails failed: $($failures -join '; ')"
}

Write-Host "PASS: Unreal source guardrails" -ForegroundColor Green
