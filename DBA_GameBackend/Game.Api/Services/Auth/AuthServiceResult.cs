/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Api.Services.Auth;

public sealed class AuthServiceResult
{
    public bool Success { get; }
    public string? AccessToken { get; }
    public string? RefreshToken { get; }
    public Guid? AccountId { get; }
    public Guid? PlayerId { get; }
    public string? Nickname { get; }
    public string? ErrorCode { get; }
    public string? ErrorMessage { get; }

    public AuthServiceResult(bool success, string? accessToken = null, string? refreshToken = null,
        Guid? accountId = null, Guid? playerId = null, string? nickname = null, string? errorCode = null, string? errorMessage = null)
    {
        Success = success;
        AccessToken = accessToken;
        RefreshToken = refreshToken;
        AccountId = accountId;
        PlayerId = playerId;
        Nickname = nickname;
        ErrorCode = errorCode;
        ErrorMessage = errorMessage;
    }

    public static AuthServiceResult Fail(string errorCode, string message) =>
        new(false, null, null, null, null, null, errorCode, message);
}

public interface IAuthService
{
    Task<AuthServiceResult> GuestLoginAsync(Shared.Contracts.Auth.GuestLoginRequest request, string? ip, string? userAgent);
    Task<AuthServiceResult> DevLoginAsync(Shared.Contracts.Auth.DevLoginRequest request, string? ip, string? userAgent);
    Task<AuthServiceResult> AccountLoginAsync(Shared.Contracts.Auth.AccountLoginRequest request, string? ip, string? userAgent);
    Task<AuthServiceResult> AccountRegisterAsync(Shared.Contracts.Auth.AccountRegisterRequest request, string? ip, string? userAgent);
    Task<AuthServiceResult> RefreshTokenAsync(string refreshToken, string? ip);
    Task LogoutAsync(Guid accountId, string? refreshToken);
    Task<Shared.Contracts.Auth.MeResponse?> GetMeAsync(Guid accountId);
    Task<AuthServiceResult> SteamExternalLoginAsync(string steamTicket, string? ip, string? userAgent);
    Task<AuthServiceResult> EosExternalLoginAsync(string eosTicket, string? ip, string? userAgent);
    Task<AuthServiceResult> WeChatExternalLoginAsync(string code, string? nickname, string? ip, string? userAgent);
    Task<Shared.Contracts.Auth.PasswordChangeResponse> ChangePasswordAsync(Guid accountId, string oldPassword, string newPassword);
    Task<Shared.Contracts.Auth.PasswordChangeResponse> ResetPasswordAsync(string email, string? token, string? newPassword);
}
