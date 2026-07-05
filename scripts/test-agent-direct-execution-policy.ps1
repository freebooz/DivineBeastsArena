<#
Validates the repository-wide direct execution policy.
Chinese strings are built from code points so Windows PowerShell can parse this
script correctly even when the file is read as ANSI.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$controlPrompt = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_UE5_8_Codex_*.md" -File | Select-Object -First 1

function New-CodePointText {
  param(
    [Parameter(Mandatory = $true)][int[]]$CodePoints
  )

  return -join ($CodePoints | ForEach-Object { [char]$_ })
}

$missingControlPromptText = New-CodePointText @(0x7F3A, 0x5C11, 0x603B, 0x63A7, 0x63D0, 0x793A, 0x8BCD, 0x6587, 0x4EF6, 0xFF1A)
$missingRequiredFileText = New-CodePointText @(0x7F3A, 0x5C11, 0x5FC5, 0x9700, 0x6587, 0x4EF6, 0xFF1A)
$missingPolicyTokenText = New-CodePointText @(0x7F3A, 0x5C11, 0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x4EE4, 0x724C, 0xFF1A)
$directExecutionDefaultText = New-CodePointText @(0x540E, 0x7EED, 0x4EFB, 0x52A1, 0x9ED8, 0x8BA4, 0x201C, 0x76F4, 0x63A5, 0x6267, 0x884C, 0x201D)
$directExecutionNonInteractiveText = New-CodePointText @(0x9ED8, 0x8BA4, 0x975E, 0x4EA4, 0x4E92, 0x6267, 0x884C)
$directExecutionNoAskText = New-CodePointText @(0x4E0D, 0x518D, 0x5148, 0x8BE2, 0x95EE, 0x662F, 0x5426, 0x7EE7, 0x7EED)
$directExecutionNoWaitText = New-CodePointText @(0x4E0D, 0x4EE5, 0x7B49, 0x5F85, 0x786E, 0x8BA4, 0x66FF, 0x4EE3, 0x53EF, 0x9A8C, 0x8BC1, 0x63A8, 0x8FDB)
$directExecutionNoNextStageAskText = New-CodePointText @(0x4E0D, 0x518D, 0x8BE2, 0x95EE, 0x201C, 0x662F, 0x5426, 0x7EE7, 0x7EED, 0x201D, 0x201C, 0x662F, 0x5426, 0x8FDB, 0x5165, 0x4E0B, 0x4E00, 0x9636, 0x6BB5, 0x201D, 0x6216, 0x201C, 0x662F, 0x5426, 0x6267, 0x884C, 0x5E38, 0x89C4, 0x9A8C, 0x8BC1, 0x201D)
$directExecutionNextStepText = New-CodePointText @(0x9ED8, 0x8BA4, 0x76F4, 0x63A5, 0x8FDB, 0x5165, 0x4E0B, 0x4E00, 0x53EF, 0x9A8C, 0x8BC1, 0x6B65, 0x9AA4)
$directExecutionContinuousText = New-CodePointText @(0x9ED8, 0x8BA4, 0x6301, 0x7EED, 0x63A8, 0x8FDB)
$directExecutionRegularStageText = New-CodePointText @(0x5E38, 0x89C4, 0x9636, 0x6BB5, 0x5207, 0x6362)
$directExecutionNotCheckpointText = New-CodePointText @(0x5747, 0x4E0D, 0x662F, 0x786E, 0x8BA4, 0x70B9)
$directExecutionNoPlanAsDoneText = New-CodePointText @(0x4E0D, 0x80FD, 0x5C06, 0x8BA1, 0x5212, 0x8F93, 0x51FA, 0x3001, 0x7B49, 0x5F85, 0x786E, 0x8BA4, 0x6216, 0x53CD, 0x590D, 0x8BE2, 0x95EE, 0x4F5C, 0x4E3A, 0x9ED8, 0x8BA4, 0x5B8C, 0x6210, 0x72B6, 0x6001)
$externalInputsMissingText = New-CodePointText @(0x5916, 0x90E8, 0x53D1, 0x5E03, 0x8F93, 0x5165, 0x6682, 0x7F3A, 0x65F6)
$continueLocalVerificationText = New-CodePointText @(0x7EE7, 0x7EED, 0x63A8, 0x8FDB, 0x672C, 0x5730, 0x53EF, 0x9A8C, 0x8BC1, 0x7684, 0x5DE5, 0x7A0B, 0x3001, 0x81EA, 0x52A8, 0x5316, 0x3001, 0x6D4B, 0x8BD5, 0x548C, 0x6587, 0x6863, 0x95ED, 0x73AF)
$higherPriorityConfirmText = New-CodePointText @(0x53EA, 0x6709, 0x5728, 0x66F4, 0x9AD8, 0x4F18, 0x5148, 0x7EA7, 0x89C4, 0x5219, 0x6216, 0x8FD0, 0x884C, 0x5E73, 0x53F0, 0x5F3A, 0x5236, 0x8981, 0x6C42, 0x65F6, 0x624D, 0x4E2D, 0x65AD, 0x6267, 0x884C, 0x5E76, 0x8BF7, 0x6C42, 0x786E, 0x8BA4)
$directExecutionContractText = New-CodePointText @(0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x5951, 0x7EA6)
$directExecutionDefaultNoQuoteText = New-CodePointText @(0x540E, 0x7EED, 0x4EFB, 0x52A1, 0x9ED8, 0x8BA4, 0x76F4, 0x63A5, 0x6267, 0x884C)

