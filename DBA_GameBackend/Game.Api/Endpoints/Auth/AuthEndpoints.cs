/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Auth;
using Game.Api.Extensions;
using Game.Api.Services.Auth;
using System.Security.Claims;

namespace Game.Api.Endpoints.Auth;

/// <summary>
/// 认证相关接口 / Authentication APIs
/// </summary>
public static class AuthEndpoints
{
    /// <summary>
    /// 注册认证端点 / Register authentication endpoints
    /// </summary>
    public static void MapAuthEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/auth")
            .WithTags("认证")
            .RequireRateLimiting("auth");

        group.MapPost("/guest-login", GuestLogin)
            .WithSummary("访客登录")
            .WithDescription(@"
访客登录是一种无需注册即可快速体验游戏的登录方式。
系统会根据设备ID生成或关联游客账号，每个设备可创建多个访客账号。
如果设备已关联账号，则直接返回已存在账号的登录信息。

**请求示例：**
```json
{
  ""deviceId"": ""可选的设备UUID""
}
```

**响应示例：**
```json
{
  ""success"": true,
  ""data"": {
    ""accessToken"": ""jwt-token"",
    ""refreshToken"": ""base64-token"",
    ""playerId"": ""uuid"",
    ""nickname"": ""Player_name""
  }
}
```
");

        group.MapPost("/dev-login", DevLogin)
            .WithSummary("开发者登录")
            .WithDescription(@"
仅在开发环境可用，用于开发者快速测试。
生产环境中此接口会被禁用。

**请求示例：**
```json
{
  ""username"": ""TestPlayer"",
  ""password"": ""password123""
}
```
");

        group.MapPost("/account/login", AccountLogin)
            .WithSummary("账号登录")
            .WithDescription(@"
使用用户名和密码登录游戏账号。

**请求示例：**
```json
{
  ""username"": ""Player123"",
  ""password"": ""password123""
}
```
");

        group.MapPost("/account/register", AccountRegister)
            .WithSummary("账号注册")
            .WithDescription(@"
注册新的游戏账号。

**请求示例：**
```json
{
  ""username"": ""NewPlayer"",
  ""password"": ""password123"",
  ""email"": ""optional@email.com""
}
```
");

        group.MapPost("/login", LegacyLogin)
            .WithSummary("客户端兼容登录")
            .WithDescription("兼容旧版 Unreal 客户端的 /api/auth/login 请求与扁平响应格式。");

        group.MapPost("/register", LegacyRegister)
            .WithSummary("客户端兼容注册")
            .WithDescription("兼容旧版 Unreal 客户端的 /api/auth/register 请求与扁平响应格式。");

        group.MapPost("/external/wechat", WeChatExternalLogin)
            .WithSummary("微信登录")
            .WithDescription(@"
通过微信进行认证登录（当前为模拟模式）。
生产环境需要与微信 API 进行票据验证。

**请求示例：**
```json
{
  ""code"": ""微信授权码"",
  ""nickname"": ""可选的昵称""
}
```
");

        group.MapPost("/refresh", RefreshToken)
            .WithSummary("刷新访问令牌")
            .WithDescription(@"
使用刷新令牌获取新的访问令牌和刷新令牌。
刷新令牌有效期为30天，每次刷新后会自动延期。
刷新令牌使用后会被撤销，同时生成新的令牌对。

**请求示例：**
```json
{
  ""refreshToken"": ""base64-token""
}
```
");

        group.MapPost("/logout", Logout)
            .WithSummary("退出登录")
            .WithDescription(@"
退出当前账号，撤销刷新令牌。
需要携带有效的访问令牌。

**调用方式：**
POST /api/auth/logout?refreshToken=xxx
Header: Authorization: Bearer <access-token>
")
            .RequireAuthorization();

        group.MapGet("/me", GetMe)
            .WithSummary("获取当前账号信息")
            .WithDescription(@"
返回当前登录账号的详细信息，包括账号ID、玩家ID、昵称和账号类型。
需要携带有效的访问令牌。
")
            .RequireAuthorization();

        group.MapPost("/external/steam", SteamExternalLogin)
            .WithSummary("Steam外部登录")
            .WithDescription(@"
通过Steam平台进行认证登录（当前为模拟模式）。
生产环境需要与Steam API进行票据验证。

**请求示例：**
```json
{
  ""steamTicket"": ""steam-ticket-string""
}
```
");

        group.MapPost("/external/eos", EosExternalLogin)
            .WithSummary("EOS外部登录")
            .WithDescription(@"
通过Epic Online Services进行认证登录（当前为模拟模式）。
生产环境需要与EOS API进行票据验证。

**请求示例：**
```json
{
  ""eosTicket"": ""eos-ticket-string""
}
```
");

        group.MapPost("/change-password", ChangePassword)
            .WithSummary("修改密码")
            .WithDescription(@"
修改当前账号的密码。需要登录后调用。

**请求示例：**
```json
{
  ""oldPassword"": ""oldPass123"",
  ""newPassword"": ""newPass456""
}
```
")
            .RequireAuthorization();

        group.MapPost("/reset-password", ResetPassword)
            .WithSummary("重置密码")
            .WithDescription(@"
通过邮箱重置密码。如果提供了token和新密码，直接更新；如果只提供邮箱，发送重置邮件。

**请求示例：**
```json
{
  ""email"": ""user@example.com"",
  ""token"": ""可选的重置token"",
  ""newPassword"": ""可选的新密码""
}
```
");
    }

    /// <summary>
    /// 访客登录 / Guest Login
    /// </summary>
    private static async Task<IResult> GuestLogin(GuestLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.GuestLoginAsync(request, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<GuestLoginResponse>.Ok(
            new GuestLoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 开发者登录 / Developer Login
    /// </summary>
    private static async Task<IResult> DevLogin(DevLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.DevLoginAsync(request, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 账号登录 / Account Login
    /// </summary>
    private static async Task<IResult> AccountLogin(AccountLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountLoginAsync(request, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 账号注册 / Account Register
    /// </summary>
    private static async Task<IResult> AccountRegister(AccountRegisterRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountRegisterAsync(request, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    private static async Task<IResult> LegacyLogin(AccountLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountLoginAsync(request, ip, userAgent);
        return Results.Ok(ToLegacyLoginResponse(result, "Email"));
    }

    private static async Task<IResult> LegacyRegister(AccountRegisterRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountRegisterAsync(request, ip, userAgent);
        return Results.Ok(ToLegacyLoginResponse(result, "Email"));
    }

    /// <summary>
    /// 刷新访问令牌 / Refresh Access Token
    /// </summary>
    private static async Task<IResult> RefreshToken(RefreshTokenRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var result = await auth.RefreshTokenAsync(request.RefreshToken ?? string.Empty, ip);
        if (!result.Success)
            return ErrorResponse.Unauthorized(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<RefreshTokenResponse>.Ok(
            new RefreshTokenResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId, result.Nickname)));
    }

    /// <summary>
    /// 退出登录 / Logout
    /// </summary>
    private static async Task<IResult> Logout(LogoutRequest? request, IAuthService auth, HttpContext ctx)
    {
        var accountId = ctx.User.FindFirst(ClaimTypes.NameIdentifier)?.Value
            ?? ctx.User.FindFirst("sub")?.Value;
        if (Guid.TryParse(accountId, out var accId))
            await auth.LogoutAsync(accId, request?.RefreshToken);
        return Results.Ok(ApiResponse.Ok());
    }

    /// <summary>
    /// 获取当前账号信息 / Get Current Account Info
    /// </summary>
    private static async Task<IResult> GetMe(IAuthService auth, HttpContext ctx)
    {
        var accountId = ctx.User.FindFirst(ClaimTypes.NameIdentifier)?.Value
            ?? ctx.User.FindFirst("sub")?.Value;
        if (!Guid.TryParse(accountId, out var accId))
            return ErrorResponse.Unauthorized().ToProblem();
        var me = await auth.GetMeAsync(accId);
        if (me == null)
            return ErrorResponse.NotFound("Account not found").ToProblem();
        return Results.Ok(ApiResponse<MeResponse>.Ok(me));
    }

    /// <summary>
    /// Steam外部登录 / Steam External Login
    /// </summary>
    private static async Task<IResult> SteamExternalLogin(SteamExternalLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.SteamExternalLoginAsync(request.SteamTicket, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<ExternalLoginResponse>.Ok(
            new ExternalLoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// EOS外部登录 / EOS External Login
    /// </summary>
    private static async Task<IResult> EosExternalLogin(EosExternalLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.EosExternalLoginAsync(request.EosTicket ?? request.EosToken ?? string.Empty, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<ExternalLoginResponse>.Ok(
            new ExternalLoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 微信外部登录 / WeChat External Login
    /// </summary>
    private static async Task<IResult> WeChatExternalLogin(WeChatExternalLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.WeChatExternalLoginAsync(request.Code, request.Nickname, ip, userAgent);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.ErrorMessage ?? result.ErrorCode!).ToProblem();
        return Results.Ok(ApiResponse<WeChatLoginResponse>.Ok(
            new WeChatLoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!, true)));
    }

    /// <summary>
    /// 修改密码 / Change Password
    /// </summary>
    private static async Task<IResult> ChangePassword(ChangePasswordRequest request, IAuthService auth, HttpContext ctx)
    {
        var accountId = ctx.User.FindFirst(ClaimTypes.NameIdentifier)?.Value
            ?? ctx.User.FindFirst("sub")?.Value;
        if (!Guid.TryParse(accountId, out var accId))
            return ErrorResponse.Unauthorized().ToProblem();
        var result = await auth.ChangePasswordAsync(accId, request.OldPassword, request.NewPassword);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.Message ?? "Password change failed").ToProblem();
        return Results.Ok(ApiResponse<PasswordChangeResponse>.Ok(result));
    }

    /// <summary>
    /// 重置密码 / Reset Password
    /// </summary>
    private static async Task<IResult> ResetPassword(ResetPasswordRequest request, IAuthService auth, HttpContext ctx)
    {
        var result = await auth.ResetPasswordAsync(request.Email, request.Token, request.NewPassword);
        if (!result.Success)
            return ErrorResponse.BadRequest(result.Message ?? "Password reset failed").ToProblem();
        return Results.Ok(ApiResponse<PasswordChangeResponse>.Ok(result));
    }

    private static object ToLegacyLoginResponse(AuthServiceResult result, string loginType)
    {
        if (!result.Success)
        {
            return new
            {
                success = false,
                error = result.ErrorMessage ?? result.ErrorCode ?? "Login failed"
            };
        }

        var now = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        return new
        {
            success = true,
            token = result.AccessToken,
            refreshToken = result.RefreshToken,
            account = new
            {
                accountId = result.AccountId?.ToString() ?? result.PlayerId?.ToString() ?? string.Empty,
                playerId = result.PlayerId?.ToString() ?? string.Empty,
                displayName = result.Nickname ?? "Player",
                loginType,
                status = "Normal",
                level = 1,
                experience = 0,
                createTime = now,
                lastLoginTime = now
            }
        };
    }
}
