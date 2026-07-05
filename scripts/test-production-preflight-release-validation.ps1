<#
Checks production-preflight.ps1 wires release readiness diagnostics and
validation after producing the release readiness report.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$preflightPath = Join-Path $repoRoot "scripts\production-preflight.ps1"
$content = Get-Content -Raw -Encoding UTF8 -LiteralPath $preflightPath

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-Index {
    param([Parameter(Mandatory = $true)][string]$Needle)

    return $content.IndexOf($Needle, [System.StringComparison]::Ordinal)
}

function Get-LastIndex {
    param([Parameter(Mandatory = $true)][string]$Needle)

    return $content.LastIndexOf($Needle, [System.StringComparison]::Ordinal)
}

$readinessIndex = Get-Index 'Invoke-Check "release readiness report"'
$blockerIndex = Get-Index 'Invoke-Check "release blocker actions"'
$blockerValidationIndex = Get-Index 'Invoke-Check "release blocker action validation"'
$externalOnlyValidationIndex = Get-Index 'Invoke-Check "release blockers external-only validation"'
$readinessPostureRefreshIndex = Get-Index 'Invoke-Check "release readiness blocker posture refresh"'
$developmentContinuationValidationIndex = Get-Index 'Invoke-Check "development continuation readiness validation"'
$templateIndex = Get-Index 'Invoke-Check "release input template"'
$templateValidationIndex = Get-Index 'Invoke-Check "release input template validation"'
$valuesTemplateIndex = Get-Index 'Invoke-Check "release input values template"'
$valuesValidationIndex = Get-Index 'Invoke-Check "release input values template validation"'
$commandPlanTemplateCheckIndex = Get-Index 'Invoke-Check "release command plan template check"'
$gitWorkspaceIndex = Get-Index 'Invoke-Check "git workspace"'
$productionEvidenceContractsIndex = Get-Index 'Invoke-Check "production evidence contracts"'
$moduleBoundariesIndex = Get-Index 'Invoke-Check "Unreal module boundaries"'
$moduleBoundaryFixturesIndex = Get-Index 'Invoke-Check "Unreal module boundary fixtures"'
$baselineEntrypointsIndex = Get-Index 'Invoke-Check "Unreal baseline entrypoints"'
$baselineEntrypointFixturesIndex = Get-Index 'Invoke-Check "Unreal baseline entrypoint fixtures"'
$mobaFoundationIndex = Get-Index 'Invoke-Check "Unreal Moba foundation"'
$mobaFoundationFixturesIndex = Get-Index 'Invoke-Check "Unreal Moba foundation fixtures"'
$sourceGuardrailsIndex = Get-Index 'Invoke-Check "Unreal source guardrails"'
$internalRouteProtectionIndex = Get-Index 'Invoke-Check "internal API route protection"'
$internalRouteProtectionFixturesIndex = Get-Index 'Invoke-Check "internal API route protection fixtures"'
$playerIdClaimBoundaryIndex = Get-Index 'Invoke-Check "player_id claim boundary"'
$backendTestIndex = Get-Index 'Invoke-Check "backend dotnet test"'

