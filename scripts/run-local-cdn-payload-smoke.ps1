<#
Runs a local launcher CDN smoke test against a prepared CDN payload directory.

Use this after prepare-client-cdn-payload.ps1 and before uploading to a real CDN.
The script starts a temporary localhost HTTP server, invokes run-launcher-cdn-smoke.ps1,
then stops the server.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PayloadRoot,
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$InstallRoot = "",
    [int]$Port = 0,
    [int]$StartupTimeoutSec = 15,
    [int]$DownloadTimeoutSec = 60
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "local-cdn-payload-smoke-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[local-cdn-payload-smoke] " + $Message) -ForegroundColor Cyan
}

function Get-FreeTcpPort {
    $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)
    try {
        $listener.Start()
        return ([System.Net.IPEndPoint]$listener.LocalEndpoint).Port
    }
    finally {
        $listener.Stop()
    }
}

function Resolve-Python {
    $python = Get-Command "python" -ErrorAction SilentlyContinue
    if ($python) {
        return $python.Source
    }

    $py = Get-Command "py" -ErrorAction SilentlyContinue
    if ($py) {
        return $py.Source
    }

    throw "python was not found. Install Python 3 or run launcher CDN smoke against an existing HTTP server."
}

function Copy-PayloadForLocalSmoke {
    param(
        [Parameter(Mandatory = $true)][string]$SourceRoot,
        [Parameter(Mandatory = $true)][string]$DestinationRoot,
        [Parameter(Mandatory = $true)][string]$LocalDownloadUrl
    )

    if (Test-Path -LiteralPath $DestinationRoot) {
        Remove-Item -LiteralPath $DestinationRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $DestinationRoot | Out-Null
    Get-ChildItem -LiteralPath $SourceRoot -Force | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $DestinationRoot -Recurse -Force
    }

    $manifestPath = Join-Path $DestinationRoot "launcher-manifest.json"
    $manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath | ConvertFrom-Json
    $manifest.downloadUrl = $LocalDownloadUrl
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
}

$resolvedPayloadRoot = (Resolve-Path -LiteralPath $PayloadRoot).ProviderPath
if (-not (Test-Path -LiteralPath (Join-Path $resolvedPayloadRoot "launcher-manifest.json"))) {
    throw "launcher-manifest.json was not found under PayloadRoot: $resolvedPayloadRoot"
}

if ($Port -le 0) {
    $Port = Get-FreeTcpPort
}

$manifestUrl = "http://127.0.0.1:$Port/launcher-manifest.json"
$localDownloadUrl = "http://127.0.0.1:$Port/"
$servedPayloadRoot = Join-Path $repoRoot ".tmp\local-cdn-payload-smoke\served\$RunId"
Copy-PayloadForLocalSmoke -SourceRoot $resolvedPayloadRoot -DestinationRoot $servedPayloadRoot -LocalDownloadUrl $localDownloadUrl
$python = Resolve-Python
$serverProcess = $null
$serverStdoutPath = Join-Path $repoRoot ".tmp\local-cdn-payload-smoke\server-$RunId.out.log"
$serverStderrPath = Join-Path $repoRoot ".tmp\local-cdn-payload-smoke\server-$RunId.err.log"

try {
    Write-Step "starting temporary HTTP server on $manifestUrl"
    $serverProcess = Start-Process `
        -FilePath $python `
        -ArgumentList @("-m", "http.server", "$Port", "--bind", "127.0.0.1") `
        -WorkingDirectory $servedPayloadRoot `
        -WindowStyle Hidden `
        -RedirectStandardOutput $serverStdoutPath `
        -RedirectStandardError $serverStderrPath `
        -PassThru

    $deadline = (Get-Date).AddSeconds($StartupTimeoutSec)
    $serverReady = $false
    do {
        Start-Sleep -Milliseconds 250
        if ($serverProcess.HasExited) {
            $serverError = if (Test-Path -LiteralPath $serverStderrPath) {
                Get-Content -Raw -Encoding UTF8 -LiteralPath $serverStderrPath
            }
            else {
                ""
            }
            throw "temporary HTTP server stopped unexpectedly with exit code $($serverProcess.ExitCode). $serverError"
        }

        try {
            $response = Invoke-WebRequest -Uri $manifestUrl -UseBasicParsing -TimeoutSec 2
            if ($response.StatusCode -eq 200) {
                $serverReady = $true
                break
            }
        }
        catch {
        }
    } while ((Get-Date) -lt $deadline)

    if (-not $serverReady) {
        throw "temporary HTTP server did not serve launcher-manifest.json within $StartupTimeoutSec seconds"
    }

    $smokeArgs = @{
        ManifestUrl = $manifestUrl
        EvidenceDir = $EvidenceDir
        RunId = $RunId
        TimeoutSec = $DownloadTimeoutSec
        AllowLocalHttp = $true
    }
    if (-not [string]::IsNullOrWhiteSpace($InstallRoot)) {
        $smokeArgs.InstallRoot = $InstallRoot
    }

    & (Join-Path $repoRoot "scripts\run-launcher-cdn-smoke.ps1") @smokeArgs
    Write-Host "PASS: local-cdn-payload-smoke completed" -ForegroundColor Green
}
finally {
    if ($serverProcess -and -not $serverProcess.HasExited) {
        Write-Step "stopping temporary HTTP server"
        Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    }
}
