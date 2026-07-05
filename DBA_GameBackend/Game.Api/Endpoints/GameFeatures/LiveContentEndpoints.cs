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
    private static void MapLiveContentEndpoints(IEndpointRouteBuilder app)
    {
        // 公告系统
        var announcement = app.MapGroup("/api/announcements").WithTags("公告");
        announcement.MapGet("/", GetAnnouncements)
            .WithSummary("获取公告列表")
            .WithDescription("获取当前有效的公告列表");

        // 活动系统
        var events = app.MapGroup("/api/events").WithTags("活动");
        events.MapGet("/", GetEvents)
            .WithSummary("获取活动列表")
            .WithDescription("获取当前有效的活动列表");
        events.MapGet("/me/progress", GetMyEventProgress)
            .WithSummary("获取我的活动进度")
            .WithDescription("获取当前玩家参与活动的进度")
            .RequireAuthorization();

        // 成就系统
        var achievement = app.MapGroup("/api/players/me/achievements").WithTags("成就");
        achievement.MapGet("/", GetAchievements)
            .WithSummary("获取成就列表")
            .WithDescription("获取当前玩家的成就列表")
            .RequireAuthorization();
    }

    // ==================== 公告系统 ====================

    private static async Task<IResult> GetAnnouncements(string? channel, GameDbContext db)
    {
        var now = DateTimeOffset.UtcNow;
        var query = db.Announcements
            .Where(x => x.IsActive && x.StartAt <= now && (x.EndAt == null || x.EndAt > now));

        if (!string.IsNullOrEmpty(channel))
            query = query.Where(x => x.Channel == channel);

        var announcements = await query
            .OrderByDescending(x => x.Priority)
            .ThenByDescending(x => x.CreatedAt)
            .Take(20)
            .Select(x => new AnnouncementDto(x.Id, x.Title, x.Content, x.Type, x.Priority, x.StartAt, x.EndAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AnnouncementListResponse>.Ok(new AnnouncementListResponse(announcements)));
    }

    // ==================== 活动系统 ====================

    private static async Task<IResult> GetEvents(GameDbContext db)
    {
        var now = DateTimeOffset.UtcNow;
        var events = await db.GameEvents
            .Where(x => x.Status == "ACTIVE" && x.StartAt <= now && (x.EndAt == null || x.EndAt > now))
            .OrderByDescending(x => x.CreatedAt)
            .Select(x => new GameEventDto(x.Id, x.EventKey, x.Title, x.Description, x.Type, x.Status, x.StartAt, x.EndAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<EventListResponse>.Ok(new EventListResponse(events)));
    }

    private static async Task<IResult> GetMyEventProgress(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var progress = await db.PlayerEventProgresses
            .Include(x => x.Event)
            .Where(x => x.PlayerId == playerId.Value)
            .Where(x => x.Event!.Status == "ACTIVE")
            .Select(x => new PlayerEventProgressDto(
                x.EventId, x.Event!.EventKey, x.Event.Title, x.Progress, x.Target, x.IsCompleted, x.IsRewarded))
            .ToListAsync();

        return Results.Ok(ApiResponse<object>.Ok(progress));
    }

    // ==================== 成就系统 ====================

    private static async Task<IResult> GetAchievements(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var achievements = await db.Achievements
            .OrderBy(x => x.Order)
            .Select(x => new AchievementDto(x.Id, x.AchievementKey, x.Title, x.Description, x.Category,
                x.Icon, 0, x.MaxProgress, false, null))
            .ToListAsync();

        var playerAchievements = await db.PlayerAchievements
            .Where(x => x.PlayerId == playerId.Value)
            .ToDictionaryAsync(x => x.AchievementId);

        var result = achievements.Select(x =>
        {
            if (playerAchievements.TryGetValue(x.Id, out var pa))
            {
                return x with { Progress = pa.Progress, IsUnlocked = pa.IsUnlocked, UnlockedAt = pa.UnlockedAt };
            }
            return x;
        }).ToList();

        return Results.Ok(ApiResponse<AchievementListResponse>.Ok(new AchievementListResponse(result)));
    }
}
