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
    private static void MapCommerceEndpoints(IEndpointRouteBuilder app)
    {
        // 商城系统
        var shop = app.MapGroup("/api/shop").WithTags("商城");
        shop.MapGet("/items", GetShopItems)
            .WithSummary("获取商品列表")
            .WithDescription("获取商城商品列表");
        shop.MapPost("/purchase", PurchaseItem)
            .WithSummary("购买商品")
            .WithDescription("购买商城商品")
            .RequireAuthorization();
    }

    // ==================== 商城系统 ====================

    private static async Task<IResult> GetShopItems(string? category, GameDbContext db)
    {
        // 模拟商城数据 - 实际应从数据库读取
        var items = new List<ShopItemDto>
        {
            new("skin_001", "蓝色皮肤", "蓝色主题皮肤", null, 100, "COIN", "SKIN", false, 0, null),
            new("weapon_001", "新手武器", "新手武器礼包", null, 50, "COIN", "WEAPON", false, 0, null),
            new("gem_001", "宝石礼包", "宝石礼包月卡", null, 300, "USD", "PACKAGE", false, 100, null),
        };

        if (!string.IsNullOrEmpty(category))
            items = items.Where(x => x.Category == category).ToList();

        return Results.Ok(ApiResponse<ShopItemsResponse>.Ok(new ShopItemsResponse(items)));
    }

    private static async Task<IResult> PurchaseItem(PurchaseRequest request, Guid playerId, GameDbContext db)
    {
        // 简化实现 - 实际需要支付网关集成
        return ErrorResponse.BadRequest("支付功能开发中").ToProblem();
    }
}
