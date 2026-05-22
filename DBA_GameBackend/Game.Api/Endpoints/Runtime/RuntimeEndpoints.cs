/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Runtime;
using Game.Shared.Contracts.GameServer;
using Game.Shared.Common;
using Game.Api.Extensions;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Api.Services.Settlement;
using Microsoft.EntityFrameworkCore;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using SettlementSubmitMatchResultRequest = Game.Shared.Contracts.Settlement.SubmitMatchResultRequest;
using SettlementMatchPlayerResultDto = Game.Shared.Contracts.Settlement.MatchPlayerResultDto;

namespace Game.Api.Endpoints.Runtime;

/// <summary>
/// 运行时相关接口（游戏服务器管理）/ Runtime APIs (Game Server Management)
/// </summary>
public static class RuntimeEndpoints
{
    /// <summary>
    /// 注册运行时端点 / Register runtime endpoints
    /// </summary>
    public static void MapRuntimeEndpoints(this IEndpointRouteBuilder app)
    {
        var runtimeServers = app.MapGroup("/runtime/servers").WithTags("Runtime");
        runtimeServers.MapPost("/register", RuntimeRegister);
        runtimeServers.MapPost("/ready", RuntimeReady);
        runtimeServers.MapPost("/heartbeat", RuntimeHeartbeat);
        runtimeServers.MapPost("/player-joined", RuntimePlayerJoined);
        runtimeServers.MapPost("/player-left", RuntimePlayerLeft);
        runtimeServers.MapPost("/match-started", RuntimeMatchStarted);
        runtimeServers.MapPost("/match-ended", RuntimeMatchEnded);
        app.MapPost("/runtime/matches/results", RuntimeMatchResults).WithTags("Runtime");

        var internalGroup = app.MapGroup("/internal/runtime").WithTags("运行时(内部)");

        internalGroup.MapPost("/servers", AllocateServer)
            .WithSummary("分配游戏服务器（内部）")
            .WithDescription(@"
内部接口，从运行时池分配一个可用的游戏服务器。
返回服务器的连接信息和运行时令牌。

**请求示例：**
```json
{
  ""mode"": ""classic"",
  ""mapId"": ""map_001"",
  ""sessionId"": ""uuid""
}
```
");

        internalGroup.MapGet("/servers/{serverId}", GetServer)
            .WithSummary("获取服务器信息（内部）")
            .WithDescription(@"
内部接口，获取指定服务器的详细信息。

**调用方式：**
GET /internal/runtime/servers/{serverId}
");

        internalGroup.MapPost("/servers/{serverId}/heartbeat", Heartbeat)
            .WithSummary("服务器心跳（内部）")
            .WithDescription(@"
内部接口，游戏服务器定期发送心跳以表明其仍然存活。
心跳间隔应小于30秒，否则服务器会被视为不健康。

**请求示例：**
```json
{
  ""currentPlayers"": 8,
  ""maxPlayers"": 10
}
```
");

        internalGroup.MapPost("/servers/{serverId}/ready", MarkReady)
            .WithSummary("标记服务器就绪（内部）")
            .WithDescription(@"
内部接口，游戏服务器完成初始化后调用，表明可以开始接受玩家连接。
");

        internalGroup.MapPost("/servers/{serverId}/stopped", MarkStopped)
            .WithSummary("标记服务器停止（内部）")
            .WithDescription(@"
内部接口，游戏服务器正常或异常停止后调用。
用于释放服务器资源并更新会话状态。

**请求示例：**
```json
{
  ""exitCode"": ""0"",
  ""crashReason"": null
}
```
");
    }

    private static async Task<IResult> RuntimeRegister(RuntimeRegisterRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        server.Status = "STARTING";
        server.LastHeartbeatAt = DateTimeOffset.UtcNow;
        AddServerEvent(db, request.ServerId, "RUNTIME_REGISTER", "{}");
        AddSessionEvent(db, request.SessionId, "SERVER_REGISTERED", $$"""{"serverId":"{{request.ServerId}}"}""");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeReady(RuntimeReadyRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var session = await db.GameSessions.FindAsync(request.SessionId);
        if (session is null) return ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem();

        server.Status = "READY";
        server.ReadyAt = DateTimeOffset.UtcNow;
        server.LastHeartbeatAt = DateTimeOffset.UtcNow;
        session.Status = "WAITING_PLAYERS";
        session.ServerId = server.Id;
        session.ServerIp = server.Ip;
        session.ServerPort = server.Port;
        session.UpdatedAt = DateTimeOffset.UtcNow;
        AddServerEvent(db, request.ServerId, "READY", "{}");
        AddSessionEvent(db, request.SessionId, "READY", $$"""{"serverId":"{{request.ServerId}}"}""");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeHeartbeat(RuntimeHeartbeatRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        server.LastHeartbeatAt = DateTimeOffset.UtcNow;
        server.UpdatedAt = DateTimeOffset.UtcNow;
        AddServerEvent(db, request.ServerId, "HEARTBEAT", "{}");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimePlayerJoined(RuntimePlayerJoinedRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var playerSession = await db.PlayerSessions
            .FirstOrDefaultAsync(x => x.GameSessionId == request.SessionId && x.PlayerId == request.PlayerId);
        if (playerSession is null) return ErrorResponse.NotFound(ErrorCodes.SessionPlayerNotInSession).ToProblem();

        if (!string.IsNullOrWhiteSpace(request.PlayerSessionToken) &&
            !string.Equals(playerSession.SessionTokenHash, HashToken(request.PlayerSessionToken), StringComparison.Ordinal))
        {
            return ErrorResponse.Unauthorized("Invalid player session token").ToProblem();
        }

        playerSession.Status = "JOINED";
        playerSession.JoinedAt = DateTimeOffset.UtcNow;
        AddSessionEvent(db, request.SessionId, "PLAYER_JOINED", $$"""{"playerId":"{{request.PlayerId}}"}""");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimePlayerLeft(RuntimePlayerLeftRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var playerSession = await db.PlayerSessions
            .FirstOrDefaultAsync(x => x.GameSessionId == request.SessionId && x.PlayerId == request.PlayerId);
        if (playerSession is not null)
        {
            playerSession.Status = "LEFT";
            playerSession.LeftAt = DateTimeOffset.UtcNow;
        }

        AddSessionEvent(db, request.SessionId, "PLAYER_LEFT", $$"""{"playerId":"{{request.PlayerId}}"}""");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeMatchStarted(RuntimeMatchStartedRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var session = await db.GameSessions.FindAsync(request.SessionId);
        if (session is null) return ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem();

        session.Status = "IN_PROGRESS";
        session.StartedAt ??= DateTimeOffset.UtcNow;
        session.UpdatedAt = DateTimeOffset.UtcNow;
        server.Status = "IN_PROGRESS";
        server.LastHeartbeatAt = DateTimeOffset.UtcNow;
        AddSessionEvent(db, request.SessionId, "MATCH_STARTED", "{}");
        AddServerEvent(db, request.ServerId, "MATCH_STARTED", "{}");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeMatchEnded(RuntimeMatchEndedRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var session = await db.GameSessions.FindAsync(request.SessionId);
        if (session is null) return ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem();

        session.Status = "SETTLING";
        session.UpdatedAt = DateTimeOffset.UtcNow;
        server.Status = "ENDING";
        AddSessionEvent(db, request.SessionId, "MATCH_ENDED", "{}");
        AddServerEvent(db, request.ServerId, "MATCH_ENDED", "{}");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeMatchResults(RuntimeMatchResultsRequest request, GameDbContext db, ISettlementService settlement)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var playerIds = await db.PlayerSessions
            .Where(x => x.GameSessionId == request.SessionId)
            .Select(x => x.PlayerId)
            .ToListAsync();
        if (request.Players.Any(x => !playerIds.Contains(x.PlayerId)))
        {
            return ErrorResponse.BadRequest("Match result contains players not in session").ToProblem();
        }

        var payload = new SettlementSubmitMatchResultRequest(
            request.SessionId,
            request.IdempotencyKey,
            string.IsNullOrWhiteSpace(request.ResultJson) ? JsonSerializer.Serialize(request) : request.ResultJson,
            request.Players.Select(x => new SettlementMatchPlayerResultDto(
                x.PlayerId, x.Team, x.Result, x.Kills, x.Deaths, x.Assists, x.Score, x.ExpDelta, x.Rewards)).ToList());

        var result = await settlement.SubmitMatchResultAsync(payload);
        return result is null
            ? ErrorResponse.BadRequest("Failed to settle match result").ToProblem()
            : Results.Ok(ApiResponse<object>.Ok(new { matchResultId = result.Id }));
    }

    private static async Task<Game.Infrastructure.Database.Entities.GameServerInstance?> ValidateRuntimeAsync(
        GameDbContext db,
        Guid serverId,
        Guid sessionId,
        string runtimeToken)
    {
        var tokenHash = HashToken(runtimeToken);
        return await db.GameServerInstances
            .FirstOrDefaultAsync(x =>
                x.Id == serverId &&
                x.SessionId == sessionId &&
                x.RuntimeTokenHash == tokenHash &&
                x.RuntimeTokenExpiresAt > DateTimeOffset.UtcNow);
    }

    private static void AddSessionEvent(GameDbContext db, Guid sessionId, string eventType, string payloadJson)
    {
        db.SessionEvents.Add(new Game.Infrastructure.Database.Entities.SessionEvent
        {
            Id = Guid.NewGuid(),
            GameSessionId = sessionId,
            EventType = eventType,
            PayloadJson = payloadJson,
            CreatedAt = DateTimeOffset.UtcNow
        });
    }

    private static void AddServerEvent(GameDbContext db, Guid serverId, string eventType, string payloadJson)
    {
        db.GameServerEvents.Add(new Game.Infrastructure.Database.Entities.GameServerEvent
        {
            Id = Guid.NewGuid(),
            ServerId = serverId,
            EventType = eventType,
            PayloadJson = payloadJson,
            CreatedAt = DateTimeOffset.UtcNow
        });
    }

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static async Task<IResult> AllocateServer(AllocateServerRequest request, Services.Runtime.IGameServerService svc)
    {
        var server = await svc.AllocateServerAsync(request);
        return server == null
            ? ErrorResponse.BadRequest("Failed to allocate server").ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> GetServer(Guid serverId, Services.Runtime.IGameServerService svc)
    {
        var server = await svc.GetServerByIdAsync(serverId);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> Heartbeat(Guid serverId, HeartbeatRequest request, Services.Runtime.IGameServerService svc)
    {
        var server = await svc.HeartbeatAsync(serverId, request);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> MarkReady(Guid serverId, Services.Runtime.IGameServerService svc)
    {
        var server = await svc.MarkReadyAsync(serverId);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> MarkStopped(Guid serverId, StoppedServerRequest request, Services.Runtime.IGameServerService svc)
    {
        var server = await svc.MarkStoppedAsync(serverId, request.ExitCode, request.CrashReason);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse.Ok());
    }
}