Assert-True ($gitWorkspaceIndex -ge 0) "Expected production preflight to inspect git workspace state."
Assert-True ($productionEvidenceContractsIndex -gt $gitWorkspaceIndex) "Expected production preflight to run production evidence contracts after git workspace inspection."
Assert-True ($moduleBoundariesIndex -gt $productionEvidenceContractsIndex) "Expected production preflight to run Unreal module boundary validation after production evidence contracts."
Assert-True ($moduleBoundariesIndex -ge 0) "Expected production preflight to run Unreal module boundary validation."
Assert-True ($moduleBoundaryFixturesIndex -gt $moduleBoundariesIndex) "Expected production preflight to run Unreal module boundary fixtures after the real boundary validation."
Assert-True ($baselineEntrypointsIndex -gt $moduleBoundaryFixturesIndex) "Expected production preflight to run Unreal baseline entrypoints after module boundary fixtures."
Assert-True ($baselineEntrypointFixturesIndex -gt $baselineEntrypointsIndex) "Expected production preflight to run Unreal baseline entrypoint fixtures after the real baseline validation."
Assert-True ($mobaFoundationIndex -ge 0) "Expected production preflight to run Unreal Moba foundation validation."
Assert-True ($mobaFoundationIndex -gt $baselineEntrypointFixturesIndex) "Expected production preflight to run Unreal Moba foundation after baseline entrypoint fixtures."
Assert-True ($mobaFoundationFixturesIndex -gt $mobaFoundationIndex) "Expected production preflight to run Unreal Moba foundation fixtures after the real foundation validation."
Assert-True ($sourceGuardrailsIndex -gt $mobaFoundationFixturesIndex) "Expected production preflight to run Unreal source guardrails after Moba foundation fixtures."
Assert-True ($internalRouteProtectionIndex -gt $sourceGuardrailsIndex) "Expected production preflight to validate internal API route protection after source guardrails."
Assert-True ($internalRouteProtectionFixturesIndex -gt $internalRouteProtectionIndex) "Expected production preflight to run internal API route protection fixtures after the real source validation."
Assert-True ($playerIdClaimBoundaryIndex -gt $internalRouteProtectionFixturesIndex) "Expected production preflight to run player_id claim boundary validation after internal API route protection fixtures."
Assert-True ($backendTestIndex -gt $playerIdClaimBoundaryIndex) "Expected production preflight to run backend tests after player_id claim boundary validation."
Assert-True ($readinessIndex -ge 0) "Expected production preflight to generate the release readiness report."
Assert-True ($blockerIndex -gt $readinessIndex) "Expected production preflight to diagnose release blockers after readiness report."
Assert-True ($blockerValidationIndex -gt $blockerIndex) "Expected production preflight to validate release blocker actions after diagnosis."
Assert-True ($externalOnlyValidationIndex -gt $blockerValidationIndex) "Expected production preflight to validate release blockers are external-input-only after action validation."
Assert-True ($readinessPostureRefreshIndex -gt $externalOnlyValidationIndex) "Expected production preflight to refresh release readiness after external-only validation."
Assert-True ($developmentContinuationValidationIndex -gt $readinessPostureRefreshIndex) "Expected production preflight to validate development continuation after readiness posture refresh."
Assert-True ($templateIndex -gt $developmentContinuationValidationIndex) "Expected production preflight to write release input template after development continuation validation."
Assert-True ($templateValidationIndex -gt $templateIndex) "Expected production preflight to validate release input template after writing it."
Assert-True ($valuesTemplateIndex -gt $templateValidationIndex) "Expected production preflight to write release input values template after validating the input template."
Assert-True ($valuesValidationIndex -gt $valuesTemplateIndex) "Expected production preflight to validate release input values template after writing it."
Assert-True ($commandPlanTemplateCheckIndex -gt $valuesValidationIndex) "Expected production preflight to write the release command plan template check after validating input values."
Assert-True ($content -match [regex]::Escape("diagnose-release-blockers.ps1")) "Expected production preflight to call diagnose-release-blockers.ps1."
Assert-True ($content -match [regex]::Escape("validate-release-blocker-actions.ps1")) "Expected production preflight to call validate-release-blocker-actions.ps1."
Assert-True ($content -match [regex]::Escape("validate-release-blockers-external-only.ps1")) "Expected production preflight to call validate-release-blockers-external-only.ps1."
Assert-True ((Get-LastIndex "write-release-readiness-report.ps1") -gt $externalOnlyValidationIndex) "Expected production preflight to rewrite release readiness report after external-only validation."
Assert-True ($content -match [regex]::Escape("validate-development-continuation-readiness.ps1")) "Expected production preflight to call validate-development-continuation-readiness.ps1."
Assert-True ($content -match [regex]::Escape("write-release-input-template.ps1")) "Expected production preflight to call write-release-input-template.ps1."
Assert-True ($content -match [regex]::Escape("validate-release-input-template.ps1")) "Expected production preflight to call validate-release-input-template.ps1."
Assert-True ($content -match [regex]::Escape("write-release-input-values-template.ps1")) "Expected production preflight to call write-release-input-values-template.ps1."
Assert-True ($content -match [regex]::Escape("validate-release-input-values.ps1")) "Expected production preflight to call validate-release-input-values.ps1."
Assert-True ($content -match [regex]::Escape("resolve-release-input-template.ps1")) "Expected production preflight to call resolve-release-input-template.ps1."
Assert-True ($content -match [regex]::Escape("release-command-plan.template-check.json")) "Expected production preflight to write release-command-plan.template-check.json."
Assert-True ($content -match [regex]::Escape("validate-production-evidence-contracts.ps1")) "Expected production preflight to call validate-production-evidence-contracts.ps1."
Assert-True ($content -match [regex]::Escape("test-unreal-module-boundaries.ps1")) "Expected production preflight to call test-unreal-module-boundaries.ps1."
Assert-True ($content -match [regex]::Escape("test-unreal-baseline-entrypoints.ps1")) "Expected production preflight to call test-unreal-baseline-entrypoints.ps1."
Assert-True ($content -match [regex]::Escape("test-unreal-moba-foundation.ps1")) "Expected production preflight to call test-unreal-moba-foundation.ps1."
Assert-True ($content -match [regex]::Escape("validate-internal-api-route-protection.ps1")) "Expected production preflight to call validate-internal-api-route-protection.ps1."
Assert-True ($content -match [regex]::Escape("test-internal-api-route-protection-contract.ps1")) "Expected production preflight to call test-internal-api-route-protection-contract.ps1."
Assert-True ($content -match [regex]::Escape("test-player-id-claim-boundary-contract.ps1")) "Expected production preflight to call test-player-id-claim-boundary-contract.ps1."
Assert-True ($content -match [regex]::Escape("-RequireValid")) "Expected production preflight release validation checks to require valid derived reports."

Write-Host "PASS: production preflight release validation wiring" -ForegroundColor Green
