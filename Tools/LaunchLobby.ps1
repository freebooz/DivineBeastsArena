param(
    [ValidateSet("Server", "Clients", "AutoClients", "All", "Stop")]
    [string]$Mode = "All"
)

$ErrorActionPreference = "Stop"

$Root = "d:\DivineBeastsArena\Saved\StagedBuilds\Windows\DivineBeastsArena"
$Exe = Join-Path $Root "Binaries\Win64\DivineBeastsArena.exe"
$LogDir = Join-Path $Root "Saved\Logs"
$LockPath = Join-Path $env:TEMP "DivineBeastsArena_LaunchLobby.lock"

if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

$LockStream = [System.IO.File]::Open($LockPath, [System.IO.FileMode]::OpenOrCreate, [System.IO.FileAccess]::ReadWrite, [System.IO.FileShare]::None)
try {

function Get-DBAProcess {
    Get-CimInstance Win32_Process |
        Where-Object {
            $_.Name -eq "DivineBeastsArena.exe" -or
            $_.Name -eq "DivineBeastsArenaServer.exe" -or
            $_.CommandLine -like "*DivineBeastsArena*"
        } |
        Where-Object {
            $_.Name -notin @("powershell.exe", "cmd.exe", "Code.exe") -and
            $_.CommandLine -notmatch "LaunchLobby.ps1|run_server.bat|run_client.bat|StartServer.bat|StartClient.bat"
        }
}

function Test-IsServerProcess($Process) {
    return $Process.CommandLine -match "(^|\s)-server(\s|$)|DBAHeadlessLobbyServer"
}

function Stop-ProcessIds($Processes) {
    foreach ($Process in $Processes) {
        Stop-Process -Id $Process.ProcessId -Force -ErrorAction SilentlyContinue
    }
}

if ($Mode -eq "Stop") {
    Stop-ProcessIds (Get-DBAProcess)
    exit 0
}

$Processes = @(Get-DBAProcess)
$Servers = @($Processes | Where-Object { Test-IsServerProcess $_ })
$Clients = @($Processes | Where-Object { -not (Test-IsServerProcess $_) })

if ($Mode -eq "Server" -or $Mode -eq "All") {
    Stop-ProcessIds $Servers
    Start-Sleep -Milliseconds 500
    Start-Process -FilePath $Exe -WorkingDirectory $Root -ArgumentList @(
        "/Game/Maps/Lobby/LobbyMap?listen",
        "-server",
        "-nosteam",
        "-port=7777",
        "-nullrhi",
        "-nosound",
        "-unattended",
        "-NoSplash",
        "-DBAHeadlessLobbyServer",
        "-log",
        "-abslog=$LogDir\LobbyServer.log"
    )
}

if ($Mode -eq "Clients" -or $Mode -eq "AutoClients" -or $Mode -eq "All") {
    Stop-ProcessIds $Clients
    Start-Sleep -Milliseconds 500

    if ($Mode -eq "AutoClients") {
        $ClientArgs = @(
        @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-DBASkipSplash", "-DBAAutoLobbyFlow", "-DBASaveSlotSuffix=ClientA", "-DBAGuestAccountId=LobbyClientA", "-DBAGuestDisplayName=LobbyClientA", "-DBAAutoCharacterName=LobbyA_Rat", "-DBAAutoPartyLeader", "-DBAAutoInviteAccountId=LobbyClientB", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=50", "-WinY=50", "-abslog=$LogDir\ClientA.log"),
        @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-DBASkipSplash", "-DBAAutoLobbyFlow", "-DBASaveSlotSuffix=ClientB", "-DBAGuestAccountId=LobbyClientB", "-DBAGuestDisplayName=LobbyClientB", "-DBAAutoCharacterName=LobbyB_Ox", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=1400", "-WinY=50", "-abslog=$LogDir\ClientB.log")
        )
    }
    else {
        $ClientArgs = @(
            @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-log", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=50", "-WinY=50", "-abslog=$LogDir\ClientA.log"),
            @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-log", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=1400", "-WinY=50", "-abslog=$LogDir\ClientB.log")
        )
    }

    for ($Index = 0; $Index -lt 2; ++$Index) {
        Start-Process -FilePath $Exe -WorkingDirectory $Root -ArgumentList $ClientArgs[$Index]
    }
}

Start-Sleep -Seconds 2
Get-DBAProcess |
    Select-Object ProcessId, CommandLine |
    Format-Table -AutoSize
}
finally {
    if ($LockStream) {
        $LockStream.Dispose()
    }
}
