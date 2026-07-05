param(
    [string]$BaseUrl = "http://localhost:8083",
    [string]$UnrealRoot = "D:\UnrealEngine-5.8.0-release",
    [string]$InternalApiKey = "DEV-INTERNAL-API-KEY-MIN-32-CHARS-ONLY",
    [string]$ProjectPath = "",
    [switch]$UsePackagedServer,
    [string]$ServerExePath = "",
    [string]$PackagedRoot = "",
    [int]$TimeoutSec = 15,
    [int]$BackendProbeTimeoutSec = 60,
    [int]$BackendProbeIntervalMs = 1000,
    [int]$ServerBootWaitSec = 25,
    [int]$ClientStartWaitMs = 3000,
    [int]$ClientValidationWaitSec = 30,
    [switch]$SkipClientLaunch,
    [switch]$KeepProcessesAlive,
    [string]$EvidenceDir = "",
    [string]$RunId = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $RepoRoot "DBA_GameClient\DivineBeastsArena.uproject"
}
$LogDir = Join-Path $RepoRoot ".tmp\local-ue-validation"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

function Write-Step([string]$Message) {
    Write-Host ("[ue-flow] " + $Message)
}


function Ensure-InternalApiKey([string]$InternalApiKey, [hashtable]$Headers) {
    if ($Headers.Count -eq 0 -or [string]::IsNullOrWhiteSpace($InternalApiKey)) {
        throw "Internal API key is required for managed server allocation/release in this flow."
    }
}

