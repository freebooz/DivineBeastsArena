<#
Prepares a public Windows client release package by separating debug symbols.

Input is a staged Unreal Windows client package. Output:
- public-client-package: all runtime files except debug symbols
- symbols-package: .pdb/.dbg/.dSYM files preserving relative paths
- manifest-ready JSON summary with SHA256 hashes
#>

[CmdletBinding()]
param(
    [string]$StagedPackageRoot = "",
    [string]$PublicPackageRoot = "",
    [string]$SymbolsPackageRoot = "",
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-client-release-package-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[client-release] " + $Message) -ForegroundColor Cyan
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

function Resolve-StagedPackageRoot {
    param([string]$ExplicitRoot)

    $candidates = @()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitRoot)) {
        $candidates += $ExplicitRoot
    }
    $candidates += @(
        (Join-Path $repoRoot ".tmp\packaged-client-shipping-fixed\Windows"),
        (Join-Path $repoRoot ".tmp\packaged-client-shipping\Windows"),
        (Join-Path $repoRoot ".tmp\packaged-server\Windows"),
        (Join-Path $repoRoot "DBA_GameClient\Saved\StagedBuilds\Windows")
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

    throw "No staged Windows client package with DivineBeastsArena.exe was found. Pass -StagedPackageRoot after packaging the client."
}

function Copy-RelativeFile {
    param(
        [Parameter(Mandatory = $true)][string]$SourceRoot,
        [Parameter(Mandatory = $true)][string]$SourceFile,
        [Parameter(Mandatory = $true)][string]$DestinationRoot
    )

    $relativePath = Get-PortableRelativePath -RootPath $SourceRoot -FilePath $SourceFile
    $destination = Join-Path $DestinationRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
    $parent = Split-Path -Parent $destination
    if (-not (Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }
    Copy-Item -LiteralPath $SourceFile -Destination $destination -Force
    return $relativePath
}

function New-HashEntries {
    param(
        [Parameter(Mandatory = $true)][string]$RootPath,
        [Parameter(Mandatory = $true)][object[]]$Files
    )

    $entries = @()
    foreach ($file in $Files) {
        $entries += [ordered]@{
            path = Get-PortableRelativePath -RootPath $RootPath -FilePath $file.FullName
            sizeBytes = [int64]$file.Length
            sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        }
    }
    return $entries
}

$resolvedStagedRoot = Resolve-StagedPackageRoot -ExplicitRoot $StagedPackageRoot
if ([string]::IsNullOrWhiteSpace($PublicPackageRoot)) {
    $PublicPackageRoot = Join-Path $repoRoot ".tmp\client-release\public\$RunId"
}
if ([string]::IsNullOrWhiteSpace($SymbolsPackageRoot)) {
    $SymbolsPackageRoot = Join-Path $repoRoot ".tmp\client-release\symbols\$RunId"
}

$resolvedPublicRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($PublicPackageRoot)
$resolvedSymbolsRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($SymbolsPackageRoot)
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)

foreach ($path in @($resolvedPublicRoot, $resolvedSymbolsRoot)) {
    if (Test-Path -LiteralPath $path) {
        Remove-Item -LiteralPath $path -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $path | Out-Null
}
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

$stagedFiles = @(Get-ChildItem -LiteralPath $resolvedStagedRoot -Recurse -File | Sort-Object FullName)
if ($stagedFiles.Count -eq 0) {
    throw "staged client package contains no files: $resolvedStagedRoot"
}

$debugSymbolExtensions = @(".pdb", ".dbg", ".dSYM")
$debugSymbolFiles = @($stagedFiles | Where-Object { $_.Extension -in $debugSymbolExtensions })
$runtimeFiles = @($stagedFiles | Where-Object { $_.Extension -notin $debugSymbolExtensions })

if ($runtimeFiles.Count -eq 0) {
    throw "staged client package contains no runtime files after symbol separation: $resolvedStagedRoot"
}

Write-Step "creating public-client-package: $resolvedPublicRoot"
foreach ($file in $runtimeFiles) {
    $null = Copy-RelativeFile -SourceRoot $resolvedStagedRoot -SourceFile $file.FullName -DestinationRoot $resolvedPublicRoot
}

Write-Step "creating symbols-package: $resolvedSymbolsRoot"
foreach ($file in $debugSymbolFiles) {
    $null = Copy-RelativeFile -SourceRoot $resolvedStagedRoot -SourceFile $file.FullName -DestinationRoot $resolvedSymbolsRoot
}

$publicFiles = @(Get-ChildItem -LiteralPath $resolvedPublicRoot -Recurse -File | Sort-Object FullName)
$symbolFiles = @(Get-ChildItem -LiteralPath $resolvedSymbolsRoot -Recurse -File -ErrorAction SilentlyContinue | Sort-Object FullName)
$publicDebugSymbols = @($publicFiles | Where-Object { $_.Extension -in $debugSymbolExtensions })
if ($publicDebugSymbols.Count -gt 0) {
    throw "public releasePackage still contains debug symbols: $($publicDebugSymbols[0].FullName)"
}

$publicClientExe = Get-ChildItem -LiteralPath $resolvedPublicRoot -Recurse -File -Filter "DivineBeastsArena.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $publicClientExe) {
    throw "public releasePackage does not contain DivineBeastsArena.exe: $resolvedPublicRoot"
}

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "client-release-package"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    stagedPackageRoot = $resolvedStagedRoot
    publicClientPackageRoot = $resolvedPublicRoot
    symbolsPackageRoot = $resolvedSymbolsRoot
    releasePackage = [ordered]@{
        fileCount = $publicFiles.Count
        totalBytes = [int64](($publicFiles | Measure-Object -Property Length -Sum).Sum)
        clientExecutable = Get-PortableRelativePath -RootPath $resolvedPublicRoot -FilePath $publicClientExe.FullName
        debugSymbolCount = $publicDebugSymbols.Count
        sha256 = New-HashEntries -RootPath $resolvedPublicRoot -Files $publicFiles
    }
    symbolsPackage = [ordered]@{
        fileCount = $symbolFiles.Count
        totalBytes = [int64](($symbolFiles | Measure-Object -Property Length -Sum).Sum)
        debugSymbolCount = $debugSymbolFiles.Count
        sha256 = New-HashEntries -RootPath $resolvedSymbolsRoot -Files $symbolFiles
    }
}

$summaryPath = Join-Path $resolvedEvidenceDir ("client-release-package-{0}.json" -f $RunId)
$summary | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Step "wrote release package evidence: $summaryPath"
Write-Host "PASS: public client release package and symbols package prepared" -ForegroundColor Green
