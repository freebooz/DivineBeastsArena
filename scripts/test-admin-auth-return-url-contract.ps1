<#
Validates that Angular Admin preserves safe first-party return URLs when an
unauthenticated operator is redirected to login.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function Assert-FileContains {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string[]]$RequiredTokens
  )

  $fullPath = Join-Path $repoRoot $RelativePath
  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $RelativePath"
  }

  $content = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath
  $missing = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missing.Count -gt 0) {
    throw "$RelativePath is missing Admin auth return-url contract tokens: $($missing -join ', ')"
  }
}

Assert-FileContains "DBA_GameAdmin\src\app\core\auth.guard.ts" @(
  "CanActivateChildFn",
  "RouterStateSnapshot",
  "returnUrl",
  "state.url",
  "queryParams"
)

Assert-FileContains "DBA_GameAdmin\src\app\app.routes.ts" @(
  "canActivateChild",
  "authGuard"
)

Assert-FileContains "DBA_GameAdmin\src\app\pages\login-page.component.ts" @(
  "ActivatedRoute",
  "queryParamMap.get('returnUrl')",
  "safeReturnUrl",
  "returnUrl.startsWith('/')",
  "!returnUrl.startsWith('//')",
  "navigateByUrl",
  "void this.router.navigateByUrl(this.safeReturnUrl())"
)

Write-Host "PASS: Admin auth return-url contract" -ForegroundColor Green
