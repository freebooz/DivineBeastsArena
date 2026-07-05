<#
Validates repository-level production evidence automation contracts.

This script is intentionally lightweight so GitHub-hosted runners can verify
the contract shape even when they cannot execute UE packaged validation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function New-CodePointText {
    param(
        [Parameter(Mandatory = $true)][int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function New-TestEqualTextToken {
    param(
        [Parameter(Mandatory = $true)][string]$Message,
        [Parameter(Mandatory = $true)][string]$Expression
    )

    return 'TestEqual(TEXT("' + $Message + '"), ' + $Expression
}

$mainLobbyRecentKillsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x51FB, 0x6740, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Kills, 8)'
$mainLobbyRecentDeathsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6B7B, 0x4EA1, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Deaths, 1)'
$mainLobbyRecentAssistsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x52A9, 0x653B, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Assists, 6)'
$mainLobbyRecentDurationAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6301, 0x7EED, 0x65F6, 0x95F4, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.DurationSeconds, 420)'
$mainLobbyRecentCombatSummaryAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6218, 0x6597, 0x6458, 0x8981, 0x5E94, 0x88AB, 0x683C, 0x5F0F, 0x5316)) 'Summary.CombatSummary, FString(TEXT("KDA 8/1/6 / 07:00")))'
$mainLobbyRecentPlayedAtAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6E38, 0x73A9, 0x65F6, 0x95F4, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.PlayedAtUtc, FString(TEXT("2026-07-01T02:00:00Z")))'
$mainLobbyRecentExpDeltaAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x7ECF, 0x9A8C, 0x53D8, 0x5316, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.ExpDelta, static_cast<int64>(1200))'
$mainLobbyRecentCoinRewardAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x91D1, 0x5E01, 0x5956, 0x52B1, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.CoinReward, static_cast<int64>(12))'
$mainLobbyRecentHonorRewardAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x8363, 0x8A89, 0x5956, 0x52B1, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.HonorReward, static_cast<int64>(5))'
$mainLobbyRecentRewardSummaryAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x5956, 0x52B1, 0x6458, 0x8981, 0x5E94, 0x5305, 0x542B, 0x5168, 0x90E8, 0x6570, 0x503C, 0x5956, 0x52B1)) 'Summary.RewardSummary, FString(TEXT("coin +12 / gem +2 / honor +5")))'
$directExecutionPolicyContractStepText = New-CodePointText @(0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x5951, 0x7EA6)

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $missing = @($RequiredSymbols | Where-Object { $content -notmatch [regex]::Escape($_) })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing contract symbols: $($missing -join ', ')"
    }
}

function Assert-FileMatches {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredPatterns
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $missing = @($RequiredPatterns | Where-Object { $content -notmatch $_ })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing contract patterns: $($missing -join ', ')"
    }
}

function Assert-FileDoesNotContain {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$ForbiddenSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $present = @($ForbiddenSymbols | Where-Object { $content -match [regex]::Escape($_) })
    if ($present.Count -gt 0) {
        throw "$RelativePath contains forbidden contract symbols: $($present -join ', ')"
    }
}

