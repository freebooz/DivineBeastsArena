param(
    [string]$BaseUrl = "http://localhost:8083",
    [int]$TimeoutSec = 15
)

$ErrorActionPreference = "Stop"

function Write-Step([string]$Message) {
    Write-Host ("[flow] " + $Message)
}

function Convert-ToJsonBody($Body) {
    if ($null -eq $Body) {
        return $null
    }

    return ($Body | ConvertTo-Json -Depth 20 -Compress)
}

function Invoke-Json(
    [string]$Method,
    [string]$Path,
    $Body = $null,
    [string]$Token = "",
    [switch]$AllowProblem
) {
    $headers = @{}
    if (-not [string]::IsNullOrWhiteSpace($Token)) {
        $headers["Authorization"] = "Bearer $Token"
    }

    $uri = $BaseUrl.TrimEnd("/") + $Path
    $jsonBody = Convert-ToJsonBody $Body
    try {
        if ($null -eq $jsonBody) {
            return Invoke-RestMethod -Uri $uri -Method $Method -Headers $headers -TimeoutSec $TimeoutSec
        }

        return Invoke-RestMethod -Uri $uri -Method $Method -Headers $headers -ContentType "application/json" -Body $jsonBody -TimeoutSec $TimeoutSec
    }
    catch {
        if (-not $AllowProblem) {
            throw
        }

        $response = $_.Exception.Response
        if ($null -eq $response) {
            throw
        }

        $stream = $response.GetResponseStream()
        $reader = [System.IO.StreamReader]::new($stream)
        $text = $reader.ReadToEnd()
        return [pscustomobject]@{
            statusCode = [int]$response.StatusCode
            problem = $text
        }
    }
}

function Assert-Ok($Response, [string]$Name) {
    if ($null -eq $Response) {
        throw "$Name returned no response"
    }

    if ($Response.PSObject.Properties.Name -contains "success" -and -not $Response.success) {
        $details = $Response | ConvertTo-Json -Depth 10 -Compress
        throw "$Name failed: $details"
    }
}

function Get-EnvelopeData($Response, [string]$Name) {
    Assert-Ok $Response $Name
    if ($Response.PSObject.Properties.Name -contains "data") {
        return $Response.data
    }

    return $Response
}

function New-Guest([string]$Suffix) {
    $deviceId = "codex-flow-$Suffix-$RunId"
    $displayName = "Codex$Suffix$RunShort"
    $login = Invoke-Json "POST" "/api/auth/guest-login" @{
        deviceId = $deviceId
        deviceName = "CodexSmoke-$Suffix"
        platform = "Windows"
        displayName = $displayName
    }
    $data = Get-EnvelopeData $login "guest login $Suffix"
    if ([string]::IsNullOrWhiteSpace($data.accessToken)) {
        throw "guest login $Suffix did not return accessToken"
    }

    Write-Step ("guest {0}: playerId={1}" -f $Suffix, $data.playerId)
    return [pscustomobject]@{
        Suffix = $Suffix
        DeviceId = $deviceId
        DisplayName = $displayName
        PlayerId = [string]$data.playerId
        Token = [string]$data.accessToken
    }
}

function Ensure-Character($Guest) {
    $characters = Invoke-Json "GET" "/api/account/characters" $null $Guest.Token
    Assert-Ok $characters "list characters $($Guest.Suffix)"

    $selectedId = [string]$characters.selectedCharacterId
    if ([string]::IsNullOrWhiteSpace($selectedId) -and $characters.characters.Count -gt 0) {
        $selectedId = [string]$characters.characters[0].characterId
    }

    if ([string]::IsNullOrWhiteSpace($selectedId)) {
        $created = Invoke-Json "POST" "/api/account/characters" @{
            characterName = "Hero_$($Guest.Suffix)_$RunShort"
            zodiac = "Rat"
            primaryElement = "Water"
            fiveCamp = "East"
        } $Guest.Token
        Assert-Ok $created "create character $($Guest.Suffix)"
        $selectedId = [string]$created.character.characterId
    }

    $selected = Invoke-Json "POST" "/api/account/characters/$selectedId/select" $null $Guest.Token
    Assert-Ok $selected "select character $($Guest.Suffix)"
    Write-Step ("character {0}: characterId={1}" -f $Guest.Suffix, $selectedId)
    return $selectedId
}

$RunId = [guid]::NewGuid().ToString("N")
$RunShort = $RunId.Substring(0, 8)
$RoomId = ""
$SessionId = ""

