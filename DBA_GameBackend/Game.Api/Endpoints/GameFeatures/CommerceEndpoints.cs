/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义商城商品查询和 mock 购买接口。
- 阅读重点：商品目录为开发环境 mock 数据，购买接口会写入订单、钱包流水、背包和背包日志。
- 修改提示：接入真实支付后保留 DTO 契约，将支付确认逻辑下沉到服务层。
*/

using System.Text.Json;
using Game.Api.Extensions;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Common;
using Game.Shared.Contracts.GameFeatures;
using Game.Shared.Errors;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Endpoints.GameFeatures;

public static partial class GameFeatureEndpoints
{
    private static void MapCommerceEndpoints(IEndpointRouteBuilder app)
    {
        var shop = app.MapGroup("/api/shop").WithTags("商城");
        shop.MapGet("/items", GetShopItems)
            .WithSummary("获取商品列表")
            .WithDescription("获取商城商品列表");
        shop.MapPost("/purchase", PurchaseItem)
            .WithSummary("购买商品")
            .WithDescription("使用 mock 钱包或 mock 外部支付购买商品")
            .RequireAuthorization();
    }

    private static async Task<IResult> GetShopItems(string? category, GameDbContext db)
    {
        await Task.CompletedTask;
        var items = GetMockShopCatalog();

        if (!string.IsNullOrWhiteSpace(category))
            items = items.Where(x => string.Equals(x.Category, category, StringComparison.OrdinalIgnoreCase)).ToList();

        return Results.Ok(ApiResponse<ShopItemsResponse>.Ok(new ShopItemsResponse(items)));
    }

    private static async Task<IResult> PurchaseItem(PurchaseRequest request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        if (request.Quantity <= 0)
            return ErrorResponse.BadRequest("Quantity must be greater than zero.").ToProblem();

        var shopItem = GetMockShopCatalog()
            .FirstOrDefault(x => string.Equals(x.ItemId, request.ItemId, StringComparison.OrdinalIgnoreCase));
        if (shopItem == null)
            return ErrorResponse.NotFound("Shop item not found.").ToProblem();

        if (shopItem.Stock > 0 && request.Quantity > shopItem.Stock)
            return ErrorResponse.BadRequest("Not enough mock stock.").ToProblem();

        var now = DateTimeOffset.UtcNow;
        var totalAmount = shopItem.Price * request.Quantity;
        var order = new OrderRecord
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId.Value,
            Platform = string.IsNullOrWhiteSpace(request.PaymentMethod) ? "MOCK" : request.PaymentMethod.ToUpperInvariant(),
            PlatformOrderId = $"mock-{Guid.NewGuid():N}",
            Status = "COMPLETED",
            Amount = totalAmount,
            Currency = shopItem.Currency,
            ItemJson = JsonSerializer.Serialize(new
            {
                itemId = shopItem.ItemId,
                shopItem.Name,
                quantity = request.Quantity,
                unitPrice = shopItem.Price
            }),
            CreatedAt = now,
            PaidAt = now,
            CompletedAt = now,
            UpdatedAt = now
        };

        if (UsesWallet(shopItem.Currency))
        {
            var balance = await db.WalletBalances
                .FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.CurrencyType == shopItem.Currency);
            if (balance == null || balance.Balance < totalAmount)
                return ErrorResponse.BadRequest("Insufficient mock wallet balance.").ToProblem();

            var before = balance.Balance;
            balance.Balance -= totalAmount;
            balance.UpdatedAt = now;
            db.WalletLedgers.Add(new WalletLedger
            {
                Id = Guid.NewGuid(),
                PlayerId = playerId.Value,
                CurrencyType = shopItem.Currency,
                Amount = -totalAmount,
                BalanceBefore = before,
                BalanceAfter = balance.Balance,
                BizType = "SHOP_PURCHASE",
                BizId = order.Id.ToString(),
                IdempotencyKey = $"shop-purchase-{order.Id:N}",
                CreatedAt = now
            });
        }

        var inventoryItem = await db.InventoryItems
            .FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.ItemId == shopItem.ItemId);
        var quantityBefore = inventoryItem?.Quantity ?? 0;

        if (inventoryItem == null)
        {
            inventoryItem = new InventoryItem
            {
                Id = Guid.NewGuid(),
                PlayerId = playerId.Value,
                ItemId = shopItem.ItemId,
                Quantity = request.Quantity,
                CreatedAt = now,
                UpdatedAt = now
            };
            db.InventoryItems.Add(inventoryItem);
        }
        else
        {
            inventoryItem.Quantity += request.Quantity;
            inventoryItem.UpdatedAt = now;
        }

        db.InventoryLogs.Add(new InventoryLog
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId.Value,
            ItemId = shopItem.ItemId,
            QuantityDelta = request.Quantity,
            QuantityBefore = quantityBefore,
            QuantityAfter = inventoryItem.Quantity,
            Reason = "mock shop purchase",
            BizType = "SHOP_PURCHASE",
            BizId = order.Id.ToString(),
            CreatedAt = now
        });

        db.OrderRecords.Add(order);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new
        {
            orderId = order.Id,
            order.Status,
            itemId = shopItem.ItemId,
            request.Quantity,
            totalAmount,
            currency = shopItem.Currency
        }));
    }

    private static bool UsesWallet(string currency)
    {
        return string.Equals(currency, "COIN", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(currency, "GEM", StringComparison.OrdinalIgnoreCase);
    }

    private static List<ShopItemDto> GetMockShopCatalog()
    {
        return new List<ShopItemDto>
        {
            new("skin_001", "Blue Arena Skin", "Blue themed character skin for UI and inventory API testing.", null, 100, "COIN", "SKIN", false, 0, null),
            new("weapon_001", "Starter Weapon Pack", "Starter weapon bundle used by mock purchase flows.", null, 50, "COIN", "WEAPON", false, 0, null),
            new("gem_001", "Gem Monthly Pack", "Mock external payment package for payment and order API testing.", null, 300, "USD", "PACKAGE", false, 100, null),
            new("event_ticket", "Event Ticket", "Limited event entry ticket with finite mock stock.", null, 25, "GEM", "EVENT", true, 20, DateTimeOffset.UtcNow.AddDays(14)),
        };
    }
}
