<#
Runs the client release evidence bundle for a Windows public client package.

This is an orchestration script. It delegates package hashing, code signing,
launcher install/update smoke, and optional CDN smoke to the focused scripts
that already own those contracts.

Examples:
  .\scripts\run-client-release-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -RunId rc1
  .\scripts\run-client-release-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -DownloadUrl https://cdn.example.com/releases/0.1.0.0/ -ManifestUrl https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json -RequireSigned
  .\scripts\run-client-release-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -PrepareCdnPayload -PayloadRoot .tmp\cdn-upload\0.1.0.0 -RunLocalCdnPayloadSmoke -DownloadUrl https://cdn.example.com/releases/0.1.0.0/
  .\scripts\run-client-release-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -CaptureLauncherUiEvidence -RunId rc-ui
  .\scripts\run-client-release-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -SignPackage -CertificateThumbprint "<thumbprint>" -RequireSigned
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [string]$PackageRoot = "",
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence"),
    [string]$RunId = "",
    [string]$Version = "0.1.0.0",
    [ValidateSet("Development", "Shipping")]
    [string]$BuildConfiguration = "Shipping",
    [string]$DownloadUrl = "",
    [string]$ManifestUrl = "",
    [switch]$PrepareCdnPayload,
    [string]$PayloadRoot = "",
    [switch]$RunLocalCdnPayloadSmoke,
    [switch]$SignPackage,
    [string]$SignToolPath = "",
    [string]$CertificateThumbprint = "",
    [string]$CertificateSubject = "",
    [string]$PfxPath = "",
    [string]$PfxPasswordEnvironmentVariable = "DBA_CODE_SIGNING_PFX_PASSWORD",
    [string]$TimestampUrl = "http://timestamp.digicert.com",
    [switch]$RequireSigned,
    [switch]$SkipLauncherInstallUpdate,
    [switch]$CaptureLauncherUiEvidence,
    [switch]$SkipCdnSmoke,
    [switch]$SkipReleasePrerequisiteCheck,
    [switch]$AllowLocalHttpCdn
)

$ErrorActionPreference = "Stop"
$signPackageWhatIf = $WhatIfPreference
$WhatIfPreference = $false
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "client-release-evidence-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[client-release-evidence] " + $Message) -ForegroundColor Cyan
}

function Resolve-ClientPackageRoot {
    param([string]$ExplicitPackageRoot)

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitPackageRoot)) {
        $candidates += $ExplicitPackageRoot
    }
    $candidates += @(
        (Join-Path $repoRoot ".tmp\client-release\public"),
        (Join-Path $repoRoot ".tmp\packaged-client\Windows"),
        (Join-Path $repoRoot ".tmp\packaged-server\Windows"),
        (Join-Path $repoRoot "DBA_GameClient\Saved\StagedBuilds\Windows"),
        (Join-Path $repoRoot "Artifacts\UnrealClient\Windows")
    )

    foreach ($candidate in $candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate) -or -not (Test-Path -LiteralPath $candidate)) {
            continue
        }

        $resolved = (Resolve-Path -LiteralPath $candidate).ProviderPath
        $clientExe = Get-ChildItem -LiteralPath $resolved -Recurse -File -Filter "DivineBeastsArena.exe" -ErrorAction SilentlyContinue |
            Select-Object -First 1
        if ($clientExe) {
            return $resolved
        }
    }

    throw "No Windows client package with DivineBeastsArena.exe was found. Pass -PackageRoot."
}

function Read-JsonFile {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action
    )

    Write-Step $Name
    & $Action
}

$resolvedPackageRoot = Resolve-ClientPackageRoot -ExplicitPackageRoot $PackageRoot
$resolvedEvidenceRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceRoot)
$clientEvidenceDir = Join-Path $resolvedEvidenceRoot "client"
New-Item -ItemType Directory -Force -Path $clientEvidenceDir | Out-Null