function Assert-RepoDoesNotContain {
    param(
        [Parameter(Mandatory = $true)][string]$SearchRoot,
        [Parameter(Mandatory = $true)][string]$Pattern,
        [string[]]$Globs = @("*")
    )

    $fullRoot = Join-Path $repoRoot $SearchRoot
    if (-not (Test-Path -LiteralPath $fullRoot)) {
        throw "Search root is missing: $SearchRoot"
    }

    $args = @("-n", $Pattern, $fullRoot)
    foreach ($glob in $Globs) {
        $args += @("-g", $glob)
    }

    $matches = & rg @args
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed while scanning $SearchRoot for $Pattern"
    }

    if ($matches) {
        throw "$SearchRoot contains forbidden pattern '$Pattern':`n$($matches -join "`n")"
    }
}

Assert-FileContains "scripts\production-preflight.ps1" @(
    "CollectEvidence",
    "RequireReleaseReady",
    "EvidenceRoot",
    "RunId",
    "InternalApiKey",
    "UsePackagedUnrealServer",
    "release blocker actions",
    "diagnose-release-blockers.ps1",
    "release blocker action validation",
    "validate-release-blocker-actions.ps1",
    "release blockers external-only validation",
    "validate-release-blockers-external-only.ps1",
    "release readiness blocker posture refresh",
    "development continuation readiness validation",
    "validate-development-continuation-readiness.ps1",
    "release input template",
    "write-release-input-template.ps1",
    "release input template validation",
    "validate-release-input-template.ps1",
    "release input values template",
    "write-release-input-values-template.ps1",
    "release input values template validation",
    "validate-release-input-values.ps1",
    "release command plan template check",
    "resolve-release-input-template.ps1",
    "production evidence contracts",
    "validate-production-evidence-contracts.ps1",
    "Unreal module boundary fixtures",
    "test-unreal-module-boundaries.ps1",
    "Unreal baseline entrypoint fixtures",
    "test-unreal-baseline-entrypoints.ps1",
    "internal API route protection",
    "validate-internal-api-route-protection.ps1",
    "internal API route protection fixtures",
    "test-internal-api-route-protection-contract.ps1",
    "player_id claim boundary",
    "test-player-id-claim-boundary-contract.ps1",
    "Unreal Moba foundation",
    "validate-unreal-moba-foundation.ps1",
    "Unreal Moba foundation fixtures",
    "test-unreal-moba-foundation.ps1",
    "Resolve-InternalApiKey",
    "-EvidenceDir",
    "-RunId",
    "-UsePackagedServer",
    "collect-production-evidence.ps1",
    "write-release-readiness-report.ps1",
    "-RequireReady"
)

Assert-FileContains "scripts\start-local-ue-validation.ps1" @(
    "EvidenceDir",
    "RunId",
    "Write-UeValidationEvidence",
    "ue-online-validation",
    "RunNameSuffix",
    "RunNameSuffixSource",
    "runtimePlayerJoinedOk",
    "server player-joined backend success validation",
    "server player-joined backend failure scan"
)

Assert-FileContains "scripts\collect-production-evidence.ps1" @(
    "unreal.online_validation",
    '$ueOnlineEvidenceDescription',
    "ue-online-validation",
    "Test-UeOnlineValidationReadyEvidence",
    "hasRuntimeContext",
    "roomId",
    "sessionId",
    "serverId",
    "allocatedPort",
    "clientConnectPort",
    "processIds",
    "runtimePlayerJoinedOk",
    "clientATravelCompleted",
    "clientBTravelCompleted",
    "unreal.ai_showcase_automation",
    '$aiShowcaseEvidenceDescription',
    "ai-showcase-automation",
    "Test-AiShowcaseAutomationReadyEvidence",
    "RequiredAiShowcaseAutomationTestCount",
    "automationReady",
    "logErrorCount",
    "requestedTestCount",
    "passedTestCount",
    "downloadUrlHasHost",
    "Test-AbsoluteUrlWithHost",
    "TryCreate",
    "manifestUrlHasHost",
    "trustedSignedFileCount",
    "invalidSignedFileCount",
    "repair_game_downloads_local_package_and_persists_version",
    "exitCode",
    "buildExitCode",
    "screenshotExitCode",
    "domExitCode",
    "missingMarkers"
)

Assert-FileDoesNotContain "scripts\collect-production-evidence.ps1" @(
    "UE packaged or editor online validation",
    "AI_Showcase UI/VFX automation regression evidence"
)

Assert-FileContains ".github\workflows\unreal-evidence.yml" @(
    "workflow_dispatch",
    "self-hosted",
    "Windows",
    "Unreal",
    "DBA_INTERNAL_API_KEY",
    "run_ai_showcase_automation",
    "Run Unreal production evidence",
    "run-unreal-evidence.ps1",
    "PackageServer",
    "UsePackagedServer",
    "SkipAIShowcaseAutomation",
    "upload-artifact"
)

Assert-FileContains ".github\workflows\client-release-evidence.yml" @(
    "workflow_dispatch",
    "self-hosted",
    "Windows",
    "ClientRelease",
    "package_root",
    "download_url",
    "manifest_url",
    "sign_package",
    "certificate_thumbprint",
    "pfx_path",
    "DBA_CODE_SIGNING_PFX_PASSWORD",
    "diagnose-client-release-runner.ps1",
    "diagnose-client-release-prerequisites.ps1",
    "run-client-release-evidence.ps1",
    "collect-production-evidence.ps1",
    "write-release-readiness-report.ps1",
    "diagnose-release-blockers.ps1",
    "validate-release-blocker-actions.ps1",
    "validate-release-blockers-external-only.ps1",
    "validate-development-continuation-readiness.ps1",
    "write-release-input-template.ps1",
    "validate-release-input-template.ps1",
    "write-release-input-values-template.ps1",
    "validate-release-input-values.ps1",
    "resolve-release-input-template.ps1",
    "release-command-plan.template-check.json",
    "upload-artifact"
)

Assert-FileContains ".github\workflows\solution-ci.yml" @(
    "evidence-structure",
    "timeout-minutes",
    "actions/setup-python",
    "python -m pip install pyyaml",
    "test-production-evidence-automation.ps1",
    "actions/upload-artifact",
    "failure()",
    ".tmp",
    "retention-days"
)

Assert-FileContains "scripts\diagnose-client-release-runner.ps1" @(
    "client-release-runner-diagnostic",
    "PackageRoot",
    "DownloadUrl",
    "ManifestUrl",
    "SkipSigningProbe",
    "CertificateThumbprint",
    "CertificateSubject",
    "PfxPath",
    "DBA_CODE_SIGNING_PFX_PASSWORD",
    "client.package_root",
    "client.cooked_content",
    "release.prerequisites",
    "diagnose-client-release-prerequisites.ps1",
    "validate-production-evidence-contracts.ps1"
)

Assert-FileContains "scripts\test-client-release-runner-diagnostic.ps1" @(
    "diagnose-client-release-runner.ps1",
    "New-TextFromCodePoints",
    "client-release-runner-diagnostic-tests-{0}",
    '$successMessage',
    '$forbiddenFixedFixtureRootMessage',
    '$forbiddenOldSuccessMessage',
    "client.package_root",
    "release.prerequisites"
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
    "test-client-release-prerequisites.ps1",
    "test-client-release-runner-diagnostic.ps1",
    "test-production-evidence-collector.ps1",
    "test-client-package-url-policy.ps1",
    "test-client-cdn-payload-url-policy.ps1",
    "test-launcher-cdn-smoke-url-policy.ps1",
    "test-release-readiness-report.ps1",
    "test-release-blocker-actions.ps1",
    "test-release-blocker-action-validation.ps1",
    "test-release-blockers-external-only.ps1",
    "test-development-continuation-readiness.ps1",
    "test-release-input-template.ps1",
    "test-release-input-template-validation.ps1",
    "test-resolve-release-input-template.ps1",
    "test-write-release-input-values-template.ps1",
    "test-release-input-values-validation.ps1",
    "test-production-preflight-release-validation.ps1",
    "test-unreal-module-boundaries.ps1",
    "test-unreal-baseline-entrypoints.ps1",
    "test-unreal-source-guardrails.ps1",
    "test-agent-direct-execution-policy.ps1",
    "test-unreal-cpp-logic-blueprint-boundary.ps1",
    "test-unreal-data-asset-no-hardcoding-policy.ps1",
    "test-playable-skill-catalog-defaults-data-asset-contract.ps1",
    "test-zodiac-character-lobby-skill-data-asset-boundary.ps1",
    "test-unreal-ui-event-async-policy.ps1",
    "test-unreal-chinese-log-output-policy.ps1",
    "test-gas-ability-cpp-lifecycle-boundary.ps1",
    "test-runtime-player-join-build-summary-contract.ps1",
    "test-runtime-match-lifecycle-contract.ps1",
    "test-gamebackend-player-match-history-contract.ps1",
    "test-main-lobby-match-history-contract.ps1",
    "test-internal-api-route-protection-contract.ps1",
    "test-player-id-claim-boundary-contract.ps1",
    "test-session-connection-build-summary-contract.ps1",
    "test-dedicated-server-url-build-summary-admission-contract.ps1",
    "test-unreal-moba-foundation.ps1",
    "test-player-unit-frame-controller-contract.ps1",
    "test-player-unit-frame-widget-binding.ps1",
    "test-player-unit-frame-ultimate-energy-max-sync.ps1",
    "test-arena-hud-controller-player-unit-frame.ps1",
    "test-arena-hud-root-player-unit-frame-handoff.ps1",
    "test-arena-hud-ability-bar-character-binding.ps1",
    "test-arena-ability-bar-cooldown-slot-indexing.ps1",
    "test-arena-ability-bar-cooldown-event-sync.ps1",
    "test-arena-hud-ultimate-energy-sync.ps1",
    "test-arena-hud-ultimate-energy-default-constants.ps1",
    "test-arena-hud-ultimate-ready-prompt-sync.ps1",
    "test-arena-hud-chain-resonance-sync.ps1",
    "test-arena-hud-chain-resonance-constants.ps1",
    "test-arena-hud-chain-resonance-panel-boundaries.ps1",
    "test-arena-hud-aura-summary-panel-contract.ps1",
    "test-arena-hud-connection-warning-contract.ps1",
    "test-arena-hud-self-cast-bar-contract.ps1",
    "test-damage-calculator-chain-constants.ps1",
    "test-damage-calculator-chain-tier-semantics.ps1",
    "test-damage-calculator-element-count-constant.ps1",
    "test-damage-calculator-resonance-damage-constants.ps1",
    "test-defense-reduction-constant.ps1",
    "test-ability-system-resonance-constants.ps1",
    "test-ability-system-ultimate-passive-regen-constant.ps1",
    "test-ability-system-cooldown-slot-constants.ps1",
    "test-ability-system-input-activation-feedback.ps1",
    "test-ability-system-input-cooldown-authority-gate.ps1",
    "test-ability-system-avatar-actor-context-contract.ps1",
    "test-ability-system-target-teamid-cpp-boundary.ps1",
    "test-android-touch-input-bridge-server-boundary.ps1",
    "test-client-prediction-local-runtime-boundary.ps1",
    "test-lobby-player-controller-local-input-binding.ps1",
    "test-lobby-player-controller-local-skill-cast-boundary.ps1",
    "test-zodiac-character-local-skill-rpc-boundary.ps1",
    "test-zodiac-character-internal-cast-authority-boundary.ps1",
    "test-zodiac-character-legacy-cooldown-indexing.ps1",
    "test-zodiac-character-ability-cooldown-query.ps1",
    "test-rpc-handler-server-character-context.ps1",
    "test-rpc-handler-server-move-execution.ps1",
    "test-rpc-handler-server-lock-target-execution.ps1",
    "test-rpc-handler-stale-locked-target-clear.ps1",
    "test-rpc-handler-server-attack-execution.ps1",
    "test-rpc-handler-wrapper-validation.ps1",
    "test-rpc-handler-ability-cooldown-validation.ps1",
    "test-rpc-ability-input-semantic-boundary.ps1",
    "test-arena-hud-momentum-sync.ps1",
    "test-arena-hud-status-effects-sync.ps1",
    "test-arena-hud-event-feedback-sync.ps1",
    "test-arena-hud-event-feed-widget-sync.ps1",
    "test-game-ui-manager-arena-hud-controller.ps1",
    "test-game-ui-manager-arena-hud-runtime-updates.ps1",
    "test-game-ui-manager-arena-hud-entrypoint-server-boundaries.ps1",
    "test-game-ui-manager-arena-hud-hide-server-boundary.ps1",
    "test-game-ui-manager-main-lobby-entrypoint-server-boundary.ps1",
    "test-game-ui-manager-main-lobby-hide-server-boundary.ps1",
    "test-game-ui-manager-interaction-progress-server-boundary.ps1",
    "test-game-ui-manager-interaction-prompt-hide-server-boundary.ps1",
    "test-game-ui-manager-lobby-hud-server-boundary.ps1",
    "test-game-ui-manager-widget-factory-server-boundaries.ps1",
    "test-game-ui-manager-retry-timer-server-boundaries.ps1",
    "test-game-ui-manager-login-flow-state-server-boundary.ps1",
    "test-game-ui-manager-login-flow-start-server-boundary.ps1",
    "test-game-ui-manager-login-flow-server-boundary.ps1",
    "test-game-ui-manager-login-flow-hide-server-boundary.ps1",
    "test-game-ui-manager-input-mode-restore-server-boundary.ps1",
    "test-game-ui-manager-splash-video-timer-server-boundary.ps1",
    "test-game-ui-manager-splash-video-hide-server-boundary.ps1",
    "test-zodiac-character-arena-hud-sync.ps1",
    "test-zodiac-character-arena-hud-sync-cache.ps1",
    "test-zodiac-character-arena-hud-attribute-delegates.ps1",
    "test-zodiac-character-arena-hud-critical-state.ps1",
    "test-zodiac-character-arena-hud-ultimate-ready-prompt.ps1",
    "test-playable-skill-catalog-defaults-data-asset-contract.ps1",
    "test-zodiac-character-lobby-skill-data-asset-boundary.ps1",
    "test-zodiac-character-ultimate-energy-constants.ps1",
    "test-zodiac-character-skill-slot-count-constants.ps1",
    "test-zodiac-character-gas-input-activation-bridge.ps1",
    "test-zodiac-character-server-cast-authority-boundary.ps1",
    "test-zodiac-character-server-cast-rpc-validation.ps1",
    "test-zodiac-character-gas-skill-feedback-hud-announcement.ps1",
    "test-skill-vfx-damage-authority-boundary.ps1",
    "test-skill-projectile-damage-authority-boundary.ps1",
    "test-skill-projectile-cpp-hit-boundary.ps1",
    "test-skill-projectile-hit-entrypoint-cpp-only.ps1",
    "test-skill-projectile-runtime-entrypoints-cpp-only.ps1",
    "test-chain-lightning-damage-authority-boundary.ps1",
    "test-damage-calculator-authority-boundary.ps1",
    "test-healing-shield-authority-boundary.ps1",
    "test-ability-system-state-authority-boundary.ps1",
    "test-zodiac-character-fallback-state-authority-boundary.ps1",
    "test-zodiac-character-death-team-authority-boundary.ps1",
    "test-zodiac-character-death-finalize-timer-boundary.ps1",
    "test-zodiac-character-death-idempotent-boundary.ps1",
    "test-unreal-ui-runtime-chinese-output-contract.ps1",
    "test-player-state-match-stats-authority-boundary.ps1",
    "test-monster-ai-state-authority-boundary.ps1",
    "test-monster-ai-movement-authority-boundary.ps1",
    "test-zodiac-ultimate-energy-cost-constants.ps1",
    "test-zodiac-character-arena-hud-chain-announcement.ps1",
    "test-fixed-skill-group-datatable-diagnostic.ps1",
    "test-data-table-count-constants.ps1",
    "test-fixed-skill-group-asset-test-constants.ps1",
    "test-fixed-skill-group-source-csv.ps1",
    "test-fixed-skill-group-datatable-import.ps1",
    "diagnose-fixed-skill-group-datatable.ps1",
    "validate-internal-api-route-protection.ps1",
    "test-player-id-claim-boundary-contract.ps1",
    "resolve-release-input-template.ps1",
    "write-release-input-values-template.ps1",
    "validate-release-input-values.ps1",
    "validate-unreal-moba-foundation.ps1",
    "write-fixed-skill-group-source-csv.ps1",
    "import-fixed-skill-group-datatable.ps1",
    "changed the working directory",
    "validate-production-evidence-contracts.ps1",
    "validate-release-blocker-actions.ps1",
    "validate-release-blockers-external-only.ps1",
    "client-release-evidence.yml",
    "unreal-evidence.yml",
    "directExecutionPolicyContractStepText",
    "New-CodePointText @(0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x5951, 0x7EA6)",
    "PASS: production evidence automation tests"
)

Assert-FileContains "docs\Development\README.md" @(
    "validate-unreal-source-guardrails.ps1",
    "test-unreal-source-guardrails.ps1",
    "validate-unreal-baseline-entrypoints.ps1",
    "validate-unreal-moba-foundation.ps1",
    "test-production-evidence-automation.ps1",
    "diagnose-fixed-skill-group-datatable.ps1",
    "write-fixed-skill-group-source-csv.ps1",
    "import-fixed-skill-group-datatable.ps1",
    "fixed skill group",
    "DataTable",
    "baseline validator",
    "player-joined",
    "Session travel URL",
    "URL admission",
    "Runtime"
)

Assert-FileContains "scripts\test-production-evidence-collector.ps1" @(
    "collect-production-evidence.ps1",
    "New-TextFromCodePoints",
    '$successMessage',
    '$expectedCollectorManifestMessageContract',
    '$expectedCollectorMissingTitleContract',
    '$expectedCollectorReadyMessageContract',
    '$forbiddenOldCollectorManifestMessage',
    '$forbiddenOldCollectorMissingTitle',
    '$forbiddenOldCollectorReadyMessage',
    "failed AI_Showcase automation evidence",
    "stale 4/4 AI_Showcase automation evidence",
    "failed UE online validation evidence",
    "vulnerable npm audit evidence",
    "clean Admin/Website/Launcher npm audit evidence",
    "vulnerable Trivy evidence",
    "clean API/Worker Trivy evidence",
    "vulnerable NuGet evidence",
    "clean NuGet evidence",
    "failed k6 evidence",
    "clean login/matchmaking k6 evidence",
    "client download metadata not to match load.k6",
    "mixed k6 evidence",
    "failed, skipped, and metadata k6 files to be excluded",
    "clean login/matchmaking k6 JSON and log",
    "mixed-launcher-install-update-evidence",
    "failed and stderr launcher install/update support files",
    "clean launcher install/update JSON and stdout log",
    "mixed-launcher-ui-visual-evidence",
    "failed and stderr launcher UI visual support files",
    "clean launcher UI visual JSON, screenshot, and non-stderr logs",
    "failed backup restore rehearsal evidence",
    "backup restore evidence without a restore database",
    "clean backup restore rehearsal evidence",
    "mixed backup restore evidence",
    "incomplete backup restore files to be excluded",
    "clean backup restore JSON and log",
    "failed deploy rollback smoke evidence",
    "deploy rollback smoke evidence without metrics check",
    "clean deploy rollback smoke evidence",
    "inconsistent release prerequisite evidence",
    "hostless package launcher evidence",
    "hostless launcher CDN smoke evidence",
    "inconsistent code signing evidence",
    "inconsistent launcher install/update evidence",
    "inconsistent launcher UI visual evidence",
    "inconsistent UE online validation evidence",
    "trustedSignedFileCount",
    "exitCode",
    "screenshotExitCode",
    "clientConnectPort",
    "processIds",
    "https://",
    "release-readiness-report.json",
    "release-readiness-report.md",
    "release-blocker-actions.json",
    "release-blocker-actions.md",
    "release-blocker-action-validation.json",
    "release-blockers-external-only-validation.json",
    "development-continuation-readiness-validation.json",
    "release-input-template.json",
    "release-input-template.md",
    "release-input-template-validation.json",
    "release-input-values.template.json",
    "release-input-values.template.md",
    "release-input-values-validation.json",
    "release-command-plan.template-check.json",
    "release-command-plan.template-check.md"
)

Assert-FileContains "scripts\collect-production-evidence.ps1" @(
    "New-TextFromCodePoints",
    '$manifestWrittenMessage',
    '$missingEvidenceTitleMessage',
    '$allEvidencePresentMessage',
    '$missingEvidenceFailureMessage',
    "production-evidence-manifest.json",
    "security.nuget",
    "client.release_prerequisites"
)

Assert-FileContains "scripts\test-release-readiness-report.ps1" @(
    "write-release-readiness-report.ps1",
    "New-TextFromCodePoints",
    "release-readiness-report-tests-{0}",
    '$successMessage',
    '$forbiddenFixedTestRootMessage',
    '$forbiddenOldSuccessMessage',
    "blockingRequirementCount",
    "presentRequirementCount",
    "developmentContinuationReady",
    "releaseBlockerPosture",
    "releaseIdMatchesManifest",
    "blockerCountMatchesManifest",
    "release-readiness-report-stale-posture",
    "release-readiness-report-count-mismatch",
    "externalOnlyReleaseBlockers",
    "RequireReady"
)

Assert-FileContains "scripts\diagnose-release-blockers.ps1" @(
    "release-blocker-actions",
    "release-readiness-report.json",
    "observedReasons",
    "observedReasonCount",
    "latestEvidencePath",
    "missingExternalInputs",
    "automationBlocked",
    "blockingExternalInputs",
    "inputResolutionHints",
    "Get-InputResolutionHints",
    "Input resolution hints",
    "Automation status",
    "Missing external inputs",
    "MaxObservedReasonsPerBlocker",
    "ModifiedAtUtc",
    "Sort-Object ModifiedAtUtc -Descending",
    "showing",
    "Get-ObservedReasons",
    "blockingIssues",
    "releaseReadinessNotes",
    "signingReadinessNotes",
    "manifestUrlIsHttps",
    "unsignedFileCount",
    "client.release_prerequisites",
    "diagnose-client-release-prerequisites.ps1",
    "-RequireSigningIdentity",
    "-RequireSignTool",
    "client.package_launcher",
    "run-client-release-evidence.ps1",
    "client.cdn_launcher_smoke",
    "run-launcher-cdn-smoke.ps1",
    "client.code_signing",
    "sign-client-release-package.ps1",
    "unreal.ai_showcase_automation",
    "run-ai-showcase-automation.ps1",
    "RequiredAiShowcaseAutomationTestCount",
    "automationReady",
    "logErrorCount=0",
    'requestedTestCount=$RequiredAiShowcaseAutomationTestCount',
    'passedTestCount=$RequiredAiShowcaseAutomationTestCount',
    "release run id",
    "unblockCriteria"
)

Assert-FileContains "scripts\test-release-blocker-actions.ps1" @(
    "diagnose-release-blockers.ps1",
    "observedReasons",
    "Observed reasons",
    "observedReasonCount",
    "latestEvidencePath",
    "missingExternalInputs",
    "automationBlocked",
    "blockingExternalInputs",
    "inputResolutionHints",
    "Input resolution hints",
    "Expected package input hint to include -PackageRoot",
    "Expected CDN input hint to include -ManifestUrl",
    "Expected signing input hint to include certificate parameters",
    "unreal.ai_showcase_automation",
    "run-ai-showcase-automation.ps1",
    "automationReady=False",
    "requestedTestCount=4",
    "passedTestCount=4",
    "requestedTestCount=5",
    "passedTestCount=5",
    "Expected AI_Showcase automation action",
    "Expected AI_Showcase stale 4/4 test count observed reasons",
    "Expected AI_Showcase unblock criteria to require the current 5/5 automation suite",
    "Automation status",
    "automationBlocked: True",
    "Missing external inputs",
    "latest evidence: client/client-package-launcher-new.json",
    "real HTTPS CDN manifest URL",
    "trusted Authenticode signing identity",
    "showing 5 of",
    "client-package-launcher-new.json",
    "Expected newest package evidence to be reported first",
    "Expected package action to route through release evidence bundle with valid placeholders",
    "Expected prerequisite diagnostic action to include executable release input placeholders",
    "Expected CDN smoke action with valid placeholders",
    "<public-shipping-package-root>",
    "<real-https-cdn-download-url>",
    "<real-https-cdn-manifest-url>",
    "download_url_example",
    "unsignedFileCount=7",
    "client.release_prerequisites",
    "client.package_launcher",
    "client.cdn_launcher_smoke",
    "client.code_signing",
    "PASS: release blocker action fixtures"
)

Assert-FileContains "scripts\validate-release-blocker-actions.ps1" @(
    "release-blocker-action-validation",
    "release-blocker-actions.json",
    "RequireValid",
    "reportKind",
    "kindIsValid",
    "linkedReportPath",
    "reportPathIsPresent",
    "reportPathExists",
    "linkedReportKind",
    "reportKindIsValid",
    "reportReleaseId",
    "reportReleaseIdMatches",
    "inputResolutionHints",
    "Get-Placeholder",
    "missingPlaceholders",
    "missingScripts",
    "missingActionKeyCount",
    "missingActionCommandCount",
    "duplicateActionKeyCount",
    "invalidParameters",
    "invalidParameterCount",
    "scriptMismatchCount",
    "scriptMismatches",
    "reportedBlockerCount",
    "blockerCountIsPresent",
    "blockerCountMatchesActions",
    "Get-CommandScriptPath",
    "manualActionCount",
    "manualActions",
    "Get-ScriptDeclaredParameters",
    "Get-CommandParameters",
    "Release blocker action validation"
)

Assert-FileContains "scripts\test-release-blocker-action-validation.ps1" @(
    "validate-release-blocker-actions.ps1",
    "release-blocker-action-validation",
    "<undeclared-placeholder>",
    "-NoSuchParam",
    "RequireValid",
    "Expected invalid blocker action validation to fail",
    "Expected invalid parameter validation detail",
    "Expected script mismatch validation detail",
    "Expected manual blocker actions to be counted but not fail validation",
    "release-blocker-actions-count-mismatch",
    "Expected blocker count mismatch validation to fail",
    "blockerCountMatchesActions",
    "release-blocker-actions-wrong-kind",
    "Expected wrong-kind action validation to fail",
    "kindIsValid",
    "release-blocker-actions-missing-count",
    "Expected missing blocker count validation to fail",
    "blockerCountIsPresent",
    "release-blocker-actions-malformed-fields",
    "Expected malformed action fields validation to fail",
    "missingActionKeyCount",
    "missingActionCommandCount",
    "release-blocker-actions-duplicate-key",
    "Expected duplicate action key validation to fail",
    "duplicateActionKeyCount",
    "release-blocker-actions-missing-report-path",
    "Expected missing reportPath validation to fail",
    "reportPathIsPresent",
    "release-blocker-actions-missing-report-file",
    "Expected missing linked readiness report file validation to fail",
    "reportPathExists",
    "release-blocker-actions-release-id-mismatch",
    "Expected linked readiness report releaseId mismatch validation to fail",
    "reportReleaseIdMatches",
    "PASS: release blocker action validation fixtures"
)

Assert-FileContains "scripts\validate-release-blockers-external-only.ps1" @(
    "release-blockers-external-only-validation",
    "release-blocker-actions.json",
    "RequireValid",
    "externalOnly",
    "externalBlockerCount",
    "reportedBlockerCount",
    "blockerCountMatchesActions",
    "localAutomationBlockerCount",
    "emptyExternalInputBlockerCount",
    "duplicateActionKeyCount",
    "duplicateActionKeys",
    "automationBlocked",
    "blockingExternalInputs",
    "New-TextFromCodePoints",
    '$jsonWrittenMessage',
    '$validMessage',
    '$invalidMessage',
    '$requireValidFailureMessage'
)

Assert-FileContains "scripts\test-release-blockers-external-only.ps1" @(
    "validate-release-blockers-external-only.ps1",
    "release-blockers-external-only-validation",
    "unreal.ai_showcase_automation",
    "client.package_launcher",
    "release-blocker-actions-count-mismatch",
    "release-blocker-actions-duplicate-key",
    "blockerCountMatchesActions",
    "localAutomationBlockerCount",
    "emptyExternalInputBlockerCount",
    "duplicateActionKeyCount",
    "Expected local automation blocker fixture to fail under -RequireValid",
    "Expected empty external-input blocker fixture to fail under -RequireValid",
    "Expected blocker count mismatch fixture to fail under -RequireValid",
    "Expected duplicate blocker key fixture to fail under -RequireValid",
    '$successMessage',
    '$expectedJsonMessageContract',
    '$expectedValidMessageContract',
    '$expectedInvalidMessageContract',
    '$expectedRequireValidMessageContract',
    '$forbiddenOldJsonMessageContract',
    '$forbiddenOldValidMessageContract',
    '$forbiddenOldInvalidMessageContract',
    '$forbiddenOldRequireValidMessageContract'
)

Assert-FileContains "scripts\validate-development-continuation-readiness.ps1" @(
    "development-continuation-readiness-validation",
    "release-readiness-report.json",
    "developmentContinuationReady",
    "reportedDevelopmentContinuationReady",
    "computedDevelopmentContinuationReady",
    "manifestReleaseId",
    "reportMatchesManifest",
    "postureKindIsValid",
    "postureReleaseIdMatchesReport",
    "postureBlockerCountMatchesBlockingRequirements",
    "readyForRelease",
    "localAutomationBlockerCount",
    "RequireReady",
    "New-TextFromCodePoints",
    '$jsonWrittenMessage',
    '$readyMessage',
    '$blockedMessage',
    '$blockedRequireReadyMessage'
)

Assert-FileContains "scripts\test-development-continuation-readiness.ps1" @(
    "validate-development-continuation-readiness.ps1",
    "development-continuation-readiness-validation",
    "Expected external-only fixture to pass development continuation",
    "Expected local automation blocker fixture to fail under -RequireReady",
    "inconsistent-development-ready",
    "Expected validator to recompute and reject inconsistent development continuation readiness",
    "stale-development-ready",
    "Expected stale report validation to record manifest mismatch",
    "stale-posture-development-ready",
    "Expected stale release blocker posture to fail under -RequireReady when posture releaseId differs",
    "postureReleaseIdMatchesReport",
    "posture-count-mismatch-development-ready",
    "Expected posture blocker count mismatch to fail under -RequireReady",
    "postureBlockerCountMatchesBlockingRequirements",
    '$successMessage',
    '$expectedJsonMessageContract',
    '$expectedReadyMessageContract',
    '$expectedBlockedMessageContract',
    '$expectedBlockedRequireReadyContract',
    '$forbiddenOldJsonMessageContract',
    '$forbiddenOldReadyMessageContract',
    '$forbiddenOldBlockedMessageContract',
    '$forbiddenOldRequireReadyMessageContract'
)

Assert-FileContains "scripts\write-release-input-template.ps1" @(
    "release-input-template",
    "release-blocker-actions.json",
    "inputResolutionHints",
    "inputNames",
    "automationBlocked",
    "blockedBy",
    "primaryParameter",
    "placeholder",
    "suggestedCommands",
    "Suggested commands",
    "release run id",
    "run-ai-showcase-automation",
    "run-client-release-evidence",
    "run-launcher-cdn-smoke",
    "sign-client-release-package",
    "Release input template"
)

Assert-FileContains "scripts\test-release-input-template.ps1" @(
    "write-release-input-template.ps1",
    "release-input-template",
    "release run id",
    "GITHUB_RUN_ID",
    "public Shipping package root",
    "real HTTPS CDN manifest URL",
    "trusted Authenticode signing identity",
    "-RunId",
    "-PackageRoot",
    "-ManifestUrl",
    "-CertificateSubject",
    "suggestedCommands",
    "Suggested commands",
    "Expected AI_Showcase suggested command",
    "Expected no suggested commands when there are no blockers",
    "release input template no-blocker fixture",
    "release input template partial-input fixture",
    "Expected partial template not to suggest client release evidence without package and download inputs",
    "Expected partial template not to suggest CDN smoke without an install root input",
    "Expected release evidence suggested command with placeholders",
    "Expected CDN smoke suggested command",
    "Expected signing suggested command",
    "release input template prerequisite fixture",
    "diagnose-client-release-prerequisites",
    "Expected prerequisite template to suggest a complete release prerequisite diagnostic command",
    "PASS: release input template fixtures"
)

Assert-FileContains "scripts\validate-release-input-template.ps1" @(
    "release-input-template-validation",
    "release-input-template.json",
    "RequireValid",
    "missingPlaceholders",
    "missingScripts",
    "invalidParameters",
    "missingInputReferences",
    "missingCommandInputReferences",
    "missingCompatibleInputReferences",
    "asymmetricCompatibleInputReferences",
    "usesInputs",
    "compatibleInputs",
    "declaredInputCount",
    "actualInputCount",
    "inputCountMatches",
    "invalidParameterCount",
    "missingInputReferenceCount",
    "missingCommandInputReferenceCount",
    "missingCompatibleInputReferenceCount",
    "asymmetricCompatibleInputReferenceCount",
    "scriptCount",
    "Get-ScriptDeclaredParameters",
    "Get-CommandParameters",
    "placeholderCount",
    "suggestedCommands",
    "Release input template validation"
)

Assert-FileContains "scripts\test-release-input-template-validation.ps1" @(
    "validate-release-input-template.ps1",
    "release-input-template-validation",
    "release-input-template-invalid-uses-input",
    "release-input-template-invalid-command-uses-input",
    "release-input-template-invalid-compatible-input",
    "release-input-template-invalid-asymmetric-compatible-input",
    "release-input-template-invalid-input-count",
    "<missing-install-root>",
    "-NoSuchParam",
    "RequireValid",
    "Expected invalid template validation to fail",
    "Expected invalid parameter validation to fail",
    "Expected invalid usesInputs validation to fail",
    "Expected command placeholder usesInputs validation to fail",
    "Expected invalid compatibleInputs validation to fail",
    "Expected asymmetric compatibleInputs validation to fail",
    "Expected invalid inputCount validation to fail",
    "Expected missing usesInputs detail",
    "Expected command placeholder usesInputs detail",
    "Expected missing compatibleInputs detail",
    "Expected asymmetric compatibleInputs detail",
    "inputCountMatches",
    "Expected invalid parameter detail",
    "PASS: release input template validation fixtures"
)

Assert-FileContains "scripts\resolve-release-input-template.ps1" @(
    "release-command-plan",
    "release-input-template.json",
    "release input values JSON",
    "RequireComplete",
    "Test-ReleaseInputValues",
    "valuesValidation",
    "valuesValid",
    "blankValueCount",
    "placeholderValueCount",
    "exampleUrlCount",
    "insecureUrlCount",
    "missingInputs",
    "unresolvedPlaceholders",
    "unresolvedPlaceholderCount",
    "suggestedCommands",
    "usesInputs",
    "Release command plan"
)

Assert-FileContains "scripts\test-resolve-release-input-template.ps1" @(
    "resolve-release-input-template.ps1",
    "release-input-values",
    "release-command-plan",
    "diagnose-client-release-prerequisites",
    "Expected complete values to produce a complete command plan",
    "Expected incomplete values to fail under -RequireComplete",
    "Expected invalid values to fail command plan completion under -RequireComplete",
    "Expected command plan to include invalid values validation",
    "valuesValidation",
    "exampleUrlCount",
    "insecureUrlCount",
    "Expected missing manifest input detail",
    "PASS: resolve release input template fixtures"
)

Assert-FileContains "scripts\write-release-input-values-template.ps1" @(
    "release-input-values",
    "release-input-template.json",
    "isTemplate",
    "compatibleInputs",
    "blockedBy",
    "primaryParameter",
    "Release input values template"
)

Assert-FileContains "scripts\test-write-release-input-values-template.ps1" @(
    "write-release-input-values-template.ps1",
    "release-input-values.template.json",
    "release-input-values.template.md",
    "resolve-release-input-template.ps1",
    "Expected blank values template to fail command plan completion",
    "PASS: release input values template fixtures"
)

Assert-FileContains "scripts\validate-release-input-values.ps1" @(
    "release-input-values-validation",
    "release-input-values.json",
    "RequireValid",
    "blankValues",
    "placeholderValues",
    "exampleUrls",
    "insecureUrls",
    "blankValueCount",
    "placeholderValueCount",
    "exampleUrlCount",
    "insecureUrlCount",
    "inputCountMatches",
    "Release input values validation"
)

Assert-FileContains "scripts\test-release-input-values-validation.ps1" @(
    "validate-release-input-values.ps1",
    "release-input-values-validation",
    "Expected blank release input values to fail",
    "Expected example release input values to fail",
    "Expected complete values to be valid",
    "cdn.example.com",
    "placeholderValueCount",
    "exampleUrlCount",
    "insecureUrlCount",
    "PASS: release input values validation fixtures"
)

Assert-FileContains "scripts\test-production-preflight-release-validation.ps1" @(
    "production-preflight.ps1",
    "release blocker actions",
    "release blocker action validation",
    "release blockers external-only validation",
    "release input template",
    "release input template validation",
    "release input values template",
    "release input values template validation",
    "release command plan template check",
    "release readiness blocker posture refresh",
    "production evidence contracts",
    "Unreal module boundary fixtures",
    "Unreal baseline entrypoint fixtures",
    "Unreal Moba foundation fixtures",
    "internal API route protection",
    "internal API route protection fixtures",
    "player_id claim boundary",
    "diagnose-release-blockers.ps1",
    "validate-release-blocker-actions.ps1",
    "validate-release-blockers-external-only.ps1",
    "validate-development-continuation-readiness.ps1",
    "write-release-input-template.ps1",
    "validate-release-input-template.ps1",
    "write-release-input-values-template.ps1",
    "validate-release-input-values.ps1",
    "resolve-release-input-template.ps1",
    "validate-production-evidence-contracts.ps1",
    "test-unreal-module-boundaries.ps1",
    "test-unreal-baseline-entrypoints.ps1",
    "validate-internal-api-route-protection.ps1",
    "test-internal-api-route-protection-contract.ps1",
    "test-player-id-claim-boundary-contract.ps1",
    "test-unreal-moba-foundation.ps1",
    "PASS: production preflight release validation wiring"
)

Assert-FileContains "scripts\test-unreal-module-boundaries.ps1" @(
    "validate-unreal-module-boundaries.ps1",
    "-ClientSourceRoot",
    "GameCore reverse dependency fixture",
    "GameMoba reverse dependency fixture",
    "quoted include boundary fixture",
    "angle include boundary fixture",
    "parent include fixture",
    "PASS: Unreal module boundary fixtures"
)

Assert-FileContains "scripts\test-unreal-baseline-entrypoints.ps1" @(
    "validate-unreal-baseline-entrypoints.ps1",
    "-ClientSourceRoot",
    "missing log category fixture",
    "duplicate log category fixture",
    "missing travel validation fixture",
    "fixed skill group identity fixture",
    "PASS: Unreal baseline entrypoint fixtures"
)

Assert-FileContains "scripts\run-unreal-evidence.ps1" @(
    "diagnose-unreal-evidence-runner.ps1",
    "diagnose-unreal-packaged-server-readiness.ps1",
    "package-unreal-dedicated-server.ps1",
    "run-ai-showcase-automation.ps1",
    "EvidenceDir",
    "RunId",
    "start-local-ue-validation.ps1",
    "collect-production-evidence.ps1",
    "PackageServer",
    "UsePackagedServer",
    "SkipClientLaunch",
    "SkipAIShowcaseAutomation",
    "Resolve-InternalApiKey"
)

Assert-FileContains "scripts\run-ai-showcase-automation.ps1" @(
    "UnrealEditor-Cmd.exe",
    "-ExecCmds=Automation RunTests",
    "-TestExit=Automation Test Queue Empty",
    "DivineBeastsArena.Showcase.AIShowcase",
    "EvidenceDir",
    "RunId",
    "LogPath",
    "ai-showcase-automation",
    "automationReady",
    "logErrorCount",
    "logWarningCount",
    "requestedTestCount",
    "passedTestCount"
)

Assert-FileContains "scripts\test-ai-showcase-widget-tree-contract.ps1" @(
    "New-CodePointText",
    "AIShowcaseWidgetTreeContract",
    "AIShowcaseMenu_TitleText",
    "AIShowcaseMenu_StartButton",
    "AIShowcaseHUD_HealthBar",
    "AIShowcaseHUD_EnergyBar",
    "AIShowcaseHUD_EventFeedBox",
    '$evidence\.requestedTestCount -ne 5',
    '$passText'
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
    "AI_Showcase widget tree contract",
    "test-ai-showcase-widget-tree-contract.ps1"
)

Assert-FileContains "scripts\test-runtime-player-join-build-summary-contract.ps1" @(
    "Runtime player-joined build summary contract",
    "RuntimePlayerJoinedRequest",
    "RuntimePlayerJoinValidator",
    "BuildSummaryMismatchMessage",
    "ValidateBuildSummary_WhenFixedSkillGroupIsTampered_ReturnsFalse",
    "ValidateBuildSummary_WhenFiveCampChangesButSkillGroupMatches_ReturnsTrue",
    "BuildPlayerJoinedEventPayload_WhenFiveCampRequestDiffers_UsesFrozenBuildSummary",
    "TEXT(""fiveCamp"")",
    "TEXT(""fixedSkillGroupId"")",
    "PASS: runtime player-joined build summary contract"
)

Assert-FileContains "scripts\test-runtime-match-lifecycle-contract.ps1" @(
    "Runtime match lifecycle contract",
    "RuntimeMatchStartedRequest",
    "RuntimeMatchEndedRequest",
    "RuntimeMatchResultsRequest",
    "RuntimePlayerResultDto",
    "RuntimeMatchResults",
    "RuntimeMatchResultsValidator.cs",
    "RuntimeMatchResultsValidator",
    "ValidateAndBuildPayload",
    'internalGroup.MapPost("/servers", AllocateServer)',
    'internalGroup.MapGet("/servers/{serverId}", GetServer)',
    'internalGroup.MapPost("/from-room", CreateFromRoom)',
    'internalGroup.MapPost("/{sessionId}/allocate-server", AllocateServer)',
    'internalGroup.MapPost("/{sessionId}/mark-in-progress", MarkInProgress)',
    "InternalApiKeyEndpointFilter.Validate(httpContext)",
    "RuntimeLifecycleService.cs",
    "RuntimeLifecycleService",
    "InternalApiKeyEndpointFilter.cs",
    "InternalApiKeyEndpointFilter",
    'HeaderName = "X-Internal-Api-Key"',
    "Validate(HttpContext httpContext)",
    "Invalid internal api key",
    'app.MapGroup("/internal/game-servers")',
    'app.MapGroup("/internal/servers")',
    "MarkMatchStartedAsync",
    "MarkMatchEndedAsync",
    "public partial class Program",
    "x.Id == serverId && x.SessionId == sessionId",
    "hasActivePlayerNotJoined",
    "x.LeftAt == null",
    'x.JoinedAt == null || x.Status != "JOINED"',
    'session.Status is not ("IN_PROGRESS" or "SETTLING")',
    'server.Status is not ("IN_PROGRESS" or "ENDING")',
    "AddSessionEventOnceAsync",
    "AddServerEventOnceAsync",
    "GetMatchResultAsync",
    "GetSessionResultsAsync",
    "string.IsNullOrWhiteSpace(request.IdempotencyKey)",
    "request.IdempotencyKey.Trim()",
    "existingForSession",
    "existingForIdempotencyKey",
    "x.SessionId == request.SessionId",
    "x.IdempotencyKey == idempotencyKey",
    "IdempotencyKey = idempotencyKey",
    "submittedPlayers",
    "sessionPlayers.Count == 0 || request.Players.Count == 0",
    "request.Players.Count != submittedPlayers.Count",
    "SetEquals",
    "playerResults",
    "NormalizePlayerResult(x.Result)",
    "!IsValidPlayerResult(x)",
    "HasValidNonNegativePlayerValues(x)",
    "IsNonNegativeRewardQuantity",
    "player.Rewards.All",
    "Result = playerResults[player.PlayerId]",
    'result is "win" or "loss" or "draw"',
    "sessionPlayerTeams",
    "NormalizeTeam(x.Team)",
    "sessionPlayerTeams[x.PlayerId].Length == 0",
    'session.Status != "SETTLING"',
    "Include(x => x.PlayerResults)",
    "OrderByDescending(x => x.CreatedAt)",
    "SettlementEndpoints.cs",
    'internalGroup.MapGet("/sessions/{sessionId}/matches/results", GetSessionResults)',
    "svc.GetSessionResultsAsync(sessionId)",
    "ToResponse(result)",
    "result.PlayerResults.Select",
    "MissingIdempotencyKeyMessage",
    "MissingPlayersMessage",
    "DuplicatePlayersMessage",
    "UnknownPlayersMessage",
    "MissingSessionPlayersMessage",
    "MissingPlayerTeamMessage",
    "TeamMismatchMessage",
    "InvalidPlayerResultMessage",
    "InvalidPlayerNumericValueMessage",
    ".Select(x => new { x.PlayerId, x.Team })",
    "ToDictionaryAsync(x => x.PlayerId, x => x.Team)",
    "IReadOnlyDictionary<Guid, string?> sessionPlayerTeams",
    "sessionPlayerTeams.ContainsKey",
    "NormalizeTeam",
    "NormalizePlayerResult(player.Result)",
    "!IsValidPlayerResult",
    "HasValidNonNegativePlayerValues(player)",
    "IsNonNegativeRewardQuantity",
    "player.Rewards.All",
    "NormalizePlayerResult(x.Result)",
    'result is "win" or "loss" or "draw"',
    "SettlementSubmitMatchResultRequest",
    "settlement.SubmitMatchResultAsync(validation.Payload)",
    "RuntimeMatchResultsValidatorTests.cs",
    "RuntimeLifecycleServiceTests.cs",
    "SettlementEndpointsTests.cs",
    "RuntimeEndpointsTests.cs",
    "SessionEndpointsTests.cs",
    "GameServerEndpointsTests.cs",
    "RuntimeMatchResultsValidatorTests",
    "RuntimeLifecycleServiceTests",
    "SettlementEndpointsTests",
    "RuntimeEndpointsTests",
    "SessionEndpointsTests",
    "GameServerEndpointsTests",
    "GetInternalServer_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized",
    "GetInternalServer_WithInternalApiKey_ReachesRuntimeHandler",
    "RuntimePlayerJoined_WithoutPlayerSessionToken_ReturnsUnauthorizedWithoutJoining",
    "RuntimePlayerJoined_WithExpiredPlayerSessionToken_ReturnsUnauthorizedWithoutJoining",
    "string.IsNullOrWhiteSpace(playerSession.SessionTokenHash)",
    "playerSession.SessionTokenExpiresAt <= DateTimeOffset.UtcNow",
    "string.IsNullOrWhiteSpace(request.PlayerSessionToken)",
    "Invalid player session token",
    "RuntimePlayerLeft",
    "if (playerSession is null) return ErrorResponse.NotFound(ErrorCodes.SessionPlayerNotInSession).ToProblem()",
    "if (playerSession.LeftAt is not null)",
    'playerSession.JoinedAt is null || playerSession.Status != "JOINED"',
    "Player has not joined session",
    'AddSessionEvent(db, request.SessionId, "PLAYER_LEFT"',
    "PlayerSessionToken: null",
    "playerSessionTokenExpiresAt: DateTimeOffset.UtcNow.AddMinutes(-1)",
    'Assert.Equal("CONNECTED", playerSession.Status)',
    "Assert.Null(playerSession.JoinedAt)",
    'x.EventType == "PLAYER_JOINED"',
    "RuntimePlayerLeft_WhenPlayerIsNotInSession_ReturnsNotFoundWithoutWritingEvent",
    "RuntimePlayerLeft_WhenPlayerHasNotJoined_ReturnsBadRequestWithoutWritingEvent",
    "RuntimePlayerLeft_WhenRepeated_DoesNotRewriteLeftAtOrDuplicateEvent",
    'playerSessionStatus: "CONNECTED"',
    "Assert.Null(playerSession.LeftAt)",
    'JoinedAt = playerSessionStatus == "JOINED"',
    '"/runtime/servers/player-left"',
    "unknownPlayerId",
    "firstLeftAt",
    "Assert.Equal(1, await db.SessionEvents.CountAsync",
    'x.EventType == "PLAYER_LEFT"',
    "RuntimeMatchResults_WhenReportedTeamDiffersFromSession_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenIdempotencyKeyIsBlank_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerResultRowsAreDuplicated_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayersAreEmpty_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerTeamIsMissing_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerIsNotInSession_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenSessionPlayerIsMissing_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerResultIsInvalid_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerStatsContainNegativeValue_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam",
    "RuntimeMatchResults_WithJsonRewardQuantity_GrantsRewardsStatsAndCompletesSession",
    "RuntimeMatchResults_WhenRetried_ReturnsSameResultWithoutDoubleGrantingRewardsOrStats",
    "RuntimeMatchResults_CanBeReadFromSettlementSessionResultsWithRewards",
    "PlayerMatchHistories",
    "runtime-token-blank-result-idempotency",
    "runtime-token-duplicate-result-player",
    "runtime-token-empty-result-players",
    "runtime-token-missing-result-team",
    "runtime-token-unknown-result-player",
    "runtime-token-missing-session-player",
    "match-result-duplicate-player",
    "match-result-empty-players",
    "match-result-missing-team",
    "match-result-unknown-player",
    "match-result-missing-session-player",
    "match-result-invalid-player-result",
    "match-result-negative-player-stats",
    "match-result-negative-reward",
    "runtime-token-positive-result-reward",
    "match-result-runtime-json-reward",
    "runtime-token-result-retry",
    "match-result-runtime-retry",
    "runtime-token-settlement-query",
    "match-result-runtime-query",
    "PlayerMatchHistory",
    "SeedPlayerProgressionAsync",
    "GetRewardInt",
    "ApiResponse<JsonElement>",
    "ApiResponse<IReadOnlyList<MatchResultResponse>>",
    'TryGetProperty("matchResultId", out var matchResultIdElement)',
    "matchResultIdElement.TryGetGuid(out var matchResultId)",
    "Assert.Equal(firstMatchResultId, retryMatchResultId)",
    '"/internal/settlement/sessions/{sessionId}/matches/results"',
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    'Assert.Contains("\"schema\":\"runtime-endpoint-test\"", matchResult.ResultJson)',
    'Assert.Equal(5, GetRewardInt(player.Rewards, "coin"))',
    "Array.Empty<RuntimePlayerResultDto>()",
    '"eliminated"',
    "ExpDelta: -1",
    '["coin"] = -5',
    "Match result idempotency key is required.",
    "Match result contains duplicate players.",
    "Match result must contain at least one player.",
    "Match result contains players without a team.",
    "Match result contains players not in session",
    "Match result is missing players from session.",
    "Match result contains an invalid player result.",
    "Match result contains an invalid player numeric value.",
    "Assert.Equal(5, item.Quantity)",
    "Assert.Equal(1, await db.InventoryLogs.CountAsync",
    "Assert.Equal(1200, profile.Exp)",
    "Assert.Equal(1, stats.TotalMatches)",
    'Assert.Equal("COMPLETED", session.Status)',
    "Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode)",
    'sessionStatus: "SETTLING"',
    'serverStatus: "ENDING"',
    "secondPlayerId: missingPlayerId",
    'secondTeam: "red"',
    "MarkInProgress_WithoutInternalApiKey_ReturnsUnauthorized",
    "MarkInProgress_WithWrongInternalApiKey_ReturnsUnauthorized",
    "MarkInProgress_WithInternalApiKey_ReachesSessionHandler",
    "ListManagedServers_WithoutInternalApiKey_ReturnsUnauthorized",
    "ListManagedServers_WithWrongInternalApiKey_ReturnsUnauthorized",
    "ListManagedServers_WithInternalApiKey_ReachesManagerHandler",
    "GetLegacyInternalServer_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetLegacyInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized",
    "GetLegacyInternalServer_WithInternalApiKey_ReachesGameServerHandler",
    "GetSessionResults_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetSessionResults_WithWrongInternalApiKey_ReturnsUnauthorized",
    "MarkMatchStartedAsync_UpdatesSessionServerAndWritesEvents",
    "MarkMatchEndedAsync_UpdatesSessionServerAndWritesEvents",
    "MarkMatchStartedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents",
    "MarkMatchStartedAsync_WhenActivePlayerHasNotJoined_ReturnsFalseWithoutEvents",
    "MarkMatchEndedAsync_WhenMatchHasNotStarted_ReturnsFalseWithoutEvents",
    "MarkMatchEndedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents",
    "MarkMatchStartedAsync_WhenRepeated_DoesNotWriteDuplicateEvents",
    "MarkMatchEndedAsync_WhenRepeated_DoesNotWriteDuplicateEvents",
    "CreatePlayerSession",
    'CreatePlayerSession(sessionId, playerId, "CONNECTED")',
    'Assert.Equal("WAITING_PLAYERS", session.Status)',
    'Assert.Equal("READY", server.Status)',
    "GetMatchResultAsync_ReturnsPlayerResults",
    "GetSessionResultsAsync_ReturnsLatestFirstWithPlayerResults",
    "SubmitMatchResultAsync_WhenIdempotencyKeyIsBlank_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenMatchHasNotEnded_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenIdempotencyKeyBelongsToOtherSession_ReturnsNullWithoutReusingOtherResult",
    "SubmitMatchResultAsync_WhenSessionPlayerIsMissing_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerIsDuplicated_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenNoSessionPlayersExist_ReturnsNullWithoutCompletingSession",
    "SubmitMatchResultAsync_WhenPlayerTeamDiffersFromSession_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerResultIsInvalid_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerStatsContainNegativeValue_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenRewardQuantityIsNegative_ReturnsNullWithoutRewardsOrStats",
    "SeedSettlingSession",
    "SeedSettlingSessionWithoutPlayers",
    "SeedAdditionalPlayer",
    "CreateSettlementRequest",
    "result-before-match-ended",
    "result-missing-session-player",
    "result-duplicate-player",
    "result-empty-session-players",
    "result-team-mismatch",
    "result-invalid-player-result",
    "result-negative-player-stats",
    "result-negative-reward",
    "shared-result-key",
    "Assert.Null(result)",
    "Assert.Equal(0, await db.MatchResults.CountAsync(x => x.SessionId == secondSessionId)",
    "Assert.Equal(0, await db.InventoryLogs.CountAsync",
    "Assert.Equal(0, await db.SessionEvents.CountAsync",
    "Assert.Equal(0, stats.TotalMatches)",
    "Assert.Equal(0, secondStats.TotalMatches)",
    "Assert.Equal(0, firstStats.TotalMatches)",
    'Assert.Equal("SETTLING", session.Status)',
    'Assert.Equal("ENDING", server.Status)',
    'Assert.Equal("SETTLING", secondSession.Status)',
    "Assert.Null(session.EndedAt)",
    'Team = "blue"',
    "GetSessionResults_ReturnsLatestResultsWithPlayerDetails",
    "WebApplicationFactory<Program>",
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    "401|Unauthorized|Invalid internal api key",
    '"/internal/settlement/sessions/{sessionId}/matches/results"',
    "ValidateAndBuildPayload_WithKnownPlayers_MapsSettlementPayload",
    "ValidateAndBuildPayload_WhenSessionPlayerIsMissing_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerIsDuplicated_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerTeamIsMissing_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerResultIsInvalid_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerStatsContainNegativeValue_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenRewardQuantityIsNegative_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerTeamDoesNotMatchSession_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenReportedTeamHasDifferentCasing_UsesSessionTeamInPayload",
    "CreateSessionTeams",
    "SubmitMatchResultAsync_RepeatedSubmission_DoesNotDoubleGrantRewardsOrStats",
    "FDBA_GameBackendRuntimePlayerResult",
    "NotifyMatchResults",
    "BuildMatchResultsPayload",
    "virtual void HandleMatchHasStarted() override",
    "virtual void HandleMatchHasEnded() override",
    "ReportBackendMatchStarted",
    "ReportBackendMatchResults",
    "RuntimeService->NotifyMatchStarted",
    "RuntimeService->NotifyMatchEnded",
    "RuntimeService->NotifyMatchResults",
    "ue-match-result-%s",
    "DBAPlayerState.h",
    "ADBAPlayerState",
    "RecordKill",
    "RecordDeath",
    "RecordAssist",
    "AddMatchScore",
    "SetMatchTeamId",
    "GetMatchTeamId",
    "BuildRuntimePlayerResult",
    "GetMatchKills",
    "GetMatchDeaths",
    "GetMatchScore",
    "MatchKills",
    "MatchDeaths",
    "MatchAssists",
    "MatchScore",
    "MatchExpDelta",
    "MatchTeamId",
    "DOREPLIFETIME",
    "DOREPLIFETIME(ADBAPlayerState, MatchTeamId)",
    "PlayerStateClass = ADBAPlayerState::StaticClass()",
    "DBAPlayerState->BuildRuntimePlayerResult",
    "BackendRuntimePlayerTeamIds",
    "ResolveBackendMatchTeamIdFromOptions",
    "SyncBackendMatchTeamId",
    "DBAUrlOptions::TryExtractTeamId",
    "BackendTeamId <= 0",
    "BuildBackendRuntimeTeamName(BackendTeamId)",
    "BackendRuntimePlayerTeamIds.Add",
    "ZodiacCharacter->SetTeamID",
    "ApplyBackendMatchResultsOutcome",
    "FBackendMatchTeamOutcome",
    "BuildBackendMatchTeamOutcome",
    "BuildBackendMatchResultsJson",
    "winnerPlayerId",
    "winnerTeam",
    "TeamScores",
    "PlayerResult.Team",
    "PlayerResult.Result = TEXT(""win"")",
    "PlayerResult.Result = TEXT(""loss"")",
    "PlayerResult.Result = TEXT(""draw"")",
    "DBADamageCalculator.cpp",
    "ResolveDBAPlayerState",
    "RecordMatchEliminationStats",
    "VictimPlayerState->RecordDeath",
    "AttackerPlayerState->RecordKill",
    "!ZodiacChar->IsDead()",
    "TMap<FString, int32> Rewards",
    "GameBackendRuntimeServiceTests.cpp",
    "FDBA_GameBackendRuntimeMatchResultsPayloadTest",
    "DivineBeastsArena.GameBackendClient.Runtime.BuildMatchResultsPayload",
    "TEXT(""idempotencyKey"")",
    "TEXT(""players"")",
    "TEXT(""rewards"")",
    "FJsonValueObject",
    "PASS: Runtime match lifecycle contract"
)

Assert-FileContains "scripts\validate-internal-api-route-protection.ps1" @(
    "Validates that every Minimal API /internal route group has internal API key",
    "EndpointRoot",
    "Get-InternalRouteGroups",
    'app\.MapGroup',
    "/internal",
    "Get-MappedHandlers",
    "Get-DirectInternalRouteHandlers",
    "Chain =",
    "Handler =",
    "direct handler expression is missing endpoint/internal API key validation",
    'app\.Map',
    "Get-ChainedInternalRouteGroups",
    "Get-ChainedMappedHandlers",
    "Test-HandlerHasInternalApiValidation",
    "InternalApiKeyEndpointFilter.RequireInternalApiKey",
    "InternalApiKeyEndpointFilter.Validate",
    "HttpContext",
    "Unprotected /internal route groups detected",
    "PASS: internal API route protection validation"
)

Assert-FileContains "scripts\test-gamebackend-player-match-history-contract.ps1" @(
    "FDBA_GameBackendMatchHistoryEntry",
    "FDBA_GameBackendMatchHistoryPage",
    "TryParseMatchHistoryData",
    "ResultJson",
    "WinnerTeam",
    "ExpDelta",
    "Rewards",
    "GameBackendPlayerServiceTests.cpp",
    "MatchHistoryJsonParsesSettlementOutcome",
    "missingContractText",
    "expDeltaAssertionText",
    "passText"
)

Assert-FileContains "scripts\test-main-lobby-match-history-contract.ps1" @(
    "Main Lobby match history contract",
    "FDBALobbyRecentMatchSummary",
    "RefreshMatchHistory",
    "UpdateMatchHistoryFromJson",
    "GetRecentMatchSummary",
    "OnRecentMatchSummaryUpdated",
    "DBAMainLobbyMatchHistoryTests.cpp",
    "MainLobby.MatchHistoryUpdatesRecentSummary",
    "PASS: Main Lobby match history contract"
)

Assert-FileContains "scripts\test-internal-api-route-protection-contract.ps1" @(
    "internal API route protection validator",
    "ProtectedByFilterEndpoints",
    "ProtectedByHandlerEndpoints",
    "DirectProtectedByFilterEndpoints",
    "DirectLambdaProtectedByFilterEndpoints",
    "UnprotectedInternalEndpoints",
    "DirectUnprotectedInternalEndpoints",
    "DirectLambdaUnprotectedInternalEndpoints",
    "ChainedUnprotectedInternalEndpoints",
    "validate-internal-api-route-protection.ps1",
    "/internal/filter-protected",
    "/internal/handler-protected",
    "/internal/direct-filter-protected",
    "/internal/direct-lambda-filter-protected",
    "/internal/unprotected",
    "/internal/direct-unprotected",
    "/internal/direct-lambda-unprotected",
    "/internal/chained-unprotected",
    "Expected validator to reject an unprotected /internal route fixture",
    "Expected validator to reject an unprotected direct /internal route fixture",
    "Expected validator to reject an unprotected direct lambda /internal route fixture",
    "Expected validator to reject an unprotected chained /internal route fixture",
    "PASS: internal API route protection contract"
)

Assert-FileContains "scripts\test-player-id-claim-boundary-contract.ps1" @(
    "player_id claim boundary contract",
    "DBA_GameBackend\Game.Api\Endpoints",
    "FindFirst\(""player_id""\)\s*\?\?\s*ctx\.User\.FindFirst\(ClaimTypes\.NameIdentifier\)",
    "Player-scoped endpoints must not fall back from player_id to account NameIdentifier"
)

Assert-FileContains "scripts\test-session-connection-build-summary-contract.ps1" @(
    "session connection build summary contract",
    "SessionConnectionResponse",
    "int TeamId",
    "CharacterBuildSummaryDto? CharacterBuildSummary",
    'session.Status is not ("WAITING_PLAYERS" or "IN_PROGRESS")',
    "playerSession.Team",
    "Assert.Equal(1, connection.TeamId)",
    "GetConnectionInfoAsync_WhenServerIsAllocatedButNotReady_ReturnsNullWithoutReissuingToken",
    'storedSession.Status = "ALLOCATING_SERVER"',
    "originalTokenHash",
    "originalTokenExpiresAt",
    "GetConnectionInfoAsync_ReturnsFrozenSelectedCharacterBuildSummary",
    "GetConnectionInfoAsync_WhenSelectedCharacterBuildSummaryIsPadded_ReturnsNormalizedSummary",
    "GetConnectionInfoAsync_WhenSelectedCharacterFixedSkillGroupIsTampered_FreezesComputedSkillGroup",
    "TryBuildTravelUrlFromConnectionData",
    "BuildTravelUrlIncludesFrozenBuildSummary",
    "DBATeamId=1",
    "DBATeamId=2",
    "ConnectionJsonBuildsTravelUrlWithNestedBuildSummary",
    "Connection.TeamId",
    "TEXT(""DBATeamId"")",
    "TEXT(""teamId"")",
    "TEXT(""DBAZodiac"")",
    "TEXT(""DBAFixedSkillGroupId"")",
    "PASS: session connection build summary contract"
)

Assert-FileContains "scripts\test-dedicated-server-url-build-summary-admission-contract.ps1" @(
    "Dedicated Server URL build-summary admission contract",
    "DBAUrlOptions.h",
    "DBAUrlOptions.cpp",
    "DBAGameModeBase.cpp",
    "DBAUrlOptionsTests.cpp",
    "FGenericPlatformHttp::UrlDecode",
    "ParseStableZodiacName",
    "ParseStableElementName",
    "ParseStableFiveCampName",
    "DBACharacterBuild::MakeFixedSkillGroupId",
    "FixedSkillGroupId == ExpectedFixedSkillGroupId",
    "TryExtractTeamId",
    "OutTeamId",
    "BackendTeamId <= 0",
    "BuildBackendRuntimeTeamName(BackendTeamId)",
    "FDBAUrlOptionsBuildSummaryAdmissionTest",
    "DBAFixedSkillGroupId=Rat_Water",
    "DBATeamId=1",
    "TeamId=2",
    "rat_water",
    "Rat_Fire",
    "TamperedSummary",
    "MissingSummary",
    "MissingTeamId",
    "NonPositiveTeamId",
    "PASS: Dedicated Server URL build-summary admission contract"
)

Assert-FileContains "scripts\diagnose-fixed-skill-group-datatable.ps1" @(
    "FixedSkillGroups DataTable",
    "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    "UnrealEditor-Cmd.exe",
    "DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows",
    "-NullRHI",
    "CommandOnly",
    "AssetFileExists",
    "projectFileMissingMessage",
    "assetFileMissingMessage",
    "automationFailedMessage"
)

Assert-FileContains "scripts\test-fixed-skill-group-datatable-diagnostic.ps1" @(
    "diagnose-fixed-skill-group-datatable.ps1",
    "diagnostic script defines Chinese runtime message fragments",
    "command-only output includes the expected editor automation command",
    "missing project path fails fast",
    "missing UnrealEditor-Cmd fails fast",
    "DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows"
)

Assert-FileContains "scripts\test-data-table-count-constants.ps1" @(
    "DBAZodiacHeroDataAsset.cpp",
    "DBAAbilitySetDataAsset.cpp",
    "DBAStaticDataAsset.cpp",
    "DBAConstants::ZodiacCount",
    "DBAConstants::ElementCount",
    "DBAConstants::ElementActiveAbilityRowCount",
    "DBAConstants::ElementResonanceRowCount",
    "DBAConstants::FixedSkillGroupRowCount",
    "PASS: Data table count constants contract"
)

Assert-FileContains "scripts\test-fixed-skill-group-asset-test-constants.ps1" @(
    "DBAFixedSkillGroupDataTests.cpp",
    "New-CodePointText",
    "DBAConstants::ZodiacCount",
    "DBAConstants::ElementCount",
    "DBAConstants::FixedSkillGroupRowCount",
    '$passText'
)

Assert-FileContains "scripts\write-fixed-skill-group-source-csv.ps1" @(
    "DT_FixedSkillGroups.csv",
    "Rat",
    "Snake",
    "Gold",
    "Water",
    "RowId",
    "ZodiacType",
    "ElementType",
    "ResonanceElement",
    "ValidateOnly",
    "rowCountMismatchMessage",
    "missingSourceCsvMessage"
)

Assert-FileContains "scripts\test-fixed-skill-group-source-csv.ps1" @(
    "write-fixed-skill-group-source-csv.ps1",
    "writer validation diagnostics use Chinese human-readable output",
    "writer creates deterministic 60-row source csv",
    "ValidateOnly",
    "Rat_Water",
    "Snake_Gold",
    "forbiddenEnglishDiagnostics"
)

Assert-FileContains "scripts\import-fixed-skill-group-datatable.ps1" @(
    "import_fixed_skill_group_datatable.py",
    "DT_FixedSkillGroups.csv",
    "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    "/Script/DivineBeastsArena.DBAZodiacElementFixedSkillGroupRow",
    "CommandOnly",
    "WillWriteAsset",
    "UnrealEditor-Cmd.exe"
)

Assert-FileContains "scripts\test-fixed-skill-group-datatable-import.ps1" @(
    "import-fixed-skill-group-datatable.ps1",
    "command-only output includes guarded import command",
    "missing csv fails fast",
    "missing project path fails fast",
    "missing UnrealEditor-Cmd fails fast",
    "import_fixed_skill_group_datatable.py"
)

Assert-FileContains "scripts\unreal\import_fixed_skill_group_datatable.py" @(
    "CSVImportFactory",
    "AssetImportTask",
    "EditorAssetLibrary.save_asset",
    "DataTableFunctionLibrary.get_data_table_row_names",
    "EXPECTED_ROW_COUNT = 60",
    "/Game/DBA/Data/Tables/DT_FixedSkillGroups"
)

Assert-FileContains "DBA_GameClient\Content\DBA\Data\Tables\Source\DT_FixedSkillGroups.csv" @(
    "Name",
    "RowId",
    "ZodiacType",
    "ElementType",
    "Rat_Water",
    "Snake_Gold",
    "Pig_Earth"
)

Assert-FileContains "scripts\package-unreal-dedicated-server.ps1" @(
    '-serverconfig=$Configuration',
    '-clientconfig=$Configuration',
    "IncludeClientCook"
)

Assert-FileContains "scripts\prepare-client-release-package.ps1" @(
    "public-client-package",
    "symbols-package",
    "debugSymbolCount",
    "DivineBeastsArena.exe",
    "sha256",
    "releasePackage"
)

Assert-FileContains "scripts\collect-client-package-evidence.ps1" @(
    "client-package-launcher",
    "launcher-manifest",
    "DivineBeastsArena.exe",
    "version.txt",
    "sha256",
    "installSmoke",
    "BuildConfiguration",
    "DisallowDebugSymbols",
    "debugSymbolCount",
    "Resolve-ReleaseUri",
    "downloadUrlHasHost",
    "valid absolute URL with a host",
    "releaseReady"
)

Assert-FileContains "scripts\test-client-package-url-policy.ps1" @(
    "collect-client-package-evidence.ps1",
    "DivineBeastsArena.exe",
    "DivineBeastsArena-Windows.pak",
    "https://",
    "downloadUrlHasHost",
    "valid absolute URL with a host",
    "PASS: client package URL policy fixtures"
)

Assert-FileContains "scripts\collect-production-evidence.ps1" @(
    "security.nuget",
    "Test-NuGetVulnerabilityReadyEvidence",
    "vulnerability-report",
    "has the following vulnerable packages",
    "security.npm",
    "Test-NpmAuditReadyEvidence",
    "requiredApps",
    "metadata.vulnerabilities",
    "high",
    "critical",
    "admin",
    "website",
    "launcher",
    "security.trivy",
    "Test-TrivyReadyEvidence",
    "requiredImages",
    "runs",
    "results",
    "api",
    "worker",
    "load.k6",
    "Get-K6ReadyEvidencePaths",
    "Test-K6ReadyEvidence",
    "requiredTests",
    '(^|/)k6[-_/].*\.(json|txt|log)$',
    '(^|/)load/.*\.(json|txt|log)$',
    "http_req_failed",
    "http_reqs",
    "iterations",
    "matchmaking",
    "ops.backup_restore",
    "Get-BackupRestoreReadyEvidencePaths",
    "Test-BackupRestoreReadyEvidence",
    "backup-restore-rehearsal",
    "schemaVersion",
    "restoreDatabase",
    "publicTableCount",
    "ops.deploy_rollback",
    "Test-DeployRollbackReadyEvidence",
    "production-smoke-backend",
    "live health",
    "ready health",
    "version api",
    "launcher manifest",
    "metrics endpoint",
    "guest login",
    "client.package_launcher",
    "client.release_prerequisites",
    "client-release-prerequisites",
    "Test-ClientReleasePrerequisiteReadyEvidence",
    "readyForReleaseInputs",
    "blockingIssueCount",
    "blockingIssues",
    "clientExePath",
    "fileCount",
    "urls.downloadUrl",
    "urls.manifestUrl",
    "signing.required",
    "certificateFound",
    "signToolPath",
    "Test-DerivedEvidenceOutput",
    "Split-Path -Leaf",
    "release-readiness-report.json",
    "release-readiness-report.md",
    "release-blocker-actions.json",
    "release-blocker-actions.md",
    "release-blocker-action-validation.json",
    "release-blockers-external-only-validation.json",
    "development-continuation-readiness-validation.json",
    "release-input-template.json",
    "release-input-template.md",
    "release-input-template-validation.json",
    "release-input-values.template.json",
    "release-input-values.template.md",
    "release-input-values-validation.json",
    "release-command-plan.template-check.json",
    "release-command-plan.template-check.md",
    '$clientPackageEvidenceDescription',
    "client-package-launcher",
    "launcher-manifest",
    "releaseReady",
    "downloadUrlHasHost",
    "incomplete",
    "client.cdn_launcher_smoke",
    "launcher-cdn-smoke",
    "client.code_signing",
    "code-signing",
    "client.launcher_install_update",
    "launcher-install-update-smoke",
    "Get-LauncherInstallUpdateReadyEvidencePaths",
    "client.launcher_ui_visual",
    "Get-LauncherUiVisualReadyEvidencePaths",
    "launcher-ui-visual-evidence"
)

Assert-FileDoesNotContain "scripts\collect-production-evidence.ps1" @(
    "Client package, launcher manifest, SHA256 file list"
)

Assert-FileContains "scripts\run-launcher-cdn-smoke.ps1" @(
    "launcher-cdn-smoke",
    "ManifestUrl",
    "downloadUrl",
    "Resolve-SmokeUri",
    "must be a valid absolute URL with a host",
    "sha256",
    "version.txt",
    "cdnReady",
    "AllowLocalHttp"
)

Assert-FileContains "scripts\test-launcher-cdn-smoke-url-policy.ps1" @(
    "run-launcher-cdn-smoke.ps1",
    "https://",
    "http://download.example.com/manifest.json",
    "ManifestUrl must be a valid absolute URL",
    "ManifestUrl must be HTTPS unless -AllowLocalHttp is used for localhost smoke",
    "PASS: launcher CDN smoke URL policy fixtures"
)

Assert-FileContains "scripts\run-local-cdn-payload-smoke.ps1" @(
    "local-cdn-payload-smoke",
    "PayloadRoot",
    "run-launcher-cdn-smoke.ps1",
    "launcher-manifest.json",
    "Resolve-Python",
    "python",
    "Start-Process",
    "Get-FreeTcpPort",
    "Copy-PayloadForLocalSmoke",
    "LocalDownloadUrl",
    "Stop-Process",
    "AllowLocalHttp"
)

Assert-FileContains "scripts\prepare-client-cdn-payload.ps1" @(
    "client-cdn-payload",
    "PackageRoot",
    "PayloadRoot",
    "DownloadUrl",
    "ManifestUrl",
    "launcher-manifest.json",
    "cdn-upload-manifest",
    "sha256",
    "Resolve-ReleaseUri",
    "must be a valid absolute URL with a host",
    "Assert-ReleaseUrlPolicy",
    "Should-ExcludeGeneratedReleaseMetadata",
    "AllowLocalHttp",
    "payloadReady"
)

Assert-FileContains "scripts\test-client-cdn-payload-url-policy.ps1" @(
    "prepare-client-cdn-payload.ps1",
    "DivineBeastsArena.exe",
    "https://",
    "http://download.example.com/releases/1.0.0/",
    "DownloadUrl must be a valid absolute URL",
    "DownloadUrl must be HTTPS unless -AllowLocalHttp is used for localhost payload validation",
    "PASS: client CDN payload URL policy fixtures"
)

Assert-FileContains "scripts\collect-code-signing-evidence.ps1" @(
    "code-signing",
    "Get-AuthenticodeSignature",
    "signingReady",
    "signedFileCount",
    "unsignedFileCount",
    "trustedSignedFileCount"
)

Assert-FileContains "scripts\sign-client-release-package.ps1" @(
    "sign-client-release-package",
    "signtool.exe",
    "CertificateThumbprint",
    "CertificateSubject",
    "PfxPath",
    "TimestampUrl",
    "Get-SignableClientFiles",
    "collect-code-signing-evidence.ps1",
    "RequireSigned"
)

Assert-FileContains "scripts\diagnose-client-release-prerequisites.ps1" @(
    "client-release-prerequisites",
    "New-TextFromCodePoints",
    '$reportWrittenMessage',
    '$blockingIssuesTitleMessage',
    '$successReadyMessage',
    '$blockingFailureMessage',
    "PackageRoot",
    "DownloadUrl",
    "ManifestUrl",
    "RequireManifestUrl",
    "AllowLocalHttp",
    "RequireSigningIdentity",
    "CertificateThumbprint",
    "CertificateSubject",
    "PfxPath",
    "PfxPasswordEnvironmentVariable",
    "RequireSignTool",
    "FailOnBlockingIssues",
    "readyForReleaseInputs",
    "blockingIssues",
    "download_url_example",
    "manifest_url_example",
    "signtool_missing"
)

Assert-FileContains "scripts\test-client-release-prerequisites.ps1" @(
    "diagnose-client-release-prerequisites.ps1",
    "New-TextFromCodePoints",
    "client-release-prerequisites-tests-{0}",
    '$successMessage',
    '$forbiddenFixedFixtureRootMessage',
    '$forbiddenOldSuccessMessage',
    "cdn.example.com",
    "https://",
    "download_url_invalid",
    '$expectedChineseUrlHostMessage',
    '$expectedDiagnosticReportMessageContract',
    '$forbiddenOldDiagnosticReportMessage',
    "download_url_example",
    "manifest_url_example",
    "readyForReleaseInputs"
)

Assert-FileContains "scripts\run-launcher-install-update-smoke.ps1" @(
    "launcher-install-update-smoke",
    "cargo test",
    "repair_game_downloads_local_package_and_persists_version",
    "installUpdateReady",
    "versionPersisted",
    "hashVerified"
)

Assert-FileContains "scripts\capture-launcher-ui-evidence.ps1" @(
    "launcher-ui-visual-evidence",
    "npm run build",
    "npm run preview",
    "Resolve-Browser",
    "headless",
    "screenshot",
    "uiMarkersReady",
    "screenshotReady",
    "uiEvidenceReady"
)

Assert-FileContains "scripts\run-client-release-evidence.ps1" @(
    "client-release-evidence",
    "collect-client-package-evidence.ps1",
    "prepare-client-cdn-payload.ps1",
    "sign-client-release-package.ps1",
    "collect-code-signing-evidence.ps1",
    "run-launcher-install-update-smoke.ps1",
    "run-launcher-cdn-smoke.ps1",
    "run-local-cdn-payload-smoke.ps1",
    "capture-launcher-ui-evidence.ps1",
    "diagnose-client-release-prerequisites.ps1",
    "PrepareCdnPayload",
    "PayloadRoot",
    "RunLocalCdnPayloadSmoke",
    "CaptureLauncherUiEvidence",
    "SkipCdnSmoke",
    "SkipReleasePrerequisiteCheck",
    "SignPackage",
    "CertificateThumbprint",
    "CertificateSubject",
    "PfxPath",
    "PfxPasswordEnvironmentVariable",
    "TimestampUrl",
    "RequireSigned",
    "ManifestUrl",
    "cdnPayloadReady",
    "cdnUploadManifest",
    "localCdnPayloadSmokeReady",
    "localCdnPayloadSmoke",
    "launcherUiVisualReady",
    "launcherUiVisual",
    "releasePrerequisiteChecked",
    "releasePrerequisiteReady",
    "releasePrerequisites",
    "signingAttempted",
    "releaseReady"
)

Assert-FileContains "scripts\write-release-readiness-report.ps1" @(
    "release-readiness-report",
    "production-evidence-manifest.json",
    "blockingRequirements",
    "readyForRelease",
    "developmentContinuationReady",
    "releaseBlockerPosture",
    "release-blockers-external-only-validation.json",
    "externalOnlyReleaseBlockers",
    "localAutomationBlockers",
    "Missing or incomplete production evidence",
    "Release readiness report"
)

Assert-FileContains "scripts\diagnose-unreal-evidence-runner.ps1" @(
    "unreal-evidence-runner-diagnostic",
    "UnrealEditor.exe",
    "RunUAT.bat",
    "backend.health",
    "backend.internal_api_key",
    "validate-production-evidence-contracts.ps1"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Runtime\RuntimePlayerJoinValidator.cs" @(
    "BuildSummaryMismatchMessage",
    "BuildSummaryMissingMessage",
    "FrozenBuildSummaryInvalidMessage",
    "BuildPlayerJoinedEventPayload",
    "NormalizeEventValue",
    "NormalizeFrozenBuildSummary",
    "expectedFrozenFixedSkillGroupId",
    "bHasAnyFrozenBuildSummary",
    "bHasCompleteFrozenBuildSummary",
    "IsMissingRequiredChoice",
    "IsNoneChoice",
    "CharacterBuildRules.NormalizeChoice",
    "CharacterBuildRules.BuildFixedSkillGroupId",
    "request.Zodiac",
    "request.PrimaryElement",
    "request.FixedSkillGroupId",
    "team = NormalizeEventValue(playerSession.Team)",
    "fiveCamp = NormalizeEventValue(playerSession.FiveCamp)",
    'value?.Trim().Equals("None", StringComparison.OrdinalIgnoreCase)'
)

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\Character\CharacterBuildDtos.cs" @(
    "CharacterBuildRules",
    "BuildSummary",
    "BuildFixedSkillGroupId",
    "NormalizeChoice",
    "var normalizedZodiac = NormalizeChoice(zodiac, ""Rat"");",
    "var normalizedPrimaryElement = NormalizeChoice(primaryElement, ""Water"");",
    "var trimmed = value.Trim();",
    'trimmed.Equals("None", StringComparison.OrdinalIgnoreCase)',
    'return $"{NormalizeChoice(zodiac, "Rat")}_{NormalizeChoice(primaryElement, "Water")}";'
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\CharacterBuildRulesTests.cs" @(
    "CharacterBuildRulesTests",
    "BuildFixedSkillGroupId_WhenZodiacAndElementAreProvided_UsesZodiacAndElement",
    "BuildFixedSkillGroupId_WhenValuesAreMissingOrPadded_NormalizesChoices",
    'InlineData(" None ", " None ", "Rat_Water")',
    "BuildFixedSkillGroupId_WhenFiveCampChanges_DoesNotChangeSkillGroup",
    "BuildSummary_WhenValuesAreMissingOrPadded_NormalizesAllFields"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Account\AccountEndpoints.cs" @(
    "var buildSummary = CharacterBuildRules.BuildSummary",
    "Zodiac = buildSummary.Zodiac",
    "PrimaryElement = buildSummary.PrimaryElement",
    "FiveCamp = buildSummary.FiveCamp",
    "FixedSkillGroupId = buildSummary.FixedSkillGroupId"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RuntimePlayerJoinBuildSummaryTests.cs" @(
    "ValidateBuildSummary_WhenFixedSkillGroupIsTampered_ReturnsFalse",
    "ValidateBuildSummary_WhenFiveCampChangesButSkillGroupMatches_ReturnsTrue",
    "ValidateBuildSummary_WhenFrozenBuildExistsButFixedSkillGroupIsMissing_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenBuildExistsButRequiredChoiceIsNone_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenBuildContainsNone_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenChoicesArePaddedButValid_ReturnsTrue",
    "ValidateBuildSummary_WhenFrozenFixedSkillGroupDoesNotMatchFrozenIdentity_ReturnsFalse",
    "ValidateBuildSummary_WhenNoFrozenBuildSummaryExists_AllowsLegacySession",
    "ValidateBuildSummary_WhenFrozenBuildSummaryIsPartial_ReturnsFalse",
    "BuildPlayerJoinedEventPayload_WhenFiveCampRequestDiffers_UsesFrozenBuildSummary",
    "BuildPlayerJoinedEventPayload_WhenFrozenBuildSummaryIsPadded_WritesNormalizedSummary",
    "BuildPlayerJoinedEventPayload_WhenTeamIsPadded_WritesNormalizedTeam",
    "BuildPlayerJoinedEventPayload_WhenNoFrozenBuildSummaryExists_DoesNotInventDefaultSummary",
    "BuildSummaryMissingMessage",
    'InlineData(" None ", "Water", "Rat_Water")',
    'InlineData("Rat", "Water", " None ")'
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Session\SessionService.cs" @(
    "Task<bool> EnsureFrozenBuildSummaryAsync",
    "if (!await EnsureFrozenBuildSummaryAsync(playerSession))",
    "ToCharacterBuildSummary(PlayerCharacter selectedCharacter)",
    "bHasAnyFrozenBuildSummary",
    "bHasCompleteFrozenBuildSummary",
    "var existingBuildSummary = CharacterBuildRules.BuildSummary",
    "expectedFixedSkillGroupId",
    "return CharacterBuildRules.BuildSummary",
    "selectedCharacter.Zodiac",
    "selectedCharacter.PrimaryElement",
    "selectedCharacter.FiveCamp",
    "playerSession.Zodiac",
    "playerSession.PrimaryElement",
    "playerSession.FiveCamp"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "BuildBackendRuntimeTeamName",
    "ResolveBackendMatchTeamId(PlayerController, BackendRuntimePlayerTeamIds)",
    "RuntimeService->NotifyPlayerJoined(PlayerId, PlayerSessionToken, BackendRuntimeTeam"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RoomSessionServiceTests.cs" @(
    "GetConnectionInfoAsync_WhenServerIsAllocatedButNotReady_ReturnsNullWithoutReissuingToken",
    'storedSession.Status = "ALLOCATING_SERVER"',
    'storedSession.Status = "WAITING_PLAYERS"',
    "originalTokenHash",
    "originalTokenExpiresAt",
    "GetConnectionInfoAsync_WhenSelectedCharacterBuildSummaryIsPadded_ReturnsNormalizedSummary",
    "GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPadded_PersistsNormalizedSummary",
    "GetConnectionInfoAsync_WhenExistingFrozenFixedSkillGroupIsTampered_ReturnsNull",
    "GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPartial_ReturnsNull",
    "GetConnectionInfoAsync_WhenSelectedCharacterFixedSkillGroupIsTampered_FreezesComputedSkillGroup",
    "FlowArenaPaddedRat",
    "FlowArenaPartialFrozen",
    "FlowArenaTamperedSource",
    'playerSession.Zodiac = " Rat "',
    'playerSession.FixedSkillGroupId = "Tiger_Fire"',
    "Assert.Equal(""Rat"", playerSession.Zodiac)",
    "Assert.Equal(""Water"", playerSession.PrimaryElement)",
    "Assert.Equal(""East"", playerSession.FiveCamp)"
)

Assert-RepoDoesNotContain "DBA_GameBackend\Game.Api" "private static string BuildSkillGroupId" @("*.cs")

Assert-FileContains "DBA_GameBackend\Game.Infrastructure\Database\Seed\DevelopmentDataSeeder.cs" @(
    "CharacterBuildRules.BuildFixedSkillGroupId",
    "FixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId"
)

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\Settlement\SettlementDtos.cs" @(
    "public record MatchResultResponse",
    "string ResultJson",
    "IReadOnlyDictionary<string, object> Rewards",
    "IReadOnlyList<MatchPlayerResultDto> Players"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Settlement\SettlementEndpoints.cs" @(
    "private static MatchResultResponse ToResponse(MatchResult result)",
    "result.ResultJson",
    "new MatchPlayerResultDto",
    "ParseRewardJson(player.RewardJson)",
    "JsonSerializer.Deserialize<Dictionary<string, JsonElement>>",
    "NormalizeRewardValue",
    "JsonValueKind.Number",
    "value.GetRawText()"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\SettlementEndpointsTests.cs" @(
    "Game.Shared.Contracts.Settlement",
    "GetSessionResults_ReturnsLatestResultsWithPlayerDetails",
    "SubmitResult_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutRewardsOrStats",
    "SubmitResult_WithJsonRewardQuantity_GrantsRewardsAndCompletesSession",
    "SubmitResult_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam",
    "SubmitResult_WhenRetried_ReturnsPlayerDetailsWithoutDoubleGrantingRewardsOrStats",
    "settlement-endpoint-negative-reward",
    "settlement-endpoint-json-reward",
    "settlement-endpoint-frozen-team",
    "settlement-endpoint-retry-json-reward",
    '"schema":"frozen-team-test"',
    "Assert.Equal(firstResponse.Data!.Id, retryResponse.Data!.Id)",
    '["coin"] = -5',
    "var responsePlayer = Assert.Single(response.Data.Players)",
    "var retryPlayer = Assert.Single(retryResponse.Data.Players)",
    'Assert.Equal(playerId, responsePlayer.PlayerId)',
    'Assert.Equal(playerId, retryPlayer.PlayerId)',
    'Assert.Equal("blue", responsePlayer.Team)',
    'Assert.Equal("win", responsePlayer.Result)',
    'Assert.Equal("blue", playerResult.Team)',
    'Assert.Equal("blue", history.Team)',
    'Assert.Equal(5, GetRewardInt(responsePlayer.Rewards, "coin"))',
    'Assert.Equal(5, GetRewardInt(retryPlayer.Rewards, "coin"))',
    'Assert.Equal(5, item.Quantity)',
    'Assert.Equal(1, await db.InventoryLogs.CountAsync',
    'Assert.Equal("COMPLETED", session.Status)',
    "Failed to submit match result",
    "InMemoryEventId.TransactionIgnoredWarning",
    "latestResult.ResultJson",
    "olderResult.ResultJson",
    '"winnerTeam":"blue"',
    '"schema":"endpoint-test"',
    'Assert.Equal("blue", player.Team)',
    "Assert.Equal(900, player.ExpDelta)",
    'Assert.Equal(1, GetRewardInt(player.Rewards, "coin"))',
    "JsonElement element when element.ValueKind == JsonValueKind.Number"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Settlement\SettlementService.cs" @(
    "using System.Text.Json",
    "existingForSession",
    "Include(x => x.PlayerResults)",
    "PlayerMatchHistories",
    "new PlayerMatchHistory",
    "Team = sessionPlayerTeams[player.PlayerId]",
    "Result = playerResults[player.PlayerId]",
    "DurationSeconds = duration",
    "JsonElement { ValueKind: JsonValueKind.Number } quantity",
    "quantity.TryGetInt64(out var longQuantity)",
    "quantity.TryGetDouble(out var doubleQuantity)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Inventory\InventoryService.cs" @(
    "using System.Text.Json",
    "TryGetRewardQuantity",
    "JsonElement { ValueKind: JsonValueKind.Number }",
    "element.TryGetInt64(out var longQty)",
    "element.TryGetDouble(out var doubleQty)"
)

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\Admin\AdminDtos.cs" @(
    "public record AdminMatchListItem",
    "string? WinnerTeam",
    "public record AdminMatchDetailResponse",
    "string? WinnerTeam",
    "IReadOnlyDictionary<string, int> TeamDistribution",
    "public record AdminMatchPlayerItem",
    "IReadOnlyDictionary<string, object> Rewards"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Admin\AdminEndpoints.cs" @(
    "private static async Task<IResult> ListPlayers(GameDbContext db, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> ListAuditLogs(GameDbContext db, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> ListFeedback(GameDbContext db, string? status = null, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> ListSupportTickets(GameDbContext db, string? status = null, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> ListMatches(GameDbContext db, int page = 1, int pageSize = 50)",
    "ExtractWinnerTeam(x.ResultJson)",
    "private static async Task<IResult> GetMatch",
    "ExtractWinnerTeam(matchEntity.ResultJson)",
    "BuildTeamDistribution(matchEntity.PlayerResults)",
    "TryGetNonEmptyString(root, ""winnerTeam"")",
    "TryGetNonEmptyString(root, ""winner_team"")",
    "new AdminMatchPlayerItem",
    "ParseRewardJson(p.RewardJson)",
    "JsonSerializer.Deserialize<Dictionary<string, JsonElement>>",
    "NormalizeRewardValue",
    "JsonValueKind.Number",
    "value.GetRawText()"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Admin\AdminGameServerEndpoints.cs" @(
    "private static async Task<IResult> ListServers(GameDbContext db, string? status = null, int page = 1, int pageSize = 50)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Admin\AdminClientVersionEndpoints.cs" @(
    "private static async Task<IResult> ListClientVersions(GameDbContext db, int page = 1, int pageSize = 50)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\InventoryEndpoints.cs" @(
    "private static async Task<IResult> GetInventory(HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetUnlocks(HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetInventoryLogs(GameDbContext db, int page = 1, int pageSize = 50)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Feedback\FeedbackEndpoints.cs" @(
    "private static async Task<IResult> GetRecentFeedback(GameDbContext db, int page = 1, int pageSize = 50)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\RankingEndpoints.cs" @(
    "private static async Task<IResult> GetRanking(string mode, GameDbContext db, int page = 1, int pageSize = 50)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\PlayerHistoryEndpoints.cs" @(
    "private static async Task<IResult> GetMatchHistory(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> SubmitReport(SubmitReportRequest request, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetMyTickets(HttpContext ctx, GameDbContext db, int page = 1, int pageSize = 50)",
    "private static async Task<IResult> CreateTicket(CreateTicketRequest request, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetTicketDetail(Guid ticketId, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> ReplyTicket(Guid ticketId, ReplyTicketRequest request, HttpContext ctx, GameDbContext db)",
    "var playerId = GetPlayerId(ctx)",
    "db.MatchPlayerResults",
    "Include(x => x.MatchResult)",
    "playerResult?.MatchResult?.ResultJson ?? ""{}""",
    "ExtractWinnerTeam(playerResult?.MatchResult?.ResultJson)",
    "playerResult?.ExpDelta ?? 0",
    "ParseRewardJson(playerResult.RewardJson)",
    "private static string? ExtractWinnerTeam(string? resultJson)",
    'TryGetNonEmptyString(root, "winnerTeam")',
    'TryGetNonEmptyString(root, "winner_team")',
    "private static string? TryGetNonEmptyString(JsonElement root, string propertyName)",
    "private static IReadOnlyDictionary<string, object> ParseRewardJson(string rewardJson)",
    "private static object NormalizeRewardValue(JsonElement value)",
    "new TicketListResponse(tickets, totalCount, page, pageSize)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\CommerceEndpoints.cs" @(
    "private static async Task<IResult> PurchaseItem(PurchaseRequest request, HttpContext ctx, GameDbContext db)",
    "var playerId = GetPlayerId(ctx)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\SessionReconnectEndpoints.cs" @(
    "private static async Task<IResult> Reconnect(",
    "[FromBody] ReconnectRequest request",
    "HttpContext ctx",
    "[FromServices] IJwtTokenService jwt",
    "var playerId = GetPlayerId(ctx)",
    "jwt.HashToken(request.ReconnectToken)",
    "FixedTimeEquals",
    "ErrorResponse.Unauthorized("
)

Assert-FileContains "DBA_GameBackend\Game.Api\Extensions\EndpointResultsExtensions.cs" @(
    "Results.Json(error.ToApiResponse(), statusCode: error.Status)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\LiveContentEndpoints.cs" @(
    "private static async Task<IResult> GetMyEventProgress(HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetAchievements(HttpContext ctx, GameDbContext db)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameFeatures\SocialEndpoints.cs" @(
    "private static async Task<IResult> GetFriends(HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetFriendRequests(HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> SendFriendRequest(FriendRequestDto request, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> AcceptFriendRequest(Guid requestId, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> RejectFriendRequest(Guid requestId, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> RemoveFriend(Guid friendId, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> GetMails(HttpContext ctx, GameDbContext db, bool unreadOnly = false)",
    "private static async Task<IResult> ReadMail(Guid mailId, HttpContext ctx, GameDbContext db)",
    "private static async Task<IResult> ClaimAttachment(Guid mailId, Guid attachmentId, HttpContext ctx, GameDbContext db)"
)

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\GameFeatures\GameFeatureDtos.cs" @(
    "public record MatchHistoryDto",
    "string ResultJson",
    "string? WinnerTeam",
    "long ExpDelta",
    "IReadOnlyDictionary<string, object> Rewards",
    "public record TicketListResponse",
    "int TotalCount",
    "int Page",
    "int PageSize"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\AdminMatchEndpointsTests.cs" @(
    "GetMatch_ReturnsResultJsonAndPlayerRewardsForOperationsDiagnostics",
    "GetMatch_ReturnsStructuredTeamOutcomeForOperationsDiagnostics",
    "ListMatches_ReturnsStructuredWinnerTeamForOperationsDiagnostics",
    "RuntimeMatchResults_CanBeReadFromAdminMatchDetailsForOperationsDiagnostics",
    "/api/admin/auth/login",
    "/api/admin/matches/{matchId}",
    "/api/admin/matches/{matchResultId}",
    "/api/admin/matches",
    "/runtime/matches/results",
    '"winnerTeam":"blue"',
    '"schema":"admin-endpoint-test"',
    '"schema":"runtime-admin-query-test"',
    "runtime-token-admin-query",
    "match-result-runtime-admin-query",
    'Assert.Equal("blue", item.WinnerTeam)',
    'Assert.Equal("blue", response.Data!.WinnerTeam)',
    'Assert.Equal("blue", adminResponse.Data.WinnerTeam)',
    'Assert.Equal(1, response.Data.TeamDistribution["blue"])',
    'Assert.Equal(1, adminResponse.Data.TeamDistribution["blue"])',
    'Assert.Equal(1, response.Data.TeamDistribution["red"])',
    'Assert.Equal(50, GetRewardInt(player.Rewards, "coin"))',
    'Assert.Equal(7, GetRewardInt(player.Rewards, "coin"))',
    "JsonElement element when element.ValueKind == JsonValueKind.Number"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\AdminListPagingDefaultsTests.cs" @(
    "AdminListEndpoints_WithoutPagingQuery_ReturnDefaultPage",
    "/api/admin/players",
    "/api/admin/audit-logs",
    "/api/admin/feedback",
    "/api/admin/support/tickets",
    "/api/admin/matches",
    "/api/admin/servers",
    "/api/admin/client-versions",
    "PaginatedEndpoints_WithoutPagingQuery_DoNotReturnServerError",
    "/api/admin/inventory/logs",
    "/api/feedback/recent",
    "/api/rankings/ranked",
    "Assert.Equal(1, data.GetProperty(""page"").GetInt32())",
    "Assert.Equal(50, data.GetProperty(""pageSize"").GetInt32())"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\PlayerListPagingDefaultsTests.cs" @(
    "PlayerListEndpoints_WithoutPagingQuery_UseJwtPlayerAndDefaultPage",
    "PlayerReadEndpoints_WithoutQueryPlayerId_UseJwtPlayer",
    "/api/players/me/matches",
    "/api/support/tickets",
    "/api/players/me/inventory/",
    "/api/players/me/inventory/unlocks",
    "/api/events/me/progress",
    "/api/players/me/achievements/",
    "/api/friends/",
    "/api/friends/requests",
    "/api/mails/",
    "PlayerSupportAndReportWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer",
    "PlayerSocialWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer",
    "PlayerMailWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer",
    "PlayerShopPurchase_WithoutQueryPlayerId_UsesJwtPlayer",
    "PlayerReconnect_WithoutQueryPlayerId_UsesJwtPlayerAndValidToken",
    "PlayerReconnect_WithWrongReconnectToken_ReturnsUnauthorized",
    "PlayerMatchHistory_AfterRuntimeSettlement_ReturnsSettledPlayerResult",
    "/api/reports/",
    "/api/support/tickets/{ticketId}",
    "/api/friends/request",
    "/api/shop/purchase",
    "/api/sessions/{sessionId}/reconnect",
    "/runtime/matches/results",
    "runtime-token-player-history",
    "match-result-player-history",
    '"schema":"player-history-test"',
    'Assert.Equal(1, historyResponse.Data!.TotalCount)',
    'Assert.Equal("win", match.Result)',
    'Assert.Equal(1600, match.Score)',
    'Assert.Contains("\"schema\":\"player-history-test\"", match.ResultJson)',
    'Assert.Equal("blue", match.WinnerTeam)',
    'TryGetProperty("expDelta", out var expDelta)',
    'Assert.Equal(900, expDelta.GetInt64())',
    'TryGetProperty("rewards", out var rewards)',
    'Assert.Equal(9, rewards.GetProperty("coin").GetInt32())',
    'TryGetProperty("winnerTeam", out var winnerTeam)',
    'Assert.Equal("blue", winnerTeam.GetString())',
    'TryGetProperty("resultJson", out var resultJson)',
    'Assert.Contains("\"schema\":\"player-history-test\"", resultJson.GetString())',
    "SeedReconnectSessionAsync",
    "SeedRuntimeSessionAsync",
    "ReconnectTokenHash = jwt.HashToken(reconnectToken)",
    "AssertReconnectTokenHashDiffersAsync",
    "IJwtTokenService",
    "new AuthenticationHeaderValue(""Bearer"", player.Token)",
    "Assert.Equal(1, data.GetProperty(""page"").GetInt32())",
    "Assert.Equal(50, data.GetProperty(""pageSize"").GetInt32())"
)

Assert-FileContains "DBA_GameAdmin\src\app\core\models.ts" @(
    "export interface MatchListItem",
    "winnerTeam?: string | null",
    "export interface MatchPlayerItem",
    "winnerTeam?: string | null",
    "teamDistribution?: Record<string, number>",
    "rewards: Record<string, unknown>"
)

Assert-FileContains "DBA_GameAdmin\src\app\pages\admin-pages.ts" @(
    "formatResultSummary(match)",
    "formatResultSummary(match: MatchListItem)",
    "match.winnerTeam?.trim()",
    "winnerTeam",
    "winner_team",
    "schema",
    "formatRewards(player.rewards)",
    "formatRewards(rewards: Record<string, unknown> | null | undefined)",
    "Object.entries(rewards ?? {})",
    "team-outcome-winner",
    "team-outcome-distribution",
    "formatTeamOutcome(match)",
    "formatTeamDistribution(match.players, match.teamDistribution)",
    "teamDistribution?: Record<string, number> | null",
    "extractWinnerTeam(match.resultJson)",
    "player.team?.trim()",
    "key}: ${String(value)}",
    "match.id",
    "match.sessionId",
    "match.durationSeconds",
    "match.createdAt",
    "prettyResult"
)

Assert-FileContains "scripts\test-admin-match-reward-display-contract.ps1" @(
    "DBA_GameAdmin\src\app\core\models.ts",
    "DBA_GameAdmin\src\app\pages\admin-pages.ts",
    "DBA_GameBackend\docs\api.md",
    "formatResultSummary(match)",
    "match.winnerTeam?.trim()",
    "winnerTeam",
    "winner_team",
    "schema",
    "Admin match result diagnostics",
    "Team outcome summary",
    'winnerTeam`',
    'teamDistribution`',
    "rewards: Record<string, unknown>",
    "formatRewards(player.rewards)",
    "team-outcome-winner",
    "team-outcome-distribution",
    "formatTeamOutcome(match)",
    "formatTeamDistribution(match.players, match.teamDistribution)",
    "match.sessionId",
    "match.durationSeconds",
    "PASS: Admin match reward display contract"
)

