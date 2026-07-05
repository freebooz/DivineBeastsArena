<#
Starts the local backend stack for UE online validation.

This keeps production validation strict while using Development overrides for
local compose runs where the UE dedicated server is launched by
start-local-ue-validation.ps1 instead of by the API container.
#>

[CmdletBinding()]
param(
    [string]$EnvFile = ".env",
    [string]$BaseUrl = "http://localhost:8080",
    [int]$TimeoutSec = 90,
    [switch]$SkipBuild,
    [switch]$SkipMigrations,
    [switch]$SkipWorker
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$backendRoot = Join-Path -Path $repoRoot -ChildPath "DBA_GameBackend"
$envPath = Join-Path -Path $backendRoot -ChildPath $EnvFile

if (-not (Test-Path -LiteralPath $envPath)) {
    throw "Backend env file was not found: $envPath"
}

function Set-ScopedEnv {
    param(
        [Parameter(Mandatory = $true)][hashtable]$Snapshot,
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Value
    )

    if (-not $Snapshot.ContainsKey($Name)) {
        $Snapshot[$Name] = [Environment]::GetEnvironmentVariable($Name, "Process")
    }

    [Environment]::SetEnvironmentVariable($Name, $Value, "Process")
}

function Restore-ScopedEnv {
    param([Parameter(Mandatory = $true)][hashtable]$Snapshot)

    foreach ($name in $Snapshot.Keys) {
        [Environment]::SetEnvironmentVariable($name, $Snapshot[$name], "Process")
    }
}

function Wait-BackendLive {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][int]$TimeoutSec
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    $liveUrl = $Url.TrimEnd("/") + "/health/live"
    while ((Get-Date) -lt $deadline) {
        try {
            $response = Invoke-WebRequest -Uri $liveUrl -Method Get -TimeoutSec 3 -UseBasicParsing
            if ($response.StatusCode -ge 200 -and $response.StatusCode -lt 300) {
                return
            }
        }
        catch {
            Start-Sleep -Seconds 2
        }
    }

    throw "Backend did not become live within ${TimeoutSec}s: $liveUrl"
}

$envSnapshot = @{}
try {
    Set-ScopedEnv $envSnapshot "ASPNETCORE_ENVIRONMENT" "Development"
    Set-ScopedEnv $envSnapshot "DOTNET_ENVIRONMENT" "Development"
    Set-ScopedEnv $envSnapshot "GAME_SERVER_MODE" "LocalProcess"
    Set-ScopedEnv $envSnapshot "GAME_SERVER_ALLOW_MOCK_ALLOCATION" "true"
    Set-ScopedEnv $envSnapshot "API_HTTP_BIND" "127.0.0.1"

    Push-Location $backendRoot
    try {
        $dependencyArgs = @("compose", "--env-file", $EnvFile, "up", "-d", "postgres", "redis")
        Write-Host ("[backend-compose] docker {0}" -f ($dependencyArgs -join " "))
        & docker @dependencyArgs
        if ($LASTEXITCODE -ne 0) {
            throw "docker $($dependencyArgs -join ' ') exited with code $LASTEXITCODE"
        }

        if (-not $SkipMigrations) {
            Set-ScopedEnv $envSnapshot "DATABASE_RUN_MIGRATIONS_AND_EXIT" "true"
            $migrationArgs = @("compose", "--env-file", $EnvFile, "run", "--rm")
            if (-not $SkipBuild) {
                $migrationArgs += "--build"
            }
            $migrationArgs += "game-api"

            Write-Host ("[backend-compose] docker {0}" -f ($migrationArgs -join " "))
            & docker @migrationArgs
            if ($LASTEXITCODE -ne 0) {
                throw "docker $($migrationArgs -join ' ') exited with code $LASTEXITCODE"
            }
        }

        Set-ScopedEnv $envSnapshot "DATABASE_RUN_MIGRATIONS_AND_EXIT" "false"

        $services = @("game-api")
        if (-not $SkipWorker) {
            $services += "game-worker"
        }

        $args = @("compose", "--env-file", $EnvFile, "up", "-d")
        if (-not $SkipBuild) {
            $args += "--build"
        }
        $args += $services

        Write-Host ("[backend-compose] docker {0}" -f ($args -join " "))
        & docker @args
        if ($LASTEXITCODE -ne 0) {
            throw "docker $($args -join ' ') exited with code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }

    Wait-BackendLive -Url $BaseUrl -TimeoutSec $TimeoutSec
    Write-Host ("[backend-compose] backend live: {0}" -f $BaseUrl) -ForegroundColor Green
}
finally {
    Restore-ScopedEnv $envSnapshot
}
