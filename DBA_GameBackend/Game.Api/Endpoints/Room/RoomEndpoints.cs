/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Room;
using Game.Shared.Common;
using Game.Api.Extensions;
using Game.Shared.Errors;
using System.Security.Claims;

namespace Game.Api.Endpoints.Room;

/// <summary>
/// 房间相关接口 / Room APIs
/// </summary>
public static class RoomEndpoints
{
    /// <summary>
    /// 注册房间端点 / Register room endpoints
    /// </summary>
    public static void MapRoomEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/rooms").WithTags("房间");

        group.MapPost("/", CreateRoom)
            .WithSummary("创建房间")
            .WithDescription(@"
创建一个新的游戏房间，房主可以设置房间名称、密码、游戏模式等。
创建者自动成为房间所有者。

**请求示例：**
```json
{
  ""name"": ""MyRoom"",
  ""password"": ""optional-password"",
  ""maxPlayers"": 10,
  ""mode"": ""classic"",
  ""mapId"": ""map_001""
}
```
")
            .RequireAuthorization();

        group.MapGet("/", ListRooms)
            .WithSummary("获取房间列表")
            .WithDescription(@"
获取当前可加入的房间列表，支持按游戏模式和地区筛选。
不返回需要密码的房间。

**调用方式：**
GET /api/rooms?mode=classic&region=na
");

        group.MapGet("/{roomId}", GetRoom)
            .WithSummary("获取房间详情")
            .WithDescription(@"
获取指定房间的详细信息，包括玩家列表、房间状态等。
无需登录即可访问。
");

        group.MapPost("/{roomId}/join", JoinRoom)
            .WithSummary("加入房间")
            .WithDescription(@"
加入指定房间，如果房间有密码需要提供密码。
如果房间已满或已开始游戏，则无法加入。

**请求示例：**
```json
{
  ""password"": ""room-password""
}
```
")
            .RequireAuthorization();

        group.MapPost("/{roomId}/leave", LeaveRoom)
            .WithSummary("离开房间")
            .WithDescription(@"
离开当前所在房间，如果是房主离开则房间会被解散。
")
            .RequireAuthorization();

        group.MapPost("/{roomId}/ready", SetReady)
            .WithSummary("设置准备状态")
            .WithDescription(@"
设置自己在房间中的准备状态。
所有玩家准备后房主可以开始游戏。

**请求示例：**
```json
{
  ""isReady"": true
}
```
")
            .RequireAuthorization();

        group.MapPost("/{roomId}/start", StartGame)
            .WithSummary("开始游戏")
            .WithDescription(@"
房主开始游戏，所有玩家必须都已准备。
开始后会创建游戏会话并分配游戏服务器。
")
            .RequireAuthorization();

        group.MapPost("/{roomId}/kick", KickPlayer)
            .WithSummary("踢出玩家")
            .WithDescription(@"
房主可以将指定玩家踢出房间。
只有房主有权限踢人。

**请求示例：**
```json
{
  ""playerId"": ""uuid""
}
```
")
            .RequireAuthorization();

        group.MapPost("/{roomId}/transfer-owner", TransferOwner)
            .WithSummary("转让房主")
            .WithDescription(@"
将房间所有权转让给其他玩家。
只有当前房主可以操作。

**请求示例：**
```json
{
  ""newOwnerPlayerId"": ""uuid""
}
```
")
            .RequireAuthorization();
    }

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static async Task<IResult> CreateRoom(CreateRoomRequest request, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        try
        {
            var room = await svc.CreateRoomAsync(request, playerId.Value);
            return Results.Ok(ApiResponse<RoomResponse>.Ok(room));
        }
        catch (InvalidOperationException ex)
        {
            return ErrorResponse.BadRequest(ex.Message).ToProblem();
        }
    }

    private static async Task<IResult> ListRooms(string mode, string region, Services.Room.IRoomService svc) =>
        Results.Ok(ApiResponse<IReadOnlyList<RoomResponse>>.Ok(await svc.GetRoomsAsync(mode, region)));

    private static async Task<IResult> GetRoom(Guid roomId, Services.Room.IRoomService svc)
    {
        var room = await svc.GetRoomAsync(roomId);
        return room == null
            ? ErrorResponse.NotFound(ErrorCodes.RoomNotFound).ToProblem()
            : Results.Ok(ApiResponse<RoomResponse>.Ok(room));
    }

    private static async Task<IResult> JoinRoom(Guid roomId, JoinRoomRequest request, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        try
        {
            var room = await svc.JoinRoomAsync(roomId, playerId.Value, request.Password);
            return room == null
                ? ErrorResponse.NotFound(ErrorCodes.RoomNotFound).ToProblem()
                : Results.Ok(ApiResponse<RoomResponse>.Ok(room));
        }
        catch (InvalidOperationException ex)
        {
            return ErrorResponse.BadRequest(ex.Message).ToProblem();
        }
    }

    private static async Task<IResult> LeaveRoom(Guid roomId, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        await svc.LeaveRoomAsync(roomId, playerId.Value);
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> SetReady(Guid roomId, ReadyRoomRequest request, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        await svc.SetReadyAsync(roomId, playerId.Value, request.IsReady);
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> StartGame(Guid roomId, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        try
        {
            var room = await svc.StartGameAsync(roomId, playerId.Value);
            return room == null
                ? ErrorResponse.NotFound(ErrorCodes.RoomNotFound).ToProblem()
                : Results.Ok(ApiResponse<RoomResponse>.Ok(room));
        }
        catch (InvalidOperationException ex)
        {
            return ErrorResponse.BadRequest(ex.Message).ToProblem();
        }
    }

    private static async Task<IResult> KickPlayer(Guid roomId, KickPlayerRequest request, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        var success = await svc.KickPlayerAsync(roomId, playerId.Value, request.PlayerId);
        return success ? Results.Ok(ApiResponse.Ok()) : ErrorResponse.Forbidden().ToProblem();
    }

    private static async Task<IResult> TransferOwner(Guid roomId, TransferOwnerRequest request, Services.Room.IRoomService svc, HttpContext ctx)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        var room = await svc.TransferOwnerAsync(roomId, playerId.Value, request.NewOwnerPlayerId);
        return room == null
            ? ErrorResponse.Forbidden().ToProblem()
            : Results.Ok(ApiResponse<RoomResponse>.Ok(room));
    }
}