if ([string]::IsNullOrWhiteSpace($DownloadUrl)) {
    $DownloadUrl = "https://cdn.example.com/releases/$Version/"
}

$packageRunId = "$RunId-package"
$signingRunId = "$RunId-signing"
$launcherRunId = "$RunId-launcher-install-update"
$launcherUiRunId = "$RunId-launcher-ui-visual"
$cdnPayloadRunId = "$RunId-cdn-payload"
$localCdnPayloadSmokeRunId = "$RunId-local-cdn-payload-smoke"
$cdnRunId = "$RunId-cdn-smoke"

if ($RunLocalCdnPayloadSmoke -and -not $PrepareCdnPayload) {
    throw "-RunLocalCdnPayloadSmoke requires -PrepareCdnPayload so there is a prepared payload to serve locally."
}

$resolvedPayloadRoot = ""
if ($PrepareCdnPayload) {
    if ([string]::IsNullOrWhiteSpace($PayloadRoot)) {
        $resolvedPayloadRoot = Join-Path $repoRoot (Join-Path ".tmp\client-cdn-payload" $cdnPayloadRunId)
    }
    else {
        $resolvedPayloadRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($PayloadRoot)
    }
}

$releasePrerequisitePath = ""
$releasePrerequisiteReady = $null
$shouldRunReleasePrerequisites = -not $SkipReleasePrerequisiteCheck -and (
    -not [string]::IsNullOrWhiteSpace($ManifestUrl) -or
    $RequireSigned -or
    $SignPackage
)
if ($shouldRunReleasePrerequisites) {
    $releasePrerequisitePath = Join-Path $clientEvidenceDir ("client-release-prerequisites-{0}.json" -f $RunId)
    $prerequisiteArgs = @{
        PackageRoot = $resolvedPackageRoot
        DownloadUrl = $DownloadUrl
        OutputJsonPath = $releasePrerequisitePath
        FailOnBlockingIssues = $true
    }
    if (-not [string]::IsNullOrWhiteSpace($ManifestUrl)) {
        $prerequisiteArgs.ManifestUrl = $ManifestUrl
        $prerequisiteArgs.RequireManifestUrl = $true
    }
    if ($AllowLocalHttpCdn) {
        $prerequisiteArgs.AllowLocalHttp = $true
    }
    if ($SignPackage) {
        $prerequisiteArgs.RequireSigningIdentity = $true
        $prerequisiteArgs.RequireSignTool = $true
        $prerequisiteArgs.PfxPasswordEnvironmentVariable = $PfxPasswordEnvironmentVariable
        if (-not [string]::IsNullOrWhiteSpace($SignToolPath)) {
            $prerequisiteArgs.SignToolPath = $SignToolPath
        }
        if (-not [string]::IsNullOrWhiteSpace($CertificateThumbprint)) {
            $prerequisiteArgs.CertificateThumbprint = $CertificateThumbprint
        }
        if (-not [string]::IsNullOrWhiteSpace($CertificateSubject)) {
            $prerequisiteArgs.CertificateSubject = $CertificateSubject
        }
        if (-not [string]::IsNullOrWhiteSpace($PfxPath)) {
            $prerequisiteArgs.PfxPath = $PfxPath
        }
    }

    Invoke-Step "diagnosing client release prerequisites" {
        & (Join-Path $repoRoot "scripts\diagnose-client-release-prerequisites.ps1") @prerequisiteArgs
    }
    $releasePrerequisite = Read-JsonFile -Path $releasePrerequisitePath
    $releasePrerequisiteReady = ($releasePrerequisite.readyForReleaseInputs -eq $true)
}

$packageArgs = @{
    PackageRoot = $resolvedPackageRoot
    EvidenceDir = $clientEvidenceDir
    RunId = $packageRunId
    Version = $Version
    BuildConfiguration = $BuildConfiguration
    DownloadUrl = $DownloadUrl
    CopyInstallSmoke = $true
}
if ($BuildConfiguration -eq "Shipping") {
    $packageArgs.DisallowDebugSymbols = $true
}

