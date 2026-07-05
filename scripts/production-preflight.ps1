<#
Production preflight for the DivineBeastsArena monorepo.
Run from the repository root:
  .\scripts\production-preflight.ps1
#>

[CmdletBinding()]
param(
    [switch]$SkipNode,
    [switch]$SkipDocker,
    [switch]$SkipCargo,
    [switch]$SkipUnreal,
    [switch]$SkipUnrealAutomation,
    [switch]$SkipUnrealServerSmoke,
    [switch]$SkipUnrealOnlineValidation,
    [switch]$RunUnrealOnlineClients,
    [switch]$CollectEvidence,
    [switch]$RequireReleaseReady,
    [switch]$UsePackagedUnrealServer,
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$UnrealProjectPath = "",
    [string]$UnrealAutomationTests = "DivineBeastsArena.Combat.PlayableSkillCatalog",
    [string]$UnrealOnlineBaseUrl = "http://localhost:8083",
    [string]$InternalApiKey = $env:DBA_INTERNAL_API_KEY,
    [string]$EvidenceRoot = "",
    [string]$RunId = "",
    [string]$PackagedRoot = "",
    [string]$ServerExePath = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$failures = New-Object System.Collections.Generic.List[string]

function Invoke-Check {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Script
    )

    Write-Host ""
    Write-Host "==> $Name" -ForegroundColor Cyan
    try {
        & $Script
        Write-Host "PASS: $Name" -ForegroundColor Green
    }
    catch {
        $message = $_.Exception.Message
        $failures.Add("${Name}: $message")
        Write-Host "FAIL: $Name" -ForegroundColor Red
        Write-Host $message -ForegroundColor Red
    }
}

function Test-HttpEndpoint {
    param(
        [Parameter(Mandatory = $true)][string]$Url
    )

    try {
        $response = Invoke-WebRequest -Uri $Url -Method Head -TimeoutSec 3 -UseBasicParsing
        return ($response.StatusCode -ge 200 -and $response.StatusCode -lt 500)
    }
    catch {
        return $false
    }
}

function Resolve-BackendUrl {
    param(
        [Parameter(Mandatory = $true)][string[]]$Candidates
    )

    foreach ($candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) {
            continue
        }

        $liveUrl = $candidate.TrimEnd("/") + "/health/live"
        Write-Host ("[preflight] probing backend liveness: {0}" -f $liveUrl)
        if (Test-HttpEndpoint $liveUrl) {
            return $candidate.TrimEnd("/")
        }
    }

    return ""
}

function Resolve-InternalApiKey {
    param(
        [string]$ExplicitInternalApiKey,
        [string]$BackendDirectory
    )

    if (-not [string]::IsNullOrWhiteSpace($ExplicitInternalApiKey)) {
        return $ExplicitInternalApiKey
    }

    $envPath = Join-Path $BackendDirectory ".env"
    if (-not (Test-Path -LiteralPath $envPath)) {
        return ""
    }

    $line = Get-Content -Encoding UTF8 $envPath |
        Where-Object { $_ -match "^INTERNAL_API_KEY=" } |
        Select-Object -First 1

    if (-not $line) {
        return ""
    }

    return $line.Substring("INTERNAL_API_KEY=".Length)
}

