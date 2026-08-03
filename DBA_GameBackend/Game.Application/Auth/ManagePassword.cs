/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：实现配置驱动的密码修改与重置用例，不依赖 EF Core 或 BCrypt。
*/

using Game.Shared.Contracts.Auth;
using Game.Shared.Options;

namespace Game.Application.Auth;

public sealed record PasswordAccountSnapshot(
    Guid AccountId,
    string Status,
    string? PasswordHash);

public interface IPasswordAccountStore
{
    Task<PasswordAccountSnapshot?> FindByAccountIdAsync(
        Guid accountId,
        CancellationToken cancellationToken = default);

    Task<PasswordAccountSnapshot?> FindByEmailAsync(
        string email,
        CancellationToken cancellationToken = default);

    Task UpdatePasswordAndRevokeSessionsAsync(
        Guid accountId,
        string newPasswordHash,
        CancellationToken cancellationToken = default);
}

public interface ISecureCredentialComparer
{
    bool Equals(string? suppliedValue, string configuredValue);
}

public interface IChangePasswordUseCase
{
    Task<PasswordChangeResponse> ExecuteAsync(
        Guid accountId,
        string oldPassword,
        string newPassword,
        CancellationToken cancellationToken = default);
}

public interface IResetPasswordUseCase
{
    Task<PasswordChangeResponse> ExecuteAsync(
        string email,
        string? resetToken,
        string? newPassword,
        CancellationToken cancellationToken = default);
}

public sealed class ChangePasswordUseCase(
    IPasswordAccountStore store,
    IPasswordCredentialVerifier passwordCredential,
    AuthenticationPolicyOptions options) : IChangePasswordUseCase
{
    public async Task<PasswordChangeResponse> ExecuteAsync(
        Guid accountId,
        string oldPassword,
        string newPassword,
        CancellationToken cancellationToken = default)
    {
        var messages = options.Messages;
        if (string.IsNullOrWhiteSpace(oldPassword) || string.IsNullOrWhiteSpace(newPassword))
            return Failure(messages.PasswordFieldsRequired);
        if (newPassword.Length < options.MinimumPasswordLength)
            return Failure(messages.PasswordTooShort);

        var account = await store.FindByAccountIdAsync(accountId, cancellationToken);
        if (account == null)
            return Failure(messages.AccountNotFound);
        if (IsBanned(account.Status))
            return Failure(messages.AccountBanned);
        if (string.IsNullOrEmpty(account.PasswordHash))
            return Failure(messages.PasswordCredentialMissing);
        if (!passwordCredential.Verify(oldPassword, account.PasswordHash))
            return Failure(messages.OldPasswordIncorrect);
        if (passwordCredential.Verify(newPassword, account.PasswordHash))
            return Failure(messages.PasswordUnchanged);

        await store.UpdatePasswordAndRevokeSessionsAsync(
            account.AccountId,
            passwordCredential.Hash(newPassword),
            cancellationToken);
        return Success(messages.PasswordChanged);
    }

    private static bool IsBanned(string status) =>
        string.Equals(status, "BANNED", StringComparison.OrdinalIgnoreCase);

    private static PasswordChangeResponse Success(string message) => new(true, message);
    private static PasswordChangeResponse Failure(string message) => new(false, message);
}

public sealed class ResetPasswordUseCase(
    IPasswordAccountStore store,
    IPasswordCredentialVerifier passwordCredential,
    ISecureCredentialComparer secureComparer,
    AuthenticationPolicyOptions options) : IResetPasswordUseCase
{
    public async Task<PasswordChangeResponse> ExecuteAsync(
        string email,
        string? resetToken,
        string? newPassword,
        CancellationToken cancellationToken = default)
    {
        var messages = options.Messages;
        if (string.IsNullOrWhiteSpace(email) || string.IsNullOrWhiteSpace(newPassword))
            return Failure(messages.EmailAndPasswordRequired);
        if (newPassword.Length < options.MinimumPasswordLength)
            return Failure(messages.PasswordTooShort);
        if (string.IsNullOrWhiteSpace(options.PasswordResetBootstrapToken))
            return Failure(messages.ResetServiceUnavailable);
        if (!secureComparer.Equals(resetToken, options.PasswordResetBootstrapToken))
            return Failure(messages.ResetTokenInvalid);

        var account = await store.FindByEmailAsync(email.Trim(), cancellationToken);
        if (account == null)
            return Failure(messages.AccountNotFound);
        if (string.Equals(account.Status, "BANNED", StringComparison.OrdinalIgnoreCase))
            return Failure(messages.AccountBanned);

        await store.UpdatePasswordAndRevokeSessionsAsync(
            account.AccountId,
            passwordCredential.Hash(newPassword),
            cancellationToken);
        return Success(messages.PasswordResetCompleted);
    }

    private static PasswordChangeResponse Success(string message) => new(true, message);
    private static PasswordChangeResponse Failure(string message) => new(false, message);
}
