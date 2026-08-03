/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：编排大厅/对局连接信息签发，统一校验会话状态、服务器绑定、角色冻结构筑和凭证持久化。
- 架构约束：不依赖 HTTP、EF Core 或数据库实体；持久化通过 ISessionAdmissionStore 端口完成。
*/

using Game.Application.Characters;
using Game.Shared.Contracts.Character;

namespace Game.Application.Sessions;

public sealed record SelectedSessionCharacter(
    Guid CharacterId,
    string? Zodiac,
    string? PrimaryElement,
    string? FiveCamp);

public sealed record SessionAdmissionSnapshot(
    Guid SessionId,
    string SessionStatus,
    string? ServerIp,
    int? ServerPort,
    Guid? ServerInstanceId,
    string? SessionBuildId,
    Guid? BoundServerSessionId,
    string? ServerBuildId,
    Guid PlayerSessionId,
    Guid PlayerId,
    Guid? CharacterId,
    string? Team,
    int? SlotIndex,
    CharacterBuildSnapshot FrozenBuild,
    SelectedSessionCharacter? SelectedCharacter,
    string CurrentSessionTokenHash);

public sealed record SessionAdmissionCommit(
    Guid SessionId,
    Guid PlayerSessionId,
    Guid PlayerId,
    Guid CharacterId,
    Guid ServerInstanceId,
    string BuildId,
    CharacterBuildSummaryDto Build,
    string ExpectedSessionTokenHash,
    IssuedSessionCredential ConnectionCredential,
    IssuedSessionCredential ReconnectCredential);

public sealed record IssuedSessionConnection(
    Guid SessionId,
    string ServerIp,
    int ServerPort,
    string JoinTicket,
    DateTimeOffset JoinTicketExpiresAt,
    Guid PlayerId,
    Guid CharacterId,
    Guid ServerInstanceId,
    string BuildId,
    int TeamId,
    CharacterBuildSummaryDto Build,
    string ReconnectToken,
    DateTimeOffset ReconnectTokenExpiresAt);

public interface ISessionAdmissionStore
{
    Task<SessionAdmissionSnapshot?> LoadAsync(
        Guid sessionId,
        Guid playerId,
        CancellationToken cancellationToken = default);

    Task<bool> TryCommitAsync(
        SessionAdmissionCommit commit,
        CancellationToken cancellationToken = default);
}

public interface IIssueSessionConnectionUseCase
{
    Task<IssuedSessionConnection?> ExecuteAsync(
        Guid sessionId,
        Guid playerId,
        CancellationToken cancellationToken = default);
}

public sealed class IssueSessionConnectionUseCase(
    ISessionAdmissionStore store,
    ICharacterBuildPolicy characterBuildPolicy,
    ISessionCredentialIssuer credentialIssuer) : IIssueSessionConnectionUseCase
{
    public async Task<IssuedSessionConnection?> ExecuteAsync(
        Guid sessionId,
        Guid playerId,
        CancellationToken cancellationToken = default)
    {
        var snapshot = await store.LoadAsync(sessionId, playerId, cancellationToken);
        if (snapshot is null
            || snapshot.SessionStatus is not ("WAITING_PLAYERS" or "IN_PROGRESS")
            || string.IsNullOrWhiteSpace(snapshot.ServerIp)
            || snapshot.ServerPort is null
            || snapshot.ServerInstanceId is null)
        {
            return null;
        }

        var buildId = snapshot.SessionBuildId?.Trim();
        var serverBuildId = snapshot.ServerBuildId?.Trim();
        if (snapshot.BoundServerSessionId != snapshot.SessionId
            || string.IsNullOrWhiteSpace(buildId)
            || !string.Equals(buildId, serverBuildId, StringComparison.Ordinal))
        {
            return null;
        }

        var resolvedBuild = ResolveBuild(snapshot);
        if (resolvedBuild is null)
        {
            return null;
        }

        var connectionCredential = credentialIssuer.IssueConnectionCredential();
        var reconnectCredential = credentialIssuer.IssueReconnectCredential();
        var commit = new SessionAdmissionCommit(
            snapshot.SessionId,
            snapshot.PlayerSessionId,
            snapshot.PlayerId,
            resolvedBuild.Value.CharacterId,
            snapshot.ServerInstanceId.Value,
            buildId,
            resolvedBuild.Value.Build,
            snapshot.CurrentSessionTokenHash,
            connectionCredential,
            reconnectCredential);
        if (!await store.TryCommitAsync(commit, cancellationToken))
        {
            return null;
        }

        return new IssuedSessionConnection(
            snapshot.SessionId,
            snapshot.ServerIp,
            snapshot.ServerPort.Value,
            connectionCredential.Plaintext,
            connectionCredential.ExpiresAt,
            snapshot.PlayerId,
            resolvedBuild.Value.CharacterId,
            snapshot.ServerInstanceId.Value,
            buildId,
            ResolveTeamId(snapshot.Team, snapshot.SlotIndex),
            resolvedBuild.Value.Build,
            reconnectCredential.Plaintext,
            reconnectCredential.ExpiresAt);
    }

    private (Guid CharacterId, CharacterBuildSummaryDto Build)? ResolveBuild(SessionAdmissionSnapshot snapshot)
    {
        var normalizedFrozen = characterBuildPolicy.NormalizeBuild(snapshot.FrozenBuild);
        if (!normalizedFrozen.IsValid)
        {
            return null;
        }

        if (normalizedFrozen.HasBuild)
        {
            var characterId = snapshot.CharacterId ?? snapshot.SelectedCharacter?.CharacterId;
            return characterId is null
                ? null
                : (characterId.Value, normalizedFrozen.Build!);
        }

        var selectedCharacter = snapshot.SelectedCharacter;
        if (selectedCharacter is null)
        {
            return null;
        }

        return (
            selectedCharacter.CharacterId,
            characterBuildPolicy.BuildSummary(
                selectedCharacter.Zodiac,
                selectedCharacter.PrimaryElement,
                selectedCharacter.FiveCamp));
    }

    private static int ResolveTeamId(string? team, int? slotIndex)
    {
        var normalizedTeam = team?.Trim();
        if (!string.IsNullOrWhiteSpace(normalizedTeam))
        {
            if (int.TryParse(normalizedTeam, out var numericTeam) && numericTeam > 0)
            {
                return numericTeam;
            }

            if (normalizedTeam.Equals("blue", StringComparison.OrdinalIgnoreCase)
                || normalizedTeam.Equals("team1", StringComparison.OrdinalIgnoreCase))
            {
                return 1;
            }

            if (normalizedTeam.Equals("red", StringComparison.OrdinalIgnoreCase)
                || normalizedTeam.Equals("team2", StringComparison.OrdinalIgnoreCase))
            {
                return 2;
            }
        }

        return slotIndex.HasValue ? slotIndex.Value % 2 + 1 : 0;
    }
}
