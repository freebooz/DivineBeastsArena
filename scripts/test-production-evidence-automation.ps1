<#
Runs lightweight production evidence automation tests.

This is a local/CI-friendly umbrella check for script contracts, release
fixtures, PowerShell syntax, and workflow YAML parsing. It intentionally does
not launch Unreal, sign packages, upload CDN payloads, or run external services.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

function New-CodePointText {
    param(
        [Parameter(Mandatory = $true)][int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

$directExecutionPolicyContractStepText = New-CodePointText @(0x76F4, 0x63A5, 0x6267, 0x884C, 0x7B56, 0x7565, 0x5951, 0x7EA6)

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action
    )

    $stepLocation = (Get-Location).ProviderPath
    Write-Host ("[production-evidence-tests] {0}" -f $Name) -ForegroundColor Cyan
    & $Action
    $currentLocation = (Get-Location).ProviderPath
    if ($currentLocation -ne $stepLocation) {
        throw "Production evidence test step '$Name' changed the working directory from '$stepLocation' to '$currentLocation'."
    }
}

function Test-PowerShellSyntax {
    param([Parameter(Mandatory = $true)][string[]]$RelativePaths)

    foreach ($relativePath in $RelativePaths) {
        $path = Join-Path $repoRoot $relativePath
        if (-not (Test-Path -LiteralPath $path)) {
            throw "PowerShell syntax target is missing: $relativePath"
        }

        $tokens = $null
        $parseErrors = $null
        [System.Management.Automation.Language.Parser]::ParseFile($path, [ref]$tokens, [ref]$parseErrors) | Out-Null
        if ($parseErrors.Count -gt 0) {
            throw "PowerShell parse errors in ${relativePath}: $($parseErrors.Message -join '; ')"
        }
    }
}

function Test-WorkflowYaml {
    param([Parameter(Mandatory = $true)][string[]]$RelativePaths)

    $python = Get-Command "python" -ErrorAction SilentlyContinue
    if (-not $python) {
        $python = Get-Command "py" -ErrorAction SilentlyContinue
    }
    if (-not $python) {
        throw "python or py is required to parse workflow YAML in this automation test."
    }

    $script = @"
import yaml
from pathlib import Path
for rel in [
$((($RelativePaths | ForEach-Object { "    r'$_'" }) -join ",`n"))
]:
    path = Path(rel)
    data = yaml.safe_load(path.read_text(encoding='utf-8'))
    if not isinstance(data, dict) or not data.get('jobs'):
        raise SystemExit(f'{path}: missing jobs')
    print(f'{path}: {data.get("name")} jobs={",".join(data.get("jobs", {}).keys())}')
"@

    Push-Location $repoRoot
    try {
        $script | & $python.Source -
    }
    finally {
        Pop-Location
    }
}

Invoke-Step "client release prerequisite fixtures" {
    & (Join-Path $repoRoot "scripts\test-client-release-prerequisites.ps1")
}

Invoke-Step "client release runner diagnostic fixtures" {
    & (Join-Path $repoRoot "scripts\test-client-release-runner-diagnostic.ps1")
}

Invoke-Step "release readiness report fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-readiness-report.ps1")
}

Invoke-Step "release blocker action fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-blocker-actions.ps1")
}

Invoke-Step "release blocker action validation fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-blocker-action-validation.ps1")
}

Invoke-Step "release blockers external-only fixtures via validate-release-blockers-external-only.ps1" {
    & (Join-Path $repoRoot "scripts\test-release-blockers-external-only.ps1")
}

Invoke-Step "development continuation readiness fixtures" {
    & (Join-Path $repoRoot "scripts\test-development-continuation-readiness.ps1")
}

Invoke-Step "release input template fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-input-template.ps1")
}

Invoke-Step "release input template validation fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-input-template-validation.ps1")
}

Invoke-Step "release input template resolver fixtures" {
    & (Join-Path $repoRoot "scripts\test-resolve-release-input-template.ps1")
}

Invoke-Step "release input values template fixtures" {
    & (Join-Path $repoRoot "scripts\test-write-release-input-values-template.ps1")
}

Invoke-Step "release input values validation fixtures" {
    & (Join-Path $repoRoot "scripts\test-release-input-values-validation.ps1")
}

Invoke-Step "production preflight release validation wiring" {
    & (Join-Path $repoRoot "scripts\test-production-preflight-release-validation.ps1")
}

