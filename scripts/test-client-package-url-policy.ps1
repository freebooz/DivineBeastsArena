<#
Exercises client package launcher evidence URL readiness against temporary fixtures.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$scriptPath = Join-Path $repoRoot "scripts\collect-client-package-evidence.ps1"
$fixtureRoot = Join-Path $repoRoot (".tmp\client-package-url-policy-tests-{0}" -f [guid]::NewGuid().ToString("N"))
$packageRoot = Join-Path $fixtureRoot "package"
$evidenceDir = Join-Path $fixtureRoot "evidence"

function New-FixturePackage {
    param([Parameter(Mandatory = $true)][string]$Root)

    if (Test-Path -LiteralPath $Root) {
        Remove-Item -LiteralPath $Root -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path (Join-Path $Root "Content\Paks") | Out-Null
    Set-Content -LiteralPath (Join-Path $Root "DivineBeastsArena.exe") -Value "fixture exe" -Encoding ASCII
    Set-Content -LiteralPath (Join-Path $Root "Content\Paks\DivineBeastsArena-Windows.pak") -Value "fixture pak" -Encoding ASCII
}

function Read-Json {
    param([Parameter(Mandatory = $true)][string]$Path)
    Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

New-FixturePackage -Root $packageRoot
New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null

$runId = "hostless-url"
& $scriptPath `
    -PackageRoot $packageRoot `
    -EvidenceDir $evidenceDir `
    -RunId $runId `
    -Version "1.0.0.0" `
    -BuildConfiguration Shipping `
    -DownloadUrl "https://" `
    -DisallowDebugSymbols | Out-Null

$report = Read-Json -Path (Join-Path $evidenceDir "client-package-launcher-$runId.json")
if ($report.releaseReady -ne $false) {
    throw "Expected hostless DownloadUrl to prevent releaseReady client package evidence."
}
if ($report.downloadUrlHasHost -ne $false) {
    throw "Expected hostless DownloadUrl to set downloadUrlHasHost=false."
}
if ($report.downloadUrlIsHttps -ne $false) {
    throw "Expected hostless DownloadUrl not to count as HTTPS release URL."
}
if (($report.releaseReadinessNotes -join "`n") -notmatch "valid absolute URL with a host") {
    throw "Expected hostless DownloadUrl readiness note to mention valid absolute URL with a host."
}

Write-Host "PASS: client package URL policy fixtures"
