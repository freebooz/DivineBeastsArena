/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：测试结算和背包的幂等、事务边界和玩家数据更新规则。
- 阅读重点：先看测试数据中的 session/server/player 绑定，再看重复提交后数量是否只变化一次。
- 修改提示：新增奖励类型、战报字段或背包流水约束时，请优先补充本文件测试。
*/

using Game.Api.Services.Inventory;
using Game.Api.Services.Settlement;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.Settlement;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Diagnostics;
using Microsoft.Extensions.Logging.Abstractions;

namespace Game.Api.Tests;

public class SettlementInventoryServiceTests
{
    [Fact]
    public async Task AddItemAsync_WithSameBizId_IsIdempotentForSamePlayerAndItem()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var playerId = Guid.NewGuid();

        await inventory.AddItemAsync(playerId, "coin", 3, null, "MATCH_REWARD", "MATCH_REWARD", "match-001");
        await inventory.AddItemAsync(playerId, "coin", 3, null, "MATCH_REWARD", "MATCH_REWARD", "match-001");

        var item = await db.InventoryItems.SingleAsync(x => x.PlayerId == playerId && x.ItemId == "coin");
        Assert.Equal(3, item.Quantity);
        Assert.Equal(1, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId && x.ItemId == "coin"));
    }

    [Fact]
    public async Task SubmitMatchResultAsync_RepeatedSubmission_DoesNotDoubleGrantRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = new SubmitMatchResultRequest(
            sessionId,
            "result-001",
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
                    Rewards: new Dictionary<string, object> { ["coin"] = 5 })
            });

        var first = await settlement.SubmitMatchResultAsync(request);
        var second = await settlement.SubmitMatchResultAsync(request with { IdempotencyKey = "result-001-retry" });

        Assert.NotNull(first);
        Assert.NotNull(second);
        Assert.Equal(first!.Id, second!.Id);
        Assert.Equal(1, await db.MatchResults.CountAsync(x => x.SessionId == sessionId));
        Assert.Equal(1, await db.MatchPlayerResults.CountAsync(x => x.PlayerId == playerId));

        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        var item = await db.InventoryItems.SingleAsync(x => x.PlayerId == playerId && x.ItemId == "coin");
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);

        Assert.Equal(1200, profile.Exp);
        Assert.Equal(2, profile.Level);
        Assert.Equal(1, stats.TotalMatches);
        Assert.Equal(1, stats.Wins);
        Assert.Equal(5, item.Quantity);
        Assert.Equal(1, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId && x.ItemId == "coin"));
        Assert.Equal("COMPLETED", session.Status);
    }

    [Fact]
    public async Task SubmitMatchResultAsync_WhenIdempotencyKeyIsBlank_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(sessionId, playerId, "   ", 1200);

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task SubmitMatchResultAsync_WhenMatchHasNotEnded_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedRunningSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = new SubmitMatchResultRequest(
            sessionId,
            "result-before-match-ended",
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
                    Rewards: new Dictionary<string, object> { ["coin"] = 5 })
            });

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
        Assert.Equal(0, await db.InventoryItems.CountAsync(x => x.PlayerId == playerId));
        Assert.Equal(0, await db.InventoryLogs.CountAsync(x => x.PlayerId == playerId));

        var profile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == playerId);
        var stats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == playerId);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);

        Assert.Equal(0, profile.Exp);
        Assert.Equal(1, profile.Level);
        Assert.Equal(0, stats.TotalMatches);
        Assert.Equal("IN_PROGRESS", session.Status);
    }

    [Fact]
    public async Task SubmitMatchResultAsync_WhenIdempotencyKeyBelongsToOtherSession_ReturnsNullWithoutReusingOtherResult()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var firstSessionId = Guid.NewGuid();
        var firstServerId = Guid.NewGuid();
        var firstPlayerId = Guid.NewGuid();
        var secondSessionId = Guid.NewGuid();
        var secondServerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();

        SeedSettlingSession(db, firstSessionId, firstServerId, firstPlayerId);
        SeedSettlingSession(db, secondSessionId, secondServerId, secondPlayerId);
        await db.SaveChangesAsync();

        var firstRequest = CreateSettlementRequest(firstSessionId, firstPlayerId, "shared-result-key", 1200);
        var secondRequest = CreateSettlementRequest(secondSessionId, secondPlayerId, "shared-result-key", 900);

        var first = await settlement.SubmitMatchResultAsync(firstRequest);
        var second = await settlement.SubmitMatchResultAsync(secondRequest);

        Assert.NotNull(first);
        Assert.Null(second);
        Assert.Equal(1, await db.MatchResults.CountAsync());
        Assert.Equal(1, await db.MatchResults.CountAsync(x => x.SessionId == firstSessionId));
        Assert.Equal(0, await db.MatchResults.CountAsync(x => x.SessionId == secondSessionId));
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync(x => x.PlayerId == secondPlayerId));
        Assert.Equal(0, await db.InventoryItems.CountAsync(x => x.PlayerId == secondPlayerId));
        Assert.Equal(0, await db.InventoryLogs.CountAsync(x => x.PlayerId == secondPlayerId));

        var firstProfile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == firstPlayerId);
        var secondProfile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == secondPlayerId);
        var secondStats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == secondPlayerId);
        var secondSession = await db.GameSessions.SingleAsync(x => x.Id == secondSessionId);

        Assert.Equal(1200, firstProfile.Exp);
        Assert.Equal(0, secondProfile.Exp);
        Assert.Equal(0, secondStats.TotalMatches);
        Assert.Equal("SETTLING", secondSession.Status);
    }

    [Fact]
    public async Task SubmitMatchResultAsync_WhenSessionPlayerIsMissing_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var firstPlayerId = Guid.NewGuid();
        var missingPlayerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, firstPlayerId);
        SeedAdditionalPlayer(db, sessionId, missingPlayerId, "MissingSettlementTester");
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(sessionId, firstPlayerId, "result-missing-session-player", 1200);

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
        Assert.Equal(0, await db.InventoryItems.CountAsync(x => x.PlayerId == firstPlayerId));
        Assert.Equal(0, await db.InventoryLogs.CountAsync(x => x.PlayerId == firstPlayerId));

        var firstProfile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == firstPlayerId);
        var missingProfile = await db.PlayerProfiles.SingleAsync(x => x.PlayerId == missingPlayerId);
        var firstStats = await db.PlayerStatistics.SingleAsync(x => x.PlayerId == firstPlayerId);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);

        Assert.Equal(0, firstProfile.Exp);
        Assert.Equal(0, missingProfile.Exp);
        Assert.Equal(0, firstStats.TotalMatches);
        Assert.Equal("SETTLING", session.Status);
    }

    [Fact]
    public async Task SubmitMatchResultAsync_WhenPlayerIsDuplicated_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = new SubmitMatchResultRequest(
            sessionId,
            "result-duplicate-player",
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
                    Rewards: new Dictionary<string, object> { ["coin"] = 5 }),
                new MatchPlayerResultDto(
                    playerId,
                    "blue",
                    "win",
                    Kills: 2,
                    Deaths: 0,
                    Assists: 1,
                    Score: 500,
                    ExpDelta: 900,
                    Rewards: new Dictionary<string, object> { ["coin"] = 3 })
            });

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task SubmitMatchResultAsync_WhenNoSessionPlayersExist_ReturnsNullWithoutCompletingSession()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();

        SeedSettlingSessionWithoutPlayers(db, sessionId, serverId);
        await db.SaveChangesAsync();

        var request = new SubmitMatchResultRequest(
            sessionId,
            "result-empty-session-players",
            """{"winner":"none"}""",
            Array.Empty<MatchPlayerResultDto>());

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
        Assert.Equal(0, await db.MatchResults.CountAsync());
        Assert.Equal(0, await db.MatchPlayerResults.CountAsync());
        Assert.Equal(0, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId));

        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);

        Assert.Equal("SETTLING", session.Status);
        Assert.Equal("ENDING", server.Status);
        Assert.Null(session.EndedAt);
    }

    [Fact]
    public async Task SubmitMatchResultAsync_WhenPlayerTeamDiffersFromSession_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        db.PlayerSessions.Local.Single(x => x.GameSessionId == sessionId && x.PlayerId == playerId).Team = "red";
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(sessionId, playerId, "result-team-mismatch", 1200);

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task SubmitMatchResultAsync_WhenPlayerResultIsInvalid_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(
            sessionId,
            playerId,
            "result-invalid-player-result",
            1200,
            playerResult: "eliminated");

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task SubmitMatchResultAsync_WhenPlayerStatsContainNegativeValue_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(
            sessionId,
            playerId,
            "result-negative-player-stats",
            -1);

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task SubmitMatchResultAsync_WhenRewardQuantityIsNegative_ReturnsNullWithoutRewardsOrStats()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedSettlingSession(db, sessionId, serverId, playerId);
        await db.SaveChangesAsync();

        var request = CreateSettlementRequest(
            sessionId,
            playerId,
            "result-negative-reward",
            1200,
            rewards: new Dictionary<string, object> { ["coin"] = -5 });

        var result = await settlement.SubmitMatchResultAsync(request);

        Assert.Null(result);
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
    public async Task GetMatchResultAsync_ReturnsPlayerResults()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();

        SeedCompletedMatchResult(db, sessionId, serverId, playerId, DateTimeOffset.UtcNow);
        await db.SaveChangesAsync();

        var resultId = await db.MatchResults
            .Where(x => x.SessionId == sessionId)
            .Select(x => x.Id)
            .SingleAsync();

        db.ChangeTracker.Clear();

        var result = await settlement.GetMatchResultAsync(resultId);

        Assert.NotNull(result);
        var playerResult = Assert.Single(result!.PlayerResults);
        Assert.Equal(playerId, playerResult.PlayerId);
        Assert.Equal("win", playerResult.Result);
    }

    [Fact]
    public async Task GetSessionResultsAsync_ReturnsLatestFirstWithPlayerResults()
    {
        await using var db = CreateDbContext();
        var inventory = new InventoryService(db, NullLogger<InventoryService>.Instance);
        var settlement = new SettlementService(db, inventory, NullLogger<SettlementService>.Instance);
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var firstPlayerId = Guid.NewGuid();
        var secondPlayerId = Guid.NewGuid();
        var earlier = DateTimeOffset.UtcNow.AddMinutes(-5);
        var later = DateTimeOffset.UtcNow;

        SeedCompletedMatchResult(db, sessionId, serverId, firstPlayerId, earlier);
        SeedCompletedMatchResult(db, sessionId, serverId, secondPlayerId, later);
        await db.SaveChangesAsync();

        db.ChangeTracker.Clear();

        var results = await settlement.GetSessionResultsAsync(sessionId);

        Assert.Collection(
            results,
            latest =>
            {
                Assert.Equal(later, latest.CreatedAt);
                Assert.Single(latest.PlayerResults, x => x.PlayerId == secondPlayerId);
            },
            oldest =>
            {
                Assert.Equal(earlier, oldest.CreatedAt);
                Assert.Single(oldest.PlayerResults, x => x.PlayerId == firstPlayerId);
            });
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"settlement-inventory-{Guid.NewGuid()}")
            .ConfigureWarnings(x => x.Ignore(InMemoryEventId.TransactionIgnoredWarning))
            .Options;
        return new GameDbContext(options);
    }

    private static SubmitMatchResultRequest CreateSettlementRequest(
        Guid sessionId,
        Guid playerId,
        string idempotencyKey,
        int expDelta,
        string playerResult = "win",
        IReadOnlyDictionary<string, object>? rewards = null)
    {
        return new SubmitMatchResultRequest(
            sessionId,
            idempotencyKey,
            """{"winner":"blue"}""",
            new[]
            {
                new MatchPlayerResultDto(
                    playerId,
                    "blue",
                    playerResult,
                    Kills: 4,
                    Deaths: 1,
                    Assists: 2,
                    Score: 900,
                    ExpDelta: expDelta,
                    Rewards: rewards ?? new Dictionary<string, object> { ["coin"] = 5 })
            });
    }

    private static void SeedRunningSession(GameDbContext db, Guid sessionId, Guid serverId, Guid playerId)
    {
        db.GameSessions.Add(new GameSession
        {
            Id = sessionId,
            SourceType = "ROOM",
            SourceId = Guid.NewGuid(),
            Mode = "classic",
            MapId = "arena_01",
            Region = "cn",
            Status = "IN_PROGRESS",
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
            Status = "IN_PROGRESS",
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-9),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
        });
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = "SettlementTester",
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
        db.PlayerStatistics.Add(new PlayerStatistics
        {
            PlayerId = playerId,
            UpdatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
    }

    private static void SeedSettlingSession(GameDbContext db, Guid sessionId, Guid serverId, Guid playerId)
    {
        SeedRunningSession(db, sessionId, serverId, playerId);
        var session = db.GameSessions.Local.Single(x => x.Id == sessionId);
        var server = db.GameServerInstances.Local.Single(x => x.Id == serverId);
        session.Status = "SETTLING";
        server.Status = "ENDING";
    }

    private static void SeedSettlingSessionWithoutPlayers(GameDbContext db, Guid sessionId, Guid serverId)
    {
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
            StartedAt = DateTimeOffset.UtcNow.AddMinutes(-8),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-8)
        });
    }

    private static void SeedAdditionalPlayer(GameDbContext db, Guid sessionId, Guid playerId, string nickname)
    {
        db.PlayerSessions.Add(new PlayerSession
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            PlayerId = playerId,
            Team = "blue",
            Status = "JOINED",
            SessionTokenHash = "hash",
            SessionTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(10),
            CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-9)
        });
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = nickname,
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
        db.PlayerStatistics.Add(new PlayerStatistics
        {
            PlayerId = playerId,
            UpdatedAt = DateTimeOffset.UtcNow.AddDays(-1)
        });
    }

    private static void SeedCompletedMatchResult(GameDbContext db, Guid sessionId, Guid serverId, Guid playerId, DateTimeOffset createdAt)
    {
        var resultId = Guid.NewGuid();
        db.MatchResults.Add(new MatchResult
        {
            Id = resultId,
            SessionId = sessionId,
            ServerId = serverId,
            Mode = "classic",
            MapId = "arena_01",
            DurationSeconds = 600,
            ResultJson = """{"winner":"blue"}""",
            IdempotencyKey = $"result-{resultId:N}",
            CreatedAt = createdAt
        });
        db.MatchPlayerResults.Add(new MatchPlayerResult
        {
            Id = Guid.NewGuid(),
            MatchResultId = resultId,
            PlayerId = playerId,
            Team = "blue",
            Result = "win",
            Kills = 3,
            Deaths = 1,
            Assists = 2,
            Score = 800,
            ExpDelta = 500,
            RewardJson = """{"coin":5}""",
            CreatedAt = createdAt
        });
    }
}