Invoke-Step "production evidence collector fixtures" {
    & (Join-Path $repoRoot "scripts\test-production-evidence-collector.ps1")
}

Invoke-Step "Client package URL policy fixtures" {
    & (Join-Path $repoRoot "scripts\test-client-package-url-policy.ps1")
}

Invoke-Step "Client CDN payload URL policy fixtures" {
    & (Join-Path $repoRoot "scripts\test-client-cdn-payload-url-policy.ps1")
}

Invoke-Step "Launcher CDN smoke URL policy fixtures" {
    & (Join-Path $repoRoot "scripts\test-launcher-cdn-smoke-url-policy.ps1")
}

Invoke-Step "Unreal module boundary fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-module-boundaries.ps1")
}

Invoke-Step "Unreal baseline entrypoint fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-baseline-entrypoints.ps1")
}

Invoke-Step "Unreal source guardrail fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-source-guardrails.ps1")
}

Invoke-Step $directExecutionPolicyContractStepText {
    & (Join-Path $repoRoot "scripts\test-agent-direct-execution-policy.ps1")
}

Invoke-Step "Unreal C++ logic / Blueprint boundary contract" {
    & (Join-Path $repoRoot "scripts\test-unreal-cpp-logic-blueprint-boundary.ps1")
}

Invoke-Step "Unreal DataAsset / no-hardcoding policy contract" {
    & (Join-Path $repoRoot "scripts\test-unreal-data-asset-no-hardcoding-policy.ps1")
}

Invoke-Step "Playable skill default catalog DataAsset contract" {
    & (Join-Path $repoRoot "scripts\test-playable-skill-catalog-defaults-data-asset-contract.ps1")
}

Invoke-Step "Zodiac character lobby skill DataAsset boundary contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-lobby-skill-data-asset-boundary.ps1")
}

Invoke-Step "Unreal UI event / async interface policy contract" {
    & (Join-Path $repoRoot "scripts\test-unreal-ui-event-async-policy.ps1")
}

Invoke-Step "Unreal Chinese log output policy contract" {
    & (Join-Path $repoRoot "scripts\test-unreal-chinese-log-output-policy.ps1")
}

Invoke-Step "UE UI runtime Chinese output contract" {
    & (Join-Path $repoRoot "scripts\test-unreal-ui-runtime-chinese-output-contract.ps1")
}

Invoke-Step "GAS ability C++ lifecycle boundary contract" {
    & (Join-Path $repoRoot "scripts\test-gas-ability-cpp-lifecycle-boundary.ps1")
}

Invoke-Step "Runtime player-joined build summary contract" {
    & (Join-Path $repoRoot "scripts\test-runtime-player-join-build-summary-contract.ps1")
}

Invoke-Step "Runtime match lifecycle contract" {
    & (Join-Path $repoRoot "scripts\test-runtime-match-lifecycle-contract.ps1")
}

Invoke-Step "GameBackendClient player match history contract" {
    & (Join-Path $repoRoot "scripts\test-gamebackend-player-match-history-contract.ps1")
}

Invoke-Step "Main Lobby match history contract" {
    & (Join-Path $repoRoot "scripts\test-main-lobby-match-history-contract.ps1")
}

Invoke-Step "internal API route protection contract" {
    & (Join-Path $repoRoot "scripts\test-internal-api-route-protection-contract.ps1")
}

Invoke-Step "player_id claim boundary contract" {
    & (Join-Path $repoRoot "scripts\test-player-id-claim-boundary-contract.ps1")
}

Invoke-Step "Session connection build summary contract" {
    & (Join-Path $repoRoot "scripts\test-session-connection-build-summary-contract.ps1")
}

Invoke-Step "Dedicated Server URL build-summary admission contract" {
    & (Join-Path $repoRoot "scripts\test-dedicated-server-url-build-summary-admission-contract.ps1")
}

Invoke-Step "Unreal Moba foundation fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-moba-foundation.ps1")
}

Invoke-Step "PlayerUnitFrame controller contract" {
    & (Join-Path $repoRoot "scripts\test-player-unit-frame-controller-contract.ps1")
}

Invoke-Step "PlayerUnitFrame widget binding contract" {
    & (Join-Path $repoRoot "scripts\test-player-unit-frame-widget-binding.ps1")
}

Invoke-Step "PlayerUnitFrame UltimateEnergy max sync contract" {
    & (Join-Path $repoRoot "scripts\test-player-unit-frame-ultimate-energy-max-sync.ps1")
}

