using Game.Api.Services.ServerDirectory;
using Game.Shared.Common;
using Game.Shared.Contracts.GameServer;

namespace Game.Api.Endpoints.ServerDirectory;

public static class ServerDirectoryEndpoints
{
    public static void MapServerDirectoryEndpoints(this IEndpointRouteBuilder app)
    {
        app.MapGet("/api/v1/servers", GetServers)
            .WithTags("区服目录 v1")
            .WithSummary("获取可展示的游戏区服目录")
            .WithDescription("按 region、clientVersion、platform 过滤；不返回 Dedicated Server 单局实例信息。");
    }

    private static async Task<IResult> GetServers(string? region, string? clientVersion, string? platform,
        IServerDirectoryService service, CancellationToken cancellationToken)
    {
        var servers = await service.GetServersAsync(new ServerDirectoryQuery(region, clientVersion, platform), cancellationToken);
        return Results.Ok(ApiResponse<IReadOnlyList<ServerDirectoryServerDto>>.Ok(servers));
    }
}
