/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Operation;
using Game.Api.Extensions;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;

namespace Game.Api.Endpoints.Operation;

/// <summary>
/// 运营相关接口 / Operation APIs
/// </summary>
public static class OperationEndpoints
{
    /// <summary>
    /// 注册运营端点 / Register operation endpoints
    /// </summary>
    public static void MapOperationEndpoints(this IEndpointRouteBuilder app)
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
            .RequireRateLimiting("admin");
        adminInventory.MapPost("/grant", GrantItem)
            .WithSummary("发放物品")
            .WithDescription("管理员发放物品给玩家")
            .RequireAuthorization();
        adminInventory.MapPost("/deduct", DeductItem)
            .WithSummary("扣除物品")
            .WithDescription("管理员扣除玩家物品")
            .RequireAuthorization();
        adminInventory.MapGet("/logs", GetInventoryLogs)
            .WithSummary("获取物品日志")
            .WithDescription("查看物品发放/扣除记录")
            .RequireAuthorization();

        // 排行榜
        var ranking = app.MapGroup("/api/rankings").WithTags("排行榜");
        ranking.MapGet("/{mode}", GetRanking)
            .WithSummary("获取排行榜")
            .WithDescription("获取指定模式的排行榜");
        ranking.MapGet("/{mode}/player/{playerId}", GetPlayerRank)
            .WithSummary("获取玩家排名")
            .WithDescription("获取指定玩家在排行榜中的位置");

        // 好友系统
        var friends = app.MapGroup("/api/friends").WithTags("好友");
        friends.MapGet("/", GetFriends)
            .WithSummary("获取好友列表")
            .WithDescription("获取当前玩家的好友列表")
            .RequireAuthorization();
        friends.MapGet("/requests", GetFriendRequests)
            .WithSummary("获取好友申请列表")
            .WithDescription("获取收到的好友申请列表")
            .RequireAuthorization();
        friends.MapPost("/request", SendFriendRequest)
            .WithSummary("发送好友申请")
            .WithDescription("向其他玩家发送好友申请")
            .RequireAuthorization();
        friends.MapPost("/{requestId}/accept", AcceptFriendRequest)
            .WithSummary("接受好友申请")
            .WithDescription("接受指定的好友申请")
            .RequireAuthorization();
        friends.MapPost("/{requestId}/reject", RejectFriendRequest)
            .WithSummary("拒绝好友申请")
            .WithDescription("拒绝指定的好友申请")
            .RequireAuthorization();
        friends.MapDelete("/{friendId}", RemoveFriend)
            .WithSummary("删除好友")
            .WithDescription("删除指定好友")
            .RequireAuthorization();

        // 邮件系统
        var mail = app.MapGroup("/api/mails").WithTags("邮件");
        mail.MapGet("/", GetMails)
            .WithSummary("获取邮件列表")
            .WithDescription("获取当前玩家的邮件列表")
            .RequireAuthorization();
        mail.MapPost("/{mailId}/read", ReadMail)
            .WithSummary("标记邮件已读")
            .WithDescription("标记指定邮件为已读")
            .RequireAuthorization();
        mail.MapPost("/{mailId}/attachments/{attachmentId}/claim", ClaimAttachment)
            .WithSummary("领取附件")
            .WithDescription("领取邮件中的附件物品")
            .RequireAuthorization();

        // 商城系统
        var shop = app.MapGroup("/api/shop").WithTags("商城");
        shop.MapGet("/items", GetShopItems)
            .WithSummary("获取商品列表")
            .WithDescription("获取商城商品列表");
        shop.MapPost("/purchase", PurchaseItem)
            .WithSummary("购买商品")
            .WithDescription("购买商城商品")
            .RequireAuthorization();

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

        // 战绩查询
        var matchHistory = app.MapGroup("/api/players/me/matches").WithTags("战绩");
        matchHistory.MapGet("/", GetMatchHistory)
            .WithSummary("获取历史战绩")
            .WithDescription("获取当前玩家的历史比赛记录")
            .RequireAuthorization();

