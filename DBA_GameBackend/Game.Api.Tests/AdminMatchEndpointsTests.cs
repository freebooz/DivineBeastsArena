/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证 Admin 赛果详情接口能输出 Dedicated Server / Settlement 产生的赛后展示与奖励信息。
- 阅读重点：测试通过真实 WebApplicationFactory、JWT 登录和 InMemory 数据库，不依赖外部服务。
- 修改提示：新增运营赛果字段时，请同步更新 Admin DTO、Admin endpoint 和 production evidence contract。
*/

using System.Net;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Game.Shared.Contracts.GameServer;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Storage;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace Game.Api.Tests;

public class AdminMatchEndpointsTests
{
    private const string AdminPassword = "Admin@123456";

    [Fact]
    public async Task GetMatch_ReturnsResultJsonAndPlayerRewardsForOperationsDiagnostics()
    {
        await using var factory = CreateFactory();
        var matchId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedAdminAndMatchAsync(factory, matchId, playerId);

        var client = factory.CreateClient();
        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var response = await client.GetFromJsonAsync<ApiResponse<AdminMatchDetailResponse>>(
            $"/api/admin/matches/{matchId}");

        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        Assert.Contains("\"winnerTeam\":\"blue\"", response.Data!.ResultJson);
        Assert.Contains("\"schema\":\"admin-endpoint-test\"", response.Data.ResultJson);

        var player = Assert.Single(response.Data.Players);
        Assert.Equal(playerId, player.PlayerId);
        Assert.Equal("blue", player.Team);
        Assert.Equal("win", player.Result);
        Assert.Equal(1200, player.Score);
        Assert.Equal(250, player.ExpDelta);
        Assert.Equal(50, GetRewardInt(player.Rewards, "coin"));
    }

    [Fact]
    public async Task GetMatch_ReturnsStructuredTeamOutcomeForOperationsDiagnostics()
    {
        await using var factory = CreateFactory();
        var matchId = Guid.NewGuid();
        var firstPlayerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();
        await SeedAdminAndMatchAsync(factory, matchId, firstPlayerId, secondPlayerId);

        var client = factory.CreateClient();
        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var response = await client.GetFromJsonAsync<ApiResponse<AdminMatchDetailResponse>>(
            $"/api/admin/matches/{matchId}");

        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        Assert.Equal("blue", response.Data!.WinnerTeam);
        Assert.Equal(1, response.Data.TeamDistribution["blue"]);
        Assert.Equal(1, response.Data.TeamDistribution["red"]);
    }

    [Fact]
    public async Task ListMatches_ReturnsStructuredWinnerTeamForOperationsDiagnostics()
    {
        await using var factory = CreateFactory();
        var matchId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        await SeedAdminAndMatchAsync(factory, matchId, playerId);

        var client = factory.CreateClient();
        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var httpResponse = await client.GetAsync("/api/admin/matches");
        var responseBody = await httpResponse.Content.ReadAsStringAsync();
        Assert.True(httpResponse.IsSuccessStatusCode, responseBody);
        var response = await httpResponse.Content.ReadFromJsonAsync<ApiResponse<AdminMatchListResponse>>();

        Assert.NotNull(response);
        Assert.True(response!.Success);
        Assert.NotNull(response.Data);
        var item = Assert.Single(response.Data!.Items);
        Assert.Equal(matchId, item.Id);
        Assert.Equal("blue", item.WinnerTeam);
    }

