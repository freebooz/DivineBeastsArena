/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Player;
using Game.Api.Extensions;
using Game.Api.Services.Player;
using Game.Shared.Errors;
using System.Security.Claims;

namespace Game.Api.Endpoints.Player;

/// <summary>
/// 玩家相关接口 / Player APIs
/// </summary>
public static class PlayerEndpoints
{
    /// <summary>
    /// 注册玩家端点 / Register player endpoints
    /// </summary>
    public static void MapPlayerEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/players").WithTags("玩家");

        group.MapGet("/me/profile", GetMyProfile)
            .WithSummary("获取当前玩家个人资料")
            .WithDescription(@"
获取当前登录玩家的个人资料信息，包括昵称、等级、经验值、头像等。
需要携带有效的访问令牌。

**响应示例：**
```json
{
  ""success"": true,
  ""data"": {
    ""playerId"": ""uuid"",
    ""nickname"": ""Player_name"",
    ""avatar"": null,
    ""level"": 1,
    ""exp"": 0
  }
}
```
")
            .RequireAuthorization();

        group.MapGet("/me/settings", GetMySettings)
            .WithSummary("获取当前玩家设置")
            .WithDescription(@"
获取当前玩家的游戏设置，包括画质、音量、控制等配置。
设置以JSON格式存储。
")
            .RequireAuthorization();

        group.MapGet("/me/stats", GetMyStats)
            .WithSummary("获取当前玩家统计数据")
            .WithDescription(@"
获取当前玩家的游戏统计数据，包括总场次、胜率、击杀/死亡/助攻等。
这些数据会在比赛结束后自动更新。
")
            .RequireAuthorization();

        group.MapGet("/me/unlocks", GetMyUnlocks)
            .WithSummary("获取当前玩家已解锁内容")
            .WithDescription(@"
获取当前玩家已解锁的皮肤、道具、成就等奖励。
解锁内容来源于比赛奖励、活动或商城购买。
")
            .RequireAuthorization();

        group.MapGet("/{playerId}/public", GetPublicProfile)
            .WithSummary("获取指定玩家公开资料")
            .WithDescription(@"
获取指定玩家的公开信息，无需登录即可访问。
仅返回昵称、头像、等级等公开信息。

**调用方式：**
GET /api/players/{playerId}/public
");
    }

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id") ?? ctx.User.FindFirst(ClaimTypes.NameIdentifier);
        if (claim == null || !Guid.TryParse(claim.Value, out var id)) return null;
        return id;
    }

    private static async Task<IResult> GetMyProfile(IPlayerService playerSvc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null) return ErrorResponse.Unauthorized().ToProblem();
        var profile = await playerSvc.GetProfileAsync(playerId.Value);
        if (profile == null) return ErrorResponse.NotFound(ErrorCodes.PlayerNotFound).ToProblem();
        return Results.Ok(ApiResponse<PlayerProfileResponse>.Ok(profile));
    }

    private static async Task<IResult> GetMySettings(IPlayerService playerSvc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null) return ErrorResponse.Unauthorized().ToProblem();
        var settings = await playerSvc.GetSettingsAsync(playerId.Value);
        if (settings == null) return ErrorResponse.NotFound("Settings not found").ToProblem();
        return Results.Ok(ApiResponse<PlayerSettingsResponse>.Ok(settings));
    }

    private static async Task<IResult> GetMyStats(IPlayerService playerSvc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null) return ErrorResponse.Unauthorized().ToProblem();
        var stats = await playerSvc.GetStatisticsAsync(playerId.Value);
        if (stats == null) return ErrorResponse.NotFound("Statistics not found").ToProblem();
        return Results.Ok(ApiResponse<PlayerStatisticsResponse>.Ok(stats));
    }

    private static async Task<IResult> GetMyUnlocks(IPlayerService playerSvc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (playerId == null) return ErrorResponse.Unauthorized().ToProblem();
        var unlocks = await playerSvc.GetUnlocksAsync(playerId.Value);
        return Results.Ok(ApiResponse<PlayerUnlocksResponse>.Ok(unlocks!));
    }

    private static async Task<IResult> GetPublicProfile(Guid playerId, IPlayerService playerSvc)
    {
        var profile = await playerSvc.GetPublicProfileAsync(playerId);
        if (profile == null) return ErrorResponse.NotFound(ErrorCodes.PlayerNotFound).ToProblem();
        return Results.Ok(ApiResponse<PlayerPublicProfileResponse>.Ok(profile));
    }
}