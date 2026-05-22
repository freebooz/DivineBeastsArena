param(
    [string]$ApiBaseUrl = "http://localhost:8080",
    [switch]$LiveApi,
    [switch]$IncludeGameClient,
    [string]$UnrealBuildBat = "E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat"
)

$ErrorActionPreference = "Stop"

function Invoke-Check {
    param(
        [string]$Name,
        [scriptblock]$Action
    )

    Write-Host "==> $Name"
    & $Action
    Write-Host "OK  $Name"
}

Invoke-Check "Backend solution build" {
    Push-Location "$PSScriptRoot\..\DBA_GameBackend"
    try { dotnet build GameBackend.slnx --nologo } finally { Pop-Location }
}

Invoke-Check "Backend tests" {
    Push-Location "$PSScriptRoot\..\DBA_GameBackend"
    try { dotnet test --no-build --nologo } finally { Pop-Location }
}

Invoke-Check "Backend vulnerable package scan" {
    Push-Location "$PSScriptRoot\..\DBA_GameBackend"
    try { dotnet list GameBackend.slnx package --vulnerable } finally { Pop-Location }
}

Invoke-Check "Admin build" {
    Push-Location "$PSScriptRoot\..\DBA_GameAdmin"
    try { dotnet build GameAdmin.csproj --nologo } finally { Pop-Location }
}

Invoke-Check "Admin vulnerable package scan" {
    Push-Location "$PSScriptRoot\..\DBA_GameAdmin"
    try { dotnet list GameAdmin.csproj package --vulnerable } finally { Pop-Location }
}

Invoke-Check "Website build" {
    Push-Location "$PSScriptRoot\..\DBA_GameWebsite"
    try { npm run build } finally { Pop-Location }
}

Invoke-Check "Website npm audit" {
    Push-Location "$PSScriptRoot\..\DBA_GameWebsite"
    try { npm audit --audit-level=moderate } finally { Pop-Location }
}

Invoke-Check "Launcher build" {
    Push-Location "$PSScriptRoot\..\DBA_GameLauncher"
    try { npm run build } finally { Pop-Location }
}

Invoke-Check "Launcher npm audit" {
    Push-Location "$PSScriptRoot\..\DBA_GameLauncher"
    try { npm audit --audit-level=moderate } finally { Pop-Location }
}

if ($IncludeGameClient) {
    Invoke-Check "Unreal DBA_GameClient editor build" {
        $projectPath = Resolve-Path "$PSScriptRoot\..\DBA_GameClient\DivineBeastsArena.uproject"
        if (-not (Test-Path $UnrealBuildBat)) {
            throw "Unreal Build.bat not found: $UnrealBuildBat"
        }
        & $UnrealBuildBat DivineBeastsArenaEditor Win64 Development "-Project=$projectPath" -NoHotReload
    }
}

if ($LiveApi) {
    Invoke-Check "Live API liveness" {
        Invoke-RestMethod "$ApiBaseUrl/health/live" | Out-Null
    }

    Invoke-Check "Live platform catalog" {
        Invoke-RestMethod "$ApiBaseUrl/api/platform/applications" | Out-Null
    }

    Invoke-Check "Live launcher manifest" {
        Invoke-RestMethod "$ApiBaseUrl/launcher/manifest.json" | Out-Null
    }
}

Write-Host ""
Write-Host "Optional live API checks: .\scripts\check-platform.ps1 -LiveApi -ApiBaseUrl $ApiBaseUrl"
Write-Host "Optional Unreal check: .\scripts\check-platform.ps1 -IncludeGameClient"

