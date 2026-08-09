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
using Game.Infrastructure.Redis;
using Game.Api.Services.Runtime;
using Game.Api.Services.Settlement;
using Game.Application.Sessions;
using Microsoft.EntityFrameworkCore;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

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
        runtimeServers.MapPost("/validate-join-ticket", RuntimePlayerJoined);
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

        if (server.Status is not ("READY" or "ALLOCATED" or "IN_PROGRESS" or "ENDING"))
        {
            server.Status = "STARTING";
        }
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

    private static async Task<IResult> RuntimePlayerJoined(
        RuntimePlayerJoinedRequest request,
        GameDbContext db,
        IConsumeJoinTicketUseCase consumeJoinTicket,
        IGameTicketRedisRegistry gameTicketRegistry)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("运行时令牌无效。").ToProblem();

        var joinTicket = string.IsNullOrWhiteSpace(request.JoinTicket)
            ? request.PlayerSessionToken
            : request.JoinTicket;
        var consumeCommand = new ConsumeJoinTicketCommand(
            request.PlayerId,
            request.CharacterId ?? Guid.Empty,
            request.SessionId,
            server.Id,
            request.BuildId ?? string.Empty,
            joinTicket ?? string.Empty,
            request.Zodiac ?? string.Empty,
            request.PrimaryElement ?? string.Empty,
            request.FiveCamp,
            request.FixedSkillGroupId ?? string.Empty);
        // Redis 首先原子删除同一张已签发票据；PostgreSQL 随后以完整上下文条件更新作为最终权威校验。
        // 两层均拒绝复用，且两处都不记录 Ticket 明文。
        if (!await gameTicketRegistry.TryConsumeAsync(consumeCommand))
        {
            return ErrorResponse.Unauthorized("一次性入服票据无效、已过期、已使用或与入服上下文不匹配。").ToProblem();
        }

        var consumed = await consumeJoinTicket.ExecuteAsync(consumeCommand);
        if (consumed is null)
        {
            return ErrorResponse.Unauthorized("一次性入服票据无效、已过期、已使用或与入服上下文不匹配。").ToProblem();
        }

        AddSessionEvent(db, request.SessionId, "PLAYER_JOINED", JsonSerializer.Serialize(new
        {
            playerId = consumed.AccountId,
            characterId = consumed.CharacterId,
            team = consumed.Team,
            zodiac = consumed.Zodiac,
            primaryElement = consumed.PrimaryElement,
            fiveCamp = consumed.FiveCamp,
            fixedSkillGroupId = consumed.FixedSkillGroupId
        }));
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimePlayerLeft(RuntimePlayerLeftRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var playerSession = await db.PlayerSessions
            .FirstOrDefaultAsync(x => x.GameSessionId == request.SessionId && x.PlayerId == request.PlayerId);
        if (playerSession is null) return ErrorResponse.NotFound(ErrorCodes.SessionPlayerNotInSession).ToProblem();

        if (playerSession.LeftAt is not null)
        {
            return Results.Ok(ApiResponse.Ok());
        }

        if (playerSession.JoinedAt is null || playerSession.Status != "JOINED")
        {
            return ErrorResponse.BadRequest("Player has not joined session").ToProblem();
        }

        playerSession.Status = "LEFT";
        playerSession.LeftAt = DateTimeOffset.UtcNow;

        AddSessionEvent(db, request.SessionId, "PLAYER_LEFT", $$"""{"playerId":"{{request.PlayerId}}"}""");
        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RuntimeMatchStarted(RuntimeMatchStartedRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        return await RuntimeLifecycleService.MarkMatchStartedAsync(db, request.ServerId, request.SessionId)
            ? Results.Ok(ApiResponse.Ok())
            : ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem();
    }

    private static async Task<IResult> RuntimeMatchEnded(RuntimeMatchEndedRequest request, GameDbContext db)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        return await RuntimeLifecycleService.MarkMatchEndedAsync(db, request.ServerId, request.SessionId)
            ? Results.Ok(ApiResponse.Ok())
            : ErrorResponse.NotFound(ErrorCodes.SessionNotFound).ToProblem();
    }

    private static async Task<IResult> RuntimeMatchResults(RuntimeMatchResultsRequest request, GameDbContext db, ISettlementService settlement)
    {
        var server = await ValidateRuntimeAsync(db, request.ServerId, request.SessionId, request.RuntimeToken);
        if (server is null) return ErrorResponse.Unauthorized("Invalid runtime token").ToProblem();

        var sessionPlayerTeams = await db.PlayerSessions
            .Where(x => x.GameSessionId == request.SessionId)
            .Select(x => new { x.PlayerId, x.Team })
            .ToDictionaryAsync(x => x.PlayerId, x => x.Team);
        var validation = RuntimeMatchResultsValidator.ValidateAndBuildPayload(request, sessionPlayerTeams);
        if (!validation.IsValid || validation.Payload is null)
        {
            return ErrorResponse.BadRequest(validation.ErrorMessage ?? "Invalid match result").ToProblem();
        }

        var result = await settlement.SubmitMatchResultAsync(validation.Payload);
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

    private static async Task<IResult> AllocateServer(
        AllocateServerRequest request,
        Services.Runtime.IGameServerService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var server = await svc.AllocateServerAsync(request);
        return server == null
            ? ErrorResponse.BadRequest("Failed to allocate server").ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> GetServer(
        Guid serverId,
        Services.Runtime.IGameServerService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var server = await svc.GetServerByIdAsync(serverId);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> Heartbeat(
        Guid serverId,
        HeartbeatRequest request,
        Services.Runtime.IGameServerService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var server = await svc.HeartbeatAsync(serverId, request);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> MarkReady(
        Guid serverId,
        Services.Runtime.IGameServerService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var server = await svc.MarkReadyAsync(serverId);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<ServerInfoResponse>.Ok(new ServerInfoResponse(
                server.Id, server.Ip, server.Port, "", server.RuntimeTokenExpiresAt!.Value)));
    }

    private static async Task<IResult> MarkStopped(
        Guid serverId,
        StoppedServerRequest request,
        Services.Runtime.IGameServerService svc,
        HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;

        var server = await svc.MarkStoppedAsync(serverId, request.ExitCode, request.CrashReason);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse.Ok());
    }
}
