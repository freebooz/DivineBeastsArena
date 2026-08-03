/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义刷新令牌轮换与登出撤销用例，隐藏哈希算法和数据库事务。
*/

namespace Game.Application.Auth;

public enum RefreshCredentialStatus
{
    Success,
    InvalidOrExpired,
    AccountBanned
}

public sealed record RefreshCredentialResult(
    RefreshCredentialStatus Status,
    LoginCredentialResponse? Credentials = null,
    LoginCredentialSubject? Subject = null);

public interface IRefreshCredentialHasher
{
    string Hash(string refreshToken);
}

public interface IRefreshCredentialRotationStore
{
    Task<RefreshCredentialResult> RotateAsync(
        string refreshTokenHash,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default);
}

public interface ILogoutCredentialStore
{
    Task RevokeAsync(
        Guid accountId,
        string? refreshTokenHash,
        CancellationToken cancellationToken = default);
}

public interface IRotateRefreshCredentialUseCase
{
    Task<RefreshCredentialResult> ExecuteAsync(
        string? refreshToken,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default);
}

public interface ILogoutUseCase
{
    Task ExecuteAsync(
        Guid accountId,
        string? refreshToken,
        CancellationToken cancellationToken = default);
}

public sealed class RotateRefreshCredentialUseCase(
    IRefreshCredentialHasher hasher,
    IRefreshCredentialRotationStore store) : IRotateRefreshCredentialUseCase
{
    public Task<RefreshCredentialResult> ExecuteAsync(
        string? refreshToken,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrWhiteSpace(refreshToken))
        {
            return Task.FromResult(new RefreshCredentialResult(
                RefreshCredentialStatus.InvalidOrExpired));
        }

        return store.RotateAsync(
            hasher.Hash(refreshToken),
            ipAddress,
            userAgent,
            cancellationToken);
    }
}

public sealed class LogoutUseCase(
    IRefreshCredentialHasher hasher,
    ILogoutCredentialStore store) : ILogoutUseCase
{
    public Task ExecuteAsync(
        Guid accountId,
        string? refreshToken,
        CancellationToken cancellationToken = default)
    {
        if (accountId == Guid.Empty)
            return Task.CompletedTask;

        var tokenHash = string.IsNullOrWhiteSpace(refreshToken)
            ? null
            : hasher.Hash(refreshToken);
        return store.RevokeAsync(accountId, tokenHash, cancellationToken);
    }
}
