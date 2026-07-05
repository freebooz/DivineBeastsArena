<#
Read-only diagnostic for local UE online validation prerequisites.
Checks:
  - Docker CLI / daemon / compose config
  - backend health endpoints
  - UE editor path / uproject path / optional packaged server path
  - local compose container states
#>

[CmdletBinding()]
param(
    [string[]]$BaseUrls = @("http://localhost:8080", "http://localhost:8083"),
    [string]$UnrealRoot = "D:\UnrealEngine-5.8.0-release",
    [string]$ProjectPath = "",
    [switch]$UsePackagedServer
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$backendRoot = Join-Path -Path $repoRoot -ChildPath "DBA_GameBackend"
$projectPathResolved = if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject"
}
else {
    $ProjectPath
}

$results = New-Object System.Collections.Generic.List[object]
$issues = New-Object System.Collections.Generic.List[string]
$reachableBackendCount = 0

function Add-CheckResult {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][ValidateSet("PASS", "WARN", "FAIL")][string]$Status,
        [Parameter(Mandatory = $true)][string]$Detail
    )

    $results.Add([pscustomobject]@{
        Name = $Name
        Status = $Status
        Detail = $Detail
    })

    $color = switch ($Status) {
        "PASS" { "Green" }
        "WARN" { "Yellow" }
        default { "Red" }
    }

    Write-Host ("[{0}] {1}: {2}" -f $Status, $Name, $Detail) -ForegroundColor $color

    if ($Status -eq "FAIL") {
        $issues.Add(("{0}: {1}" -f $Name, $Detail))
    }
}

function Test-HttpUrl {
    param([Parameter(Mandatory = $true)][string]$Url)

    try {
        $response = Invoke-WebRequest -Uri $Url -Method Get -TimeoutSec 3 -UseBasicParsing
        return [pscustomobject]@{
            Reachable = $true
            StatusCode = [int]$response.StatusCode
            Message = ("HTTP {0}" -f $response.StatusCode)
        }
    }
    catch {
        return [pscustomobject]@{
            Reachable = $false
            StatusCode = 0
            Message = $_.Exception.Message
        }
    }
}

function Read-SimpleEnvFile {
    param([Parameter(Mandatory = $true)][string]$Path)

    $map = @{}
    if (-not (Test-Path -LiteralPath $Path)) {
        return $map
    }

    foreach ($line in Get-Content -LiteralPath $Path -Encoding UTF8) {
        if ([string]::IsNullOrWhiteSpace($line) -or $line.TrimStart().StartsWith("#")) {
            continue
        }

        $split = $line.Split("=", 2)
        if ($split.Count -ne 2) {
            continue
        }

        $map[$split[0].Trim()] = $split[1].Trim()
    }

    return $map
}

function Test-DockerComposeConfig {
    param(
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [Parameter(Mandatory = $true)][string]$EnvFile
    )

    try {
        Push-Location $WorkingDirectory
        try {
            $output = & docker compose --env-file $EnvFile config 2>&1
            return [pscustomobject]@{
                Success = ($LASTEXITCODE -eq 0)
                Output = ($output -join "`n")
            }
        }
        finally {
            Pop-Location
        }
    }
    catch {
        return [pscustomobject]@{
            Success = $false
            Output = $_.Exception.Message
        }
    }
}

Write-Host ""
Write-Host "==> Local UE online readiness diagnostic" -ForegroundColor Cyan

$envFile = Join-Path $backendRoot ".env"
$envExampleFile = Join-Path $backendRoot ".env.example"
$activeEnvFile = if (Test-Path -LiteralPath $envFile) { ".env" } else { ".env.example" }
$activeEnvPath = if (Test-Path -LiteralPath $envFile) { $envFile } else { $envExampleFile }

if (Test-Path -LiteralPath $envFile) {
    Add-CheckResult "backend env file" "PASS" ".env is present and will be used by local compose."
}
else {
    Add-CheckResult "backend env file" "WARN" ".env is missing; diagnostics are falling back to .env.example."
}