Invoke-Step "Arena HUD controller PlayerUnitFrame contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-controller-player-unit-frame.ps1")
}

Invoke-Step "Arena HUD root PlayerUnitFrame handoff contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-root-player-unit-frame-handoff.ps1")
}

Invoke-Step "Arena HUD root FiveCamp theme boundary contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-root-five-camp-theme-boundary.ps1")
}

Invoke-Step "Arena HUD AbilityBar character binding contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-ability-bar-character-binding.ps1")
}

Invoke-Step "Arena AbilityBar cooldown slot indexing contract" {
    & (Join-Path $repoRoot "scripts\test-arena-ability-bar-cooldown-slot-indexing.ps1")
}

Invoke-Step "Zodiac character legacy cooldown indexing contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-legacy-cooldown-indexing.ps1")
}

Invoke-Step "Zodiac character ability cooldown query contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-ability-cooldown-query.ps1")
}

Invoke-Step "Arena AbilityBar slot boundary contract" {
    & (Join-Path $repoRoot "scripts\test-arena-ability-bar-slot-boundary-contract.ps1")
}

Invoke-Step "Arena AbilityBar cooldown event sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-ability-bar-cooldown-event-sync.ps1")
}

Invoke-Step "Arena HUD UltimateEnergy sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-ultimate-energy-sync.ps1")
}

Invoke-Step "Arena HUD UltimateEnergy default constants contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-ultimate-energy-default-constants.ps1")
}

Invoke-Step "Arena HUD UltimateReadyPrompt sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-ultimate-ready-prompt-sync.ps1")
}

Invoke-Step "Arena HUD Chain/Resonance sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-chain-resonance-sync.ps1")
}

Invoke-Step "Arena HUD Chain/Resonance constants contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-chain-resonance-constants.ps1")
}

Invoke-Step "Arena HUD Chain/Resonance panel boundary contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-chain-resonance-panel-boundaries.ps1")
}

Invoke-Step "Arena HUD AuraSummaryPanel contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-aura-summary-panel-contract.ps1")
}

Invoke-Step "Arena HUD ConnectionWarning contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-connection-warning-contract.ps1")
}

Invoke-Step "Arena HUD SelfCastBar contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-self-cast-bar-contract.ps1")
}

Invoke-Step "DamageCalculator chain constants contract" {
    & (Join-Path $repoRoot "scripts\test-damage-calculator-chain-constants.ps1")
}

Invoke-Step "DamageCalculator chain tier semantics contract" {
    & (Join-Path $repoRoot "scripts\test-damage-calculator-chain-tier-semantics.ps1")
}

Invoke-Step "DamageCalculator element count constant contract" {
    & (Join-Path $repoRoot "scripts\test-damage-calculator-element-count-constant.ps1")
}

Invoke-Step "DamageCalculator resonance damage constants contract" {
    & (Join-Path $repoRoot "scripts\test-damage-calculator-resonance-damage-constants.ps1")
}

Invoke-Step "Defense reduction constant contract" {
    & (Join-Path $repoRoot "scripts\test-defense-reduction-constant.ps1")
}

Invoke-Step "AbilitySystem resonance constants contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-resonance-constants.ps1")
}

Invoke-Step "AbilitySystem UltimateEnergy passive regen constant contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-ultimate-passive-regen-constant.ps1")
}

Invoke-Step "AbilitySystem cooldown slot constants contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-cooldown-slot-constants.ps1")
}

Invoke-Step "AbilitySystem input activation feedback contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-input-activation-feedback.ps1")
}

Invoke-Step "AbilitySystem input cooldown authority gate contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-input-cooldown-authority-gate.ps1")
}

Invoke-Step "AbilitySystem avatar actor context contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-avatar-actor-context-contract.ps1")
}

Invoke-Step "AbilitySystem target TeamId C++ boundary contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-target-teamid-cpp-boundary.ps1")
}

Invoke-Step "Android touch input bridge server boundary" {
    & (Join-Path $repoRoot "scripts\test-android-touch-input-bridge-server-boundary.ps1")
}

Invoke-Step "Client prediction local runtime boundary" {
    & (Join-Path $repoRoot "scripts\test-client-prediction-local-runtime-boundary.ps1")
}

Invoke-Step "Lobby PlayerController local input binding contract" {
    & (Join-Path $repoRoot "scripts\test-lobby-player-controller-local-input-binding.ps1")
}

