/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：实现会话只读查询端口并返回应用层快照。
*/

using Game.Application.Sessions;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Sessions;

public sealed class EfSessionQueryStore(GameDbContext db) : ISessionQueryStore
{
    public async Task<SessionLifecycleState?> FindByIdAsync(
        Guid sessionId,
        CancellationToken cancellationToken = default)
    {
        return await db.GameSessions
            .AsNoTracking()
            .Where(x => x.Id == sessionId)
            .Select(x => new SessionLifecycleState(
                x.Id,
                x.SourceType,
                x.Mode,
                x.MapId,
                x.Region,
                x.Status,
                x.ServerIp,
                x.ServerPort,
                x.MaxPlayers,
                x.CreatedAt,
                x.StartedAt))
            .FirstOrDefaultAsync(cancellationToken);
    }
}