$envMap = Read-SimpleEnvFile -Path $activeEnvPath
$requiredKeys = @("POSTGRES_PASSWORD", "REDIS_PASSWORD", "JWT_SECRET", "INTERNAL_API_KEY")
foreach ($key in $requiredKeys) {
    if (-not $envMap.ContainsKey($key)) {
        Add-CheckResult ("env:{0}" -f $key) "FAIL" ("{0} is missing {1}." -f $activeEnvFile, $key)
        continue
    }

    $value = $envMap[$key]
    if ([string]::IsNullOrWhiteSpace($value) -or $value -like "change-me*") {
        Add-CheckResult ("env:{0}" -f $key) "WARN" ("{0} still contains a placeholder value for {1}." -f $activeEnvFile, $key)
    }
    else {
        Add-CheckResult ("env:{0}" -f $key) "PASS" ("{0} contains a non-placeholder value for {1}." -f $activeEnvFile, $key)
    }
}

$serverMode = if ($envMap.ContainsKey("GAME_SERVER_MODE")) { $envMap["GAME_SERVER_MODE"] } else { "LocalProcess" }
$aspNetEnvironment = if ($envMap.ContainsKey("ASPNETCORE_ENVIRONMENT")) { $envMap["ASPNETCORE_ENVIRONMENT"] } else { "Production" }
$allowMockAllocation = if ($envMap.ContainsKey("GAME_SERVER_ALLOW_MOCK_ALLOCATION")) { $envMap["GAME_SERVER_ALLOW_MOCK_ALLOCATION"] } else { "false" }
$ueServerHostDir = if ($envMap.ContainsKey("UE_SERVER_HOST_DIR")) { $envMap["UE_SERVER_HOST_DIR"] } else { ".\game-server" }
$ueServerExecutablePath = if ($envMap.ContainsKey("UE_SERVER_EXECUTABLE_PATH")) { $envMap["UE_SERVER_EXECUTABLE_PATH"] } else { "/opt/game-server/GameServer.sh" }

if ($aspNetEnvironment -eq "Production" -and $serverMode -eq "LocalProcess") {
    $hostServerDir = if ([System.IO.Path]::IsPathRooted($ueServerHostDir)) {
        $ueServerHostDir
    }
    else {
        Join-Path $backendRoot $ueServerHostDir
    }
    $expectedHostServerFile = Join-Path $hostServerDir (Split-Path -Leaf $ueServerExecutablePath)

    if (-not (Test-Path -LiteralPath $expectedHostServerFile)) {
        Add-CheckResult "backend server executable" "WARN" ("Production LocalProcess expects a packaged server file mounted at {0}; use scripts\start-local-backend-compose.ps1 for local Development mock allocation, or provide a packaged server for production-like validation." -f $expectedHostServerFile)
    }
    else {
        Add-CheckResult "backend server executable" "PASS" ("Packaged server file exists for Production LocalProcess: {0}" -f $expectedHostServerFile)
    }
}
elseif ($allowMockAllocation -eq "true") {
    Add-CheckResult "backend server executable" "WARN" "Mock server allocation is enabled; this is acceptable only for local validation."
}

