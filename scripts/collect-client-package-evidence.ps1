<#
Creates manifest-ready evidence for a packaged Windows client and launcher repair contract.

The script does not package Unreal by itself. Use package-unreal-dedicated-server.ps1
with -IncludeClientCook or a release pipeline first, then point this script at the
staged Windows client package.
#>

[CmdletBinding()]
param(
    [string]$PackageRoot = "",
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$Version = "0.1.0.0",
    [ValidateSet("Development", "Shipping")]
    [string]$BuildConfiguration = "Development",
    [string]$DownloadUrl = "",
    [string]$InstallSmokeRoot = "",
    [switch]$CopyInstallSmoke,
    [switch]$DisallowDebugSymbols
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-client-package-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if ([string]::IsNullOrWhiteSpace($DownloadUrl)) {
    $DownloadUrl = "https://cdn.example.com/releases/$Version/"
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[client-package] " + $Message) -ForegroundColor Cyan
}

function Get-PortableRelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$RootPath,
        [Parameter(Mandatory = $true)][string]$FilePath
    )

    $normalizedRoot = [System.IO.Path]::GetFullPath($RootPath)
    if (-not $normalizedRoot.EndsWith([System.IO.Path]::DirectorySeparatorChar.ToString())) {
        $normalizedRoot = $normalizedRoot + [System.IO.Path]::DirectorySeparatorChar
    }

    $rootUri = New-Object System.Uri($normalizedRoot)
    $fileUri = New-Object System.Uri([System.IO.Path]::GetFullPath($FilePath))
    return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($fileUri).ToString()).Replace("\", "/")
}

function Resolve-ClientPackageRoot {
    param([string]$ExplicitPackageRoot)

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitPackageRoot)) {
        $candidates += $ExplicitPackageRoot
    }
    $candidates += @(
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

    throw "No staged Windows client package with DivineBeastsArena.exe was found. Run .\scripts\package-unreal-dedicated-server.ps1 -IncludeClientCook or pass -PackageRoot."
}