Invoke-Step "Lobby PlayerController local skill cast boundary" {
    & (Join-Path $repoRoot "scripts\test-lobby-player-controller-local-skill-cast-boundary.ps1")
}

Invoke-Step "Zodiac character local skill RPC boundary" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-local-skill-rpc-boundary.ps1")
}

Invoke-Step "Zodiac character internal cast authority boundary" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-internal-cast-authority-boundary.ps1")
}

Invoke-Step "RPC handler server character context contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-server-character-context.ps1")
}

Invoke-Step "RPC handler server move execution contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-server-move-execution.ps1")
}

Invoke-Step "RPC handler server lock target execution contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-server-lock-target-execution.ps1")
}

Invoke-Step "RPC handler stale locked target clear contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-stale-locked-target-clear.ps1")
}

Invoke-Step "RPC handler server attack execution contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-server-attack-execution.ps1")
}

Invoke-Step "RPC handler wrapper validation contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-wrapper-validation.ps1")
}

Invoke-Step "RPC handler ability cooldown validation contract" {
    & (Join-Path $repoRoot "scripts\test-rpc-handler-ability-cooldown-validation.ps1")
}

Invoke-Step "RPC ability input semantic boundary" {
    & (Join-Path $repoRoot "scripts\test-rpc-ability-input-semantic-boundary.ps1")
}

Invoke-Step "Zodiac character GAS skill feedback HUD announcement contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-gas-skill-feedback-hud-announcement.ps1")
}

Invoke-Step "Skill VFX damage authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-skill-vfx-damage-authority-boundary.ps1")
}

Invoke-Step "Skill projectile damage authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-skill-projectile-damage-authority-boundary.ps1")
}

Invoke-Step "Skill projectile C++ hit boundary contract" {
    & (Join-Path $repoRoot "scripts\test-skill-projectile-cpp-hit-boundary.ps1")
}

Invoke-Step "Skill projectile hit entrypoint C++ only contract" {
    & (Join-Path $repoRoot "scripts\test-skill-projectile-hit-entrypoint-cpp-only.ps1")
}

Invoke-Step "Skill projectile runtime entrypoints C++ only contract" {
    & (Join-Path $repoRoot "scripts\test-skill-projectile-runtime-entrypoints-cpp-only.ps1")
}

Invoke-Step "Chain Lightning damage authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-chain-lightning-damage-authority-boundary.ps1")
}

Invoke-Step "DamageCalculator authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-damage-calculator-authority-boundary.ps1")
}

Invoke-Step "Healing and shield authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-healing-shield-authority-boundary.ps1")
}

Invoke-Step "AbilitySystem state authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-ability-system-state-authority-boundary.ps1")
}

Invoke-Step "Zodiac character fallback state authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-fallback-state-authority-boundary.ps1")
}

Invoke-Step "Zodiac character death/team authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-death-team-authority-boundary.ps1")
}

Invoke-Step "Zodiac character death finalize timer boundary contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-death-finalize-timer-boundary.ps1")
}

Invoke-Step "Zodiac character death idempotent boundary contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-death-idempotent-boundary.ps1")
}

Invoke-Step "PlayerState match stats authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-player-state-match-stats-authority-boundary.ps1")
}

Invoke-Step "Monster AI state authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-monster-ai-state-authority-boundary.ps1")
}

Invoke-Step "Monster AI movement authority boundary contract" {
    & (Join-Path $repoRoot "scripts\test-monster-ai-movement-authority-boundary.ps1")
}

Invoke-Step "Arena HUD Momentum sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-momentum-sync.ps1")
}

Invoke-Step "Arena HUD status effects sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-status-effects-sync.ps1")
}

Invoke-Step "Arena HUD event feedback sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-event-feedback-sync.ps1")
}

Invoke-Step "Arena HUD event feed widget sync contract" {
    & (Join-Path $repoRoot "scripts\test-arena-hud-event-feed-widget-sync.ps1")
}

Invoke-Step "Game UI manager Arena HUD controller contract" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-arena-hud-controller.ps1")
}

Invoke-Step "Game UI manager Arena HUD runtime updates contract" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-arena-hud-runtime-updates.ps1")
}

Invoke-Step "Game UI manager Arena HUD entrypoint server boundaries" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-arena-hud-entrypoint-server-boundaries.ps1")
}

