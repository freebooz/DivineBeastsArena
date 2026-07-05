<#
Read-only diagnostics for a self-hosted Windows client release evidence runner.

This script checks repository workflow files, release scripts, public package
inputs, CDN URL policy, launcher build tools, and optional signing tooling before
running .github/workflows/client-release-evidence.yml.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PackageRoot,
    [Parameter(Mandatory = $true)][string]$DownloadUrl,
    [Parameter(Mandatory = $true)][string]$ManifestUrl,
    [string]$JsonOutputPath = "",
    [switch]$SkipSigningProbe,
    [string]$CertificateThumbprint = "",
    [string]$CertificateSubject = "",
    [string]$PfxPath = "",
    [string]$PfxPasswordEnvironmentVariable = "DBA_CODE_SIGNING_PFX_PASSWORD",
    [string]$SignToolPath = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$checks = [System.Collections.Generic.List[object]]::new()

function Add-Check {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Status,
        [string]$Detail = ""
    )

    $checks.Add([ordered]@{
        name = $Name
        status = $Status
        detail = $Detail
    }) | Out-Null

    $color = switch ($Status) {
        "PASS" { "Green" }
        "WARN" { "Yellow" }
        default { "Red" }
    }
    Write-Host ("{0} {1} - {2}" -f $Status, $Name, $Detail) -ForegroundColor $color
}

function Test-PathCheck {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Path
    )

    if (Test-Path -LiteralPath $Path) {
        Add-Check $Name "PASS" $Path
    }
    else {
        Add-Check $Name "FAIL" $Path
    }
}

function Test-CommandCheck {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string[]]$Commands
    )

    foreach ($commandName in $Commands) {
        $command = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($command) {
            Add-Check $Name "PASS" $command.Source
            return
        }
    }

    Add-Check $Name "FAIL" ("Missing command: {0}" -f ($Commands -join ", "))
}

function Resolve-SignToolCandidate {
    param([string]$ExplicitPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath)) {
        if (Test-Path -LiteralPath $ExplicitPath) {
            return (Resolve-Path -LiteralPath $ExplicitPath).ProviderPath
        }
        return ""
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

    return ""
}

$isWindows = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows)
Add-Check "runner.os.windows" ($(if ($isWindows) { "PASS" } else { "FAIL" })) ([System.Runtime.InteropServices.RuntimeInformation]::OSDescription)

