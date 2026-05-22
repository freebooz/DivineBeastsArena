/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Match;
using Game.Shared.Common;
using Game.Api.Extensions;
using Game.Shared.Errors;
using System.Security.Claims;

namespace Game.Api.Endpoints.Match;

/// <summary>
/// 匹配系统相关接口 / Matchmaking APIs
/// </summary>
public static class MatchEndpoints
{
    /// <summary>
    /// 注册匹配端点 / Register matchmaking endpoints
    /// </summary>
    public static void MapMatchEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/matchmaking").WithTags("匹配");

        group.MapPost("/tickets", CreateTicket)
            .WithSummary("创建匹配票据")
            .WithDescription(@"
创建匹配票据进入匹配队列，系统会自动匹配合适的对手。
可以指定想要的游戏模式和地图偏好。

**请求示例：**
```json
{
  ""mode"": ""ranked"",
  ""mapPreferences"": [""map_001"", ""map_002""],
  ""teamSize"": 1
}
```

**响应示例：**
```json
{
  ""success"": true,
  ""data"": {
    ""ticketId"": ""uuid"",
    ""status"": ""pending"",
    ""estimatedWaitTimeSeconds"": 60
  }
}
```
")
            .RequireAuthorization();

        group.MapGet("/tickets/{ticketId}", GetTicket)
            .WithSummary("获取匹配票据状态")
            .WithDescription(@"
查询指定匹配票据的当前状态。
状态包括：pending（匹配中）、found（已找到对手）、expired（已过期）、cancelled（已取消）。

**调用方式：**
GET /api/matchmaking/tickets/{ticketId}
")
            .RequireAuthorization();

        group.MapDelete("/tickets/{ticketId}", CancelTicket)
            .WithSummary("取消匹配票据")
            .WithDescription(@"
取消当前正在匹配的票据，退出匹配队列。
只有票据创建者可以取消自己的票据。

**调用方式：**
DELETE /api/matchmaking/tickets/{ticketId}
")
            .RequireAuthorization();
    }

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static async Task<IResult> CreateTicket(CreateMatchmakingTicketRequest request, Services.Match.IMatchService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        try
        {
            var ticket = await svc.CreateTicketAsync(playerId.Value, request);
            return Results.Ok(ApiResponse<MatchmakingTicketResponse>.Ok(ticket));
        }
        catch (InvalidOperationException ex)
        {
            return ErrorResponse.BadRequest(ex.Message).ToProblem();
        }
    }

    private static async Task<IResult> GetTicket(Guid ticketId, Services.Match.IMatchService svc)
    {
        var ticket = await svc.GetTicketAsync(ticketId);
        return ticket == null
            ? ErrorResponse.NotFound(ErrorCodes.MatchTicketNotFound).ToProblem()
            : Results.Ok(ApiResponse<MatchmakingTicketResponse>.Ok(ticket));
    }

    private static async Task<IResult> CancelTicket(Guid ticketId, Services.Match.IMatchService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        await svc.CancelTicketAsync(ticketId, playerId.Value);
        return Results.Ok(ApiResponse.Ok());
    }
}