/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 /internal/runtime 管理接口的内部 API Key 保护。
- 阅读重点：这里测试的是管理接口保护；/runtime/* Dedicated Server 回调仍使用 runtimeToken。
- 修改提示：新增 Runtime 内部管理端点时，请同步覆盖缺失 key、错误 key 和正确 key 路径。
*/

using System.Net.Http.Json;
using System.Net;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.GameServer;
using Game.Shared.Contracts.Settlement;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Storage;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace Game.Api.Tests;

public class RuntimeEndpointsTests
{
    private const string InternalApiKey = "TEST-INTERNAL-API-KEY-MIN-32-CHARS";

    [Fact]
    public async Task GetInternalServer_WithoutInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/runtime/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetInternalServer_WithWrongInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", "wrong-key");

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/runtime/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetInternalServer_WithInternalApiKey_ReachesRuntimeHandler()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);

        var (statusCode, response) = await GetApiResponseAsync(client, $"/internal/runtime/servers/{Guid.NewGuid()}");

        Assert.Equal(HttpStatusCode.NotFound, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("404|Not Found", response.Message);
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenReportedTeamDiffersFromSession_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-team-contract";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-team-mismatch",
            """{"winnerTeam":"red"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "red",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains a player team that does not match the session.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenIdempotencyKeyIsBlank_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-blank-result-idempotency";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            " ",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result idempotency key is required.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayerResultRowsAreDuplicated_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-duplicate-result-player";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-duplicate-player",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>()),
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains duplicate players.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayersAreEmpty_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-empty-result-players";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-empty-players",
            """{"winnerTeam":"blue"}""",
            Array.Empty<RuntimePlayerResultDto>());

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result must contain at least one player.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayerTeamIsMissing_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-missing-result-team";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-missing-team",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " ",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains players without a team.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayerIsNotInSession_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var unknownPlayerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-unknown-result-player";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-unknown-player",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    unknownPlayerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains players not in session", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenSessionPlayerIsMissing_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var missingPlayerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-missing-session-player";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING",
            secondPlayerId: missingPlayerId,
            secondTeam: "red");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-missing-session-player",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result is missing players from session.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayerResultIsInvalid_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-invalid-player-result";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-invalid-player-result",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "eliminated",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains an invalid player result.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenPlayerStatsContainNegativeValue_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-negative-player-stats";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-negative-player-stats",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: -1,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains an invalid player numeric value.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutSettlement()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-negative-reward";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-negative-reward",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    "blue",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>
                    {
                        ["coin"] = -5
                    })
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Match result contains an invalid player numeric value.", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
    }

    [Fact]
    public async Task RuntimePlayerJoined_WithoutPlayerSessionToken_ReturnsUnauthorizedWithoutJoining()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-joined";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            playerSessionToken: "player-session-token",
            playerSessionStatus: "CONNECTED");

        var request = new RuntimePlayerJoinedRequest(
            serverId,
            sessionId,
            runtimeToken,
            playerId,
            "blue",
            SlotIndex: 0,
            PlayerSessionToken: null,
            Zodiac: "Rat",
            PrimaryElement: "Water",
            FiveCamp: "North",
            FixedSkillGroupId: "rat-water");

        var httpResponse = await client.PostAsJsonAsync("/runtime/servers/player-joined", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.Unauthorized, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("Invalid player session token", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        Assert.Equal("CONNECTED", playerSession.Status);
        Assert.Null(playerSession.JoinedAt);
        Assert.Equal(0, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "PLAYER_JOINED"));
    }

    [Fact]
    public async Task RuntimePlayerJoined_WithExpiredPlayerSessionToken_ReturnsUnauthorizedWithoutJoining()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-joined-expired";
        const string playerSessionToken = "expired-player-session-token";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            playerSessionToken: playerSessionToken,
            playerSessionStatus: "CONNECTED",
            playerSessionTokenExpiresAt: DateTimeOffset.UtcNow.AddMinutes(-1));

        var request = new RuntimePlayerJoinedRequest(
            serverId,
            sessionId,
            runtimeToken,
            playerId,
            "blue",
            SlotIndex: 0,
            PlayerSessionToken: playerSessionToken,
            Zodiac: "Rat",
            PrimaryElement: "Water",
            FiveCamp: "North",
            FixedSkillGroupId: "rat-water");

        var httpResponse = await client.PostAsJsonAsync("/runtime/servers/player-joined", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.Unauthorized, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("Invalid player session token", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        Assert.Equal("CONNECTED", playerSession.Status);
        Assert.Null(playerSession.JoinedAt);
        Assert.Equal(0, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "PLAYER_JOINED"));
    }

    [Fact]
    public async Task RuntimePlayerLeft_WhenPlayerIsNotInSession_ReturnsNotFoundWithoutWritingEvent()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var unknownPlayerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-left-unknown";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimePlayerLeftRequest(
            serverId,
            sessionId,
            runtimeToken,
            unknownPlayerId);

        var httpResponse = await client.PostAsJsonAsync("/runtime/servers/player-left", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.NotFound, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "PLAYER_LEFT"));
    }

    [Fact]
    public async Task RuntimePlayerLeft_WhenPlayerHasNotJoined_ReturnsBadRequestWithoutWritingEvent()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-left-not-joined";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            playerSessionStatus: "CONNECTED");

        var request = new RuntimePlayerLeftRequest(
            serverId,
            sessionId,
            runtimeToken,
            playerId);

        var httpResponse = await client.PostAsJsonAsync("/runtime/servers/player-left", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("Player has not joined session", response.Message);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        Assert.Equal("CONNECTED", playerSession.Status);
        Assert.Null(playerSession.LeftAt);
        Assert.Equal(0, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "PLAYER_LEFT"));
    }

    [Fact]
    public async Task RuntimePlayerLeft_WhenRepeated_DoesNotRewriteLeftAtOrDuplicateEvent()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-left-repeat";
        await SeedRuntimeSessionAsync(factory, sessionId, serverId, playerId, "blue", runtimeToken);

        var request = new RuntimePlayerLeftRequest(
            serverId,
            sessionId,
            runtimeToken,
            playerId);

        var firstResponse = await client.PostAsJsonAsync("/runtime/servers/player-left", request);
        await using var firstScope = factory.Services.CreateAsyncScope();
        var firstDb = firstScope.ServiceProvider.GetRequiredService<GameDbContext>();
        var firstPlayerSession = await firstDb.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        var firstLeftAt = firstPlayerSession.LeftAt;

        var secondResponse = await client.PostAsJsonAsync("/runtime/servers/player-left", request);

        Assert.Equal(HttpStatusCode.OK, firstResponse.StatusCode);
        Assert.Equal(HttpStatusCode.OK, secondResponse.StatusCode);
        Assert.NotNull(firstLeftAt);
        await using var secondScope = factory.Services.CreateAsyncScope();
        var db = secondScope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        Assert.Equal("LEFT", playerSession.Status);
        Assert.Equal(firstLeftAt, playerSession.LeftAt);
        Assert.Equal(1, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "PLAYER_LEFT"));
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-team-normalization";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-team-normalization",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 50,
                    new Dictionary<string, object>())
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.NotNull(response);
        Assert.True(response!.Success);
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var matchResult = await db.MatchResults.Include(x => x.PlayerResults).SingleAsync();
        var playerResult = Assert.Single(matchResult.PlayerResults);
        Assert.Equal("blue", playerResult.Team);
    }

    [Fact]
    public async Task RuntimeMatchResults_WithJsonRewardQuantity_GrantsRewardsStatsAndCompletesSession()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-positive-result-reward";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");
        await SeedPlayerProgressionAsync(factory, playerId);

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-runtime-json-reward",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 5
                    })
            });

        var httpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<JsonElement>>();

        Assert.Equal(HttpStatusCode.OK, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.True(response.Data.TryGetProperty("matchResultId", out var matchResultIdElement));
        Assert.True(matchResultIdElement.TryGetGuid(out var matchResultId));

        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var matchResult = await db.MatchResults.Include(x => x.PlayerResults).SingleAsync(x => x.Id == matchResultId);
        Assert.Equal(sessionId, matchResult.SessionId);
        var playerResult = Assert.Single(matchResult.PlayerResults);
        Assert.Equal(playerId, playerResult.PlayerId);
        Assert.Equal("blue", playerResult.Team);
        Assert.Equal("win", playerResult.Result);
        Assert.Equal(1200, playerResult.ExpDelta);
        var item = await db.InventoryItems.SingleAsync(x => x.PlayerId == playerId && x.ItemId == "coin");
        Assert.Equal(5, item.Quantity);
        Assert.Equal(1, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId && x.ItemId == "coin"));
        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        Assert.Equal(1200, profile.Exp);
        Assert.Equal(1, stats.TotalMatches);
        Assert.Equal(1, stats.Wins);
        Assert.Equal("COMPLETED", session.Status);
    }

    [Fact]
    public async Task RuntimeMatchResults_WhenRetried_ReturnsSameResultWithoutDoubleGrantingRewardsOrStats()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-result-retry";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");
        await SeedPlayerProgressionAsync(factory, playerId);

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-runtime-retry",
            """{"winnerTeam":"blue"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 5
                    })
            });

        var firstHttpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var firstResponse = await firstHttpResponse.Content.ReadFromJsonAsync<ApiResponse<JsonElement>>();
        var retryHttpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var retryResponse = await retryHttpResponse.Content.ReadFromJsonAsync<ApiResponse<JsonElement>>();

        Assert.Equal(HttpStatusCode.OK, firstHttpResponse.StatusCode);
        Assert.Equal(HttpStatusCode.OK, retryHttpResponse.StatusCode);
        Assert.NotNull(firstResponse);
        Assert.NotNull(retryResponse);
        Assert.True(firstResponse!.Success);
        Assert.True(retryResponse!.Success);
        Assert.True(firstResponse.Data.TryGetProperty("matchResultId", out var firstMatchResultIdElement));
        Assert.True(retryResponse.Data.TryGetProperty("matchResultId", out var retryMatchResultIdElement));
        Assert.True(firstMatchResultIdElement.TryGetGuid(out var firstMatchResultId));
        Assert.True(retryMatchResultIdElement.TryGetGuid(out var retryMatchResultId));
        Assert.Equal(firstMatchResultId, retryMatchResultId);

        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(1, await db.MatchResults.CountAsync(x => x.SessionId == sessionId));
        Assert.Equal(1, await db.MatchPlayerResults.CountAsync(x => x.PlayerId == playerId));
        var item = await db.InventoryItems.SingleAsync(x => x.PlayerId == playerId && x.ItemId == "coin");
        Assert.Equal(5, item.Quantity);
        Assert.Equal(1, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId && x.ItemId == "coin"));
        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        Assert.Equal(1200, profile.Exp);
        Assert.Equal(1, stats.TotalMatches);
        Assert.Equal(1, stats.Wins);
        Assert.Equal("COMPLETED", session.Status);
    }

    [Fact]
    public async Task RuntimeMatchResults_CanBeReadFromSettlementSessionResultsWithRewards()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-settlement-query";
        await SeedRuntimeSessionAsync(
            factory,
            sessionId,
            serverId,
            playerId,
            "blue",
            runtimeToken,
            sessionStatus: "SETTLING",
            serverStatus: "ENDING");
        await SeedPlayerProgressionAsync(factory, playerId);

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-runtime-query",
            """{"winnerTeam":"blue","schema":"runtime-endpoint-test"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 3,
                    Deaths: 1,
                    Assists: 2,
                    Score: 1200,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 5
                    })
            });

        var submitHttpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var submitResponse = await submitHttpResponse.Content.ReadFromJsonAsync<ApiResponse<JsonElement>>();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var queryResponse = await client.GetFromJsonAsync<ApiResponse<IReadOnlyList<MatchResultResponse>>>(
            $"/internal/settlement/sessions/{sessionId}/matches/results");

        Assert.Equal(HttpStatusCode.OK, submitHttpResponse.StatusCode);
        Assert.NotNull(submitResponse);
        Assert.True(submitResponse!.Success);
        Assert.True(submitResponse.Data.TryGetProperty("matchResultId", out var matchResultIdElement));
        Assert.True(matchResultIdElement.TryGetGuid(out var matchResultId));
        Assert.NotNull(queryResponse);
        Assert.True(queryResponse!.Success);
        var matchResult = Assert.Single(queryResponse.Data!);
        Assert.Equal(matchResultId, matchResult.Id);
        Assert.Equal(sessionId, matchResult.SessionId);
        Assert.Contains("\"winnerTeam\":\"blue\"", matchResult.ResultJson);
        Assert.Contains("\"schema\":\"runtime-endpoint-test\"", matchResult.ResultJson);
        var player = Assert.Single(matchResult.Players);
        Assert.Equal(playerId, player.PlayerId);
        Assert.Equal("blue", player.Team);
        Assert.Equal("win", player.Result);
        Assert.Equal(1200, player.Score);
        Assert.Equal(1200, player.ExpDelta);
        Assert.Equal(5, GetRewardInt(player.Rewards, "coin"));
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"runtime-endpoints-{Guid.NewGuid()}";
        var dbRoot = new InMemoryDatabaseRoot();
        return new WebApplicationFactory<Program>()
            .WithWebHostBuilder(builder =>
            {
                builder.UseEnvironment("Development");
                builder.ConfigureAppConfiguration((_, config) =>
                {
                    config.AddInMemoryCollection(new Dictionary<string, string?>
                    {
                        ["Database:ConnectionString"] = "Host=localhost;Database=dba_test;Username=test;Password=test",
                        ["Redis:ConnectionString"] = "localhost:6379",
                        ["Jwt:Secret"] = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-ENDPOINTS",
                        ["Jwt:Issuer"] = "GameApi.Tests",
                        ["Jwt:Audience"] = "GameApi.Tests",
                        ["InternalApi:Key"] = InternalApiKey,
                        ["GameServerManager:ServerMode"] = "External",
                        ["GameServerManager:PublicIp"] = "127.0.0.1",
                        ["GameServerManager:BackendUrl"] = "http://localhost:8080",
                        ["SeedData:Enabled"] = "false",
                        ["Swagger:Enabled"] = "false"
                    });
                });
                builder.ConfigureServices(services =>
                {
                    services.RemoveAll<DbContextOptions<GameDbContext>>();
                    services.RemoveAll<IDbContextOptionsConfiguration<GameDbContext>>();
                    services.AddDbContext<GameDbContext>(options => options
                        .UseInMemoryDatabase(dbName, dbRoot)
                        .ConfigureWarnings(x => x.Ignore(InMemoryEventId.TransactionIgnoredWarning)));
                });
            });
    }

    private static async Task<(HttpStatusCode StatusCode, ApiResponse<object>? Response)> GetApiResponseAsync(
        HttpClient client,
        string path)
    {
        var httpResponse = await client.GetAsync(path);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();
        return (httpResponse.StatusCode, response);
    }

    private static async Task SeedRuntimeSessionAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        Guid serverId,
        Guid playerId,
        string team,
        string runtimeToken,
        string? playerSessionToken = null,
        string playerSessionStatus = "JOINED",
        DateTimeOffset? playerSessionTokenExpiresAt = null,
        string sessionStatus = "IN_PROGRESS",
        string serverStatus = "IN_PROGRESS",
        Guid? secondPlayerId = null,
        string? secondTeam = null)
    {
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "ROOM",
            SourceId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Status = sessionStatus,
            ServerId = serverId,
            MaxPlayers = 2,
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-5),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-10)
        });
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = team,
            SlotIndex = 0,
            Status = playerSessionStatus,
            JoinedAt = playerSessionStatus == "JOINED" ? DateTimeOffset.UtcNow.AddMinutes(-7) : null,
            SessionTokenHash = HashToken(playerSessionToken ?? "player-session-token"),
            SessionTokenExpiresAt = playerSessionTokenExpiresAt ?? DateTimeOffset.UtcNow.AddMinutes(30),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
        });
        if (secondPlayerId.HasValue)
        {
            db.PlayerSessions.Add(new PlayerSession
            {
                Id = Guid.NewGuid(),
                GameSessionId = sessionId,
                PlayerId = secondPlayerId.Value,
                Team = secondTeam ?? "red",
                SlotIndex = 1,
                Status = playerSessionStatus,
                JoinedAt = playerSessionStatus == "JOINED" ? DateTimeOffset.UtcNow.AddMinutes(-7) : null,
                SessionTokenHash = HashToken("second-player-session-token"),
                SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(30),
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
            });
        }
        db.GameServerInstances.Add(new GameServerInstance
        {
            Id = serverId,
            SessionId = sessionId,
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Ip = "127.0.0.1",
            Port = 7777,
            RuntimeTokenHash = HashToken(runtimeToken),
            RuntimeTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(10),
            Status = serverStatus,
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-8),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-8)
        });
        await db.SaveChangesAsync();
    }

    private static async Task SeedPlayerProgressionAsync(WebApplicationFactory<Program> factory, Guid playerId)
    {
        await using var scope = factory.Services.CreateAsyncScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = "RuntimeEndpointTester",
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
        db.PlayerStatistics.Add(new PlayerStatistics
        {
            PlayerId = playerId,
            UpdatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
        await db.SaveChangesAsync();
    }

    private static int GetRewardInt(IReadOnlyDictionary<string, object> rewards, string key)
    {
        Assert.True(rewards.TryGetValue(key, out var value), $"Expected reward key '{key}'.");
        return value switch
        {
            JsonElement element when element.ValueKind == JsonValueKind.Number => element.GetInt32(),
            int intValue => intValue,
            long longValue => checked((int)longValue),
            _ => throw new InvalidOperationException($"Reward '{key}' is not an integer value.")
        };
    }

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }
}
