<#
Read-only diagnostics for a self-hosted Windows Unreal evidence runner.

This script checks the machine and repository prerequisites needed before
running .github/workflows/unreal-evidence.yml.
#>

[CmdletBinding()]
param(
    [string]$BaseUrl = "http://localhost:8080",
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$ProjectPath = "",
    [string]$InternalApiKey = $env:DBA_INTERNAL_API_KEY,
    [string]$JsonOutputPath = "",
    [switch]$SkipBackendProbe
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\DivineBeastsArena.uproject"
}

function Resolve-InternalApiKey {
    param([string]$ExplicitInternalApiKey)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitInternalApiKey)) {
        return $ExplicitInternalApiKey
    }

    $envPath = Join-Path $repoRoot "DBA_GameBackend\.env"
    if (-not (Test-Path -LiteralPath $envPath)) {
        return ""
    }

    $line = Get-Content -Encoding UTF8 $envPath |
        Where-Object { $_ -match "^INTERNAL_API_KEY=" } |
        Select-Object -First 1

    if (-not $line) {
        return ""
    }

    return $line.Substring("INTERNAL_API_KEY=".Length)
}

$resolvedInternalApiKey = Resolve-InternalApiKey -ExplicitInternalApiKey $InternalApiKey
$checks = New-Object System.Collections.Generic.List[object]

function Add-Check {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Status,
        [string]$Detail = ""
    )

    $checks.Add([ordered]@{
        name = $Name
        status = $Status
        detail = $Detail
    })

    $color = switch ($Status) {
        "PASS" { "Green" }
        "WARN" { "Yellow" }
        default { "Red" }
    }
    Write-Host ("{0} {1} - {2}" -f $Status, $Name, $Detail) -ForegroundColor $color
}

function Test-PathCheck {
    param(
        [string]$Name,
        [string]$Path
    )

    if (Test-Path -LiteralPath $Path) {
        Add-Check $Name "PASS" $Path
    }
    else {
        Add-Check $Name "FAIL" $Path
    }
}

function Test-BackendHealth {
    param([string]$Url)

    if ($SkipBackendProbe) {
        Add-Check "backend.health" "WARN" "Skipped by -SkipBackendProbe"
        return
    }

    if ([string]::IsNullOrWhiteSpace($Url)) {
        Add-Check "backend.health" "FAIL" "BaseUrl is empty"
        return
    }

    $healthUrl = $Url.TrimEnd("/") + "/health/live"
    try {
        $response = Invoke-WebRequest -Uri $healthUrl -Method Get -TimeoutSec 10 -UseBasicParsing
        if ($response.StatusCode -ge 200 -and $response.StatusCode -lt 500) {
            Add-Check "backend.health" "PASS" $healthUrl
            return
        }

        Add-Check "backend.health" "FAIL" "$healthUrl returned HTTP $($response.StatusCode)"
    }
    catch {
        Add-Check "backend.health" "FAIL" "$healthUrl failed: $($_.Exception.Message)"
    }
}

$isWindows = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows)
Add-Check "runner.os.windows" ($(if ($isWindows) { "PASS" } else { "FAIL" })) ([System.Runtime.InteropServices.RuntimeInformation]::OSDescription)

Test-PathCheck "repo.workflow.unreal-evidence" (Join-Path $repoRoot ".github\workflows\unreal-evidence.yml")
Test-PathCheck "repo.script.validate-production-evidence-contracts" (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1")
Test-PathCheck "repo.script.run-unreal-evidence" (Join-Path $repoRoot "scripts\run-unreal-evidence.ps1")
Test-PathCheck "repo.script.start-local-ue-validation" (Join-Path $repoRoot "scripts\start-local-ue-validation.ps1")
Test-PathCheck "repo.script.collect-production-evidence" (Join-Path $repoRoot "scripts\collect-production-evidence.ps1")
Test-PathCheck "repo.script.package-unreal-dedicated-server" (Join-Path $repoRoot "scripts\package-unreal-dedicated-server.ps1")

Test-PathCheck "unreal.root" $UnrealRoot
Test-PathCheck "unreal.editor" (Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor.exe")
Test-PathCheck "unreal.editor_cmd" (Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
Test-PathCheck "unreal.runuat" (Join-Path $UnrealRoot "Engine\Build\BatchFiles\RunUAT.bat")
Test-PathCheck "unreal.project" $ProjectPath

if ([string]::IsNullOrWhiteSpace($resolvedInternalApiKey)) {
    Add-Check "backend.internal_api_key" "FAIL" "Missing -InternalApiKey, DBA_INTERNAL_API_KEY, and DBA_GameBackend/.env INTERNAL_API_KEY"
}
else {
    Add-Check "backend.internal_api_key" "PASS" ("configured length={0}" -f $resolvedInternalApiKey.Length)
}

Test-BackendHealth -Url $BaseUrl

try {
    & (Join-Path $repoRoot "scripts\validate-production-evidence-contracts.ps1") | Out-Host
    Add-Check "evidence.contracts" "PASS" "validate-production-evidence-contracts.ps1"
}
catch {
    Add-Check "evidence.contracts" "FAIL" $_.Exception.Message
}

$summary = [ordered]@{
    schemaVersion = "1.0"
    kind = "unreal-evidence-runner-diagnostic"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    repoRoot = $repoRoot
    baseUrl = $BaseUrl
    unrealRoot = $UnrealRoot
    projectPath = $ProjectPath
    checks = $checks.ToArray()
}

if (-not [string]::IsNullOrWhiteSpace($JsonOutputPath)) {
    $outputDirectory = Split-Path -Parent $JsonOutputPath
    if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
        New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
    }

    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $JsonOutputPath -Encoding UTF8
    Write-Host ("Diagnostic JSON written: {0}" -f $JsonOutputPath)
}

$failed = @($checks.ToArray() | Where-Object { $_.status -eq "FAIL" })
if ($failed.Count -gt 0) {
    Write-Host ("SUMMARY: {0} runner prerequisite check(s) failed." -f $failed.Count) -ForegroundColor Red
    exit 1
}

Write-Host "SUMMARY: Unreal evidence runner prerequisites passed." -ForegroundColor Green
