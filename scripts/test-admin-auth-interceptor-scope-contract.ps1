<#
Validates that the Angular Admin auth interceptor only attaches the admin JWT
to first-party API requests.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$interceptorPath = Join-Path $repoRoot "DBA_GameAdmin\src\app\core\auth.interceptor.ts"

if (-not (Test-Path -LiteralPath $interceptorPath)) {
  throw "Admin auth interceptor is missing: DBA_GameAdmin\src\app\core\auth.interceptor.ts"
}

$content = Get-Content -Raw -Encoding UTF8 -LiteralPath $interceptorPath

foreach ($token in @(
  "HttpErrorResponse",
  "Router",
  "catchError",
  "throwError",
  "environment.apiBaseUrl",
  "shouldAttachToken",
  "handleAuthFailure",
  "error.status === 401 || error.status === 403",
  "auth.signOut()",
  "router.navigate(['/login']",
  "returnUrl: window.location.pathname + window.location.search",
  "url.startsWith('/')",
  "new URL(url, window.location.origin)",
  "configuredApiOrigin",
  "requestOrigin",
  "return next(request);",
  'Authorization: `Bearer ${token}`'
)) {
  if (-not $content.Contains($token)) {
    throw "Admin auth interceptor scope contract is missing token: $token"
  }
}

Write-Host "PASS: Admin auth interceptor scope contract" -ForegroundColor Green
