/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：定义账号凭据认证用例，不依赖 EF Core、BCrypt 或 API 协议。
*/

namespace Game.Application.Auth;

public enum CredentialAuthenticationStatus
{
    Success,
    InvalidCredentials,
    AccountBanned
}

public sealed record StoredCredentialAccount(
    Guid AccountId,
    Guid PlayerId,
    string DisplayName,
    string AccountType,
    string Status,
    string? Email,
    string? PasswordHash);

public sealed record CredentialAuthenticationResult(
    CredentialAuthenticationStatus Status,
    LoginCredentialSubject? Subject = null);

public interface IAuthenticationCredentialStore
{
    Task<StoredCredentialAccount?> FindRegularAccountAsync(
        string loginName,
        CancellationToken cancellationToken = default);

    Task<StoredCredentialAccount?> FindDevelopmentAccountAsync(
        string displayName,
        CancellationToken cancellationToken = default);
}

public interface IPasswordCredentialVerifier
{
    string Hash(string password);
    bool Verify(string password, string passwordHash);
}

public interface IAuthenticateCredentialsUseCase
{
    Task<CredentialAuthenticationResult> AuthenticateRegularAsync(
        string loginName,
        string password,
        CancellationToken cancellationToken = default);

    Task<CredentialAuthenticationResult> AuthenticateDevelopmentAsync(
        string displayName,
        string? password,
        CancellationToken cancellationToken = default);
}

public sealed class AuthenticateCredentialsUseCase(
    IAuthenticationCredentialStore store,
    IPasswordCredentialVerifier passwordVerifier) : IAuthenticateCredentialsUseCase
{
    public async Task<CredentialAuthenticationResult> AuthenticateRegularAsync(
        string loginName,
        string password,
        CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrWhiteSpace(loginName) || string.IsNullOrWhiteSpace(password))
            return Invalid();

        var account = await store.FindRegularAccountAsync(loginName.Trim(), cancellationToken);
        if (account == null || string.IsNullOrEmpty(account.PasswordHash) ||
            !passwordVerifier.Verify(password, account.PasswordHash))
        {
            return Invalid();
        }

        return Complete(account);
    }

    public async Task<CredentialAuthenticationResult> AuthenticateDevelopmentAsync(
        string displayName,
        string? password,
        CancellationToken cancellationToken = default)
    {
        if (string.IsNullOrWhiteSpace(displayName))
            return Invalid();

        var account = await store.FindDevelopmentAccountAsync(displayName.Trim(), cancellationToken);
        if (account == null)
            return Invalid();
        if (IsBanned(account))
            return Banned();
        if (!string.IsNullOrEmpty(password) && !string.IsNullOrEmpty(account.PasswordHash) &&
            !passwordVerifier.Verify(password, account.PasswordHash))
        {
            return Invalid();
        }

        return Success(account);
    }

    private static CredentialAuthenticationResult Complete(StoredCredentialAccount account)
    {
        return IsBanned(account) ? Banned() : Success(account);
    }

    private static bool IsBanned(StoredCredentialAccount account) =>
        string.Equals(account.Status, "BANNED", StringComparison.OrdinalIgnoreCase);

    private static CredentialAuthenticationResult Success(StoredCredentialAccount account) =>
        new(
            CredentialAuthenticationStatus.Success,
            new LoginCredentialSubject(
                account.AccountId,
                account.PlayerId,
                account.DisplayName,
                account.AccountType));

    private static CredentialAuthenticationResult Invalid() =>
        new(CredentialAuthenticationStatus.InvalidCredentials);

    private static CredentialAuthenticationResult Banned() =>
        new(CredentialAuthenticationStatus.AccountBanned);
}