Assert-FileContains "DBA_GameBackend\docs\api.md" @(
    "Player match history",
    "/api/players/me/matches",
    "resultJson",
    "winnerTeam",
    "winner_team",
    "expDelta",
    "rewards",
    "MatchPlayerResult",
    "Admin match result diagnostics",
    "resultJson",
    "winnerTeam",
    "winner_team",
    "Team outcome summary",
    'winnerTeam`',
    'teamDistribution`',
    "rewards"
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
    "Admin match reward display contract",
    "test-admin-match-reward-display-contract.ps1",
    "Admin auth session storage contract",
    "test-admin-auth-session-storage-contract.ps1",
    "Admin auth interceptor scope contract",
    "test-admin-auth-interceptor-scope-contract.ps1",
    "Admin auth return-url contract",
    "test-admin-auth-return-url-contract.ps1",
    "Launcher CSP contract",
    "test-launcher-csp-contract.ps1",
    "Launcher manifest URL policy contract",
    "test-launcher-manifest-url-policy-contract.ps1"
)

Assert-FileContains "DBA_GameAdmin\src\app\core\auth.service.ts" @(
    "sessionStorage.setItem",
    "sessionStorage.removeItem",
    "sessionStorage.getItem",
    "dba.admin.session",
    "clearExpiredSession",
    "this.clearExpiredSession()",
    "this.clearExpiredSession(false)"
)

