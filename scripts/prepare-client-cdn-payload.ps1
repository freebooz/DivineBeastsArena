<#
Prepares a client release payload directory for CDN upload.

The script copies a public Windows client package into a clean payload root,
writes launcher-manifest.json using the final CDN download URL, and writes a
cdn-upload-manifest evidence file with SHA256 and size for every payload file.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PackageRoot,
    [string]$PayloadRoot = "",
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$Version = "0.1.0.0",
    [Parameter(Mandatory = $true)][string]$DownloadUrl,
    [string]$ManifestUrl = "",
    [switch]$AllowLocalHttp
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "client-cdn-payload-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if ([string]::IsNullOrWhiteSpace($PayloadRoot)) {
    $PayloadRoot = Join-Path $repoRoot ".tmp\client-cdn-payload\$RunId"
}

if ([string]::IsNullOrWhiteSpace($ManifestUrl)) {
    $ManifestUrl = $DownloadUrl.TrimEnd("/") + "/launcher-manifest.json"
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[client-cdn-payload] " + $Message) -ForegroundColor Cyan
}

function Test-LocalHttpUrl {
    param([Parameter(Mandatory = $true)][uri]$Uri)

    if ($Uri.Scheme -ne "http") {
        return $false
    }
    return $Uri.Host -in @("localhost", "127.0.0.1", "::1")
}

function Resolve-ReleaseUri {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $uri = $null
    if (-not [System.Uri]::TryCreate($Url, [System.UriKind]::Absolute, [ref]$uri) -or [string]::IsNullOrWhiteSpace($uri.Host)) {
        throw "$Name must be a valid absolute URL with a host: $Url"
    }

    return $uri
}

function Assert-ReleaseUrlPolicy {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $uri = Resolve-ReleaseUri -Url $Url -Name $Name
    if ($uri.Scheme -eq "https") {
        return
    }
    if ($AllowLocalHttp -and (Test-LocalHttpUrl -Uri $uri)) {
        return
    }

    throw "$Name must be HTTPS unless -AllowLocalHttp is used for localhost payload validation: $Url"
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

function Join-SafeRelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$RelativePath
    )

    $normalized = $RelativePath.Replace("\", "/")
    if ([string]::IsNullOrWhiteSpace($normalized) -or $normalized.StartsWith("/") -or $normalized.Contains("../")) {
        throw "unsafe relative payload path: $RelativePath"
    }

    return Join-Path $BasePath ($normalized -replace "/", [System.IO.Path]::DirectorySeparatorChar)
}

function Should-ExcludeGeneratedReleaseMetadata {
    param([Parameter(Mandatory = $true)][string]$RelativePath)

    $fileName = [System.IO.Path]::GetFileName($RelativePath)
    return $fileName -like "launcher-manifest*.json" -or
        $fileName -like "cdn-upload-manifest*.json" -or
        $fileName -like "client-release-evidence*.json" -or
        $fileName -like "client-package-launcher*.json" -or
        $fileName -like "code-signing*.json" -or
        $fileName -like "launcher-cdn-smoke*.json" -or
        $fileName -like "launcher-install-update-smoke*.json"
}

$resolvedPackageRoot = (Resolve-Path -LiteralPath $PackageRoot).ProviderPath
$resolvedPayloadRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($PayloadRoot)
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)

Assert-ReleaseUrlPolicy -Url $DownloadUrl -Name "DownloadUrl"
Assert-ReleaseUrlPolicy -Url $ManifestUrl -Name "ManifestUrl"

if (-not $DownloadUrl.TrimEnd("/").EndsWith($Version, [System.StringComparison]::OrdinalIgnoreCase)) {
    Write-Host "WARN: DownloadUrl does not end with the Version value. Verify CDN layout intentionally separates version path." -ForegroundColor Yellow
}

$packageFiles = @(
    Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File |
        Where-Object {
            $relativePath = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $_.FullName
            -not (Should-ExcludeGeneratedReleaseMetadata -RelativePath $relativePath)
        } |
        Sort-Object FullName
)
if ($packageFiles.Count -eq 0) {
    throw "client package contains no files: $resolvedPackageRoot"
}

