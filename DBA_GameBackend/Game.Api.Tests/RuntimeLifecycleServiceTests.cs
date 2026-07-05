/*
中文阅读说明：
- 所属应用：DBA_GameBackend 测试项目。
- 文件职责：验证 Runtime match-started / match-ended 的服务层状态推进和事件落库。
- 阅读重点：Dedicated Server 只负责上报生命周期，后端必须权威维护 Session / Server 状态。
- 修改提示：新增 Runtime 生命周期字段时，请先补这里的行为测试，再修改服务实现。
*/

using Game.Api.Services.Runtime;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Tests;

public class RuntimeLifecycleServiceTests
{
    [Fact]
    public async Task MarkMatchStartedAsync_UpdatesSessionServerAndWritesEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "WAITING_PLAYERS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "READY"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchStartedAsync(db, serverId, sessionId);

        Assert.True(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        Assert.Equal("IN_PROGRESS", session.Status);
        Assert.NotNull(session.StartedAt);
        Assert.NotNull(session.UpdatedAt);
        Assert.Equal("IN_PROGRESS", server.Status);
        Assert.NotNull(server.LastHeartbeatAt);
        Assert.Contains(await db.SessionEvents.ToListAsync(), x => x.GameSessionId == sessionId && x.EventType == "MATCH_STARTED");
        Assert.Contains(await db.GameServerEvents.ToListAsync(), x => x.ServerId == serverId && x.EventType == "MATCH_STARTED");
    }

    [Fact]
    public async Task MarkMatchEndedAsync_UpdatesSessionServerAndWritesEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "IN_PROGRESS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "IN_PROGRESS"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchEndedAsync(db, serverId, sessionId);

        Assert.True(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        Assert.Equal("SETTLING", session.Status);
        Assert.NotNull(session.UpdatedAt);
        Assert.Equal("ENDING", server.Status);
        Assert.Contains(await db.SessionEvents.ToListAsync(), x => x.GameSessionId == sessionId && x.EventType == "MATCH_ENDED");
        Assert.Contains(await db.GameServerEvents.ToListAsync(), x => x.ServerId == serverId && x.EventType == "MATCH_ENDED");
    }

    [Fact]
    public async Task MarkMatchEndedAsync_WhenMatchHasNotStarted_ReturnsFalseWithoutEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "WAITING_PLAYERS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "READY"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchEndedAsync(db, serverId, sessionId);

        Assert.False(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        Assert.Equal("WAITING_PLAYERS", session.Status);
        Assert.Equal("READY", server.Status);
        Assert.Empty(await db.SessionEvents.ToListAsync());
        Assert.Empty(await db.GameServerEvents.ToListAsync());
    }