Invoke-Step "Game UI manager Arena HUD hide server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-arena-hud-hide-server-boundary.ps1")
}

Invoke-Step "Game UI manager MainLobby entrypoint server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-main-lobby-entrypoint-server-boundary.ps1")
}

Invoke-Step "Game UI manager MainLobby hide server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-main-lobby-hide-server-boundary.ps1")
}

Invoke-Step "Game UI manager interaction progress server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-interaction-progress-server-boundary.ps1")
}

Invoke-Step "Game UI manager interaction prompt hide server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-interaction-prompt-hide-server-boundary.ps1")
}

Invoke-Step "Game UI manager Lobby HUD server boundary contract" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-lobby-hud-server-boundary.ps1")
}

Invoke-Step "Game UI manager widget factory server boundaries" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-widget-factory-server-boundaries.ps1")
}

Invoke-Step "Game UI manager retry timer server boundaries" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-retry-timer-server-boundaries.ps1")
}

Invoke-Step "Game UI manager login flow state server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-login-flow-state-server-boundary.ps1")
}

Invoke-Step "Game UI manager login flow start server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-login-flow-start-server-boundary.ps1")
}

Invoke-Step "Game UI manager login flow server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-login-flow-server-boundary.ps1")
}

Invoke-Step "Game UI manager login flow hide server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-login-flow-hide-server-boundary.ps1")
}

Invoke-Step "Game UI manager input mode restore server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-input-mode-restore-server-boundary.ps1")
}

Invoke-Step "Game UI manager splash video timer server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-splash-video-timer-server-boundary.ps1")
}

Invoke-Step "Game UI manager splash video hide server boundary" {
    & (Join-Path $repoRoot "scripts\test-game-ui-manager-splash-video-hide-server-boundary.ps1")
}

Invoke-Step "Zodiac character Arena HUD sync contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-sync.ps1")
}

Invoke-Step "Zodiac character Arena HUD sync cache contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-sync-cache.ps1")
}

Invoke-Step "Zodiac character Arena HUD attribute delegate contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-attribute-delegates.ps1")
}

Invoke-Step "Zodiac character Arena HUD critical state contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-critical-state.ps1")
}

Invoke-Step "Zodiac character Arena HUD UltimateReadyPrompt contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-ultimate-ready-prompt.ps1")
}

Invoke-Step "Zodiac character UltimateEnergy constants contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-ultimate-energy-constants.ps1")
}

Invoke-Step "Zodiac character skill slot count constants contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-skill-slot-count-constants.ps1")
}

Invoke-Step "Zodiac character GAS input activation bridge contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-gas-input-activation-bridge.ps1")
}

Invoke-Step "Zodiac character server cast authority boundary" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-server-cast-authority-boundary.ps1")
}

Invoke-Step "Zodiac character server cast RPC validation boundary" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-server-cast-rpc-validation.ps1")
}

Invoke-Step "Zodiac ultimate energy cost constants contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-ultimate-energy-cost-constants.ps1")
}

Invoke-Step "Zodiac character Arena HUD chain announcement contract" {
    & (Join-Path $repoRoot "scripts\test-zodiac-character-arena-hud-chain-announcement.ps1")
}

Invoke-Step "AI_Showcase automation runner fixtures" {
    & (Join-Path $repoRoot "scripts\test-ai-showcase-automation-runner.ps1")
}

Invoke-Step "AI_Showcase widget tree contract" {
    & (Join-Path $repoRoot "scripts\test-ai-showcase-widget-tree-contract.ps1")
}

Invoke-Step "FixedSkillGroups DataTable diagnostic fixtures" {
    & (Join-Path $repoRoot "scripts\test-fixed-skill-group-datatable-diagnostic.ps1")
}

Invoke-Step "Data table count constants contract" {
    & (Join-Path $repoRoot "scripts\test-data-table-count-constants.ps1")
}

Invoke-Step "FixedSkillGroup asset test constants contract" {
    & (Join-Path $repoRoot "scripts\test-fixed-skill-group-asset-test-constants.ps1")
}

Invoke-Step "FixedSkillGroups source CSV fixtures" {
    & (Join-Path $repoRoot "scripts\test-fixed-skill-group-source-csv.ps1")
}

Invoke-Step "FixedSkillGroups DataTable import fixtures" {
    & (Join-Path $repoRoot "scripts\test-fixed-skill-group-datatable-import.ps1")
}

