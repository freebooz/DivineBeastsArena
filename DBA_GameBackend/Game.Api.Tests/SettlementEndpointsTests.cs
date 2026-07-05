/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 Settlement HTTP endpoint 的统一响应和玩家结算明细输出。
- 阅读重点：测试宿主使用 InMemory 数据库，不依赖外部 PostgreSQL、Redis 或真实 Dedicated Server。
- 修改提示：新增结算展示字段或路由时，请同步更新本文件和 Runtime match lifecycle 契约。
*/

using System.Net.Http.Json;
using System.Net;
using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Settlement;
using Game.Api.Endpoints.Settlement;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace Game.Api.Tests;

public class SettlementEndpointsTests
{
    private const string InternalApiKey = "TEST-INTERNAL-API-KEY-MIN-32-CHARS";

    [Fact]
    public async Task GetSessionResults_WithoutInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();

        var (statusCode, response) = await GetApiResponseAsync(
            client,
            $"/internal/settlement/sessions/{Guid.NewGuid()}/matches/results");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetSessionResults_WithWrongInternalApiKey_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", "wrong-key");

        var (statusCode, response) = await GetApiResponseAsync(
            client,
            $"/internal/settlement/sessions/{Guid.NewGuid()}/matches/results");

        Assert.Equal(HttpStatusCode.Unauthorized, statusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("401|Unauthorized|Invalid internal api key", response.Message);
    }

    [Fact]
    public async Task GetSessionResults_ReturnsLatestResultsWithPlayerDetails()
    {
        await using var factory = CreateFactory();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var olderPlayerId = Guid.NewGuid();
        var latestPlayerId = Guid.NewGuid();
        var older = DateTimeOffset.UtcNow.AddMinutes(-3);
        var latest = DateTimeOffset.UtcNow;

        await SeedMatchResultAsync(factory, sessionId, serverId, olderPlayerId, older, "loss", 120);
        await SeedMatchResultAsync(factory, sessionId, serverId, latestPlayerId, latest, "win", 900);

        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var response = await client.GetFromJsonAsync<ApiResponse<IReadOnlyList<MatchResultResponse>>>(
            $"/internal/settlement/sessions/{sessionId}/matches/results");

        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        Assert.Collection(
            response.Data!,
            latestResult =>
            {
                Assert.Equal(latest, latestResult.CreatedAt);
                Assert.Contains("\"winnerTeam\":\"blue\"", latestResult.ResultJson);
                Assert.Contains("\"schema\":\"endpoint-test\"", latestResult.ResultJson);
                var player = Assert.Single(latestResult.Players);
                Assert.Equal(latestPlayerId, player.PlayerId);
                Assert.Equal("blue", player.Team);
                Assert.Equal("win", player.Result);
                Assert.Equal(900, player.Score);
                Assert.Equal(900, player.ExpDelta);
                Assert.Equal(1, GetRewardInt(player.Rewards, "coin"));
            },
            olderResult =>
            {
                Assert.Equal(older, olderResult.CreatedAt);
                Assert.Contains("\"winnerTeam\":\"blue\"", olderResult.ResultJson);
                var player = Assert.Single(olderResult.Players);
                Assert.Equal(olderPlayerId, player.PlayerId);
                Assert.Equal("blue", player.Team);
                Assert.Equal("loss", player.Result);
                Assert.Equal(120, player.Score);
                Assert.Equal(120, player.ExpDelta);
                Assert.Equal(1, GetRewardInt(player.Rewards, "coin"));
            });
    }

    [Fact]
    public async Task SubmitResult_WhenRewardQuantityIsNegative_ReturnsBadRequestWithoutRewardsOrStats()
    {
        await using var factory = CreateFactory();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedSettlingSessionAsync(factory, sessionId, serverId, playerId);

        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var request = new SubmitMatchResultRequest(
            sessionId,
            "settlement-endpoint-negative-reward",
            """{"winner":"blue"}""",
            new[]
            {
                new MatchPlayerResultDto(
                    playerId,
                    "blue",
                    "win",
                    Kills: 4,
                    Deaths: 1,
                    Assists: 2,
                    Score: 900,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = -5
                    })
            });

