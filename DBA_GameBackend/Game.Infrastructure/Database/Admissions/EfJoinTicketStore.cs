/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：使用 EF Core 原子校验并消费一次性入服票据，实现 Game.Application 定义的存储端口。
- 安全约束：数据库只保存票据 Hash；消费条件包含账号、角色、会话、服务器实例、构建版本和冻结构筑。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Application.Sessions;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Admissions;

public sealed class EfJoinTicketStore(GameDbContext db) : IJoinTicketStore
{
    public async Task<ConsumedJoinTicket?> TryConsumeAsync(
        ConsumeJoinTicketCommand command,
        CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        var presentedTicketHash = HashToken(command.JoinTicket);
        var consumedTicketHash = HashToken(Convert.ToBase64String(RandomNumberGenerator.GetBytes(32)));

        var affectedRows = await db.PlayerSessions
            .Where(x => x.GameSessionId == command.SessionId
                && x.PlayerId == command.AccountId
                && x.CharacterId == command.CharacterId
                && x.SessionTokenServerId == command.ServerInstanceId
                && x.SessionTokenBuildId == command.BuildId
                && x.SessionTokenHash == presentedTicketHash
                && x.SessionTokenExpiresAt > now
                && x.Status != "JOINED"
                && x.Zodiac == command.Zodiac
                && x.PrimaryElement == command.PrimaryElement
                && x.FixedSkillGroupId == command.FixedSkillGroupId
                && x.GameSession != null
                && x.GameSession.ServerId == command.ServerInstanceId
                && x.GameSession.BuildVersion == command.BuildId)
            .ExecuteUpdateAsync(setters => setters
                .SetProperty(x => x.Status, "JOINED")
                .SetProperty(x => x.JoinedAt, now)
                .SetProperty(x => x.SessionTokenHash, consumedTicketHash)
                .SetProperty(x => x.SessionTokenExpiresAt, now),
                cancellationToken);
        if (affectedRows != 1)
        {
            return null;
        }

        return await db.PlayerSessions
            .Where(x => x.GameSessionId == command.SessionId && x.PlayerId == command.AccountId)
            .Select(x => new ConsumedJoinTicket(
                x.PlayerId,
                x.CharacterId!.Value,
                x.GameSessionId,
                command.ServerInstanceId,
                command.BuildId,
                x.Team,
                x.Zodiac!,
                x.PrimaryElement!,
                x.FiveCamp,
                x.FixedSkillGroupId!))
            .SingleAsync(cancellationToken);
    }

    private static string HashToken(string token)
    {
        return Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(token))).ToLowerInvariant();
    }
}
