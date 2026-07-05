/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API 测试。
- 文件职责：验证玩家鉴权列表接口缺省分页参数时仍能从 JWT 玩家身份返回首屏数据。
- 阅读重点：测试种子玩家账号并签发真实 Bearer JWT，不传 query playerId/page/pageSize，覆盖客户端首屏直连路径。
*/

using System.Net;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using Game.Infrastructure.Auth;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
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

public class PlayerListPagingDefaultsTests
{
    private const string JwtSecret = "TEST-JWT-SECRET-MINIMUM-32-CHARS-FOR-PLAYER-LIST-ENDPOINTS";
    private const string JwtIssuer = "GameApi.Tests";
    private const string JwtAudience = "GameApi.Tests";

    public static IEnumerable<object[]> PlayerListEndpointsWithoutPaging()
    {
        yield return new object[] { "/api/players/me/matches" };
        yield return new object[] { "/api/support/tickets" };
    }

    public static IEnumerable<object[]> PlayerReadEndpointsWithoutQueryPlayerId()
    {
        yield return new object[] { "/api/players/me/inventory/" };
        yield return new object[] { "/api/players/me/inventory/unlocks" };
        yield return new object[] { "/api/events/me/progress" };
        yield return new object[] { "/api/players/me/achievements/" };
        yield return new object[] { "/api/friends/" };
        yield return new object[] { "/api/friends/requests" };
        yield return new object[] { "/api/mails/" };
    }

    [Theory]
    [MemberData(nameof(PlayerListEndpointsWithoutPaging))]
    public async Task PlayerListEndpoints_WithoutPagingQuery_UseJwtPlayerAndDefaultPage(string path)
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var httpResponse = await client.GetAsync(path);
        var responseBody = await httpResponse.Content.ReadAsStringAsync();

        Assert.True(httpResponse.IsSuccessStatusCode, $"{(int)httpResponse.StatusCode} {httpResponse.StatusCode}: {responseBody}");

        using var payload = JsonDocument.Parse(responseBody);
        var root = payload.RootElement;
        Assert.True(root.GetProperty("success").GetBoolean());