Assert-FileDoesNotContain "DBA_GameAdmin\src\app\core\auth.service.ts" @(
    "localStorage"
)

Assert-FileContains "scripts\test-admin-auth-session-storage-contract.ps1" @(
    "sessionStorage.setItem",
    "sessionStorage.removeItem",
    "sessionStorage.getItem",
    "clearExpiredSession",
    "localStorage",
    "PASS: Admin auth session storage contract"
)

Assert-FileContains "DBA_GameAdmin\src\app\core\auth.interceptor.ts" @(
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
    'Authorization: `Bearer ${token}`'
)

Assert-FileContains "scripts\test-admin-auth-interceptor-scope-contract.ps1" @(
    "HttpErrorResponse",
    "catchError",
    "handleAuthFailure",
    "auth.signOut()",
    "environment.apiBaseUrl",
    "shouldAttachToken",
    "url.startsWith('/')",
    "configuredApiOrigin",
    "requestOrigin",
    "PASS: Admin auth interceptor scope contract"
)

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

Assert-FileContains "scripts\test-admin-auth-return-url-contract.ps1" @(
    "CanActivateChildFn",
    "RouterStateSnapshot",
    "safeReturnUrl",
    "!returnUrl.startsWith('//')",
    "PASS: Admin auth return-url contract"
)

