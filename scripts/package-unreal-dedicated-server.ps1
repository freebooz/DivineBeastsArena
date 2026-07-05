<#
Builds, cooks, stages, and archives the Unreal Dedicated Server package.

The output is intended for scripts/diagnose-unreal-packaged-server-readiness.ps1
and scripts/smoke-unreal-dedicated-server.ps1 -UsePackagedServer.
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
  [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
  [string]$ProjectPath = "",
  [string]$ArchiveDirectory = "",
  [string]$Map = "/Game/Maps/Lobby/LobbyMap",
  [ValidateSet("Development", "Shipping")]
  [string]$Configuration = "Development",
  [switch]$IncludeClientCook,
  [switch]$SkipBuild,
  [switch]$SkipCook,
  [switch]$NoPak,
  [switch]$NoArchive
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
  $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
  $ProjectPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\DivineBeastsArena.uproject"
}

if ([string]::IsNullOrWhiteSpace($ArchiveDirectory)) {
  $ArchiveDirectory = Join-Path -Path $repoRoot -ChildPath ".tmp\packaged-server"
}

$runUat = Join-Path -Path $UnrealRoot -ChildPath "Engine\Build\BatchFiles\RunUAT.bat"
if (-not (Test-Path -LiteralPath $runUat)) {
  throw "RunUAT.bat not found: $runUat"
}

$projectFullPath = (Resolve-Path -LiteralPath $ProjectPath).ProviderPath
$archiveFullPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ArchiveDirectory)
New-Item -ItemType Directory -Force -Path $archiveFullPath | Out-Null

$uatArgs = @(
  "BuildCookRun",
  "-project=`"$projectFullPath`"",
  "-noP4",
  "-server",
  "-serverplatform=Win64",
  "-serverconfig=$Configuration",
  "-clientconfig=$Configuration",
  "-stage",
  "-utf8output",
  "-map=$Map"
)

if (-not $IncludeClientCook) {
  $uatArgs += "-noclient"
}

if ($SkipBuild) {
  $uatArgs += "-skipbuild"
}
else {
  $uatArgs += "-build"
}

if ($SkipCook) {
  $uatArgs += "-skipcook"
}
else {
  $uatArgs += "-cook"
}

if (-not $NoPak) {
  $uatArgs += "-pak"
}

if (-not $NoArchive) {
  $uatArgs += @(
    "-archive",
    "-archivedirectory=`"$archiveFullPath`""
  )
}

Write-Host "Running Unreal Dedicated Server packaging:" -ForegroundColor Cyan
Write-Host "$runUat $($uatArgs -join ' ')"

if ($PSCmdlet.ShouldProcess($projectFullPath, "Package Unreal Dedicated Server to $archiveFullPath")) {
  & $runUat @uatArgs
  if ($LASTEXITCODE -ne 0) {
    throw "RunUAT failed with exit code $LASTEXITCODE"
  }

  Write-Host "PASS: Unreal Dedicated Server package command completed" -ForegroundColor Green
  Write-Host "ArchiveDirectory: $archiveFullPath"
}
