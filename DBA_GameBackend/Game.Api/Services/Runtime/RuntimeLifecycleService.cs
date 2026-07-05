/*
中文阅读说明：
- 所属应用：DBA_GameBackend Runtime 服务。
- 文件职责：集中维护 Dedicated Server 上报的比赛生命周期状态推进与事件落库。
- 阅读重点：Endpoint 只负责鉴权和 HTTP 响应，本服务负责 Session / Server 的权威状态变化。
- 修改提示：新增生命周期事件时优先在这里补行为，并同步 RuntimeLifecycleServiceTests。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Runtime;

public static class RuntimeLifecycleService
{
    public static async Task<bool> MarkMatchStartedAsync(GameDbContext db, Guid serverId, Guid sessionId)
    {
        var session = await db.GameSessions.FindAsync(sessionId);
        if (session is null)
        {
            return false;
        }

        var server = await db.GameServerInstances
            .FirstOrDefaultAsync(x => x.Id == serverId && x.SessionId == sessionId);
        if (server is null)
        {
            return false;
        }

        var hasActivePlayerNotJoined = await db.PlayerSessions.AnyAsync(x =>
            x.GameSessionId == sessionId &&
            x.LeftAt == null &&
            (x.JoinedAt == null || x.Status != "JOINED"));
        if (hasActivePlayerNotJoined)
        {
            return false;
        }

        session.Status = "IN_PROGRESS";
        session.StartedAt ??= DateTimeOffset.UtcNow;
        session.UpdatedAt = DateTimeOffset.UtcNow;
        server.Status = "IN_PROGRESS";
        server.LastHeartbeatAt = DateTimeOffset.UtcNow;
        await AddSessionEventOnceAsync(db, sessionId, "MATCH_STARTED", "{}");
        await AddServerEventOnceAsync(db, serverId, "MATCH_STARTED", "{}");
        await db.SaveChangesAsync();
        return true;
    }

    public static async Task<bool> MarkMatchEndedAsync(GameDbContext db, Guid serverId, Guid sessionId)
    {
        var session = await db.GameSessions.FindAsync(sessionId);
        if (session is null)
        {
            return false;
        }

        var server = await db.GameServerInstances
            .FirstOrDefaultAsync(x => x.Id == serverId && x.SessionId == sessionId);
        if (server is null)
        {
            return false;
        }

        if (session.Status is not ("IN_PROGRESS" or "SETTLING") ||
            server.Status is not ("IN_PROGRESS" or "ENDING"))
        {
            return false;
        }

        session.Status = "SETTLING";
        session.UpdatedAt = DateTimeOffset.UtcNow;
        server.Status = "ENDING";
        await AddSessionEventOnceAsync(db, sessionId, "MATCH_ENDED", "{}");
        await AddServerEventOnceAsync(db, serverId, "MATCH_ENDED", "{}");
        await db.SaveChangesAsync();
        return true;
    }

    private static async Task AddSessionEventOnceAsync(GameDbContext db, Guid sessionId, string eventType, string payloadJson)
    {
        if (await db.SessionEvents.AnyAsync(x => x.GameSessionId == sessionId && x.EventType == eventType))
        {
            return;
        }

        db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            EventType = eventType,
            PayloadJson = payloadJson
        });
    }

    private static async Task AddServerEventOnceAsync(GameDbContext db, Guid serverId, string eventType, string payloadJson)
    {
        if (await db.GameServerEvents.AnyAsync(x => x.ServerId == serverId && x.EventType == eventType))
        {
            return;
        }

        db.GameServerEvents.Add(new GameServerEvent
        {
            Id = Guid.NewGuid(),
            ServerId = serverId,
            EventType = eventType,
            PayloadJson = payloadJson
        });
    }
}
