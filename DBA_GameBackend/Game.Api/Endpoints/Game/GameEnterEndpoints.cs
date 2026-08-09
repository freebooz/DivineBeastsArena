/*
中文阅读说明：
- 这是角色选择页唯一的“进入游戏”公网端点；它只接受业务意图，绝不接受客户端提供的服务器地址、DS 实例或 GameTicket。
- 端点复用既有认证 claim、会话分配与票据签发服务，以适配层完成 /api/v1 新契约迁移。
*/

using Game.Api.Services.Session;
using Game.Shared.Common;
using Game.Shared.Contracts.Session;
using Game.Shared.Errors;

namespace Game.Api.Endpoints.Game;

public static class GameEnterEndpoints
{
    public static void MapGameEnterEndpoints(this IEndpointRouteBuilder app)
    {
        app.MapPost("/api/v1/game/enter", EnterAsync)
            .WithTags("游戏会话")
            .WithSummary("申请进入当前区服游戏世界")
            .WithDescription("验证角色归属与区服后分配开发 Dedicated Server，并在就绪时签发短生命周期一次性 GameTicket。")
            .RequireAuthorization();
    }

    private static async Task<IResult> EnterAsync(
        GameEnterRequest request,
        IGameEnterService service,
        HttpContext context,
        CancellationToken cancellationToken)
    {
        var accountClaim = context.User.FindFirst("player_id");
        if (accountClaim is null || !Guid.TryParse(accountClaim.Value, out var accountId))
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var result = await service.EnterAsync(accountId, request, cancellationToken);
        return result.Disposition switch
        {
            GameEnterDisposition.Ready or GameEnterDisposition.Pending => Results.Ok(ApiResponse<GameEnterResponse>.Ok(result.Response!)),
            _ => ErrorResponse.BadRequest(result.ErrorMessage ?? "进入游戏请求无效。").ToProblem()
        };
    }
}
