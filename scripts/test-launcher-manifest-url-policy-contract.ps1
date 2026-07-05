<#
Validates that the Tauri launcher applies the same network URL policy to
manifest fetch URLs and manifest downloadUrl values.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$launcherLibPath = Join-Path $repoRoot "DBA_GameLauncher\src-tauri\src\lib.rs"

if (-not (Test-Path -LiteralPath $launcherLibPath)) {
  throw "Launcher Rust library is missing: DBA_GameLauncher\src-tauri\src\lib.rs"
}

$content = Get-Content -Raw -Encoding UTF8 -LiteralPath $launcherLibPath

foreach ($token in @(
  "fn validate_network_url",
  "fn extract_url_host",
  'validate_network_url(&url, "ManifestUrl")',
  'validate_network_url(&manifest.download_url, "Manifest downloadUrl")',
  "must include a valid host",
  'trimmed.strip_prefix("https://")',
  'trimmed.strip_prefix("http://")',
  '"localhost" | "127.0.0.1" | "::1"',
  "must use HTTPS unless it points to localhost",
  "validate_manifest_rejects_external_http_download_url",
  "validate_network_url_accepts_ipv6_loopback_http_for_local_validation",
  "validate_network_url_rejects_https_url_without_host"
)) {
  if (-not $content.Contains($token)) {
    throw "Launcher manifest URL policy is missing contract token: $token"
  }
}

Write-Host "PASS: Launcher manifest URL policy contract" -ForegroundColor Green
