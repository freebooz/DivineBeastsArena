<#
Production preflight for the DivineBeastsArena monorepo.
Run from the repository root:
  .\scripts\production-preflight.ps1
#>

[CmdletBinding()]
param(
    [switch]$SkipNode,
    [switch]$SkipDocker,
    [switch]$SkipCargo,
    [switch]$SkipUnreal,
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$failures = New-Object System.Collections.Generic.List[string]

function Invoke-Check {
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

Set-Location $repoRoot

Invoke-Check "git workspace" {
    $status = git status --short
    if ($status) {
        Write-Host $status
        Write-Host "Note: dirty workspace is allowed for preflight, but production release should be committed or cleaned." -ForegroundColor Yellow
    }
}

Invoke-Check "backend dotnet test" {
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "dotnet" @("test", "GameBackend.sln")
}

Invoke-Check "admin dotnet build" {
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameAdmin") "dotnet" @("build")
}

if (-not $SkipNode) {
    Invoke-Check "website npm build" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameWebsite") "npm" @("install")
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameWebsite") "npm" @("run", "build")
    }

    Invoke-Check "launcher npm build" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "npm" @("install")
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "npm" @("run", "build")
    }
}

if (-not $SkipCargo) {
    Invoke-Check "launcher cargo check" {
        if (-not (Get-Command cargo -ErrorAction SilentlyContinue)) {
            throw "cargo not found. Install Rust or rerun with -SkipCargo."
        }
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameLauncher") "cargo" @("check", "--manifest-path", "src-tauri/Cargo.toml")
    }
}

if (-not $SkipDocker) {
    Invoke-Check "backend docker compose config" {
        if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
            throw "docker not found. Install Docker or rerun with -SkipDocker."
        }
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "docker" @("compose", "--env-file", ".env.example", "config")
    }

    Invoke-Check "observability docker compose config" {
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "docker" @("compose", "--env-file", ".env.example", "-f", "docker-compose.observability.yml", "config")
    }
}

if (-not $SkipUnreal) {
    Invoke-Check "UnrealBuildTool availability" {
        $ubt = Get-Command UnrealBuildTool -ErrorAction SilentlyContinue
        if ($ubt) {
            Write-Host "UnrealBuildTool: $($ubt.Source)"
            return
        }

        $candidateRoots = @()
        if (-not [string]::IsNullOrWhiteSpace($UnrealRoot)) {
            $candidateRoots += $UnrealRoot
        }
        $candidateRoots += "E:\UnrealEngine-5.7.1-release"

        foreach ($root in $candidateRoots) {
            $candidatePaths = @(
                (Join-Path $root "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"),
                (Join-Path $root "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe")
            )

            foreach ($candidate in $candidatePaths) {
                if (Test-Path $candidate) {
                    Write-Host "UnrealBuildTool: $candidate"
                    return
                }
            }
        }

        if (-not $ubt) {
            throw "UnrealBuildTool not found. This machine cannot verify UE C++ compilation."
        }
    }
}

Write-Host ""
if ($failures.Count -gt 0) {
    Write-Host "Production preflight failed:" -ForegroundColor Red
    foreach ($failure in $failures) {
        Write-Host "- $failure" -ForegroundColor Red
    }
    exit 1
}

Write-Host "Production preflight passed." -ForegroundColor Green
