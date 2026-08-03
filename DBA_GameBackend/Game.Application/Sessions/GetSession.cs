/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义会话只读查询用例，避免 API 直接访问 EF Core。
*/

namespace Game.Application.Sessions;

public interface ISessionQueryStore
{
    Task<SessionLifecycleState?> FindByIdAsync(
        Guid sessionId,
        CancellationToken cancellationToken = default);
}

public interface IGetSessionUseCase
{
    Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        CancellationToken cancellationToken = default);
}

public sealed class GetSessionUseCase(ISessionQueryStore store) : IGetSessionUseCase
{
    public Task<SessionLifecycleState?> ExecuteAsync(
        Guid sessionId,
        CancellationToken cancellationToken = default)
    {
        return sessionId == Guid.Empty
            ? Task.FromResult<SessionLifecycleState?>(null)
            : store.FindByIdAsync(sessionId, cancellationToken);
    }
}
