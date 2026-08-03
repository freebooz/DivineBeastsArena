/*
中文阅读说明：
- 所属应用：DBA_GameBackend 共享配置层。
- 文件职责：承载认证密码规则和中文响应文案，禁止在认证服务中硬编码业务配置。
*/

namespace Game.Shared.Options;

public sealed class AuthenticationPolicyOptions
{
    public const string Section = "AuthenticationPolicy";

    public int MinimumPasswordLength { get; init; }
    public string ActiveAccountStatus { get; init; } = string.Empty;
    public string GuestAccountType { get; init; } = string.Empty;
    public string RegisteredAccountType { get; init; } = string.Empty;
    public string GuestDisplayNamePrefix { get; init; } = string.Empty;
    public int GuestDisplayNameSuffixLength { get; init; }
    public int InitialPlayerLevel { get; init; }
    public long InitialPlayerExperience { get; init; }
    public string InitialPlayerSettingsJson { get; init; } = string.Empty;
    public string? PasswordResetBootstrapToken { get; set; }
    public AuthenticationPolicyMessages Messages { get; init; } = new();
}

public sealed class AuthenticationPolicyMessages
{
    public string PasswordFieldsRequired { get; init; } = string.Empty;
    public string EmailAndPasswordRequired { get; init; } = string.Empty;
    public string PasswordTooShort { get; init; } = string.Empty;
    public string AccountNotFound { get; init; } = string.Empty;
    public string AccountBanned { get; init; } = string.Empty;
    public string PasswordCredentialMissing { get; init; } = string.Empty;
    public string OldPasswordIncorrect { get; init; } = string.Empty;
    public string PasswordUnchanged { get; init; } = string.Empty;
    public string ResetServiceUnavailable { get; init; } = string.Empty;
    public string ResetTokenInvalid { get; init; } = string.Empty;
    public string PasswordChanged { get; init; } = string.Empty;
    public string PasswordResetCompleted { get; init; } = string.Empty;
    public string UsernameOrEmailRequired { get; init; } = string.Empty;
    public string UsernameTaken { get; init; } = string.Empty;
    public string IdentityConflict { get; init; } = string.Empty;
    public string InvalidOnboarding { get; init; } = string.Empty;
    public string InvalidCredentials { get; init; } = string.Empty;
    public string DevelopmentLoginDisabled { get; init; } = string.Empty;
    public string RefreshTokenInvalid { get; init; } = string.Empty;
    public string SteamLoginUnavailable { get; init; } = string.Empty;
    public string EosLoginUnavailable { get; init; } = string.Empty;
    public string WeChatLoginUnavailable { get; init; } = string.Empty;
}