Invoke-Step "collecting client package launcher evidence" {
    & (Join-Path $repoRoot "scripts\collect-client-package-evidence.ps1") @packageArgs
}

if ($SignPackage) {
    $signingArgs = @{
        PackageRoot = $resolvedPackageRoot
        EvidenceDir = $clientEvidenceDir
        RunId = $signingRunId
        PfxPasswordEnvironmentVariable = $PfxPasswordEnvironmentVariable
        TimestampUrl = $TimestampUrl
    }
    if (-not [string]::IsNullOrWhiteSpace($SignToolPath)) {
        $signingArgs.SignToolPath = $SignToolPath
    }
    if (-not [string]::IsNullOrWhiteSpace($CertificateThumbprint)) {
        $signingArgs.CertificateThumbprint = $CertificateThumbprint
    }
    if (-not [string]::IsNullOrWhiteSpace($CertificateSubject)) {
        $signingArgs.CertificateSubject = $CertificateSubject
    }
    if (-not [string]::IsNullOrWhiteSpace($PfxPath)) {
        $signingArgs.PfxPath = $PfxPath
    }
    if ($RequireSigned) {
        $signingArgs.RequireSigned = $true
    }
    if ($signPackageWhatIf) {
        $signingArgs.WhatIf = $true
    }

    Invoke-Step "signing client package and collecting code signing evidence" {
        & (Join-Path $repoRoot "scripts\sign-client-release-package.ps1") @signingArgs
    }
}
else {
    $signingArgs = @{
        PackageRoot = $resolvedPackageRoot
        EvidenceDir = $clientEvidenceDir
        RunId = $signingRunId
    }
    if ($RequireSigned) {
        $signingArgs.RequireSigned = $true
    }

    Invoke-Step "collecting code signing evidence" {
        & (Join-Path $repoRoot "scripts\collect-code-signing-evidence.ps1") @signingArgs
    }
}

if (-not $SkipLauncherInstallUpdate) {
    Invoke-Step "running launcher install/update smoke" {
        & (Join-Path $repoRoot "scripts\run-launcher-install-update-smoke.ps1") `
            -EvidenceDir $clientEvidenceDir `
            -RunId $launcherRunId
    }
}

$launcherUiEvidencePath = ""
if ($CaptureLauncherUiEvidence) {
    $launcherUiEvidencePath = Join-Path $clientEvidenceDir ("launcher-ui-visual-evidence-{0}.json" -f $launcherUiRunId)
    Invoke-Step "capturing launcher UI visual evidence" {
        & (Join-Path $repoRoot "scripts\capture-launcher-ui-evidence.ps1") `
            -EvidenceDir $clientEvidenceDir `
            -RunId $launcherUiRunId
    }
}

$cdnUploadManifestPath = ""
if ($PrepareCdnPayload) {
    $cdnPayloadArgs = @{
        PackageRoot = $resolvedPackageRoot
        EvidenceDir = $clientEvidenceDir
        RunId = $cdnPayloadRunId
        Version = $Version
        DownloadUrl = $DownloadUrl
    }
    if (-not [string]::IsNullOrWhiteSpace($ManifestUrl)) {
        $cdnPayloadArgs.ManifestUrl = $ManifestUrl
    }
    if (-not [string]::IsNullOrWhiteSpace($resolvedPayloadRoot)) {
        $cdnPayloadArgs.PayloadRoot = $resolvedPayloadRoot
    }
    if ($AllowLocalHttpCdn) {
        $cdnPayloadArgs.AllowLocalHttp = $true
    }

    Invoke-Step "preparing client CDN payload" {
        & (Join-Path $repoRoot "scripts\prepare-client-cdn-payload.ps1") @cdnPayloadArgs
    }
    $cdnUploadManifestPath = Join-Path $clientEvidenceDir ("cdn-upload-manifest-{0}.json" -f $cdnPayloadRunId)
}

