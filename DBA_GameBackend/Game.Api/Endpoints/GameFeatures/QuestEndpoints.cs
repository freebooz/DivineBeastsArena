/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API。
- 文件职责：定义任务系统相关 HTTP 接口，包括任务列表、任务详情、接受任务和领取奖励。
- 阅读重点：种子数据在首次查询时自动写入；奖励发放走钱包或背包，并记录流水与日志。
- 修改提示：奖励结构变化时同步更新 RewardJson 解析逻辑；正式环境应将种子数据迁移到数据库配置。
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
    // 开发阶段种子任务，后续迁移到数据库配置
    private static readonly (string Key, string Title, string Desc, string Type, string Category, int Target, string Reward, int Order)[] SeedQuests = new[]
    {
        ("daily_play_1", "每日首胜", "完成1场对战", "DAILY", "MATCH", 1, """{"currency":"GEM","amount":10}""", 1),
        ("daily_play_3", "每日三场", "完成3场对战", "DAILY", "MATCH", 3, """{"currency":"GEM","amount":20}""", 2),
        ("daily_kill_10", "每日击杀", "累计击杀10次", "DAILY", "COMBAT", 10, """{"currency":"COIN","amount":100}""", 3),
        ("weekly_play_10", "每周十场", "完成10场对战", "WEEKLY", "MATCH", 10, """{"currency":"GEM","amount":50}""", 1),
        ("main_first_win", "首胜", "获得第一场胜利", "MAIN", "MATCH", 1, """{"itemId":"skin_001","quantity":1}""", 1),
    };

    private static void MapQuestEndpoints(IEndpointRouteBuilder app)
    {
        var quests = app.MapGroup("/api/quests").WithTags("任务");

        quests.MapGet("/", GetQuests)
            .WithSummary("获取任务列表")
            .WithDescription("获取当前有效的任务列表及玩家进度，支持按任务类型筛选")
            .RequireAuthorization();

        quests.MapGet("/{questId}", GetQuestDetail)
            .WithSummary("获取任务详情")
            .WithDescription("获取指定任务的详情及当前玩家的进度")
            .RequireAuthorization();

        quests.MapPost("/{questId}/accept", AcceptQuest)
            .WithSummary("接受任务")
            .WithDescription("接受指定任务，创建玩家任务进度记录")
            .RequireAuthorization();

        quests.MapPost("/{questId}/claim-reward", ClaimQuestReward)
            .WithSummary("领取任务奖励")
            .WithDescription("领取已完成任务的奖励，发放到背包或钱包，已领取则幂等返回")
            .RequireAuthorization();
    }

    // ==================== 获取任务列表 ====================

    private static async Task<IResult> GetQuests(HttpContext ctx, GameDbContext db, string? questType = null)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        // 数据库中不存在任务时，自动写入开发阶段种子数据
        if (!await db.Quests.AnyAsync())
        {
            await EnsureSeedQuestsAsync(db);
        }

        var query = db.Quests.Where(x => x.IsActive);

        if (!string.IsNullOrEmpty(questType))
        {
            query = query.Where(x => x.QuestType == questType);
        }

        var quests = await query
            .OrderBy(x => x.SortOrder)
            .ThenBy(x => x.QuestKey)
            .Select(x => new
            {
                x.Id,
                x.QuestKey,
                x.Title,
                x.Description,
                x.QuestType,
                x.Category,
                x.TargetProgress,
                x.RewardJson,
                x.SortOrder
            })
            .ToListAsync();

        var playerQuests = await db.PlayerQuests
            .Where(x => x.PlayerId == playerId.Value)
            .ToDictionaryAsync(x => x.QuestId);

        var result = quests.Select(x =>
        {
            var progress = 0;
            var status = "NONE";
            if (playerQuests.TryGetValue(x.Id, out var pq))
            {
                progress = pq.Progress;
                status = pq.Status;
            }
            return new QuestDto(
                x.Id,
                x.QuestKey,
                x.Title,
                x.Description,
                x.QuestType,
                x.Category,
                x.TargetProgress,
                x.RewardJson,
                x.SortOrder,
                progress,
                status);
        }).ToList();

        return Results.Ok(ApiResponse<QuestListResponse>.Ok(new QuestListResponse(result)));
    }

    // ==================== 获取任务详情 ====================

    private static async Task<IResult> GetQuestDetail(Guid questId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var quest = await db.Quests
            .Where(x => x.Id == questId && x.IsActive)
            .Select(x => new
            {
                x.Id,
                x.QuestKey,
                x.Title,
                x.Description,
                x.QuestType,
                x.Category,
                x.TargetProgress,
                x.RewardJson,
                x.SortOrder
            })
            .FirstOrDefaultAsync();

        if (quest == null)
        {
            return ErrorResponse.NotFound("任务不存在").ToProblem();
        }

        var playerQuest = await db.PlayerQuests
            .FirstOrDefaultAsync(x => x.QuestId == questId && x.PlayerId == playerId.Value);

        var detail = new QuestDetailResponse(
            quest.Id,
            quest.QuestKey,
            quest.Title,
            quest.Description,
            quest.QuestType,
            quest.Category,
            quest.TargetProgress,
            quest.RewardJson,
            quest.SortOrder,
            playerQuest?.Progress ?? 0,
            playerQuest?.Status ?? "NONE",
            playerQuest?.AcceptedAt,
            playerQuest?.CompletedAt,
            playerQuest?.RewardedAt,
            playerQuest?.ExpiredAt);

        return Results.Ok(ApiResponse<QuestDetailResponse>.Ok(detail));
    }

    // ==================== 接受任务 ====================

    private static async Task<IResult> AcceptQuest(Guid questId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var quest = await db.Quests.FirstOrDefaultAsync(x => x.Id == questId && x.IsActive);
        if (quest == null)
        {
            return ErrorResponse.NotFound("任务不存在").ToProblem();
        }

        // 检查是否已接受过该任务
        var existing = await db.PlayerQuests
            .FirstOrDefaultAsync(x => x.QuestId == questId && x.PlayerId == playerId.Value);

        if (existing != null)
        {
            // 已存在记录时幂等返回当前状态
            return Results.Ok(ApiResponse<object>.Ok(new
            {
                PlayerQuestId = existing.Id,
                QuestId = questId,
                Status = existing.Status,
                Progress = existing.Progress
            }));
        }

        var playerQuest = new PlayerQuest
        {
            PlayerId = playerId.Value,
            QuestId = questId,
            Progress = 0,
            Status = "ACCEPTED",
            AcceptedAt = DateTimeOffset.UtcNow
        };

        db.PlayerQuests.Add(playerQuest);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new
        {
            PlayerQuestId = playerQuest.Id,
            QuestId = questId,
            Status = playerQuest.Status,
            Progress = playerQuest.Progress
        }));
    }

    // ==================== 领取任务奖励 ====================

    private static async Task<IResult> ClaimQuestReward(Guid questId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var quest = await db.Quests.FirstOrDefaultAsync(x => x.Id == questId && x.IsActive);
        if (quest == null)
        {
            return ErrorResponse.NotFound("任务不存在").ToProblem();
        }

        var playerQuest = await db.PlayerQuests
            .FirstOrDefaultAsync(x => x.QuestId == questId && x.PlayerId == playerId.Value);

        if (playerQuest == null)
        {
            return ErrorResponse.BadRequest("任务尚未接受，无法领取奖励").ToProblem();
        }

        // 幂等处理：已领取奖励的任务不重复发放
        if (playerQuest.Status == "REWARDED")
        {
            return Results.Ok(ApiResponse<ClaimQuestRewardResponse>.Ok(new ClaimQuestRewardResponse(
                quest.Id,
                quest.QuestKey,
                quest.Title,
                quest.RewardJson,
                playerQuest.Status,
                playerQuest.RewardedAt ?? DateTimeOffset.UtcNow)));
        }

        if (playerQuest.Status != "COMPLETED")
        {
            return ErrorResponse.BadRequest($"任务当前状态为 {playerQuest.Status}，仅 COMPLETED 状态可领取奖励").ToProblem();
        }

        // 解析并发放奖励
        await GrantQuestRewardAsync(db, playerId.Value, quest);

        playerQuest.Status = "REWARDED";
        playerQuest.RewardedAt = DateTimeOffset.UtcNow;

        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<ClaimQuestRewardResponse>.Ok(new ClaimQuestRewardResponse(
            quest.Id,
            quest.QuestKey,
            quest.Title,
            quest.RewardJson,
            playerQuest.Status,
            playerQuest.RewardedAt.Value)));
    }

    // ==================== 辅助方法 ====================

    /// <summary>
    /// 写入开发阶段种子任务数据 / Seed development quest data
    /// </summary>
    private static async Task EnsureSeedQuestsAsync(GameDbContext db)
    {
        // 二次检查，避免并发场景下重复写入
        if (await db.Quests.AnyAsync())
        {
            return;
        }

        foreach (var seed in SeedQuests)
        {
            db.Quests.Add(new Quest
            {
                Id = Guid.NewGuid(),
                QuestKey = seed.Key,
                Title = seed.Title,
                Description = seed.Desc,
                QuestType = seed.Type,
                Category = seed.Category,
                TargetProgress = seed.Target,
                RewardJson = seed.Reward,
                SortOrder = seed.Order,
                IsActive = true,
                CreatedAt = DateTimeOffset.UtcNow
            });
        }

        await db.SaveChangesAsync();
    }

    /// <summary>
    /// 解析奖励 JSON 并发放到钱包或背包 / Parse reward JSON and grant to wallet or inventory
    /// 支持两种奖励结构：
    /// - 货币奖励：{"currency":"GEM","amount":10}
    /// - 物品奖励：{"itemId":"skin_001","quantity":1}
    /// </summary>
    private static async Task GrantQuestRewardAsync(GameDbContext db, Guid playerId, Quest quest)
    {
        using var doc = JsonDocument.Parse(quest.RewardJson);
        var root = doc.RootElement;

        var idempotencyKey = $"quest:{quest.Id}:{playerId}";

        if (root.TryGetProperty("currency", out var currencyEl) && root.TryGetProperty("amount", out var amountEl))
        {
            // 货币奖励：写入钱包余额和流水
            var currency = currencyEl.GetString() ?? string.Empty;
            var amount = amountEl.GetInt64();

            var balance = await db.WalletBalances
                .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.CurrencyType == currency);

            long balanceBefore;
            if (balance == null)
            {
                balanceBefore = 0;
                balance = new WalletBalance
                {
                    PlayerId = playerId,
                    CurrencyType = currency,
                    Balance = amount,
                    UpdatedAt = DateTimeOffset.UtcNow
                };
                db.WalletBalances.Add(balance);
            }
            else
            {
                balanceBefore = balance.Balance;
                balance.Balance += amount;
                balance.UpdatedAt = DateTimeOffset.UtcNow;
            }

            db.WalletLedgers.Add(new WalletLedger
            {
                PlayerId = playerId,
                CurrencyType = currency,
                Amount = amount,
                BalanceBefore = balanceBefore,
                BalanceAfter = balanceBefore + amount,
                BizType = "QUEST",
                BizId = quest.QuestKey,
                IdempotencyKey = idempotencyKey,
                CreatedAt = DateTimeOffset.UtcNow
            });
        }
        else if (root.TryGetProperty("itemId", out var itemIdEl) && root.TryGetProperty("quantity", out var quantityEl))
        {
            // 物品奖励：写入背包和物品日志
            var itemId = itemIdEl.GetString() ?? string.Empty;
            var quantity = quantityEl.GetInt64();

            var item = await db.InventoryItems
                .FirstOrDefaultAsync(x => x.PlayerId == playerId && x.ItemId == itemId);

            long quantityBefore;
            if (item == null)
            {
                quantityBefore = 0;
                item = new InventoryItem
                {
                    PlayerId = playerId,
                    ItemId = itemId,
                    Quantity = quantity,
                    CreatedAt = DateTimeOffset.UtcNow,
                    UpdatedAt = DateTimeOffset.UtcNow
                };
                db.InventoryItems.Add(item);
            }
            else
            {
                quantityBefore = item.Quantity;
                item.Quantity += quantity;
                item.UpdatedAt = DateTimeOffset.UtcNow;
            }

            db.InventoryLogs.Add(new InventoryLog
            {
                PlayerId = playerId,
                ItemId = itemId,
                QuantityDelta = quantity,
                QuantityBefore = quantityBefore,
                QuantityAfter = quantityBefore + quantity,
                Reason = $"任务奖励：{quest.Title}",
                BizType = "QUEST",
                BizId = quest.QuestKey,
                CreatedAt = DateTimeOffset.UtcNow
            });
        }
        else
        {
            // 无法识别的奖励结构，记录但不阻断流程
            // 正式环境应补充中文错误日志或告警
        }

        await Task.CompletedTask;
    }
}
