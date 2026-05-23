/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 GameFeatures 相关 HTTP 接口路由、鉴权要求、请求解析和统一响应。
- 阅读重点：每个 partial 文件对应一个功能域；总入口只负责聚合注册。
- 修改提示：新增功能时优先放到对应领域文件，避免 GameFeatureEndpoints 再次膨胀。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Api.Extensions;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;

namespace Game.Api.Endpoints.GameFeatures;
public static partial class GameFeatureEndpoints
{
    private static void MapRankingEndpoints(IEndpointRouteBuilder app)
    {
        // 排行榜
        var ranking = app.MapGroup("/api/rankings").WithTags("排行榜");
        ranking.MapGet("/{mode}", GetRanking)
            .WithSummary("获取排行榜")
            .WithDescription("获取指定模式的排行榜");
        ranking.MapGet("/{mode}/player/{playerId}", GetPlayerRank)
            .WithSummary("获取玩家排名")
            .WithDescription("获取指定玩家在排行榜中的位置");
    }

    // ==================== 排行榜 ====================

    private static async Task<IResult> GetRanking(string mode, int page, int pageSize, GameDbContext db)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var rankings = await db.PlayerRankings
            .Where(x => x.Mode == mode)
            .OrderBy(x => x.Rank)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Include(x => x.Player)
            .Select(x => new RankingEntry(
                x.Rank,
                x.PlayerId,
                x.Player!.Nickname,
                x.Rating,
                x.TotalMatches,
                x.Wins,
                x.TotalMatches > 0 ? x.Wins * 100 / x.TotalMatches : 0))
            .ToListAsync();

        return Results.Ok(ApiResponse<RankingResponse>.Ok(new RankingResponse(mode, rankings)));
    }

    private static async Task<IResult> GetPlayerRank(string mode, Guid playerId, GameDbContext db)
    {
        var ranking = await db.PlayerRankings
            .Where(x => x.Mode == mode && x.PlayerId == playerId)
            .Include(x => x.Player)
            .FirstOrDefaultAsync();

        if (ranking == null)
            return ErrorResponse.NotFound("未找到排名").ToProblem();

        return Results.Ok(ApiResponse<PlayerRankResponse>.Ok(new PlayerRankResponse(
            playerId, ranking.Player!.Nickname, ranking.Rank, ranking.Rating)));
    }
}
