/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：读取房间/匹配来源并在事务中幂等创建会话和玩家会话。
*/

using Game.Application.Sessions;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using Npgsql;

namespace Game.Infrastructure.Database.Sessions;

public sealed class EfSessionCreationStore(GameDbContext db) : ISessionCreationStore
{
    public async Task<SessionLifecycleState?> FindExistingAsync(
        string sourceType,
        Guid sourceId,
        CancellationToken cancellationToken = default)
    {
        var session = await db.GameSessions
            .AsNoTracking()
            .FirstOrDefaultAsync(
                x => x.SourceType == sourceType && x.SourceId == sourceId,
                cancellationToken);
        return session is null ? null : ToState(session);
    }

    public async Task<RoomSessionSource?> LoadRoomSourceAsync(
        Guid roomId,
        CancellationToken cancellationToken = default)
    {
        var room = await db.GameRooms.AsNoTracking().FirstOrDefaultAsync(x => x.Id == roomId, cancellationToken);
        if (room is null)
        {
            return null;
        }

        var roomPlayers = await db.GameRoomPlayers
            .AsNoTracking()
            .Where(x => x.RoomId == roomId && x.LeftAt == null)
            .ToListAsync(cancellationToken);
        var selectedCharacters = await LoadSelectedCharactersAsync(
            roomPlayers.Select(x => x.PlayerId),
            cancellationToken);
        var participants = roomPlayers
            .Select(x => new SessionSourceParticipant(
                x.PlayerId,
                x.SlotIndex,
                x.Team,
                selectedCharacters.GetValueOrDefault(x.PlayerId)))
            .ToList();
        return new RoomSessionSource(
            room.Id,
            room.Mode,
            room.MapId,
            room.Region,
            room.MaxPlayers,
            participants);
    }

    public async Task<MatchSessionSource?> LoadQueuedMatchSourceAsync(
        Guid ticketId,
        CancellationToken cancellationToken = default)
    {
        var ticket = await db.MatchmakingTickets
            .AsNoTracking()
            .FirstOrDefaultAsync(x => x.Id == ticketId && x.Status == "QUEUED", cancellationToken);
        if (ticket is null)
        {
            return null;
        }

        var selectedCharacters = await LoadSelectedCharactersAsync(
            new[] { ticket.PlayerId },
            cancellationToken);
        return new MatchSessionSource(
            ticket.Id,
            ticket.PlayerId,
            ticket.Mode,
            ticket.Region,
            selectedCharacters.GetValueOrDefault(ticket.PlayerId));
    }

    public Task<SessionLifecycleState?> TryCreateFromRoomAsync(
        SessionCreationDraft draft,
        CancellationToken cancellationToken = default)
    {
        return TryCreateAsync(draft, updateSource: UpdateRoomSource, cancellationToken);
    }

    public Task<SessionLifecycleState?> TryCreateFromMatchAsync(
        SessionCreationDraft draft,
        CancellationToken cancellationToken = default)
    {
        return TryCreateAsync(draft, updateSource: UpdateMatchSource, cancellationToken);
    }

    private async Task<SessionLifecycleState?> TryCreateAsync(
        SessionCreationDraft draft,
        Func<SessionCreationDraft, CancellationToken, Task<bool>> updateSource,
        CancellationToken cancellationToken)
    {
        await using var transaction = await db.Database.BeginTransactionAsync(cancellationToken);
        var existing = await db.GameSessions
            .FirstOrDefaultAsync(
                x => x.SourceType == draft.SourceType && x.SourceId == draft.SourceId,
                cancellationToken);
        if (existing is not null)
        {
            await transaction.CommitAsync(cancellationToken);
            return ToState(existing);
        }

        if (!await updateSource(draft, cancellationToken))
        {
            await transaction.RollbackAsync(cancellationToken);
            return null;
        }

        var session = new GameSession
        {
            Id = draft.SessionId,
            SourceType = draft.SourceType,
            SourceId = draft.SourceId,
            Mode = draft.Mode,
            MapId = draft.MapId,
            Region = draft.Region,
            Status = "CREATED",
            MaxPlayers = draft.MaxPlayers,
            RetryCount = 0,
            CreatedAt = draft.CreatedAt
        };
        db.GameSessions.Add(session);
        foreach (var participant in draft.Participants)
        {
            db.PlayerSessions.Add(new PlayerSession
            {
                Id = Guid.NewGuid(),
                GameSessionId = session.Id,
                PlayerId = participant.PlayerId,
                CharacterId = participant.CharacterId,
                SlotIndex = participant.SlotIndex,
                Team = participant.Team,
                Zodiac = participant.Build?.Zodiac,
                PrimaryElement = participant.Build?.PrimaryElement,
                FiveCamp = participant.Build?.FiveCamp,
                FixedSkillGroupId = participant.Build?.FixedSkillGroupId,
                Status = "CREATED",
                SessionTokenHash = participant.ProvisionalCredential.Hash,
                SessionTokenExpiresAt = participant.ProvisionalCredential.ExpiresAt,
                CreatedAt = draft.CreatedAt
            });
        }

        try
        {
            await db.SaveChangesAsync(cancellationToken);
            await transaction.CommitAsync(cancellationToken);
            return ToState(session);
        }
        catch (DbUpdateException exception)
            when (exception.InnerException is PostgresException
                { SqlState: PostgresErrorCodes.UniqueViolation })
        {
            await transaction.RollbackAsync(cancellationToken);
            db.ChangeTracker.Clear();
            return await FindExistingAsync(draft.SourceType, draft.SourceId, cancellationToken);
        }
    }

    private async Task<bool> UpdateRoomSource(
        SessionCreationDraft draft,
        CancellationToken cancellationToken)
    {
        var room = await db.GameRooms.FirstOrDefaultAsync(x => x.Id == draft.SourceId, cancellationToken);
        if (room is null)
        {
            return false;
        }
        room.Status = "IN_GAME";
        return true;
    }

    private async Task<bool> UpdateMatchSource(
        SessionCreationDraft draft,
        CancellationToken cancellationToken)
    {
        var ticket = await db.MatchmakingTickets
            .FirstOrDefaultAsync(
                x => x.Id == draft.SourceId && x.Status == "QUEUED",
                cancellationToken);
        if (ticket is null)
        {
            return false;
        }
        ticket.Status = "MATCHED";
        ticket.MatchedSessionId = draft.SessionId;
        return true;
    }

    private async Task<Dictionary<Guid, SelectedSessionCharacter>> LoadSelectedCharactersAsync(
        IEnumerable<Guid> playerIds,
        CancellationToken cancellationToken)
    {
        var ids = playerIds.Distinct().ToList();
        var characters = await db.PlayerCharacters
            .AsNoTracking()
            .Where(x => ids.Contains(x.PlayerId) && x.IsSelected)
            .OrderByDescending(x => x.LastUsedAt)
            .ToListAsync(cancellationToken);
        return characters
            .GroupBy(x => x.PlayerId)
            .ToDictionary(
                x => x.Key,
                x => new SelectedSessionCharacter(
                    x.First().Id,
                    x.First().Zodiac,
                    x.First().PrimaryElement,
                    x.First().FiveCamp));
    }

    private static SessionLifecycleState ToState(GameSession session)
    {
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
