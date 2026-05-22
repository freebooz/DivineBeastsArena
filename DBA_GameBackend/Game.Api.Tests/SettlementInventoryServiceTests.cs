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

        SeedRunningSession(db, sessionId, serverId, playerId);
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

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"settlement-inventory-{Guid.NewGuid()}")
            .ConfigureWarnings(x => x.Ignore(InMemoryEventId.TransactionIgnoredWarning))
            .Options;
        return new GameDbContext(options);
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
}