        var httpResponse = await client.PostAsJsonAsync("/internal/settlement/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<object>>();

        Assert.Equal(HttpStatusCode.BadRequest, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.False(response!.Success);
        Assert.Contains("400|Bad Request|Failed to submit match result", response.Message);
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
        Assert.Equal(0, await db.InventoryItems.CountAsync(x => x.PlayerId == playerId));
        Assert.Equal(0, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId));
        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        Assert.Equal(0, profile.Exp);
        Assert.Equal(0, stats.TotalMatches);
        Assert.Equal("SETTLING", session.Status);
    }

    [Fact]
    public async Task SubmitResult_WithJsonRewardQuantity_GrantsRewardsAndCompletesSession()
    {
        await using var factory = CreateFactory();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedSettlingSessionAsync(factory, sessionId, serverId, playerId);

        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var request = new SubmitMatchResultRequest(
            sessionId,
            "settlement-endpoint-json-reward",
            """{"winner":"blue"}""",
            new[]
            {
                new MatchPlayerResultDto(
                    playerId,
                    "blue",
                    "win",
                    Kills: 4,
                    Deaths: 1,
                    Assists: 2,
                    Score: 900,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 5
                    })
            });

        var httpResponse = await client.PostAsJsonAsync("/internal/settlement/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<MatchResultResponse>>();

        Assert.Equal(HttpStatusCode.OK, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        Assert.Equal(sessionId, response.Data!.SessionId);
        var responsePlayer = Assert.Single(response.Data.Players);
        Assert.Equal(playerId, responsePlayer.PlayerId);
        Assert.Equal(5, GetRewardInt(responsePlayer.Rewards, "coin"));

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(1, await db.MatchResults.CountAsync());
        Assert.Equal(1, await db.MatchPlayerResults.CountAsync());
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
    public async Task SubmitResult_WhenReportedTeamCasingDiffers_SettlesWithFrozenSessionTeam()
    {
        await using var factory = CreateFactory();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedSettlingSessionAsync(factory, sessionId, serverId, playerId);

        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var request = new SubmitMatchResultRequest(
            sessionId,
            "settlement-endpoint-frozen-team",
            """{"winner":"blue","schema":"frozen-team-test"}""",
            new[]
            {
                new MatchPlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 2,
                    Deaths: 0,
                    Assists: 4,
                    Score: 700,
                    ExpDelta: 500,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 3
                    })
            });

        var httpResponse = await client.PostAsJsonAsync("/internal/settlement/matches/results", request);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<MatchResultResponse>>();

        Assert.Equal(HttpStatusCode.OK, httpResponse.StatusCode);
        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        var responsePlayer = Assert.Single(response.Data!.Players);
        Assert.Equal("blue", responsePlayer.Team);
        Assert.Equal("win", responsePlayer.Result);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerResult = await db.MatchPlayerResults.SingleAsync(x => x.PlayerId == playerId);
        var history = await db.PlayerMatchHistories.SingleAsync(x => x.PlayerId == playerId);
        Assert.Equal("blue", playerResult.Team);
        Assert.Equal("blue", history.Team);
        Assert.Equal("win", history.Result);
    }

    [Fact]
    public async Task SubmitResult_WhenRetried_ReturnsPlayerDetailsWithoutDoubleGrantingRewardsOrStats()
    {
        await using var factory = CreateFactory();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedSettlingSessionAsync(factory, sessionId, serverId, playerId);

        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Add("X-Internal-Api-Key", InternalApiKey);
        var request = new SubmitMatchResultRequest(
            sessionId,
            "settlement-endpoint-retry-json-reward",
            """{"winner":"blue"}""",
            new[]
            {
                new MatchPlayerResultDto(
                    playerId,
                    "blue",
                    "win",
                    Kills: 4,
                    Deaths: 1,
                    Assists: 2,
                    Score: 900,
                    ExpDelta: 1200,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 5
                    })
            });