function Ensure-BackendReady([string]$Url, [int]$TimeoutSec, [int]$PollIntervalMs) {
    $uri = [uri]$Url
    $backendHost = $uri.Host
    $port = if ($uri.Port -gt 0) { $uri.Port } else { if ($uri.Scheme -eq "https") { 443 } else { 80 } }
    $deadline = (Get-Date).AddSeconds($TimeoutSec)

    Write-Step "waiting for backend tcp [$backendHost]:$port"
    while ((Get-Date) -lt $deadline) {
        try {
            $test = Test-NetConnection -ComputerName $backendHost -Port $port -WarningAction SilentlyContinue
            if ($test.TcpTestSucceeded) {
                return
            }
        }
        catch {
            # continue retry
        }

        Start-Sleep -Milliseconds $PollIntervalMs
    }

    throw "backend endpoint not reachable within ${TimeoutSec}s: $Url. Run .\scripts\diagnose-local-ue-online-readiness.ps1 to inspect Docker, Compose, health endpoints, and UE paths."
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

function Ensure-Character($Guest, [string]$Zodiac, [string]$PrimaryElement, [string]$FiveCamp) {
    $characters = Invoke-Json "GET" "/api/account/characters" $null $Guest.Token
    Assert-Ok $characters "list characters $($Guest.Suffix)"

    $characterId = [string]$characters.selectedCharacterId
    if ([string]::IsNullOrWhiteSpace($characterId) -and $characters.characters.Count -gt 0) {
        $characterId = [string]$characters.characters[0].characterId
    }
    if ([string]::IsNullOrWhiteSpace($characterId)) {
        $created = Invoke-Json "POST" "/api/account/characters" @{
            characterName = "UE_$($Guest.Suffix)_$RunNameSuffix"
            zodiac = $Zodiac
            primaryElement = $PrimaryElement
            fiveCamp = $FiveCamp
        } $Guest.Token
        Assert-Ok $created "create character $($Guest.Suffix)"
        $characterId = [string]$created.character.characterId
    }

    Assert-Ok (Invoke-Json "POST" "/api/account/characters/$characterId/select" $null $Guest.Token) "select character $($Guest.Suffix)"
    return $characterId
}

function New-PlayerConnectString($Guest, $Connection, [string]$HostPort, [string]$FallbackZodiac, [string]$FallbackElement, [string]$FallbackFiveCamp) {
    $build = $Connection.characterBuildSummary
    $zodiac = if ($null -ne $build -and -not [string]::IsNullOrWhiteSpace([string]$build.zodiac)) { [string]$build.zodiac } else { $FallbackZodiac }
    $element = if ($null -ne $build -and -not [string]::IsNullOrWhiteSpace([string]$build.primaryElement)) { [string]$build.primaryElement } else { $FallbackElement }
    $fiveCamp = if ($null -ne $build -and -not [string]::IsNullOrWhiteSpace([string]$build.fiveCamp)) { [string]$build.fiveCamp } else { $FallbackFiveCamp }
    $fixedSkillGroupId = if ($null -ne $build -and -not [string]::IsNullOrWhiteSpace([string]$build.fixedSkillGroupId)) { [string]$build.fixedSkillGroupId } else { "$($zodiac)_$($element)" }

    $query = @(
        "PlayerId=$([uri]::EscapeDataString([string]$Guest.PlayerId))",
        "PlayerSessionToken=$([uri]::EscapeDataString([string]$Connection.playerSessionToken))",
        "DBALobbyZodiac=$([uri]::EscapeDataString($zodiac))",
        "DBAZodiac=$([uri]::EscapeDataString($zodiac))",
        "DBAElement=$([uri]::EscapeDataString($element))",
        "DBAFiveCamp=$([uri]::EscapeDataString($fiveCamp))",
        "DBAFixedSkillGroupId=$([uri]::EscapeDataString($fixedSkillGroupId))"
    ) -join "&"

    return "$($HostPort)?$query"
}

function Wait-LogPattern {
    param(
        [string]$Path,
        [string]$Pattern,
        [int]$TimeoutSec = 30
    )
    if ([string]::IsNullOrWhiteSpace($Path)) {
        throw "Wait-LogPattern requires a non-empty log path."
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    while ((Get-Date) -lt $deadline) {
        if (Test-Path -LiteralPath $Path) {
            $content = Get-Content -Path $Path -Tail 150 -ErrorAction SilentlyContinue
            if ($content | Select-String -SimpleMatch -Pattern "Fatal error", "Critical error", "Assertion failed", "LogWindows: Error") {
                throw "server log contains critical errors in: $Path"
            }
            if ($content | Select-String -SimpleMatch -Pattern $Pattern) {
                return
            }
        }
        Start-Sleep -Seconds 1
    }

    $tail = if (Test-Path -LiteralPath $Path) { Get-Content -Path $Path -Tail 80 -ErrorAction SilentlyContinue } else { @() }
    throw "waited ${TimeoutSec}s for server log pattern `"$Pattern`" but not found. log tail:`n$($tail -join "`n")"
}

function Get-ServerListenPort {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    $match = Get-Content -Path $Path -Tail 200 -ErrorAction SilentlyContinue |
        Select-String -Pattern "IpNetDriver listening on port (?<port>\d+)" |
        Select-Object -Last 1
    if ($match -and $match.Matches.Count -gt 0) {
        return [int]$match.Matches[0].Groups["port"].Value
    }

    return $null
}

function Resolve-PackagedServerExe {
    param(
        [string]$ExplicitServerExePath,
        [string]$ExplicitPackagedRoot,
        [string]$ProjectDirectory,
        [string]$RepositoryRoot
    )

    if (-not [string]::IsNullOrWhiteSpace($ExplicitServerExePath)) {
        if (-not (Test-Path -LiteralPath $ExplicitServerExePath)) {
            throw "Packaged server executable not found: $ExplicitServerExePath"
        }
        return (Resolve-Path -LiteralPath $ExplicitServerExePath).ProviderPath
    }

    $roots = @()
    if (-not [string]::IsNullOrWhiteSpace($ExplicitPackagedRoot)) {
        $roots += $ExplicitPackagedRoot
    }
    $roots += @(
        (Join-Path $RepositoryRoot ".tmp\packaged-server"),
        (Join-Path $ProjectDirectory "Saved\StagedBuilds"),
        (Join-Path $RepositoryRoot "Artifacts\UnrealServer")
    )

    $candidates = @()
    foreach ($root in $roots) {
        if (-not (Test-Path -LiteralPath $root)) {
            continue
        }
        $candidates += Get-ChildItem -LiteralPath $root -Recurse -File -Filter "DivineBeastsArenaServer.exe" -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty FullName
    }

    if (-not $candidates -or $candidates.Count -eq 0) {
        throw "No staged packaged server executable found. Build/stage one first, or pass -ServerExePath. Checked: $($roots -join ', ')"
    }

    return ($candidates | Sort-Object | Select-Object -First 1)
}

function Test-CookedServerContent {
    param([string]$ServerExe)

    $cursor = Split-Path -Parent $ServerExe
    for ($i = 0; $i -lt 8 -and -not [string]::IsNullOrWhiteSpace($cursor); $i++) {
        $pakFiles = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "*.pak" -ErrorAction SilentlyContinue | Select-Object -First 1
        $assetRegistry = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "AssetRegistry.bin" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($pakFiles -or $assetRegistry) {
            return $true
        }

        $parent = Split-Path -Parent $cursor
        if ($parent -eq $cursor) {
            break
        }
        $cursor = $parent
    }

    return $false
}

function Assert-LogPatternCount {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Pattern,
        [Parameter(Mandatory = $true)][int]$MinimumCount,
        [Parameter(Mandatory = $true)][string]$Name
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "$Name failed because log file was not found: $Path"
    }

    $matches = Select-String -LiteralPath $Path -Pattern $Pattern -CaseSensitive:$false -ErrorAction SilentlyContinue
    $count = @($matches).Count
    if ($count -lt $MinimumCount) {
        $tail = Get-Content -Path $Path -Tail 80 -ErrorAction SilentlyContinue
        throw "$Name expected at least $MinimumCount match(es) for pattern `"$Pattern`", found $count. log tail:`n$($tail -join "`n")"
    }
}

function Assert-LogDoesNotContain {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string[]]$Patterns,
        [Parameter(Mandatory = $true)][string]$Name
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "$Name failed because log file was not found: $Path"
    }

    foreach ($pattern in $Patterns) {
        $match = Select-String -LiteralPath $Path -Pattern $pattern -CaseSensitive:$false -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($match) {
            throw "$Name found blocked pattern `"$pattern`" in $Path`: $($match.Line)"
        }
    }
}