Test-PathCheck "repo.workflow.client-release-evidence" (Join-Path $repoRoot ".github\workflows\client-release-evidence.yml")
Test-PathCheck "repo.script.validate-production-evidence-contracts" (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1")
Test-PathCheck "repo.script.diagnose-client-release-prerequisites" (Join-Path $repoRoot "scripts\diagnose-client-release-prerequisites.ps1")
Test-PathCheck "repo.script.run-client-release-evidence" (Join-Path $repoRoot "scripts\run-client-release-evidence.ps1")
Test-PathCheck "repo.script.collect-production-evidence" (Join-Path $repoRoot "scripts\collect-production-evidence.ps1")
Test-PathCheck "repo.script.write-release-readiness-report" (Join-Path $repoRoot "scripts\write-release-readiness-report.ps1")
Test-PathCheck "repo.launcher.package_json" (Join-Path $repoRoot "DBA_GameLauncher\package.json")
Test-PathCheck "repo.launcher.cargo_manifest" (Join-Path $repoRoot "DBA_GameLauncher\src-tauri\Cargo.toml")

Test-CommandCheck "tool.node" @("node")
Test-CommandCheck "tool.npm" @("npm")
Test-CommandCheck "tool.cargo" @("cargo")

if (Test-Path -LiteralPath $PackageRoot) {
    $resolvedPackageRoot = (Resolve-Path -LiteralPath $PackageRoot).ProviderPath
    Add-Check "client.package_root" "PASS" $resolvedPackageRoot
    $clientExe = Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File -Filter "DivineBeastsArena.exe" -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($clientExe) {
        Add-Check "client.package_exe" "PASS" $clientExe.FullName
    }
    else {
        Add-Check "client.package_exe" "FAIL" "DivineBeastsArena.exe was not found under PackageRoot"
    }

    $cookedContainers = @(Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File -Include "*.pak", "*.utoc", "*.ucas" -ErrorAction SilentlyContinue)
    if ($cookedContainers.Count -gt 0) {
        Add-Check "client.cooked_content" "PASS" ("containerCount={0}" -f $cookedContainers.Count)
    }
    else {
        Add-Check "client.cooked_content" "FAIL" "No .pak/.utoc/.ucas cooked content containers were found"
    }
}
else {
    Add-Check "client.package_root" "FAIL" $PackageRoot
}

$prerequisiteOutputPath = ""
if (-not [string]::IsNullOrWhiteSpace($JsonOutputPath)) {
    $diagnosticDir = Split-Path -Parent $JsonOutputPath
    if ($diagnosticDir) {
        $prerequisiteOutputPath = Join-Path $diagnosticDir "client-release-runner-prerequisites.json"
    }
}
if ([string]::IsNullOrWhiteSpace($prerequisiteOutputPath)) {
    $prerequisiteOutputPath = Join-Path $repoRoot ".tmp\client-release-runner-diagnostic\client-release-runner-prerequisites.json"
}

try {
    & (Join-Path $repoRoot "scripts\diagnose-client-release-prerequisites.ps1") `
        -PackageRoot $PackageRoot `
        -DownloadUrl $DownloadUrl `
        -ManifestUrl $ManifestUrl `
        -RequireManifestUrl `
        -OutputJsonPath $prerequisiteOutputPath `
        -FailOnBlockingIssues | Out-Host
    Add-Check "release.prerequisites" "PASS" $prerequisiteOutputPath
}
catch {
    Add-Check "release.prerequisites" "FAIL" $_.Exception.Message
}

if ($SkipSigningProbe) {
    Add-Check "signing.probe" "WARN" "Skipped by -SkipSigningProbe"
}
else {
    $identityModes = @(
        -not [string]::IsNullOrWhiteSpace($CertificateThumbprint),
        -not [string]::IsNullOrWhiteSpace($CertificateSubject),
        -not [string]::IsNullOrWhiteSpace($PfxPath)
    ) | Where-Object { $_ }

    if ($identityModes.Count -ne 1) {
        Add-Check "signing.identity" "FAIL" "Pass exactly one signing identity: CertificateThumbprint, CertificateSubject, or PfxPath."
    }
    else {
        Add-Check "signing.identity" "PASS" "one signing identity supplied"
    }

    $signTool = Resolve-SignToolCandidate -ExplicitPath $SignToolPath
    if ([string]::IsNullOrWhiteSpace($signTool)) {
        Add-Check "signing.signtool" "FAIL" "signtool.exe was not found"
    }
    else {
        Add-Check "signing.signtool" "PASS" $signTool
    }

    if (-not [string]::IsNullOrWhiteSpace($PfxPath)) {
        if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($PfxPasswordEnvironmentVariable))) {
            Add-Check "signing.pfx_password" "FAIL" "Missing environment variable: $PfxPasswordEnvironmentVariable"
        }
        else {
            Add-Check "signing.pfx_password" "PASS" $PfxPasswordEnvironmentVariable
        }
    }
}

try {
    & (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1") | Out-Host
    Add-Check "evidence.contracts" "PASS" "validate-production-evidence-contracts.ps1"
}
catch {
    Add-Check "evidence.contracts" "FAIL" $_.Exception.Message
}

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "client-release-runner-diagnostic"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    repoRoot = $repoRoot
    packageRoot = $PackageRoot
    downloadUrl = $DownloadUrl
    manifestUrl = $ManifestUrl
    checks = $checks.ToArray()
}

if (-not [string]::IsNullOrWhiteSpace($JsonOutputPath)) {
    $outputDirectory = Split-Path -Parent $JsonOutputPath
    if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
        New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $JsonOutputPath -Encoding UTF8
    Write-Host ("Diagnostic JSON written: {0}" -f $JsonOutputPath)
}

$failed = @($checks.ToArray() | Where-Object { $_.status -eq "FAIL" })
if ($failed.Count -gt 0) {
    Write-Host ("SUMMARY: {0} client release runner prerequisite check(s) failed." -f $failed.Count) -ForegroundColor Red
    exit 1
}

Write-Host "SUMMARY: Client release runner prerequisites passed." -ForegroundColor Green
exit 0