        // 举报系统
        var report = app.MapGroup("/api/reports").WithTags("举报");
        report.MapPost("/", SubmitReport)
            .WithSummary("提交举报")
            .WithDescription("举报违规玩家")
            .RequireAuthorization();

        // 客服工单
        var support = app.MapGroup("/api/support").WithTags("客服");
        support.MapGet("/tickets", GetMyTickets)
            .WithSummary("获取我的工单")
            .WithDescription("获取当前玩家的客服工单")
            .RequireAuthorization();
        support.MapPost("/tickets", CreateTicket)
            .WithSummary("创建工单")
            .WithDescription("创建新的客服工单")
            .RequireAuthorization();
        support.MapGet("/tickets/{ticketId}", GetTicketDetail)
            .WithSummary("获取工单详情")
            .WithDescription("获取客服工单详情及回复")
            .RequireAuthorization();
        support.MapPost("/tickets/{ticketId}/reply", ReplyTicket)
            .WithSummary("回复工单")
            .WithDescription("回复客服工单")
            .RequireAuthorization();

        // 版本检测
        var version = app.MapGroup("/api/version").WithTags("版本");
        version.MapGet("/check", CheckVersion)
            .WithSummary("检查版本更新")
            .WithDescription("检查客户端版本是否有更新");

        // 运营统计
        var analytics = app.MapGroup("/api/admin/analytics").WithTags("运营统计(管理员)");
        analytics.MapGet("/overview", GetOverviewStats)
            .WithSummary("数据概览")
            .WithDescription("获取运营数据概览")
            .RequireAuthorization();
        analytics.MapGet("/retention", GetRetentionStats)
            .WithSummary("留存分析")
            .WithDescription("获取用户留存数据分析")
            .RequireAuthorization();

