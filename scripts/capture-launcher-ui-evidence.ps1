<#
Captures player-facing launcher UI visual evidence.

The script builds the launcher web UI, serves it through Vite preview, captures a
headless browser screenshot, dumps the rendered DOM, and writes a JSON evidence
summary for the production evidence manifest.

Examples:
  .\scripts\capture-launcher-ui-evidence.ps1
  .\scripts\capture-launcher-ui-evidence.ps1 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-launcher-ui
#>

[CmdletBinding()]
param(
    [string]$LauncherDir = (Join-Path $PSScriptRoot "..\DBA_GameLauncher"),
    [string]$EvidenceDir = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\client"),
    [string]$RunId = "",
    [string]$BrowserPath = "",
    [int]$Port = 0,
    [int]$StartupTimeoutSec = 30
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RunId)) {
    $RunId = "launcher-ui-visual-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[launcher-ui-visual] " + $Message) -ForegroundColor Cyan
}

function Resolve-CommandSource {
    param([Parameter(Mandatory = $true)][string[]]$Names)

    foreach ($name in $Names) {
        $command = Get-Command $name -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }

    throw "Could not find command: $($Names -join ', ')"
}

function Resolve-Browser {
    param([string]$ExplicitBrowserPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitBrowserPath)) {
        if (-not (Test-Path -LiteralPath $ExplicitBrowserPath)) {
            throw "BrowserPath does not exist: $ExplicitBrowserPath"
        }
        return (Resolve-Path -LiteralPath $ExplicitBrowserPath).ProviderPath
    }

    $candidates = @(
        "msedge.exe",
        "chrome.exe",
        "chromium.exe",
        "C:\Program Files\Microsoft Edge\Application\msedge.exe",
        "C:\Program Files (x86)\Microsoft Edge\Application\msedge.exe",
        "C:\Program Files\Google\Chrome\Application\chrome.exe",
        "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"
    )

    foreach ($candidate in $candidates) {
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).ProviderPath
        }
    }

    throw "Could not find Microsoft Edge, Chrome, or Chromium for headless screenshot capture."
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

function Invoke-Process {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [Parameter(Mandatory = $true)][string]$StdoutPath,
        [Parameter(Mandatory = $true)][string]$StderrPath
    )

    $process = Start-Process `
        -FilePath $FilePath `
        -ArgumentList $ArgumentList `
        -WorkingDirectory $WorkingDirectory `
        -NoNewWindow `
        -Wait `
        -PassThru `
        -RedirectStandardOutput $StdoutPath `
        -RedirectStandardError $StderrPath

    return $process.ExitCode
}

function Wait-HttpReady {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][int]$TimeoutSec
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        try {
            $response = Invoke-WebRequest -Uri $Url -UseBasicParsing -TimeoutSec 2
            if ($response.StatusCode -ge 200 -and $response.StatusCode -lt 500) {
                return $true
            }
        }
        catch {
        }

        Start-Sleep -Milliseconds 500
    }

    return $false
}

function Stop-ProcessTree {
    param([Parameter(Mandatory = $true)][int]$TargetProcessId)

    $children = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object { $_.ParentProcessId -eq $TargetProcessId })
    foreach ($child in $children) {
        Stop-ProcessTree -TargetProcessId $child.ProcessId
    }

    $process = Get-Process -Id $TargetProcessId -ErrorAction SilentlyContinue
    if ($process) {
        Stop-Process -Id $TargetProcessId -Force -ErrorAction SilentlyContinue
        try {
            $null = $process.WaitForExit(5000)
        }
        catch {
        }
    }
}

function Wait-FilesReadable {
    param(
        [Parameter(Mandatory = $true)][string[]]$Paths,
        [int]$TimeoutSec = 10
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        $locked = $false
        foreach ($path in $Paths) {
            if (-not (Test-Path -LiteralPath $path)) {
                continue
            }

            try {
                $stream = [System.IO.File]::Open($path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
                $stream.Dispose()
            }
            catch {
                $locked = $true
                break
            }
        }

        if (-not $locked) {
            return $true
        }

        Start-Sleep -Milliseconds 250
    }

    return $false
}

$resolvedLauncherDir = (Resolve-Path -LiteralPath $LauncherDir).ProviderPath
$resolvedEvidenceDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceDir)
New-Item -ItemType Directory -Force -Path $resolvedEvidenceDir | Out-Null

if ($Port -le 0) {
    $Port = Get-FreeTcpPort
}

$npmPath = Resolve-CommandSource @("npm.cmd", "npm")
$browser = Resolve-Browser -ExplicitBrowserPath $BrowserPath
$pageUrl = "http://127.0.0.1:$Port/"

$buildStdoutPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.build.log" -f $RunId)
$buildStderrPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.build.stderr.log" -f $RunId)
$previewStdoutPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.preview.log" -f $RunId)
$previewStderrPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.preview.stderr.log" -f $RunId)
$browserStdoutPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.browser.log" -f $RunId)
$browserStderrPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.browser.stderr.log" -f $RunId)
$domPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.dom.html" -f $RunId)
$screenshotPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.png" -f $RunId)
$summaryPath = Join-Path $resolvedEvidenceDir ("launcher-ui-visual-evidence-{0}.json" -f $RunId)

