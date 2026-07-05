<#
Validates the Runtime match lifecycle and settlement handoff contract.

This lightweight source contract keeps the MVP Dedicated Server lifecycle
stable without launching Unreal or external services.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath
    $missing = @($RequiredSymbols | Where-Object { $content -notmatch [regex]::Escape($_) })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing Runtime match lifecycle contract symbols: $($missing -join ', ')"
    }
}

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\GameServer\GameServerDtos.cs" @(
    "RuntimeMatchStartedRequest",
    "RuntimeMatchEndedRequest",
    "RuntimeMatchResultsRequest",
    "RuntimePlayerResultDto",
    "IdempotencyKey",
    "ExpDelta",
    "Rewards"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Runtime\RuntimeEndpoints.cs" @(
    'runtimeServers.MapPost("/match-started", RuntimeMatchStarted)',
    'runtimeServers.MapPost("/match-ended", RuntimeMatchEnded)',
    'internalGroup.MapPost("/servers", AllocateServer)',
    'internalGroup.MapGet("/servers/{serverId}", GetServer)',
    'app.MapPost("/runtime/matches/results", RuntimeMatchResults)',
    "InternalApiKeyEndpointFilter.Validate(httpContext)",
    "if (unauthorized is not null) return unauthorized",
    "RuntimeLifecycleService.MarkMatchStartedAsync",
    "RuntimeLifecycleService.MarkMatchEndedAsync",
    "RuntimeMatchResults",
    "string.IsNullOrWhiteSpace(playerSession.SessionTokenHash)",
    "playerSession.SessionTokenExpiresAt <= DateTimeOffset.UtcNow",
    "string.IsNullOrWhiteSpace(request.PlayerSessionToken)",
    "Invalid player session token",
    "RuntimePlayerLeft",
    "if (playerSession is null) return ErrorResponse.NotFound(ErrorCodes.SessionPlayerNotInSession).ToProblem()",
    "if (playerSession.LeftAt is not null)",
    'playerSession.JoinedAt is null || playerSession.Status != "JOINED"',
    "Player has not joined session",
    'AddSessionEvent(db, request.SessionId, "PLAYER_LEFT"',
    "RuntimeMatchResultsValidator.ValidateAndBuildPayload",
    ".Select(x => new { x.PlayerId, x.Team })",
    "ToDictionaryAsync(x => x.PlayerId, x => x.Team)",
    "settlement.SubmitMatchResultAsync(validation.Payload)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Session\SessionEndpoints.cs" @(
    'internalGroup.MapPost("/from-room", CreateFromRoom)',
    'internalGroup.MapPost("/{sessionId}/allocate-server", AllocateServer)',
    'internalGroup.MapPost("/{sessionId}/mark-in-progress", MarkInProgress)',
    "InternalApiKeyEndpointFilter.Validate(httpContext)",
    "if (unauthorized is not null) return unauthorized",
    "GetConnection(",
    "RequireAuthorization()"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Program.cs" @(
    "public partial class Program"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Extensions\InternalApiKeyEndpointFilter.cs" @(
    "InternalApiKeyEndpointFilter",
    'HeaderName = "X-Internal-Api-Key"',
    "RequireInternalApiKey",
    "Validate(HttpContext httpContext)",
    'configuration["InternalApi:Key"]',
    "Invalid internal api key"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\GameServer\GameServerEndpoints.cs" @(
    'app.MapGroup("/internal/game-servers")',
    'app.MapGroup("/internal/servers")',
    "InternalApiKeyEndpointFilter.RequireInternalApiKey",
    "InternalApiKeyEndpointFilter.Validate(httpContext)",
    "Register(",
    "GetServer(",
    "GetActiveServers("
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Runtime\RuntimeLifecycleService.cs" @(
    "RuntimeLifecycleService",
    "MarkMatchStartedAsync",
    "MarkMatchEndedAsync",
    "x.Id == serverId && x.SessionId == sessionId",
    "AddSessionEventOnceAsync",
    "AddServerEventOnceAsync",
    "AnyAsync(x => x.GameSessionId == sessionId && x.EventType == eventType)",
    "AnyAsync(x => x.ServerId == serverId && x.EventType == eventType)",
    "hasActivePlayerNotJoined",
    "x.LeftAt == null",
    'x.JoinedAt == null || x.Status != "JOINED"',
    'session.Status is not ("IN_PROGRESS" or "SETTLING")',
    'server.Status is not ("IN_PROGRESS" or "ENDING")',
    'session.Status = "IN_PROGRESS"',
    'server.Status = "IN_PROGRESS"',
    'session.Status = "SETTLING"',
    'server.Status = "ENDING"',
    'AddSessionEventOnceAsync(db, sessionId, "MATCH_STARTED", "{}")',
    'AddServerEventOnceAsync(db, serverId, "MATCH_STARTED", "{}")',
    'AddSessionEventOnceAsync(db, sessionId, "MATCH_ENDED", "{}")',
    'AddServerEventOnceAsync(db, serverId, "MATCH_ENDED", "{}")'
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Runtime\RuntimeMatchResultsValidator.cs" @(
    "RuntimeMatchResultsValidator",
    "ValidateAndBuildPayload",
    "MissingIdempotencyKeyMessage",
    "MissingPlayersMessage",
    "DuplicatePlayersMessage",
    "UnknownPlayersMessage",
    "MissingSessionPlayersMessage",
    "MissingPlayerTeamMessage",
    "TeamMismatchMessage",
    "InvalidPlayerResultMessage",
    "InvalidPlayerNumericValueMessage",
    "IReadOnlyDictionary<Guid, string?> sessionPlayerTeams",
    "sessionPlayerTeams.ContainsKey",
    "NormalizeTeam",
    "NormalizePlayerResult(player.Result)",
    "!IsValidPlayerResult",
    "HasValidNonNegativePlayerValues(player)",
    "IsNonNegativeRewardQuantity",
    "player.Rewards.All",
    "NormalizePlayerResult(x.Result)",
    'result is "win" or "loss" or "draw"',
    "SettlementSubmitMatchResultRequest",
    "JsonSerializer.Serialize(request)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Settlement\SettlementService.cs" @(
    "SubmitMatchResultAsync",
    "IdempotencyKey",
    "string.IsNullOrWhiteSpace(request.IdempotencyKey)",
    "request.IdempotencyKey.Trim()",
    "existingForSession",
    "existingForIdempotencyKey",
    "x.SessionId == request.SessionId",
    "x.IdempotencyKey == idempotencyKey",
    "IdempotencyKey = idempotencyKey",
    "submittedPlayers",
    "sessionPlayers.Count == 0 || request.Players.Count == 0",
    "request.Players.Count != submittedPlayers.Count",
    "SetEquals",
    "playerResults",
    "NormalizePlayerResult(x.Result)",
    "!IsValidPlayerResult(x)",
    "HasValidNonNegativePlayerValues(x)",
    "IsNonNegativeRewardQuantity",
    "player.Rewards.All",
    "Result = playerResults[player.PlayerId]",
    'result is "win" or "loss" or "draw"',
    "sessionPlayerTeams",
    "NormalizeTeam(x.Team)",
    "sessionPlayerTeams[x.PlayerId].Length == 0",
    'session.Status != "SETTLING"',
    'session.Status = "COMPLETED"',
    'server.Status = "ENDING"',
    'EventType = "SETTLEMENT_COMPLETED"',
    "GetMatchResultAsync",
    "GetSessionResultsAsync",
    "Include(x => x.PlayerResults)",
    "OrderByDescending(x => x.CreatedAt)",
    "GrantRewardsAsync",
    "TotalMatches",
    "Wins"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Endpoints\Settlement\SettlementEndpoints.cs" @(
    'internalGroup.MapGet("/sessions/{sessionId}/matches/results", GetSessionResults)',
    "GetSessionResults(",
    "Guid sessionId",
    "InternalApiKeyEndpointFilter.Validate(httpContext)",
    "if (unauthorized is not null) return unauthorized",
    "svc.GetSessionResultsAsync(sessionId)",
    "ToResponse(result)",
    "result.PlayerResults.Select"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\SettlementInventoryServiceTests.cs" @(
    "SubmitMatchResultAsync_RepeatedSubmission_DoesNotDoubleGrantRewardsOrStats",
    "SubmitMatchResultAsync_WhenIdempotencyKeyIsBlank_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenMatchHasNotEnded_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenIdempotencyKeyBelongsToOtherSession_ReturnsNullWithoutReusingOtherResult",
    "SubmitMatchResultAsync_WhenSessionPlayerIsMissing_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerIsDuplicated_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenNoSessionPlayersExist_ReturnsNullWithoutCompletingSession",
    "SubmitMatchResultAsync_WhenPlayerTeamDiffersFromSession_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerResultIsInvalid_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenPlayerStatsContainNegativeValue_ReturnsNullWithoutRewardsOrStats",
    "SubmitMatchResultAsync_WhenRewardQuantityIsNegative_ReturnsNullWithoutRewardsOrStats",
    "GetMatchResultAsync_ReturnsPlayerResults",
    "GetSessionResultsAsync_ReturnsLatestFirstWithPlayerResults",
    "SeedSettlingSession",
    "SeedSettlingSessionWithoutPlayers",
    "SeedAdditionalPlayer",
    "CreateSettlementRequest",
    "result-001",
    "result-001-retry",
    "result-before-match-ended",
    "result-missing-session-player",
    "result-duplicate-player",
    "result-empty-session-players",
    "result-team-mismatch",
    "result-invalid-player-result",
    "result-negative-player-stats",
    "result-negative-reward",
    "shared-result-key",
    "Assert.Equal(1, await db.MatchResults.CountAsync",
    "Assert.Equal(0, await db.MatchResults.CountAsync(x => x.SessionId == secondSessionId)",
    "Assert.Equal(1, await db.MatchPlayerResults.CountAsync",
    "Assert.Null(result)",
    "Assert.Equal(0, await db.InventoryLogs.CountAsync",
    "Assert.Equal(0, await db.SessionEvents.CountAsync",
    "Assert.Equal(0, stats.TotalMatches)",
    "Assert.Equal(0, secondStats.TotalMatches)",
    "Assert.Equal(0, firstStats.TotalMatches)",
    'Assert.Equal("IN_PROGRESS", session.Status)',
    'Assert.Equal("SETTLING", session.Status)',
    'Assert.Equal("ENDING", server.Status)',
    'Assert.Equal("SETTLING", secondSession.Status)',
    "Assert.Null(session.EndedAt)",
    'Team = "blue"',
    'Assert.Equal("COMPLETED", session.Status)'
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\SettlementEndpointsTests.cs" @(
    "SettlementEndpointsTests",
    "GetSessionResults_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetSessionResults_WithWrongInternalApiKey_ReturnsUnauthorized",
    "GetSessionResults_ReturnsLatestResultsWithPlayerDetails",
    "SubmitResult_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutRewardsOrStats",
    "SubmitResult_WithJsonRewardQuantity_GrantsRewardsAndCompletesSession",
    "SubmitResult_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam",
    "SubmitResult_WhenRetried_ReturnsPlayerDetailsWithoutDoubleGrantingRewardsOrStats",
    "settlement-endpoint-negative-reward",
    "settlement-endpoint-json-reward",
    "settlement-endpoint-frozen-team",
    "settlement-endpoint-retry-json-reward",
    '"schema":"frozen-team-test"',
    "Assert.Equal(firstResponse.Data!.Id, retryResponse.Data!.Id)",
    '["coin"] = -5',
    "var responsePlayer = Assert.Single(response.Data.Players)",
    "var retryPlayer = Assert.Single(retryResponse.Data.Players)",
    'Assert.Equal(playerId, responsePlayer.PlayerId)',
    'Assert.Equal(playerId, retryPlayer.PlayerId)',
    'Assert.Equal("blue", responsePlayer.Team)',
    'Assert.Equal("win", responsePlayer.Result)',
    'Assert.Equal("blue", playerResult.Team)',
    'Assert.Equal("blue", history.Team)',
    'Assert.Equal(5, GetRewardInt(responsePlayer.Rewards, "coin"))',
    'Assert.Equal(5, GetRewardInt(retryPlayer.Rewards, "coin"))',
    'Assert.Equal(5, item.Quantity)',
    'Assert.Equal(1, await db.InventoryLogs.CountAsync',
    'Assert.Equal("COMPLETED", session.Status)',
    "Failed to submit match result",
    "InMemoryEventId.TransactionIgnoredWarning",
    "WebApplicationFactory<Program>",
    "RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>",
    "UseInMemoryDatabase(dbName)",
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    "401|Unauthorized|Invalid internal api key",
    '"/internal/settlement/sessions/{sessionId}/matches/results"',
    "ApiResponse<IReadOnlyList<MatchResultResponse>>",
    "Assert.Collection"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Settlement\SettlementService.cs" @(
    "using System.Text.Json",
    "existingForSession",
    "Include(x => x.PlayerResults)",
    "PlayerMatchHistories",
    "new PlayerMatchHistory",
    "Team = sessionPlayerTeams[player.PlayerId]",
    "Result = playerResults[player.PlayerId]",
    "DurationSeconds = duration",
    "JsonElement { ValueKind: JsonValueKind.Number } quantity",
    "quantity.TryGetInt64(out var longQuantity)",
    "quantity.TryGetDouble(out var doubleQuantity)"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Inventory\InventoryService.cs" @(
    "using System.Text.Json",
    "TryGetRewardQuantity",
    "JsonElement { ValueKind: JsonValueKind.Number }",
    "element.TryGetInt64(out var longQty)",
    "element.TryGetDouble(out var doubleQty)"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RuntimeEndpointsTests.cs" @(
    "RuntimeEndpointsTests",
    "GetInternalServer_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized",
    "GetInternalServer_WithInternalApiKey_ReachesRuntimeHandler",
    "RuntimePlayerJoined_WithoutPlayerSessionToken_ReturnsUnauthorizedWithoutJoining",
    "RuntimePlayerJoined_WithExpiredPlayerSessionToken_ReturnsUnauthorizedWithoutJoining",
    "PlayerSessionToken: null",
    "playerSessionTokenExpiresAt: DateTimeOffset.UtcNow.AddMinutes(-1)",
    'Assert.Equal("CONNECTED", playerSession.Status)',
    "Assert.Null(playerSession.JoinedAt)",
    'x.EventType == "PLAYER_JOINED"',
    "RuntimePlayerLeft_WhenPlayerIsNotInSession_ReturnsNotFoundWithoutWritingEvent",
    "RuntimePlayerLeft_WhenPlayerHasNotJoined_ReturnsBadRequestWithoutWritingEvent",
    "RuntimePlayerLeft_WhenRepeated_DoesNotRewriteLeftAtOrDuplicateEvent",
    'playerSessionStatus: "CONNECTED"',
    "Assert.Null(playerSession.LeftAt)",
    'JoinedAt = playerSessionStatus == "JOINED"',
    '"/runtime/servers/player-left"',
    "unknownPlayerId",
    "firstLeftAt",
    "Assert.Equal(1, await db.SessionEvents.CountAsync",
    'x.EventType == "PLAYER_LEFT"',
    "RuntimeMatchResults_WhenReportedTeamDiffersFromSession_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenIdempotencyKeyIsBlank_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerResultRowsAreDuplicated_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayersAreEmpty_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerTeamIsMissing_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerIsNotInSession_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenSessionPlayerIsMissing_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerResultIsInvalid_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenPlayerStatsContainNegativeValue_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutSettlement",
    "RuntimeMatchResults_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam",
    "RuntimeMatchResults_WithJsonRewardQuantity_GrantsRewardsStatsAndCompletesSession",
    "RuntimeMatchResults_WhenRetried_ReturnsSameResultWithoutDoubleGrantingRewardsOrStats",
    "RuntimeMatchResults_CanBeReadFromSettlementSessionResultsWithRewards",
    "runtime-token-blank-result-idempotency",
    "runtime-token-duplicate-result-player",
    "runtime-token-empty-result-players",
    "runtime-token-missing-result-team",
    "runtime-token-unknown-result-player",
    "runtime-token-missing-session-player",
    "match-result-duplicate-player",
    "match-result-empty-players",
    "match-result-missing-team",
    "match-result-unknown-player",
    "match-result-missing-session-player",
    "match-result-invalid-player-result",
    "match-result-negative-player-stats",
    "match-result-negative-reward",
    "runtime-token-positive-result-reward",
    "match-result-runtime-json-reward",
    "runtime-token-result-retry",
    "match-result-runtime-retry",
    "runtime-token-settlement-query",
    "match-result-runtime-query",
    "SeedPlayerProgressionAsync",
    "GetRewardInt",
    "ApiResponse<JsonElement>",
    "ApiResponse<IReadOnlyList<MatchResultResponse>>",
    'TryGetProperty("matchResultId", out var matchResultIdElement)',
    "matchResultIdElement.TryGetGuid(out var matchResultId)",
    "Assert.Equal(firstMatchResultId, retryMatchResultId)",
    '"/internal/settlement/sessions/{sessionId}/matches/results"',
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    'Assert.Contains("\"schema\":\"runtime-endpoint-test\"", matchResult.ResultJson)',
    'Assert.Equal(5, GetRewardInt(player.Rewards, "coin"))',
    "Array.Empty<RuntimePlayerResultDto>()",
    '"eliminated"',
    "ExpDelta: -1",
    '["coin"] = -5',
    "Match result idempotency key is required.",
    "Match result contains duplicate players.",
    "Match result must contain at least one player.",
    "Match result contains players without a team.",
    "Match result contains players not in session",
    "Match result is missing players from session.",
    "Match result contains an invalid player result.",
    "Match result contains an invalid player numeric value.",
    "Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode)",
    "Assert.Equal(0, await db.MatchResults.CountAsync())",
    "Assert.Equal(0, await db.MatchPlayerResults.CountAsync())",
    "Assert.Equal(5, item.Quantity)",
    "Assert.Equal(1, await db.InventoryLogs.CountAsync",
    "Assert.Equal(1200, profile.Exp)",
    "Assert.Equal(1, stats.TotalMatches)",
    'Assert.Equal("COMPLETED", session.Status)',
    'sessionStatus: "SETTLING"',
    'serverStatus: "ENDING"',
    "secondPlayerId: missingPlayerId",
    'secondTeam: "red"',
    "WebApplicationFactory<Program>",
    "RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>",
    "InMemoryDatabaseRoot",
    "UseInMemoryDatabase(dbName, dbRoot)",
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    '"/internal/runtime/servers/{Guid.NewGuid()}"',
    "401|Unauthorized|Invalid internal api key",
    "404|Not Found"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\SessionEndpointsTests.cs" @(
    "SessionEndpointsTests",
    "MarkInProgress_WithoutInternalApiKey_ReturnsUnauthorized",
    "MarkInProgress_WithWrongInternalApiKey_ReturnsUnauthorized",
    "MarkInProgress_WithInternalApiKey_ReachesSessionHandler",
    "WebApplicationFactory<Program>",
    "RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>",
    "UseInMemoryDatabase(dbName)",
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    '"/internal/sessions/{Guid.NewGuid()}/mark-in-progress"',
    "401|Unauthorized|Invalid internal api key",
    "404|Not Found"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\GameServerEndpointsTests.cs" @(
    "GameServerEndpointsTests",
    "ListManagedServers_WithoutInternalApiKey_ReturnsUnauthorized",
    "ListManagedServers_WithWrongInternalApiKey_ReturnsUnauthorized",
    "ListManagedServers_WithInternalApiKey_ReachesManagerHandler",
    "GetLegacyInternalServer_WithoutInternalApiKey_ReturnsUnauthorized",
    "GetLegacyInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized",
    "GetLegacyInternalServer_WithInternalApiKey_ReachesGameServerHandler",
    "WebApplicationFactory<Program>",
    "RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>",
    "UseInMemoryDatabase(dbName)",
    'client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey)',
    '"/internal/game-servers/"',
    '"/internal/servers/{Guid.NewGuid()}"',
    "401|Unauthorized|Invalid internal api key",
    "404|Not Found"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RuntimeMatchResultsValidatorTests.cs" @(
    "RuntimeMatchResultsValidatorTests",
    "ValidateAndBuildPayload_WithKnownPlayers_MapsSettlementPayload",
    "ValidateAndBuildPayload_WhenResultJsonIsBlank_UsesSerializedRuntimeRequest",
    "ValidateAndBuildPayload_WhenIdempotencyKeyIsBlank_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayersAreEmpty_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerIsNotInSession_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenSessionPlayerIsMissing_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerIsDuplicated_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerTeamIsMissing_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerTeamDoesNotMatchSession_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerResultIsInvalid_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenPlayerStatsContainNegativeValue_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenRewardQuantityIsNegative_ReturnsInvalid",
    "ValidateAndBuildPayload_WhenReportedTeamHasDifferentCasing_UsesSessionTeamInPayload",
    "MissingPlayerTeamMessage",
    "TeamMismatchMessage",
    "InvalidPlayerResultMessage",
    "InvalidPlayerNumericValueMessage",
    'Result = "eliminated"',
    "ExpDelta = -1",
    '["coin"] = -5',
    "CreateSessionTeams",
    "match-result-001",
    "runtime-token-001"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RuntimeLifecycleServiceTests.cs" @(
    "RuntimeLifecycleServiceTests",
    "MarkMatchStartedAsync_UpdatesSessionServerAndWritesEvents",
    "MarkMatchEndedAsync_UpdatesSessionServerAndWritesEvents",
    "MarkMatchStartedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents",
    "MarkMatchStartedAsync_WhenActivePlayerHasNotJoined_ReturnsFalseWithoutEvents",
    "MarkMatchEndedAsync_WhenMatchHasNotStarted_ReturnsFalseWithoutEvents",
    "MarkMatchEndedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents",
    "MarkMatchStartedAsync_WhenRepeated_DoesNotWriteDuplicateEvents",
    "MarkMatchEndedAsync_WhenRepeated_DoesNotWriteDuplicateEvents",
    "RuntimeLifecycleService.MarkMatchStartedAsync",
    "RuntimeLifecycleService.MarkMatchEndedAsync",
    "CreatePlayerSession",
    'CreatePlayerSession(sessionId, playerId, "CONNECTED")',
    'Assert.Equal("CONNECTED", playerSession.Status)',
    "Assert.Null(playerSession.JoinedAt)",
    'Assert.Equal("IN_PROGRESS", session.Status)',
    'Assert.Equal("WAITING_PLAYERS", session.Status)',
    'Assert.Equal("READY", server.Status)',
    'Assert.Equal("SETTLING", session.Status)'
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendRuntimeService.h" @(
    "FDBA_GameBackendRuntimePlayerResult",
    "PlayerId",
    "Result",
    "Kills",
    "Deaths",
    "Assists",
    "Score",
    "ExpDelta",
    "TMap<FString, int32> Rewards",
    "NotifyMatchStarted",
    "NotifyMatchEnded",
    "NotifyMatchResults",
    "BuildMatchResultsPayload"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp" @(
    'PostRuntime(TEXT("/runtime/servers/match-started")',
    'PostRuntime(TEXT("/runtime/servers/match-ended")',
    'PostRuntime(TEXT("/runtime/matches/results")',
    'TEXT("idempotencyKey")',
    'TEXT("resultJson")',
    'TEXT("players")',
    'TEXT("playerId")',
    'TEXT("kills")',
    'TEXT("deaths")',
    'TEXT("assists")',
    'TEXT("score")',
    'TEXT("expDelta")',
    'TEXT("rewards")',
    "FJsonValueObject"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendRuntimeServiceTests.cpp" @(
    "FDBA_GameBackendRuntimeMatchResultsPayloadTest",
    "DivineBeastsArena.GameBackendClient.Runtime.BuildMatchResultsPayload",
    "BuildMatchResultsPayload",
    "match-result-001",
    "runtime-token-001",
    "Players && Players->Num() == 1",
    'GetNumberField(TEXT("coin"))',
    'GetNumberField(TEXT("honor"))'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Framework\DBAGameModeBase.h" @(
    "virtual void HandleMatchHasStarted() override",
    "virtual void HandleMatchHasEnded() override",
    "ReportBackendMatchStarted",
    "ReportBackendMatchResults",
    "BackendRuntimePlayerTeamIds"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Player\DBAPlayerState.h" @(
    "class DIVINEBEASTSARENA_API ADBAPlayerState",
    "RecordKill",
    "RecordDeath",
    "RecordAssist",
    "AddMatchScore",
    "SetMatchResult",
    "SetMatchTeamId",
    "GetMatchTeamId",
    "GetMatchKills",
    "GetMatchDeaths",
    "GetMatchScore",
    "BuildRuntimePlayerResult",
    "ReplicatedUsing",
    "MatchKills",
    "MatchDeaths",
    "MatchAssists",
    "MatchScore",
    "MatchExpDelta",
    "MatchTeamId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp" @(
    "void ADBAPlayerState::GetLifetimeReplicatedProps",
    "DOREPLIFETIME",
    "FDBA_GameBackendRuntimePlayerResult ADBAPlayerState::BuildRuntimePlayerResult",
    "Result.PlayerId = BackendPlayerId",
    "Result.Team = MatchTeamId > 0",
    "Result.Kills = MatchKills",
    "Result.Deaths = MatchDeaths",
    "Result.Assists = MatchAssists",
    "Result.Score = MatchScore",
    "Result.ExpDelta = MatchExpDelta",
    "DOREPLIFETIME(ADBAPlayerState, MatchTeamId)"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp" @(
    "GameDBA/Player/DBAPlayerState.h",
    "ResolveDBAPlayerState",
    "RecordMatchEliminationStats",
    "VictimPlayerState->RecordDeath",
    "AttackerPlayerState->RecordKill",
    "!ZodiacChar->IsDead()",
    "ZodiacChar->OnDeath()"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "void ADBAGameModeBase::HandleMatchHasStarted()",
    "Super::HandleMatchHasStarted()",
    "ReportBackendMatchStarted",
    "void ADBAGameModeBase::ReportBackendMatchStarted()",
    "RuntimeService->NotifyMatchStarted",
    "void ADBAGameModeBase::HandleMatchHasEnded()",
    "Super::HandleMatchHasEnded()",
    "RuntimeService->NotifyMatchEnded",
    "ReportBackendMatchResults",
    "ResolveBackendMatchTeamIdFromOptions",
    "SyncBackendMatchTeamId",
    "DBAUrlOptions::TryExtractTeamId",
    "BackendTeamId <= 0",
    "BuildBackendRuntimeTeamName(BackendTeamId)",
    "BackendRuntimePlayerTeamIds.Add",
    "ZodiacCharacter->SetTeamID",
    "DBAPlayerState->SetMatchTeamId",
    "ApplyBackendMatchResultsOutcome",
    "FBackendMatchTeamOutcome",
    "BuildBackendMatchTeamOutcome",
    "BuildBackendMatchResultsJson",
    "winnerPlayerId",
    "winnerTeam",
    "TeamScores",
    "PlayerResult.Team",
    'PlayerResult.Result = TEXT("win")',
    'PlayerResult.Result = TEXT("loss")',
    'PlayerResult.Result = TEXT("draw")',
    "void ADBAGameModeBase::ReportBackendMatchResults()",
    "FDBA_GameBackendRuntimePlayerResult",
    "BackendRuntimePlayerIds",
    "PlayerStateClass = ADBAPlayerState::StaticClass()",
    "Cast<ADBAPlayerState>",
    "DBAPlayerState->BuildRuntimePlayerResult",
    'FString::Printf(TEXT("ue-match-result-%s")',
    "RuntimeService->NotifyMatchResults",
    "ResultJson",
    "winner"
)

Write-Host "PASS: Runtime match lifecycle contract" -ForegroundColor Green
