/*
中文阅读说明：
- 所属应用：DBA_GameBackend 应用层。
- 文件职责：编排访客设备登录与正式账号注册，所有初始数据由认证策略配置驱动。
*/

using Game.Shared.Options;

namespace Game.Application.Auth;

public enum AccountOnboardingStatus
{
    Success,
    InvalidInput,
    DuplicateIdentity,
    AccountBanned
}

public sealed record AccountOnboardingResult(
    AccountOnboardingStatus Status,
    LoginCredentialSubject? Subject = null,
    string? Message = null);

public sealed record GuestAccountDraft(
    Guid AccountId,
    Guid IdentityId,
    Guid PlayerId,
    Guid DeviceLoginId,
    string DeviceHash,
    string? DeviceName,
    string? Platform,
    string DisplayName,
    string AccountType,
    string AccountStatus,
    int InitialLevel,
    long InitialExperience,
    string InitialSettingsJson,
    DateTimeOffset CreatedAt);

public sealed record RegisteredAccountDraft(
    Guid AccountId,
    Guid IdentityId,
    Guid PlayerId,
    string DisplayName,
    string? Email,
    string PasswordHash,
    string AccountType,
    string AccountStatus,
    int InitialLevel,
    long InitialExperience,
    string InitialSettingsJson,
    DateTimeOffset CreatedAt);

public interface IDeviceIdentifierHasher
{
    string Hash(string deviceIdentifier);
}

public interface IAccountOnboardingStore
{
    Task<AccountOnboardingResult?> ResolveGuestAsync(
        string deviceHash,
        string? deviceName,
        string? platform,
        CancellationToken cancellationToken = default);

    Task<AccountOnboardingResult> CreateGuestAsync(
        GuestAccountDraft draft,
        CancellationToken cancellationToken = default);

    Task<AccountOnboardingResult> RegisterAsync(
        RegisteredAccountDraft draft,
        CancellationToken cancellationToken = default);
}

public interface IGuestLoginUseCase
{
    Task<AccountOnboardingResult> ExecuteAsync(
        string? deviceId,
        string? deviceName,
        string? platform,
        CancellationToken cancellationToken = default);
}

public interface IRegisterAccountUseCase
{
    Task<AccountOnboardingResult> ExecuteAsync(
        string? username,
        string? email,
        string? password,
        CancellationToken cancellationToken = default);
}

public sealed class GuestLoginUseCase(
    IDeviceIdentifierHasher deviceHasher,
    IAccountOnboardingStore store,
    AuthenticationPolicyOptions options) : IGuestLoginUseCase
{
    public async Task<AccountOnboardingResult> ExecuteAsync(
        string? deviceId,
        string? deviceName,
        string? platform,
        CancellationToken cancellationToken = default)
    {
        var resolvedDeviceId = string.IsNullOrWhiteSpace(deviceId)
            ? Guid.NewGuid().ToString("N")
            : deviceId.Trim();
        var deviceHash = deviceHasher.Hash(resolvedDeviceId);
        var normalizedDeviceName = Normalize(deviceName);
        var normalizedPlatform = Normalize(platform);
        var existing = await store.ResolveGuestAsync(
            deviceHash,
            normalizedDeviceName,
            normalizedPlatform,
            cancellationToken);
        if (existing != null)
            return existing;

        var now = DateTimeOffset.UtcNow;
        var playerId = Guid.NewGuid();
        var suffix = Guid.NewGuid().ToString("N")[..options.GuestDisplayNameSuffixLength];
        var draft = new GuestAccountDraft(
            Guid.NewGuid(),
            Guid.NewGuid(),
            playerId,
            Guid.NewGuid(),
            deviceHash,
            normalizedDeviceName,
            normalizedPlatform,
            $"{options.GuestDisplayNamePrefix}{suffix}",
            options.GuestAccountType,
            options.ActiveAccountStatus,
            options.InitialPlayerLevel,
            options.InitialPlayerExperience,
            options.InitialPlayerSettingsJson,
            now);
        var created = await store.CreateGuestAsync(draft, cancellationToken);
        return created.Status == AccountOnboardingStatus.DuplicateIdentity && created.Message == null
            ? created with { Message = options.Messages.IdentityConflict }
            : created;
    }

    private static string? Normalize(string? value) =>
        string.IsNullOrWhiteSpace(value) ? null : value.Trim();
}

public sealed class RegisterAccountUseCase(
    IPasswordCredentialVerifier passwordCredential,
    IAccountOnboardingStore store,
    AuthenticationPolicyOptions options) : IRegisterAccountUseCase
{
    public async Task<AccountOnboardingResult> ExecuteAsync(
        string? username,
        string? email,
        string? password,
        CancellationToken cancellationToken = default)
    {
        var displayName = FirstNonEmpty(username, BuildUsernameFromEmail(email));
        if (string.IsNullOrWhiteSpace(displayName))
            return Invalid(options.Messages.UsernameOrEmailRequired);
        if (string.IsNullOrWhiteSpace(password) || password.Length < options.MinimumPasswordLength)
            return Invalid(options.Messages.PasswordTooShort);

        var now = DateTimeOffset.UtcNow;
        var draft = new RegisteredAccountDraft(
            Guid.NewGuid(),
            Guid.NewGuid(),
            Guid.NewGuid(),
            displayName,
            Normalize(email),
            passwordCredential.Hash(password),
            options.RegisteredAccountType,
            options.ActiveAccountStatus,
            options.InitialPlayerLevel,
            options.InitialPlayerExperience,
            options.InitialPlayerSettingsJson,
            now);
        var registered = await store.RegisterAsync(draft, cancellationToken);
        return registered.Status == AccountOnboardingStatus.DuplicateIdentity && registered.Message == null
            ? registered with { Message = options.Messages.UsernameTaken }
            : registered;
    }

    private static AccountOnboardingResult Invalid(string message) =>
        new(AccountOnboardingStatus.InvalidInput, Message: message);

    private static string? FirstNonEmpty(params string?[] values)
    {
        foreach (var value in values)
        {
            if (!string.IsNullOrWhiteSpace(value))
                return value.Trim();
        }
        return null;
    }

    private static string? BuildUsernameFromEmail(string? email)
    {
        var normalized = Normalize(email);
        if (normalized == null)
            return null;
        var separator = normalized.IndexOf('@');
        return separator > 0 ? normalized[..separator] : normalized;
    }

    private static string? Normalize(string? value) =>
        string.IsNullOrWhiteSpace(value) ? null : value.Trim();
}
