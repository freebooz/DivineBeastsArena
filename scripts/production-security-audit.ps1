<#
Runs local production security checks that mirror the repository security-ci scope.

Examples:
  .\scripts\production-security-audit.ps1 -SkipContainerScan
  .\scripts\production-security-audit.ps1 -UseDockerizedTrivy
  .\scripts\production-security-audit.ps1 -EvidenceDir .\Artifacts\ProductionEvidence\security
  .\scripts\production-security-audit.ps1
#>

[CmdletBinding()]
param(
    [switch]$SkipNuGet,
    [switch]$SkipNpm,
    [switch]$SkipContainerScan,
    [switch]$UseDockerizedTrivy,
    [string]$TrivyCommand = "trivy",
    [string]$TrivyImage = "aquasec/trivy:latest",
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\security"),
    [string]$RunId
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$resolvedEvidenceDir = $null
$failures = New-Object System.Collections.Generic.List[string]

if (-not $RunId) {
    $RunId = "local-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if (-not (Test-Path -LiteralPath $EvidenceDir)) {
    New-Item -ItemType Directory -Path $EvidenceDir -Force | Out-Null
}
$resolvedEvidenceDir = (Resolve-Path -LiteralPath $EvidenceDir).Path

function Get-EvidencePath {
    param([Parameter(Mandatory = $true)][string]$FileName)

    return Join-Path $resolvedEvidenceDir $FileName
}

function Invoke-AuditCheck {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Script
    )

    Write-Host ""
    Write-Host "==> $Name" -ForegroundColor Cyan
    try {
        & $Script
        Write-Host "PASS: $Name" -ForegroundColor Green
    }
    catch {
        $message = $_.Exception.Message
        $failures.Add("${Name}: $message")
        Write-Host "FAIL: $Name" -ForegroundColor Red
        Write-Host $message -ForegroundColor Red
    }
}

