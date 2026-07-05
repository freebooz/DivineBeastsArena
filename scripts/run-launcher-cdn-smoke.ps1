<#
Runs a launcher-style CDN smoke test from a manifest URL.

The smoke downloads every manifest file into a clean install directory, verifies
SHA256, writes version.txt, and emits manifest-ready evidence. Local HTTP is
allowed only when -AllowLocalHttp is explicit; release-ready evidence requires
HTTPS and non-example CDN URLs.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$ManifestUrl,
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$InstallRoot = "",
    [int]$TimeoutSec = 60,
    [switch]$AllowLocalHttp
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-launcher-cdn-smoke-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if ([string]::IsNullOrWhiteSpace($InstallRoot)) {
    $InstallRoot = Join-Path $repoRoot ".tmp\launcher-cdn-smoke\$RunId"
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[launcher-cdn] " + $Message) -ForegroundColor Cyan
}

function Test-LocalHttpUrl {
    param([Parameter(Mandatory = $true)][uri]$Uri)

    if ($Uri.Scheme -ne "http") {
        return $false
    }
    return $Uri.Host -in @("localhost", "127.0.0.1", "::1")
}

function Resolve-SmokeUri {
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

function Assert-SmokeUrlPolicy {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $uri = Resolve-SmokeUri -Url $Url -Name $Name
    if ($uri.Scheme -eq "https") {
        return
    }
    if ($AllowLocalHttp -and (Test-LocalHttpUrl -Uri $uri)) {
        return
    }
    throw "$Name must be HTTPS unless -AllowLocalHttp is used for localhost smoke: $Url"
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

    if ([string]::IsNullOrWhiteSpace($RelativePath)) {
        throw "manifest file name is empty"
    }
    $normalized = $RelativePath.Replace("\", "/")
    if ($normalized.StartsWith("/") -or $normalized.Contains("../")) {
        throw "manifest file path is unsafe: $RelativePath"
    }
    return Join-Path $BasePath ($normalized -replace "/", [System.IO.Path]::DirectorySeparatorChar)
}

function Get-FileDownloadUrl {
    param(
        [Parameter(Mandatory = $true)][string]$BaseDownloadUrl,
        [Parameter(Mandatory = $true)][string]$FileName,
        [Parameter(Mandatory = $true)][int]$TotalFiles,
        [Parameter(Mandatory = $true)][int]$Index
    )

    $trimmed = $BaseDownloadUrl.Trim()
    if ([string]::IsNullOrWhiteSpace($trimmed)) {
        throw "manifest downloadUrl is empty"
    }
    if ($trimmed.EndsWith("/")) {
        return "$trimmed$FileName"
    }
    if ($TotalFiles -eq 1 -and $Index -eq 0) {
        return $trimmed
    }
    throw "manifest downloadUrl must end with '/' for multi-file smoke: $trimmed"
}

function Assert-ManifestFile {
    param([Parameter(Mandatory = $true)]$File)

    $null = Join-SafeRelativePath -BasePath "." -RelativePath ([string]$File.name)
    if ([int64]$File.size -le 0) {
        throw "manifest file size must be greater than zero: $($File.name)"
    }
    if ([string]$File.sha256 -notmatch "^[0-9a-fA-F]{64}$") {
        throw "manifest file SHA256 is invalid: $($File.name)"
    }
}

Assert-SmokeUrlPolicy -Url $ManifestUrl -Name "ManifestUrl"
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)
$resolvedInstallRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($InstallRoot)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null
if (Test-Path -LiteralPath $resolvedInstallRoot) {
    Remove-Item -LiteralPath $resolvedInstallRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $resolvedInstallRoot | Out-Null

Write-Step "fetching launcher manifest: $ManifestUrl"
$webClient = New-Object System.Net.WebClient
$webClient.Encoding = [System.Text.Encoding]::UTF8
$manifestContent = $webClient.DownloadString($ManifestUrl)
$manifest = $manifestContent | ConvertFrom-Json
if (-not $manifest.version) {
    throw "launcher manifest is missing version"
}
if (-not $manifest.downloadUrl) {
    throw "launcher manifest is missing downloadUrl"
}
if (-not $manifest.files -or $manifest.files.Count -eq 0) {
    throw "launcher manifest has no files"
}
Assert-SmokeUrlPolicy -Url ([string]$manifest.downloadUrl) -Name "downloadUrl"

$downloadUrlIsHttps = ([string]$manifest.downloadUrl).StartsWith("https://", [System.StringComparison]::OrdinalIgnoreCase)
$manifestUrlIsHttps = $ManifestUrl.StartsWith("https://", [System.StringComparison]::OrdinalIgnoreCase)
$downloadUrlIsExample = [string]$manifest.downloadUrl -like "https://cdn.example.com/*"
$manifestUrlIsExample = $ManifestUrl -like "https://cdn.example.com/*"

$files = @($manifest.files)
$downloadedFiles = @()
for ($index = 0; $index -lt $files.Count; $index++) {
    $file = $files[$index]
    Assert-ManifestFile -File $file
    $fileUrl = Get-FileDownloadUrl -BaseDownloadUrl ([string]$manifest.downloadUrl) -FileName ([string]$file.name) -TotalFiles $files.Count -Index $index
    Assert-SmokeUrlPolicy -Url $fileUrl -Name "file download URL"

    $destination = Join-SafeRelativePath -BasePath $resolvedInstallRoot -RelativePath ([string]$file.name)
    $parent = Split-Path -Parent $destination
    if (-not (Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    Write-Step "downloading $($file.name)"
    Invoke-WebRequest -Uri $fileUrl -OutFile $destination -TimeoutSec $TimeoutSec | Out-Null
    $hash = (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -ne ([string]$file.sha256).ToLowerInvariant()) {
        throw "downloaded file SHA256 mismatch: $($file.name)"
    }
    $actualSize = (Get-Item -LiteralPath $destination).Length
    if ($actualSize -ne [int64]$file.size) {
        throw "downloaded file size mismatch: $($file.name). Expected=$($file.size) Actual=$actualSize"
    }

    $downloadedFiles += [ordered]@{
        name = [string]$file.name
        size = [int64]$actualSize
        sha256 = $hash
    }
}

Set-Content -LiteralPath (Join-Path $resolvedInstallRoot "version.txt") -Value ([string]$manifest.version).Trim() -Encoding UTF8
$versionFileHash = (Get-FileHash -LiteralPath (Join-Path $resolvedInstallRoot "version.txt") -Algorithm SHA256).Hash.ToLowerInvariant()
$cdnReady = $manifestUrlIsHttps -and $downloadUrlIsHttps -and -not $manifestUrlIsExample -and -not $downloadUrlIsExample

$evidence = [ordered]@{
    schemaVersion = "1.0"
    kind = "launcher-cdn-smoke"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    ManifestUrl = $ManifestUrl
    version = [string]$manifest.version
    downloadUrl = [string]$manifest.downloadUrl
    installRoot = $resolvedInstallRoot
    downloadedFileCount = $downloadedFiles.Count
    downloadedFiles = $downloadedFiles
    versionFile = [ordered]@{
        path = Get-PortableRelativePath -RootPath $resolvedInstallRoot -FilePath (Join-Path $resolvedInstallRoot "version.txt")
        sha256 = $versionFileHash
    }
    AllowLocalHttp = [bool]$AllowLocalHttp
    manifestUrlIsHttps = $manifestUrlIsHttps
    downloadUrlIsHttps = $downloadUrlIsHttps
    manifestUrlIsExample = $manifestUrlIsExample
    downloadUrlIsExample = $downloadUrlIsExample
    cdnReady = $cdnReady
}

$evidencePath = Join-Path $resolvedEvidenceDir ("launcher-cdn-smoke-{0}.json" -f $RunId)
$evidence | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
Write-Step "wrote launcher CDN smoke evidence: $evidencePath"
Write-Host "PASS: launcher CDN smoke completed" -ForegroundColor Green