Write-Step "baseUrl=$BaseUrl"
Assert-Ok (Invoke-Json "GET" "/health/live") "health live"
Assert-Ok (Invoke-Json "GET" "/health/ready") "health ready"
Assert-Ok (Invoke-Json "GET" "/api/config/manifest?channel=stable&region=global") "config manifest"

$guestA = New-Guest "A"
$guestB = New-Guest "B"

Assert-Ok (Invoke-Json "GET" "/api/players/me/profile" $null $guestA.Token) "profile A"
Assert-Ok (Invoke-Json "GET" "/api/players/me/settings" $null $guestA.Token) "settings A"
Assert-Ok (Invoke-Json "GET" "/api/players/me/stats" $null $guestA.Token) "stats A"
Assert-Ok (Invoke-Json "GET" "/api/players/me/unlocks" $null $guestA.Token) "unlocks A"

$null = Ensure-Character $guestA
$null = Ensure-Character $guestB

$room = Get-EnvelopeData (Invoke-Json "POST" "/api/rooms/" @{
    mode = "classic"
    mapId = "DBA_Arena_Test"
    region = "local"
    maxPlayers = 2
    visibility = "public"
    password = $null
} $guestA.Token) "create room"
$RoomId = [string]$room.id
Write-Step ("room created: roomId={0}" -f $RoomId)

Assert-Ok (Invoke-Json "GET" "/api/rooms/?mode=classic&region=local") "list rooms"
Assert-Ok (Invoke-Json "GET" "/api/rooms/$RoomId") "get room"
Assert-Ok (Invoke-Json "POST" "/api/rooms/$RoomId/join" @{ password = $null } $guestB.Token) "join room B"
Assert-Ok (Invoke-Json "POST" "/api/rooms/$RoomId/ready" @{ isReady = $true } $guestB.Token) "ready B"

$startedRoom = Get-EnvelopeData (Invoke-Json "POST" "/api/rooms/$RoomId/start" $null $guestA.Token) "start room"
Write-Step ("room started: status={0}, players={1}" -f $startedRoom.status, $startedRoom.players.Count)

$session = Get-EnvelopeData (Invoke-Json "POST" "/internal/sessions/from-room" @{ roomId = $RoomId }) "create session from room"
$SessionId = [string]$session.id
Write-Step ("session created: sessionId={0}, status={1}" -f $SessionId, $session.status)

$runtimeToken = "codex-runtime-$RunShort"
$allocated = Get-EnvelopeData (Invoke-Json "POST" "/internal/sessions/$SessionId/allocate-server" @{
    sessionId = $SessionId
    ip = "127.0.0.1"
    port = 7777
    runtimeToken = $runtimeToken
}) "allocate session server"
Write-Step ("session allocated: endpoint={0}:{1}, status={2}" -f $allocated.serverIp, $allocated.serverPort, $allocated.status)

$connectionA = Get-EnvelopeData (Invoke-Json "GET" "/api/sessions/$SessionId/connection" $null $guestA.Token) "connection A"
$connectionB = Get-EnvelopeData (Invoke-Json "GET" "/api/sessions/$SessionId/connection" $null $guestB.Token) "connection B"
if ([string]::IsNullOrWhiteSpace($connectionA.playerSessionToken) -or [string]::IsNullOrWhiteSpace($connectionB.playerSessionToken)) {
    throw "session connection did not return player tokens"
}
Write-Step "session connections returned for both players"

$ticket = Get-EnvelopeData (Invoke-Json "POST" "/api/matchmaking/tickets" @{
    mode = "classic"
    region = "local"
    mmr = 1000
} $guestA.Token) "create matchmaking ticket"
$ticketId = [string]$ticket.id
Assert-Ok (Invoke-Json "GET" "/api/matchmaking/tickets/$ticketId" $null $guestA.Token) "get matchmaking ticket"
Assert-Ok (Invoke-Json "DELETE" "/api/matchmaking/tickets/$ticketId" $null $guestA.Token) "cancel matchmaking ticket"
Write-Step ("matchmaking ticket created and cancelled: ticketId={0}" -f $ticketId)

Assert-Ok (Invoke-Json "POST" "/api/rooms/$RoomId/leave" $null $guestB.Token) "leave room B"
Write-Step "flow smoke completed"

[pscustomobject]@{
    BaseUrl = $BaseUrl
    GuestAPlayerId = $guestA.PlayerId
    GuestBPlayerId = $guestB.PlayerId
    RoomId = $RoomId
    SessionId = $SessionId
    ServerEndpoint = "$($allocated.serverIp):$($allocated.serverPort)"
}
