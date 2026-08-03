/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义并执行一次性入服票据消费用例，不依赖 EF、HTTP 或 Dedicated Server 实现。
- 安全约束：票据必须同时绑定账号、角色、会话、服务器实例和构建版本；明文票据不得进入日志。
*/

using Game.Shared.Contracts.Character;
using Game.Application.Characters;

namespace Game.Application.Sessions;

public sealed record ConsumeJoinTicketCommand(
    Guid AccountId,
    Guid CharacterId,
    Guid SessionId,
    Guid ServerInstanceId,
    string BuildId,
    string JoinTicket,
    string Zodiac,
    string PrimaryElement,
    string? FiveCamp,
    string FixedSkillGroupId);

public sealed record ConsumedJoinTicket(
    Guid AccountId,
    Guid CharacterId,
    Guid SessionId,
    Guid ServerInstanceId,
    string BuildId,
    string? Team,
    string Zodiac,
    string PrimaryElement,
    string? FiveCamp,
    string FixedSkillGroupId);

public interface IJoinTicketStore
{
    Task<ConsumedJoinTicket?> TryConsumeAsync(
        ConsumeJoinTicketCommand command,
        CancellationToken cancellationToken = default);
}

public interface IConsumeJoinTicketUseCase
{
    Task<ConsumedJoinTicket?> ExecuteAsync(
        ConsumeJoinTicketCommand command,
        CancellationToken cancellationToken = default);
}

public sealed class ConsumeJoinTicketUseCase(
    IJoinTicketStore store,
    ICharacterBuildPolicy characterBuildPolicy) : IConsumeJoinTicketUseCase
{
    public Task<ConsumedJoinTicket?> ExecuteAsync(
        ConsumeJoinTicketCommand command,
        CancellationToken cancellationToken = default)
    {
        if (command.AccountId == Guid.Empty
            || command.CharacterId == Guid.Empty
            || command.SessionId == Guid.Empty
            || command.ServerInstanceId == Guid.Empty
            || string.IsNullOrWhiteSpace(command.BuildId)
            || string.IsNullOrWhiteSpace(command.JoinTicket)
            || string.IsNullOrWhiteSpace(command.Zodiac)
            || string.IsNullOrWhiteSpace(command.PrimaryElement)
            || string.IsNullOrWhiteSpace(command.FixedSkillGroupId))
        {
            return Task.FromResult<ConsumedJoinTicket?>(null);
        }

        var normalizedBuild = characterBuildPolicy.NormalizeBuild(new CharacterBuildSnapshot(
            command.Zodiac,
            command.PrimaryElement,
            command.FiveCamp,
            command.FixedSkillGroupId));
        if (!normalizedBuild.IsValid || !normalizedBuild.HasBuild)
        {
            return Task.FromResult<ConsumedJoinTicket?>(null);
        }

        var build = normalizedBuild.Build!;
        var normalizedCommand = command with
        {
            BuildId = command.BuildId.Trim(),
            Zodiac = build.Zodiac,
            PrimaryElement = build.PrimaryElement,
            FiveCamp = build.FiveCamp,
            FixedSkillGroupId = build.FixedSkillGroupId
        };
        return store.TryConsumeAsync(normalizedCommand, cancellationToken);
    }
}