        // 断线重连
        var reconnect = app.MapGroup("/api/sessions").WithTags("断线重连");
        reconnect.MapPost("/{sessionId}/reconnect", Reconnect)
            .WithSummary("断线重连")
            .WithDescription("使用重连令牌重新连接游戏会话")
            .RequireAuthorization();
    }

    // ==================== 背包/物品 ====================

    private static async Task<IResult> GetInventory(Guid playerId, GameDbContext db)
    {
        var items = await db.InventoryItems
            .Where(x => x.PlayerId == playerId && x.Quantity > 0)
            .Select(x => new InventoryItemDto(x.ItemId, x.Quantity, x.ExpiresAt))
            .ToListAsync();
        return Results.Ok(ApiResponse<InventoryResponse>.Ok(new InventoryResponse(playerId, items)));
    }

    private static async Task<IResult> GetUnlocks(Guid playerId, GameDbContext db)
    {
        var unlocks = await db.PlayerUnlocks
            .Where(x => x.PlayerId == playerId)
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

    private static async Task<IResult> GetInventoryLogs(int page, int pageSize, GameDbContext db)
    {
        var logs = await db.InventoryLogs
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new { x.Id, x.PlayerId, x.ItemId, x.QuantityDelta, x.Reason, x.CreatedAt })
            .ToListAsync();
        return Results.Ok(ApiResponse<object>.Ok(logs));
    }

    // ==================== 排行榜 ====================

    private static async Task<IResult> GetRanking(string mode, int page, int pageSize, GameDbContext db)
    {
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

    // ==================== 好友系统 ====================

    private static async Task<IResult> GetFriends(Guid playerId, GameDbContext db)
    {
        var friends = await db.FriendRelations
            .Where(x => x.PlayerId == playerId)
            .Include(x => x.Player)
            .Select(x => new FriendInfo(x.FriendId, x.Player!.Nickname, x.Player.Avatar, x.Player.Level))
            .ToListAsync();

        return Results.Ok(ApiResponse<FriendsResponse>.Ok(new FriendsResponse(friends)));
    }

    private static async Task<IResult> GetFriendRequests(Guid playerId, GameDbContext db)
    {
        var requests = await db.FriendRequests
            .Where(x => x.ReceiverId == playerId && x.Status == "PENDING")
            .Join(db.PlayerProfiles, fr => fr.SenderId, pp => pp.PlayerId, (fr, pp) => new { fr, pp })
            .Select(x => new FriendRequestResponse(x.fr.Id, x.fr.SenderId, x.pp.Nickname, x.fr.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<object>.Ok(requests));
    }

    private static async Task<IResult> SendFriendRequest(FriendRequestDto request, Guid playerId, GameDbContext db)
    {
        var exists = await db.FriendRequests
            .AnyAsync(x => x.SenderId == playerId && x.ReceiverId == request.ReceiverId && x.Status == "PENDING");
        if (exists)
            return ErrorResponse.BadRequest("已发送过好友申请").ToProblem();

        var friendRequest = new FriendRequest { SenderId = playerId, ReceiverId = request.ReceiverId };
        db.FriendRequests.Add(friendRequest);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { RequestId = friendRequest.Id }));
    }

    private static async Task<IResult> AcceptFriendRequest(Guid requestId, Guid playerId, GameDbContext db)
    {
        var friendRequest = await db.FriendRequests
            .FirstOrDefaultAsync(x => x.Id == requestId && x.ReceiverId == playerId && x.Status == "PENDING");

        if (friendRequest == null)
            return ErrorResponse.NotFound("好友申请不存在").ToProblem();

        friendRequest.Status = "ACCEPTED";
        friendRequest.RespondedAt = DateTimeOffset.UtcNow;

        db.FriendRelations.Add(new FriendRelation { PlayerId = playerId, FriendId = friendRequest.SenderId });
        db.FriendRelations.Add(new FriendRelation { PlayerId = friendRequest.SenderId, FriendId = playerId });

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RejectFriendRequest(Guid requestId, Guid playerId, GameDbContext db)
    {
        var friendRequest = await db.FriendRequests
            .FirstOrDefaultAsync(x => x.Id == requestId && x.ReceiverId == playerId && x.Status == "PENDING");

        if (friendRequest == null)
            return ErrorResponse.NotFound("好友申请不存在").ToProblem();

        friendRequest.Status = "REJECTED";
        friendRequest.RespondedAt = DateTimeOffset.UtcNow;
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RemoveFriend(Guid friendId, Guid playerId, GameDbContext db)
    {
        var rel1 = await db.FriendRelations.FirstOrDefaultAsync(x => x.PlayerId == playerId && x.FriendId == friendId);
        var rel2 = await db.FriendRelations.FirstOrDefaultAsync(x => x.PlayerId == friendId && x.FriendId == playerId);

        if (rel1 != null) db.FriendRelations.Remove(rel1);
        if (rel2 != null) db.FriendRelations.Remove(rel2);

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    // ==================== 邮件系统 ====================

    private static async Task<IResult> GetMails(Guid playerId, bool unreadOnly, GameDbContext db)
    {
        var query = db.Mails.Where(x => x.ReceiverId == playerId && !x.IsDeleted);

        if (unreadOnly)
            query = query.Where(x => !x.IsRead);

        var mails = await query
            .OrderByDescending(x => x.CreatedAt)
            .Take(50)
            .Select(x => new MailDto(x.Id, x.Title, x.Content, x.MailType, x.IsRead,
                x.AttachmentJson != "[]", x.CreatedAt))
            .ToListAsync();

        var unreadCount = await db.Mails.CountAsync(x => x.ReceiverId == playerId && !x.IsRead && !x.IsDeleted);

        return Results.Ok(ApiResponse<MailListResponse>.Ok(new MailListResponse(mails, unreadCount)));
    }

    private static async Task<IResult> ReadMail(Guid mailId, Guid playerId, GameDbContext db)
    {
        var mail = await db.Mails.FirstOrDefaultAsync(x => x.Id == mailId && x.ReceiverId == playerId);
        if (mail == null) return ErrorResponse.NotFound("邮件不存在").ToProblem();

        mail.IsRead = true;
        mail.ReadAt = DateTimeOffset.UtcNow;
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> ClaimAttachment(Guid mailId, Guid attachmentId, Guid playerId, GameDbContext db)
    {
        var attachment = await db.MailAttachments
            .Include(x => x.Mail)
            .FirstOrDefaultAsync(x => x.Id == attachmentId && x.MailId == mailId && !x.IsClaimed);

        if (attachment?.Mail?.ReceiverId != playerId)
            return ErrorResponse.NotFound("附件不存在").ToProblem();

        attachment.IsClaimed = true;
        attachment.ClaimedAt = DateTimeOffset.UtcNow;

        // 添加物品到背包
        var item = await db.InventoryItems.FirstOrDefaultAsync(x => x.PlayerId == playerId && x.ItemId == attachment.ItemId);
        if (item == null)
        {
            item = new InventoryItem { PlayerId = playerId, ItemId = attachment.ItemId, Quantity = attachment.Quantity };
            db.InventoryItems.Add(item);
        }
        else
        {
            item.Quantity += attachment.Quantity;
            item.UpdatedAt = DateTimeOffset.UtcNow;
        }

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
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

    private static async Task<IResult> GetMyEventProgress(Guid playerId, GameDbContext db)
    {
        var progress = await db.PlayerEventProgresses
            .Include(x => x.Event)
            .Where(x => x.PlayerId == playerId)
            .Where(x => x.Event!.Status == "ACTIVE")
            .Select(x => new PlayerEventProgressDto(
                x.EventId, x.Event!.EventKey, x.Event.Title, x.Progress, x.Target, x.IsCompleted, x.IsRewarded))
            .ToListAsync();

        return Results.Ok(ApiResponse<object>.Ok(progress));
    }

    // ==================== 成就系统 ====================

    private static async Task<IResult> GetAchievements(Guid playerId, GameDbContext db)
    {
        var achievements = await db.Achievements
            .OrderBy(x => x.Order)
            .Select(x => new AchievementDto(x.Id, x.AchievementKey, x.Title, x.Description, x.Category,
                x.Icon, 0, x.MaxProgress, false, null))
            .ToListAsync();

        var playerAchievements = await db.PlayerAchievements
            .Where(x => x.PlayerId == playerId)
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

    // ==================== 战绩查询 ====================

    private static async Task<IResult> GetMatchHistory(Guid playerId, int page, int pageSize, GameDbContext db)
    {
        var totalCount = await db.PlayerMatchHistories.CountAsync(x => x.PlayerId == playerId);

        var matches = await db.PlayerMatchHistories
            .Where(x => x.PlayerId == playerId)
            .OrderByDescending(x => x.PlayedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new MatchHistoryDto(
                x.SessionId, x.Mode, x.MapId, x.Team, x.Result, x.Kills, x.Deaths, x.Assists,
                x.Score, x.DurationSeconds, x.PlayedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<MatchHistoryResponse>.Ok(new MatchHistoryResponse(matches, totalCount, page, pageSize)));
    }

    // ==================== 举报系统 ====================

    private static async Task<IResult> SubmitReport(SubmitReportRequest request, Guid playerId, GameDbContext db)
    {
        var report = new Report
        {
            ReporterId = playerId,
            ReportedPlayerId = request.ReportedPlayerId,
            ReportType = request.ReportType,
            Content = request.Content,
            EvidenceJson = System.Text.Json.JsonSerializer.Serialize(request.EvidenceUrls ?? new List<string>())
        };

        db.Reports.Add(report);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { ReportId = report.Id }));
    }

    // ==================== 客服工单 ====================

    private static async Task<IResult> GetMyTickets(Guid playerId, int page, int pageSize, GameDbContext db)
    {
        var totalCount = await db.SupportTickets.CountAsync(x => x.PlayerId == playerId);

        var tickets = await db.SupportTickets
            .Where(x => x.PlayerId == playerId)
            .OrderByDescending(x => x.CreatedAt)
            .Skip((page - 1) * pageSize)
            .Take(pageSize)
            .Select(x => new TicketDto(x.Id, x.TicketType, x.Subject, x.Status, x.Priority, x.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<TicketListResponse>.Ok(new TicketListResponse(tickets, totalCount)));
    }

    private static async Task<IResult> CreateTicket(CreateTicketRequest request, Guid playerId, GameDbContext db)
    {
        var ticket = new SupportTicket
        {
            PlayerId = playerId,
            TicketType = request.TicketType,
            Subject = request.Subject,
            Content = request.Content,
            Priority = request.Priority
        };

        db.SupportTickets.Add(ticket);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { TicketId = ticket.Id }));
    }

    private static async Task<IResult> GetTicketDetail(Guid ticketId, Guid playerId, GameDbContext db)
    {
        var ticket = await db.SupportTickets
            .Include(x => x.Player)
            .FirstOrDefaultAsync(x => x.Id == ticketId && x.PlayerId == playerId);

        if (ticket == null) return ErrorResponse.NotFound("工单不存在").ToProblem();

        var replies = await db.TicketReplies
            .Where(x => x.TicketId == ticketId && !x.IsInternal)
            .Join(db.PlayerProfiles, tr => tr.PlayerId, pp => pp.PlayerId, (tr, pp) => new { tr, pp })
            .OrderBy(x => x.tr.CreatedAt)
            .Select(x => new TicketReplyDto(x.tr.Id, x.tr.Content, x.tr.IsInternal, x.tr.PlayerId, x.tr.AdminId,
                x.pp.Nickname, x.tr.CreatedAt))
            .ToListAsync();

        var detail = new TicketDetailDto(
            ticket.Id, ticket.TicketType, ticket.Subject, ticket.Content, ticket.Status,
            ticket.Priority, ticket.CreatedAt, replies);

        return Results.Ok(ApiResponse<TicketDetailDto>.Ok(detail));
    }

    private static async Task<IResult> ReplyTicket(Guid ticketId, ReplyTicketRequest request, Guid playerId, GameDbContext db)
    {
        var ticket = await db.SupportTickets.FirstOrDefaultAsync(x => x.Id == ticketId && x.PlayerId == playerId);
        if (ticket == null) return ErrorResponse.NotFound("工单不存在").ToProblem();

        var reply = new TicketReply
        {
            TicketId = ticketId,
            PlayerId = playerId,
            Content = request.Content,
            IsInternal = false
        };

        db.TicketReplies.Add(reply);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
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

    // ==================== 断线重连 ====================

    private static async Task<IResult> Reconnect(Guid sessionId, ReconnectRequest request, Guid playerId, GameDbContext db)
    {
        var playerSession = await db.PlayerSessions
            .Include(x => x.GameSession)
            .FirstOrDefaultAsync(x => x.GameSessionId == sessionId && x.PlayerId == playerId);

        if (playerSession == null)
            return ErrorResponse.NotFound("未找到会话").ToProblem();

        var reconnectTokenExpiresAt = playerSession.ReconnectTokenExpiresAt;
        if (playerSession.ReconnectTokenHash == null ||
            !reconnectTokenExpiresAt.HasValue ||
            reconnectTokenExpiresAt.Value < DateTimeOffset.UtcNow)
            return ErrorResponse.Unauthorized("重连令牌已过期").ToProblem();

        return Results.Ok(ApiResponse<ReconnectResponse>.Ok(new ReconnectResponse(
            sessionId,
            playerSession.GameSession!.ServerIp ?? "",
            playerSession.GameSession.ServerPort ?? 0,
            "",
            reconnectTokenExpiresAt.Value)));
    }

    // ==================== 辅助方法 ====================

    private static Guid? GetPlayerId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("player_id") ?? ctx.User.FindFirst(ClaimTypes.NameIdentifier);
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static Guid? GetAdminId(HttpContext ctx)
    {
        var claim = ctx.User.FindFirst("admin_id");
        return claim != null && Guid.TryParse(claim.Value, out var id) ? id : null;
    }

    private static void AddAdminAuditLog(
        GameDbContext db,
        Guid adminId,
        string action,
        string targetType,
        string targetId,
        string reason,
        HttpContext ctx)
    {
        db.AdminAuditLogs.Add(new AdminAuditLog
        {
            AdminUserId = adminId,
            Action = action,
            TargetType = targetType,
            TargetId = targetId,
            Reason = reason.Trim(),
            IpAddress = ctx.Connection.RemoteIpAddress?.ToString(),
            UserAgent = ctx.Request.Headers.UserAgent.ToString()
        });
    }
}
