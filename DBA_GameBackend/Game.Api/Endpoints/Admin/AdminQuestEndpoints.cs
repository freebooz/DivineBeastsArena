/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义 Admin 端点中任务系统管理相关接口，包括任务列表、创建、更新和停用。
- 阅读重点：写操作均记录审计日志；任务停用通过 IsActive=false 软下线，不删除实体。
- 修改提示：任务奖励结构变化时同步更新 RewardJson 解析逻辑与客户端任务端点。
*/

using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.Admin;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.Admin;

public static partial class AdminEndpoints
{
    private static void MapAdminQuestEndpoints(RouteGroupBuilder admin)
    {
        admin.MapGet("/quests", ListQuests)
            .WithSummary("查询任务列表")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
        admin.MapPost("/quests", CreateQuest)
            .WithSummary("创建任务")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        admin.MapPut("/quests/{questId:guid}", UpdateQuest)
            .WithSummary("更新任务")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        admin.MapPost("/quests/{questId:guid}/deactivate", DeactivateQuest)
            .WithSummary("停用任务")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
    }

    // ==================== 查询任务列表 ====================

    private static async Task<IResult> ListQuests(GameDbContext db, int page = 1, int pageSize = 50)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var query = db.Quests.AsNoTracking()
            .OrderBy(x => x.SortOrder)
            .ThenBy(x => x.QuestKey);

        var total = await query.CountAsync();
        var items = await query
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new AdminQuestItem(
                x.Id,
                x.QuestKey,
                x.Title,
                x.Description,
                x.QuestType,
                x.Category,
                x.TargetProgress,
                x.RewardJson,
                x.SortOrder,
                x.IsActive,
                x.CreatedAt,
                x.UpdatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<AdminQuestListResponse>.Ok(
            new AdminQuestListResponse(items, total, page, pageSize)));
    }

    // ==================== 创建任务 ====================

    private static async Task<IResult> CreateQuest(
        AdminCreateQuestRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        var validation = ValidateQuestRequest(request);
        if (validation is not null)
        {
            return validation;
        }

        var questKey = request.QuestKey!.Trim();
        if (await db.Quests.AnyAsync(x => x.QuestKey == questKey, cancellationToken))
        {
            return ErrorResponse.Conflict($"任务键 {questKey} 已存在。").ToProblem();
        }

        var adminId = GetAdminId(ctx);
        var now = DateTimeOffset.UtcNow;
        var quest = new Quest
        {
            Id = Guid.NewGuid(),
            QuestKey = questKey,
            Title = request.Title!.Trim(),
            Description = request.Description?.Trim() ?? string.Empty,
            QuestType = request.QuestType!.Trim(),
            Category = request.Category?.Trim() ?? "GENERAL",
            TargetProgress = request.TargetProgress,
            RewardJson = string.IsNullOrWhiteSpace(request.RewardJson) ? "{}" : request.RewardJson.Trim(),
            SortOrder = request.SortOrder,
            IsActive = true,
            CreatedAt = now,
            UpdatedAt = now
        };

        db.Quests.Add(quest);

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_QUEST_CREATE",
            "Quest",
            quest.Id.ToString(),
            $"创建任务 {quest.QuestKey}",
            ctx);

        await db.SaveChangesAsync(cancellationToken);

        return Results.Ok(ApiResponse<AdminQuestItem>.Ok(MapQuestToItem(quest)));
    }

    // ==================== 更新任务 ====================

    private static async Task<IResult> UpdateQuest(
        Guid questId,
        AdminUpdateQuestRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        var validation = ValidateQuestRequest(request);
        if (validation is not null)
        {
            return validation;
        }

        var quest = await db.Quests.FirstOrDefaultAsync(x => x.Id == questId, cancellationToken);
        if (quest is null)
        {
            return ErrorResponse.NotFound("任务不存在。").ToProblem();
        }

        var questKey = request.QuestKey!.Trim();
        if (await db.Quests.AnyAsync(x => x.QuestKey == questKey && x.Id != questId, cancellationToken))
        {
            return ErrorResponse.Conflict($"任务键 {questKey} 已被其他任务占用。").ToProblem();
        }

        var adminId = GetAdminId(ctx);
        var now = DateTimeOffset.UtcNow;

        quest.QuestKey = questKey;
        quest.Title = request.Title!.Trim();
        quest.Description = request.Description?.Trim() ?? string.Empty;
        quest.QuestType = request.QuestType!.Trim();
        quest.Category = request.Category?.Trim() ?? "GENERAL";
        quest.TargetProgress = request.TargetProgress;
        quest.RewardJson = string.IsNullOrWhiteSpace(request.RewardJson) ? "{}" : request.RewardJson.Trim();
        quest.SortOrder = request.SortOrder;
        quest.UpdatedAt = now;

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_QUEST_UPDATE",
            "Quest",
            quest.Id.ToString(),
            $"更新任务 {quest.QuestKey}",
            ctx);

        await db.SaveChangesAsync(cancellationToken);

        return Results.Ok(ApiResponse<AdminQuestItem>.Ok(MapQuestToItem(quest)));
    }

    // ==================== 停用任务 ====================

    private static async Task<IResult> DeactivateQuest(
        Guid questId,
        AdminDeactivateQuestRequest request,
        GameDbContext db,
        HttpContext ctx,
        CancellationToken cancellationToken)
    {
        if (string.IsNullOrWhiteSpace(request.Reason))
        {
            return ErrorResponse.BadRequest("停用任务必须填写 reason。").ToProblem();
        }

        var quest = await db.Quests.FirstOrDefaultAsync(x => x.Id == questId, cancellationToken);
        if (quest is null)
        {
            return ErrorResponse.NotFound("任务不存在。").ToProblem();
        }

        var adminId = GetAdminId(ctx);
        var now = DateTimeOffset.UtcNow;

        quest.IsActive = false;
        quest.UpdatedAt = now;

        await AddAuditLogAsync(
            db,
            adminId,
            "ADMIN_QUEST_DEACTIVATE",
            "Quest",
            quest.Id.ToString(),
            request.Reason.Trim(),
            ctx);

        await db.SaveChangesAsync(cancellationToken);

        return Results.Ok(ApiResponse<AdminQuestItem>.Ok(MapQuestToItem(quest)));
    }

    // ==================== 辅助方法 ====================

    private static IResult? ValidateQuestRequest(AdminCreateQuestRequest request)
    {
        if (string.IsNullOrWhiteSpace(request.QuestKey))
        {
            return ErrorResponse.BadRequest("questKey 不能为空。").ToProblem();
        }
        if (string.IsNullOrWhiteSpace(request.Title))
        {
            return ErrorResponse.BadRequest("title 不能为空。").ToProblem();
        }
        if (string.IsNullOrWhiteSpace(request.QuestType))
        {
            return ErrorResponse.BadRequest("questType 不能为空。").ToProblem();
        }
        if (request.TargetProgress < 0)
        {
            return ErrorResponse.BadRequest("targetProgress 不能为负数。").ToProblem();
        }
        return null;
    }

    private static AdminQuestItem MapQuestToItem(Quest quest) => new(
        quest.Id,
        quest.QuestKey,
        quest.Title,
        quest.Description,
        quest.QuestType,
        quest.Category,
        quest.TargetProgress,
        quest.RewardJson,
        quest.SortOrder,
        quest.IsActive,
        quest.CreatedAt,
        quest.UpdatedAt);
}