function Assert-LauncherManifestFile {
    param([Parameter(Mandatory = $true)]$FileEntry)

    if ([string]::IsNullOrWhiteSpace($FileEntry.name)) {
        throw "launcher-manifest file name is empty"
    }
    if ($FileEntry.name.StartsWith("/") -or $FileEntry.name.StartsWith("\") -or $FileEntry.name.Contains("../") -or $FileEntry.name.Contains("..\")) {
        throw "launcher-manifest file path is unsafe: $($FileEntry.name)"
    }
    if ($FileEntry.sha256.Length -ne 64 -or $FileEntry.sha256 -notmatch "^[0-9a-f]{64}$") {
        throw "launcher-manifest file SHA256 is invalid: $($FileEntry.name)"
    }
    if ([int64]$FileEntry.size -le 0) {
        throw "launcher-manifest file size must be greater than zero: $($FileEntry.name)"
    }
}

function Resolve-ReleaseUri {
    param([Parameter(Mandatory = $true)][string]$Url)

    $uri = $null
    if (-not [System.Uri]::TryCreate($Url, [System.UriKind]::Absolute, [ref]$uri) -or [string]::IsNullOrWhiteSpace($uri.Host)) {
        return $null
    }

    return $uri
}

function Copy-InstallSmokeFiles {
    param(
        [Parameter(Mandatory = $true)][string]$SourceRoot,
        [Parameter(Mandatory = $true)][string]$DestinationRoot,
        [Parameter(Mandatory = $true)]$ManifestFiles,
        [Parameter(Mandatory = $true)][string]$Version
    )

    if (Test-Path -LiteralPath $DestinationRoot) {
        Remove-Item -LiteralPath $DestinationRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $DestinationRoot | Out-Null

    foreach ($file in $ManifestFiles) {
        $source = Join-Path $SourceRoot ($file.name -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        $destination = Join-Path $DestinationRoot ($file.name -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        $parent = Split-Path -Parent $destination
        if (-not (Test-Path -LiteralPath $parent)) {
            New-Item -ItemType Directory -Force -Path $parent | Out-Null
        }
        Copy-Item -LiteralPath $source -Destination $destination -Force

        $copiedHash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($copiedHash -ne $file.sha256) {
            throw "install smoke copied file hash mismatch: $($file.name)"
        }
    }

    Set-Content -LiteralPath (Join-Path $DestinationRoot "version.txt") -Value $Version -Encoding UTF8
}

$resolvedPackageRoot = Resolve-ClientPackageRoot -ExplicitPackageRoot $PackageRoot
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

if ([string]::IsNullOrWhiteSpace($InstallSmokeRoot)) {
    $InstallSmokeRoot = Join-Path $repoRoot ".tmp\launcher-install-smoke\$RunId"
}
$resolvedInstallSmokeRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($InstallSmokeRoot)

Write-Step "hashing staged client package: $resolvedPackageRoot"
$packageFiles = @(
    Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File |
        Sort-Object FullName
)
if ($packageFiles.Count -eq 0) {
    throw "client package contains no files: $resolvedPackageRoot"
}

$clientExe = @($packageFiles | Where-Object { $_.Name -eq "DivineBeastsArena.exe" })[0]
if (-not $clientExe) {
    throw "DivineBeastsArena.exe was not found in client package: $resolvedPackageRoot"
}

$contentContainers = @($packageFiles | Where-Object { $_.Extension -in @(".pak", ".utoc", ".ucas") })
if ($contentContainers.Count -eq 0) {
    throw "client package has no cooked content container (.pak/.utoc/.ucas): $resolvedPackageRoot"
}

$debugSymbolFiles = @($packageFiles | Where-Object { $_.Extension -in @(".pdb", ".dbg", ".dSYM") })
if ($DisallowDebugSymbols -and $debugSymbolFiles.Count -gt 0) {
    $examples = ($debugSymbolFiles | Select-Object -First 5 | ForEach-Object {
        Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $_.FullName
    }) -join ", "
    throw "debug symbol files are not allowed for release client package evidence. Count=$($debugSymbolFiles.Count). Examples: $examples"
}

$manifestFiles = @()
foreach ($file in $packageFiles) {
    $relativePath = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $file.FullName
    $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    $entry = [ordered]@{
        name = $relativePath
        sha256 = $hash
        size = [int64]$file.Length
    }
    Assert-LauncherManifestFile -FileEntry $entry
    $manifestFiles += $entry
}

$manifest = [ordered]@{
    version = $Version
    downloadUrl = $DownloadUrl
    files = $manifestFiles
}

$manifestPath = Join-Path $resolvedEvidenceDir ("launcher-manifest-{0}.json" -f $RunId)
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

$installSmoke = [ordered]@{
    mode = if ($CopyInstallSmoke) { "copy-and-verify-all-files" } else { "manifest-and-hash-contract-only" }
    path = $resolvedInstallSmokeRoot
    copiedFiles = 0
    versionFile = $false
}

if ($CopyInstallSmoke) {
    Write-Step "copying and verifying launcher install smoke: $resolvedInstallSmokeRoot"
    Copy-InstallSmokeFiles -SourceRoot $resolvedPackageRoot -DestinationRoot $resolvedInstallSmokeRoot -ManifestFiles $manifestFiles -Version $Version
    $installSmoke.copiedFiles = $manifestFiles.Count
    $installSmoke.versionFile = (Test-Path -LiteralPath (Join-Path $resolvedInstallSmokeRoot "version.txt"))
}

$totalBytes = ($packageFiles | Measure-Object -Property Length -Sum).Sum
$downloadUrlUri = Resolve-ReleaseUri -Url $DownloadUrl
$downloadUrlHasHost = $null -ne $downloadUrlUri
$downloadUrlIsHttps = $downloadUrlHasHost -and $downloadUrlUri.Scheme -eq "https"
$downloadUrlIsExample = $DownloadUrl -like "https://cdn.example.com/*"
$releaseReady = $BuildConfiguration -eq "Shipping" -and $debugSymbolFiles.Count -eq 0 -and $downloadUrlHasHost -and $downloadUrlIsHttps -and -not $downloadUrlIsExample
$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "client-package-launcher"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    packageRoot = $resolvedPackageRoot
    version = $Version
    buildConfiguration = $BuildConfiguration
    downloadUrl = $DownloadUrl
    fileCount = $manifestFiles.Count
    totalBytes = [int64]$totalBytes
    clientExecutable = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $clientExe.FullName
    contentContainerCount = $contentContainers.Count
    debugSymbolCount = $debugSymbolFiles.Count
    downloadUrlHasHost = $downloadUrlHasHost
    downloadUrlIsHttps = $downloadUrlIsHttps
    downloadUrlIsExample = $downloadUrlIsExample
    releaseReady = $releaseReady
    releaseReadinessNotes = @(
        if ($BuildConfiguration -ne "Shipping") { "BuildConfiguration is $BuildConfiguration; production release evidence requires Shipping." }
        if ($debugSymbolFiles.Count -gt 0) { "Debug symbol files are present; production release package should separate symbols from the public client package." }
        if (-not $downloadUrlHasHost) { "DownloadUrl must be a valid absolute URL with a host." }
        if (-not $downloadUrlIsHttps) { "DownloadUrl is not HTTPS." }
        if ($downloadUrlIsExample) { "DownloadUrl is an example CDN URL, not a release CDN URL." }
    )
    launcherManifestPath = $manifestPath
    sha256 = [ordered]@{
        manifest = (Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
        clientExecutable = (Get-FileHash -LiteralPath $clientExe.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    installSmoke = $installSmoke
}

$summaryPath = Join-Path $resolvedEvidenceDir ("client-package-launcher-{0}.json" -f $RunId)
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Step "wrote launcher manifest evidence: $manifestPath"
Write-Step "wrote client package evidence: $summaryPath"
Write-Host "PASS: client package launcher evidence collected" -ForegroundColor Green
