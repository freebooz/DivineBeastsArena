<#
Runs a short local Dedicated Server smoke test.

Default mode uses UnrealEditor.exe with -server so uncooked local maps can load.
Use -UsePackagedServer only after cooking or staging server content.

Run from the repository root:
  .\scripts\smoke-unreal-dedicated-server.ps1
#>

[CmdletBinding()]
param(
  [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
  [string]$ProjectPath = "",
  [string]$Map = "/Game/Maps/Lobby/LobbyMap",
  [int]$Port = 17779,
  [int]$RunSeconds = 20,
  [switch]$UsePackagedServer,
  [string]$ServerExePath = "",
  [string]$PackagedRoot = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
  $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
  $ProjectPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\DivineBeastsArena.uproject"
}

$projectFullPath = (Resolve-Path -LiteralPath $ProjectPath).ProviderPath
$projectDir = Split-Path -Parent $projectFullPath
$logDir = Join-Path -Path $repoRoot -ChildPath ".tmp\server-smoke"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$runId = Get-Date -Format "yyyyMMdd-HHmmss"
$logPath = Join-Path -Path $logDir -ChildPath "server-smoke-$runId.log"

function Resolve-PackagedServerExe {
  param(
    [string]$ExplicitServerExePath,
    [string]$ExplicitPackagedRoot,
    [string]$ProjectDirectory,
    [string]$RepositoryRoot
  )

  if (-not [string]::IsNullOrWhiteSpace($ExplicitServerExePath)) {
    if (-not (Test-Path -LiteralPath $ExplicitServerExePath)) {
      throw "Packaged server executable not found: $ExplicitServerExePath"
    }
    return (Resolve-Path -LiteralPath $ExplicitServerExePath).ProviderPath
  }

  $roots = @()
  if (-not [string]::IsNullOrWhiteSpace($ExplicitPackagedRoot)) {
    $roots += $ExplicitPackagedRoot
  }
  $roots += @(
    (Join-Path -Path $RepositoryRoot -ChildPath ".tmp\packaged-server"),
    (Join-Path -Path $ProjectDirectory -ChildPath "Saved\StagedBuilds"),
    (Join-Path -Path $RepositoryRoot -ChildPath "Artifacts\UnrealServer")
  )

  $candidates = @()
  foreach ($root in $roots) {
    if (-not (Test-Path -LiteralPath $root)) {
      continue
    }
    $candidates += Get-ChildItem -LiteralPath $root -Recurse -File -Filter "DivineBeastsArenaServer.exe" -ErrorAction SilentlyContinue |
      Select-Object -ExpandProperty FullName
  }

  if (-not $candidates -or $candidates.Count -eq 0) {
    throw "No staged packaged server executable found. Build/stage one first, or pass -ServerExePath. Checked: $($roots -join ', ')"
  }

  return ($candidates | Sort-Object | Select-Object -First 1)
}

function Test-CookedServerContent {
  param([string]$ServerExe)

  $cursor = Split-Path -Parent $ServerExe
  for ($i = 0; $i -lt 8 -and -not [string]::IsNullOrWhiteSpace($cursor); $i++) {
    $pakFiles = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "*.pak" -ErrorAction SilentlyContinue | Select-Object -First 1
    $assetRegistry = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "AssetRegistry.bin" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($pakFiles -or $assetRegistry) {
      return $true
    }
    $parent = Split-Path -Parent $cursor
    if ($parent -eq $cursor) {
      break
    }
    $cursor = $parent
  }

  return $false
}

if ($UsePackagedServer) {
  $serverExe = Resolve-PackagedServerExe `
    -ExplicitServerExePath $ServerExePath `
    -ExplicitPackagedRoot $PackagedRoot `
    -ProjectDirectory $projectDir `
    -RepositoryRoot $repoRoot

  if (-not (Test-CookedServerContent -ServerExe $serverExe)) {
    throw "Packaged server content was not found near $serverExe. Do not run the raw Binaries\Win64 server exe for this smoke; cook/stage server content first."
  }
  $args = @(
    $Map
  )
  $workingDir = Split-Path -Parent $serverExe
}
else {
  $serverExe = Join-Path -Path $UnrealRoot -ChildPath "Engine\Binaries\Win64\UnrealEditor.exe"
  if (-not (Test-Path -LiteralPath $serverExe)) {
    throw "UnrealEditor.exe not found: $serverExe"
  }
  $args = @(
    "`"$projectFullPath`"",
    $Map,
    "-server"
  )
  $workingDir = $projectDir
}

$args += @(
  "-log",
  "-abslog=`"$logPath`"",
  "-port=$Port",
  "-DBAHeadlessLobbyServer",
  "-DBAUseMockBackend",
  "-DBASkipBackendRuntime"
)

Write-Host "Starting server smoke: $serverExe" -ForegroundColor Cyan
Write-Host "Log: $logPath"

$process = $null
try {
  $process = Start-Process -FilePath $serverExe -ArgumentList ($args -join " ") -PassThru -WindowStyle Hidden -WorkingDirectory $workingDir
  Start-Sleep -Seconds $RunSeconds

  $isAlive = $false
  try {
    $null = Get-Process -Id $process.Id -ErrorAction Stop
    $isAlive = $true
  }
  catch {
    $isAlive = $false
  }

  if (-not (Test-Path -LiteralPath $logPath)) {
    throw "Server smoke log was not created: $logPath"
  }

  $criticalPatterns = @(
    "Fatal error",
    "Critical error",
    "Assertion failed",
    "LogWindows: Error"
  )
  $criticalMatches = Select-String -LiteralPath $logPath -Pattern $criticalPatterns
  if ($criticalMatches) {
    $tail = Get-Content -LiteralPath $logPath -Encoding UTF8 -Tail 80
    throw "Server smoke found critical log entries:`n$($criticalMatches | Select-Object -First 8 | ForEach-Object { $_.Line } | Out-String)`nLog tail:`n$($tail -join "`n")"
  }

  if (-not $isAlive) {
    $tail = Get-Content -LiteralPath $logPath -Encoding UTF8 -Tail 80
    throw "Server exited before $RunSeconds seconds.`nLog tail:`n$($tail -join "`n")"
  }

  $loadedMap = Select-String -LiteralPath $logPath -Pattern "Load map complete $Map" -SimpleMatch
  if (-not $loadedMap) {
    $tail = Get-Content -LiteralPath $logPath -Encoding UTF8 -Tail 80
    throw "Server did not report map load completion for $Map.`nLog tail:`n$($tail -join "`n")"
  }

  Write-Host "PASS: Unreal dedicated server smoke" -ForegroundColor Green
  Write-Host "ProcessId: $($process.Id)"
  Write-Host "Log: $logPath"
}
finally {
  if ($process) {
    try {
      $running = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
      if ($running) {
        Stop-Process -Id $process.Id -Force
      }
    }
    catch {
      Write-Host "Warning: failed to stop server process $($process.Id): $($_.Exception.Message)" -ForegroundColor Yellow
    }
  }
}