        var data = root.GetProperty("data");
        Assert.Equal(1, data.GetProperty("page").GetInt32());
        Assert.Equal(50, data.GetProperty("pageSize").GetInt32());
    }

    [Theory]
    [MemberData(nameof(PlayerReadEndpointsWithoutQueryPlayerId))]
    public async Task PlayerReadEndpoints_WithoutQueryPlayerId_UseJwtPlayer(string path)
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var httpResponse = await client.GetAsync(path);
        var responseBody = await httpResponse.Content.ReadAsStringAsync();

        Assert.True(httpResponse.IsSuccessStatusCode, $"{(int)httpResponse.StatusCode} {httpResponse.StatusCode}: {responseBody}");

        using var payload = JsonDocument.Parse(responseBody);
        Assert.True(payload.RootElement.GetProperty("success").GetBoolean());
    }

    [Fact]
    public async Task PlayerSupportAndReportWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var reportResponse = await client.PostAsJsonAsync(
            "/api/reports/",
            new SubmitReportRequest(null, "OTHER", "Suspicious match behavior with reproducible evidence.", Array.Empty<string>()));
        await AssertSuccessAsync(reportResponse);

        var ticketResponse = await client.PostAsJsonAsync(
            "/api/support/tickets",
            new CreateTicketRequest("BUG", "Cannot enter arena", "Client stays at loading after matchmaking.", "NORMAL"));
        var ticketId = await ReadGuidDataPropertyAsync(ticketResponse, "ticketId");

        var detailResponse = await client.GetAsync($"/api/support/tickets/{ticketId}");
        await AssertSuccessAsync(detailResponse);

        var replyResponse = await client.PostAsJsonAsync(
            $"/api/support/tickets/{ticketId}/reply",
            new ReplyTicketRequest("Adding a screenshot and logs.", false));
        await AssertSuccessAsync(replyResponse);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(player.PlayerId, (await db.Reports.SingleAsync()).ReporterId);
        Assert.Equal(player.PlayerId, (await db.SupportTickets.SingleAsync()).PlayerId);
        Assert.Equal(player.PlayerId, (await db.TicketReplies.SingleAsync()).PlayerId);
    }

    [Fact]
    public async Task PlayerSocialWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var otherPlayerId = await SeedProfileOnlyAsync(factory, "FriendTarget");
        var removablePlayerId = await SeedProfileOnlyAsync(factory, "RemovableFriend");
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var sendResponse = await client.PostAsJsonAsync("/api/friends/request", new FriendRequestDto(otherPlayerId));
        await AssertSuccessAsync(sendResponse);

        var acceptRequestId = await SeedFriendRequestAsync(factory, otherPlayerId, player.PlayerId);
        var acceptResponse = await client.PostAsync($"/api/friends/{acceptRequestId}/accept", null);
        await AssertSuccessAsync(acceptResponse);

        var rejectRequestId = await SeedFriendRequestAsync(factory, otherPlayerId, player.PlayerId);
        var rejectResponse = await client.PostAsync($"/api/friends/{rejectRequestId}/reject", null);
        await AssertSuccessAsync(rejectResponse);

        await SeedFriendRelationAsync(factory, player.PlayerId, removablePlayerId);
        var removeResponse = await client.DeleteAsync($"/api/friends/{removablePlayerId}");
        await AssertSuccessAsync(removeResponse);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Contains(await db.FriendRequests.ToListAsync(), x => x.SenderId == player.PlayerId && x.ReceiverId == otherPlayerId);
        Assert.Equal("ACCEPTED", (await db.FriendRequests.SingleAsync(x => x.Id == acceptRequestId)).Status);
        Assert.Equal("REJECTED", (await db.FriendRequests.SingleAsync(x => x.Id == rejectRequestId)).Status);
        Assert.False(await db.FriendRelations.AnyAsync(x => x.PlayerId == player.PlayerId && x.FriendId == removablePlayerId));
    }

    [Fact]
    public async Task PlayerMailWriteEndpoints_WithoutQueryPlayerId_UseJwtPlayer()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var (mailId, attachmentId) = await SeedMailWithAttachmentAsync(factory, player.PlayerId);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var readResponse = await client.PostAsync($"/api/mails/{mailId}/read", null);
        await AssertSuccessAsync(readResponse);

        var claimResponse = await client.PostAsync($"/api/mails/{mailId}/attachments/{attachmentId}/claim", null);
        await AssertSuccessAsync(claimResponse);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.True((await db.Mails.SingleAsync(x => x.Id == mailId)).IsRead);
        Assert.True((await db.MailAttachments.SingleAsync(x => x.Id == attachmentId)).IsClaimed);
        Assert.Equal(3, (await db.InventoryItems.SingleAsync(x => x.PlayerId == player.PlayerId && x.ItemId == "coin")).Quantity);
    }

    [Fact]
    public async Task PlayerShopPurchase_WithoutQueryPlayerId_UsesJwtPlayer()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        await SeedWalletAsync(factory, player.PlayerId, "COIN", 150);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var purchaseResponse = await client.PostAsJsonAsync(
            "/api/shop/purchase",
            new PurchaseRequest("skin_001", 1, "MOCK"));
        await AssertSuccessAsync(purchaseResponse);

        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        Assert.Equal(player.PlayerId, (await db.OrderRecords.SingleAsync()).PlayerId);
        Assert.Equal(50, (await db.WalletBalances.SingleAsync(x => x.PlayerId == player.PlayerId && x.CurrencyType == "COIN")).Balance);
        Assert.Equal(1, (await db.InventoryItems.SingleAsync(x => x.PlayerId == player.PlayerId && x.ItemId == "skin_001")).Quantity);
    }

    [Fact]
    public async Task PlayerReconnect_WithoutQueryPlayerId_UsesJwtPlayerAndValidToken()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var reconnectToken = $"reconnect-{Guid.NewGuid():N}";
        var sessionId = await SeedReconnectSessionAsync(factory, player.PlayerId, reconnectToken);
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var reconnectResponse = await client.PostAsJsonAsync(
            $"/api/sessions/{sessionId}/reconnect",
            new ReconnectRequest(reconnectToken));
        await AssertSuccessAsync(reconnectResponse);
    }

    [Fact]
    public async Task PlayerReconnect_WithWrongReconnectToken_ReturnsUnauthorized()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var sessionId = await SeedReconnectSessionAsync(factory, player.PlayerId, $"reconnect-{Guid.NewGuid():N}");
        await AssertReconnectTokenHashDiffersAsync(factory, sessionId, "wrong-reconnect-token");
        var client = factory.CreateClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);

        var reconnectResponse = await client.PostAsJsonAsync(
            $"/api/sessions/{sessionId}/reconnect",
            new ReconnectRequest("wrong-reconnect-token"));
        var body = await reconnectResponse.Content.ReadAsStringAsync();

        Assert.Equal(System.Net.HttpStatusCode.Unauthorized, reconnectResponse.StatusCode);
        Assert.Contains("重连令牌", body);
    }

    [Fact]
    public async Task PlayerMatchHistory_AfterRuntimeSettlement_ReturnsSettledPlayerResult()
    {
        await using var factory = CreateFactory();
        var player = await SeedPlayerAndCreateAccessTokenAsync(factory);
        var client = factory.CreateClient();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        const string runtimeToken = "runtime-token-player-history";
        await SeedRuntimeSessionAsync(factory, sessionId, serverId, player.PlayerId, "blue", runtimeToken);

        var request = new RuntimeMatchResultsRequest(
            serverId,
            sessionId,
            runtimeToken,
            "match-result-player-history",
            """{"winnerTeam":"blue","schema":"player-history-test"}""",
            new[]
            {
                new RuntimePlayerResultDto(
                    player.PlayerId,
                    " BLUE ",
                    "WIN",
                    Kills: 5,
                    Deaths: 2,
                    Assists: 7,
                    Score: 1600,
                    ExpDelta: 900,
                    new Dictionary<string, object>
                    {
                        ["coin"] = 9
                    })
            });

        var runtimeResponse = await client.PostAsJsonAsync("/runtime/matches/results", request);
        var runtimeBody = await runtimeResponse.Content.ReadAsStringAsync();
        Assert.Equal(HttpStatusCode.OK, runtimeResponse.StatusCode);
        Assert.Contains("\"success\":true", runtimeBody);

        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", player.Token);
        var historyHttpResponse = await client.GetAsync("/api/players/me/matches");
        var historyBody = await historyHttpResponse.Content.ReadAsStringAsync();
        var historyResponse = await historyHttpResponse.Content.ReadFromJsonAsync<ApiResponse<MatchHistoryResponse>>();

        Assert.Equal(HttpStatusCode.OK, historyHttpResponse.StatusCode);
        Assert.NotNull(historyResponse);
        Assert.True(historyResponse!.Success);
        Assert.NotNull(historyResponse.Data);
        Assert.Equal(1, historyResponse.Data!.TotalCount);
        var match = Assert.Single(historyResponse.Data.Matches);
        Assert.Equal(sessionId, match.SessionId);
        Assert.Equal("classic", match.Mode);
        Assert.Equal("arena_01", match.MapId);
        Assert.Equal("blue", match.Team);
        Assert.Equal("win", match.Result);
        Assert.Equal(5, match.Kills);
        Assert.Equal(2, match.Deaths);
        Assert.Equal(7, match.Assists);
        Assert.Equal(1600, match.Score);
        Assert.Contains("\"schema\":\"player-history-test\"", match.ResultJson);
        Assert.Equal("blue", match.WinnerTeam);
        Assert.True(match.DurationSeconds > 0);

        using var historyJson = JsonDocument.Parse(historyBody);
        var historyMatch = historyJson.RootElement.GetProperty("data").GetProperty("matches")[0];
        Assert.True(historyMatch.TryGetProperty("expDelta", out var expDelta), historyBody);
        Assert.Equal(900, expDelta.GetInt64());
        Assert.True(historyMatch.TryGetProperty("rewards", out var rewards), historyBody);
        Assert.Equal(9, rewards.GetProperty("coin").GetInt32());
        Assert.True(historyMatch.TryGetProperty("winnerTeam", out var winnerTeam), historyBody);
        Assert.Equal("blue", winnerTeam.GetString());
        Assert.True(historyMatch.TryGetProperty("resultJson", out var resultJson), historyBody);
        Assert.Contains("\"schema\":\"player-history-test\"", resultJson.GetString());
    }

    private static WebApplicationFactory<Program> CreateFactory()
    {
        var dbName = $"player-list-paging-defaults-{Guid.NewGuid()}";
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
                        ["Jwt:Secret"] = JwtSecret,
                        ["Jwt:Issuer"] = JwtIssuer,
                        ["Jwt:Audience"] = JwtAudience,
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

    private static async Task<(string Token, Guid PlayerId)> SeedPlayerAndCreateAccessTokenAsync(WebApplicationFactory<Program> factory)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var accountId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        var now = DateTimeOffset.UtcNow;

        var account = new Account
        {
            Id = accountId,
            AccountType = "GUEST",
            Status = "ACTIVE",
            CreatedAt = now
        };
        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = accountId,
            PlayerId = playerId,
            DisplayName = "PagingTester",
            CreatedAt = now
        };

        db.Accounts.Add(account);
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = "PagingTester",
            Level = 1,
            Exp = 0,
            CreatedAt = now
        });
        db.PlayerIdentities.Add(identity);
        db.PlayerStatistics.Add(new PlayerStatistics
        {
            PlayerId = playerId,
            TotalMatches = 0,
            Wins = 0,
            Losses = 0,
            Draws = 0,
            Kills = 0,
            Deaths = 0,
            Assists = 0,
            Score = 0,
            PlayTimeSeconds = 0,
            UpdatedAt = now
        });
        db.PlayerSettings.Add(new PlayerSettings
        {
            PlayerId = playerId,
            SettingsJson = "{}",
            UpdatedAt = now
        });

        await db.SaveChangesAsync();

        var jwt = scope.ServiceProvider.GetRequiredService<IJwtTokenService>();
        return (jwt.GenerateTokens(account, identity).AccessToken, playerId);
    }

    private static async Task<Guid> SeedProfileOnlyAsync(WebApplicationFactory<Program> factory, string nickname)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var playerId = Guid.NewGuid();
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = nickname,
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();
        return playerId;
    }

    private static async Task<Guid> SeedFriendRequestAsync(WebApplicationFactory<Program> factory, Guid senderId, Guid receiverId)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var request = new FriendRequest
        {
            Id = Guid.NewGuid(),
            SenderId = senderId,
            ReceiverId = receiverId,
            Status = "PENDING",
            CreatedAt = DateTimeOffset.UtcNow
        };
        db.FriendRequests.Add(request);
        await db.SaveChangesAsync();
        return request.Id;
    }

    private static async Task SeedFriendRelationAsync(WebApplicationFactory<Program> factory, Guid playerId, Guid friendId)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.FriendRelations.Add(new FriendRelation { Id = Guid.NewGuid(), PlayerId = playerId, FriendId = friendId });
        db.FriendRelations.Add(new FriendRelation { Id = Guid.NewGuid(), PlayerId = friendId, FriendId = playerId });
        await db.SaveChangesAsync();
    }

    private static async Task<(Guid MailId, Guid AttachmentId)> SeedMailWithAttachmentAsync(WebApplicationFactory<Program> factory, Guid playerId)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var mailId = Guid.NewGuid();
        var attachmentId = Guid.NewGuid();
        db.Mails.Add(new Mail
        {
            Id = mailId,
            ReceiverId = playerId,
            MailType = "GIFT",
            Title = "Welcome Gift",
            Content = "Claim your starter gift.",
            AttachmentJson = """[{"itemId":"coin","quantity":3}]""",
            CreatedAt = DateTimeOffset.UtcNow
        });
        db.MailAttachments.Add(new MailAttachment
        {
            Id = attachmentId,
            MailId = mailId,
            ItemId = "coin",
            Quantity = 3
        });
        await db.SaveChangesAsync();
        return (mailId, attachmentId);
    }

    private static async Task SeedWalletAsync(WebApplicationFactory<Program> factory, Guid playerId, string currency, long balance)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        db.WalletBalances.Add(new WalletBalance
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            CurrencyType = currency,
            Balance = balance,
            UpdatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();
    }

    private static async Task<Guid> SeedReconnectSessionAsync(WebApplicationFactory<Program> factory, Guid playerId, string reconnectToken)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var jwt = scope.ServiceProvider.GetRequiredService<IJwtTokenService>();
        var sessionId = Guid.NewGuid();
        var now = DateTimeOffset.UtcNow;

        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "MATCHMAKING",
            Mode = "ranked",
            MapId = "arena_01",
            Region = "cn",
            Status = "IN_PROGRESS",
            ServerIp = "127.0.0.1",
            ServerPort = 7777,
            MaxPlayers = 10,
            CreatedAt = now,
            StartedAt = now
        });
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = "blue",
            Status = "CONNECTED",
            SessionTokenHash = jwt.HashToken($"session-{Guid.NewGuid():N}"),
            SessionTokenExpiresAt = now.AddMinutes(30),
            ReconnectTokenHash = jwt.HashToken(reconnectToken),
            ReconnectTokenExpiresAt = now.AddMinutes(10),
            JoinedAt = now,
            CreatedAt = now
        });

        await db.SaveChangesAsync();
        return sessionId;
    }

    private static async Task SeedRuntimeSessionAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        Guid serverId,
        Guid playerId,
        string team,
        string runtimeToken)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var now = DateTimeOffset.UtcNow;

        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "MATCHMAKING",
            SourceId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Status = "SETTLING",
            ServerId = serverId,
            MaxPlayers = 2,
            StartedAt = now.AddMinutes(-6),
            CreatedAt = now.AddMinutes(-10)
        });
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = team,
            SlotIndex = 0,
            Status = "JOINED",
            JoinedAt = now.AddMinutes(-5),
            SessionTokenHash = HashToken("player-history-session-token"),
            SessionTokenExpiresAt = now.AddMinutes(30),
            CreatedAt = now.AddMinutes(-9)
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
            RuntimeTokenExpiresAt = now.AddMinutes(10),
            Status = "ENDING",
            StartedAt = now.AddMinutes(-8),
            CreatedAt = now.AddMinutes(-8)
        });

        await db.SaveChangesAsync();
    }

    private static async Task AssertReconnectTokenHashDiffersAsync(
        WebApplicationFactory<Program> factory,
        Guid sessionId,
        string rejectedToken)
    {
        using var scope = factory.Services.CreateScope();
        var db = scope.ServiceProvider.GetRequiredService<GameDbContext>();
        var jwt = scope.ServiceProvider.GetRequiredService<IJwtTokenService>();
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId);

        Assert.NotEqual(jwt.HashToken(rejectedToken), playerSession.ReconnectTokenHash);
    }

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static async Task AssertSuccessAsync(HttpResponseMessage response)
    {
        var body = await response.Content.ReadAsStringAsync();
        Assert.True(response.IsSuccessStatusCode, $"{(int)response.StatusCode} {response.StatusCode}: {body}");
        using var payload = JsonDocument.Parse(body);
        Assert.True(payload.RootElement.GetProperty("success").GetBoolean(), body);
    }

    private static async Task<Guid> ReadGuidDataPropertyAsync(HttpResponseMessage response, string propertyName)
    {
        var body = await response.Content.ReadAsStringAsync();
        Assert.True(response.IsSuccessStatusCode, $"{(int)response.StatusCode} {response.StatusCode}: {body}");
        using var payload = JsonDocument.Parse(body);
        var root = payload.RootElement;
        Assert.True(root.GetProperty("success").GetBoolean(), body);
        return root.GetProperty("data").GetProperty(propertyName).GetGuid();
    }
}