if ($null -eq $controlPrompt) {
  throw "${missingControlPromptText}docs\Development\ZodiacArena_UE5_8_Codex_*.md"
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
    throw "${missingRequiredFileText}$Path"
  }

  $content = Get-Content -LiteralPath $fullPath -Encoding UTF8 -Raw
  $missingTokens = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missingTokens.Count -gt 0) {
    throw "${Path} ${missingPolicyTokenText}$($missingTokens -join ', ')"
  }
}

$policyTokens = @(
  'PolicyId: `DBA.Agent.DirectExecution`',
  $directExecutionDefaultText,
  $directExecutionNonInteractiveText,
  $directExecutionNoAskText,
  $directExecutionNoWaitText,
  $directExecutionNoNextStageAskText,
  $directExecutionNextStepText,
  $directExecutionContinuousText,
  $directExecutionRegularStageText,
  $directExecutionNotCheckpointText,
  $directExecutionNoPlanAsDoneText,
  $externalInputsMissingText,
  $continueLocalVerificationText,
  $higherPriorityConfirmText
)

Assert-FileContains "AGENTS.md" $policyTokens
Assert-FileContains $controlPrompt.FullName $policyTokens

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
  "directExecutionPolicyContractStepText",
  "New-CodePointText @(0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x5951, 0x7EA6)",
  "test-agent-direct-execution-policy.ps1"
)

Assert-FileContains "scripts\validate-production-evidence-contracts.ps1" @(
  "test-agent-direct-execution-policy.ps1",
  'PASS: {0}'
)

$boardFiles = Get-ChildItem -LiteralPath (Join-Path -Path $repoRoot -ChildPath "docs\Development") -Filter "ZodiacArena_*.md" -File
$boardHasPolicy = $false
foreach ($boardFile in $boardFiles) {
  $boardContent = Get-Content -LiteralPath $boardFile.FullName -Encoding UTF8 -Raw
  if ($boardContent.Contains("DBA.Agent.DirectExecution") -and $boardContent.Contains($directExecutionDefaultNoQuoteText)) {
    $boardHasPolicy = $true
    break
  }
}

if (-not $boardHasPolicy) {
  throw "${missingPolicyTokenText}DBA.Agent.DirectExecution, $directExecutionDefaultNoQuoteText"
}

Write-Host ("PASS: {0}" -f $directExecutionContractText) -ForegroundColor Green
