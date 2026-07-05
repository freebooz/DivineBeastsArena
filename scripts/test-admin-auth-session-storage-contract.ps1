<#
Validates that the Angular Admin app stores privileged JWT sessions in
sessionStorage instead of localStorage.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$authServicePath = Join-Path $repoRoot "DBA_GameAdmin\src\app\core\auth.service.ts"

if (-not (Test-Path -LiteralPath $authServicePath)) {
  throw "Admin AuthService is missing: DBA_GameAdmin\src\app\core\auth.service.ts"
}

$content = Get-Content -Raw -Encoding UTF8 -LiteralPath $authServicePath

foreach ($token in @(
  "sessionStorage.setItem",
  "sessionStorage.removeItem",
  "sessionStorage.getItem",
  "dba.admin.session",
  "clearExpiredSession",
  "this.clearExpiredSession()",
  "return null"
)) {
  if (-not $content.Contains($token)) {
    throw "Admin AuthService is missing session storage contract token: $token"
  }
}

if ($content.Contains("localStorage")) {
  throw "Admin AuthService must not persist privileged admin JWT sessions in localStorage."
}

Write-Host "PASS: Admin auth session storage contract" -ForegroundColor Green
