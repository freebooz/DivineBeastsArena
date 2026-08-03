/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：统一会话开始、完成和失败的状态转换规则。
- 架构约束：状态机位于应用层；EF 更新与事件落库通过 ISessionLifecycleStore 端口完成。
*/

namespace Game.Application.Sessions;

public enum SessionLifecycleTransition
{
    Start,
    Complete,
    Fail
}

public sealed record SessionLifecycleState(
    Guid SessionId,
    string SourceType,
    string Mode,
    string MapId,
    string Region,
    string Status,
    string? ServerIp,
    int? ServerPort,
    int MaxPlayers,
    DateTimeOffset CreatedAt,
    DateTimeOffset? StartedAt);

public sealed record SessionLifecycleMutation(
    Guid SessionId,
    IReadOnlySet<string>? AllowedCurrentStatuses,
    string TargetStatus,
    string EventType,
    string? FailureReason,
    DateTimeOffset OccurredAt,
    bool SetStartedAt,
    bool SetEndedAt);

public interface ISessionLifecycleStore
{
    Task<SessionLifecycleState?> TryApplyAsync(
        SessionLifecycleMutation mutation,
        CancellationToken cancellationToken = default);
}

public interface IChangeSessionLifecycleUseCase
{
    Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        SessionLifecycleTransition transition,
        string? failureReason = null,
        CancellationToken cancellationToken = default);
}

public sealed class ChangeSessionLifecycleUseCase(
    ISessionLifecycleStore store,
    TimeProvider timeProvider) : IChangeSessionLifecycleUseCase
{
    private static readonly IReadOnlySet<string> StartableStatuses =
        new HashSet<string>(StringComparer.Ordinal) { "WAITING_PLAYERS" };

    private static readonly IReadOnlySet<string> CompletableStatuses =
        new HashSet<string>(StringComparer.Ordinal) { "IN_PROGRESS", "SETTLING" };

    public Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        SessionLifecycleTransition transition,
        string? failureReason = null,
        CancellationToken cancellationToken = default)
    {
        var mutation = transition switch
        {
            SessionLifecycleTransition.Start => new SessionLifecycleMutation(
                sessionId,
                StartableStatuses,
                "IN_PROGRESS",
                "STARTED",
                null,
                timeProvider.GetUtcNow(),
                SetStartedAt: true,
                SetEndedAt: false),
            SessionLifecycleTransition.Complete => new SessionLifecycleMutation(
                sessionId,
                CompletableStatuses,
                "COMPLETED",
                "COMPLETED",
                null,
                timeProvider.GetUtcNow(),
                SetStartedAt: false,
                SetEndedAt: true),
            SessionLifecycleTransition.Fail => new SessionLifecycleMutation(
                sessionId,
                null,
                "FAILED",
                "FAILED",
                failureReason,
                timeProvider.GetUtcNow(),
                SetStartedAt: false,
                SetEndedAt: true),
            _ => throw new ArgumentOutOfRangeException(nameof(transition), transition, "不支持的会话生命周期转换。")
        };

        return store.TryApplyAsync(mutation, cancellationToken);
    }
}