function Resolve-PreflightRunId {
    param([string]$ExplicitRunId)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitRunId)) {
        return ($ExplicitRunId -replace "[^A-Za-z0-9_.-]", "-")
    }

    return "preflight-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Invoke-CommandInDir {
    param(
        [Parameter(Mandatory = $true)][string]$Directory,
        [Parameter(Mandatory = $true)][string]$Command,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )

    Push-Location $Directory
    try {
        & $Command @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "$Command $($Arguments -join ' ') exited with code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Resolve-UnrealTool {
    param(
        [Parameter(Mandatory = $true)][string]$ToolName,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $tool = Get-Command $ToolName -ErrorAction SilentlyContinue
    if ($tool) {
        return $tool.Source
    }

    $candidateRoots = @()
    if (-not [string]::IsNullOrWhiteSpace($UnrealRoot)) {
        $candidateRoots += $UnrealRoot
    }
    $candidateRoots += "D:\UnrealEngine-5.8.0-release"

    foreach ($root in ($candidateRoots | Select-Object -Unique)) {
        foreach ($relativePath in $RelativePaths) {
            $candidate = Join-Path $root $relativePath
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    throw "$ToolName not found. Set UNREAL_ENGINE_ROOT or pass -UnrealRoot."
}

Set-Location $repoRoot

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

$resolvedRunId = Resolve-PreflightRunId -ExplicitRunId $RunId
$resolvedEvidenceRoot = if ([string]::IsNullOrWhiteSpace($EvidenceRoot)) {
    Join-Path $repoRoot "Artifacts\ProductionEvidence"
}
else {
    $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceRoot)
}

if ($CollectEvidence -and -not (Test-Path -LiteralPath $resolvedEvidenceRoot)) {
    New-Item -ItemType Directory -Force -Path $resolvedEvidenceRoot | Out-Null
}

Invoke-Check "git workspace" {
    $status = git status --short
    if ($status) {
        Write-Host $status
        Write-Host "Note: dirty workspace is allowed for preflight, but production release should be committed or cleaned." -ForegroundColor Yellow
    }
}

Invoke-Check "production evidence contracts" {
    & (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1")
}

Invoke-Check "Unreal module boundaries" {
    & (Join-Path $repoRoot "scripts\validate-unreal-module-boundaries.ps1")
}

Invoke-Check "Unreal module boundary fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-module-boundaries.ps1")
}

Invoke-Check "Unreal baseline entrypoints" {
    & (Join-Path $repoRoot "scripts\validate-unreal-baseline-entrypoints.ps1")
}

Invoke-Check "Unreal baseline entrypoint fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-baseline-entrypoints.ps1")
}

Invoke-Check "Unreal Moba foundation" {
    & (Join-Path $repoRoot "scripts\validate-unreal-moba-foundation.ps1")
}

Invoke-Check "Unreal Moba foundation fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-moba-foundation.ps1")
}

Invoke-Check "Unreal source guardrails" {
    & (Join-Path $repoRoot "scripts\validate-unreal-source-guardrails.ps1")
}

Invoke-Check "Unreal source guardrail fixtures" {
    & (Join-Path $repoRoot "scripts\test-unreal-source-guardrails.ps1")
}

Invoke-Check "internal API route protection" {
    & (Join-Path $repoRoot "scripts\validate-internal-api-route-protection.ps1")
}

Invoke-Check "internal API route protection fixtures" {
    & (Join-Path $repoRoot "scripts\test-internal-api-route-protection-contract.ps1")
}

Invoke-Check "player_id claim boundary" {
    & (Join-Path $repoRoot "scripts\test-player-id-claim-boundary-contract.ps1")
}

Invoke-Check "backend dotnet test" {
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "dotnet" @("test", "GameBackend.sln", "--configuration", "Release")
}

Invoke-Check "admin angular build" {
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameAdmin") "npm" @("ci")
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameAdmin") "npm" @("run", "build")
}

if (-not $SkipNode) {
    Invoke-Check "website npm build" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameWebsite") "npm" @("install")
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameWebsite") "npm" @("run", "build")
    }

    Invoke-Check "launcher npm build" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "npm" @("install")
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "npm" @("run", "build")
    }
}

if (-not $SkipCargo) {
    Invoke-Check "launcher cargo test" {
        if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
            throw "cargo not found. Install Rust or rerun with -SkipCargo."
        }
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "cargo" @("test", "--manifest-path", "src-tauri/Cargo.toml")
    }

    Invoke-Check "launcher cargo check" {
        if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
            throw "cargo not found. Install Rust or rerun with -SkipCargo."
        }
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "cargo" @("check", "--manifest-path", "src-tauri/Cargo.toml")
    }
}

