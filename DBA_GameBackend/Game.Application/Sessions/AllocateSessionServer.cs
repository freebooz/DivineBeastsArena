/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：校验服务器地址分配命令，并通过端口更新已绑定的 Dedicated Server 与会话。
- 安全约束：Runtime Token 的 Hash 与有效期校验由基础设施实现，明文不得记录。
*/

namespace Game.Application.Sessions;

public sealed record AllocateSessionServerCommand(
    Guid SessionId,
    string Ip,
    int Port,
    string RuntimeToken,
    DateTimeOffset OccurredAt,
    IReadOnlySet<string> AllowedCurrentStatuses);

public interface ISessionServerAllocationStore
{
    Task<SessionLifecycleState?> TryAllocateAsync(
        AllocateSessionServerCommand command,
        CancellationToken cancellationToken = default);
}

public interface IAllocateSessionServerUseCase
{
    Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        string? ip,
        int port,
        string? runtimeToken,
        CancellationToken cancellationToken = default);
}

public sealed class AllocateSessionServerUseCase(
    ISessionServerAllocationStore store,
    TimeProvider timeProvider) : IAllocateSessionServerUseCase
{
    private static readonly IReadOnlySet<string> AllocatableStatuses =
        new HashSet<string>(StringComparer.Ordinal)
        {
            "CREATED",
            "ALLOCATING_SERVER",
            "WAITING_PLAYERS"
        };

    public Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        string? ip,
        int port,
        string? runtimeToken,
        CancellationToken cancellationToken = default)
    {
        if (sessionId == Guid.Empty
            || string.IsNullOrWhiteSpace(ip)
            || port is < 1 or > 65535
            || string.IsNullOrWhiteSpace(runtimeToken))
        {
            return Task.FromResult<SessionLifecycleState?>(null);
        }

        return store.TryAllocateAsync(
            new AllocateSessionServerCommand(
                sessionId,
                ip.Trim(),
                port,
                runtimeToken,
                timeProvider.GetUtcNow(),
                AllocatableStatuses),
            cancellationToken);
    }
}
