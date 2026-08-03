/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：编排登录令牌签发和凭据持久化，隔离 JWT、哈希算法与 EF Core。
*/

namespace Game.Application.Auth;

public sealed record LoginCredentialSubject(
    Guid AccountId,
    Guid PlayerId,
    string DisplayName,
    string AccountType);

public sealed record IssuedLoginCredentials(
    string AccessToken,
    string RefreshToken,
    string RefreshTokenHash,
    DateTimeOffset AccessTokenExpiresAt,
    DateTimeOffset RefreshTokenExpiresAt);

public sealed record LoginCredentialResponse(
    string AccessToken,
    string RefreshToken,
    DateTimeOffset AccessTokenExpiresAt);

public interface ILoginCredentialIssuer
{
    IssuedLoginCredentials Issue(LoginCredentialSubject subject);
}

public interface ILoginCredentialStore
{
    Task SaveAsync(
        LoginCredentialSubject subject,
        string refreshTokenHash,
        DateTimeOffset refreshTokenExpiresAt,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default);
}

public interface IIssueLoginCredentialsUseCase
{
    Task<LoginCredentialResponse> ExecuteAsync(
        LoginCredentialSubject subject,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default);
}

public sealed class IssueLoginCredentialsUseCase(
    ILoginCredentialIssuer issuer,
    ILoginCredentialStore store) : IIssueLoginCredentialsUseCase
{
    public async Task<LoginCredentialResponse> ExecuteAsync(
        LoginCredentialSubject subject,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default)
    {
        if (subject.AccountId == Guid.Empty || subject.PlayerId == Guid.Empty ||
            string.IsNullOrWhiteSpace(subject.DisplayName) || string.IsNullOrWhiteSpace(subject.AccountType))
        {
            throw new InvalidOperationException("登录令牌签发缺少有效的账号身份信息。");
        }

        var credentials = issuer.Issue(subject);
        await store.SaveAsync(
            subject,
            credentials.RefreshTokenHash,
            credentials.RefreshTokenExpiresAt,
            ipAddress,
            userAgent,
            cancellationToken);

        return new LoginCredentialResponse(
            credentials.AccessToken,
            credentials.RefreshToken,
            credentials.AccessTokenExpiresAt);
    }
}
