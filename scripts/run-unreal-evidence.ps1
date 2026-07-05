<#
Runs the reusable Unreal production evidence flow.

This is the local and GitHub self-hosted runner entrypoint for:
1. production evidence contract validation,
2. read-only runner diagnostics,
3. optional Dedicated Server packaging/readiness checks,
4. UE online validation evidence,
5. production evidence manifest collection.
#>

[CmdletBinding()]
param(
    [string]$BaseUrl = "http://localhost:8080",
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$ProjectPath = "",
    [string]$InternalApiKey = $env:DBA_INTERNAL_API_KEY,
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence"),
    [string]$RunId = "",
    [switch]$PackageServer,
    [switch]$UsePackagedServer,
    [switch]$SkipClientLaunch,
    [switch]$SkipAIShowcaseAutomation,
    [int]$ClientValidationWaitSec = 45,
    [string]$PackagedRoot = "",
    [string]$ServerExePath = "",
    [switch]$SkipBackendProbe
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\DivineBeastsArena.uproject"
}

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-ue-evidence-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[unreal-evidence] " + $Message) -ForegroundColor Cyan
}

function Resolve-InternalApiKey {
    param([string]$ExplicitInternalApiKey)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitInternalApiKey)) {
        return $ExplicitInternalApiKey
    }

    $envPath = Join-Path $repoRoot "DBA_GameBackend\.env"
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

$resolvedEvidenceRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceRoot)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceRoot | Out-Null

$resolvedInternalApiKey = Resolve-InternalApiKey -ExplicitInternalApiKey $InternalApiKey
if ([string]::IsNullOrWhiteSpace($resolvedInternalApiKey)) {
    throw "Internal API key is required. Pass -InternalApiKey, set DBA_INTERNAL_API_KEY, or configure DBA_GameBackend/.env INTERNAL_API_KEY."
}

Write-Step "validating production evidence contracts"
& (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1")

Write-Step "diagnosing Unreal evidence runner"
$diagnosticArgs = @{
    BaseUrl = $BaseUrl
    UnrealRoot = $UnrealRoot
    ProjectPath = $ProjectPath
    InternalApiKey = $resolvedInternalApiKey
    JsonOutputPath = (Join-Path $resolvedEvidenceRoot ("unreal\runner-diagnostic-{0}.json" -f $RunId))
}
if ($SkipBackendProbe) {
    $diagnosticArgs.SkipBackendProbe = $true
}
& (Join-Path $repoRoot "scripts\diagnose-unreal-evidence-runner.ps1") @diagnosticArgs

if ($PackageServer) {
    Write-Step "packaging Unreal Dedicated Server"
    $packageArgs = @{
        UnrealRoot = $UnrealRoot
        ProjectPath = $ProjectPath
    }
    if (-not [string]::IsNullOrWhiteSpace($PackagedRoot)) {
        $packageArgs.ArchiveDirectory = $PackagedRoot
    }
    & (Join-Path $repoRoot "scripts\package-unreal-dedicated-server.ps1") @packageArgs
}

if ($UsePackagedServer) {
    Write-Step "diagnosing packaged server readiness"
    $readinessArgs = @{
        ProjectPath = $ProjectPath
    }
    if (-not [string]::IsNullOrWhiteSpace($PackagedRoot)) {
        $readinessArgs.PackagedRoot = $PackagedRoot
    }
    if (-not [string]::IsNullOrWhiteSpace($ServerExePath)) {
        $readinessArgs.ServerExePath = $ServerExePath
    }
    & (Join-Path $repoRoot "scripts\diagnose-unreal-packaged-server-readiness.ps1") @readinessArgs
}

if (-not $SkipAIShowcaseAutomation) {
    Write-Step "running AI_Showcase automation regression"
    & (Join-Path $repoRoot "scripts\run-ai-showcase-automation.ps1") `
        -UnrealRoot $UnrealRoot `
        -ProjectPath $ProjectPath `
        -EvidenceDir $resolvedEvidenceRoot `
        -RunId $RunId
}

Write-Step "running UE online validation"
$validationArgs = @{
    BaseUrl = $BaseUrl
    UnrealRoot = $UnrealRoot
    ProjectPath = $ProjectPath
    InternalApiKey = $resolvedInternalApiKey
    EvidenceDir = $resolvedEvidenceRoot
    RunId = $RunId
    ClientValidationWaitSec = $ClientValidationWaitSec
}
if ($UsePackagedServer) {
    $validationArgs.UsePackagedServer = $true
}
if ($SkipClientLaunch) {
    $validationArgs.SkipClientLaunch = $true
}
if (-not [string]::IsNullOrWhiteSpace($PackagedRoot)) {
    $validationArgs.PackagedRoot = $PackagedRoot
}
if (-not [string]::IsNullOrWhiteSpace($ServerExePath)) {
    $validationArgs.ServerExePath = $ServerExePath
}
& (Join-Path $repoRoot "scripts\start-local-ue-validation.ps1") @validationArgs

Write-Step "collecting production evidence manifest"
& (Join-Path $repoRoot "scripts\collect-production-evidence.ps1") `
    -EvidenceRoot $resolvedEvidenceRoot `
    -ReleaseId $RunId

Write-Step "completed Unreal evidence run: $RunId"