if (-not $SkipDocker) {
    Invoke-Check "backend docker compose config" {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw "docker not found. Install Docker or rerun with -SkipDocker."
        }
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "docker" @("compose", "--env-file", ".env.example", "config")
    }

    Invoke-Check "observability docker compose config" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "docker" @("compose", "--env-file", ".env.example", "-f", "docker-compose.observability.yml", "config")
    }
}

if (-not $SkipUnreal) {
    Invoke-Check "UnrealBuildTool availability" {
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Write-Host "UnrealBuildTool: $ubt"
    }

    Invoke-Check "Unreal editor target build" {
        $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
            Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
        }
        else {
            Resolve-Path $UnrealProjectPath
        }
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $ubt @(
            "DivineBeastsArenaEditor",
            "Win64",
            "Development",
            "-Project=$projectPath",
            "-WaitMutex",
            "-NoHotReloadFromIDE"
        )
    }

    Invoke-Check "Unreal dedicated server target build" {
        $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
            Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
        }
        else {
            Resolve-Path $UnrealProjectPath
        }
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $ubt @(
            "DivineBeastsArenaServer",
            "Win64",
            "Development",
            "-Project=$projectPath",
            "-WaitMutex",
            "-NoHotReloadFromIDE"
        )
    }

    if (-not $SkipUnrealServerSmoke) {
        Invoke-Check "Unreal dedicated server smoke" {
            $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
                Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
            }
            else {
                Resolve-Path $UnrealProjectPath
            }
            & (Join-Path $repoRoot "scripts\smoke-unreal-dedicated-server.ps1") -UnrealRoot $UnrealRoot -ProjectPath $projectPath
        }
    }

    if (-not $SkipUnrealAutomation) {
        Invoke-Check "Unreal automation tests" {
            $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
                Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
            }
            else {
                Resolve-Path $UnrealProjectPath
            }
            $editorCmd = Resolve-UnrealTool "UnrealEditor-Cmd" @("Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
            Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $editorCmd @(
                "$projectPath",
                "-ExecCmds=Automation RunTests $UnrealAutomationTests; Quit",
                "-unattended",
                "-nop4",
                "-nosplash",
                "-NullRHI",
                "-log"
            )
        }
    }

    if (-not $SkipUnrealOnlineValidation) {
        Invoke-Check "Unreal online validation smoke" {
            $resolvedBackendUrl = Resolve-BackendUrl @(
                $UnrealOnlineBaseUrl,
                "http://localhost:8080",
                "http://127.0.0.1:8080",
                "http://localhost:8083",
                "http://127.0.0.1:8083"
            )
            if ([string]::IsNullOrWhiteSpace($resolvedBackendUrl)) {
                throw "backend health probe failed for configured URLs. Pass -SkipUnrealOnlineValidation or set -UnrealOnlineBaseUrl."
            }

            Write-Host ("[preflight] resolved online validation base url: {0}" -f $resolvedBackendUrl)
            $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
                Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
            }
            else {
                Resolve-Path $UnrealProjectPath
            }

            $resolvedInternalApiKey = Resolve-InternalApiKey `
                -ExplicitInternalApiKey $InternalApiKey `
                -BackendDirectory (Join-Path $repoRoot "DBA_GameBackend")

            $validationArgs = @{
                BaseUrl = $resolvedBackendUrl
                UnrealRoot = $UnrealRoot
                ProjectPath = $projectPath
                BackendProbeTimeoutSec = 30
                InternalApiKey = $resolvedInternalApiKey
            }

            if (-not $RunUnrealOnlineClients) {
                $validationArgs.SkipClientLaunch = $true
            }

            if ($CollectEvidence) {
                # Forwarded start-local-ue-validation.ps1 contract: -EvidenceDir, -RunId, and optionally -UsePackagedServer.
                $validationArgs.EvidenceDir = $resolvedEvidenceRoot
                $validationArgs.RunId = "$resolvedRunId-ue-online"
            }

            if ($UsePackagedUnrealServer) {
                $validationArgs.UsePackagedServer = $true
            }

            if (-not [string]::IsNullOrWhiteSpace($PackagedRoot)) {
                $validationArgs.PackagedRoot = $PackagedRoot
            }

            if (-not [string]::IsNullOrWhiteSpace($ServerExePath)) {
                $validationArgs.ServerExePath = $ServerExePath
            }

            & (Join-Path $repoRoot "scripts\start-local-ue-validation.ps1") @validationArgs
        }
    }
}

