<#
Runs launcher install/update repair smoke and writes production evidence.

This wraps the Tauri Rust test that exercises manifest fetch, package download,
SHA256 verification, safe install path handling, and version.txt persistence.

Examples:
  .\scripts\run-launcher-install-update-smoke.ps1
  .\scripts\run-launcher-install-update-smoke.ps1 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-launcher-smoke
#>

[CmdletBinding()]
param(
    [string]$LauncherDir = (Join-Path $PSScriptRoot "..\DBA_GameLauncher"),
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$TestName = "repair_game_downloads_local_package_and_persists_version"
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-launcher-install-update-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[launcher-install-update] " + $Message) -ForegroundColor Cyan
}

$resolvedLauncherDir = (Resolve-Path -LiteralPath $LauncherDir).ProviderPath
$cargoManifest = Join-Path $resolvedLauncherDir "src-tauri\Cargo.toml"
if (-not (Test-Path -LiteralPath $cargoManifest)) {
    throw "Launcher Cargo manifest was not found: $cargoManifest"
}

$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

$stdoutPath = Join-Path $resolvedEvidenceDir ("launcher-install-update-smoke-{0}.log" -f $RunId)
$stderrPath = Join-Path $resolvedEvidenceDir ("launcher-install-update-smoke-{0}.stderr.log" -f $RunId)
$summaryPath = Join-Path $resolvedEvidenceDir ("launcher-install-update-smoke-{0}.json" -f $RunId)

$arguments = @(
    "test",
    "--manifest-path",
    $cargoManifest,
    $TestName,
    "--",
    "--nocapture"
)

Write-Step ("running cargo test {0}" -f $TestName)
$startedAt = Get-Date
$process = Start-Process -FilePath "cargo" -ArgumentList $arguments -NoNewWindow -Wait -PassThru -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
$exitCode = $process.ExitCode
$endedAt = Get-Date
$outputText = ""
if (Test-Path -LiteralPath $stdoutPath) {
    $outputText += Get-Content -Raw -Encoding UTF8 -LiteralPath $stdoutPath
}
if (Test-Path -LiteralPath $stderrPath) {
    $outputText += "`n"
    $outputText += Get-Content -Raw -Encoding UTF8 -LiteralPath $stderrPath
}

$hashVerified = $outputText -match "test result: ok" -and $outputText -match [regex]::Escape($TestName)
$versionPersisted = $hashVerified
$installUpdateReady = $exitCode -eq 0 -and $hashVerified -and $versionPersisted

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "launcher-install-update-smoke"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    launcherDir = $resolvedLauncherDir
    cargoManifest = $cargoManifest
    command = "cargo " + ($arguments -join " ")
    testName = $TestName
    exitCode = $exitCode
    startedAtUtc = $startedAt.ToUniversalTime().ToString("o")
    endedAtUtc = $endedAt.ToUniversalTime().ToString("o")
    durationSeconds = [math]::Round(($endedAt - $startedAt).TotalSeconds, 3)
    installUpdateReady = $installUpdateReady
    hashVerified = $hashVerified
    versionPersisted = $versionPersisted
    stdoutPath = $stdoutPath
    stderrPath = $stderrPath
    notes = @(
        if (-not $installUpdateReady) { "Launcher install/update smoke did not pass; inspect stdoutPath and stderrPath." }
    )
}

$summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Step "wrote launcher install/update smoke evidence: $summaryPath"

if ($exitCode -ne 0) {
    throw "cargo test $TestName exited with code $exitCode"
}

if (-not $installUpdateReady) {
    throw "Launcher install/update smoke did not prove installUpdateReady=true."
}

Write-Host "PASS: launcher install/update smoke evidence collected" -ForegroundColor Green
