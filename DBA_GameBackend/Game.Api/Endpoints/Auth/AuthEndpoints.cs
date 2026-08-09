/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Auth;
using Game.Shared.Errors;
using Game.Api.Extensions;
using Game.Api.Services.Auth;
using Game.Api.Services.Player;
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
        var v1 = app.MapGroup("/api/v1/auth")
            .WithTags("认证 v1")
            .RequireRateLimiting("auth");

        v1.MapPost("/register", AccountRegister)
            .WithSummary("注册账号");
        v1.MapPost("/login", AccountLogin)
            .WithSummary("账号登录");
        v1.MapPost("/refresh", RefreshToken)
            .WithSummary("轮换刷新令牌");
        v1.MapPost("/logout", Logout)
            .WithSummary("退出登录并撤销刷新令牌")
            .RequireAuthorization();
        v1.MapGet("/me", GetMe)
            .WithSummary("获取当前账号信息")
            .RequireAuthorization();
        v1.MapPost("/player-name/generate", GeneratePlayerGameName)
            .WithSummary("获取当前玩家的 3–5 个汉字游戏名")
            .RequireAuthorization();
        v1.MapPost("/player-name/ensure", EnsurePlayerGameNameCompatibility)
            .WithSummary("兼容旧版自动玩家名接口")
            .WithDescription("已废弃，请迁移到 /api/v1/auth/player-name/generate。")
            .RequireAuthorization();

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
            return ToAuthFailure(result);
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
            return ToAuthFailure(result);
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 账号登录 / Account Login
    /// </summary>
    private static async Task<IResult> AccountLogin(AccountLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/login");
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountLoginAsync(request, ip, userAgent);
        if (!result.Success)
            return ToAuthFailure(result);
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    /// <summary>
    /// 账号注册 / Account Register
    /// </summary>
    private static async Task<IResult> AccountRegister(AccountRegisterRequest request, IAuthService auth, HttpContext ctx)
    {
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/register");
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountRegisterAsync(request, ip, userAgent);
        if (!result.Success)
            return ToAuthFailure(result);
        return Results.Ok(ApiResponse<LoginResponse>.Ok(
            new LoginResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId!.Value, result.Nickname!)));
    }

    private static async Task<IResult> LegacyLogin(AccountLoginRequest request, IAuthService auth, HttpContext ctx)
    {
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/login");
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var userAgent = ctx.Request.Headers.UserAgent.ToString();
        var result = await auth.AccountLoginAsync(request, ip, userAgent);
        return Results.Ok(ToLegacyLoginResponse(result, "Email"));
    }

    private static async Task<IResult> LegacyRegister(AccountRegisterRequest request, IAuthService auth, HttpContext ctx)
    {
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/register");
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
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/refresh");
        var ip = ctx.Connection.RemoteIpAddress?.ToString();
        var result = await auth.RefreshTokenAsync(request.RefreshToken ?? string.Empty, ip);
        if (!result.Success)
            return ToAuthFailure(result);
        return Results.Ok(ApiResponse<RefreshTokenResponse>.Ok(
            new RefreshTokenResponse(result.AccessToken!, result.RefreshToken!, result.PlayerId, result.Nickname)));
    }

    /// <summary>
    /// 退出登录 / Logout
    /// </summary>
    private static async Task<IResult> Logout(LogoutRequest? request, IAuthService auth, HttpContext ctx)
    {
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/logout");
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
        ApplyLegacyAuthDeprecationHeaders(ctx, "/api/v1/auth/me");
        var accountId = ctx.User.FindFirst(ClaimTypes.NameIdentifier)?.Value
            ?? ctx.User.FindFirst("sub")?.Value;
        if (!Guid.TryParse(accountId, out var accId))
            return ErrorResponse.Unauthorized().ToProblem();
        var me = await auth.GetMeAsync(accId);
        if (me == null)
            return ErrorResponse.NotFound("账号不存在。").ToProblem();
        return Results.Ok(ApiResponse<MeResponse>.Ok(me));
    }

    /// <summary>
    /// 获取当前认证玩家的 3–5 个汉字游戏名。
    /// 接口保持幂等：首次调用生成并持久化，后续调用返回同一名称；客户端不能指定玩家或候选名称。
    /// </summary>
    private static async Task<IResult> GeneratePlayerGameName(IPlayerService playerService, HttpContext ctx, CancellationToken cancellationToken)
    {
        var playerIdText = ctx.User.FindFirst("player_id")?.Value;
        if (!Guid.TryParse(playerIdText, out var playerId))
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var result = await playerService.EnsureGeneratedGameNameAsync(playerId, cancellationToken);
        if (!result.Success || string.IsNullOrWhiteSpace(result.Nickname))
        {
            return ErrorResponse.Create(
                StatusCodes.Status503ServiceUnavailable,
                "游戏玩家名生成失败",
                result.ErrorMessage ?? "暂时无法生成游戏玩家名。",
                code: ErrorCodes.PlayerGameNameGenerationFailed).ToProblem();
        }

        return Results.Ok(ApiResponse<PlayerGameNameResponse>.Ok(
            new PlayerGameNameResponse(playerId, result.Nickname, result.WasGenerated)));
    }

    /// <summary>
    /// 旧 /ensure 路径的只读迁移入口。它复用同一生成逻辑，不形成第二套玩家名实现。
    /// </summary>
    private static Task<IResult> EnsurePlayerGameNameCompatibility(
        IPlayerService playerService,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        ctx.Response.Headers["Deprecation"] = "true";
        ctx.Response.Headers["Link"] = "</api/v1/auth/player-name/generate>; rel=\"successor-version\"";
        return GeneratePlayerGameName(playerService, ctx, cancellationToken);
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

    private static IResult ToAuthFailure(AuthServiceResult result)
    {
        var code = result.ErrorCode ?? ErrorCodes.InternalError;
        var (status, title) = code switch
        {
            ErrorCodes.AuthInvalidCredentials => (StatusCodes.Status401Unauthorized, "认证失败"),
            ErrorCodes.AuthTokenExpired => (StatusCodes.Status401Unauthorized, "令牌已过期"),
            ErrorCodes.AuthRefreshTokenReused => (StatusCodes.Status401Unauthorized, "刷新令牌已重放"),
            ErrorCodes.AuthAccountDisabled => (StatusCodes.Status403Forbidden, "账号不可用"),
            ErrorCodes.PlayerNicknameTaken => (StatusCodes.Status409Conflict, "账号标识已被使用"),
            ErrorCodes.ValidationError => (StatusCodes.Status400BadRequest, "认证请求无效"),
            _ => (StatusCodes.Status400BadRequest, "认证请求失败")
        };

        var message = result.ErrorMessage ?? "认证请求未完成。";
        return ErrorResponse.Create(status, title, message, code: code).ToProblem();
    }

    /**
     * 只对 /api/auth 下已有 v1 successor 的兼容路由添加废弃响应头。
     * guest/dev/external/password 等尚无 v1 successor 的接口不会被误标记。
     */
    private static void ApplyLegacyAuthDeprecationHeaders(HttpContext context, string successorPath)
    {
        if (!context.Request.Path.StartsWithSegments("/api/auth"))
        {
            return;
        }

        context.Response.Headers["Deprecation"] = "true";
        context.Response.Headers["Link"] = $"<{successorPath}>; rel=\"successor-version\"";
    }

    private static object ToLegacyLoginResponse(AuthServiceResult result, string loginType)
    {
        if (!result.Success)
        {
            return new
            {
                success = false,
                error = result.ErrorMessage ?? result.ErrorCode ?? "登录失败。"
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
                displayName = result.Nickname ?? "玩家",
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