if ($CollectEvidence) {
    Invoke-Check "production evidence manifest" {
        & (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
            -EvidenceRoot $resolvedEvidenceRoot `
            -ReleaseId $resolvedRunId
    }
}

if ($CollectEvidence -or $RequireReleaseReady) {
    Invoke-Check "release readiness report" {
        $reportScript = Join-Path $repoRoot "scripts\write-release-readiness-report.ps1"
        if ($RequireReleaseReady) {
            & $reportScript -EvidenceRoot $resolvedEvidenceRoot -RequireReady
        }
        else {
            & $reportScript -EvidenceRoot $resolvedEvidenceRoot
        }
    }

    Invoke-Check "release blocker actions" {
        & (Join-Path $repoRoot "scripts\diagnose-release-blockers.ps1") `
            -ReportPath (Join-Path $resolvedEvidenceRoot "release-readiness-report.json")
    }

    Invoke-Check "release blocker action validation" {
        & (Join-Path $repoRoot "scripts\validate-release-blocker-actions.ps1") `
            -ActionReportPath (Join-Path $resolvedEvidenceRoot "release-blocker-actions.json") `
            -RequireValid
    }

    Invoke-Check "release blockers external-only validation" {
        & (Join-Path $repoRoot "scripts\validate-release-blockers-external-only.ps1") `
            -ActionReportPath (Join-Path $resolvedEvidenceRoot "release-blocker-actions.json") `
            -RequireValid
    }

    Invoke-Check "release readiness blocker posture refresh" {
        & (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1") `
            -EvidenceRoot $resolvedEvidenceRoot
    }

    Invoke-Check "development continuation readiness validation" {
        & (Join-Path $repoRoot "scripts\validate-development-continuation-readiness.ps1") `
            -ReadinessReportPath (Join-Path $resolvedEvidenceRoot "release-readiness-report.json") `
            -RequireReady
    }

    Invoke-Check "release input template" {
        & (Join-Path $repoRoot "scripts\write-release-input-template.ps1") `
            -ActionReportPath (Join-Path $resolvedEvidenceRoot "release-blocker-actions.json")
    }

    Invoke-Check "release input template validation" {
        & (Join-Path $repoRoot "scripts\validate-release-input-template.ps1") `
            -TemplatePath (Join-Path $resolvedEvidenceRoot "release-input-template.json") `
            -RequireValid
    }

    Invoke-Check "release input values template" {
        & (Join-Path $repoRoot "scripts\write-release-input-values-template.ps1") `
            -TemplatePath (Join-Path $resolvedEvidenceRoot "release-input-template.json")
    }

    Invoke-Check "release input values template validation" {
        & (Join-Path $repoRoot "scripts\validate-release-input-values.ps1") `
            -ValuesPath (Join-Path $resolvedEvidenceRoot "release-input-values.template.json")
    }

    Invoke-Check "release command plan template check" {
        & (Join-Path $repoRoot "scripts\resolve-release-input-template.ps1") `
            -TemplatePath (Join-Path $resolvedEvidenceRoot "release-input-template.json") `
            -ValuesPath (Join-Path $resolvedEvidenceRoot "release-input-values.template.json") `
            -OutputJsonPath (Join-Path $resolvedEvidenceRoot "release-command-plan.template-check.json") `
            -OutputMarkdownPath (Join-Path $resolvedEvidenceRoot "release-command-plan.template-check.md")
    }
}

Write-Host ""
if ($failures.Count -gt 0) {
    Write-Host "Production preflight failed:" -ForegroundColor Red
    foreach ($failure in $failures) {
        Write-Host "- $failure" -ForegroundColor Red
    }
    exit 1
}

Write-Host "Production preflight passed." -ForegroundColor Green
