/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：执行应用层批准的会话状态转换，并在同一次保存中写入生命周期事件。
*/

using System.Text.Json;
using Game.Application.Sessions;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Sessions;

public sealed class EfSessionLifecycleStore(GameDbContext db) : ISessionLifecycleStore
{
    public async Task<SessionLifecycleState?> TryApplyAsync(
        SessionLifecycleMutation mutation,
        CancellationToken cancellationToken = default)
    {
        var session = await db.GameSessions
            .FirstOrDefaultAsync(x => x.Id == mutation.SessionId, cancellationToken);
        if (session is null
            || mutation.AllowedCurrentStatuses is not null
                && !mutation.AllowedCurrentStatuses.Contains(session.Status))
        {
            return null;
        }

        session.Status = mutation.TargetStatus;
        if (mutation.SetStartedAt)
        {
            session.StartedAt = mutation.OccurredAt;
        }
        if (mutation.SetEndedAt)
        {
            session.EndedAt = mutation.OccurredAt;
        }

        db.SessionEvents.Add(new SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = mutation.SessionId,
            EventType = mutation.EventType,
            PayloadJson = mutation.FailureReason is null
                ? "{}"
                : JsonSerializer.Serialize(new { reason = mutation.FailureReason }),
            CreatedAt = mutation.OccurredAt
        });
        await db.SaveChangesAsync(cancellationToken);

        return new SessionLifecycleState(
            session.Id,
            session.SourceType,
            session.Mode,
            session.MapId,
            session.Region,
            session.Status,
            session.ServerIp,
            session.ServerPort,
            session.MaxPlayers,
            session.CreatedAt,
            session.StartedAt);
    }
}
