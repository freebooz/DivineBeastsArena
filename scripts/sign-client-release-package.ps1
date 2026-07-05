<#
Signs a public Windows client release package and writes code-signing evidence.

Examples:
  .\scripts\sign-client-release-package.ps1 -PackageRoot .\.tmp\client-release\public\prod -CertificateThumbprint "<thumbprint>" -RequireSigned
  .\scripts\sign-client-release-package.ps1 -PackageRoot .\.tmp\client-release\public\prod -PfxPath .\certs\release.pfx -PfxPasswordEnvironmentVariable DBA_CODE_SIGNING_PFX_PASSWORD -RequireSigned
#>

[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [Parameter(Mandatory = $true)][string]$PackageRoot,
    [string]$EvidenceDir = "",
    [string]$RunId = "",
    [string]$SignToolPath = "",
    [string]$CertificateThumbprint = "",
    [string]$CertificateSubject = "",
    [string]$PfxPath = "",
    [string]$PfxPasswordEnvironmentVariable = "DBA_CODE_SIGNING_PFX_PASSWORD",
    [string]$TimestampUrl = "http://timestamp.digicert.com",
    [switch]$RequireSigned,
    [switch]$SkipEvidence
)

$ErrorActionPreference = "Stop"
$signingWhatIf = $WhatIfPreference
$WhatIfPreference = $false
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$resolvedPackageRoot = Resolve-Path $PackageRoot

if ([string]::IsNullOrWhiteSpace($EvidenceDir)) {
    $EvidenceDir = Join-Path $repoRoot "Artifacts\ProductionEvidence\client"
}

$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "sign-client-release-package-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Resolve-SignTool {
    param([string]$ExplicitPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath)) {
        if (-not (Test-Path -LiteralPath $ExplicitPath)) {
            throw "SignToolPath does not exist: $ExplicitPath"
        }
        return (Resolve-Path $ExplicitPath).Path
    }

    $command = Get-Command "signtool.exe" -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $kitRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    if (Test-Path -LiteralPath $kitRoot) {
        $candidate = Get-ChildItem -Path $kitRoot -Recurse -Filter "signtool.exe" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\signtool\.exe$" } |
            Sort-Object FullName -Descending |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }

    throw "signtool.exe not found. Install Windows SDK or pass -SignToolPath."
}

function Get-SignableClientFiles {
    param([string]$Root)

    $extensions = @(".exe", ".dll", ".msi", ".msix", ".appx")
    Get-ChildItem -LiteralPath $Root -Recurse -File |
        Where-Object { $extensions -contains $_.Extension.ToLowerInvariant() } |
        Sort-Object FullName
}

function Get-RelativePathFromRoot {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [Parameter(Mandatory = $true)][string]$Path
    )

    $rootPath = [System.IO.Path]::GetFullPath($Root).TrimEnd("\", "/") + [System.IO.Path]::DirectorySeparatorChar
    $targetPath = [System.IO.Path]::GetFullPath($Path)
    $rootUri = New-Object System.Uri($rootPath)
    $targetUri = New-Object System.Uri($targetPath)
    return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($targetUri).ToString()).Replace("/", "\")
}

function Get-CertificateArguments {
    param(
        [string]$Thumbprint,
        [string]$Subject,
        [string]$Pfx,
        [string]$PfxPasswordEnv
    )

    $modes = @(
        -not [string]::IsNullOrWhiteSpace($Thumbprint),
        -not [string]::IsNullOrWhiteSpace($Subject),
        -not [string]::IsNullOrWhiteSpace($Pfx)
    ) | Where-Object { $_ }

    if ($modes.Count -ne 1) {
        throw "Pass exactly one signing identity: -CertificateThumbprint, -CertificateSubject, or -PfxPath."
    }

    if (-not [string]::IsNullOrWhiteSpace($Thumbprint)) {
        return @("/sha1", ($Thumbprint -replace "\s", ""))
    }

    if (-not [string]::IsNullOrWhiteSpace($Subject)) {
        return @("/n", $Subject)
    }

    $resolvedPfx = Resolve-Path $Pfx
    $password = [Environment]::GetEnvironmentVariable($PfxPasswordEnv)
    if ([string]::IsNullOrWhiteSpace($password)) {
        throw "PFX password environment variable is not set: $PfxPasswordEnv"
    }

    return @("/f", $resolvedPfx.Path, "/p", $password)
}

$signTool = Resolve-SignTool -ExplicitPath $SignToolPath
$certificateArgs = Get-CertificateArguments `
    -Thumbprint $CertificateThumbprint `
    -Subject $CertificateSubject `
    -Pfx $PfxPath `
    -PfxPasswordEnv $PfxPasswordEnvironmentVariable
$signableFiles = @(Get-SignableClientFiles -Root $resolvedPackageRoot.Path)

if ($signableFiles.Count -eq 0) {
    throw "No signable client files found under $($resolvedPackageRoot.Path)."
}

Write-Host "Sign tool: $signTool"
Write-Host "Signable file count: $($signableFiles.Count)"

foreach ($file in $signableFiles) {
    $relativePath = Get-RelativePathFromRoot -Root $resolvedPackageRoot.Path -Path $file.FullName
    $args = @(
        "sign",
        "/fd", "SHA256",
        "/td", "SHA256",
        "/tr", $TimestampUrl
    ) + $certificateArgs + @($file.FullName)

    if ($signingWhatIf) {
        Write-Host ("What if: Performing the operation `"sign-client-release-package`" on target `"{0}`"." -f $relativePath)
        continue
    }

    if (-not $PSCmdlet.ShouldProcess($relativePath, "sign-client-release-package")) {
        continue
    }

    & $signTool @args
    if ($LASTEXITCODE -ne 0) {
        throw "signtool.exe failed for $relativePath with exit code $LASTEXITCODE"
    }
}

if (-not $SkipEvidence) {
    $evidenceArgs = @{
        PackageRoot = $resolvedPackageRoot.Path
        EvidenceDir = $resolvedEvidenceDir
        RunId = $RunId
    }

    if ($RequireSigned) {
        $evidenceArgs.RequireSigned = $true
    }

    & (Join-Path $repoRoot "scripts\collect-code-signing-evidence.ps1") @evidenceArgs
}
