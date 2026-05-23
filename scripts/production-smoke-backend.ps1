<#
Smoke test for a running Game.Api environment.
Example:
  .\scripts\production-smoke-backend.ps1 -BaseUrl "https://api.example.com"
#>

[CmdletBinding()]
param(
    [string]$BaseUrl = "http://localhost:8080",
    [switch]$GuestLogin
)

$ErrorActionPreference = "Stop"
$BaseUrl = $BaseUrl.TrimEnd("/")

function Invoke-Smoke {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Script
    )

    Write-Host "==> $Name" -ForegroundColor Cyan
    & $Script
    Write-Host "PASS: $Name" -ForegroundColor Green
}

Invoke-Smoke "live health" {
    Invoke-RestMethod -Method Get -Uri "$BaseUrl/health/live" | Out-Null
}

Invoke-Smoke "ready health" {
    Invoke-RestMethod -Method Get -Uri "$BaseUrl/health/ready" | Out-Null
}

Invoke-Smoke "version api" {
    Invoke-RestMethod -Method Get -Uri "$BaseUrl/api/version" | Out-Null
}

Invoke-Smoke "launcher manifest" {
    $manifest = Invoke-RestMethod -Method Get -Uri "$BaseUrl/launcher/manifest.json?channel=stable&platform=Windows"
    if (-not $manifest.version) {
        throw "launcher manifest is missing version"
    }
}

Invoke-Smoke "metrics endpoint" {
    $metrics = Invoke-WebRequest -Method Get -Uri "$BaseUrl/metrics"
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
        $result = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/auth/guest-login" -ContentType "application/json" -Body $body
        if (-not $result.success) {
            throw "guest login returned success=false"
        }
    }
}

Write-Host ""
Write-Host "Backend smoke test passed: $BaseUrl" -ForegroundColor Green
