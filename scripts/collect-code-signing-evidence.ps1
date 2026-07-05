<#
Creates production evidence for Windows client package code signing.

Examples:
  .\scripts\collect-code-signing-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release
  .\scripts\collect-code-signing-evidence.ps1 -PackageRoot .tmp\client-release\public\my-release -RequireSigned
#>

[CmdletBinding()]
param(
    [string]$PackageRoot = "",
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string[]]$Extensions = @(".exe", ".dll", ".msi", ".msix", ".appx"),
    [switch]$RequireSigned
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-code-signing-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[code-signing] " + $Message) -ForegroundColor Cyan
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

$normalizedExtensions = @($Extensions | ForEach-Object {
    $extension = $_.Trim()
    if (-not $extension.StartsWith(".")) {
        $extension = "." + $extension
    }
    $extension.ToLowerInvariant()
} | Sort-Object -Unique)

$resolvedPackageRoot = Resolve-ClientPackageRoot -ExplicitPackageRoot $PackageRoot
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

Write-Step "inspecting Authenticode signatures under: $resolvedPackageRoot"
$signableFiles = @(
    Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File |
        Where-Object { $normalizedExtensions -contains $_.Extension.ToLowerInvariant() } |
        Sort-Object FullName
)

if ($signableFiles.Count -eq 0) {
    throw "No signable files were found under package root: $resolvedPackageRoot"
}

$fileEvidence = @()
foreach ($file in $signableFiles) {
    $signature = Get-AuthenticodeSignature -LiteralPath $file.FullName
    $signer = $signature.SignerCertificate
    $timeStamper = $signature.TimeStamperCertificate

    $fileEvidence += [ordered]@{
        path = Get-PortableRelativePath -RootPath $resolvedPackageRoot -FilePath $file.FullName
        sizeBytes = [int64]$file.Length
        sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        status = [string]$signature.Status
        statusMessage = [string]$signature.StatusMessage
        isTrustedValid = ($signature.Status -eq "Valid")
        signerSubject = if ($signer) { $signer.Subject } else { "" }
        signerThumbprint = if ($signer) { $signer.Thumbprint } else { "" }
        signerNotAfterUtc = if ($signer) { $signer.NotAfter.ToUniversalTime().ToString("o") } else { "" }
        timeStamperSubject = if ($timeStamper) { $timeStamper.Subject } else { "" }
        timeStamperThumbprint = if ($timeStamper) { $timeStamper.Thumbprint } else { "" }
    }
}

$unsignedFiles = @($fileEvidence | Where-Object { $_.status -eq "NotSigned" })
$trustedSignedFiles = @($fileEvidence | Where-Object { $_.isTrustedValid })
$invalidSignedFiles = @($fileEvidence | Where-Object { $_.status -ne "Valid" -and $_.status -ne "NotSigned" })
$signingReady = $signableFiles.Count -gt 0 -and
    $unsignedFiles.Count -eq 0 -and
    $invalidSignedFiles.Count -eq 0 -and
    $trustedSignedFiles.Count -eq $signableFiles.Count

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "code-signing"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    packageRoot = $resolvedPackageRoot
    extensions = $normalizedExtensions
    signableFileCount = $signableFiles.Count
    signedFileCount = @($fileEvidence | Where-Object { $_.status -ne "NotSigned" }).Count
    trustedSignedFileCount = $trustedSignedFiles.Count
    unsignedFileCount = $unsignedFiles.Count
    invalidSignedFileCount = $invalidSignedFiles.Count
    signingReady = $signingReady
    signingReadinessNotes = @(
        if ($unsignedFiles.Count -gt 0) { "Unsigned files are present; production client release requires trusted Authenticode signatures." }
        if ($invalidSignedFiles.Count -gt 0) { "One or more signatures are invalid or untrusted." }
        if ($trustedSignedFiles.Count -ne $signableFiles.Count) { "Not every signable file has a trusted valid signature." }
    )
    files = $fileEvidence
}

$summaryPath = Join-Path $resolvedEvidenceDir ("code-signing-{0}.json" -f $RunId)
$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8

Write-Step "wrote code signing evidence: $summaryPath"

if ($RequireSigned -and -not $signingReady) {
    throw "Code signing evidence is not release-ready. Unsigned=$($unsignedFiles.Count), Invalid=$($invalidSignedFiles.Count), TrustedValid=$($trustedSignedFiles.Count)/$($signableFiles.Count)."
}

Write-Host "PASS: code signing evidence collected" -ForegroundColor Green
