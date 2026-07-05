<#
Runs a production smoke test for a running Game.Api environment and writes manifest-ready evidence.

Examples:
  .\scripts\production-smoke-backend.ps1 -BaseUrl "https://api.example.com"
  .\scripts\production-smoke-backend.ps1 -BaseUrl "http://localhost:8080" -EvidenceDir .\Artifacts\ProductionEvidence\ops
#>

[CmdletBinding()]
param(
    [string]$BaseUrl = "http://localhost:8080",
    [switch]$GuestLogin,
    [int]$TimeoutSec = 15,
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\ops"),
    [string]$RunId
)

$ErrorActionPreference = "Stop"
$BaseUrl = $BaseUrl.TrimEnd("/")

if (-not $RunId) {
    $RunId = "local-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if (-not (Test-Path -LiteralPath $EvidenceDir)) {
    New-Item -ItemType Directory -Path $EvidenceDir -Force | Out-Null
}

$resolvedEvidenceDir = (Resolve-Path -LiteralPath $EvidenceDir).Path
$logPath = Join-Path $resolvedEvidenceDir "production-smoke-backend-${RunId}.log"
$summaryPath = Join-Path $resolvedEvidenceDir "production-smoke-backend-${RunId}.json"
$checks = New-Object System.Collections.Generic.List[object]

function Write-SmokeLog {
    param([string]$Message = "")

    $Message | Tee-Object -FilePath $logPath -Append
}

function Write-SmokeEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$Status,
        [Parameter(Mandatory = $true)][int]$ExitCode
    )

    $summary = [ordered]@{
        schemaVersion = "1.0"
        runId = $RunId
        status = $Status
        exitCode = $ExitCode
        generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
        baseUrl = $BaseUrl
        guestLogin = [bool]$GuestLogin
        timeoutSec = $TimeoutSec
        logFile = $logPath
        checks = $checks
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8
}

function Invoke-Smoke {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Script
    )

    $startedAt = Get-Date
    Write-SmokeLog "==> CHECK: $Name"
    try {
        & $Script
        $durationMs = [int]((Get-Date) - $startedAt).TotalMilliseconds
        $checks.Add([ordered]@{
            name = $Name
            status = "passed"
            durationMs = $durationMs
        }) | Out-Null
        Write-SmokeLog "PASS: $Name"
    }
    catch {
        $durationMs = [int]((Get-Date) - $startedAt).TotalMilliseconds
        $checks.Add([ordered]@{
            name = $Name
            status = "failed"
            durationMs = $durationMs
            error = $_.Exception.Message
        }) | Out-Null
        Write-SmokeLog "FAIL: $Name"
        Write-SmokeLog $_.Exception.Message
        throw
    }
}

try {
    Write-SmokeLog "Backend production smoke run: $RunId"
    Write-SmokeLog "BaseUrl: $BaseUrl"
    Write-SmokeLog "Evidence log: $logPath"
    Write-SmokeLog "Evidence summary: $summaryPath"

    Invoke-Smoke "live health" {
        Invoke-RestMethod -Method Get -Uri "$BaseUrl/health/live" -TimeoutSec $TimeoutSec | Out-Null
    }

    Invoke-Smoke "ready health" {
        Invoke-RestMethod -Method Get -Uri "$BaseUrl/health/ready" -TimeoutSec $TimeoutSec | Out-Null
    }

    Invoke-Smoke "version api" {
        Invoke-RestMethod -Method Get -Uri "$BaseUrl/api/version" -TimeoutSec $TimeoutSec | Out-Null
    }

    Invoke-Smoke "launcher manifest" {
        $manifest = Invoke-RestMethod -Method Get -Uri "$BaseUrl/launcher/manifest.json?channel=stable&platform=Windows" -TimeoutSec $TimeoutSec
        if (-not $manifest.version) {
            throw "launcher manifest is missing version"
        }
    }

    Invoke-Smoke "metrics endpoint" {
        $metrics = Invoke-WebRequest -Method Get -Uri "$BaseUrl/metrics" -TimeoutSec $TimeoutSec -UseBasicParsing
        if ($metrics.StatusCode -lt 200 -or $metrics.StatusCode -ge 300) {
            throw "metrics endpoint returned status $($metrics.StatusCode)"
        }
    }

    if ($GuestLogin) {
        Invoke-Smoke "guest login" {
            $body = @{
                deviceId = "smoke-$([Guid]::NewGuid().ToString("N"))"
                deviceName = "production-smoke"
                platform = "Windows"
            } | ConvertTo-Json
            $result = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/auth/guest-login" -ContentType "application/json" -Body $body -TimeoutSec $TimeoutSec
            if (-not $result.success) {
                throw "guest login returned success=false"
            }
        }
    }

    Write-SmokeLog ""
    Write-SmokeLog "Backend smoke test passed: $BaseUrl"
    Write-SmokeEvidence -Status "passed" -ExitCode 0
    Write-SmokeLog "Backend smoke evidence written: $summaryPath"
}
catch {
    Write-SmokeEvidence -Status "failed" -ExitCode 1
    Write-SmokeLog "Backend smoke evidence written: $summaryPath"
    throw
}
