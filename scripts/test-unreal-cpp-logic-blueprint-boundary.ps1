<#
Validates the global C++ logic / Blueprint configuration boundary.
Run from the repository root:
  .\scripts\test-unreal-cpp-logic-blueprint-boundary.ps1
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
    throw "$Path is missing C++ logic / Blueprint boundary tokens: $($missingTokens -join ', ')"
  }
}

Assert-FileContains "AGENTS.md" @(
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

Assert-FileContains $controlPrompt.FullName @(
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

Assert-FileContains "scripts\validate-unreal-source-guardrails.ps1" @(
  "Test-CppLogicBlueprintBoundaryPolicy",
  "AGENTS.md",
  "ZodiacArena_UE5_8_Codex_",
  "C++",
  "Gameplay",
  "GAS",
  "Blueprint",
  "DataAsset",
  "UPROPERTY",
  "UFUNCTION",
  "Subsystem"
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
  "test-unreal-cpp-logic-blueprint-boundary.ps1",
  "Unreal C++ logic / Blueprint boundary contract"
)

Assert-FileContains "scripts\validate-production-evidence-contracts.ps1" @(
  "test-unreal-cpp-logic-blueprint-boundary.ps1",
  "PASS: Unreal C++ logic / Blueprint boundary contract"
)

Write-Host "PASS: Unreal C++ logic / Blueprint boundary contract" -ForegroundColor Green
