# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

param(
    [ValidateSet("Server", "Clients", "AutoClients", "All", "Stop")]
    [string]$Mode = "All"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$ProjectFile = Join-Path $ProjectRoot "DivineBeastsArena.uproject"

$PackagedRoots = @(
    (Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\Windows\DivineBeastsArena"),
    (Join-Path $ProjectRoot "Saved\StagedBuilds\Windows\DivineBeastsArena")
)

$Root = $null
$NewestExe = $null
$UseEditor = $false
foreach ($CandidateRoot in $PackagedRoots) {
    $CandidateExe = Join-Path $CandidateRoot "Binaries\Win64\DivineBeastsArena.exe"
    if (Test-Path -LiteralPath $CandidateExe) {
        $ExeItem = Get-Item -LiteralPath $CandidateExe
        if (-not $NewestExe -or $ExeItem.LastWriteTime -gt $NewestExe.LastWriteTime) {
            $NewestExe = $ExeItem
            $Root = $CandidateRoot
        }
    }
}

if (-not $Root) {
    $ProjectJson = Get-Content -Raw -LiteralPath $ProjectFile | ConvertFrom-Json
    $EngineAssociation = $ProjectJson.EngineAssociation
    $EngineRoot = $null

    if ($EngineAssociation) {
        $EngineBuilds = Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
        if ($EngineBuilds -and $EngineBuilds.PSObject.Properties.Name -contains $EngineAssociation) {
            $EngineRoot = $EngineBuilds.$EngineAssociation
        }
    }

    if (-not $EngineRoot) {
        $InstalledEngine = Get-ItemProperty "HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.8" -ErrorAction SilentlyContinue
        if ($InstalledEngine) {
            $EngineRoot = $InstalledEngine.InstalledDirectory
        }
    }

    if (-not $EngineRoot) {
        $SourceEngineRoot = "D:\UnrealEngine-5.8.0-release"
        if (Test-Path -LiteralPath (Join-Path $SourceEngineRoot "Engine\Binaries\Win64\UnrealEditor.exe")) {
            $EngineRoot = $SourceEngineRoot
        }
    }

    if (-not $EngineRoot) {
        throw "Missing staged executable and could not resolve Unreal Engine path."
    }

    $EditorExe = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
    if (-not (Test-Path -LiteralPath $EditorExe)) {
        throw "Missing UnrealEditor executable: $EditorExe"
    }

    $Root = $ProjectRoot
    $NewestExe = Get-Item -LiteralPath $EditorExe
    $UseEditor = $true
}

$Exe = $NewestExe.FullName
$LogDir = Join-Path $ProjectRoot "Saved\Logs"
$LockPath = Join-Path $env:TEMP "DivineBeastsArena_LaunchLobby.lock"

if (-not (Test-Path -LiteralPath $Exe)) {
    throw "Missing executable: $Exe"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function New-LaunchArgs {
    param([string[]]$LaunchArgs)

    if ($UseEditor) {
        return @($ProjectFile) + $LaunchArgs + @("-game")
    }

    return $LaunchArgs
}

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
    $StartedServer = Start-Process -FilePath $Exe -WorkingDirectory $Root -WindowStyle Hidden -PassThru -ArgumentList (New-LaunchArgs -LaunchArgs @(
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
    ))
    Start-Sleep -Seconds 2
    $ServersAfterStart = @(Get-DBAProcess | Where-Object { Test-IsServerProcess $_ })
    foreach ($ServerProcess in $ServersAfterStart) {
        if ($StartedServer -and $ServerProcess.ProcessId -ne $StartedServer.Id) {
            Stop-Process -Id $ServerProcess.ProcessId -Force -ErrorAction SilentlyContinue
        }
    }
}

if ($Mode -eq "Clients" -or $Mode -eq "AutoClients" -or $Mode -eq "All") {
    Stop-ProcessIds $Clients
    Start-Sleep -Milliseconds 500

    if ($Mode -eq "AutoClients") {
        $ClientArgs = @(
        (New-LaunchArgs -LaunchArgs @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-DBASkipSplash", "-DBAAutoLobbyFlow", "-DBASaveSlotSuffix=ClientA", "-DBAGuestAccountId=LobbyClientA", "-DBAGuestDisplayName=LobbyClientA", "-DBAAutoCharacterName=LobbyA_Rat", "-DBAAutoZodiac=Rat", "-DBAAutoPartyLeader", "-DBAAutoInviteAccountId=LobbyClientB", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=50", "-WinY=50", "-abslog=$LogDir\ClientA.log")),
        (New-LaunchArgs -LaunchArgs @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-DBASkipSplash", "-DBAAutoLobbyFlow", "-DBASaveSlotSuffix=ClientB", "-DBAGuestAccountId=LobbyClientB", "-DBAGuestDisplayName=LobbyClientB", "-DBAAutoCharacterName=LobbyB_Ox", "-DBAAutoZodiac=Ox", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=1400", "-WinY=50", "-abslog=$LogDir\ClientB.log"))
        )
    }
    else {
        $ClientArgs = @(
            (New-LaunchArgs -LaunchArgs @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-log", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=50", "-WinY=50", "-abslog=$LogDir\ClientA.log")),
            (New-LaunchArgs -LaunchArgs @("/Game/Maps/Lobby/FrontendMap", "-nosteam", "-log", "-WINDOWED", "-ResX=1280", "-ResY=720", "-WinX=1400", "-WinY=50", "-abslog=$LogDir\ClientB.log"))
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

