/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Feedback;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Feedback;

public static class FeedbackEndpoints
{
    public static void MapFeedbackEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/feedback").WithTags("Feedback");

        group.MapPost("/", SubmitFeedback)
            .WithSummary("Submit player or website feedback");

        group.MapGet("/recent", GetRecentFeedback)
            .WithSummary("Get recent feedback items for operations review")
            .RequireAuthorization();
    }

    private static async Task<IResult> SubmitFeedback(SubmitFeedbackRequest request, GameDbContext db)
    {
        if (string.IsNullOrWhiteSpace(request.Content) || request.Content.Trim().Length < 10)
        {
            return Results.BadRequest(ApiResponse.Fail("反馈内容至少需要 10 个字符。"));
        }

        var feedback = new PlayerFeedback
        {
            Nickname = NormalizeOptional(request.Nickname),
            Email = NormalizeOptional(request.Email),
            FeedbackType = string.IsNullOrWhiteSpace(request.FeedbackType) ? "GENERAL" : request.FeedbackType.Trim().ToUpperInvariant(),
            Title = NormalizeOptional(request.Title),
            Content = request.Content.Trim(),
            Status = "OPEN"
        };

        db.PlayerFeedbacks.Add(feedback);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<FeedbackResponse>.Ok(ToResponse(feedback)));
    }

    private static async Task<IResult> GetRecentFeedback(GameDbContext db, int page = 1, int pageSize = 50)
    {
        page = Math.Max(page, 1);
        pageSize = pageSize <= 0 ? 50 : Math.Clamp(pageSize, 1, 50);

        var feedback = await db.PlayerFeedbacks
            .AsNoTracking()
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => ToResponse(x))
            .ToListAsync();

        return Results.Ok(ApiResponse<IReadOnlyList<FeedbackResponse>>.Ok(feedback));
    }

    private static string? NormalizeOptional(string? value)
    {
        return string.IsNullOrWhiteSpace(value) ? null : value.Trim();
    }

    private static FeedbackResponse ToResponse(PlayerFeedback feedback)
    {
        return new FeedbackResponse(
            feedback.Id,
            feedback.FeedbackType,
            feedback.Title ?? string.Empty,
            feedback.Content,
            feedback.Status,
            feedback.CreatedAt);
    }
}