        var firstHttpResponse = await client.PostAsJsonAsync("/internal/settlement/matches/results", request);
        var firstResponse = await firstHttpResponse.Content.ReadFromJsonAsync<ApiResponse<MatchResultResponse>>();
        var retryHttpResponse = await client.PostAsJsonAsync("/internal/settlement/matches/results", request);
        var retryResponse = await retryHttpResponse.Content.ReadFromJsonAsync<ApiResponse<MatchResultResponse>>();

        Assert.Equal(HttpStatusCode.OK, firstHttpResponse.StatusCode);
        Assert.Equal(HttpStatusCode.OK, retryHttpResponse.StatusCode);
        Assert.NotNull(firstResponse);
        Assert.NotNull(firstResponse!.Data);
        Assert.NotNull(retryResponse);
        Assert.True(retryResponse!.Success);
        Assert.NotNull(retryResponse.Data);
        Assert.Equal(firstResponse.Data!.Id, retryResponse.Data!.Id);
        var retryPlayer = Assert.Single(retryResponse.Data.Players);
        Assert.Equal(playerId, retryPlayer.PlayerId);
        Assert.Equal(5, GetRewardInt(retryPlayer.Rewards, "coin"));

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(1, await db.MatchResults.CountAsync(x => x.SessionId == sessionId));
        Assert.Equal(1, await db.MatchPlayerResults.CountAsync(x => x.PlayerId == playerId));
        var item = await db.InventoryItems.SingleAsync(x => x.PlayerId == playerId && x.ItemId == "coin");
        Assert.Equal(5, item.Quantity);
        Assert.Equal(1, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId && x.ItemId == "coin"));
        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        Assert.Equal(1200, profile.Exp);
        Assert.Equal(1, stats.TotalMatches);
        Assert.Equal(1, stats.Wins);
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

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"settlement-endpoints-{Guid.NewGuid()}";
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
                        .UseInMemoryDatabase(dbName)
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

    private static async Task SeedSettlingSessionAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        Guid serverId,
        Guid playerId)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "ROOM",
            SourceId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Status = "SETTLING",
            ServerId = serverId,
            MaxPlayers = 2,
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-8),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-10)
        });
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = "blue",
            SlotIndex = 0,
            Status = "JOINED",
            SessionTokenHash = "hash",
            SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(10),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
        });
        db.GameServerInstances.Add(new GameServerInstance
        {
            Id = serverId,
            SessionId = sessionId,
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Ip = "127.0.0.1",
            Port = 7777,
            Status = "ENDING",
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-9),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
        });
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = "SettlementEndpointTester",
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

    private static async Task SeedMatchResultAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        Guid serverId,
        Guid playerId,
        DateTimeOffset createdAt,
        string result,
        int score)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var resultId = Guid.NewGuid();

        db.MatchResults.Add(new MatchResult
        {
            Id = resultId,
            SessionId = sessionId,
            ServerId = serverId,
            Mode = "classic",
            MapId = "arena_01",
            DurationSeconds = 600,
            ResultJson = """{"winnerTeam":"blue","schema":"endpoint-test"}""",
            IdempotencyKey = $"endpoint-{resultId:N}",
            CreatedAt = createdAt
        });
        db.MatchPlayerResults.Add(new MatchPlayerResult
        {
            Id = Guid.NewGuid(),
            MatchResultId = resultId,
            PlayerId = playerId,
            Team = "blue",
            Result = result,
            Kills = score / 100,
            Deaths = result == "win" ? 1 : 4,
            Assists = 2,
            Score = score,
            ExpDelta = score,
            RewardJson = """{"coin":1}""",
            CreatedAt = createdAt
        });

        await db.SaveChangesAsync();
    }
}