function Resolve-LogPath([string]$FileName) {
    if ([string]::IsNullOrWhiteSpace($FileName)) {
        throw "Resolve-LogPath requires a non-empty file name."
    }

    $resolved = Join-Path $LogDir $FileName
    if ([string]::IsNullOrWhiteSpace($resolved)) {
        throw "Failed to resolve a valid log path for $FileName"
    }

    $logDir = Split-Path -Parent $resolved
    if (-not (Test-Path -LiteralPath $logDir)) {
        New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    }

    if (-not (Test-Path -LiteralPath $resolved)) {
        New-Item -ItemType File -Path $resolved -Force | Out-Null
    }

    return $resolved
}

function Stop-IfRunning([System.Diagnostics.Process]$ProcessObj, [string]$Tag) {
    if (-not $ProcessObj) {
        return
    }

    try {
        $running = Get-Process -Id $ProcessObj.Id -ErrorAction SilentlyContinue
        if ($running) {
            Stop-Process -Id $ProcessObj.Id -Force
            Write-Step "stopped $Tag (pid=$($ProcessObj.Id))"
        }
    }
    catch {
        Write-Host "warning: failed to stop $Tag process $($ProcessObj.Id): $($_.Exception.Message)" -ForegroundColor Yellow
    }
}

function Get-SafeLogMatches {
    param(
        [string]$Path,
        [string]$Pattern,
        [int]$MaximumCount = 10
    )

    if ([string]::IsNullOrWhiteSpace($Path) -or -not (Test-Path -LiteralPath $Path)) {
        return @()
    }

    return @(
        Select-String -LiteralPath $Path -Pattern $Pattern -CaseSensitive:$false -ErrorAction SilentlyContinue |
            Select-Object -First $MaximumCount |
            ForEach-Object { $_.Line }
    )
}

