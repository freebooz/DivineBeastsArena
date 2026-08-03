/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义当前登录账号身份查询用例，隔离 API 与持久化实现。
*/

using Game.Shared.Contracts.Auth;

namespace Game.Application.Auth;

public interface IAuthenticatedAccountQueryStore
{
    Task<MeResponse?> FindByAccountIdAsync(
        Guid accountId,
        CancellationToken cancellationToken = default);
}

public interface IGetAuthenticatedAccountUseCase
{
    Task<MeResponse?> ExecuteAsync(
        Guid accountId,
        CancellationToken cancellationToken = default);
}

public sealed class GetAuthenticatedAccountUseCase(IAuthenticatedAccountQueryStore store)
    : IGetAuthenticatedAccountUseCase
{
    public Task<MeResponse?> ExecuteAsync(
        Guid accountId,
        CancellationToken cancellationToken = default)
    {
        return accountId == Guid.Empty
            ? Task.FromResult<MeResponse?>(null)
            : store.FindByAccountIdAsync(accountId, cancellationToken);
    }
}