Invoke-Step "Admin match reward display contract" {
    & (Join-Path $repoRoot "scripts\test-admin-match-reward-display-contract.ps1")
}

Invoke-Step "Admin auth session storage contract" {
    & (Join-Path $repoRoot "scripts\test-admin-auth-session-storage-contract.ps1")
}

Invoke-Step "Admin auth interceptor scope contract" {
    & (Join-Path $repoRoot "scripts\test-admin-auth-interceptor-scope-contract.ps1")
}

Invoke-Step "Admin auth return-url contract" {
    & (Join-Path $repoRoot "scripts\test-admin-auth-return-url-contract.ps1")
}

Invoke-Step "Launcher CSP contract" {
    & (Join-Path $repoRoot "scripts\test-launcher-csp-contract.ps1")
}

Invoke-Step "Launcher manifest URL policy contract" {
    & (Join-Path $repoRoot "scripts\test-launcher-manifest-url-policy-contract.ps1")
}

Invoke-Step "production evidence contract strings" {
    & (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1")
}

Invoke-Step "PowerShell syntax parse" {
    Test-PowerShellSyntax @(
        "scripts\collect-production-evidence.ps1",
        "scripts\write-release-readiness-report.ps1",
        "scripts\diagnose-release-blockers.ps1",
        "scripts\validate-release-blocker-actions.ps1",
        "scripts\write-release-input-template.ps1",
        "scripts\validate-release-input-template.ps1",
        "scripts\prepare-client-release-package.ps1",
        "scripts\collect-client-package-evidence.ps1",
        "scripts\prepare-client-cdn-payload.ps1",
        "scripts\run-client-release-evidence.ps1",
        "scripts\diagnose-client-release-prerequisites.ps1",
        "scripts\diagnose-client-release-runner.ps1",
        "scripts\test-client-package-url-policy.ps1",
        "scripts\test-client-release-prerequisites.ps1",
        "scripts\test-client-release-runner-diagnostic.ps1",
        "scripts\test-production-evidence-collector.ps1",
        "scripts\test-release-readiness-report.ps1",
        "scripts\test-release-blocker-actions.ps1",
        "scripts\test-release-blocker-action-validation.ps1",
        "scripts\test-release-input-template.ps1",
        "scripts\test-release-input-template-validation.ps1",
        "scripts\test-resolve-release-input-template.ps1",
        "scripts\resolve-release-input-template.ps1",
        "scripts\test-write-release-input-values-template.ps1",
        "scripts\write-release-input-values-template.ps1",
        "scripts\test-release-input-values-validation.ps1",
        "scripts\validate-release-input-values.ps1",
        "scripts\test-production-preflight-release-validation.ps1",
        "scripts\test-client-cdn-payload-url-policy.ps1",
        "scripts\test-launcher-cdn-smoke-url-policy.ps1",
        "scripts\test-unreal-module-boundaries.ps1",
        "scripts\test-unreal-baseline-entrypoints.ps1",
        "scripts\test-unreal-source-guardrails.ps1",
        "scripts\test-agent-direct-execution-policy.ps1",
        "scripts\test-unreal-cpp-logic-blueprint-boundary.ps1",
        "scripts\test-unreal-data-asset-no-hardcoding-policy.ps1",
        "scripts\test-unreal-ui-event-async-policy.ps1",
        "scripts\test-unreal-chinese-log-output-policy.ps1",
        "scripts\test-gas-ability-cpp-lifecycle-boundary.ps1",
        "scripts\test-runtime-match-lifecycle-contract.ps1",
        "scripts\test-gamebackend-player-match-history-contract.ps1",
        "scripts\test-main-lobby-match-history-contract.ps1",
        "scripts\validate-internal-api-route-protection.ps1",
        "scripts\test-internal-api-route-protection-contract.ps1",
        "scripts\test-player-id-claim-boundary-contract.ps1",
        "scripts\test-dedicated-server-url-build-summary-admission-contract.ps1",
        "scripts\validate-unreal-moba-foundation.ps1",
        "scripts\test-unreal-moba-foundation.ps1",
        "scripts\test-player-unit-frame-controller-contract.ps1",
        "scripts\test-player-unit-frame-widget-binding.ps1",
        "scripts\test-player-unit-frame-ultimate-energy-max-sync.ps1",
        "scripts\test-arena-hud-controller-player-unit-frame.ps1",
        "scripts\test-arena-hud-root-player-unit-frame-handoff.ps1",
        "scripts\test-arena-hud-ability-bar-character-binding.ps1",
        "scripts\test-arena-ability-bar-cooldown-slot-indexing.ps1",
        "scripts\test-arena-ability-bar-cooldown-event-sync.ps1",
        "scripts\test-arena-hud-ultimate-energy-sync.ps1",
        "scripts\test-arena-hud-ultimate-energy-default-constants.ps1",
        "scripts\test-arena-hud-ultimate-ready-prompt-sync.ps1",
        "scripts\test-arena-hud-chain-resonance-sync.ps1",
        "scripts\test-arena-hud-chain-resonance-constants.ps1",
        "scripts\test-damage-calculator-chain-constants.ps1",
        "scripts\test-damage-calculator-chain-tier-semantics.ps1",
        "scripts\test-damage-calculator-element-count-constant.ps1",
        "scripts\test-damage-calculator-resonance-damage-constants.ps1",
        "scripts\test-defense-reduction-constant.ps1",
        "scripts\test-ability-system-resonance-constants.ps1",
        "scripts\test-ability-system-ultimate-passive-regen-constant.ps1",
        "scripts\test-ability-system-cooldown-slot-constants.ps1",
        "scripts\test-ability-system-input-activation-feedback.ps1",
        "scripts\test-ability-system-input-cooldown-authority-gate.ps1",
        "scripts\test-ability-system-avatar-actor-context-contract.ps1",
        "scripts\test-ability-system-target-teamid-cpp-boundary.ps1",
        "scripts\test-rpc-handler-ability-cooldown-validation.ps1",
        "scripts\test-rpc-ability-input-semantic-boundary.ps1",
        "scripts\test-rpc-handler-server-move-execution.ps1",
        "scripts\test-rpc-handler-server-lock-target-execution.ps1",
        "scripts\test-rpc-handler-stale-locked-target-clear.ps1",
        "scripts\test-rpc-handler-server-attack-execution.ps1",
        "scripts\test-android-touch-input-bridge-server-boundary.ps1",
        "scripts\test-lobby-player-controller-local-input-binding.ps1",
        "scripts\test-lobby-player-controller-local-skill-cast-boundary.ps1",
        "scripts\test-zodiac-character-local-skill-rpc-boundary.ps1",
        "scripts\test-zodiac-character-internal-cast-authority-boundary.ps1",
        "scripts\test-zodiac-character-legacy-cooldown-indexing.ps1",
        "scripts\test-zodiac-character-ability-cooldown-query.ps1",
        "scripts\test-arena-hud-momentum-sync.ps1",
        "scripts\test-arena-hud-status-effects-sync.ps1",
        "scripts\test-arena-hud-event-feedback-sync.ps1",
        "scripts\test-arena-hud-event-feed-widget-sync.ps1",
        "scripts\test-game-ui-manager-arena-hud-controller.ps1",
        "scripts\test-game-ui-manager-arena-hud-runtime-updates.ps1",
        "scripts\test-game-ui-manager-arena-hud-entrypoint-server-boundaries.ps1",
        "scripts\test-game-ui-manager-arena-hud-hide-server-boundary.ps1",
        "scripts\test-game-ui-manager-main-lobby-entrypoint-server-boundary.ps1",
        "scripts\test-game-ui-manager-main-lobby-hide-server-boundary.ps1",
        "scripts\test-game-ui-manager-interaction-progress-server-boundary.ps1",
        "scripts\test-game-ui-manager-interaction-prompt-hide-server-boundary.ps1",
        "scripts\test-game-ui-manager-lobby-hud-server-boundary.ps1",
        "scripts\test-game-ui-manager-widget-factory-server-boundaries.ps1",
        "scripts\test-game-ui-manager-retry-timer-server-boundaries.ps1",
        "scripts\test-game-ui-manager-login-flow-state-server-boundary.ps1",
        "scripts\test-game-ui-manager-login-flow-start-server-boundary.ps1",
        "scripts\test-game-ui-manager-login-flow-server-boundary.ps1",
        "scripts\test-game-ui-manager-login-flow-hide-server-boundary.ps1",
        "scripts\test-game-ui-manager-input-mode-restore-server-boundary.ps1",
        "scripts\test-game-ui-manager-splash-video-timer-server-boundary.ps1",
        "scripts\test-game-ui-manager-splash-video-hide-server-boundary.ps1",
        "scripts\test-zodiac-character-arena-hud-sync.ps1",
        "scripts\test-zodiac-character-arena-hud-sync-cache.ps1",
        "scripts\test-zodiac-character-arena-hud-attribute-delegates.ps1",
        "scripts\test-zodiac-character-arena-hud-critical-state.ps1",
        "scripts\test-zodiac-character-arena-hud-ultimate-ready-prompt.ps1",
        "scripts\test-playable-skill-catalog-defaults-data-asset-contract.ps1",
        "scripts\test-zodiac-character-lobby-skill-data-asset-boundary.ps1",
        "scripts\test-zodiac-character-ultimate-energy-constants.ps1",
        "scripts\test-zodiac-character-skill-slot-count-constants.ps1",
        "scripts\test-zodiac-character-gas-input-activation-bridge.ps1",
        "scripts\test-zodiac-character-server-cast-authority-boundary.ps1",
        "scripts\test-zodiac-character-server-cast-rpc-validation.ps1",
        "scripts\test-zodiac-character-gas-skill-feedback-hud-announcement.ps1",
        "scripts\test-skill-vfx-damage-authority-boundary.ps1",
        "scripts\test-skill-projectile-damage-authority-boundary.ps1",
        "scripts\test-skill-projectile-cpp-hit-boundary.ps1",
        "scripts\test-skill-projectile-hit-entrypoint-cpp-only.ps1",
        "scripts\test-skill-projectile-runtime-entrypoints-cpp-only.ps1",
        "scripts\test-chain-lightning-damage-authority-boundary.ps1",
        "scripts\test-damage-calculator-authority-boundary.ps1",
        "scripts\test-healing-shield-authority-boundary.ps1",
        "scripts\test-ability-system-state-authority-boundary.ps1",
        "scripts\test-zodiac-character-fallback-state-authority-boundary.ps1",
        "scripts\test-zodiac-character-death-team-authority-boundary.ps1",
        "scripts\test-zodiac-character-death-finalize-timer-boundary.ps1",
        "scripts\test-zodiac-character-death-idempotent-boundary.ps1",
        "scripts\test-unreal-ui-runtime-chinese-output-contract.ps1",
        "scripts\test-player-state-match-stats-authority-boundary.ps1",
        "scripts\test-monster-ai-state-authority-boundary.ps1",
        "scripts\test-monster-ai-movement-authority-boundary.ps1",
        "scripts\test-zodiac-ultimate-energy-cost-constants.ps1",
        "scripts\test-zodiac-character-arena-hud-chain-announcement.ps1",
        "scripts\test-ai-showcase-automation-runner.ps1",
        "scripts\test-ai-showcase-widget-tree-contract.ps1",
        "scripts\test-fixed-skill-group-datatable-diagnostic.ps1",
        "scripts\test-data-table-count-constants.ps1",
        "scripts\test-fixed-skill-group-asset-test-constants.ps1",
        "scripts\test-fixed-skill-group-source-csv.ps1",
        "scripts\test-fixed-skill-group-datatable-import.ps1",
        "scripts\test-admin-match-reward-display-contract.ps1",
        "scripts\test-admin-auth-session-storage-contract.ps1",
        "scripts\test-admin-auth-interceptor-scope-contract.ps1",
        "scripts\test-admin-auth-return-url-contract.ps1",
        "scripts\test-launcher-csp-contract.ps1",
        "scripts\test-launcher-manifest-url-policy-contract.ps1",
        "scripts\validate-unreal-baseline-entrypoints.ps1",
        "scripts\validate-production-evidence-contracts.ps1",
        "scripts\production-preflight.ps1",
        "scripts\run-unreal-evidence.ps1",
        "scripts\diagnose-unreal-evidence-runner.ps1",
        "scripts\run-ai-showcase-automation.ps1",
        "scripts\diagnose-fixed-skill-group-datatable.ps1",
        "scripts\write-fixed-skill-group-source-csv.ps1",
        "scripts\import-fixed-skill-group-datatable.ps1"
    )
}

Invoke-Step "workflow YAML parse" {
    Test-WorkflowYaml @(
        ".github\workflows\client-release-evidence.yml",
        ".github\workflows\unreal-evidence.yml",
        ".github\workflows\solution-ci.yml",
        ".github\workflows\security-ci.yml"
    )
}

Write-Host "PASS: production evidence automation tests" -ForegroundColor Green