Write-Step "running npm run build"
$startedAt = Get-Date
$buildExitCode = Invoke-Process `
    -FilePath $npmPath `
    -ArgumentList @("run", "build") `
    -WorkingDirectory $resolvedLauncherDir `
    -StdoutPath $buildStdoutPath `
    -StderrPath $buildStderrPath

if ($buildExitCode -ne 0) {
    throw "npm run build exited with code $buildExitCode"
}

$previewProcess = $null
$previewStarted = $false
$screenshotExitCode = $null
$domExitCode = $null
try {
    Write-Step "starting npm run preview"
    $previewProcess = Start-Process `
        -FilePath $npmPath `
        -ArgumentList @("run", "preview", "--", "--host", "127.0.0.1", "--port", "$Port", "--strictPort") `
        -WorkingDirectory $resolvedLauncherDir `
        -WindowStyle Hidden `
        -PassThru `
        -RedirectStandardOutput $previewStdoutPath `
        -RedirectStandardError $previewStderrPath

    $previewStarted = Wait-HttpReady -Url $pageUrl -TimeoutSec $StartupTimeoutSec
    if (-not $previewStarted) {
        throw "Vite preview did not become ready at $pageUrl within $StartupTimeoutSec seconds."
    }

    Write-Step "capturing headless screenshot"
    $screenshotExitCode = Invoke-Process `
        -FilePath $browser `
        -ArgumentList @("--headless=new", "--disable-gpu", "--window-size=1280,720", "--screenshot=$screenshotPath", $pageUrl) `
        -WorkingDirectory $resolvedLauncherDir `
        -StdoutPath $browserStdoutPath `
        -StderrPath $browserStderrPath

    Write-Step "dumping rendered DOM"
    $domExitCode = Invoke-Process `
        -FilePath $browser `
        -ArgumentList @("--headless=new", "--disable-gpu", "--dump-dom", $pageUrl) `
        -WorkingDirectory $resolvedLauncherDir `
        -StdoutPath $domPath `
        -StderrPath $browserStderrPath
}
finally {
    if ($previewProcess -and -not $previewProcess.HasExited) {
        Write-Step "stopping Vite preview"
        Stop-ProcessTree -TargetProcessId $previewProcess.Id
        $null = Wait-FilesReadable -Paths @($previewStdoutPath, $previewStderrPath)
    }
}

$endedAt = Get-Date
$screenshotReady = $screenshotExitCode -eq 0 -and (Test-Path -LiteralPath $screenshotPath) -and ((Get-Item -LiteralPath $screenshotPath).Length -gt 0)
$domText = if (Test-Path -LiteralPath $domPath) { Get-Content -Raw -Encoding UTF8 -LiteralPath $domPath } else { "" }
$requiredMarkers = @("Divine Beasts Arena", "launcher-shell", "action-bar", "primary")
$missingMarkers = @($requiredMarkers | Where-Object { $domText -notmatch [regex]::Escape($_) })
$uiMarkersReady = $domExitCode -eq 0 -and $missingMarkers.Count -eq 0
$uiEvidenceReady = $buildExitCode -eq 0 -and $previewStarted -and $screenshotReady -and $uiMarkersReady

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "launcher-ui-visual-evidence"
    runId = $RunId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    launcherDir = $resolvedLauncherDir
    pageUrl = $pageUrl
    browserPath = $browser
    command = "npm run build; npm run preview -- --host 127.0.0.1 --port $Port --strictPort"
    buildExitCode = $buildExitCode
    previewStarted = $previewStarted
    screenshotExitCode = $screenshotExitCode
    domExitCode = $domExitCode
    screenshotReady = $screenshotReady
    uiMarkersReady = $uiMarkersReady
    uiEvidenceReady = $uiEvidenceReady
    requiredMarkers = $requiredMarkers
    missingMarkers = $missingMarkers
    startedAtUtc = $startedAt.ToUniversalTime().ToString("o")
    endedAtUtc = $endedAt.ToUniversalTime().ToString("o")
    durationSeconds = [math]::Round(($endedAt - $startedAt).TotalSeconds, 3)
    screenshotPath = $screenshotPath
    domPath = $domPath
    buildStdoutPath = $buildStdoutPath
    buildStderrPath = $buildStderrPath
    previewStdoutPath = $previewStdoutPath
    previewStderrPath = $previewStderrPath
    browserStdoutPath = $browserStdoutPath
    browserStderrPath = $browserStderrPath
    notes = @(
        if (-not $screenshotReady) { "Headless browser screenshot was not created." }
        if (-not $uiMarkersReady) { "Rendered DOM is missing required launcher UI markers: $($missingMarkers -join ', ')" }
    )
}

$summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryPath -Encoding UTF8
Write-Step "wrote launcher UI visual evidence: $summaryPath"

if (-not $uiEvidenceReady) {
    throw "Launcher UI visual evidence is not ready; inspect $summaryPath."
}

Write-Host "PASS: launcher UI visual evidence collected" -ForegroundColor Green
