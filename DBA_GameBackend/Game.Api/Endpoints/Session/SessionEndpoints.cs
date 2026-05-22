/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Session;
using Game.Shared.Common;
using Game.Api.Extensions;
using Game.Shared.Errors;

namespace Game.Api.Endpoints.Session;

/// <summary>
/// 会话相关接口 / Session APIs
/// </summary>
public static class SessionEndpoints
{
    /// <summary>
    /// 注册会话端点 / Register session endpoints
    /// </summary>
    public static void MapSessionEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/sessions").WithTags("会话");
        var internalGroup = app.MapGroup("/internal/sessions").WithTags("会话(内部)");

        // 客户端接口
        group.MapGet("/{sessionId}", GetSession)
            .WithSummary("获取会话详情")
            .WithDescription(@"
获取指定会话的详细信息，包括会话状态、玩家列表、游戏服务器信息等。

**调用方式：**
GET /api/sessions/{sessionId}
");

        group.MapGet("/{sessionId}/connection", GetConnection)
            .WithSummary("获取会话连接信息")
            .WithDescription(@"
获取当前玩家连接会话所需的连接信息，包括游戏服务器地址和端口。
只有参与该会话的玩家可以获取。

**调用方式：**
GET /api/sessions/{sessionId}/connection
Header: Authorization: Bearer <access-token>
")
            .RequireAuthorization();

        // 内部接口
        internalGroup.MapPost("/from-room", CreateFromRoom)
            .WithSummary("从房间创建会话（内部）")
            .WithDescription(@"
内部接口，当房间游戏开始时调用，用于创建游戏会话。
");

        internalGroup.MapPost("/from-match", CreateFromMatch)
            .WithSummary("从匹配创建会话（内部）")
            .WithDescription(@"
内部接口，匹配成功后调用，用于创建游戏会话。
");

        internalGroup.MapPost("/{sessionId}/allocate-server", AllocateServer)
            .WithSummary("分配游戏服务器（内部）")
            .WithDescription(@"
内部接口，为会话分配游戏服务器。
");

        internalGroup.MapPost("/{sessionId}/mark-in-progress", MarkInProgress)
            .WithSummary("标记为进行中（内部）")
            .WithDescription(@"
内部接口，标记会话游戏已开始进行。
");

        internalGroup.MapPost("/{sessionId}/mark-completed", MarkCompleted)
            .WithSummary("标记为已完成（内部）")
            .WithDescription(@"
内部接口，标记会话游戏已正常结束。
");

        internalGroup.MapPost("/{sessionId}/mark-failed", MarkFailed)
            .WithSummary("标记为失败（内部）")
            .WithDescription(@"
内部接口，标记会话游戏因异常而失败。

**请求示例：**
```json
{
  ""reason"": ""server crash""
}
```
");
    }

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static async Task<IResult> GetSession(Guid sessionId, Services.Session.ISessionService svc)
    {
        var session = await svc.GetSessionAsync(sessionId);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> GetConnection(Guid sessionId, Services.Session.ISessionService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        var conn = await svc.GetConnectionInfoAsync(sessionId, playerId.Value);
        return conn == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionPlayerNotInSession).ToProblem()
            : Results.Ok(ApiResponse<SessionConnectionResponse>.Ok(conn));
    }

    private static async Task<IResult> CreateFromRoom(InternalCreateSessionFromRoomRequest request, Services.Session.ISessionService svc)
    {
        var session = await svc.CreateFromRoomAsync(request.RoomId);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> CreateFromMatch(InternalCreateSessionFromMatchRequest request, Services.Session.ISessionService svc)
    {
        var session = await svc.CreateFromMatchAsync(request.TicketId);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> AllocateServer(Guid sessionId, InternalAllocateServerRequest request, Services.Session.ISessionService svc)
    {
        var session = await svc.AllocateServerAsync(sessionId, request.Ip, request.Port, request.RuntimeToken);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> MarkInProgress(Guid sessionId, Services.Session.ISessionService svc)
    {
        var session = await svc.MarkInProgressAsync(sessionId);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> MarkCompleted(Guid sessionId, Services.Session.ISessionService svc)
    {
        var session = await svc.MarkCompletedAsync(sessionId);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }

    private static async Task<IResult> MarkFailed(Guid sessionId, InternalMarkFailedRequest request, Services.Session.ISessionService svc)
    {
        var session = await svc.MarkFailedAsync(sessionId, request.Reason);
        return session == null
            ? ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem()
            : Results.Ok(ApiResponse<SessionResponse>.Ok(session));
    }
}