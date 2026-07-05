<#
Validates the global event-driven UI update and async external/interface access policy.
Run from the repository root:
  .\scripts\test-unreal-ui-event-async-policy.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1
if ($null -eq $controlPrompt) {
  throw "Required control prompt file is missing under docs\Development: ZodiacArena_UE5_8_Codex_*.md"
}

function Assert-FileContains {
  param(
    [Parameter(Mandatory = $true)][string]$Path,
    [Parameter(Mandatory = $true)][string[]]$RequiredTokens
  )

  $fullPath = if ([System.IO.Path]::IsPathRooted($Path)) {
    $Path
  }
  else {
    Join-Path -Path $repoRoot -ChildPath $Path
  }

  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $Path"
  }

  $content = Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
  $missingTokens = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missingTokens.Count -gt 0) {
    throw "$Path is missing UI event / async interface policy tokens: $($missingTokens -join ', ')"
  }
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

Assert-FileContains "AGENTS.md" $policyTokens
Assert-FileContains $controlPrompt.FullName $policyTokens

Assert-FileContains "scripts\validate-unreal-source-guardrails.ps1" @(
  'Test-EventDrivenUiAsyncInterfacePolicy',
  'PolicyId: `DBA.UI.EventAsync`',
  'Delegate',
  'ViewModel',
  'FieldNotify',
  'GameThread'
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
  "test-unreal-ui-event-async-policy.ps1",
  "Unreal UI event / async interface policy contract"
)

Assert-FileContains "scripts\validate-production-evidence-contracts.ps1" @(
  "test-unreal-ui-event-async-policy.ps1",
  "PASS: Unreal UI event / async interface policy contract"
)

Write-Host "PASS: Unreal UI event / async interface policy contract" -ForegroundColor Green