function Write-UeValidationEvidence {
    param(
        [Parameter(Mandatory = $true)]$Result,
        [Parameter(Mandatory = $true)][string]$Status,
        [string]$ErrorMessage = ""
    )

    if ([string]::IsNullOrWhiteSpace($EvidenceDir)) {
        return
    }

    $unrealEvidenceDir = Join-Path $EvidenceDir "unreal"
    if (-not (Test-Path -LiteralPath $unrealEvidenceDir)) {
        New-Item -ItemType Directory -Force -Path $unrealEvidenceDir | Out-Null
    }

    $safeServerJoinedLines = Get-SafeLogMatches -Path $Result.ServerLog -Pattern "Runtime player-joined 请求已发送"
    $safeServerJoinedOkLines = Get-SafeLogMatches -Path $Result.ServerLog -Pattern "runtime/servers/player-joined.*=OK"
    $safeTravelLinesA = Get-SafeLogMatches -Path $Result.ClientALog -Pattern "TravelCompleted Pending net game travel completed" -MaximumCount 3
    $safeTravelLinesB = Get-SafeLogMatches -Path $Result.ClientBLog -Pattern "TravelCompleted Pending net game travel completed" -MaximumCount 3

    $evidence = [ordered]@{
        schemaVersion = "1.0"
        kind = "ue-online-validation"
        runId = $Result.RunId
        status = $Status
        errorMessage = $ErrorMessage
        generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
        baseUrl = $BaseUrl
        usePackagedServer = [bool]$UsePackagedServer
        skipClientLaunch = [bool]$SkipClientLaunch
        keepProcessesAlive = [bool]$KeepProcessesAlive
        serverExecutable = if ($UsePackagedServer) { $ServerExe } else { "UnrealEditor.exe -server" }
        roomId = $Result.RoomId
        sessionId = $Result.SessionId
        serverId = $Result.ServerId
        allocatedPort = $Result.Port
        clientConnectPort = $Result.ClientConnectPort
        processIds = [ordered]@{
            server = $Result.ServerProcessId
            clientA = $Result.ClientAProcessId
            clientB = $Result.ClientBProcessId
        }
        logPaths = [ordered]@{
            server = $Result.ServerLog
            clientA = $Result.ClientALog
            clientB = $Result.ClientBLog
        }
        safeLogEvidence = [ordered]@{
            runtimePlayerJoined = $safeServerJoinedLines
            runtimePlayerJoinedOk = $safeServerJoinedOkLines
            clientATravelCompleted = $safeTravelLinesA
            clientBTravelCompleted = $safeTravelLinesB
        }
    }

    $evidencePath = Join-Path $unrealEvidenceDir ("ue-online-validation-{0}.json" -f $Result.RunId)
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidencePath -Encoding UTF8
    Write-Step "wrote UE online validation evidence: $evidencePath"
}

$RunShort = if ([string]::IsNullOrWhiteSpace($RunId)) {
    ([guid]::NewGuid().ToString("N")).Substring(0, 8)
}
else {
    ($RunId -replace "[^A-Za-z0-9_.-]", "-")
}
$RunNameSuffixSource = $RunShort -replace "[^A-Za-z0-9_]", "_"
$RunNameSuffix = $RunNameSuffixSource.Substring(0, [Math]::Min(8, $RunNameSuffixSource.Length))
$EditorExe = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$ProjectDir = Split-Path -Parent $ProjectPath
$ServerExe = if ($UsePackagedServer) {
    Resolve-PackagedServerExe `
        -ExplicitServerExePath $ServerExePath `
        -ExplicitPackagedRoot $PackagedRoot `
        -ProjectDirectory $ProjectDir `
        -RepositoryRoot $RepoRoot
}
else {
    $EditorExe
}
$ServerWorkingDir = if ($UsePackagedServer) { Split-Path -Parent $ServerExe } else { $ProjectDir }

if (-not (Test-Path -LiteralPath $EditorExe)) {
    throw "UnrealEditor.exe not found: $EditorExe"
}
if ($UsePackagedServer -and -not (Test-Path -LiteralPath $ServerExe)) {
    throw "DivineBeastsArenaServer.exe not found: $ServerExe"
}
if ($UsePackagedServer -and -not (Test-CookedServerContent -ServerExe $ServerExe)) {
    throw "Packaged server content was not found near $ServerExe. Run .\scripts\package-unreal-dedicated-server.ps1 before packaged online validation."
}

