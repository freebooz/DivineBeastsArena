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
    private static void MapVersionAnalyticsEndpoints(IEndpointRouteBuilder app)
    {
        // 版本检测
        var version = app.MapGroup("/api/version").WithTags("版本");
        version.MapGet("/check", CheckVersion)
            .WithSummary("检查版本更新")
            .WithDescription("检查客户端版本是否有更新");

        // 运营统计
        var analytics = app.MapGroup("/api/admin/analytics")
            .WithTags("运营统计(管理员)")
            .RequireAuthorization();
        analytics.MapGet("/overview", GetOverviewStats)
            .WithSummary("数据概览")
            .WithDescription("获取运营数据概览")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Ops);
        analytics.MapGet("/retention", GetRetentionStats)
            .WithSummary("留存分析")
            .WithDescription("获取用户留存数据分析")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Ops);
    }

    // ==================== 版本检测 ====================

    private static async Task<IResult> CheckVersion(string? channel, GameDbContext db)
    {
        channel ??= "stable";

        var version = await db.ClientVersions
            .Where(x => x.Channel == channel && x.IsActive)
            .OrderByDescending(x => x.CreatedAt)
            .FirstOrDefaultAsync();

        if (version == null)
            return ErrorResponse.NotFound("未找到版本信息").ToProblem();

        return Results.Ok(ApiResponse<VersionCheckResponse>.Ok(new VersionCheckResponse(
            version.Version, version.Channel, version.DownloadUrl, version.IsMandatory, version.ReleaseNotes)));
    }

    // ==================== 运营统计 ====================

    private static async Task<IResult> GetOverviewStats(GameDbContext db)
    {
        var today = DateTimeOffset.UtcNow.Date;
        var stats = await db.DailyStats.FirstOrDefaultAsync(x => x.Date == today);

        if (stats == null)
        {
            stats = new DailyStats { Date = today, NewUsers = 0, ActiveUsers = 0, TotalMatches = 0 };
        }

        return Results.Ok(ApiResponse<OverviewStatsResponse>.Ok(new OverviewStatsResponse(
            1000, stats.ActiveUsers, 5000, stats.TotalMatches, stats.TotalPlayTimeSeconds, stats.TotalRevenue)));
    }

    private static async Task<IResult> GetRetentionStats(int days, GameDbContext db)
    {
        var cohort = await db.RetentionCohorts
            .OrderByDescending(x => x.CohortDate)
            .FirstOrDefaultAsync();

        if (cohort == null)
        {
            cohort = new RetentionCohort { CohortDate = DateTimeOffset.UtcNow.AddDays(-30) };
        }

        return Results.Ok(ApiResponse<RetentionStatsResponse>.Ok(new RetentionStatsResponse(
            cohort.CohortDate, cohort.D0, cohort.D1, cohort.D3, cohort.D7, cohort.D14, cohort.D30,
            new List<DailyRetentionPoint>())));
    }
}
