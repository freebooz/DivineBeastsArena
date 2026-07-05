/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Text.Json;

namespace Game.Api.Services.Inventory;

public interface IInventoryService
{
    Task<IReadOnlyList<InventoryItem>> GetPlayerItemsAsync(Guid playerId);
    Task<InventoryItem?> GetItemAsync(Guid playerId, string itemId);
    Task<InventoryItem> AddItemAsync(Guid playerId, string itemId, long quantity, DateTimeOffset? expiresAt, string reason, string? bizType = null, string? bizId = null);
    Task<bool> ConsumeItemAsync(Guid playerId, string itemId, long quantity, string reason, string? bizType = null, string? bizId = null);
    Task GrantRewardsAsync(Guid playerId, IReadOnlyDictionary<string, object> rewards, string bizType, string bizId);
}

public sealed class InventoryService : IInventoryService
{
    private readonly GameDbContext _db;
    private readonly ILogger<InventoryService> _logger;

    public InventoryService(GameDbContext db, ILogger<InventoryService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<IReadOnlyList<InventoryItem>> GetPlayerItemsAsync(Guid playerId)
    {
        return await _db.InventoryItems
            .Where(x => x.PlayerId == playerId && x.Quantity > 0 && (x.ExpiresAt == null || x.ExpiresAt > DateTimeOffset.UtcNow))
            .ToListAsync();
    }

    public async Task<InventoryItem?> GetItemAsync(Guid playerId, string itemId)
    {
        return await _db.InventoryItems
            .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.ItemId == itemId);
    }

    public async Task<InventoryItem> AddItemAsync(Guid playerId, string itemId, long quantity, DateTimeOffset? expiresAt, string reason, string? bizType = null, string? bizId = null)
    {
        if (!string.IsNullOrWhiteSpace(bizType) && !string.IsNullOrWhiteSpace(bizId))
        {
            var duplicateLog = await _db.InventoryLogs
                .FirstOrDefaultAsync(x =>
                    x.PlayerId == playerId &&
                    x.ItemId == itemId &&
                    x.BizType == bizType &&
                    x.BizId == bizId &&
                    x.QuantityDelta > 0);
            if (duplicateLog != null)
            {
                var current = await GetItemAsync(playerId, itemId);
                if (current != null) return current;
            }
        }

        var existing = await _db.InventoryItems
            .FirstOrDefaultAsync(x =>
                x.PlayerId == playerId &&
                x.ItemId == itemId &&
                (x.ExpiresAt == null || x.ExpiresAt > DateTimeOffset.UtcNow));

        if (existing != null)
        {
            var before = existing.Quantity;
            existing.Quantity += quantity;
            existing.UpdatedAt = DateTimeOffset.UtcNow;

            _db.InventoryLogs.Add(new InventoryLog
            {
                Id = Guid.NewGuid(),
                PlayerId = playerId,
                ItemId = itemId,
                QuantityDelta = quantity,
                QuantityBefore = before,
                QuantityAfter = existing.Quantity,
                Reason = reason,
                BizType = bizType,
                BizId = bizId,
                CreatedAt = DateTimeOffset.UtcNow
            });

            await _db.SaveChangesAsync();
            return existing;
        }

        var item = new InventoryItem
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            ItemId = itemId,
            Quantity = quantity,
            ExpiresAt = expiresAt,
            CreatedAt = DateTimeOffset.UtcNow,
            UpdatedAt = DateTimeOffset.UtcNow
        };

        _db.InventoryItems.Add(item);

        _db.InventoryLogs.Add(new InventoryLog
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            ItemId = itemId,
            QuantityDelta = quantity,
            QuantityBefore = 0,
            QuantityAfter = quantity,
            Reason = reason,
            BizType = bizType,
            BizId = bizId,
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return item;
    }

    public async Task<bool> ConsumeItemAsync(Guid playerId, string itemId, long quantity, string reason, string? bizType = null, string? bizId = null)
    {
        var item = await _db.InventoryItems
            .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.ItemId == itemId && x.Quantity >= quantity);

        if (item == null) return false;

        var before = item.Quantity;
        item.Quantity -= quantity;
        item.UpdatedAt = DateTimeOffset.UtcNow;

        _db.InventoryLogs.Add(new InventoryLog
        {
            Id = Guid.NewGuid(),
            PlayerId = playerId,
            ItemId = itemId,
            QuantityDelta = -quantity,
            QuantityBefore = before,
            QuantityAfter = item.Quantity,
            Reason = reason,
            BizType = bizType,
            BizId = bizId,
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return true;
    }

    public async Task GrantRewardsAsync(Guid playerId, IReadOnlyDictionary<string, object> rewards, string bizType, string bizId)
    {
        foreach (var reward in rewards)
        {
            var quantity = TryGetRewardQuantity(reward.Value);
            if (quantity.HasValue)
            {
                await AddItemAsync(playerId, reward.Key, quantity.Value, null, "REWARD", bizType, bizId);
            }
        }
    }

    private static long? TryGetRewardQuantity(object value)
    {
        return value switch
        {
            long longQty => longQty,
            int intQty => intQty,
            double dblQty => (long)dblQty,
            decimal decimalQty => (long)decimalQty,
            JsonElement { ValueKind: JsonValueKind.Number } element when element.TryGetInt64(out var longQty) => longQty,
            JsonElement { ValueKind: JsonValueKind.Number } element when element.TryGetDouble(out var doubleQty) => (long)doubleQty,
            _ => null
        };
    }
}