$result = [pscustomobject]@{
    RunId = $RunShort
    RoomId = ""
    SessionId = ""
    ServerId = ""
    Port = 0
    ClientConnectPort = 0
    ServerProcessId = $null
    ClientAProcessId = $null
    ClientBProcessId = $null
    ServerLog = ""
    ClientALog = ""
    ClientBLog = ""
}

$serverProcess = $null
$clientAProcess = $null
$clientBProcess = $null
$allocatedServerId = ""
$allocation = $null
$validationStatus = "failed"
$validationErrorMessage = ""

$internalHeaders = @{}
if (-not [string]::IsNullOrWhiteSpace($InternalApiKey)) {
    $internalHeaders = @{ "X-Internal-Api-Key" = $InternalApiKey }
}

try {
    Ensure-BackendReady -Url $BaseUrl -TimeoutSec $BackendProbeTimeoutSec -PollIntervalMs $BackendProbeIntervalMs
    Ensure-InternalApiKey -InternalApiKey $InternalApiKey -Headers $internalHeaders

    Write-Step "creating backend room/session"
    $guestA = New-Guest "A"
    $guestB = New-Guest "B"
    $null = Ensure-Character $guestA "Rat" "Water" "East"
    $null = Ensure-Character $guestB "Tiger" "Fire" "North"

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

    $allocation = Get-Data (Invoke-Json "POST" "/internal/game-servers/allocate" @{
            sessionId = $sessionId
            mode = "classic"
            mapId = "LobbyMap"
            region = "local"
            buildVersion = "local-validation"
        } $null $internalHeaders) "allocate managed server"

    if ([string]::IsNullOrWhiteSpace($allocation.runtimeToken)) {
        throw "managed server allocation did not return runtimeToken; release stale active servers or allocate a new session"
    }

    $serverLog = Resolve-LogPath "server-$RunShort.log"
    $clientALog = Resolve-LogPath "client-a-$RunShort.log"
    $clientBLog = Resolve-LogPath "client-b-$RunShort.log"

    $result.RoomId = $roomId
    $result.SessionId = $sessionId
    $result.ServerId = [string]$allocation.serverId
    $result.Port = [int]$allocation.port
    $result.ClientConnectPort = [int]$allocation.port
    $result.ServerLog = $serverLog
    $result.ClientALog = $clientALog
    $result.ClientBLog = $clientBLog
    $allocatedServerId = [string]$allocation.serverId

    Write-Step "starting dedicated server on $($allocation.publicIp):$($allocation.port)"
    $serverRuntimeArgs = @(
            "-log",
            "-abslog=`"$serverLog`"",
            "-port=$($allocation.port)",
            "-sessionId=$sessionId",
            "-serverId=$($allocation.serverId)",
            "-runtimeToken=`"$($allocation.runtimeToken)`"",
            "-backendUrl=$BaseUrl",
            "-DBAHeadlessLobbyServer"
        )
    $serverArgs = if ($UsePackagedServer) {
        $serverRuntimeArgs + @("/Game/Maps/Lobby/LobbyMap")
    }
    else {
        @(
            "`"$ProjectPath`"",
            "/Game/Maps/Lobby/LobbyMap",
            "-server"
        ) + $serverRuntimeArgs
    }
    $serverProcess = Start-Process -FilePath $ServerExe -ArgumentList $serverArgs -PassThru -WorkingDirectory $ServerWorkingDir
    $result.ServerProcessId = $serverProcess.Id

    Wait-LogPattern -Path $serverLog -Pattern "Load map complete /Game/Maps/Lobby/LobbyMap" -TimeoutSec $ServerBootWaitSec
    Assert-LogDoesNotContain -Path $serverLog -Patterns @("LobbyMap-log", "LobbyMap-abslog", "Would you like to load the default map instead?") -Name "server startup map validation"
    $listenPort = Get-ServerListenPort -Path $serverLog
    if ($listenPort -and $listenPort -ne [int]$allocation.port) {
        Write-Step "server listen port differs from allocation port; clients will use detected port $listenPort"
        $result.ClientConnectPort = $listenPort
    }

    if ($SkipClientLaunch) {
        Write-Step "client launch skipped by -SkipClientLaunch"
        $validationStatus = "passed"
        return $result
    }

    $connA = Get-Data (Invoke-Json "GET" "/api/sessions/$sessionId/connection" $null $guestA.Token) "connection A"
    $connB = Get-Data (Invoke-Json "GET" "/api/sessions/$sessionId/connection" $null $guestB.Token) "connection B"

    Write-Step "starting client A and client B windows"
    $connectHostPort = "127.0.0.1:$($result.ClientConnectPort)"
    $connectA = New-PlayerConnectString $guestA $connA $connectHostPort "Rat" "Water" "East"
    $connectB = New-PlayerConnectString $guestB $connB $connectHostPort "Tiger" "Fire" "North"

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
    $result.ClientAProcessId = $clientAProcess.Id
    Start-Sleep -Milliseconds $ClientStartWaitMs
    $clientBProcess = Start-Process -FilePath $EditorExe -ArgumentList $clientArgsB -PassThru -WorkingDirectory (Split-Path -Parent $ProjectPath)
    $result.ClientBProcessId = $clientBProcess.Id
    if ($ClientValidationWaitSec -gt 0) {
        Write-Step "observing client processes for ${ClientValidationWaitSec}s"
        Start-Sleep -Seconds $ClientValidationWaitSec
    }

    Assert-LogPatternCount -Path $serverLog -Pattern "Join succeeded" -MinimumCount 2 -Name "server two-client join validation"
    Assert-LogPatternCount -Path $serverLog -Pattern "runtime/servers/player-joined" -MinimumCount 2 -Name "server player-joined runtime validation"
    Assert-LogPatternCount -Path $serverLog -Pattern "runtime/servers/player-joined.*=OK" -MinimumCount 2 -Name "server player-joined backend success validation"
    Assert-LogDoesNotContain -Path $serverLog -Patterns @("runtime/servers/player-joined.*=ERROR") -Name "server player-joined backend failure scan"
    Assert-LogDoesNotContain -Path $serverLog -Patterns @("LobbyMap-log", "LobbyMap-abslog", "Would you like to load the default map instead?") -Name "server map fallback scan"
    Assert-LogPatternCount -Path $clientALog -Pattern "TravelCompleted Pending net game travel completed" -MinimumCount 1 -Name "client A travel validation"
    Assert-LogPatternCount -Path $clientBLog -Pattern "TravelCompleted Pending net game travel completed" -MinimumCount 1 -Name "client B travel validation"
    Assert-LogDoesNotContain -Path $clientALog -Patterns @("ConnectionTimeout", "NetworkFailure", "Fatal error", "Assertion failed") -Name "client A failure scan"
    Assert-LogDoesNotContain -Path $clientBLog -Patterns @("ConnectionTimeout", "NetworkFailure", "Fatal error", "Assertion failed") -Name "client B failure scan"
    Write-Step "two-client UE validation passed"
    $validationStatus = "passed"
    return $result
}
catch {
    $validationStatus = "failed"
    $validationErrorMessage = $_.Exception.Message
    throw
}
finally {
    Write-UeValidationEvidence -Result $result -Status $validationStatus -ErrorMessage $validationErrorMessage

    if (-not $KeepProcessesAlive) {
        if (-not [string]::IsNullOrWhiteSpace($allocatedServerId)) {
            if ($internalHeaders.Count -gt 0) {
                try {
                    Write-Step "releasing managed server allocation $allocatedServerId"
                    Invoke-Json "POST" "/internal/game-servers/$allocatedServerId/release" @{
                        reason = "local ue validation cleanup"
                    } "" $internalHeaders | Out-Null
                }
                catch {
                    Write-Host "warning: failed to release managed server ${allocatedServerId}: $($_.Exception.Message)" -ForegroundColor Yellow
                }
            }
            else {
                Write-Host "warning: internal api key not provided, skip managed server release for $allocatedServerId" -ForegroundColor Yellow
            }
        }

        Stop-IfRunning $clientBProcess "UE validation client B"
        Stop-IfRunning $clientAProcess "UE validation client A"
        Stop-IfRunning $serverProcess "UE validation server"
    }
}
