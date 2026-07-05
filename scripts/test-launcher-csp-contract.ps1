<#
Validates that the Tauri launcher CSP keeps WebView network access scoped to
the local development backend and Tauri IPC origins.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$tauriConfigPath = Join-Path $repoRoot "DBA_GameLauncher\src-tauri\tauri.conf.json"

if (-not (Test-Path -LiteralPath $tauriConfigPath)) {
  throw "Launcher Tauri config is missing: DBA_GameLauncher\src-tauri\tauri.conf.json"
}

$config = Get-Content -Raw -Encoding UTF8 -LiteralPath $tauriConfigPath | ConvertFrom-Json
$csp = [string]$config.app.security.csp

if ([string]::IsNullOrWhiteSpace($csp)) {
  throw "Launcher Tauri config must define app.security.csp."
}

$connectDirectiveMatch = [regex]::Match($csp, "(^|;)\s*connect-src\s+([^;]+)")
if (-not $connectDirectiveMatch.Success) {
  throw "Launcher CSP must define a connect-src directive."
}

$connectSources = @($connectDirectiveMatch.Groups[2].Value -split "\s+" | Where-Object { $_ })

foreach ($requiredSource in @(
  "'self'",
  "ipc:",
  "http://ipc.localhost",
  "http://localhost:8080",
  "http://127.0.0.1:8080"
)) {
  if ($requiredSource -notin $connectSources) {
    throw "Launcher CSP connect-src is missing required source: $requiredSource"
  }
}

foreach ($forbiddenSource in @(
  "*",
  "http:",
  "https:",
  "https://*",
  "http://*",
  "http://localhost:*",
  "http://127.0.0.1:*",
  "ws://*",
  "wss://*"
)) {
  if ($forbiddenSource -in $connectSources) {
    throw "Launcher CSP connect-src contains forbidden broad source: $forbiddenSource"
  }
}

Write-Host "PASS: Launcher CSP contract" -ForegroundColor Green
