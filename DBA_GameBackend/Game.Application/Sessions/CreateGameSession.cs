/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：从房间或匹配票据生成会话草案，冻结已选角色构筑并签发临时凭证。
- 架构约束：来源读取与事务持久化通过 ISessionCreationStore 完成，不依赖 EF Core 实体。
*/

using Game.Application.Characters;
using Game.Shared.Contracts.Character;
using Game.Shared.Options;

namespace Game.Application.Sessions;

public sealed record SessionSourceParticipant(
    Guid PlayerId,
    int? SlotIndex,
    string? Team,
    SelectedSessionCharacter? SelectedCharacter);

public sealed record RoomSessionSource(
    Guid RoomId,
    string Mode,
    string MapId,
    string Region,
    int MaxPlayers,
    IReadOnlyList<SessionSourceParticipant> Participants);

public sealed record MatchSessionSource(
    Guid TicketId,
    Guid PlayerId,
    string Mode,
    string Region,
    SelectedSessionCharacter? SelectedCharacter);

public sealed record SessionParticipantDraft(
    Guid PlayerId,
    Guid? CharacterId,
    int? SlotIndex,
    string? Team,
    CharacterBuildSummaryDto? Build,
    IssuedSessionCredential ProvisionalCredential);

public sealed record SessionCreationDraft(
    Guid SessionId,
    string SourceType,
    Guid SourceId,
    string Mode,
    string MapId,
    string Region,
    int MaxPlayers,
    DateTimeOffset CreatedAt,
    IReadOnlyList<SessionParticipantDraft> Participants);

public interface ISessionCreationStore
{
    Task<SessionLifecycleState?> FindExistingAsync(
        string sourceType,
        Guid sourceId,
        CancellationToken cancellationToken = default);

    Task<RoomSessionSource?> LoadRoomSourceAsync(
        Guid roomId,
        CancellationToken cancellationToken = default);

    Task<MatchSessionSource?> LoadQueuedMatchSourceAsync(
        Guid ticketId,
        CancellationToken cancellationToken = default);

    Task<SessionLifecycleState?> TryCreateFromRoomAsync(
        SessionCreationDraft draft,
        CancellationToken cancellationToken = default);

    Task<SessionLifecycleState?> TryCreateFromMatchAsync(
        SessionCreationDraft draft,
        CancellationToken cancellationToken = default);
}

public interface ICreateSessionFromRoomUseCase
{
    Task<SessionLifecycleState?> ExecuteAsync(Guid roomId, CancellationToken cancellationToken = default);
}

public interface ICreateSessionFromMatchUseCase
{
    Task<SessionLifecycleState?> ExecuteAsync(Guid ticketId, CancellationToken cancellationToken = default);
}

public sealed class CreateSessionFromRoomUseCase(
    ISessionCreationStore store,
    ICharacterBuildPolicy characterBuildPolicy,
    ISessionCredentialIssuer credentialIssuer,
    TimeProvider timeProvider) : ICreateSessionFromRoomUseCase
{
    public async Task<SessionLifecycleState?> ExecuteAsync(
        Guid roomId,
        CancellationToken cancellationToken = default)
    {
        var existing = await store.FindExistingAsync("ROOM", roomId, cancellationToken);
        if (existing is not null)
        {
            return existing;
        }

        var source = await store.LoadRoomSourceAsync(roomId, cancellationToken);
        if (source is null)
        {
            return null;
        }

        var draft = new SessionCreationDraft(
            Guid.NewGuid(),
            "ROOM",
            source.RoomId,
            source.Mode,
            source.MapId,
            source.Region,
            source.MaxPlayers,
            timeProvider.GetUtcNow(),
            source.Participants.Select(BuildParticipant).ToList());
        return await store.TryCreateFromRoomAsync(draft, cancellationToken);
    }

    private SessionParticipantDraft BuildParticipant(SessionSourceParticipant participant)
    {
        var selected = participant.SelectedCharacter;
        var build = selected is null
            ? null
            : characterBuildPolicy.BuildSummary(selected.Zodiac, selected.PrimaryElement, selected.FiveCamp);
        return new SessionParticipantDraft(
            participant.PlayerId,
            selected?.CharacterId,
            participant.SlotIndex,
            participant.Team,
            build,
            credentialIssuer.IssueProvisionalCredential());
    }
}

public sealed class CreateSessionFromMatchUseCase(
    ISessionCreationStore store,
    ICharacterBuildPolicy characterBuildPolicy,
    ISessionCredentialIssuer credentialIssuer,
    SessionAdmissionOptions options,
    TimeProvider timeProvider) : ICreateSessionFromMatchUseCase
{
    public async Task<SessionLifecycleState?> ExecuteAsync(
        Guid ticketId,
        CancellationToken cancellationToken = default)
    {
        var existing = await store.FindExistingAsync("MATCH", ticketId, cancellationToken);
        if (existing is not null)
        {
            return existing;
        }

        var source = await store.LoadQueuedMatchSourceAsync(ticketId, cancellationToken);
        if (source is null)
        {
            return null;
        }

        var selected = source.SelectedCharacter;
        var build = selected is null
            ? null
            : characterBuildPolicy.BuildSummary(selected.Zodiac, selected.PrimaryElement, selected.FiveCamp);
        var participant = new SessionParticipantDraft(
            source.PlayerId,
            selected?.CharacterId,
            null,
            null,
            build,
            credentialIssuer.IssueProvisionalCredential());
        var draft = new SessionCreationDraft(
            Guid.NewGuid(),
            "MATCH",
            source.TicketId,
            source.Mode,
            options.MatchMapId.Trim(),
            source.Region,
            options.MatchMaxPlayers,
            timeProvider.GetUtcNow(),
            new[] { participant });
        return await store.TryCreateFromMatchAsync(draft, cancellationToken);
    }
}