    [Fact]
    public async Task RuntimeMatchResults_CanBeReadFromAdminMatchDetailsForOperationsDiagnostics()
    {
        await using var factory = CreateFactory();
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-admin-query";
        await SeedAdminAsync(factory);
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
            "match-result-runtime-admin-query",
            """{"winnerTeam":"blue","schema":"runtime-admin-query-test"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    playerId,
                    " BLUE ",
                    "WIN",
                    Kills: 4,
                    Deaths: 1,
                    Assists: 6,
                    Score: 1500,
                    ExpDelta: 800,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 7
                    })
            });

        var runtimeHttpResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var runtimeResponse = await runtimeHttpResponse.Content.ReadFromJsonAsync<ApiResponse<JsonElement>>();

        Assert.Equal(HttpStatusCode.OK, runtimeHttpResponse.StatusCode);
        Assert.NotNull(runtimeResponse);
        Assert.True(runtimeResponse!.Success);
        Assert.True(runtimeResponse.Data.TryGetProperty("matchResultId", out var matchResultIdElement));
        Assert.True(matchResultIdElement.TryGetGuid(out var matchResultId));

        var token = await LoginAsync(client);
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", token);

        var adminResponse = await client.GetFromJsonAsync<ApiResponse<AdminMatchDetailResponse>>(
            $"/api/admin/matches/{matchResultId}");

        Assert.NotNull(adminResponse);
        Assert.True(adminResponse!.Success);
        Assert.NotNull(adminResponse.Data);
        Assert.Equal(sessionId, adminResponse.Data!.SessionId);
        Assert.Equal("blue", adminResponse.Data.WinnerTeam);
        Assert.Equal(1, adminResponse.Data.TeamDistribution["blue"]);
        Assert.Contains("\"schema\":\"runtime-admin-query-test\"", adminResponse.Data.ResultJson);

        var player = Assert.Single(adminResponse.Data.Players);
        Assert.Equal(playerId, player.PlayerId);
        Assert.Equal("blue", player.Team);
        Assert.Equal("win", player.Result);
        Assert.Equal(1500, player.Score);
        Assert.Equal(800, player.ExpDelta);
        Assert.Equal(7, GetRewardInt(player.Rewards, "coin"));
    }

    private static async Task<string> LoginAsync(HttpClient client)
    {
        var login = await client.PostAsJsonAsync(
            "/api/admin/auth/login",
            new AdminLoginRequest("ops-admin", AdminPassword));
        login.EnsureSuccessStatusCode();

        var payload = await login.Content.ReadFromJsonAsync<ApiResponse<AdminLoginResponse>>();
        Assert.NotNull(payload);
        Assert.True(payload!.Success);
        Assert.NotNull(payload.Data);
        return payload.Data!.AccessToken;
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
        var dbName = $"admin-match-endpoints-{Guid.NewGuid()}";
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
                        ["Jwt:Secret"] = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-ADMIN-MATCH-ENDPOINTS",
                        ["Jwt:Issuer"] = "GameApi.Tests",
                        ["Jwt:Audience"] = "GameApi.Tests",
                        ["InternalApi:Key"] = "TEST-INTERNAL-API-KEY-MIN-32-CHARS",
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

    private static async Task SeedAdminAsync(WebApplicationFactory<Program> factory)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.AdminUsers.Add(new AdminUser
        {
            Id = Guid.NewGuid(),
            Username = "ops-admin",
            PasswordHash = BCrypt.Net.BCrypt.HashPassword(AdminPassword),
            Role = "OPS",
            Status = "ACTIVE",
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();
    }

    private static async Task SeedAdminAndMatchAsync(
        WebApplicationFactory<Program> factory,
        Guid matchId,
        Guid playerId,
        Guid? secondPlayerId = null)
    {
        await SeedAdminAsync(factory);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var now = DateTimeOffset.UtcNow;

        db.MatchResults.Add(new MatchResult
        {
            Id = matchId,
            SessionId = Guid.NewGuid(),
            ServerId = Guid.NewGuid(),
            Mode = "ranked",
            MapId = "arena_01",
            DurationSeconds = 720,
            ResultJson = """{"winnerTeam":"blue","schema":"admin-endpoint-test"}""",
            IdempotencyKey = $"admin-endpoint-{matchId:N}",
            CreatedAt = now
        });

        db.MatchPlayerResults.Add(new MatchPlayerResult
        {
            Id = Guid.NewGuid(),
            MatchResultId = matchId,
            PlayerId = playerId,
            Team = "blue",
            Result = "win",
            Kills = 7,
            Deaths = 1,
            Assists = 3,
            Score = 1200,
            ExpDelta = 250,
            RewardJson = """{"coin":50}""",
            CreatedAt = now
        });

        if (secondPlayerId.HasValue)
        {
            db.MatchPlayerResults.Add(new MatchPlayerResult
            {
                Id = Guid.NewGuid(),
                MatchResultId = matchId,
                PlayerId = secondPlayerId.Value,
                Team = "red",
                Result = "loss",
                Kills = 2,
                Deaths = 5,
                Assists = 1,
                Score = 600,
                ExpDelta = 100,
                RewardJson = "{}",
                CreatedAt = now
            });
        }

        await db.SaveChangesAsync();
    }

    private static async Task SeedRuntimeSessionAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        Guid serverId,
        Guid playerId,
        string team,
        string runtimeToken,
        string sessionStatus,
        string serverStatus)
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
            Status = "JOINED",
            JoinedAt = DateTimeOffset.UtcNow.AddMinutes(-7),
            SessionTokenHash = HashToken("admin-query-player-session-token"),
            SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(30),
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
            Nickname = "RuntimeAdminQueryTester",
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

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }
}
