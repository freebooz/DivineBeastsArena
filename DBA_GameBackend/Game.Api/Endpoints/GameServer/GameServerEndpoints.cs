/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.GameServer;
using Game.Shared.Common;
using Game.Shared.Errors;
using Game.Api.Extensions;
using Game.Api.Services.GameServer;
using Game.ServerManagement.DedicatedServers;

namespace Game.Api.Endpoints.GameServer;

/// <summary>
/// 游戏服务器注册相关接口 / Game Server Registration APIs
/// </summary>
public static class GameServerEndpoints
{
    /// <summary>
    /// 注册游戏服务器端点 / Register game server endpoints
    /// </summary>
    public static void MapGameServerEndpoints(this IEndpointRouteBuilder app)
    {
        var managerGroup = app.MapGroup("/internal/game-servers").WithTags("Dedicated Server Orchestration");
        managerGroup.AddEndpointFilter(RequireInternalApiKey);
        managerGroup.MapPost("/allocate", AllocateManagedServer);
        managerGroup.MapPost("/{serverId:guid}/release", ReleaseManagedServer);
        managerGroup.MapGet("/", ListManagedServers);
        managerGroup.MapGet("/{serverId:guid}", GetManagedServer);
        managerGroup.MapPost("/{serverId:guid}/kill", KillManagedServer);

        var group = app.MapGroup("/internal/servers").WithTags("游戏服务器(内部)");

        group.MapPost("/register", Register)
            .WithSummary("注册游戏服务器")
            .WithDescription(@"
游戏服务器启动时调用此接口注册到管理系统。
注册后会获得一个服务器ID，用于后续的心跳和状态更新。

**请求示例：**
```json
{
  ""ip"": ""192.168.1.100"",
  ""port"": 7777,
  ""mode"": ""classic"",
  ""mapId"": ""map_001""
}
```

**响应示例：**
```json
{
  ""success"": true,
  ""data"": {
    ""serverId"": ""uuid"",
    ""sessionId"": null,
    ""mode"": ""classic"",
    ""mapId"": ""map_001"",
    ""ip"": ""192.168.1.100"",
    ""port"": 7777,
    ""status"": ""idle"",
    ""startedAt"": ""2024-01-01T00:00:00Z"",
    ""lastHeartbeatAt"": ""2024-01-01T00:00:00Z""
  }
}
```
");

        group.MapGet("/{serverId}", GetServer)
            .WithSummary("获取游戏服务器详情")
            .WithDescription(@"
根据服务器ID获取游戏服务器的详细信息。

**调用方式：**
GET /internal/servers/{serverId}
");

        group.MapGet("/active", GetActiveServers)
            .WithSummary("获取所有活跃服务器")
            .WithDescription(@"
获取当前所有活跃的游戏服务器列表。
用于负载均衡和服务器选择。

**调用方式：**
GET /internal/servers/active
");
    }

    private static async ValueTask<object?> RequireInternalApiKey(EndpointFilterInvocationContext context, EndpointFilterDelegate next)
    {
        var configuration = context.HttpContext.RequestServices.GetRequiredService<IConfiguration>();
        var expected = configuration["InternalApi:Key"];
        var actual = context.HttpContext.Request.Headers["X-Internal-Api-Key"].ToString();
        if (string.IsNullOrWhiteSpace(expected) || !string.Equals(expected, actual, StringComparison.Ordinal))
        {
            return ErrorResponse.Unauthorized("Invalid internal api key").ToProblem();
        }

        return await next(context);
    }

    private static async Task<IResult> AllocateManagedServer(
        AllocateManagedServerRequest request,
        IDedicatedServerOrchestrator manager,
        CancellationToken cancellationToken)
    {
        var server = await manager.AllocateAsync(new AllocateDedicatedServerCommand(
            request.SessionId,
            request.Mode,
            request.MapId,
            request.Region,
            request.BuildVersion), cancellationToken);

        return server is null
            ? ErrorResponse.Conflict("No available game server capacity or UDP port").ToProblem()
            : Results.Ok(ApiResponse<DedicatedServerInstanceDto>.Ok(server));
    }

    private static async Task<IResult> ReleaseManagedServer(
        Guid serverId,
        DedicatedServerReasonRequest? request,
        IDedicatedServerOrchestrator manager,
        CancellationToken cancellationToken)
    {
        var released = await manager.ReleaseAsync(serverId, request?.Reason ?? "internal release", cancellationToken);
        return released ? Results.Ok(ApiResponse.Ok()) : ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem();
    }

    private static async Task<IResult> ListManagedServers(IDedicatedServerOrchestrator manager, CancellationToken cancellationToken)
    {
        var servers = await manager.ListAsync(cancellationToken);
        return Results.Ok(ApiResponse<IReadOnlyList<DedicatedServerInstanceDto>>.Ok(servers));
    }

    private static async Task<IResult> GetManagedServer(Guid serverId, IDedicatedServerOrchestrator manager, CancellationToken cancellationToken)
    {
        var server = await manager.GetAsync(serverId, cancellationToken);
        return server is null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<DedicatedServerInstanceDto>.Ok(server));
    }

    private static async Task<IResult> KillManagedServer(
        Guid serverId,
        DedicatedServerReasonRequest? request,
        IDedicatedServerOrchestrator manager,
        CancellationToken cancellationToken)
    {
        var killed = await manager.KillAsync(serverId, request?.Reason ?? "internal kill", cancellationToken);
        return killed ? Results.Ok(ApiResponse.Ok()) : ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem();
    }

    private static async Task<IResult> Register(RegisterGameServerRequest request, IGameServerRegistryService svc)
    {
        var server = await svc.RegisterServerAsync(request);
        return server == null
            ? ErrorResponse.Conflict("Server already registered").ToProblem()
            : Results.Ok(ApiResponse<InternalGameServerResponse>.Ok(new InternalGameServerResponse(
                server.Id, server.SessionId, server.Mode, server.MapId, server.Ip, server.Port,
                server.Status, server.StartedAt, server.LastHeartbeatAt)));
    }

    private static async Task<IResult> GetServer(Guid serverId, IGameServerRegistryService svc)
    {
        var server = await svc.GetServerAsync(serverId);
        return server == null
            ? ErrorResponse.NotFound(ErrorCodes.GameServerNotFound).ToProblem()
            : Results.Ok(ApiResponse<InternalGameServerResponse>.Ok(new InternalGameServerResponse(
                server.Id, server.SessionId, server.Mode, server.MapId, server.Ip, server.Port,
                server.Status, server.StartedAt, server.LastHeartbeatAt)));
    }

    private static async Task<IResult> GetActiveServers(IGameServerRegistryService svc)
    {
        var servers = await svc.GetActiveServersAsync();
        var responses = servers.Select(s => new InternalGameServerResponse(
            s.Id, s.SessionId, s.Mode, s.MapId, s.Ip, s.Port,
            s.Status, s.StartedAt, s.LastHeartbeatAt)).ToList();
        return Results.Ok(ApiResponse<IReadOnlyList<InternalGameServerResponse>>.Ok(responses));
    }
}

public record AllocateManagedServerRequest(Guid SessionId, string Mode, string MapId, string Region, string? BuildVersion);

public record DedicatedServerReasonRequest(string? Reason);