    [Fact]
    public async Task MarkMatchStartedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var otherSessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "WAITING_PLAYERS"));
        db.GameSessions.Add(CreateSession(otherSessionId, "WAITING_PLAYERS"));
        db.GameServerInstances.Add(CreateServer(serverId, otherSessionId, "READY"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchStartedAsync(db, serverId, sessionId);

        Assert.False(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        Assert.Equal("WAITING_PLAYERS", session.Status);
        Assert.Equal("READY", server.Status);
        Assert.Empty(await db.SessionEvents.ToListAsync());
        Assert.Empty(await db.GameServerEvents.ToListAsync());
    }

    [Fact]
    public async Task MarkMatchStartedAsync_WhenActivePlayerHasNotJoined_ReturnsFalseWithoutEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        var playerId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "WAITING_PLAYERS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "READY"));
        db.PlayerSessions.Add(CreatePlayerSession(sessionId, playerId, "CONNECTED"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchStartedAsync(db, serverId, sessionId);

        Assert.False(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        var playerSession = await db.PlayerSessions.SingleAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);
        Assert.Equal("WAITING_PLAYERS", session.Status);
        Assert.Equal("READY", server.Status);
        Assert.Equal("CONNECTED", playerSession.Status);
        Assert.Null(playerSession.JoinedAt);
        Assert.Empty(await db.SessionEvents.ToListAsync());
        Assert.Empty(await db.GameServerEvents.ToListAsync());
    }

    [Fact]
    public async Task MarkMatchEndedAsync_WhenServerBelongsToDifferentSession_ReturnsFalseWithoutEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var otherSessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "IN_PROGRESS"));
        db.GameSessions.Add(CreateSession(otherSessionId, "IN_PROGRESS"));
        db.GameServerInstances.Add(CreateServer(serverId, otherSessionId, "IN_PROGRESS"));
        await db.SaveChangesAsync();

        var result = await RuntimeLifecycleService.MarkMatchEndedAsync(db, serverId, sessionId);

        Assert.False(result);
        var session = await db.GameSessions.SingleAsync(x => x.Id == sessionId);
        var server = await db.GameServerInstances.SingleAsync(x => x.Id == serverId);
        Assert.Equal("IN_PROGRESS", session.Status);
        Assert.Equal("IN_PROGRESS", server.Status);
        Assert.Empty(await db.SessionEvents.ToListAsync());
        Assert.Empty(await db.GameServerEvents.ToListAsync());
    }

    [Fact]
    public async Task MarkMatchStartedAsync_WhenRepeated_DoesNotWriteDuplicateEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "WAITING_PLAYERS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "READY"));
        await db.SaveChangesAsync();

        var first = await RuntimeLifecycleService.MarkMatchStartedAsync(db, serverId, sessionId);
        var second = await RuntimeLifecycleService.MarkMatchStartedAsync(db, serverId, sessionId);

        Assert.True(first);
        Assert.True(second);
        Assert.Equal(1, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "MATCH_STARTED"));
        Assert.Equal(1, await db.GameServerEvents.CountAsync(x => x.ServerId == serverId && x.EventType == "MATCH_STARTED"));
    }

    [Fact]
    public async Task MarkMatchEndedAsync_WhenRepeated_DoesNotWriteDuplicateEvents()
    {
        await using var db = CreateDbContext();
        var sessionId = Guid.NewGuid();
        var serverId = Guid.NewGuid();
        db.GameSessions.Add(CreateSession(sessionId, "IN_PROGRESS"));
        db.GameServerInstances.Add(CreateServer(serverId, sessionId, "IN_PROGRESS"));
        await db.SaveChangesAsync();

        var first = await RuntimeLifecycleService.MarkMatchEndedAsync(db, serverId, sessionId);
        var second = await RuntimeLifecycleService.MarkMatchEndedAsync(db, serverId, sessionId);

        Assert.True(first);
        Assert.True(second);
        Assert.Equal(1, await db.SessionEvents.CountAsync(x => x.GameSessionId == sessionId && x.EventType == "MATCH_ENDED"));
        Assert.Equal(1, await db.GameServerEvents.CountAsync(x => x.ServerId == serverId && x.EventType == "MATCH_ENDED"));
    }

    private static GameDbContext CreateDbContext()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"runtime-lifecycle-{Guid.NewGuid()}")
            .Options;
        return new GameDbContext(options);
    }

    private static GameSession CreateSession(Guid id, string status) => new()
    {
        Id = id,
        SourceType = "room",
        Mode = "classic",
        MapId = "arena_01",
        Region = "cn",
        Status = status,
        MaxPlayers = 2
    };

    private static GameServerInstance CreateServer(Guid id, Guid sessionId, string status) => new()
    {
        Id = id,
        SessionId = sessionId,
        Mode = "classic",
        MapId = "arena_01",
        Region = "cn",
        Ip = "127.0.0.1",
        Port = 7777,
        Status = status
    };

    private static PlayerSession CreatePlayerSession(Guid sessionId, Guid playerId, string status) => new()
    {
        Id = Guid.NewGuid(),
        GameSessionId = sessionId,
        PlayerId = playerId,
        Team = "blue",
        SlotIndex = 0,
        Status = status,
        JoinedAt = status == "JOINED" ? DateTimeOffset.UtcNow.AddMinutes(-1) : null,
        CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-5)
    };
}
