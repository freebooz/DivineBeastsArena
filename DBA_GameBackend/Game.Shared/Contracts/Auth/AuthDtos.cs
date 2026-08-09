/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;

namespace Game.Shared.Contracts.Auth;

public record GuestLoginRequest(string? DeviceId, string? DeviceName = null, string? Platform = null);
public record GuestLoginResponse(string AccessToken, string RefreshToken, Guid PlayerId, string Nickname);

public record DevLoginRequest(string? Username = null, string? Password = null, string? DisplayName = null);
public record LoginResponse(string AccessToken, string RefreshToken, Guid PlayerId, string Nickname);

public record AccountLoginRequest(string? Username = null, string? Password = null, string? Email = null, string? LoginType = null, string? DeviceId = null, string? ThirdPartyToken = null);
public record AccountRegisterRequest(string? Username = null, string? Password = null, string? Email = null, string? LoginType = null, string? DeviceId = null, string? ThirdPartyToken = null);

public record RefreshTokenRequest(string? RefreshToken = null);
public record RefreshTokenResponse(string AccessToken, string RefreshToken, Guid? PlayerId = null, string? Nickname = null);
public record LogoutRequest(string? RefreshToken = null);

public record SteamExternalLoginRequest(string SteamTicket);
public record EosExternalLoginRequest(string? EosTicket = null, string? EosToken = null);
public record ExternalLoginResponse(string AccessToken, string RefreshToken, Guid PlayerId, string Nickname);

public record WeChatExternalLoginRequest(string Code, string? Nickname);
public record WeChatLoginResponse(string AccessToken, string RefreshToken, Guid PlayerId, string Nickname, bool IsNewPlayer);

public record MeResponse(Guid AccountId, Guid PlayerId, string Nickname, string AccountType, string? Email);
/** 受鉴权保护的玩家名生成接口响应；首次开户后的前台编排调用，同一玩家重复调用返回同一名称。 */
public record PlayerGameNameResponse(Guid PlayerId, string Nickname, bool WasGenerated);

// Password management requests
public record ChangePasswordRequest(string OldPassword, string NewPassword);
public record ResetPasswordRequest(string Email, string? Token, string? NewPassword);
public record PasswordChangeResponse(bool Success, string? Message);
