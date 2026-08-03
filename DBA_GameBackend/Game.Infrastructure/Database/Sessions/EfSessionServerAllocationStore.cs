/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：验证已绑定服务器的 Runtime Token，并原子保存会话与服务器监听地址。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Sessions;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Sessions;

public sealed class EfSessionServerAllocationStore(GameDbContext db) : ISessionServerAllocationStore
{
    public async Task<SessionLifecycleState?> TryAllocateAsync(
        AllocateSessionServerCommand command,
        CancellationToken cancellationToken = default)
    {
        var session = await db.GameSessions
            .FirstOrDefaultAsync(x => x.Id == command.SessionId, cancellationToken);
        if (session is null
            || !command.AllowedCurrentStatuses.Contains(session.Status)
            || session.ServerId is not Guid serverId)
        {
            return null;
        }

        var tokenHash = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(command.RuntimeToken)))
            .ToLowerInvariant();
        var server = await db.GameServerInstances
            .FirstOrDefaultAsync(
                x => x.Id == serverId
                    && x.SessionId == command.SessionId
                    && x.RuntimeTokenHash == tokenHash
                    && x.RuntimeTokenExpiresAt > command.OccurredAt,
                cancellationToken);
        if (server is null)
        {
            return null;
        }

        if (session.Status == "CREATED")
        {
            session.Status = "ALLOCATING_SERVER";
            session.AllocatedAt = command.OccurredAt;
        }

        session.ServerIp = command.Ip;
        session.ServerPort = command.Port;
        session.UpdatedAt = command.OccurredAt;
        server.Ip = command.Ip;
        server.Port = command.Port;
        server.UpdatedAt = command.OccurredAt;
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
