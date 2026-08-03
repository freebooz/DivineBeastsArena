/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：验证匹配服务在双人入队后能创建共享会话，并在查询时推进配对。
- 阅读重点：测试名称即业务规则；Arrange 创建两张票据，Act 触发配对，Assert 检查会话与票据状态。
- 修改提示：调整匹配人数或会话字段时，请同步更新这些测试。
*/

using Game.Api.Services.Match;
using Game.Infrastructure.Database;
using Game.Infrastructure.Redis;
using Game.ServerManagement.DedicatedServers;
using Game.Shared.Contracts.Match;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Options;
using StackExchange.Redis;

namespace Game.Api.Tests;

public class MatchServiceTests
{
    [Fact]
    public async Task CreateTicketAsync_WhenSecondPlayerQueues_CreatesSharedMatchedSession()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db);
        var playerA = Guid.NewGuid();
        var playerB = Guid.NewGuid();

        var ticketA = await service.CreateTicketAsync(playerA, new CreateMatchmakingTicketRequest("default", "local"));
        Assert.Equal("QUEUED", ticketA.Status);
        Assert.Null(ticketA.MatchedSessionId);

        var ticketB = await service.CreateTicketAsync(playerB, new CreateMatchmakingTicketRequest("default", "local"));
        Assert.Equal("MATCHED", ticketB.Status);
        Assert.NotNull(ticketB.MatchedSessionId);
        Assert.Equal(ticketB.MatchedSessionId, ticketB.SessionId);

        var refreshedA = await service.GetTicketAsync(ticketA.Id);
        Assert.NotNull(refreshedA);
        Assert.Equal("MATCHED", refreshedA!.Status);
        Assert.Equal(ticketB.MatchedSessionId, refreshedA.MatchedSessionId);

        var sessionId = ticketB.MatchedSessionId!.Value;
        Assert.Equal(1, await db.GameSessions.CountAsync(x => x.Id == sessionId));
        Assert.Equal(2, await db.PlayerSessions.CountAsync(x => x.GameSessionId == sessionId));
        Assert.Equal("LobbyMap", (await db.GameSessions.SingleAsync(x => x.Id == sessionId)).MapId);

        var teams = await db.PlayerSessions
            .Where(x => x.GameSessionId == sessionId)
            .Select(x => x.Team)
            .OrderBy(x => x)
            .ToListAsync();
        Assert.Equal(new[] { "1", "2" }, teams);
    }

    [Fact]
    public async Task GetTicketAsync_WhenTwoQueuedTicketsExist_PairsThem()
    {
        await using var db = CreateDbContext();
        var service = CreateService(db);
        var playerA = Guid.NewGuid();
        var playerB = Guid.NewGuid();

        var ticketA = await service.CreateTicketAsync(playerA, new CreateMatchmakingTicketRequest("classic", "cn", 1200));
        // 人为保持第二张票未触发 Create 内配对：直接插入第二张 QUEUED 票后走 GetTicket 推进。
        db.MatchmakingTickets.Add(new Game.Infrastructure.Database.Entities.MatchmakingTicket
        {
            Id = Guid.NewGuid(),
            PlayerId = playerB,
            Mode = "classic",
            Region = "cn",
            Mmr = 1210,
            Status = "QUEUED",
            CreatedAt = DateTimeOffset.UtcNow.AddSeconds(1),
            TimeoutAt = DateTimeOffset.UtcNow.AddMinutes(5)
        });
        await db.SaveChangesAsync();

        var refreshedA = await service.GetTicketAsync(ticketA.Id);
        Assert.NotNull(refreshedA);
        Assert.Equal("MATCHED", refreshedA!.Status);
        Assert.NotNull(refreshedA.MatchedSessionId);
    }

    private static MatchService CreateService(GameDbContext db)
    {
        var options = Options.Create(new DedicatedServerOrchestrationOptions
        {
            ServerMode = "External",
            PublicIp = "127.0.0.1",
            PortRangeStart = 7777,
            PortRangeEnd = 7797,
            BackendUrl = "http://localhost:8080",
            AllowMockServerAllocation = true,
            MaxServersPerMachine = 8
        });
        var orchestrator = new DedicatedServerOrchestrator(
            db,
            options,
            NullLogger<DedicatedServerOrchestrator>.Instance);
        return new MatchService(
            db,
            new NullRedisConnectionFactory(),
            orchestrator,
            options,
            NullLogger<MatchService>.Instance,
            TestCharacterBuildFactory.CreatePolicy());
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"match-service-{Guid.NewGuid()}")
            .ConfigureWarnings(x => x.Ignore(Microsoft.EntityFrameworkCore.Diagnostics.InMemoryEventId.TransactionIgnoredWarning))
            .Options;
        return new GameDbContext(options);
    }

    private sealed class NullRedisConnectionFactory : IRedisConnectionFactory
    {
        public IDatabase GetDatabase(int db = -1) => throw new NotSupportedException();

        public IServer GetServer() => throw new NotSupportedException();
    }
}