$localCdnPayloadSmokeRan = $false
$localCdnPayloadSmokeEvidencePath = ""
if ($RunLocalCdnPayloadSmoke) {
    $localCdnPayloadSmokeRan = $true
    $localCdnPayloadSmokeEvidencePath = Join-Path $clientEvidenceDir ("launcher-cdn-smoke-{0}.json" -f $localCdnPayloadSmokeRunId)
    Invoke-Step "running local CDN payload smoke" {
        & (Join-Path $repoRoot "scripts\run-local-cdn-payload-smoke.ps1") `
            -PayloadRoot $resolvedPayloadRoot `
            -EvidenceDir $clientEvidenceDir `
            -RunId $localCdnPayloadSmokeRunId
    }
}

$cdnSmokeRan = $false
if ($SkipCdnSmoke) {
    Write-Step "skipping launcher CDN smoke because -SkipCdnSmoke was provided"
}
elseif (-not [string]::IsNullOrWhiteSpace($ManifestUrl)) {
    $cdnSmokeRan = $true
    $cdnArgs = @{
        ManifestUrl = $ManifestUrl
        EvidenceDir = $clientEvidenceDir
        RunId = $cdnRunId
    }
    if ($AllowLocalHttpCdn) {
        $cdnArgs.AllowLocalHttp = $true
    }

    Invoke-Step "running launcher CDN smoke" {
        & (Join-Path $repoRoot "scripts\run-launcher-cdn-smoke.ps1") @cdnArgs
    }
}
else {
    Write-Step "skipping launcher CDN smoke because -ManifestUrl was not provided"
}

$packageEvidencePath = Join-Path $clientEvidenceDir ("client-package-launcher-{0}.json" -f $packageRunId)
$signingEvidencePath = Join-Path $clientEvidenceDir ("code-signing-{0}.json" -f $signingRunId)
$launcherEvidencePath = Join-Path $clientEvidenceDir ("launcher-install-update-smoke-{0}.json" -f $launcherRunId)
$cdnEvidencePath = Join-Path $clientEvidenceDir ("launcher-cdn-smoke-{0}.json" -f $cdnRunId)

$packageEvidence = Read-JsonFile -Path $packageEvidencePath
$signingEvidence = Read-JsonFile -Path $signingEvidencePath
$launcherEvidence = Read-JsonFile -Path $launcherEvidencePath
$launcherUiEvidence = if ($CaptureLauncherUiEvidence) { Read-JsonFile -Path $launcherUiEvidencePath } else { $null }
$cdnUploadManifest = if ($PrepareCdnPayload) { Read-JsonFile -Path $cdnUploadManifestPath } else { $null }
$localCdnPayloadSmokeEvidence = if ($localCdnPayloadSmokeRan) { Read-JsonFile -Path $localCdnPayloadSmokeEvidencePath } else { $null }
$cdnEvidence = Read-JsonFile -Path $cdnEvidencePath

$releaseReady = $packageEvidence.releaseReady -eq $true -and
    $signingEvidence.signingReady -eq $true -and
    ($SkipLauncherInstallUpdate -or $launcherEvidence.installUpdateReady -eq $true) -and
    ($cdnSmokeRan -and $cdnEvidence.cdnReady -eq $true)

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "client-release-evidence"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    packageRoot = $resolvedPackageRoot
    evidenceRoot = $resolvedEvidenceRoot
    version = $Version
    buildConfiguration = $BuildConfiguration
    downloadUrl = $DownloadUrl
    manifestUrl = $ManifestUrl
    cdnPayloadPrepared = ($PrepareCdnPayload -eq $true)
    cdnPayloadReady = if ($PrepareCdnPayload) { ($cdnUploadManifest.payloadReady -eq $true) } else { $null }
    localCdnPayloadSmokeRan = ($RunLocalCdnPayloadSmoke -eq $true)
    localCdnPayloadSmokeReady = if ($localCdnPayloadSmokeRan) { ([int]$localCdnPayloadSmokeEvidence.downloadedFileCount -gt 0) } else { $null }
    launcherUiVisualCaptured = ($CaptureLauncherUiEvidence -eq $true)
    launcherUiVisualReady = if ($CaptureLauncherUiEvidence) { ($launcherUiEvidence.uiEvidenceReady -eq $true -and $launcherUiEvidence.screenshotReady -eq $true -and $launcherUiEvidence.uiMarkersReady -eq $true) } else { $null }
    releasePrerequisiteChecked = ($shouldRunReleasePrerequisites -eq $true)
    releasePrerequisiteReady = $releasePrerequisiteReady
    signingAttempted = ($SignPackage -eq $true)
    releaseReady = $releaseReady
    packageReleaseReady = ($packageEvidence.releaseReady -eq $true)
    signingReady = ($signingEvidence.signingReady -eq $true)
    launcherInstallUpdateReady = if ($SkipLauncherInstallUpdate) { $null } else { ($launcherEvidence.installUpdateReady -eq $true) }
    cdnReady = if ($cdnSmokeRan) { ($cdnEvidence.cdnReady -eq $true) } else { $false }
    evidence = [ordered]@{
        package = $packageEvidencePath
        signing = $signingEvidencePath
        launcherInstallUpdate = if ($SkipLauncherInstallUpdate) { "" } else { $launcherEvidencePath }
        launcherUiVisual = if ($CaptureLauncherUiEvidence) { $launcherUiEvidencePath } else { "" }
        cdnUploadManifest = if ($PrepareCdnPayload) { $cdnUploadManifestPath } else { "" }
        localCdnPayloadSmoke = if ($localCdnPayloadSmokeRan) { $localCdnPayloadSmokeEvidencePath } else { "" }
        cdnSmoke = if ($cdnSmokeRan) { $cdnEvidencePath } else { "" }
        releasePrerequisites = $releasePrerequisitePath
    }
    notes = @(
        if ($packageEvidence.releaseReady -ne $true) { "Client package evidence is not release-ready; verify Shipping, no debug symbols, HTTPS, and non-example CDN URL." }
        if ($signingEvidence.signingReady -ne $true) { "Code signing evidence is not release-ready." }
        if (-not $SkipLauncherInstallUpdate -and $launcherEvidence.installUpdateReady -ne $true) { "Launcher install/update smoke is not release-ready." }
        if ($CaptureLauncherUiEvidence -and $launcherUiEvidence.uiEvidenceReady -ne $true) { "Launcher UI visual evidence is not ready; inspect launcherUiVisual evidence." }
        if ($PrepareCdnPayload -and $cdnUploadManifest.payloadReady -ne $true) { "CDN upload payload is not release-ready; verify HTTPS and non-example CDN URLs." }
        if ($localCdnPayloadSmokeRan -and [int]$localCdnPayloadSmokeEvidence.downloadedFileCount -le 0) { "Local CDN payload smoke did not download any files from the prepared payload." }
        if ($SkipCdnSmoke) { "CDN smoke was skipped because SkipCdnSmoke was provided." }
        elseif (-not $cdnSmokeRan) { "CDN smoke was skipped because ManifestUrl was not provided." }
        elseif ($cdnEvidence.cdnReady -ne $true) { "CDN smoke evidence is not release-ready." }
    )
}

$summaryPath = Join-Path $clientEvidenceDir ("client-release-evidence-{0}.json" -f $RunId)
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Step "wrote client release evidence summary: $summaryPath"

if (-not $releaseReady) {
    Write-Host "WARN: client release evidence is not release-ready; see summary notes." -ForegroundColor Yellow
}

Write-Host "PASS: client release evidence bundle collected" -ForegroundColor Green
