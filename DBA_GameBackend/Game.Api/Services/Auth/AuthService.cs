/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Application.Auth;
using Game.Shared.Contracts.Auth;
using Game.Shared.Errors;
using Game.Shared.Options;

namespace Game.Api.Services.Auth;

public sealed class AuthService : IAuthService
{
    private readonly IGuestLoginUseCase _guestLogin;
    private readonly IRegisterAccountUseCase _registerAccount;
    private readonly IGetAuthenticatedAccountUseCase _getAuthenticatedAccount;
    private readonly IAuthenticateCredentialsUseCase _authenticateCredentials;
    private readonly IIssueLoginCredentialsUseCase _issueLoginCredentials;
    private readonly IRotateRefreshCredentialUseCase _rotateRefreshCredential;
    private readonly ILogoutUseCase _logout;
    private readonly bool _isProduction;
    private readonly IChangePasswordUseCase _changePassword;
    private readonly IResetPasswordUseCase _resetPassword;
    private readonly AuthenticationPolicyOptions _authenticationPolicy;

    public AuthService(
        IGuestLoginUseCase guestLogin,
        IRegisterAccountUseCase registerAccount,
        IGetAuthenticatedAccountUseCase getAuthenticatedAccount,
        IAuthenticateCredentialsUseCase authenticateCredentials,
        IIssueLoginCredentialsUseCase issueLoginCredentials,
        IRotateRefreshCredentialUseCase rotateRefreshCredential,
        ILogoutUseCase logout,
        IChangePasswordUseCase changePassword,
        IResetPasswordUseCase resetPassword,
        AuthenticationPolicyOptions authenticationPolicy,
        IConfiguration configuration)
    {
        _guestLogin = guestLogin;
        _registerAccount = registerAccount;
        _getAuthenticatedAccount = getAuthenticatedAccount;
        _authenticateCredentials = authenticateCredentials;
        _issueLoginCredentials = issueLoginCredentials;
        _rotateRefreshCredential = rotateRefreshCredential;
        _logout = logout;
        _changePassword = changePassword;
        _resetPassword = resetPassword;
        _authenticationPolicy = authenticationPolicy;
        _isProduction = configuration["ASPNETCORE_ENVIRONMENT"] == "Production";
    }

    public async Task<AuthServiceResult> GuestLoginAsync(GuestLoginRequest request, string? ip, string? userAgent)
    {
        var onboarding = await _guestLogin.ExecuteAsync(
            request.DeviceId,
            request.DeviceName,
            request.Platform);
        return await CompleteOnboardingAsync(
            onboarding,
            ErrorCodes.ValidationError,
            ip,
            userAgent);
    }

    public async Task<AuthServiceResult> DevLoginAsync(DevLoginRequest request, string? ip, string? userAgent)
    {
        if (_isProduction)
            return Fail(ErrorCodes.AuthDevLoginDisabled, _authenticationPolicy.Messages.DevelopmentLoginDisabled);

        var username = FirstNonEmpty(request.Username, request.DisplayName);
        if (string.IsNullOrWhiteSpace(username))
            return Fail(ErrorCodes.AuthInvalidCredentials, _authenticationPolicy.Messages.InvalidCredentials);

        var authentication = await _authenticateCredentials.AuthenticateDevelopmentAsync(
            username,
            request.Password);
        return await CompleteCredentialLoginAsync(authentication, ip, userAgent);
    }

    public async Task<AuthServiceResult> AccountLoginAsync(AccountLoginRequest request, string? ip, string? userAgent)
    {
        var loginName = FirstNonEmpty(request.Username, request.Email);
        if (string.IsNullOrWhiteSpace(loginName) || string.IsNullOrWhiteSpace(request.Password))
            return Fail(ErrorCodes.AuthInvalidCredentials, _authenticationPolicy.Messages.InvalidCredentials);

        var authentication = await _authenticateCredentials.AuthenticateRegularAsync(
            loginName,
            request.Password);
        return await CompleteCredentialLoginAsync(authentication, ip, userAgent);
    }

    public async Task<AuthServiceResult> AccountRegisterAsync(AccountRegisterRequest request, string? ip, string? userAgent)
    {
        var onboarding = await _registerAccount.ExecuteAsync(
            request.Username,
            request.Email,
            request.Password);
        return await CompleteOnboardingAsync(
            onboarding,
            ErrorCodes.PlayerNicknameTaken,
            ip,
            userAgent);
    }

