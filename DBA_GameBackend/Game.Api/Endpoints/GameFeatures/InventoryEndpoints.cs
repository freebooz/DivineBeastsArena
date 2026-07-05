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
    private static void MapInventoryEndpoints(IEndpointRouteBuilder app)
    {
        // 背包/物品
        var inventory = app.MapGroup("/api/players/me/inventory").WithTags("背包");
        inventory.MapGet("/", GetInventory)
            .WithSummary("获取玩家背包")
            .WithDescription("获取当前玩家的所有物品")
            .RequireAuthorization();
        inventory.MapGet("/unlocks", GetUnlocks)
            .WithSummary("获取玩家已解锁内容")
            .WithDescription("获取玩家已解锁的皮肤、道具等")
            .RequireAuthorization();

        // 管理员背包接口
        var adminInventory = app.MapGroup("/api/admin/inventory")
            .WithTags("背包(管理员)")
            .RequireRateLimiting("admin")
            .RequireAuthorization();
        adminInventory.MapPost("/grant", GrantItem)
            .WithSummary("发放物品")
            .WithDescription("管理员发放物品给玩家")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        adminInventory.MapPost("/deduct", DeductItem)
            .WithSummary("扣除物品")
            .WithDescription("管理员扣除玩家物品")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Ops);
        adminInventory.MapGet("/logs", GetInventoryLogs)
            .WithSummary("获取物品日志")
            .WithDescription("查看物品发放/扣除记录")
            .RequireAdminRoles(AdminRoleEndpointExtensions.Viewer, AdminRoleEndpointExtensions.Support, AdminRoleEndpointExtensions.Ops);
    }

    // ==================== 背包/物品 ====================

    private static async Task<IResult> GetInventory(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var items = await db.InventoryItems
            .Where(x => x.PlayerId == playerId.Value && x.Quantity > 0)
            .Select(x => new InventoryItemDto(x.ItemId, x.Quantity, x.ExpiresAt))
            .ToListAsync();
        return Results.Ok(ApiResponse<InventoryResponse>.Ok(new InventoryResponse(playerId.Value, items)));
    }

    private static async Task<IResult> GetUnlocks(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var unlocks = await db.PlayerUnlocks
            .Where(x => x.PlayerId == playerId.Value)
            .OrderByDescending(x => x.CreatedAt)
            .Select(x => new { x.UnlockType, x.UnlockId, x.Source, x.CreatedAt })
            .ToListAsync();
        var dtos = unlocks.Select(x => new { x.UnlockType, x.UnlockId, x.Source, x.CreatedAt }).ToList();
        return Results.Ok(ApiResponse<object>.Ok(dtos));
    }

    private static async Task<IResult> GrantItem(GrantItemRequest request, GameDbContext db, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        if (!adminId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        if (string.IsNullOrWhiteSpace(request.Reason))
            return ErrorResponse.BadRequest("高危操作必须填写 reason").ToProblem();
        if (request.Quantity <= 0)
            return ErrorResponse.BadRequest("发放数量必须大于 0").ToProblem();

        var item = await db.InventoryItems
            .FirstOrDefaultAsync(x => x.PlayerId == request.PlayerId && x.ItemId == request.ItemId);

        if (item == null)
        {
            item = new InventoryItem { PlayerId = request.PlayerId, ItemId = request.ItemId, Quantity = request.Quantity };
            db.InventoryItems.Add(item);
        }
        else
        {
            item.Quantity += request.Quantity;
            item.UpdatedAt = DateTimeOffset.UtcNow;
        }

        db.InventoryLogs.Add(new InventoryLog
        {
            PlayerId = request.PlayerId,
            ItemId = request.ItemId,
            QuantityDelta = request.Quantity,
            QuantityBefore = item.Quantity - request.Quantity,
            QuantityAfter = item.Quantity,
            Reason = request.Reason,
            BizType = "GM_GRANT",
            OperatorId = adminId.Value
        });
        AddAdminAuditLog(db, adminId.Value, "ADMIN_INVENTORY_GRANT", "InventoryItem", $"{request.PlayerId}:{request.ItemId}", request.Reason, ctx);

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> DeductItem(DeductItemRequest request, GameDbContext db, HttpContext ctx)
    {
        var adminId = GetAdminId(ctx);
        if (!adminId.HasValue) return ErrorResponse.Unauthorized().ToProblem();
        if (string.IsNullOrWhiteSpace(request.Reason))
            return ErrorResponse.BadRequest("高危操作必须填写 reason").ToProblem();
        if (request.Quantity <= 0)
            return ErrorResponse.BadRequest("扣除数量必须大于 0").ToProblem();

        var item = await db.InventoryItems
            .FirstOrDefaultAsync(x => x.PlayerId == request.PlayerId && x.ItemId == request.ItemId);

        if (item == null || item.Quantity < request.Quantity)
            return ErrorResponse.BadRequest("物品数量不足").ToProblem();

        item.Quantity -= request.Quantity;
        item.UpdatedAt = DateTimeOffset.UtcNow;

        db.InventoryLogs.Add(new InventoryLog
        {
            PlayerId = request.PlayerId,
            ItemId = request.ItemId,
            QuantityDelta = -request.Quantity,
            QuantityBefore = item.Quantity + request.Quantity,
            QuantityAfter = item.Quantity,
            Reason = request.Reason,
            BizType = "GM_DEDUCT",
            OperatorId = adminId.Value
        });
        AddAdminAuditLog(db, adminId.Value, "ADMIN_INVENTORY_DEDUCT", "InventoryItem", $"{request.PlayerId}:{request.ItemId}", request.Reason, ctx);

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> GetInventoryLogs(GameDbContext db, int page = 1, int pageSize = 50)
    {
        (page, pageSize) = NormalizePaging(page, pageSize);

        var logs = await db.InventoryLogs
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new { x.Id, x.PlayerId, x.ItemId, x.QuantityDelta, x.Reason, x.CreatedAt })
            .ToListAsync();
        return Results.Ok(ApiResponse<object>.Ok(logs));
    }
}