$clientExe = @($packageFiles | Where-Object { $_.Name -eq "DivineBeastsArena.exe" })[0]
if (-not $clientExe) {
    throw "DivineBeastsArena.exe was not found in client package: $resolvedPackageRoot"
}

Write-Step "preparing clean payload root: $resolvedPayloadRoot"
if (Test-Path -LiteralPath $resolvedPayloadRoot) {
    Remove-Item -LiteralPath $resolvedPayloadRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $resolvedPayloadRoot | Out-Null
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

$manifestFiles = @()
foreach ($file in $packageFiles) {
    $relativePath = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $file.FullName
    $destination = Join-SafeRelativePath -BasePath $resolvedPayloadRoot -RelativePath $relativePath
    $parent = Split-Path -Parent $destination
    if (-not (Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    Copy-Item -LiteralPath $file.FullName -Destination $destination -Force
    $copied = Get-Item -LiteralPath $destination
    $hash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
    $manifestFiles += [ordered]@{
        name = $relativePath
        sha256 = $hash
        size = [int64]$copied.Length
    }
}

$launcherManifest = [ordered]@{
    version = $Version
    downloadUrl = $DownloadUrl
    files = $manifestFiles
}
$launcherManifestPath = Join-Path $resolvedPayloadRoot "launcher-manifest.json"
$launcherManifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $launcherManifestPath -Encoding UTF8

$payloadFiles = @(
    Get-ChildItem -LiteralPath $resolvedPayloadRoot -Recurse -File |
        Sort-Object FullName
)
$payloadIndex = @()
foreach ($file in $payloadFiles) {
    $payloadIndex += [ordered]@{
        path = Get-PortableRelativePath -RootPath $resolvedPayloadRoot -FilePath $file.FullName
        size = [int64]$file.Length
        sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
}

$downloadUrlIsHttps = $DownloadUrl.StartsWith("https://", [System.StringComparison]::OrdinalIgnoreCase)
$manifestUrlIsHttps = $ManifestUrl.StartsWith("https://", [System.StringComparison]::OrdinalIgnoreCase)
$downloadUrlIsExample = $DownloadUrl -like "https://cdn.example.com/*"
$manifestUrlIsExample = $ManifestUrl -like "https://cdn.example.com/*"
$payloadReady = $downloadUrlIsHttps -and $manifestUrlIsHttps -and -not $downloadUrlIsExample -and -not $manifestUrlIsExample

$uploadManifest = [ordered]@{
    schemaVersion = "1.0"
    kind = "cdn-upload-manifest"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    packageRoot = $resolvedPackageRoot
    payloadRoot = $resolvedPayloadRoot
    version = $Version
    DownloadUrl = $DownloadUrl
    ManifestUrl = $ManifestUrl
    launcherManifestPath = $launcherManifestPath
    payloadFileCount = $payloadIndex.Count
    payloadTotalBytes = [int64](($payloadFiles | Measure-Object -Property Length -Sum).Sum)
    payloadFiles = $payloadIndex
    clientExecutable = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $clientExe.FullName
    downloadUrlIsHttps = $downloadUrlIsHttps
    manifestUrlIsHttps = $manifestUrlIsHttps
    downloadUrlIsExample = $downloadUrlIsExample
    manifestUrlIsExample = $manifestUrlIsExample
    AllowLocalHttp = [bool]$AllowLocalHttp
    payloadReady = $payloadReady
    readinessNotes = @(
        if (-not $downloadUrlIsHttps) { "DownloadUrl is not HTTPS." }
        if (-not $manifestUrlIsHttps) { "ManifestUrl is not HTTPS." }
        if ($downloadUrlIsExample) { "DownloadUrl is an example CDN URL." }
        if ($manifestUrlIsExample) { "ManifestUrl is an example CDN URL." }
    )
}

$uploadManifestPath = Join-Path $resolvedEvidenceDir ("cdn-upload-manifest-{0}.json" -f $RunId)
$uploadManifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $uploadManifestPath -Encoding UTF8

Write-Step "wrote launcher manifest: $launcherManifestPath"
Write-Step "wrote cdn-upload-manifest evidence: $uploadManifestPath"
Write-Host "PASS: client CDN payload prepared" -ForegroundColor Green
