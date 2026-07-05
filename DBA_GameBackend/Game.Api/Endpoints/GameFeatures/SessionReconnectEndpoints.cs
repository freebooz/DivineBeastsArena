/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 GameFeatures 相关 HTTP 接口路由、鉴权要求、请求解析和统一响应。
- 阅读重点：每个 partial 文件对应一个功能域；总入口只负责聚合注册。
- 修改提示：新增功能时优先放到对应领域文件，避免 GameFeatureEndpoints 再次膨胀。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Infrastructure.Auth;
using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Api.Extensions;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;

namespace Game.Api.Endpoints.GameFeatures;
public static partial class GameFeatureEndpoints
{
    private static void MapSessionReconnectEndpoints(IEndpointRouteBuilder app)
    {
        // 断线重连
        var reconnect = app.MapGroup("/api/sessions").WithTags("断线重连");
        reconnect.MapPost("/{sessionId}/reconnect", Reconnect)
            .WithSummary("断线重连")
            .WithDescription("使用重连令牌重新连接游戏会话")
            .RequireAuthorization();
    }

    // ==================== 断线重连 ====================

    private static async Task<IResult> Reconnect(
        Guid sessionId,
        [FromBody] ReconnectRequest request,
        HttpContext ctx,
        GameDbContext db,
        [FromServices] IJwtTokenService jwt)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var playerSession = await db.PlayerSessions
            .Include(x => x.GameSession)
            .FirstOrDefaultAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId.Value);

        if (playerSession == null)
            return ErrorResponse.NotFound("未找到会话").ToProblem();

        var reconnectTokenExpiresAt = playerSession.ReconnectTokenExpiresAt;
        if (string.IsNullOrWhiteSpace(playerSession.ReconnectTokenHash) ||
            !reconnectTokenExpiresAt.HasValue ||
            reconnectTokenExpiresAt.Value < DateTimeOffset.UtcNow)
            return ErrorResponse.Unauthorized("重连令牌已过期").ToProblem();

        if (string.IsNullOrWhiteSpace(request.ReconnectToken) ||
            !FixedTimeEquals(playerSession.ReconnectTokenHash, jwt.HashToken(request.ReconnectToken)))
        {
            return ErrorResponse.Unauthorized("重连令牌无效").ToProblem();
        }

        return Results.Ok(ApiResponse<ReconnectResponse>.Ok(new ReconnectResponse(
            sessionId,
            playerSession.GameSession!.ServerIp ?? "",
            playerSession.GameSession.ServerPort ?? 0,
            "",
            reconnectTokenExpiresAt.Value)));
    }

    private static bool FixedTimeEquals(string left, string right)
    {
        var leftBytes = Encoding.UTF8.GetBytes(left);
        var rightBytes = Encoding.UTF8.GetBytes(right);
        return leftBytes.Length == rightBytes.Length &&
            CryptographicOperations.FixedTimeEquals(leftBytes, rightBytes);
    }
}
