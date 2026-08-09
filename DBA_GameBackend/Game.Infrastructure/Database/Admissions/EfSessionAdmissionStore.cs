/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：为连接信息签发用例读取会话快照，并在复核服务器绑定后持久化一次性凭证与冻结构筑。
- 安全约束：数据库只保存凭证 Hash；提交前重新检查玩家仍在会话、服务器实例和构建版本未变化。
*/

using Game.Application.Characters;
using Game.Application.Sessions;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Admissions;

public sealed class EfSessionAdmissionStore(GameDbContext db) : ISessionAdmissionStore
{
    public async Task<SessionAdmissionSnapshot?> LoadAsync(
        Guid sessionId,
        Guid playerId,
        CancellationToken cancellationToken = default)
    {
        var playerSession = await db.PlayerSessions
            .AsNoTracking()
            .FirstOrDefaultAsync(
                x => x.GameSessionId == sessionId && x.PlayerId == playerId && x.LeftAt == null,
                cancellationToken);
        if (playerSession is null)
        {
            return null;
        }

        var session = await db.GameSessions
            .AsNoTracking()
            .FirstOrDefaultAsync(x => x.Id == sessionId, cancellationToken);
        if (session is null)
        {
            return null;
        }

        var server = session.ServerId is Guid serverId
            ? await db.GameServerInstances.AsNoTracking().FirstOrDefaultAsync(x => x.Id == serverId, cancellationToken)
            : null;
        // 优先读取本次 GameSession 在分配阶段已经冻结的 CharacterId。角色选择是按区服进行的，
        // 不能在签发连接票据时再从账号的全局“最近选中角色”回退，否则快速换服后可能把另一个
        // 区服的角色构筑写入本次会话。仅当旧会话尚未冻结角色时，才保留既有 IsSelected 兼容查询。
        IQueryable<PlayerCharacter> selectedCharacterQuery = db.PlayerCharacters
            .AsNoTracking()
            .Where(x => x.PlayerId == playerId && !x.IsDeleted);
        if (playerSession.CharacterId is Guid frozenCharacterId)
        {
            selectedCharacterQuery = selectedCharacterQuery.Where(x => x.Id == frozenCharacterId);
        }
        else
        {
            selectedCharacterQuery = selectedCharacterQuery
                .Where(x => x.IsSelected)
                .OrderByDescending(x => x.LastUsedAt);
        }
        var selectedCharacter = await selectedCharacterQuery.FirstOrDefaultAsync(cancellationToken);

        return new SessionAdmissionSnapshot(
            session.Id,
            session.Status,
            session.ServerIp,
            session.ServerPort,
            session.ServerId,
            session.BuildVersion,
            server?.SessionId,
            server?.BuildVersion,
            playerSession.Id,
            playerSession.PlayerId,
            playerSession.CharacterId,
            playerSession.Team,
            playerSession.SlotIndex,
            new CharacterBuildSnapshot(
                playerSession.Zodiac,
                playerSession.PrimaryElement,
                playerSession.FiveCamp,
                playerSession.FixedSkillGroupId),
            selectedCharacter is null
                ? null
                : new SelectedSessionCharacter(
                    selectedCharacter.Id,
                    selectedCharacter.Zodiac,
                    selectedCharacter.PrimaryElement,
                    selectedCharacter.FiveCamp),
            playerSession.SessionTokenHash);
    }

    public async Task<bool> TryCommitAsync(
        SessionAdmissionCommit commit,
        CancellationToken cancellationToken = default)
    {
        var session = await db.GameSessions
            .FirstOrDefaultAsync(x => x.Id == commit.SessionId, cancellationToken);
        if (session is null
            || session.Status is not ("WAITING_PLAYERS" or "IN_PROGRESS")
            || session.ServerId != commit.ServerInstanceId
            || !string.Equals(session.BuildVersion?.Trim(), commit.BuildId, StringComparison.Ordinal))
        {
            return false;
        }

        var server = await db.GameServerInstances
            .AsNoTracking()
            .FirstOrDefaultAsync(x => x.Id == commit.ServerInstanceId, cancellationToken);
        if (server?.SessionId != commit.SessionId
            || !string.Equals(server.BuildVersion?.Trim(), commit.BuildId, StringComparison.Ordinal))
        {
            return false;
        }

        var playerSession = db.PlayerSessions.Local
            .FirstOrDefault(x => x.Id == commit.PlayerSessionId);
        if (playerSession is null)
        {
            playerSession = new PlayerSession { Id = commit.PlayerSessionId };
            db.PlayerSessions.Attach(playerSession);
        }

        var entry = db.Entry(playerSession);
        entry.Property(x => x.SessionTokenHash).OriginalValue = commit.ExpectedSessionTokenHash;
        entry.Property(x => x.LeftAt).OriginalValue = null;
        SetModified(entry.Property(x => x.CharacterId), commit.CharacterId);
        SetModified(entry.Property(x => x.Zodiac), commit.Build.Zodiac);
        SetModified(entry.Property(x => x.PrimaryElement), commit.Build.PrimaryElement);
        SetModified(entry.Property(x => x.FiveCamp), commit.Build.FiveCamp);
        SetModified(entry.Property(x => x.FixedSkillGroupId), commit.Build.FixedSkillGroupId);
        SetModified(entry.Property(x => x.SessionTokenHash), commit.ConnectionCredential.Hash);
        SetModified(entry.Property(x => x.SessionTokenExpiresAt), commit.ConnectionCredential.ExpiresAt);
        SetModified(entry.Property(x => x.SessionTokenServerId), commit.ServerInstanceId);
        SetModified(entry.Property(x => x.SessionTokenBuildId), commit.BuildId);
        SetModified(entry.Property(x => x.ReconnectTokenHash), commit.ReconnectCredential.Hash);
        SetModified(entry.Property(x => x.ReconnectTokenExpiresAt), commit.ReconnectCredential.ExpiresAt);

        try
        {
            await db.SaveChangesAsync(cancellationToken);
            return true;
        }
        catch (DbUpdateConcurrencyException)
        {
            entry.State = EntityState.Detached;
            return false;
        }
    }

    private static void SetModified<TEntity, TProperty>(
        Microsoft.EntityFrameworkCore.ChangeTracking.PropertyEntry<TEntity, TProperty> property,
        TProperty value)
        where TEntity : class
    {
        property.CurrentValue = value;
        property.IsModified = true;
    }
}
