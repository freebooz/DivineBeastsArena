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
    [switch]$SkipUnrealAutomation,
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$UnrealProjectPath = "",
    [string]$UnrealAutomationTests = "DivineBeastsArena.Combat.PlayableSkillCatalog"
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

function Resolve-UnrealTool {
    param(
        [Parameter(Mandatory = $true)][string]$ToolName,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $tool = Get-Command $ToolName -ErrorAction SilentlyContinue
    if ($tool) {
        return $tool.Source
    }

    $candidateRoots = @()
    if (-not [string]::IsNullOrWhiteSpace($UnrealRoot)) {
        $candidateRoots += $UnrealRoot
    }
    $candidateRoots += "E:\UnrealEngine-5.7.1-release"

    foreach ($root in ($candidateRoots | Select-Object -Unique)) {
        foreach ($relativePath in $RelativePaths) {
            $candidate = Join-Path $root $relativePath
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    throw "$ToolName not found. Set UNREAL_ENGINE_ROOT or pass -UnrealRoot."
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
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameBackend") "dotnet" @("test", "GameBackend.sln", "--configuration", "Release")
}

Invoke-Check "admin angular build" {
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameAdmin") "npm" @("ci")
    Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameAdmin") "npm" @("run", "build")
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
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Write-Host "UnrealBuildTool: $ubt"
    }

    Invoke-Check "Unreal editor target build" {
        $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
            Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
        }
        else {
            Resolve-Path $UnrealProjectPath
        }
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $ubt @(
            "DivineBeastsArenaEditor",
            "Win64",
            "Development",
            "-Project=$projectPath",
            "-WaitMutex",
            "-NoHotReloadFromIDE"
        )
    }

    Invoke-Check "Unreal dedicated server target build" {
        $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
            Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
        }
        else {
            Resolve-Path $UnrealProjectPath
        }
        $ubt = Resolve-UnrealTool "UnrealBuildTool" @(
            "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe",
            "Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe"
        )
        Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $ubt @(
            "DivineBeastsArenaServer",
            "Win64",
            "Development",
            "-Project=$projectPath",
            "-WaitMutex",
            "-NoHotReloadFromIDE"
        )
    }

    if (-not $SkipUnrealAutomation) {
        Invoke-Check "Unreal automation tests" {
            $projectPath = if ([string]::IsNullOrWhiteSpace($UnrealProjectPath)) {
                Resolve-Path (Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject")
            }
            else {
                Resolve-Path $UnrealProjectPath
            }
            $editorCmd = Resolve-UnrealTool "UnrealEditor-Cmd" @("Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
            Invoke-CommandInDir (Join-Path $repoRoot "DBA_GameClient") $editorCmd @(
                "$projectPath",
                "-ExecCmds=Automation RunTests $UnrealAutomationTests; Quit",
                "-unattended",
                "-nop4",
                "-nosplash",
                "-NullRHI",
                "-log"
            )
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