    public async Task<AuthServiceResult> RefreshTokenAsync(string? refreshToken, string? ip)
    {
        var rotation = await _rotateRefreshCredential.ExecuteAsync(refreshToken, ip, null);
        if (rotation.Status == RefreshCredentialStatus.AccountDisabled)
            return Fail(ErrorCodes.AuthAccountDisabled, _authenticationPolicy.Messages.AccountBanned);
        if (rotation.Status == RefreshCredentialStatus.Reused)
            return Fail(ErrorCodes.AuthRefreshTokenReused, _authenticationPolicy.Messages.RefreshTokenInvalid);
        if (rotation.Status != RefreshCredentialStatus.Success ||
            rotation.Credentials == null ||
            rotation.Subject == null)
            return Fail(ErrorCodes.AuthRefreshTokenExpired, _authenticationPolicy.Messages.RefreshTokenInvalid);

        var credentials = rotation.Credentials;
        var subject = rotation.Subject;
        return new AuthServiceResult(
            true,
            credentials.AccessToken,
            credentials.RefreshToken,
            subject.AccountId,
            subject.PlayerId,
            subject.DisplayName);
    }

    public async Task LogoutAsync(Guid accountId, string? refreshToken)
    {
        await _logout.ExecuteAsync(accountId, refreshToken);
    }

    public async Task<MeResponse?> GetMeAsync(Guid accountId)
    {
        return await _getAuthenticatedAccount.ExecuteAsync(accountId);
    }

    public Task<AuthServiceResult> SteamExternalLoginAsync(string steamTicket, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with Steam API
        return Task.FromResult(Fail(ErrorCodes.AuthSteamMockOnly, _authenticationPolicy.Messages.SteamLoginUnavailable));
    }

    public Task<AuthServiceResult> EosExternalLoginAsync(string eosTicket, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with EOS API
        return Task.FromResult(Fail(ErrorCodes.AuthEosMockOnly, _authenticationPolicy.Messages.EosLoginUnavailable));
    }

    public Task<AuthServiceResult> WeChatExternalLoginAsync(string code, string? nickname, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with WeChat API
        // TODO: Replace with actual WeChat API validation:
        // 1. Call WeChat API with code to get openid
        // 2. Check if account exists for this openid
        // 3. If not, create new account
        // 4. Return login result with access/refresh tokens
        return Task.FromResult(Fail(ErrorCodes.AuthWeChatMockOnly, _authenticationPolicy.Messages.WeChatLoginUnavailable));
    }

    private async Task<AuthServiceResult> GenerateLoginResult(
        LoginCredentialSubject subject,
        string? ipAddress,
        string? userAgent)
    {
        var credentials = await _issueLoginCredentials.ExecuteAsync(
            subject,
            ipAddress,
            userAgent);

        return new AuthServiceResult(
            true,
            credentials.AccessToken,
            credentials.RefreshToken,
            subject.AccountId,
            subject.PlayerId,
            subject.DisplayName);
    }

    private async Task<AuthServiceResult> CompleteCredentialLoginAsync(
        CredentialAuthenticationResult authentication,
        string? ipAddress,
        string? userAgent)
    {
        if (authentication.Status == CredentialAuthenticationStatus.AccountDisabled)
            return Fail(ErrorCodes.AuthAccountDisabled, _authenticationPolicy.Messages.AccountBanned);
        if (authentication.Status != CredentialAuthenticationStatus.Success || authentication.Subject == null)
            return Fail(ErrorCodes.AuthInvalidCredentials, _authenticationPolicy.Messages.InvalidCredentials);

        return await GenerateLoginResult(authentication.Subject, ipAddress, userAgent);
    }

    private async Task<AuthServiceResult> CompleteOnboardingAsync(
        AccountOnboardingResult onboarding,
        string duplicateErrorCode,
        string? ipAddress,
        string? userAgent)
    {
        if (onboarding.Status == AccountOnboardingStatus.AccountBanned)
            return Fail(ErrorCodes.AuthAccountBanned, _authenticationPolicy.Messages.AccountBanned);
        if (onboarding.Status == AccountOnboardingStatus.DuplicateIdentity)
            return Fail(duplicateErrorCode, onboarding.Message ?? _authenticationPolicy.Messages.IdentityConflict);
        if (onboarding.Status != AccountOnboardingStatus.Success || onboarding.Subject == null)
            return Fail(ErrorCodes.ValidationError, onboarding.Message ?? _authenticationPolicy.Messages.InvalidOnboarding);

        return await GenerateLoginResult(onboarding.Subject, ipAddress, userAgent);
    }

    public async Task<PasswordChangeResponse> ChangePasswordAsync(Guid accountId, string oldPassword, string newPassword)
    {
        return await _changePassword.ExecuteAsync(accountId, oldPassword, newPassword);
    }

    public async Task<PasswordChangeResponse> ResetPasswordAsync(string email, string? token, string? newPassword)
    {
        return await _resetPassword.ExecuteAsync(email, token, newPassword);
    }

    private static AuthServiceResult Fail(string errorCode, string message) =>
        new(false, null, null, null, null, null, errorCode, message);

    private static string? FirstNonEmpty(params string?[] values)
    {
        foreach (var value in values)
        {
            if (!string.IsNullOrWhiteSpace(value))
                return value.Trim();
        }

        return null;
    }

}