Assert-FileContains "scripts\test-launcher-csp-contract.ps1" @(
    "DBA_GameLauncher\src-tauri\tauri.conf.json",
    "connect-src",
    "http://localhost:8080",
    "http://127.0.0.1:8080",
    "https://*",
    "http://localhost:*",
    "PASS: Launcher CSP contract"
)

Assert-FileContains "DBA_GameLauncher\src-tauri\tauri.conf.json" @(
    "connect-src 'self' ipc: http://ipc.localhost http://localhost:8080 http://127.0.0.1:8080"
)

Assert-FileDoesNotContain "DBA_GameLauncher\src-tauri\tauri.conf.json" @(
    "https://*",
    "http://localhost:*",
    "http://127.0.0.1:*"
)

Assert-FileContains "DBA_GameLauncher\src-tauri\src\lib.rs" @(
    "fn validate_network_url",
    "fn extract_url_host",
    'validate_network_url(&url, "ManifestUrl")',
    'validate_network_url(&manifest.download_url, "Manifest downloadUrl")',
    "must include a valid host",
    'trimmed.strip_prefix("https://")',
    'trimmed.strip_prefix("http://")',
    '"localhost" | "127.0.0.1" | "::1"',
    "validate_manifest_rejects_external_http_download_url",
    "validate_network_url_accepts_ipv6_loopback_http_for_local_validation",
    "validate_network_url_rejects_https_url_without_host"
)

Assert-FileContains "scripts\test-launcher-manifest-url-policy-contract.ps1" @(
    "validate_network_url",
    "extract_url_host",
    "ManifestUrl",
    "Manifest downloadUrl",
    "must include a valid host",
    "must use HTTPS unless it points to localhost",
    "PASS: Launcher manifest URL policy contract"
)

Assert-FileContains "scripts\validate-unreal-source-guardrails.ps1" @(
    "Push-Location",
    "Pop-Location",
    "client-only Unreal modules must be guarded",
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
    "Subsystem",
    "Runtime player-joined build summary contract is incomplete",
    "ToStableZodiacName",
    "ToStableElementName",
    "ToStableFiveCampName",
    "FixedSkillGroupId",
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "AdmissionBuildSummary",
    "TEXT(""zodiac"")",
    "TEXT(""primaryElement"")",
    "TEXT(""fiveCamp"")",
    "TEXT(""fixedSkillGroupId"")",
    "Session travel build summary contract is incomplete",
    "TEXT(""characterBuildSummary"")",
    "TEXT(""DBATeamId"")",
    "TEXT(""teamId"")",
    "Connection.Zodiac",
    "Connection.PrimaryElement",
    "Connection.FiveCamp",
    "Connection.FixedSkillGroupId",
    "Connection.TeamId",
    "int32 TeamId",
    "TryBuildTravelUrlFromConnectionData",
    "TryGetObjectField(TEXT(""data"")",
    "NestedBuildSummaryObj",
    "TEXT(""serverIp"")",
    "TEXT(""serverPort"")",
    "TEXT(""sessionToken"")",
    "BuildTravelUrlIncludesFrozenBuildSummary",
    "ConnectionJsonBuildsTravelUrlWithNestedBuildSummary",
    "ConnectionJsonAcceptsNestedServerAliases",
    "ConnectionJsonAcceptsResponseEnvelopeData",
    "DBATeamId=1",
    "DBATeamId=2",
    "Test-RuntimeMatchLifecycleHandoffContract",
    "Runtime match lifecycle handoff contract is incomplete",
    "void ADBAGameModeBase::HandleMatchHasStarted()",
    "Super::HandleMatchHasStarted()",
    "ReportBackendMatchStarted",
    "RuntimeService->NotifyMatchStarted",
    "void ADBAGameModeBase::HandleMatchHasEnded()",
    "RuntimeService->NotifyMatchEnded",
    "ReportBackendMatchResults",
    "void ADBAGameModeBase::ReportBackendMatchResults()",
    "FDBA_GameBackendRuntimePlayerResult",
    "BackendRuntimePlayerIds",
    "DBAPlayerState->BuildRuntimePlayerResult",
    "FString::Printf(TEXT(""ue-match-result-%s"")",
    "BuildBackendMatchResultsJson",
    "RuntimeService->NotifyMatchResults",
    "Dedicated Server URL build summary admission coverage is incomplete"
)

Assert-FileContains "scripts\test-unreal-cpp-logic-blueprint-boundary.ps1" @(
    "Unreal C++ logic / Blueprint boundary contract",
    "ZodiacArena_UE5_8_Codex_*.md",
    "Test-CppLogicBlueprintBoundaryPolicy",
    "AGENTS.md",
    "validate-unreal-source-guardrails.ps1",
    "test-production-evidence-automation.ps1",
    "validate-production-evidence-contracts.ps1",
    "PASS: Unreal C++ logic / Blueprint boundary contract"
)

Assert-FileContains "scripts\test-unreal-data-asset-no-hardcoding-policy.ps1" @(
    "Unreal DataAsset / no-hardcoding policy contract",
    "ZodiacArena_UE5_8_Codex_*.md",
    "Test-DataAssetNoHardcodingPolicy",
    "AGENTS.md",
    "validate-unreal-source-guardrails.ps1",
    "test-production-evidence-automation.ps1",
    "validate-production-evidence-contracts.ps1",
    "PASS: Unreal DataAsset / no-hardcoding policy contract"
)

Assert-FileContains "scripts\test-unreal-ui-event-async-policy.ps1" @(
    "Unreal UI event / async interface policy contract",
    "ZodiacArena_UE5_8_Codex_*.md",
    "Test-EventDrivenUiAsyncInterfacePolicy",
    "AGENTS.md",
    "validate-unreal-source-guardrails.ps1",
    "test-production-evidence-automation.ps1",
    "validate-production-evidence-contracts.ps1",
    "PASS: Unreal UI event / async interface policy contract"
)

Assert-FileContains "scripts\test-agent-direct-execution-policy.ps1" @(
    'PolicyId: `DBA.Agent.DirectExecution`',
    "directExecutionDefaultText",
    "directExecutionNoAskText",
    "directExecutionNoWaitText",
    "directExecutionContinuousText",
    "directExecutionRegularStageText",
    "directExecutionNotCheckpointText",
    "directExecutionNoPlanAsDoneText",
    "externalInputsMissingText",
    "continueLocalVerificationText",
    "higherPriorityConfirmText",
    "directExecutionContractText",
    "AGENTS.md",
    "ZodiacArena_UE5_8_Codex_*.md",
    "test-production-evidence-automation.ps1",
    "validate-production-evidence-contracts.ps1",
    "PASS: {0}"
)

Assert-FileContains "scripts\test-unreal-chinese-log-output-policy.ps1" @(
    "Unreal Chinese log output policy contract",
    "ZodiacArena_UE5_8_Codex_*.md",
    "Test-ChineseLogOutputPolicy",
    "Assert-FileDoesNotContain",
    "DBAGameInstance.cpp",
    "DivineBeastsArena.cpp",
    "DBAGameModeBase.cpp",
    "gameModeDedicatedServerRuntimeText",
    "gameModeRuntimeReadySentText",
    "gameModeRuntimeMatchEndedText",
    "gameModeRuntimeMatchStartedText",
    "gameModeRuntimePlayerJoinMissingText",
    "gameModeRuntimePlayerJoinRejectedText",
    "gameModeRuntimePlayerJoinedSentText",
    "gameModeRuntimePlayerLeftSentText",
    "gameModeRuntimeMatchResultsSkippedText",
    "gameModeRuntimeMatchResultsSentText",
    "gameModeLaunchOptionsLabelText",
    "gameModePlayerLabelText",
    "gameModeTeamLabelText",
    "gameModeZodiacLabelText",
    "gameModeElementLabelText",
    "gameModeFixedSkillGroupLabelText",
    "gameModePlayerCountLabelText",
    "gameModeIdempotencyKeyLabelText",
    "oldRpcUnableGetAscText",
    "oldRpcMissingAscText",
    "oldRpcMissingWorldText",
    "oldRpcMissingOwnerText",
    "oldRpcInputMismatchText",
    "oldRpcMissingDbaAbilitySystemText",
    "oldRpcUltimateAscInvalidText",
    "rpcUnableGetAbilitySystemText",
    "rpcMissingAbilitySystemText",
    "rpcMissingWorldObjectText",
    "rpcMissingOwnerText",
    "rpcInputMismatchText",
    "rpcMissingDbaAbilitySystemText",
    "rpcUltimateAbilitySystemInvalidText",
    "DBARpcHandler.cpp",
    "oldRuntimeServiceConfiguredText",
    "oldRuntimeServiceSessionLabelText",
    "oldRuntimeServiceServerLabelText",
    "oldRuntimeServiceTokenLabelText",
    "oldRuntimeServiceRequestFailedText",
    "runtimeServiceConfiguredText",
    "runtimeServiceSessionLabelText",
    "runtimeServiceServerLabelText",
    "runtimeServiceTokenLabelText",
    "runtimeServiceRequestFailedText",
    "GameBackendRuntimeService.cpp",
    "gameInstanceNoneText",
    "GameBackendRuntimeServiceTests.cpp",
    "Payload should parse as JSON",
    "honor reward should be serialized",
    "DBAOnlineAccountServiceTests.cpp",
    "EndpointMissing should expose backend contract drift instead of falling back",
    "Command line guest account should be valid",
    "DBAFrontendFlowTests.cpp",
    "Guest login should enter CharacterCreate on empty role list",
    "Rejected travel context must not replace stored FixedSkillGroupId",
    "DBAOnlineAccountJsonTests.cpp",
    "Login response should parse",
    "Refresh token response should not be classified as guest by JSON shape alone",
    "DBAPlayableSkillCatalogTests.cpp",
    "Default skill catalog validates",
    "SkillSlot is duplicated",
    "Catalog-only mode has no slot 2",
    "DBAArenaHUDEventFeedTests.cpp",
    "Controller should be created",
    "Skill hit confirmed",
    "BlankEventFeedEntryIsIgnored",
    "DBAArenaHUDCombatAnnouncementTests.cpp",
    "Combat announcement should start without cached entry",
    "Chain Ready",
    "BlankCombatAnnouncementIsIgnored",
    "DBAArenaHUDObjectiveStateTests.cpp",
    "Objective state should start invalid",
    "Capture Shrine",
    "BlankObjectiveUpdateIsIgnored",
    "DBAArenaHUDCriticalStateTests.cpp",
    "Critical state should start invalid",
    "Critical state should cache latest update",
    "Critical state should clear low energy",
    "DBAArenaHUDUltimateReadyPromptTests.cpp",
    "Ultimate-ready prompt should start invalid",
    "Ultimate-ready prompt should cache latest shown state",
    "Ultimate-ready prompt should cache hidden state",
    "DBAArenaHUDStatusEffectsTests.cpp",
    "Buff cache should start empty",
    "Debuff cache should start empty",
    "CC remove should update cache",
    "DBALoginVisualLayoutTests.cpp",
    "Panel should be centered like the approved login art",
    "Primary CTA should match the reference",
    "Third tool is repair",
    "DBAMainLobbyMatchHistoryTests.cpp",
    "Match history JSON should update recent summary",
    "Recent combat summary should format",
    "Recent reward summary should include all numeric rewards",
    "GameBackendPlayerServiceTests.cpp",
    "Match history envelope should parse",
    "Coin reward should parse",
    "Honor reward should parse",
    "GameBackendSessionServiceTests.cpp",
    "Travel URL should contain SessionId",
    "Nested connection JSON should build a travel URL",
    "Response envelope data should build a travel URL",
    "DBACharacterBuildTypesTests.cpp",
    "FixedSkillGroupId should depend on Zodiac + Element",
    "Changing FiveCamp should not change FixedSkillGroupId",
    "Travel context should reject a tampered FixedSkillGroupId",
    "DBAUrlOptionsTests.cpp",
    "Dedicated Server options should reject tampered FixedSkillGroupId",
    "Dedicated Server options should reject missing TeamId",
    "DBAFixedSkillGroupDataTests.cpp",
    "DataAsset fixed skill group row name should match backend/runtime FixedSkillGroupId",
    "Missing identity dimensions should not produce a valid data-table row name",
    "FixedSkillGroups DataTable should contain 60 Zodiac x Element rows",
    "DBAAIShowcaseTests.cpp",
    "WBP_MainMenu should expose AIShowcaseMenu_TitleText",
    "Interactive prop should default to active",
    "AI_Showcase interactive prop should stay near the documented placement",
    'TEXT("None")',
    "Dedicated Server",
    "Runtime match-ended",
    "Runtime player-joined",
    "PlayerId=%s",
    "IdempotencyKey=%s",
    " Client ",
    " Editor ",
    "zodiac=%d",
    "success=%s error=%s members=%d",
    "AGENTS.md",
    "validate-unreal-source-guardrails.ps1",
    "test-production-evidence-automation.ps1",
    "validate-production-evidence-contracts.ps1",
    "PASS: Unreal Chinese log output policy contract"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GameInstance\DBAGameInstance.cpp" @(
    "zodiac=%d",
    "success=%s error=%s members=%d",
    'TEXT("None")'
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\DivineBeastsArena.cpp" @(
    "Dedicated Server",
    " Client ",
    " Editor "
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "Dedicated Server"
)

Assert-FileContains "scripts\test-unreal-ui-runtime-chinese-output-contract.ps1" @(
    "UE UI runtime Chinese output contract",
    "Press ESC to skip",
    "[UDBASplashVideoWidget] NativeConstruct",
    "Failed to create MediaPlayer",
    "Media open requested",
    "Player service unavailable.",
    "Room service unavailable.",
    "Match service unavailable.",
    "Session service unavailable.",
    "Match history response could not be parsed.",
    "Party panel widget blueprint is unavailable.",
    "Ready check completed:",
    "Portal cancelled.",
    "Loaded cursor texture asset:",
    "Failed to load cursor PNG:",
    "Failed to load any preview skeletal mesh.",
    "Mesh has no skeleton, skip idle animation.",
    "Characters updated:",
    "Native fallback layout created",
    "Failed to spawn BGM component.",
    "Submitted character creation:",
    "Failed to spawn world 3D character presentation stage.",
    "GuestLoginButton=",
    "DebugLoginButton=",
    "LoginButton \u4e3a\u7a7a",
    "garbledFlowStateText",
    "garbledEnterLobbyText",
    "garbledSharedLobbyServerText",
    "Online account service initialized:",
    "Online login failed",
    "Online registration failed",
    "Auto login failed",
    "Online character list request failed:",
    "Online character creation failed",
    "Online character selection request failed:",
    "Mock fallback unavailable",
    "Backend error:",
    "Staged movie missing, using source path",
    "Startup movie file missing or empty",
    "UDBAStartupVideoWidget.cpp",
    "startupVideoDedicatedServerText",
    "splashDedicatedServerSkipText",
    "splashDedicatedServerOpenedText",
    "splashDedicatedServerFailedText",
    "oldWaitingPlayerControllerText",
    "oldLobbyHudWaitingPlayerControllerText",
    "oldPlayerControllerCountText",
    "oldMissingPlayerControllerText",
    "oldArenaHudWidgetUnavailableText",
    "oldLobbyPlayerHudWidgetUnavailableText",
    "oldLobbyGameHudShownText",
    "oldLobbyPlayerHudViewportText",
    "oldLobbyHudRetryLimitText",
    "oldLobbyHudWaitingText",
    "oldLobbyPlayerHudCreatedText",
    "waitingPlayerControllerText",
    "lobbyPlayerInterfaceWaitingText",
    "playerControllerCountText",
    "missingPlayerControllerText",
    "levelAddressText",
    "arenaInterfaceWidgetUnavailableText",
    "lobbyPlayerInterfaceWidgetUnavailableText",
    "lobbyGameInterfaceShownText",
    "lobbyPlayerInterfaceViewportText",
    "lobbyInterfaceRetryLimitText",
    "lobbyInterfaceWaitingText",
    "lobbyPlayerInterfaceCreatedText",
    "Dedicated Server",
    "URL=%s",
    "Ticket created but ticket id is missing.",
    "UDBASplashVideoWidget.cpp",
    "DBAGameUIManager.cpp",
    "UDBASoftwareCursorWidget.cpp",
    "DBACharacterPreviewActor.cpp",
    "UDBACharacterSelectFlowWidgetBase.cpp",
    "UDBACharacterCreateFlowWidgetBase.cpp",
    "UDBALoginFlowWidgetBase.cpp",
    "DBALoginFlowSubsystem.cpp",
    "Account service unavailable",
    "Character selection failed",
    "Character selection is not available in current state",
    "Character creation is not available in current state",
    "DBAOnlineAccountService.cpp",
    "DBAOnlineAccountJson.cpp",
    "Malformed JSON response",
    "Missing characters array",
    "DBAAccountServiceBase.cpp",
    "LoadAccountSaveGame -",
    "SaveAccountSaveGame -",
    "LoadProfileSaveGame -",
    "SaveProfileSaveGame -",
    "CreateDefaultAccountSaveGame -",
    "CreateDefaultProfileSaveGame -",
    "HandleCorruptedSaveGame -",
    "DBAPlayableSkillCatalogDataAsset.cpp",
    "DBAPlayableSkillComponent.cpp",
    "SkillSlot=%d SkillId=%s",
    "ProjectileClass is not configured",
    "EffectShape is invalid",
    "SkillSpecs is empty",
    "Playable skill catalog validation failed:",
    "DBAMockAccountService.cpp",
    "GuestLogin -",
    "AutoLogin -",
    "CreateCharacter -",
    "SelectCharacter -",
    "PerformGuestLogin -",
    "DBAPartyServiceBase.cpp",
    "Party service initialized",
    "CreateParty failed: account not logged in",
    "Party not created",
    "Member not found or is leader",
    "DBAQueueServiceBase.cpp",
    "Queue service initialized",
    "StartQueue failed: missing logged-in account or party",
    "CancelQueue succeeded",
    "GameBackendAuthService.cpp",
    "GameBackendConfigService.cpp",
    "GameBackendMailService.cpp",
    "GameBackendMatchService.cpp",
    "GameBackendPlayerService.cpp",
    "GameBackendRoomService.cpp",
    "GameBackendRuntimeService.cpp",
    "GameBackendSessionService.cpp",
    "GameBackendHttpClient.cpp",
    "Request failed.",
    "Auth request failed.",
    "Runtime request failed.",
    "Backend subsystem invalid.",
    "UDBAMainLobbyWidgetController.cpp",
    "UDBALobbyPlayerHUDWidgetBase.cpp",
    "[LobbyPlayerHUD] Constructed:",
    "[LobbyPlayerHUD] Loaded skill hotkeys from FixedSkillGroup asset:",
    "PASS: UE UI runtime Chinese output contract"
)

Assert-FileContains "scripts\test-production-evidence-automation.ps1" @(
    "test-unreal-ui-runtime-chinese-output-contract.ps1",
    "UE UI runtime Chinese output contract"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Splash\UDBASplashVideoWidget.cpp" @(
    "Staged movie missing, using source path",
    "Startup movie file missing or empty",
    "Dedicated Server"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Startup\UDBAStartupVideoWidget.cpp" @(
    "Dedicated Server"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp" @(
    "URL=%s"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp" @(
    "Ticket created but ticket id is missing."
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBALobbyPlayerHUDWidgetBase.cpp" @(
    "[LobbyPlayerHUD] Constructed:",
    "[LobbyPlayerHUD] Loaded skill hotkeys from FixedSkillGroup asset:"
)

Assert-FileContains "scripts\test-gas-ability-cpp-lifecycle-boundary.ps1" @(
    "GAS ability C++ lifecycle boundary",
    "DBAElementSkillAbility_Generic.h",
    "DBAZodiacUltimateAbility_Generic.h",
    "DBAZodiacPassiveAbility_Generic.h",
    "Blueprint(Implementable|Native)Event",
    "virtual\s+void",
    "PASS: GAS ability C++ lifecycle boundary"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAElementSkillAbility_Generic.h" @(
    "BlueprintImplementableEvent",
    "BlueprintNativeEvent"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacUltimateAbility_Generic.h" @(
    "BlueprintImplementableEvent",
    "BlueprintNativeEvent"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacPassiveAbility_Generic.h" @(
    "BlueprintImplementableEvent",
    "BlueprintNativeEvent"
)

Assert-FileContains "scripts\test-unreal-source-guardrails.ps1" @(
    "completeSessionServiceContract",
    "completeSessionTypesContract",
    "completeSessionTestsContract",
    "completeUrlOptionsContract",
    "completeUrlOptionsTestsContract",
    "missingGameModeLifecycleContract",
    "missingSessionServiceContract",
    "missingSessionTeamContract",
    "missingUrlOptionsContract",
    "TryExtractTeamId",
    "missing-runtime-match-lifecycle-handoff",
    "missing-session-travel-build-summary",
    "missing-session-travel-team-id",
    "missing-url-build-summary-admission",
    "missing Runtime match lifecycle handoff",
    "missing session travel build summary contract",
    "missing session travel TeamId handoff",
    "missing Dedicated Server URL build summary admission",
    "Runtime match lifecycle handoff contract is incomplete",
    "Session travel build summary contract is incomplete",
    "Dedicated Server URL build summary admission implementation is incomplete",
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "ToStableFiveCampName",
    "ValidatesDedicatedServerBuildSummary",
    "ConnectionJsonAcceptsResponseEnvelopeData",
    "DBATeamId=1",
    "DBATeamId=2"
)

Assert-FileContains "scripts\validate-unreal-moba-foundation.ps1" @(
    "GameMoba foundation",
    "Test-BuildDependencies",
    "Test-GasFoundation",
    "Test-UiFoundation",
    "Test-NoBackendCoupling",
    "UDBAMobaAbilitySystemComponentBase",
    "UDBAMobaGameplayAbilityBase",
    "UDBAMobaHUDWidgetControllerBase",
    "UDBAMobaUserWidgetBase",
    "TWeakObjectPtr<class APlayerController>",
    "GameMoba foundation must not depend on backend client services"
)

Assert-FileContains "scripts\test-unreal-moba-foundation.ps1" @(
    "validate-unreal-moba-foundation.ps1",
    "InvalidHudController",
    "Expected invalid HUD controller fixture to fail",
    "PASS: Unreal Moba foundation fixtures"
)

Assert-FileContains "scripts\test-player-unit-frame-controller-contract.ps1" @(
    "UDBAPlayerUnitFrameWidgetController",
    "SetVitals",
    "SetCurrentLevel",
    "SetOwningPlayerController",
    "GetOwningPlayerController",
    "TWeakObjectPtr<class APlayerController>",
    "return 850.0f;",
    "PlayerUnitFrame controller still returns placeholder literal",
    "PASS: PlayerUnitFrame controller contract"
)

Assert-FileContains "scripts\test-player-unit-frame-widget-binding.ps1" @(
    "UDBAPlayerUnitFrameWidgetBase",
    "HandleControllerHPUpdated",
    "NativeConstruct",
    "CachedCurrentXP\(0\.0f\)",
    "CachedMaxXP\(100\.0f\)",
    "UpdateHP\(CachedCurrentHP,\s*CachedMaxHP\)",
    "HealthBar->SetPercent",
    "FMath::Clamp",
    "OnHPUpdated\.AddDynamic",
    "OnHPUpdated\.RemoveDynamic",
    "GetCurrentHP\(\)",
    "UpdateHP\(CurrentHP,\s*MaxHP\)",
    "PASS: PlayerUnitFrame widget binding contract"
)

Assert-FileContains "scripts\test-player-unit-frame-ultimate-energy-max-sync.ps1" @(
    "UpdateUltimateEnergyWithMax",
    "CachedMaxUltimateEnergy",
    "DBAConstants::MaxUltimateEnergy",
    "Percentage",
    "CachedUltimateEnergy",
    "PASS: PlayerUnitFrame UltimateEnergy max sync contract"
)

Assert-FileContains "scripts\test-arena-hud-controller-player-unit-frame.ps1" @(
    "UDBAArenaHUDWidgetController",
    "GetPlayerUnitFrameWidgetController",
    "SetPlayerUnitFrameWidgetController",
    "InitializeController",
    "Super::InitializeController",
    "PlayerUnitFrameWidgetController->SetOwningPlayerController",
    "UpdatePlayerLevel",
    "PlayerUnitFrameWidgetController->SetVitals",
    "PASS: Arena HUD controller PlayerUnitFrame contract"
)

Assert-FileContains "scripts\test-arena-hud-root-player-unit-frame-handoff.ps1" @(
    "UDBAArenaHUDRootWidgetBase",
    "SetPlayerUnitFrameWidgetController",
    "PlayerUnitFrameWidgetController",
    "PlayerUnitFrame->SetWidgetController",
    "PASS: Arena HUD root PlayerUnitFrame handoff contract"
)

Assert-FileContains "scripts\test-game-ui-manager-arena-hud-controller.ps1" @(
    "DBAGameUIManager",
    "UDBAArenaHUDWidgetController",
    "ShowArenaHUD",
    "CreateArenaHUDWidget",
    "direct Arena HUD widget factory calls",
    "IsWorldSafeForWidgetCreation\(World\)",
    "IsServerLikeRuntime\(World\)",
    "NewObject<\s*UDBAArenaHUDWidgetController",
    "EnsureArenaHUDWidgetController\(PC\)",
    "return\s+nullptr",
    "InPlayerController->GetWorld\(\)\s*!=\s*World",
    "ArenaHUDWidgetController->InitializeController\(InPlayerController\)",
    "ArenaHUDWidget->SetWidgetController\(ArenaHUDWidgetController\)",
    "PASS: Game UI manager Arena HUD controller contract"
)