$dockerCmd = Get-Command docker -ErrorAction SilentlyContinue
if (-not $dockerCmd) {
    Add-CheckResult "docker cli" "FAIL" "docker command was not found."
}
else {
    Add-CheckResult "docker cli" "PASS" ("docker command found at {0}" -f $dockerCmd.Source)

    $dockerVersion = & docker version --format "{{.Server.Version}}" 2>$null
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($dockerVersion)) {
        Add-CheckResult "docker daemon" "FAIL" "Docker daemon is unavailable or not running."
    }
    else {
        Add-CheckResult "docker daemon" "PASS" ("Docker daemon is reachable. Server.Version={0}" -f $dockerVersion)

        $composeConfig = Test-DockerComposeConfig -WorkingDirectory $backendRoot -EnvFile $activeEnvFile
        if ($composeConfig.Success) {
            Add-CheckResult "docker compose config" "PASS" ("docker compose --env-file {0} config succeeded." -f $activeEnvFile)
        }
        else {
            Add-CheckResult "docker compose config" "FAIL" ("Compose config failed: {0}" -f $composeConfig.Output)
        }

        $composePs = $null
        $composePsSucceeded = $false
        try {
            Push-Location $backendRoot
            try {
                $composePs = & docker compose --env-file $activeEnvFile ps -a --format json 2>$null
                $composePsSucceeded = ($LASTEXITCODE -eq 0)
            }
            finally {
                Pop-Location
            }
        }
        catch {
            $composePsSucceeded = $false
        }

        if ($composePsSucceeded -and $composePs) {
            $services = $composePs | ConvertFrom-Json
            foreach ($service in @($services)) {
                $level = if ($service.State -eq "running" -and ($service.Health -eq "" -or $service.Health -eq "healthy")) { "PASS" } else { "WARN" }
                Add-CheckResult ("compose:{0}" -f $service.Service) $level ("State={0}; Health={1}" -f $service.State, $service.Health)
            }
        }
        else {
            Add-CheckResult "docker compose ps" "WARN" "Compose service state could not be read; containers may not exist yet."
        }

        $namedContainers = & docker ps -a --format '{{.Names}}|{{.Status}}'
        if ($LASTEXITCODE -eq 0 -and $namedContainers) {
            foreach ($line in $namedContainers) {
                if ($line -match '^divine-beasts-arena-(?<name>.+?)\|(?<status>.+)$') {
                    $status = $Matches["status"]
                    $containerName = $Matches["name"]
                    $level = if ($status -match '^Up ') { "PASS" } else { "WARN" }
                    Add-CheckResult ("container:{0}" -f $containerName) $level $status
                }
            }
        }
    }
}

foreach ($baseUrl in $BaseUrls) {
    $live = Test-HttpUrl -Url ($baseUrl.TrimEnd("/") + "/health/live")
    if ($live.Reachable) {
        $reachableBackendCount++
        Add-CheckResult ("backend live:{0}" -f $baseUrl) "PASS" $live.Message
    }
    else {
        Add-CheckResult ("backend live:{0}" -f $baseUrl) "WARN" $live.Message
    }

    $ready = Test-HttpUrl -Url ($baseUrl.TrimEnd("/") + "/health/ready")
    if ($ready.Reachable) {
        Add-CheckResult ("backend ready:{0}" -f $baseUrl) "PASS" $ready.Message
    }
    else {
        Add-CheckResult ("backend ready:{0}" -f $baseUrl) "WARN" $ready.Message
    }
}

if ($reachableBackendCount -eq 0) {
    Add-CheckResult "backend health summary" "FAIL" "No configured /health/live endpoint is reachable. Local UE online validation is blocked until backend services are started."
}
else {
    Add-CheckResult "backend health summary" "PASS" ("Reachable backend live endpoints: {0}" -f $reachableBackendCount)
}

$editorExe = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor.exe"
if (Test-Path -LiteralPath $editorExe) {
    Add-CheckResult "UnrealEditor.exe" "PASS" $editorExe
}
else {
    Add-CheckResult "UnrealEditor.exe" "FAIL" ("UnrealEditor.exe was not found: {0}" -f $editorExe)
}

if (Test-Path -LiteralPath $projectPathResolved) {
    Add-CheckResult "uproject path" "PASS" $projectPathResolved
}
else {
    Add-CheckResult "uproject path" "FAIL" ("uproject was not found: {0}" -f $projectPathResolved)
}

if ($UsePackagedServer) {
    $packagedServer = Join-Path (Split-Path -Parent $projectPathResolved) "Binaries\Win64\DivineBeastsArenaServer.exe"
    if (Test-Path -LiteralPath $packagedServer) {
        Add-CheckResult "packaged server" "PASS" $packagedServer
    }
    else {
        Add-CheckResult "packaged server" "WARN" ("Packaged dedicated server was not found: {0}" -f $packagedServer)
    }
}

Write-Host ""
if ($issues.Count -gt 0) {
    Write-Host "Diagnostic result: blocking FAIL items were found." -ForegroundColor Red
    foreach ($issue in $issues) {
        Write-Host ("- {0}" -f $issue) -ForegroundColor Red
    }
    exit 1
}

Write-Host "Diagnostic result: no blocking FAIL items were found." -ForegroundColor Green
