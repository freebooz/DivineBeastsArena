param(
    [string]$BaseUrl = "http://localhost:8083",
    [string]$UnrealRoot = "E:\UnrealEngine-5.7.1-release",
    [string]$InternalApiKey = "DEV-INTERNAL-API-KEY-MIN-32-CHARS-ONLY",
    [string]$ProjectPath = "D:\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject",
    [switch]$UsePackagedServer,
    [int]$TimeoutSec = 15
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$LogDir = Join-Path $RepoRoot ".tmp\local-ue-validation"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function Write-Step([string]$Message) {
    Write-Host ("[ue-flow] " + $Message)
}

function Invoke-Json(
    [string]$Method,
    [string]$Path,
    $Body = $null,
    [string]$Token = "",
    [hashtable]$ExtraHeaders = @{}
) {
    $headers = @{}
    foreach ($key in $ExtraHeaders.Keys) {
        $headers[$key] = $ExtraHeaders[$key]
    }
    if (-not [string]::IsNullOrWhiteSpace($Token)) {
        $headers["Authorization"] = "Bearer $Token"
    }

    $uri = $BaseUrl.TrimEnd("/") + $Path
    if ($null -eq $Body) {
        return Invoke-RestMethod -Uri $uri -Method $Method -Headers $headers -TimeoutSec $TimeoutSec
    }

    $json = $Body | ConvertTo-Json -Depth 20 -Compress
    return Invoke-RestMethod -Uri $uri -Method $Method -Headers $headers -ContentType "application/json" -Body $json -TimeoutSec $TimeoutSec
}

function Assert-Ok($Response, [string]$Name) {
    if ($null -eq $Response) {
        throw "$Name returned no response"
    }
    if ($Response.PSObject.Properties.Name -contains "success" -and -not $Response.success) {
        throw "$Name failed: $($Response | ConvertTo-Json -Depth 10 -Compress)"
    }
}

function Get-Data($Response, [string]$Name) {
    Assert-Ok $Response $Name
    if ($Response.PSObject.Properties.Name -contains "data") {
        return $Response.data
    }
    return $Response
}

function New-Guest([string]$Suffix) {
    $deviceId = "ue-flow-$Suffix-$RunShort"
    $login = Get-Data (Invoke-Json "POST" "/api/auth/guest-login" @{
        deviceId = $deviceId
        deviceName = "UEValidation-$Suffix"
        platform = "Windows"
        displayName = "UE$Suffix$RunShort"
    }) "guest login $Suffix"

    return [pscustomobject]@{
        Suffix = $Suffix
        DeviceId = $deviceId
        PlayerId = [string]$login.playerId
        Token = [string]$login.accessToken
    }
}

function Ensure-Character($Guest) {
    $characters = Invoke-Json "GET" "/api/account/characters" $null $Guest.Token
    Assert-Ok $characters "list characters $($Guest.Suffix)"

    $characterId = [string]$characters.selectedCharacterId
    if ([string]::IsNullOrWhiteSpace($characterId) -and $characters.characters.Count -gt 0) {
        $characterId = [string]$characters.characters[0].characterId
    }
    if ([string]::IsNullOrWhiteSpace($characterId)) {
        $created = Invoke-Json "POST" "/api/account/characters" @{
            characterName = "UE_$($Guest.Suffix)_$RunShort"
            zodiac = "Rat"
            primaryElement = "Water"
            fiveCamp = "East"
        } $Guest.Token
        Assert-Ok $created "create character $($Guest.Suffix)"
        $characterId = [string]$created.character.characterId
    }

    Assert-Ok (Invoke-Json "POST" "/api/account/characters/$characterId/select" $null $Guest.Token) "select character $($Guest.Suffix)"
    return $characterId
}

$RunShort = ([guid]::NewGuid().ToString("N")).Substring(0, 8)
$EditorExe = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$ServerExe = if ($UsePackagedServer) {
    Join-Path (Split-Path -Parent $ProjectPath) "Binaries\Win64\DivineBeastsArenaServer.exe"
} else {
    $EditorExe
}

if (-not (Test-Path -LiteralPath $EditorExe)) {
    throw "UnrealEditor.exe not found: $EditorExe"
}
if ($UsePackagedServer -and -not (Test-Path -LiteralPath $ServerExe)) {
    throw "DivineBeastsArenaServer.exe not found: $ServerExe"
}

Write-Step "creating backend room/session"
$guestA = New-Guest "A"
$guestB = New-Guest "B"
$null = Ensure-Character $guestA
$null = Ensure-Character $guestB

$room = Get-Data (Invoke-Json "POST" "/api/rooms/" @{
    mode = "classic"
    mapId = "LobbyMap"
    region = "local"
    maxPlayers = 2
    visibility = "public"
    password = $null
} $guestA.Token) "create room"
$roomId = [string]$room.id

Assert-Ok (Invoke-Json "POST" "/api/rooms/$roomId/join" @{ password = $null } $guestB.Token) "join room B"
Assert-Ok (Invoke-Json "POST" "/api/rooms/$roomId/ready" @{ isReady = $true } $guestB.Token) "ready B"
Assert-Ok (Invoke-Json "POST" "/api/rooms/$roomId/start" $null $guestA.Token) "start room"

$session = Get-Data (Invoke-Json "POST" "/internal/sessions/from-room" @{ roomId = $roomId }) "create session"
$sessionId = [string]$session.id

$internalHeaders = @{ "X-Internal-Api-Key" = $InternalApiKey }
$allocation = Get-Data (Invoke-Json "POST" "/internal/game-servers/allocate" @{
    sessionId = $sessionId
    mode = "classic"
    mapId = "LobbyMap"
    region = "local"
    buildVersion = "local-validation"
} "" $internalHeaders) "allocate managed server"

if ([string]::IsNullOrWhiteSpace($allocation.runtimeToken)) {
    throw "managed server allocation did not return runtimeToken; release stale active servers or allocate a new session"
}

$serverLog = Join-Path $LogDir "server-$RunShort.log"
$clientALog = Join-Path $LogDir "client-a-$RunShort.log"
$clientBLog = Join-Path $LogDir "client-b-$RunShort.log"

Write-Step "starting dedicated server on $($allocation.publicIp):$($allocation.port)"
$serverArgsList = if ($UsePackagedServer) {
    @(
        "/Game/Maps/Lobby/LobbyMap",
        "-server"
    )
} else {
    @(
        "`"$ProjectPath`"",
        "/Game/Maps/Lobby/LobbyMap",
        "-server"
    )
}
$serverArgs = ($serverArgsList + @(
    "-log",
    "-abslog=`"$serverLog`"",
    "-port=$($allocation.port)",
    "-sessionId=$sessionId",
    "-serverId=$($allocation.serverId)",
    "-runtimeToken=`"$($allocation.runtimeToken)`"",
    "-backendUrl=$BaseUrl",
    "-DBAHeadlessLobbyServer"
)) -join " "
$serverProcess = Start-Process -FilePath $ServerExe -ArgumentList $serverArgs -PassThru -WorkingDirectory (Split-Path -Parent $ProjectPath)

Start-Sleep -Seconds 18

$connA = Get-Data (Invoke-Json "GET" "/api/sessions/$sessionId/connection" $null $guestA.Token) "connection A"
$connB = Get-Data (Invoke-Json "GET" "/api/sessions/$sessionId/connection" $null $guestB.Token) "connection B"

Write-Step "starting client A and client B windows"
$connectA = "127.0.0.1:$($allocation.port)?PlayerId=$($guestA.PlayerId)?PlayerSessionToken=$($connA.playerSessionToken)?DBALobbyZodiac=Rat"
$connectB = "127.0.0.1:$($allocation.port)?PlayerId=$($guestB.PlayerId)?PlayerSessionToken=$($connB.playerSessionToken)?DBALobbyZodiac=Tiger"

$clientArgsA = @(
    "`"$ProjectPath`"",
    $connectA,
    "-game",
    "-windowed",
    "-ResX=960",
    "-ResY=540",
    "-WinX=40",
    "-WinY=60",
    "-log",
    "-abslog=`"$clientALog`"",
    "-DBABackendUrl=$BaseUrl",
    "-DBASkipFrontendFlow",
    "-DBASaveSlotSuffix=UEFlowA$RunShort",
    "-DBAGuestDeviceId=ue-flow-client-a-$RunShort"
) -join " "
$clientArgsB = @(
    "`"$ProjectPath`"",
    $connectB,
    "-game",
    "-windowed",
    "-ResX=960",
    "-ResY=540",
    "-WinX=1040",
    "-WinY=60",
    "-log",
    "-abslog=`"$clientBLog`"",
    "-DBABackendUrl=$BaseUrl",
    "-DBASkipFrontendFlow",
    "-DBASaveSlotSuffix=UEFlowB$RunShort",
    "-DBAGuestDeviceId=ue-flow-client-b-$RunShort"
) -join " "

$clientAProcess = Start-Process -FilePath $EditorExe -ArgumentList $clientArgsA -PassThru -WorkingDirectory (Split-Path -Parent $ProjectPath)
Start-Sleep -Seconds 3
$clientBProcess = Start-Process -FilePath $EditorExe -ArgumentList $clientArgsB -PassThru -WorkingDirectory (Split-Path -Parent $ProjectPath)

[pscustomobject]@{
    RunId = $RunShort
    RoomId = $roomId
    SessionId = $sessionId
    ServerId = [string]$allocation.serverId
    Port = [int]$allocation.port
    ServerProcessId = $serverProcess.Id
    ClientAProcessId = $clientAProcess.Id
    ClientBProcessId = $clientBProcess.Id
    ServerLog = $serverLog
    ClientALog = $clientALog
    ClientBLog = $clientBLog
}