Assert-FileContains "scripts\test-game-ui-manager-arena-hud-runtime-updates.ps1" @(
    "UpdateArenaHUDPlayerVitals",
    "UpdateArenaHUDPlayerLevel",
    "EnsureArenaHUDWidgetController",
    "Controller->UpdatePlayerHP",
    "Controller->UpdatePlayerEnergy",
    "Controller->UpdatePlayerLevel",
    "PASS: Game UI manager Arena HUD runtime updates contract"
)

Assert-FileContains "scripts\test-game-ui-manager-arena-hud-entrypoint-server-boundaries.ps1" @(
    "GetArenaHUDLocalPlayerController",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "ShowArenaHUD",
    "UpdateArenaHUDPlayerVitals",
    "AddArenaHUDEventFeedEntry",
    "BindArenaHUDToCharacter",
    "PASS: Game UI manager Arena HUD entrypoint server boundaries"
)

Assert-FileContains "scripts\test-game-ui-manager-arena-hud-hide-server-boundary.ps1" @(
    "HideArenaHUD",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "ArenaHUDWidget->RemoveFromParent()",
    "bArenaHUDVisible = false",
    "PASS: Game UI manager Arena HUD hide server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-main-lobby-entrypoint-server-boundary.ps1" @(
    "ShowMainLobby",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "HideAllFlowWidgets()",
    "AddToViewport",
    "ApplyLobbyGameplayInputMode(World)",
    "IsLobbyGameplayWorldForUIManager(World)",
    "PASS: Game UI manager MainLobby entrypoint server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-main-lobby-hide-server-boundary.ps1" @(
    "HideMainLobby",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "MainLobbyWidget->RemoveFromParent()",
    "bMainLobbyVisible = false",
    "HideLobbyPlayerHUD()",
    "PASS: Game UI manager MainLobby hide server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-interaction-progress-server-boundary.ps1" @(
    "UpdateInteractionProgress",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "InteractionPromptWidget->UpdateInteractionProgress(Progress)",
    "PASS: Game UI manager interaction progress server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-interaction-prompt-hide-server-boundary.ps1" @(
    "HideInteractionPrompt",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "InteractionPromptWidget->HidePrompt()",
    "InteractionPromptWidget->RemoveFromParent()",
    "PASS: Game UI manager interaction prompt hide server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-lobby-hud-server-boundary.ps1" @(
    "ShowLobbyPlayerHUD",
    "CreateLobbyPlayerHUDWidget",
    "IsServerLikeRuntime\(World\)",
    "direct Lobby Player HUD factory calls",
    "CreateWidget<\s*UDBALobbyPlayerHUDWidgetBase",
    "PASS: Game UI manager Lobby HUD server boundary contract"
)

Assert-FileContains "scripts\test-game-ui-manager-widget-factory-server-boundaries.ps1" @(
    "CreateMainLobbyWidget",
    "CreateGameSettingsWidget",
    "CreateInventoryWidget",
    "CreatePartyPanelWidget",
    "CreateInvitePanelWidget",
    "CreateQueueModeSelectWidget",
    "CreateQueueStatusWidget",
    "CreateReadyCheckWidget",
    "CreateMatchFoundWidget",
    "CreatePortalConfirmWidget",
    "CreateInteractionPromptWidget",
    "CreateNewbieVillageMainWidget",
    "CreateNewbieTaskTrackerWidget",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "CreateWidget<",
    "PASS: Game UI manager widget factory server boundaries"
)

Assert-FileContains "scripts\test-game-ui-manager-retry-timer-server-boundaries.ps1" @(
    "ScheduleFlowWidgetRefreshRetry",
    "ScheduleLobbyHUDRefreshRetry",
    "HandleFlowWidgetRefreshRetry",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "World->GetTimerManager().IsTimerActive(FlowWidgetRefreshRetryTimerHandle)",
    "World->GetTimerManager().IsTimerActive(LobbyHUDRefreshRetryTimerHandle)",
    "++FlowWidgetRefreshRetryCount",
    "++LobbyHUDRefreshRetryCount",
    "RefreshLoginFlowWidgetVisibility();",
    "PASS: Game UI manager retry timer server boundaries"
)

Assert-FileContains "scripts\test-game-ui-manager-login-flow-state-server-boundary.ps1" @(
    "HandleLoginFlowStateChanged",
    "RefreshLoginFlowWidgetVisibility",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "CachedLoginFlowState = NewState",
    "TransitionTo(EDBAUIState::Lobby);",
    "IsLobbyGameplayWorldForUIManager(World)",
    "HideAllFlowWidgets();",
    "StopLoginFlowBackgroundMusic();",
    "EnsureFlowWidgetCreated(",
    "ScheduleFlowWidgetRefreshRetry();",
    "EnsureLoginFlowBackgroundMusic();",
    "PASS: Game UI manager login flow state server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-login-flow-start-server-boundary.ps1" @(
    "EnsureLoginFlowStartedFromManager",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "DBAInstance->StartLoginFlow();",
    "LoginFlow->StartLoginFlow();",
    "bLoginFlowStartRequested = true;",
    "PASS: Game UI manager login flow start server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-login-flow-server-boundary.ps1" @(
    "EnsureFlowWidgetCreated",
    "SetFlowWidgetVisible",
    "ShowSplashVideo",
    "EnsureLoginFlowBackgroundMusic",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "CreateWidget<WidgetType>",
    "CreateWidget<UDBASplashVideoWidget>",
    "UGameplayStatics::SpawnSound2D",
    "PASS: Game UI manager login flow server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-login-flow-hide-server-boundary.ps1" @(
    "HideLoginFlowWidget",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "HideAllFlowWidgets()",
    "PASS: Game UI manager login flow hide server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-input-mode-restore-server-boundary.ps1" @(
    "RestoreInputModeAfterOverlayClosed",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "ApplyFrontendInputMode(World, FocusWidget);",
    "ApplyLobbyGameplayInputMode(World);",
    "PASS: Game UI manager input mode restore server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-splash-video-timer-server-boundary.ps1" @(
    "OnSubsystemInitialize",
    "TryShowSplashVideo",
    "UWorld* World = GetWorld();",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "World->GetTimerManager().SetTimer",
    "World->GetTimerManager().ClearTimer",
    "EnsureLoginFlowStartedFromManager()",
    "ShowSplashVideo();",
    "PASS: Game UI manager splash video timer server boundary"
)

Assert-FileContains "scripts\test-game-ui-manager-splash-video-hide-server-boundary.ps1" @(
    "HideSplashVideo",
    "IsWorldSafeForWidgetCreation(World)",
    "IsServerLikeRuntime(World)",
    "SplashVideoWidget->IsInViewport()",
    "SplashVideoWidget->RemoveFromParent()",
    "PASS: Game UI manager splash video hide server boundary"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-sync.ps1" @(
    "GetHeroLevel",
    "SyncArenaHUDFromAttributes",
    "UDBAHeroGrowthAttributeSet",
    "UDBAGameUIManager",
    "UpdateArenaHUDPlayerVitals",
    "UpdateArenaHUDPlayerLevel",
    "PASS: Zodiac character Arena HUD sync contract"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-sync-cache.ps1" @(
    "bHasSyncedArenaHUDAttributes",
    "LastSyncedArenaHUDCurrentHP",
    "LastSyncedArenaHUDCurrentEnergy",
    "LastSyncedArenaHUDHeroLevel",
    "bVitalsChanged",
    "bLevelChanged",
    "PASS: Zodiac character Arena HUD sync cache contract"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-attribute-delegates.ps1" @(
    "BindArenaHUDAttributeDelegates",
    "UnbindArenaHUDAttributeDelegates",
    "GetGameplayAttributeValueChangeDelegate",
    "HandleArenaHUDAttributeChanged",
    "bHasBoundArenaHUDAttributeDelegates",
    "PASS: Zodiac character Arena HUD attribute delegate contract"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-critical-state.ps1" @(
    "ArenaHUDCriticalHealthRatioThreshold",
    "ArenaHUDCriticalEnergyRatioThreshold",
    "bHasSyncedArenaHUDCriticalState",
    "LastSyncedArenaHUDBLowHP",
    "LastSyncedArenaHUDBLowEnergy",
    "UpdateArenaHUDCriticalStateHints",
    "PASS: Zodiac character Arena HUD critical state contract"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-ultimate-ready-prompt.ps1" @(
    "bHasSyncedArenaHUDUltimateReadyPrompt",
    "bLastSyncedArenaHUDUltimateReady",
    "DBAConstants::MaxUltimateEnergy",
    "ShowArenaHUDUltimateReadyPrompt",
    "HideArenaHUDUltimateReadyPrompt",
    "PASS: Zodiac character Arena HUD UltimateReadyPrompt contract"
)

Assert-FileContains "scripts\test-zodiac-character-ultimate-energy-constants.ps1" @(
    "IsUltimateReady",
    "DBAConstants::MaxUltimateEnergy",
    "SetUltimateEnergy",
    "AddUltimateEnergy",
    "PASS: Zodiac character UltimateEnergy constants contract"
)

Assert-FileContains "scripts\test-zodiac-character-skill-slot-count-constants.ps1" @(
    "DBAConstants::ArenaCombatSkillSlotCount",
    "DBAConstants::PlayableSkillSlotCount",
    "DBAConstants::PlayableSkillArraySize",
    "DBAZodiacCharacterBase.cpp",
    "PASS: Zodiac character skill slot count constants contract"
)

Assert-FileContains "scripts\test-zodiac-character-gas-input-activation-bridge.ps1" @(
    "TryActivateAbilityByInputID",
    "MapEquippedSkillSlotToAbilityInputID",
    "EDBAAbilityInputID::Skill01",
    "EDBAAbilityInputID::Ultimate",
    "PASS: Zodiac character GAS input activation bridge contract"
)

Assert-FileContains "scripts\test-zodiac-character-server-cast-authority-boundary.ps1" @(
    "ValidateServerEquippedSkillCast",
    "HasAuthority()",
    "IsLobbyEquippedSkillSlot(SkillSlot)",
    "IsDead()",
    "TargetActor && !IsValid(TargetActor)",
    "ServerCastEquippedSkill_Implementation",
    "CastEquippedSkillInternal",
    "PASS: Zodiac character server cast authority boundary"
)

Assert-FileContains "scripts\test-zodiac-character-server-cast-rpc-validation.ps1" @(
    "WithValidation",
    "ServerCastLobbyFireball_Validate",
    "ServerCastLobbyFireballAtTarget_Validate",
    "ServerCastEquippedSkill_Validate",
    "ValidateServerEquippedSkillCast(1, nullptr)",
    "ValidateServerEquippedSkillCast(1, TargetActor)",
    "ValidateServerEquippedSkillCast(SkillSlot, TargetActor)",
    "PASS: Zodiac character server cast RPC validation contract"
)

Assert-FileContains "scripts\test-zodiac-ultimate-energy-cost-constants.ps1" @(
    "DBAZodiacUltimateAbilityBase.cpp",
    "DBARpcHandler.cpp",
    "HasEnoughUltimateEnergy",
    "ConsumeUltimateEnergy",
    "DBAConstants::MaxUltimateEnergy",
    "PASS: Zodiac ultimate energy cost constants contract"
)

Assert-FileContains "scripts\test-zodiac-character-arena-hud-chain-announcement.ps1" @(
    "ArenaHUDChainReadyAnnouncementDuration",
    "bLastSyncedArenaHUDChainReady",
    "DBAConstants::MaxChainLevel",
    "ShowArenaHUDCombatAnnouncement",
    "ChainReadyAnnouncement",
    '$successMessage'
)

Assert-FileContains "scripts\test-arena-hud-ability-bar-character-binding.ps1" @(
    "BindArenaHUDToCharacter",
    "AbilityBar->BindToCharacter",
    "NativeConstruct",
    "CacheSkillSlotWidgets",
    "BoundCharacter\.IsValid\(\)",
    "ArenaHUDCharacter",
    "UIManager->BindArenaHUDToCharacter",
    "PASS: Arena HUD AbilityBar character binding contract"
)

Assert-FileContains "scripts\test-arena-ability-bar-cooldown-slot-indexing.ps1" @(
    "UDBAAbilityBarWidgetBase.cpp",
    "CooldownArrayIndex",
    "SkillSlot\s*-\s*1",
    "Cooldowns\[CooldownArrayIndex\]",
    "ClampedCooldown",
    "ClampedManaCost",
    "DBAAbilitySlotWidget.cpp",
    "NativeConstruct",
    "UpdateHotkeyDisplay",
    "FSlateBrush",
    "FText::GetEmpty",
    "SetAbilityInfo",
    "AbilityInfo\.CurrentCooldown",
    "PASS: Arena AbilityBar cooldown slot indexing contract"
)

Assert-FileContains "scripts\test-zodiac-character-legacy-cooldown-indexing.ps1" @(
    "CastEquippedSkillInternal",
    "CooldownArrayIndex",
    "SkillSlot - 1",
    "SkillCooldowns[CooldownArrayIndex] = Spec.Cooldown",
    "SkillMaxCooldowns[CooldownArrayIndex] = Spec.Cooldown",
    "OnSkillCooldownsChanged.Broadcast(SkillCooldowns)",
    "PASS: Zodiac character legacy cooldown indexing contract"
)

Assert-FileContains "scripts\test-zodiac-character-ability-cooldown-query.ps1" @(
    "IsAbilityOnCooldown",
    "IDBACharacterRef",
    "SkillId.IsNone()",
    "GetPlayableSkillSpecs()",
    "SkillSpec\.SkillId",
    "CooldownArrayIndex",
    "SkillSlot - 1",
    "SkillCooldowns.IsValidIndex(CooldownArrayIndex)",
    "PASS: Zodiac character ability cooldown query contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp" @(
    "bool ADBAZodiacCharacterBase::IsAbilityOnCooldown(FName SkillId) const",
    "SkillId.IsNone()",
    "const TArray<FDBAPlayableSkillRuntimeSpec> PlayableSkillSpecs = GetPlayableSkillSpecs()",
    "for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : PlayableSkillSpecs)",
    "SkillSpec.SkillId == SkillId",
    "const int32 CooldownArrayIndex = SkillSpec.SkillSlot - 1",
    "SkillCooldowns.IsValidIndex(CooldownArrayIndex) && SkillCooldowns[CooldownArrayIndex] > 0.0f"
)

Assert-FileContains "scripts\test-arena-ability-bar-slot-boundary-contract.ps1" @(
    "UDBAAbilityBarWidgetBase.cpp",
    "DBAConstants::CoreCombatInputCount",
    "BP_OnAbilityUpdated",
    "BP_OnAbilityEnabledChanged",
    "PASS: Arena AbilityBar slot boundary contract"
)

Assert-FileContains "scripts\test-arena-ability-bar-cooldown-event-sync.ps1" @(
    "FOnSkillCooldownsChanged",
    "OnRep_SkillCooldowns",
    "OnSkillCooldownsChanged",
    "bRefreshCooldownsEveryTick",
    "HandleSkillCooldownsChanged",
    "PASS: Arena AbilityBar cooldown event sync contract"
)

Assert-FileContains "scripts\test-arena-overhead-widget-component-contract.ps1" @(
    "DBAOverheadWidgetComponent.h",
    "DBAOverheadWidgetComponent.cpp",
    "CachedCharacterName",
    "bCachedOverheadVisible",
    "ApplyWidgetConfig",
    "bShowHealthBar",
    "bShowName",
    "ESlateVisibility::Collapsed",
    "NM_DedicatedServer",
    "SetComponentTickEnabled\(false\)",
    "EndPlay",
    "RemoveFromParent",
    "TickComponent",
    "UpdateWidgetPosition",
    "World->GetFirstPlayerController\(\)",
    "ProjectWorldLocationToScreen",
    "OverheadWidget->SetVisibility\(ESlateVisibility::Hidden\)",
    "SetCharacterName\(CachedCharacterName\)",
    "SetOverheadVisible\(bCachedOverheadVisible\)",
    "ESlateVisibility::HitTestInvisible",
    "PASS: Arena OverheadWidgetComponent contract"
)

Assert-FileContains "scripts\test-arena-hud-status-effects-sync.ps1" @(
    "NormalizeStatusEffectId",
    "TrimStartAndEnd",
    "NormalizedBuffId",
    "CachedActiveDebuffs",
    "NormalizedDebuffId",
    "CachedActiveCCEffects",
    "NormalizedCCId",
    "PASS: Arena HUD status effects sync contract"
)

Assert-FileContains "scripts\test-arena-hud-ultimate-energy-sync.ps1" @(
    "FOnUltimateEnergyChanged",
    "ReplicatedUsing",
    "UpdateArenaHUDUltimateEnergy",
    "FMath::Clamp",
    "HandleControllerUltimateEnergyUpdated",
    "HandleArenaHUDUltimateEnergyChanged",
    "PASS: Arena HUD UltimateEnergy sync contract"
)

Assert-FileContains "scripts\test-arena-hud-ultimate-energy-default-constants.ps1" @(
    "UDBAArenaHUDWidgetController.cpp",
    "DBAZodiacCharacterBase.h",
    "DBAConstants::MaxUltimateEnergy",
    "LastSyncedArenaHUDMaxUltimateEnergy",
    "PASS: Arena HUD UltimateEnergy default constants contract"
)

Assert-FileContains "scripts\test-arena-hud-ultimate-ready-prompt-sync.ps1" @(
    "ShowUltimateReadyPrompt",
    "HideUltimateReadyPrompt",
    "FDBAArenaUltimateReadyPromptState",
    "GetLastUltimateReadyPromptState",
    "LastUltimateReadyPromptState\.bIsValid",
    "LastUltimateReadyPromptState\.bIsShown",
    "DBAArenaHUDUltimateReadyPromptTests.cpp",
    "UltimateReadyPromptCachesLatestState",
    "FOnUltimateReadyPromptShown",
    "HandleControllerUltimateReadyPromptShown",
    "UltimateReadyPrompt->ShowUltimateReady",
    "bCachedUltimateReadyVisible",
    "BP_OnUltimateReady",
    "ShowArenaHUDUltimateReadyPrompt",
    "PASS: Arena HUD UltimateReadyPrompt sync contract"
)

Assert-FileContains "scripts\test-arena-hud-chain-resonance-sync.ps1" @(
    "FOnChainLevelChanged",
    "FOnResonanceLevelChanged",
    "UpdateArenaHUDCombatState",
    "HandleControllerChainLevelUpdated",
    "HandleControllerResonanceLevelUpdated",
    "HandleArenaHUDChainLevelChanged",
    "HandleArenaHUDResonanceLevelChanged",
    "PASS: Arena HUD Chain/Resonance sync contract"
)

Assert-FileContains "scripts\test-arena-hud-chain-resonance-constants.ps1" @(
    "DBAConstants::MaxChainLevel",
    "DBAConstants::MaxResonanceLevel",
    "UDBAArenaHUDWidgetController.cpp",
    "UDBAArenaHUDRootWidgetBase.cpp",
    "DBAZodiacCharacterBase.cpp",
    "PASS: Arena HUD Chain/Resonance constants contract"
)

Assert-FileContains "scripts\test-arena-hud-chain-resonance-panel-boundaries.ps1" @(
    "UDBAChainUltimatePanelWidgetBase.cpp",
    "UDBAPassiveAndResonancePanelWidgetBase.cpp",
    "DBAConstants::MaxChainLevel",
    "DBAConstants::MaxResonanceLevel",
    "DBAConstants::CoreCombatInputCount",
    "NormalizedChainCount",
    "CachedChainCount",
    "NormalizedSlotIndex",
    "CachedPassiveSkillStates",
    "NormalizedResonanceLevel",
    "CachedResonanceLevel",
    "PASS: Arena HUD Chain/Resonance panel boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAChainUltimatePanelWidgetBase.cpp" @(
    "GameDBA/Core/DBAConstants.h",
    "NormalizedChainCount = FMath::Clamp(Count, 0, DBAConstants::MaxChainLevel)",
    "CachedChainCount = NormalizedChainCount",
    "BP_OnChainCountUpdated(CachedChainCount)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPassiveAndResonancePanelWidgetBase.cpp" @(
    "GameDBA/Core/DBAConstants.h",
    "NormalizedSlotIndex = FMath::Clamp(SlotIndex, 0, DBAConstants::CoreCombatInputCount - 1)",
    "CachedPassiveSkillStates.Add(NormalizedSlotIndex, bActive)",
    "BP_OnPassiveUpdated(NormalizedSlotIndex, CachedPassiveSkillStates[NormalizedSlotIndex])",
    "NormalizedResonanceLevel = FMath::Clamp(Level, 0, DBAConstants::MaxResonanceLevel)",
    "CachedResonanceLevel = NormalizedResonanceLevel",
    "BP_OnResonanceLevelUpdated(CachedResonanceLevel)"
)

Assert-FileContains "scripts\test-arena-hud-aura-summary-panel-contract.ps1" @(
    "UDBAAuraSummaryPanelWidgetBase.h",
    "UDBAAuraSummaryPanelWidgetBase.cpp",
    "NormalizedAuraCount",
    "CachedAuraCount",
    "BP_OnAuraCountUpdated",
    "BP_OnAuraDetailsRequested",
    "PASS: Arena HUD AuraSummaryPanel contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAuraSummaryPanelWidgetBase.cpp" @(
    "NormalizedAuraCount = FMath::Max(0, Count)",
    "CachedAuraCount = NormalizedAuraCount",
    "BP_OnAuraCountUpdated(CachedAuraCount)",
    "BP_OnAuraDetailsRequested()"
)

Assert-FileContains "scripts\test-arena-hud-connection-warning-contract.ps1" @(
    "UDBAConnectionWarningWidgetBase.h",
    "UDBAConnectionWarningWidgetBase.cpp",
    "NormalizeConnectionWarningText",
    "CachedWarningMessage",
    "bCachedWarningVisible",
    "BP_OnWarningShown",
    "BP_OnWarningHidden",
    "PASS: Arena HUD ConnectionWarning contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAConnectionWarningWidgetBase.cpp" @(
    "NormalizeConnectionWarningText",
    "TrimStartAndEnd()",
    "NormalizedText.IsEmpty()",
    "CachedWarningMessage = NormalizedMessage",
    "bCachedWarningVisible = true",
    "BP_OnWarningShown(CachedWarningMessage)",
    "CachedWarningMessage = FText::GetEmpty()",
    "bCachedWarningVisible = false",
    "BP_OnWarningHidden()"
)

Assert-FileContains "scripts\test-arena-hud-self-cast-bar-contract.ps1" @(
    "UDBASelfCastBarWidgetBase.h",
    "UDBASelfCastBarWidgetBase.cpp",
    "NormalizedDuration",
    "CachedSelfCastDuration",
    "bCachedSelfCastVisible",
    "BP_OnSelfCastProgress",
    "PASS: Arena HUD SelfCastBar contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBASelfCastBarWidgetBase.cpp" @(
    "NormalizedDuration = FMath::Max(0.0f, Duration)",
    "CachedSelfCastDuration = NormalizedDuration",
    "bCachedSelfCastVisible = true",
    "BP_OnSelfCastProgress(CachedSelfCastDuration, CachedSelfCastDuration)",
    "CachedSelfCastDuration = 0.0f",
    "bCachedSelfCastVisible = false",
    "BP_OnSelfCastProgress(0.0f, 0.0f)"
)

Assert-FileContains "scripts\test-damage-calculator-chain-constants.ps1" @(
    "DBADamageCalculator.cpp",
    "DBAConstants::MaxChainLevel",
    "DBAConstants::ChainTier1Threshold",
    "DBAConstants::ChainTier2Threshold",
    "DBAConstants::ChainTier2DamageBonus",
    "DBAConstants::ChainTier1DamageBonus",
    "PASS: DamageCalculator chain constants contract"
)

Assert-FileContains "scripts\test-damage-calculator-chain-tier-semantics.ps1" @(
    "DBADamageCalculator.cpp",
    "DBAConstants::ChainTier1Threshold",
    "DBAConstants::ChainTier2Threshold",
    "DBAConstants::ChainTier1DamageBonus",
    "DBAConstants::ChainTier2DamageBonus",
    "PASS: DamageCalculator chain tier semantics contract"
)

Assert-FileContains "scripts\test-damage-calculator-element-count-constant.ps1" @(
    "DBADamageCalculator.cpp",
    "DBAConstants.h",
    "DBAConstants::ElementCount",
    "CounterMap[DBAConstants::ElementCount]",
    "i < DBAConstants::ElementCount",
    "PASS: DamageCalculator element count constant contract"
)

Assert-FileContains "scripts\test-damage-calculator-resonance-damage-constants.ps1" @(
    "DBADamageCalculator.cpp",
    "DBAConstants.h",
    "DBAConstants::ResonanceLevel1_DamageBonus",
    "DBAConstants::ResonanceLevel2_DamageBonus",
    "DBAConstants::ResonanceLevel3_DamageBonus",
    "DBAConstants::ResonanceLevel4_DamageBonus",
    "PASS: DamageCalculator resonance damage constants contract"
)

Assert-FileContains "scripts\test-defense-reduction-constant.ps1" @(
    "DBADamageCalculator.cpp",
    "DBABattleAttributeSet.cpp",
    "DBAConstants::DefenseReductionConstant",
    "CalculatePhysicalDamageReduction",
    "PASS: Defense reduction constant contract"
)

Assert-FileContains "scripts\test-ability-system-resonance-constants.ps1" @(
    "DBAAbilitySystemComponent.cpp",
    "DBAConstants::ResonanceLevel4_SkillCount",
    "DBAConstants::ResonanceLevel3_SkillCount",
    "DBAConstants::ResonanceLevel2_SkillCount",
    "DBAConstants::ResonanceLevel1_SkillCount",
    "DBAConstants::MaxResonanceLevel",
    "PASS: AbilitySystem resonance constants contract"
)

Assert-FileContains "scripts\test-ability-system-ultimate-passive-regen-constant.ps1" @(
    "DBAAbilitySystemComponent.cpp",
    "PassiveRegenUltimateEnergy",
    "DBAConstants::UltimateEnergy_PassiveRegen",
    "PASS: AbilitySystem UltimateEnergy passive regen constant contract"
)

Assert-FileContains "scripts\test-ability-system-cooldown-slot-constants.ps1" @(
    "DBAAbilitySystemComponent.cpp",
    "CooldownSlotCount",
    "DBAConstants::ArenaCombatSkillSlotCount",
    "SlotIndex",
    "NormalizeSkillCooldowns",
    "PASS: AbilitySystem cooldown slot constants contract"
)

Assert-FileContains "scripts\test-ability-system-input-activation-feedback.ps1" @(
    "ResolveSkillCueNameForInputID",
    "OnSkillCueExecuted",
    "GetDBAAvatarCharacter",
    "EDBAAbilityInputID::Skill01",
    "EDBAAbilityInputID::Ultimate",
    '$inputFeedbackBroadcastPattern'
)

Assert-FileContains "scripts\test-ability-system-input-cooldown-authority-gate.ps1" @(
    "MapAbilityInputIDToCooldownSkillSlot",
    "EDBAAbilityInputID::Skill01",
    "EDBAAbilityInputID::Ultimate",
    "IsInputAbilityOnCooldown",
    "const ADBAZodiacCharacterBase* Character = GetDBAAvatarCharacter();",
    "not to treat PlayerState owner as the Zodiac character",
    "CooldownSkillSlot",
    "Character->GetPlayableSkillSpecs()",
    "SkillSpec\.SkillSlot",
    "Character->IsAbilityOnCooldown",
    "PASS: AbilitySystem input cooldown authority gate contract"
)

Assert-FileContains "scripts\test-ability-system-target-teamid-cpp-boundary.ps1" @(
    "ResolveActorTeamIdForAbilityTargeting",
    "Execute_GetTeamId",
    "Cast<ADBAZodiacCharacterBase>(Actor)",
    "GetTeamID()",
    "SourceTeamId\s*!=\s*TargetTeamId",
    "PASS: AbilitySystem target TeamId C++ boundary"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp" @(
    "int32 MapAbilityInputIDToCooldownSkillSlot(int32 InputID)",
    "case EDBAAbilityInputID::Skill01:",
    "case EDBAAbilityInputID::Ultimate:",
    "return DBAConstants::ArenaCombatSkillSlotCount;",
    "ADBAZodiacCharacterBase* UDBAAbilitySystemComponent::GetDBAAvatarCharacter() const",
    "ActorInfo->AvatarActor.Get()",
    "bool UDBAAbilitySystemComponent::IsInputAbilityOnCooldown(int32 InputID) const",
    "const ADBAZodiacCharacterBase* Character = GetDBAAvatarCharacter();",
    "const int32 CooldownSkillSlot = MapAbilityInputIDToCooldownSkillSlot(InputID);",
    "Character->GetPlayableSkillSpecs()",
    "SkillSpec.SkillSlot == CooldownSkillSlot",
    "Character->IsAbilityOnCooldown(SkillSpec.SkillId)"
)

Assert-FileContains "scripts\test-ability-system-avatar-actor-context-contract.ps1" @(
    "GetDBAAvatarCharacter",
    "AbilityActorInfo",
    "AvatarActor",
    "Input cooldown gate must resolve the avatar character.",
    "Target validation must resolve source from avatar character.",
    "GameplayCue must prefer avatar character as Instigator.",
    "Cooldown mirror sync must resolve the avatar character."
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp" @(
    "bool ResolveActorTeamIdForAbilityTargeting(const AActor* Actor, int32& OutTeamId)",
    "const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Actor);",
    "const int32 ResolvedTeamId = ZodiacCharacter->GetTeamID();",
    "ResolvedTeamId > 0",
    "ResolveActorTeamIdForAbilityTargeting(SourceActor, SourceTeamId)",
    "ResolveActorTeamIdForAbilityTargeting(Target, TargetTeamId)",
    "return SourceTeamId != TargetTeamId;"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp" @(
    "Execute_GetTeamId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h" @(
    "bool IsInputAbilityOnCooldown(int32 InputID) const"
)

Assert-FileContains "scripts\test-android-touch-input-bridge-server-boundary.ps1" @(
    "IsTouchInputRuntimeAllowed",
    "NM_DedicatedServer",
    "BeginPlay",
    "OnSkillButtonLongPressStart",
    "OnSkillButtonDrag",
    "OnSkillButtonRelease",
    "UpdateUltimateButtonState",
    "OnSkillWheelShowEvent.Broadcast",
    "OnSkillDirectionUpdateEvent.Broadcast",
    "OnSkillReleasedEvent.Broadcast",
    "PASS: Android touch input bridge server boundary"
)

Assert-FileContains "scripts\test-lobby-player-controller-local-input-binding.ps1" @(
    "SetupInputComponent",
    "!IsLocalController()",
    "InputComponent->BindAxis(",
    "InputComponent->BindAction(",
    "InputComponent->BindKey(",
    "Skill01",
    "LeftMouseButton",
    "PASS: Lobby PlayerController local input binding contract"
)

Assert-FileContains "scripts\test-lobby-player-controller-local-skill-cast-boundary.ps1" @(
    "CastEquippedSkillSlot",
    "!IsLocalController()",
    "GetPawn()",
    "ResolveAutoAttackTarget();",
    "ZodiacPawn->CastEquippedSkillAtTarget",
    "ZodiacPawn->CastEquippedSkill(SkillSlot);",
    "HandleUltimatePressed",
    "PASS: Lobby PlayerController local skill cast boundary"
)

Assert-FileContains "scripts\test-zodiac-character-local-skill-rpc-boundary.ps1" @(
    "CastEquippedSkill",
    "CastEquippedSkillAtTarget",
    "!HasAuthority() && !IsLocallyControlled()",
    "ServerCastEquippedSkill(",
    "CastEquippedSkillInternal",
    "PASS: Zodiac character local skill RPC boundary"
)

Assert-FileContains "scripts\test-zodiac-character-internal-cast-authority-boundary.ps1" @(
    "CastEquippedSkillInternal",
    "!HasAuthority()",
    "GetWorld()",
    "TryActivateAbilityByInputID",
    "SpawnActor<",
    "SkillCooldowns[CooldownArrayIndex] = Spec.Cooldown",
    "MulticastPlayLobbySkillCastFeedback",
    "PASS: Zodiac character internal cast authority boundary"
)

Assert-FileContains "scripts\test-client-prediction-local-runtime-boundary.ps1" @(
    "IsPredictionRuntimeAllowed",
    "NM_DedicatedServer",
    "IsLocallyControlled()",
    "TryPredictAbility",
    "TryPredictMove",
    "ApplyServerCorrection",
    "OnMoveCorrected",
    "ResolvePredictionAbilityInputID",
    "FindAbilitySpecHandleByInputID",
    "Params.AbilityHandle = AbilityHandle;",
    "Params.AbilityHandle = FGameplayAbilitySpecHandle();",
    "PrimaryComponentTick.bCanEverTick = false",
    "PASS: Client prediction local runtime boundary"
)

Assert-FileContains "scripts\test-rpc-handler-server-character-context.ps1" @(
    "ValidateServerCharacterContext",
    "ServerTryActivateAbility_Implementation",
    "ServerCancelAbility_Implementation",
    "ServerLockTarget_Implementation",
    "ServerMoveTo_Implementation",
    "ServerLockTarget_Validate",
    "ServerMoveTo_Validate",
    "ServerRequestAttack_Validate",
    "ServerUltimateAbility_Validate",
    "ValidateEnergyCost",
    "GetAbilitySystemComponent()",
    "PASS: RPC handler server character context contract"
)

Assert-FileContains "scripts\test-rpc-handler-server-move-execution.ps1" @(
    "ServerMoveTo_Implementation",
    "ServerMoveTo_Validate",
    "OwnerActor->SetActorLocation(Location)",
    "ClientMoveCorrection_Implementation(Location, ServerTime)",
    "World->GetTimeSeconds()",
    "if (!World)",
    "PASS: RPC handler server move execution contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    "OwnerActor->SetActorLocation(Location)",
    "const float ServerTime = World->GetTimeSeconds();",
    "ClientMoveCorrection_Implementation(Location, ServerTime)",
    "if (!World)",
    "ServerMoveTo_Validate"
)

Assert-FileContains "scripts\test-rpc-handler-server-lock-target-execution.ps1" @(
    "GetLockedTargetActor",
    "LockedTargetActor = TargetActor",
    "ServerLockTarget_Validate",
    "IsEnemy(GetOwner(), TargetActor)",
    "FindAttackTarget",
    "ValidateCastRange(LockedTarget, DBAConstants::DefaultAttackRange)",
    "return LockedTarget",
    "PASS: RPC handler server lock target execution contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h" @(
    "AActor* GetLockedTargetActor() const",
    "TObjectPtr<AActor> LockedTargetActor"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    "ValidateTarget(TargetActor)",
    "IsEnemy(GetOwner(), TargetActor)",
    "if (!IsEnemy(GetOwner(), TargetActor))",
    "LockedTargetActor = TargetActor",
    "AActor* LockedTarget = LockedTargetActor.Get()",
    "ValidateCastRange(LockedTarget, DBAConstants::DefaultAttackRange)",
    "return LockedTarget"
)

Assert-FileContains "scripts\test-rpc-handler-stale-locked-target-clear.ps1" @(
    'AActor* FindAttackTarget();',
    'LockedTargetActor = nullptr',
    'OverlapMultiByObjectType',
    'PASS: RPC handler stale locked target clear contract'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    'AActor* ADBARpcHandler::FindAttackTarget()',
    'if (LockedTarget)',
    'LockedTargetActor = nullptr',
    'OverlapMultiByObjectType'
)

Assert-FileContains "scripts\test-rpc-handler-server-attack-execution.ps1" @(
    "DBADamageCalculator",
    "ServerRequestAttack_Implementation",
    "ClientHitRejected_Implementation(FGameplayAbilitySpecHandle())",
    "if (Damage <= 0.0f)",
    "UWorld* World = GetWorld()",
    "if (!World)",
    "ApplyDamageToTargetWithCue",
    "GameplayCue.DBA.Skill.Impact",
    "ClientHitConfirmedWithCritical_Implementation",
    "PASS: RPC handler server attack execution contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    "#include ""GameDBA/Combat/DBADamageCalculator.h""",
    "AActor* Attacker = GetOwner()",
    "AActor* AttackTarget = FindAttackTarget()",
    "ClientHitRejected_Implementation(FGameplayAbilitySpecHandle())",
    "float Damage = CalculateAttackDamage(AttackTarget, bIsCritical)",
    "if (Damage <= 0.0f)",
    "UWorld* World = GetWorld()",
    "if (!World)",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "EDBAElement::None",
    "FGameplayTag::RequestGameplayTag(FName(TEXT(""GameplayCue.DBA.Skill.Impact"")), false)",
    "ClientHitConfirmedWithCritical_Implementation"
)

Assert-FileContains "scripts\test-rpc-handler-wrapper-validation.ps1" @(
    "ServerTryActivateAbility_Validate(Params)",
    "ServerCancelAbility_Validate(Handle)",
    "ServerLockTarget_Validate(TargetActor)",
    "ServerMoveTo_Validate(Location)",
    "ServerRequestAttack_Validate()",
    "ServerUltimateAbility_Validate(Params)",
    "PASS: RPC handler wrapper validation contract"
)

Assert-FileContains "scripts\test-rpc-handler-ability-cooldown-validation.ps1" @(
    "IsInputAbilityOnCooldown",
    "ValidateAbilityCooldown",
    "FindAbilitySpecFromHandle\(Params\.AbilityHandle\)",
    "Spec->InputID",
    "ServerTryActivateAbility_Implementation",
    "ServerTryActivateAbility_Validate",
    "ServerUltimateAbility_Implementation",
    "ServerUltimateAbility_Validate",
    "PASS: RPC handler ability cooldown validation contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    "GameDBA/GAS/DBAAbilitySystemComponent.h",
    "ValidateAbilityCooldown(Params)",
    "bool ADBARpcHandler::ValidateAbilityCooldown(const FDBAAbilityRpcParams& Params) const",
    "Cast<UDBAAbilitySystemComponent>(CharacterRef->GetAbilitySystemComponent())",
    "FindAbilitySpecFromHandle(Params.AbilityHandle)",
    "DBAAbilitySystem->IsInputAbilityOnCooldown(Spec->InputID)"
)

Assert-FileContains "scripts\test-rpc-ability-input-semantic-boundary.ps1" @(
    "ValidateAbilityInputSemantics",
    "bRequireUltimate",
    "EDBAAbilityInputID::Ultimate",
    "ServerTryActivateAbility_Implementation",
    "ServerUltimateAbility_Implementation",
    "RpcHandler->ServerUltimateAbility(Params);",
    "RpcHandler->ServerTryActivateAbility(Params);",
    "PASS: RPC ability input semantic boundary"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp" @(
    "ValidateAbilityInputSemantics(Params, false)",
    "ValidateAbilityInputSemantics(Params, true)",
    "bool ADBARpcHandler::ValidateAbilityInputSemantics(const FDBAAbilityRpcParams& Params, bool bRequireUltimate) const",
    "FindAbilitySpecFromHandle(Params.AbilityHandle)",
    "Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Ultimate)",
    "bRequireUltimate != bIsUltimateInput",
    "Spec->InputID == static_cast<int32>(EDBAAbilityInputID::None)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAClientPredictionComponent.cpp" @(
    "AbilityInputID == static_cast<int32>(EDBAAbilityInputID::Ultimate)",
    "RpcHandler->ServerUltimateAbility(Params);",
    "RpcHandler->ServerTryActivateAbility(Params);"
)

Assert-FileContains "scripts\test-zodiac-character-gas-skill-feedback-hud-announcement.ps1" @(
    "HandleArenaHUDSkillCueExecuted",
    "OnSkillCueExecuted",
    "ShowArenaHUDCombatAnnouncement",
    "ArenaHUDSkillCueAnnouncementDuration",
    '$successMessage'
)

Assert-FileContains "scripts\test-skill-vfx-damage-authority-boundary.ps1" @(
    "DBAZodiacSkillVFXComponent_Generic.cpp",
    "ApplySkillDamage",
    "ApplyAOEDamage",
    "!OwnerActor->HasAuthority()",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "PASS: Skill VFX damage authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\VFX\Components\Skill\DBAZodiacSkillVFXComponent_Generic.cpp" @(
    "bool UDBAZodiacSkillVFXComponent_Generic::ApplySkillDamage",
    "int32 UDBAZodiacSkillVFXComponent_Generic::ApplyAOEDamage",
    "!OwnerActor->HasAuthority()",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue("
)

Assert-FileContains "scripts\test-skill-projectile-damage-authority-boundary.ps1" @(
    "DBASkillProjectileBase.cpp",
    "OnProjectileHit",
    "if (HasAuthority() && HitActor && HitActor != ProjectileOwner && Damage > 0.0f)",
    "UDBADamageCalculator::CalculateFinalDamage(",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "PASS: Skill projectile damage authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBASkillProjectileBase.cpp" @(
    "void ADBASkillProjectileBase::OnProjectileHit",
    "if (HasAuthority() && HitActor && HitActor != ProjectileOwner && Damage > 0.0f)",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "OnProjectileHitResolved(HitActor, HitLocation)"
)

Assert-FileContains "scripts\test-skill-projectile-cpp-hit-boundary.ps1" @(
    "Skill projectile C++ hit boundary",
    "DBASkillProjectileBase.h",
    "DBASkillProjectileBase.cpp",
    "BlueprintImplementableEvent",
    "BlueprintNativeEvent",
    "BP_OnProjectileHit",
    "OnProjectileHitResolved(HitActor, HitLocation);",
    "PASS: Skill projectile C++ hit boundary"
)

Assert-FileContains "scripts\test-skill-projectile-hit-entrypoint-cpp-only.ps1" @(
    "Skill projectile hit entrypoint C++ only",
    "DBASkillProjectileBase.h",
    "DBASkillProjectileBase.cpp",
    "OnProjectileHit must not be BlueprintCallable",
    "HandleProjectile(Hit|Overlap)",
    "PASS: Skill projectile hit entrypoint C++ only"
)

Assert-FileContains "scripts\test-skill-projectile-runtime-entrypoints-cpp-only.ps1" @(
    "Skill projectile runtime entrypoints C++ only",
    "DBASkillProjectileBase.h",
    "Assert-EntrypointIsCppOnly",
    "InitializeProjectile",
    "LaunchProjectile",
    "must not be BlueprintCallable",
    "SetProjectileProperties",
    "PASS: Skill projectile runtime entrypoints C++ only"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h" @(
    "virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation);",
    "virtual void InitializeProjectile(",
    "void SetProjectileProperties(float InSpeed, float InRadius, float InDamage);",
    "void LaunchProjectile(const FVector& Direction);",
    "virtual void OnProjectileHitResolved(AActor* HitActor, FVector HitLocation);"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h" @(
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`r`n`	virtual void OnProjectileHit",
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`n`	virtual void OnProjectileHit",
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`r`n`	virtual void InitializeProjectile",
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`n`	virtual void InitializeProjectile",
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`r`n`	void LaunchProjectile",
    "UFUNCTION(BlueprintCallable, Category = ""DBA|Projectile"")`n`	void LaunchProjectile"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBASkillProjectileBase.cpp" @(
    "OnProjectileHitResolved(HitActor, HitLocation);",
    "void ADBASkillProjectileBase::OnProjectileHitResolved(AActor* HitActor, FVector HitLocation)"
)

Assert-FileDoesNotContain "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h" @(
    "BlueprintImplementableEvent",
    "BlueprintNativeEvent",
    "BP_OnProjectileHit"
)

Assert-FileContains "scripts\test-chain-lightning-damage-authority-boundary.ps1" @(
    "DBAChainLightningSpell.cpp",
    "CastChainLightning",
    "ApplyChainDamage",
    "if (!HasAuthority() && GetNetMode() != NM_Standalone)",
    "if (!HasAuthority())",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "PASS: Chain Lightning damage authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAChainLightningSpell.cpp" @(
    "void ADBAChainLightningSpell::CastChainLightning",
    "if (!HasAuthority() && GetNetMode() != NM_Standalone)",
    "void ADBAChainLightningSpell::ApplyChainDamage",
    "if (!HasAuthority())",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue("
)

Assert-FileContains "scripts\test-damage-calculator-authority-boundary.ps1" @(
    "DBADamageCalculator.cpp",
    "ApplyDamageToTarget",
    "ApplyDamageToTargetWithCue",
    "if (!Attacker->HasAuthority())",
    "BattleAttrSet->SetCurrentHealth",
    "Target->TakeDamage",
    "ExecuteDamageGameplayCue",
    "PASS: DamageCalculator authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp" @(
    "void UDBADamageCalculator::ApplyDamageToTarget",
    "void UDBADamageCalculator::ApplyDamageToTargetWithCue",
    "if (!Attacker->HasAuthority())",
    "BattleAttrSet->SetCurrentHealth",
    "Target->TakeDamage",
    "ExecuteDamageGameplayCue"
)

Assert-FileContains "scripts\test-healing-shield-authority-boundary.ps1" @(
    "DBABloomHealingSpell.cpp",
    "DBAHolyShieldSpell.cpp",
    "ApplyHealing",
    "ApplyShield",
    "ReleaseShield",
    "if (!HasAuthority())",
    "AttrSet->SetCurrentHealth",
    "AttrSet->SetCurrentShield",
    "PASS: Healing and shield authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBABloomHealingSpell.cpp" @(
    "void ADBABloomHealingSpell::CastBloomHealing",
    "if (!HasAuthority() && GetNetMode() != NM_Standalone)",
    "void ADBABloomHealingSpell::ApplyHealing",
    "if (!HasAuthority())",
    "AttrSet->SetCurrentHealth"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAHolyShieldSpell.cpp" @(
    "void ADBAHolyShieldSpell::CastHolyShield",
    "if (!HasAuthority() && GetNetMode() != NM_Standalone)",
    "void ADBAHolyShieldSpell::ApplyShield",
    "void ADBAHolyShieldSpell::ReleaseShield",
    "if (!HasAuthority())",
    "AttrSet->SetCurrentShield",
    "AttrSet->SetMaxShield"
)

Assert-FileContains "scripts\test-ability-system-state-authority-boundary.ps1" @(
    "DBAAbilitySystemComponent.cpp",
    "AddUltimateEnergy",
    "ConsumeUltimateEnergy",
    "AddChainLevel",
    "ResetChainLevel",
    "SetResonanceLevel",
    "GetOwnerRole()",
    "ROLE_Authority",
    "PASS: AbilitySystem state authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp" @(
    "void UDBAAbilitySystemComponent::AddUltimateEnergy",
    "bool UDBAAbilitySystemComponent::ConsumeUltimateEnergy",
    "void UDBAAbilitySystemComponent::AddChainLevel",
    "void UDBAAbilitySystemComponent::ResetChainLevel",
    "void UDBAAbilitySystemComponent::SetResonanceLevel",
    "GetOwnerRole() != ROLE_Authority",
    "UltimateEnergy = FMath::Clamp",
    "ChainLevel = FMath::Clamp",
    "ResonanceLevel = FMath::Clamp"
)

Assert-FileContains "scripts\test-zodiac-character-fallback-state-authority-boundary.ps1" @(
    "DBAZodiacCharacterBase.cpp",
    "SetUltimateEnergy",
    "AddUltimateEnergy",
    "AddChainLevel",
    "ResetChainLevel",
    "UpdateSkillCooldowns",
    "if (!HasAuthority())",
    "PASS: Zodiac character fallback state authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp" @(
    "void ADBAZodiacCharacterBase::SetUltimateEnergy",
    "void ADBAZodiacCharacterBase::AddUltimateEnergy",
    "void ADBAZodiacCharacterBase::AddChainLevel",
    "void ADBAZodiacCharacterBase::ResetChainLevel",
    "void ADBAZodiacCharacterBase::UpdateSkillCooldowns",
    "if (!HasAuthority())",
    "GetDBAAbilitySystemComponent()",
    "ASC->AddUltimateEnergy(Delta)",
    "ASC->AddChainLevel(Delta)",
    "ASC->ResetChainLevel()",
    "return GetUltimateEnergy() >= DBAConstants::MaxUltimateEnergy",
    "OutData.UltimateEnergy = GetUltimateEnergy()",
    "SkillCooldowns = NewCooldowns"
)

Assert-FileContains "scripts\test-zodiac-character-gas-state-single-source.ps1" @(
    "UDBAAbilitySystemComponent",
    "GetDBAAbilitySystemComponent()",
    "ASC->AddUltimateEnergy(Delta)",
    "ASC->AddChainLevel(Delta)",
    "ASC->ResetChainLevel()",
    "SetUltimateEnergy must not write Character UltimateEnergy directly.",
    "AddUltimateEnergy must not write Character UltimateEnergy directly.",
    "AddChainLevel must not write Character ChainLevel directly.",
    "ResetChainLevel must not write Character ChainLevel directly."
)

Assert-FileContains "scripts\test-zodiac-character-death-team-authority-boundary.ps1" @(
    "DBAZodiacCharacterBase.cpp",
    "OnDeath",
    "OnRevive",
    "SetTeamID",
    "if (!HasAuthority())",
    "TeamID = FMath::Max",
    "PASS: Zodiac character death/team authority boundary contract"
)

Assert-FileContains "scripts\test-zodiac-character-death-finalize-timer-boundary.ps1" @(
    "DeathStateFinalizeTimerHandle",
    "ClearTimer(DeathStateFinalizeTimerHandle)",
    "SetTimerForNextTick",
    "FTimerDelegate::CreateWeakLambda",
    "DeathState != EDADeathState::Dying",
    "PASS: Zodiac character death finalize timer boundary contract"
)

Assert-FileContains "scripts\test-zodiac-character-death-idempotent-boundary.ps1" @(
    "if (IsDead())",
    "DeathState = EDADeathState::Dying",
    "PlayDeathAnimation",
    "DeathStateFinalizeTimerHandle =",
    "PASS: Zodiac character death idempotent boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp" @(
    "void ADBAZodiacCharacterBase::OnDeath",
    "void ADBAZodiacCharacterBase::OnRevive",
    "void ADBAZodiacCharacterBase::SetTeamID",
    "DeathState = EDADeathState::Dying",
    "DeathState = EDADeathState::Dead",
    "DeathState = EDADeathState::Alive",
    "if (IsDead())",
    "TeamID = FMath::Max(0, NewTeamID)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h" @(
    "FTimerHandle DeathStateFinalizeTimerHandle"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp" @(
    "ClearTimer(DeathStateFinalizeTimerHandle)",
    "DeathStateFinalizeTimerHandle = World->GetTimerManager().SetTimerForNextTick",
    "FTimerDelegate::CreateWeakLambda",
    "DeathState != EDADeathState::Dying"
)

Assert-FileContains "scripts\test-player-state-match-stats-authority-boundary.ps1" @(
    "DBAPlayerState.cpp",
    "RecordKill",
    "RecordDeath",
    "RecordAssist",
    "AddMatchScore",
    "AddMatchExpDelta",
    "SetMatchResult",
    "SetMatchTeamId",
    "if (!HasAuthority())",
    "PASS: PlayerState match stats authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp" @(
    "void ADBAPlayerState::RecordKill",
    "void ADBAPlayerState::RecordDeath",
    "void ADBAPlayerState::RecordAssist",
    "void ADBAPlayerState::AddMatchScore",
    "void ADBAPlayerState::AddMatchExpDelta",
    "void ADBAPlayerState::SetMatchResult",
    "void ADBAPlayerState::SetMatchTeamId",
    "if (!HasAuthority())",
    "MatchKills +=",
    "MatchDeaths +=",
    "MatchAssists +=",
    "MatchScore =",
    "MatchExpDelta =",
    "MatchResult =",
    "MatchTeamId ="
)

Assert-FileContains "scripts\test-monster-ai-state-authority-boundary.ps1" @(
    "DBAMonsterAIComponent.cpp",
    "TransitionTo",
    "FindTarget",
    "ClearTarget",
    "AttackTarget",
    "AddAggro",
    "RemoveAggro",
    "ClearAggroList",
    "UpdateAggroList",
    "RefreshAggroTarget",
    "HasAuthority()",
    "PASS: Monster AI state authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\Monster\AI\DBAMonsterAIComponent.cpp" @(
    "void UDBAMonsterAIComponent::TransitionTo",
    "void UDBAMonsterAIComponent::FindTarget",
    "void UDBAMonsterAIComponent::ClearTarget",
    "void UDBAMonsterAIComponent::AttackTarget",
    "void UDBAMonsterAIComponent::AddAggro",
    "void UDBAMonsterAIComponent::RemoveAggro",
    "void UDBAMonsterAIComponent::ClearAggroList",
    "void UDBAMonsterAIComponent::UpdateAggroList",
    "void UDBAMonsterAIComponent::RefreshAggroTarget",
    "Owner->HasAuthority()",
    "CurrentState = NewState",
    "CurrentTarget = BestTarget",
    "CurrentTarget = nullptr",
    "Info.AddThreat",
    "AggroList.Add",
    "AggroList.RemoveAll",
    "AggroList.Empty"
)

Assert-FileContains "scripts\test-monster-ai-movement-authority-boundary.ps1" @(
    "DBAMonsterAIComponent.cpp",
    "MoveToLocation",
    "MoveToActor",
    "StopMovement",
    "GetNextPatrolPoint",
    "SetSpawnLocation",
    "HasAuthority()",
    "PASS: Monster AI movement authority boundary contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\Monster\AI\DBAMonsterAIComponent.h" @(
    "void MoveToLocation",
    "void MoveToActor",
    "void StopMovement",
    "FVector GetNextPatrolPoint",
    "void SetSpawnLocation(FVector Location);"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\Monster\AI\DBAMonsterAIComponent.cpp" @(
    "void UDBAMonsterAIComponent::MoveToLocation",
    "void UDBAMonsterAIComponent::MoveToActor",
    "void UDBAMonsterAIComponent::StopMovement",
    "FVector UDBAMonsterAIComponent::GetNextPatrolPoint",
    "void UDBAMonsterAIComponent::SetSpawnLocation",
    "Owner->HasAuthority()",
    "AIController->MoveTo",
    "Movement->RequestDirectMove",
    "AIController->StopMovement",
    "Movement->Velocity = FVector::ZeroVector",
    "CurrentPatrolIndex =",
    "SpawnLocation = Location"
)

Assert-FileContains "scripts\test-arena-hud-momentum-sync.ps1" @(
    "UpdateMomentum",
    "FOnMomentumChanged",
    "UpdateArenaHUDMomentum",
    "HandleControllerMomentumUpdated",
    "CachedMomentumLevel",
    "CachedMomentumProgress",
    "NativeConstruct",
    "PASS: Arena HUD Momentum sync contract"
)

Assert-FileContains "scripts\test-arena-hud-status-effects-sync.ps1" @(
    "AddStatusBuff",
    "OnStatusBuffAdded",
    "FDBAArenaStatusEffectEntry",
    "GetActiveStatusBuffs",
    "GetActiveStatusDebuffs",
    "GetActiveStatusCCEffects",
    "HandleControllerStatusBuffAdded",
    "NormalizeStatusWidgetId",
    "CachedActiveBuffs",
    "NormalizedBuffId",
    "BP_OnBuffsCleared",
    "AddArenaHUDBuff",
    "AddStatusDebuff",
    "OnStatusDebuffAdded",
    "HandleControllerStatusDebuffAdded",
    "CachedActiveDebuffs",
    "NormalizedDebuffId",
    "BP_OnDebuffsCleared",
    "AddArenaHUDCCEffect",
    "OnStatusCCEffectAdded",
    "HandleControllerStatusCCEffectAdded",
    "CachedActiveCCEffects",
    "NormalizedCCId",
    "BP_OnCCEffectsCleared",
    "DBAArenaHUDStatusEffectsTests.cpp",
    "StatusEffectsCacheActiveEntries",
    "PASS: Arena HUD status effects sync contract"
)

Assert-FileContains "scripts\test-arena-hud-event-feedback-sync.ps1" @(
    "ShowCombatAnnouncement",
    "FDBAArenaCombatAnnouncementEntry",
    "GetLastCombatAnnouncement",
    "OnCombatAnnouncementShown",
    "HandleControllerCombatAnnouncementShown",
    "LastCombatAnnouncement\.bIsValid",
    "NormalizeHUDFeedbackText",
    "announcementText",
    "ShowCombatAnnouncement\(FText::FromString",
    "HandleControllerCombatAnnouncementShown\(LastCombatAnnouncement\.Text",
    "DBAArenaHUDCombatAnnouncementTests.cpp",
    "CombatAnnouncementCachesLatestEntry",
    "NormalizeHUDWidgetText",
    "BP_OnAnnouncementCleared",
    "CachedAnnouncementText",
    "CachedAnnouncementDuration",
    "bCachedAnnouncementVisible",
    "UpdateCriticalStateHints",
    "FDBAArenaCriticalStateHintState",
    "GetLastCriticalStateHints",
    "LastCriticalStateHints\.bIsValid",
    "HandleControllerCriticalStateHintsChanged\(LastCriticalStateHints\.bLowHP",
    "DBAArenaHUDCriticalStateTests.cpp",
    "CriticalStateCachesLatestState",
    "BP_OnCriticalStateChanged",
    "NativeConstruct",
    "UpdateArenaObjective",
    "FDBAArenaObjectiveState",
    "GetLastArenaObjectiveState",
    "LastArenaObjectiveState\.bIsValid",
    "objectiveText",
    "UpdateArenaObjective\(FText::FromString",
    "HandleControllerArenaObjectiveUpdated\(LastArenaObjectiveState\.ObjectiveText",
    "DBAArenaHUDObjectiveStateTests.cpp",
    "ObjectiveCachesLatestState",
    "NormalizeHUDWidgetText",
    "BP_OnObjectiveUpdated",
    "CachedObjectiveText",
    "CachedObjectiveProgress",
    "bCachedObjectiveCompleted",
    "CompleteArenaObjective",
    "PASS: Arena HUD event feedback sync contract"
)

Assert-FileContains "scripts\test-arena-hud-event-feed-widget-sync.ps1" @(
    "UDBAArenaEventFeedWidgetBase",
    "AddEventFeedEntry",
    "FDBAArenaEventFeedEntry",
    "GetLastEventFeedEntry",
    "OnEventFeedEntryAdded",
    "HandleControllerEventFeedEntryAdded",
    "LastEventFeedEntry\.bIsValid",
    "NormalizeHUDFeedbackText",
    "FText::FromString",
    "cachedEventText",
    "HandleControllerEventFeedEntryAdded\(LastEventFeedEntry\.Text",
    "DBAArenaHUDEventFeedTests.cpp",
    "EventFeedCachesLatestEntry",
    "NormalizeHUDWidgetText",
    "AddArenaHUDEventFeedEntry",
    "HandleArenaHUDSkillCueExecuted",
    "PASS: Arena HUD event feed widget sync contract"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetController.h" @(
    "SetOwningPlayerController",
    "GetOwningPlayerController",
    "TWeakObjectPtr<class APlayerController>",
    "OwningPlayerController",
    "SetVitals",
    "SetCurrentLevel",
    "CurrentHP",
    "MaxHP",
    "CurrentEnergy",
    "MaxEnergy",
    "CurrentLevel"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetController.cpp" @(
    "SetOwningPlayerController",
    "GetOwningPlayerController",
    "OwningPlayerController = InPlayerController",
    "SetVitals",
    "FMath::Max",
    "OnHPUpdated.Broadcast",
    "OnEnergyUpdated.Broadcast",
    "SetCurrentLevel",
    "OnLevelUpdated.Broadcast",
    "return CurrentHP",
    "return CurrentEnergy",
    "return CurrentLevel"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.h" @(
    "HandleControllerHPUpdated",
    "HandleControllerEnergyUpdated",
    "HandleControllerLevelUpdated"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAPlayerUnitFrameWidgetBase.cpp" @(
    "GameCore/Types/DBACommonEnums.h",
    "UDBAPlayerUnitFrameWidgetController.h",
    "OnHPUpdated.RemoveDynamic",
    "OnEnergyUpdated.RemoveDynamic",
    "OnLevelUpdated.RemoveDynamic",
    "OnHPUpdated.AddDynamic",
    "OnEnergyUpdated.AddDynamic",
    "OnLevelUpdated.AddDynamic",
    "GetCurrentHP()",
    "GetCurrentEnergy()",
    "GetCurrentLevel()",
    "HandleControllerHPUpdated",
    "UpdateHP(CurrentHP, MaxHP)",
    "UpdateEnergy(CurrentEnergy, MaxEnergy)",
    "CachedCurrentHP = FMath::Max(0.0f, InCachedCurrentHP)",
    "CachedCurrentEnergy = FMath::Max(0.0f, InCachedCurrentEnergy)",
    "CachedCurrentXP = FMath::Max(0.0f, InCachedCurrentXP)",
    "CurrentLevel = FMath::Max(1, Level)",
    "NormalizedFiveCamp = FMath::Clamp",
    "static_cast<uint8>(EDBAFiveCamp::None)",
    "static_cast<uint8>(EDBAFiveCamp::Center)",
    "BP_OnApplyFiveCampTheme(NormalizedFiveCamp)",
    "UpdateLevel(Level)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h" @(
    "UDBAPlayerUnitFrameWidgetController",
    "InitializeController(APlayerController* InPlayerController) override",
    "GetPlayerUnitFrameWidgetController",
    "SetPlayerUnitFrameWidgetController",
    "UpdatePlayerLevel",
    "CurrentLevel",
    "PlayerUnitFrameWidgetController"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp" @(
    "UDBAPlayerUnitFrameWidgetController.h",
    "InitializeController(APlayerController* InPlayerController)",
    "Super::InitializeController(InPlayerController)",
    "PlayerUnitFrameWidgetController->SetOwningPlayerController(GetPlayerController())",
    "NewObject<UDBAPlayerUnitFrameWidgetController>(this)",
    "SetPlayerUnitFrameWidgetController",
    "PlayerUnitFrameWidgetController->InitializeController()",
    "PlayerUnitFrameWidgetController->SetVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy)",
    "PlayerUnitFrameWidgetController->SetCurrentLevel(CurrentLevel)",
    "UpdatePlayerLevel"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h" @(
    "UDBAPlayerUnitFrameWidgetController",
    "SetPlayerUnitFrameWidgetController",
    "PlayerUnitFrameWidgetController"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp" @(
    "GameCore/Types/DBACommonEnums.h",
    "UDBAArenaHUDWidgetController.h",
    "UDBAPlayerUnitFrameWidgetBase.h",
    "UDBAPlayerUnitFrameWidgetController.h",
    "WidgetController ? WidgetController->GetPlayerUnitFrameWidgetController() : nullptr",
    "SetPlayerUnitFrameWidgetController",
    "PlayerUnitFrame->SetWidgetController(PlayerUnitFrameWidgetController)",
    "NormalizedFiveCamp = FMath::Clamp",
    "static_cast<uint8>(EDBAFiveCamp::None)",
    "static_cast<uint8>(EDBAFiveCamp::Center)",
    "BP_OnApplyFiveCampTheme(NormalizedFiveCamp)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h" @(
    "UDBAArenaHUDWidgetController",
    "ArenaHUDWidgetController",
    "UpdateArenaHUDPlayerVitals",
    "UpdateArenaHUDPlayerLevel",
    "EnsureArenaHUDWidgetController"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp" @(
    "UDBAArenaHUDWidgetController.h",
    "IsWorldSafeForWidgetCreation(World)",
    "HideArenaHUD",
    "OnSubsystemInitialize",
    "TryShowSplashVideo",
    "ShowMainLobby",
    "HideMainLobby",
    "HideLoginFlowWidget",
    "HideSplashVideo",
    "HandleLoginFlowStateChanged",
    "RefreshLoginFlowWidgetVisibility",
    "ScheduleFlowWidgetRefreshRetry",
    "ScheduleLobbyHUDRefreshRetry",
    "HandleFlowWidgetRefreshRetry",
    "IsLobbyGameplayWorldForUIManager(World)",
    "UpdateInteractionProgress",
    "HideInteractionPrompt",
    "ShowLobbyPlayerHUD",
    "CreateLobbyPlayerHUDWidget",
    "CreateWidget<UDBALobbyPlayerHUDWidgetBase>(PC, LobbyPlayerHUDWidgetClass)",
    "CreateMainLobbyWidget",
    "CreateGameSettingsWidget",
    "CreateInventoryWidget",
    "CreatePartyPanelWidget",
    "CreateInvitePanelWidget",
    "CreateQueueModeSelectWidget",
    "CreateQueueStatusWidget",
    "CreateReadyCheckWidget",
    "CreateMatchFoundWidget",
    "CreatePortalConfirmWidget",
    "CreateInteractionPromptWidget",
    "CreateNewbieVillageMainWidget",
    "CreateNewbieTaskTrackerWidget",
    "EnsureFlowWidgetCreated",
    "SetFlowWidgetVisible",
    "ShowSplashVideo",
    "EnsureLoginFlowBackgroundMusic",
    "GetArenaHUDLocalPlayerController",
    "NewObject<UDBAArenaHUDWidgetController>(this)",
    "EnsureArenaHUDWidgetController(PC)",
    "return nullptr",
    "IsServerLikeRuntime(World)",
    "World->GetTimerManager().SetTimer(SplashVideoTimerHandle",
    "World->GetTimerManager().ClearTimer(SplashVideoTimerHandle)",
    "World->GetTimerManager().SetTimer(",
    "World->GetTimerManager().IsTimerActive(FlowWidgetRefreshRetryTimerHandle)",
    "World->GetTimerManager().IsTimerActive(LobbyHUDRefreshRetryTimerHandle)",
    "RefreshLoginFlowWidgetVisibility();",
    "CachedLoginFlowState = NewState",
    "IsLobbyGameplayWorldForUIManager(World)",
    "TransitionTo(EDBAUIState::Lobby);",
    "ArenaHUDWidgetController->InitializeController(InPlayerController)",
    "ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController)",
    "UpdateArenaHUDPlayerVitals",
    "Controller->UpdatePlayerHP(CurrentHP, MaxHP)",
    "Controller->UpdatePlayerEnergy(CurrentEnergy, MaxEnergy)",
    "UpdateArenaHUDPlayerLevel",
    "Controller->UpdatePlayerLevel(Level)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp" @(
    "CurrentHP = FMath::Max(0.0f, InCurrentHP)",
    "MaxHP = FMath::Max(0.0f, InMaxHP)",
    "CurrentEnergy = FMath::Max(0.0f, InCurrentEnergy)",
    "MaxEnergy = FMath::Max(0.0f, InMaxEnergy)",
    "MaxUltimateEnergy = FMath::Max(1.0f, InMaxEnergy)",
    "CurrentUltimateEnergy = FMath::Clamp(InCurrentEnergy, 0.0f, MaxUltimateEnergy)",
    "OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP)",
    "OnPlayerEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h" @(
    "GetHeroLevel",
    "SyncArenaHUDFromAttributes",
    "bHasSyncedArenaHUDAttributes",
    "LastSyncedArenaHUDCurrentHP",
    "LastSyncedArenaHUDCurrentEnergy",
    "LastSyncedArenaHUDHeroLevel"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp" @(
    "DBAHeroGrowthAttributeSet.h",
    "DBAGameUIManager.h",
    "SyncArenaHUDFromAttributes(true)",
    "if (!IsLocallyControlled())",
    "GameInstance->GetSubsystem<UDBAGameUIManager>()",
    "const float CurrentHP = GetCurrentHealth()",
    "const float CurrentEnergy = GetCurrentEnergy()",
    "const int32 HeroLevel = GetHeroLevel()",
    "const bool bVitalsChanged",
    "const bool bLevelChanged",
    "UpdateArenaHUDPlayerVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy)",
    "UpdateArenaHUDPlayerLevel(HeroLevel)",
    "bHasSyncedArenaHUDAttributes = true",
    "GetNumericAttributeBase(UDBAHeroGrowthAttributeSet::GetHeroLevelAttribute())"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendSessionService.cpp" @(
    "TEXT(""characterBuildSummary"")",
    "TEXT(""DBAZodiac"")",
    "TEXT(""DBAElement"")",
    "TEXT(""DBAFiveCamp"")",
    "TEXT(""DBAFixedSkillGroupId"")",
    "Connection.Zodiac",
    "Connection.PrimaryElement",
    "Connection.FiveCamp",
    "Connection.FixedSkillGroupId",
    "TryBuildTravelUrlFromConnectionData",
    "TryGetObjectField(TEXT(""data"")",
    "NestedBuildSummaryObj",
    "TEXT(""serverIp"")",
    "TEXT(""serverPort"")",
    "TEXT(""sessionToken"")"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendSessionServiceTests.cpp" @(
    "FDBA_GameBackendSessionTravelUrlBuildSummaryTest",
    "FDBA_GameBackendSessionConnectionJsonBuildSummaryTest",
    "FDBA_GameBackendSessionConnectionAliasJsonTest",
    "FDBA_GameBackendSessionEnvelopeJsonTest",
    "BuildTravelUrlIncludesFrozenBuildSummary",
    "ConnectionJsonBuildsTravelUrlWithNestedBuildSummary",
    "ConnectionJsonAcceptsNestedServerAliases",
    "ConnectionJsonAcceptsResponseEnvelopeData",
    "DBAZodiac=Rat",
    "DBAElement=Water",
    "DBAFiveCamp=East",
    "DBAFixedSkillGroupId=Rat_Water",
    "DBAZodiac=Tiger",
    "DBAElement=Fire",
    "DBAFiveCamp=South",
    "DBAFixedSkillGroupId=Tiger_Fire",
    "PlayerSessionToken=alias-token",
    "DBAFixedSkillGroupId=Dragon_Wood",
    "PlayerSessionToken=envelope-token",
    "DBAZodiac=Snake",
    "DBAElement=Gold",
    "DBAFixedSkillGroupId=Snake_Gold"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendTypes.h" @(
    "FDBA_GameBackendMatchHistoryEntry",
    "FDBA_GameBackendMatchHistoryPage",
    "FString ResultJson",
    "FString WinnerTeam",
    "int64 ExpDelta",
    "TMap<FString, int64> Rewards",
    "FString PlayedAtUtc",
    "TArray<FDBA_GameBackendMatchHistoryEntry> Matches"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendPlayerService.h" @(
    "TryParseMatchHistoryData",
    "FDBA_GameBackendMatchHistoryPage& OutPage",
    "FString& OutError"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendPlayerService.cpp" @(
    "TryParseMatchHistoryData",
    "ResolvePayloadObject",
    'Root->TryGetObjectField(TEXT("data"), DataObj)',
    'Payload->TryGetArrayField(TEXT("matches"), Matches)',
    'Entry.ResultJson = ReadStringField(MatchObj, TEXT("resultJson"))',
    'Entry.WinnerTeam = ReadStringField(MatchObj, TEXT("winnerTeam"))',
    'Entry.ExpDelta = ReadInt64Field(MatchObj, TEXT("expDelta"))',
    "ReadNumericRewards(MatchObj, Entry.Rewards)",
    "OutPage.Matches.Add(MoveTemp(Entry))"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendPlayerServiceTests.cpp" @(
    "FDBA_GameBackendPlayerMatchHistoryJsonTest",
    "MatchHistoryJsonParsesSettlementOutcome",
    "TryParseMatchHistoryData(Json, Page, Error)",
    '"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"player-history-test\"}"',
    '"winnerTeam": "blue"',
    '"expDelta": 900',
    '"rewards"',
    'Match.ExpDelta',
    'Match.Rewards.FindRef(TEXT("coin"))',
    'static_cast<int64>(9)',
    'Match.Rewards.FindRef(TEXT("honor"))',
    'static_cast<int64>(3)'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.h" @(
    "FDBALobbyRecentMatchSummary",
    "bool bHasMatch",
    "FString WinnerTeam",
    "int32 Kills",
    "int32 Deaths",
    "int32 Assists",
    "int32 DurationSeconds",
    "FString PlayedAtUtc",
    "int64 ExpDelta",
    "int64 CoinReward",
    "int64 HonorReward",
    "FString RewardSummary",
    "FString CombatSummary",
    "RefreshMatchHistory",
    "UpdateMatchHistoryFromJson",
    "GetRecentMatchSummary",
    "OnMatchHistoryUpdated",
    "OnRecentMatchSummaryUpdated",
    "HandleGetMatchHistoryResponse"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp" @(
    "void UDBAMainLobbyWidgetController::InitializeBackendLobby()",
    "RefreshMatchHistory();",
    "void UDBAMainLobbyWidgetController::RefreshMatchHistory()",
    "PlayerService->GetMyMatches(Callback)",
    "bool UDBAMainLobbyWidgetController::UpdateMatchHistoryFromJson",
    "UDBA_GameBackendPlayerService::TryParseMatchHistoryData",
    "RecentMatchSummary.bHasMatch = true",
    "RecentMatchSummary.WinnerTeam = Match.WinnerTeam",
    "RecentMatchSummary.ExpDelta = Match.ExpDelta",
    "RecentMatchSummary.Kills = Match.Kills",
    "RecentMatchSummary.Deaths = Match.Deaths",
    "RecentMatchSummary.Assists = Match.Assists",
    "RecentMatchSummary.DurationSeconds = Match.DurationSeconds",
    "RecentMatchSummary.PlayedAtUtc = Match.PlayedAtUtc",
    "RecentMatchSummary.CoinReward = Match.Rewards.FindRef(TEXT(""coin""))",
    "RecentMatchSummary.HonorReward = Match.Rewards.FindRef(TEXT(""honor""))",
    "RecentMatchSummary.RewardSummary = BuildRecentMatchRewardSummary(Match.Rewards)",
    "RecentMatchSummary.CombatSummary = BuildRecentMatchCombatSummary(Match.Kills, Match.Deaths, Match.Assists, Match.DurationSeconds)",
    "OnRecentMatchSummaryUpdated.Broadcast(RecentMatchSummary)",
    "void UDBAMainLobbyWidgetController::HandleGetMatchHistoryResponse",
    "OnMatchHistoryUpdated.Broadcast(DataJson)",
    "void UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView()",
    "TrackTelemetry(TEXT(""match_finished_client_view"")",
    "RefreshPlayerData();",
    "RefreshMatchHistory();"
)

Assert-FileMatches "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp" @(
    'void\s+UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView\(\)\s*\{(?s).*?TrackTelemetry\(TEXT\("match_finished_client_view"\).*?RefreshPlayerData\(\);.*?RefreshMatchHistory\(\);.*?\}'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAMainLobbyMatchHistoryTests.cpp" @(
    "FDBAMainLobbyMatchHistorySummaryTest",
    "MainLobby.MatchHistoryUpdatesRecentSummary",
    "UpdateMatchHistoryFromJson(Json)",
    "GetRecentMatchSummary",
    '"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"lobby-history-test\"}"',
    '"winnerTeam": "blue"',
    '"expDelta": 1200',
    '"coin": 12',
    '"gem": 2',
    '"honor": 5',
    $mainLobbyRecentKillsAssertionText,
    $mainLobbyRecentDeathsAssertionText,
    $mainLobbyRecentAssistsAssertionText,
    $mainLobbyRecentDurationAssertionText,
    $mainLobbyRecentCombatSummaryAssertionText,
    $mainLobbyRecentPlayedAtAssertionText,
    $mainLobbyRecentExpDeltaAssertionText,
    $mainLobbyRecentCoinRewardAssertionText,
    $mainLobbyRecentHonorRewardAssertionText,
    $mainLobbyRecentRewardSummaryAssertionText
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\UDBAMainLobbyWidgetBase.h" @(
    "struct FDBALobbyRecentMatchSummary",
    "BackendRefreshMatchHistory",
    "HandleRecentMatchSummaryUpdated",
    "HandleRefreshMatchHistoryClicked",
    "UpdateRecentMatchSummaryText",
    "RefreshMatchHistoryButton",
    "RecentMatchResultText",
    "RecentMatchMapText",
    "RecentMatchCombatText",
    "RecentMatchPlayedAtText",
    "RecentMatchRewardText"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetBase.cpp" @(
    "void UDBAMainLobbyWidgetBase::BackendRefreshMatchHistory()",
    "WidgetController->RefreshMatchHistory()",
    "void UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked()",
    "BackendRefreshMatchHistory();",
    "RefreshMatchHistoryButton = Cast<UButton>(FindLobbyWidgetByNames",
    "RefreshMatchHistoryButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked)",
    "RefreshMatchHistoryButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked)",
    "HandleRecentMatchSummaryUpdated(WidgetController->GetRecentMatchSummary())",
    "void UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated(const FDBALobbyRecentMatchSummary& Summary)",
    "void UDBAMainLobbyWidgetBase::UpdateRecentMatchSummaryText(const FDBALobbyRecentMatchSummary& Summary)",
    "OnRecentMatchSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated)",
    "OnRecentMatchSummaryUpdated.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated)",
    "RecentMatchResultText",
    "RecentMatchMapText",
    "RecentMatchCombatText",
    "RecentMatchPlayedAtText",
    "RecentMatchRewardText",
    "RecentMatchEmpty",
    "Summary.WinnerTeam",
    "Summary.Score",
    "Summary.CombatSummary",
    "Summary.PlayedAtUtc",
    "Summary.ExpDelta",
    "Summary.CoinReward",
    "Summary.HonorReward",
    "Summary.RewardSummary"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAUrlOptions.cpp" @(
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "OutTeamId",
    "ExtractUrlOption(Options, TEXT(""DBATeamId""))",
    "ExtractUrlOption(Options, TEXT(""TeamId""))",
    "DBACharacterBuild::MakeFixedSkillGroupId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "AdmissionBuildSummary",
    "BackendTeamId <= 0",
    "BuildBackendRuntimeTeamName(BackendTeamId)",
    "ToStableZodiacName",
    "ToStableElementName",
    "ToStableFiveCampName"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAUrlOptionsTests.cpp" @(
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "ValidatesDedicatedServerBuildSummary",
    "DBATeamId=1",
    "TeamId=2",
    "MissingTeamId",
    "NonPositiveTeamId",
    "Rat_Fire"
)

Assert-FileContains "scripts\validate-unreal-baseline-entrypoints.ps1" @(
    "Test-ArenaGameFixedSkillGroupBaseline",
    "HasValidIdentity",
    "HasValidSkillGroupIdentity",
    "BuildFixedSkillGroupRowName",
    "DBACharacterBuild::MakeFixedSkillGroupId",
    "Zodiac_%s_Element_%s",
    "UsesCanonicalBuildSummaryRowName",
    "ValidatesRowIdentity",
    "GeneratorRejectsInvalidIdentityDimensions",
    "GeneratorFallbackUsesCanonicalIdentity"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Data\DBAFixedSkillGroupData.h" @(
    "HasValidIdentity",
    "DBACharacterBuild::MakeFixedSkillGroupId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Data\DBAZodiacHeroDataAsset.h" @(
    "BuildFixedSkillGroupRowName"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Data\DBAZodiacHeroDataAsset.cpp" @(
    "BuildFixedSkillGroupRowName",
    "DBACharacterBuild::MakeFixedSkillGroupId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Services\DBASkillGroupGeneratorSubsystem.cpp" @(
    "DBACharacterBuild::MakeFixedSkillGroupId",
    "HasValidSkillGroupIdentity",
    "FoundRow->HasValidIdentity()"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAFixedSkillGroupDataTests.cpp" @(
    "UsesCanonicalBuildSummaryRowName",
    "ValidatesRowIdentity",
    "GeneratorRejectsInvalidIdentityDimensions",
    "GeneratorFallbackUsesCanonicalIdentity",
    "AssetRows",
    "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    "Rat_Water",
    "Snake_Gold",
    "Tiger_Fire"
)

Write-Host "PASS: production evidence contracts"