function Invoke-CommandInDir {
    param(
        [Parameter(Mandatory = $true)][string]$Directory,
        [Parameter(Mandatory = $true)][string]$Command,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )

    Push-Location $Directory
    try {
        & $Command @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "$Command $($Arguments -join ' ') exited with code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Invoke-TrivyImageScan {
    param(
        [Parameter(Mandatory = $true)][string]$ImageName,
        [Parameter(Mandatory = $true)][string]$EvidencePath
    )

    if ($UseDockerizedTrivy) {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw "docker not found. Install Docker or rerun without -UseDockerizedTrivy using local Trivy."
        }

        $evidenceDirectory = Split-Path -Parent $EvidencePath
        $evidenceFileName = Split-Path -Leaf $EvidencePath
        & docker run --rm `
            -v /var/run/docker.sock:/var/run/docker.sock `
            -v "${evidenceDirectory}:/evidence" `
            $TrivyImage image --format sarif --output "/evidence/${evidenceFileName}" --scanners vuln --severity CRITICAL,HIGH --exit-code 1 $ImageName
        if ($LASTEXITCODE -ne 0) {
            throw "dockerized Trivy scan exited with code $LASTEXITCODE for $ImageName"
        }
        return
    }

    if (-not (Get-Command $TrivyCommand -ErrorAction SilentlyContinue)) {
        throw "$TrivyCommand not found. Install Trivy, rerun with -UseDockerizedTrivy, or rerun with -SkipContainerScan."
    }

    & $TrivyCommand image --format sarif --output $EvidencePath --scanners vuln --severity CRITICAL,HIGH --exit-code 1 $ImageName
    if ($LASTEXITCODE -ne 0) {
        throw "$TrivyCommand image scan exited with code $LASTEXITCODE for $ImageName"
    }
}

function Test-NpmProductionAudit {
    param(
        [Parameter(Mandatory = $true)][string]$AppPath,
        [Parameter(Mandatory = $true)][string]$AppName,
        [Parameter(Mandatory = $true)][string]$EvidencePath
    )

    Push-Location $AppPath
    try {
        $npmCommand = Get-Command npm.cmd -ErrorAction SilentlyContinue
        if (-not $npmCommand) {
            $npmCommand = Get-Command npm -ErrorAction SilentlyContinue
        }
        if (-not $npmCommand) {
            throw "npm not found. Install Node.js or rerun with -SkipNpm."
        }

        Write-Host ("npm audit path: {0}" -f (Get-Location).Path)
        Write-Host ("npm executable: {0}" -f $npmCommand.Source)
        $auditJson = & $npmCommand.Source audit --omit=dev --audit-level=high --json --offline=false
        $auditExitCode = $LASTEXITCODE

        $auditText = $auditJson -join [Environment]::NewLine
        $auditText | Set-Content -LiteralPath $EvidencePath -Encoding UTF8
        Write-Host ("npm production audit evidence ({0}): {1}" -f $AppName, $EvidencePath)

        $audit = $auditText | ConvertFrom-Json
        if (-not $audit.metadata -or -not $audit.metadata.vulnerabilities) {
            throw "npm audit did not return vulnerability metadata. Raw response: $auditText"
        }

        $high = [int]$audit.metadata.vulnerabilities.high
        $critical = [int]$audit.metadata.vulnerabilities.critical
        Write-Host ("npm production audit summary: high={0} critical={1}" -f $high, $critical)
        if (($high + $critical) -gt 0) {
            if ($audit.vulnerabilities) {
                $audit.vulnerabilities.PSObject.Properties |
                    Sort-Object Name |
                    ForEach-Object {
                        Write-Host ("- {0}: severity={1}, direct={2}" -f $_.Name, $_.Value.severity, $_.Value.isDirect) -ForegroundColor Yellow
                    }
            }
            throw "npm production audit found high=$high critical=$critical in $AppPath"
        }

        if ($auditExitCode -ne 0) {
            Write-Host ("npm audit returned code {0}, but JSON high/critical counts are clean; treating as pass." -f $auditExitCode) -ForegroundColor Yellow
        }

        Write-Host ("npm production audit clean: {0}" -f $AppPath)
    }
    finally {
        Pop-Location
    }
}

if (-not $SkipNuGet) {
    Invoke-AuditCheck "NuGet vulnerable package audit" {
        if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) {
            throw "dotnet not found. Install .NET SDK or rerun with -SkipNuGet."
        }

        $backendDir = Join-Path $repoRoot "DBA_GameBackend"
        Push-Location $backendDir
        try {
            $output = dotnet list GameBackend.sln package --vulnerable --include-transitive 2>&1
            $nugetEvidencePath = Get-EvidencePath "vulnerability-report.txt"
            ($output -join [Environment]::NewLine) | Set-Content -LiteralPath $nugetEvidencePath -Encoding UTF8
            Write-Host ("NuGet vulnerability evidence: {0}" -f $nugetEvidencePath)
            $output | ForEach-Object { Write-Host $_ }
            if ($LASTEXITCODE -ne 0) {
                throw "dotnet list package exited with code $LASTEXITCODE"
            }

            $joined = ($output | Out-String)
            if ($joined -match "has the following vulnerable packages|critical|high|moderate|low") {
                throw "Vulnerable NuGet packages detected."
            }
        }
        finally {
            Pop-Location
        }
    }
}

if (-not $SkipNpm) {
    Invoke-AuditCheck "Admin npm production audit" {
        Test-NpmProductionAudit `
            -AppPath (Join-Path $repoRoot "DBA_GameAdmin") `
            -AppName "admin" `
            -EvidencePath (Get-EvidencePath "npm-audit-admin-${RunId}.json")
    }

    Invoke-AuditCheck "Website npm production audit" {
        Test-NpmProductionAudit `
            -AppPath (Join-Path $repoRoot "DBA_GameWebsite") `
            -AppName "website" `
            -EvidencePath (Get-EvidencePath "npm-audit-website-${RunId}.json")
    }

    Invoke-AuditCheck "Launcher npm production audit" {
        Test-NpmProductionAudit `
            -AppPath (Join-Path $repoRoot "DBA_GameLauncher") `
            -AppName "launcher" `
            -EvidencePath (Get-EvidencePath "npm-audit-launcher-${RunId}.json")
    }
}

if (-not $SkipContainerScan) {
    Invoke-AuditCheck "API container Trivy scan" {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw "docker not found. Install Docker or rerun with -SkipContainerScan."
        }

        $backendDir = Join-Path $repoRoot "DBA_GameBackend"
        Invoke-CommandInDir $backendDir "docker" @("build", "-f", "Game.Api/Dockerfile", "-t", "dba-game-api:local-security", ".")
        Invoke-TrivyImageScan `
            -ImageName "dba-game-api:local-security" `
            -EvidencePath (Get-EvidencePath "trivy-api-${RunId}.sarif")
    }

    Invoke-AuditCheck "Worker container Trivy scan" {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw "docker not found. Install Docker or rerun with -SkipContainerScan."
        }

        $backendDir = Join-Path $repoRoot "DBA_GameBackend"
        Invoke-CommandInDir $backendDir "docker" @("build", "-f", "Game.Worker/Dockerfile", "-t", "dba-game-worker:local-security", ".")
        Invoke-TrivyImageScan `
            -ImageName "dba-game-worker:local-security" `
            -EvidencePath (Get-EvidencePath "trivy-worker-${RunId}.sarif")
    }
}

Write-Host ""
if ($failures.Count -gt 0) {
    Write-Host "Production security audit failed:" -ForegroundColor Red
    foreach ($failure in $failures) {
        Write-Host "- $failure" -ForegroundColor Red
    }
    exit 1
}

Write-Host "Production security audit passed." -ForegroundColor Green
