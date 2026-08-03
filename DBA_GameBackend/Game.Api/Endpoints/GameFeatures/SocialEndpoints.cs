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
    private static void MapSocialEndpoints(IEndpointRouteBuilder app)
    {
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

        // 邮件系统（客户端兼容路径 /api/players/me/mails）
        var playerMail = app.MapGroup("/api/players/me/mails").WithTags("邮件");
        playerMail.MapGet("/", GetMails)
            .WithSummary("获取邮件列表")
            .WithDescription("获取当前玩家的邮件列表（兼容路径）")
            .RequireAuthorization();
        playerMail.MapGet("/{mailId}", GetMailDetail)
            .WithSummary("获取邮件详情")
            .WithDescription("获取指定邮件的详情及附件列表")
            .RequireAuthorization();
        playerMail.MapPost("/{mailId}/read", ReadMail)
            .WithSummary("标记邮件已读")
            .WithDescription("标记指定邮件为已读（兼容路径）")
            .RequireAuthorization();
        playerMail.MapPost("/{mailId}/claim", ClaimMailAllAttachments)
            .WithSummary("领取邮件所有附件")
            .WithDescription("一次性领取指定邮件中所有未领取的附件")
            .RequireAuthorization();
        playerMail.MapPost("/claim-all", ClaimAllMails)
            .WithSummary("一键领取所有邮件附件")
            .WithDescription("一次性领取当前玩家所有邮件中未领取的附件")
            .RequireAuthorization();
    }

    // ==================== 好友系统 ====================

    private static async Task<IResult> GetFriends(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var friends = await db.FriendRelations
            .Where(x => x.PlayerId == playerId.Value)
            .Join(db.PlayerProfiles, relation => relation.FriendId, profile => profile.PlayerId, (relation, profile) => profile)
            .Select(x => new FriendInfo(x.PlayerId, x.Nickname, x.Avatar, x.Level))
            .ToListAsync();

        return Results.Ok(ApiResponse<FriendsResponse>.Ok(new FriendsResponse(friends)));
    }

    private static async Task<IResult> GetFriendRequests(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var requests = await db.FriendRequests
            .Where(x => x.ReceiverId == playerId.Value && x.Status == "PENDING")
            .Join(db.PlayerProfiles, fr => fr.SenderId, pp => pp.PlayerId, (fr, pp) => new { fr, pp })
            .Select(x => new FriendRequestResponse(x.fr.Id, x.fr.SenderId, x.pp.Nickname, x.fr.CreatedAt))
            .ToListAsync();

        return Results.Ok(ApiResponse<object>.Ok(requests));
    }

    private static async Task<IResult> SendFriendRequest(FriendRequestDto request, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var exists = await db.FriendRequests
            .AnyAsync(x => x.SenderId == playerId.Value && x.ReceiverId == request.ReceiverId && x.Status == "PENDING");
        if (exists)
            return ErrorResponse.BadRequest("已发送过好友申请").ToProblem();

        var friendRequest = new FriendRequest { SenderId = playerId.Value, ReceiverId = request.ReceiverId };
        db.FriendRequests.Add(friendRequest);
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse<object>.Ok(new { RequestId = friendRequest.Id }));
    }

    private static async Task<IResult> AcceptFriendRequest(Guid requestId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var friendRequest = await db.FriendRequests
            .FirstOrDefaultAsync(x => x.Id == requestId && x.ReceiverId == playerId.Value && x.Status == "PENDING");

        if (friendRequest == null)
            return ErrorResponse.NotFound("好友申请不存在").ToProblem();

        friendRequest.Status = "ACCEPTED";
        friendRequest.RespondedAt = DateTimeOffset.UtcNow;

        db.FriendRelations.Add(new FriendRelation { PlayerId = playerId.Value, FriendId = friendRequest.SenderId });
        db.FriendRelations.Add(new FriendRelation { PlayerId = friendRequest.SenderId, FriendId = playerId.Value });

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RejectFriendRequest(Guid requestId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var friendRequest = await db.FriendRequests
            .FirstOrDefaultAsync(x => x.Id == requestId && x.ReceiverId == playerId.Value && x.Status == "PENDING");

        if (friendRequest == null)
            return ErrorResponse.NotFound("好友申请不存在").ToProblem();

        friendRequest.Status = "REJECTED";
        friendRequest.RespondedAt = DateTimeOffset.UtcNow;
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> RemoveFriend(Guid friendId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var rel1 = await db.FriendRelations.FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.FriendId == friendId);
        var rel2 = await db.FriendRelations.FirstOrDefaultAsync(x => x.PlayerId == friendId && x.FriendId == playerId.Value);

        if (rel1 != null) db.FriendRelations.Remove(rel1);
        if (rel2 != null) db.FriendRelations.Remove(rel2);

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse.Ok());
    }

    // ==================== 邮件系统 ====================

    private static async Task<IResult> GetMails(HttpContext ctx, GameDbContext db, bool unreadOnly = false)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var query = db.Mails.Where(x => x.ReceiverId == playerId.Value && !x.IsDeleted);

        if (unreadOnly)
            query = query.Where(x => !x.IsRead);

        var mails = await query
            .OrderByDescending(x => x.CreatedAt)
            .Take(50)
            .Select(x => new MailDto(x.Id, x.Title, x.Content, x.MailType, x.IsRead,
                x.AttachmentJson != "[]", x.CreatedAt))
            .ToListAsync();

        var unreadCount = await db.Mails.CountAsync(x => x.ReceiverId == playerId.Value && !x.IsRead && !x.IsDeleted);

        return Results.Ok(ApiResponse<MailListResponse>.Ok(new MailListResponse(mails, unreadCount)));
    }

    private static async Task<IResult> ReadMail(Guid mailId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var mail = await db.Mails.FirstOrDefaultAsync(x => x.Id == mailId && x.ReceiverId == playerId.Value);
        if (mail == null) return ErrorResponse.NotFound("邮件不存在").ToProblem();

        mail.IsRead = true;
        mail.ReadAt = DateTimeOffset.UtcNow;
        await db.SaveChangesAsync();

        return Results.Ok(ApiResponse.Ok());
    }

    private static async Task<IResult> ClaimAttachment(Guid mailId, Guid attachmentId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var attachment = await db.MailAttachments
            .Include(x => x.Mail)
            .FirstOrDefaultAsync(x => x.Id == attachmentId && x.MailId == mailId && !x.IsClaimed);

        if (attachment?.Mail?.ReceiverId != playerId.Value)
            return ErrorResponse.NotFound("附件不存在").ToProblem();

        attachment.IsClaimed = true;
        attachment.ClaimedAt = DateTimeOffset.UtcNow;

        // 添加物品到背包
        var item = await db.InventoryItems.FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.ItemId == attachment.ItemId);
        if (item == null)
        {
            item = new InventoryItem { PlayerId = playerId.Value, ItemId = attachment.ItemId, Quantity = attachment.Quantity };
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

    private static async Task<IResult> GetMailDetail(Guid mailId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        var mail = await db.Mails
            .FirstOrDefaultAsync(x => x.Id == mailId && x.ReceiverId == playerId.Value && !x.IsDeleted);
        if (mail == null) return ErrorResponse.NotFound("邮件不存在").ToProblem();

        var attachments = await db.MailAttachments
            .Where(x => x.MailId == mailId)
            .OrderBy(x => x.Id)
            .Select(x => new MailAttachmentDto(x.Id, x.ItemId, x.Quantity, x.IsClaimed, x.ClaimedAt))
            .ToListAsync();

        var detail = new MailDetailDto(
            mail.Id,
            mail.Title,
            mail.Content,
            mail.MailType,
            mail.IsRead,
            mail.AttachmentJson != "[]",
            mail.CreatedAt,
            mail.ReadAt,
            mail.ExpiresAt,
            attachments);

        return Results.Ok(ApiResponse<MailDetailDto>.Ok(detail));
    }

    private static async Task<IResult> ClaimMailAllAttachments(Guid mailId, HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        // 校验邮件归属当前玩家
        var mail = await db.Mails.FirstOrDefaultAsync(x => x.Id == mailId && x.ReceiverId == playerId.Value && !x.IsDeleted);
        if (mail == null) return ErrorResponse.NotFound("邮件不存在").ToProblem();

        var attachments = await db.MailAttachments
            .Where(x => x.MailId == mailId && !x.IsClaimed)
            .ToListAsync();

        if (attachments.Count == 0)
        {
            return Results.Ok(ApiResponse<ClaimAllMailsResponse>.Ok(new ClaimAllMailsResponse(0, 0, Array.Empty<MailAttachmentDto>())));
        }

        var claimedDtos = new List<MailAttachmentDto>();
        foreach (var attachment in attachments)
        {
            attachment.IsClaimed = true;
            attachment.ClaimedAt = DateTimeOffset.UtcNow;

            // 添加物品到背包
            var item = await db.InventoryItems.FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.ItemId == attachment.ItemId);
            if (item == null)
            {
                item = new InventoryItem { PlayerId = playerId.Value, ItemId = attachment.ItemId, Quantity = attachment.Quantity };
                db.InventoryItems.Add(item);
            }
            else
            {
                item.Quantity += attachment.Quantity;
                item.UpdatedAt = DateTimeOffset.UtcNow;
            }

            claimedDtos.Add(new MailAttachmentDto(attachment.Id, attachment.ItemId, attachment.Quantity, attachment.IsClaimed, attachment.ClaimedAt));
        }

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse<ClaimAllMailsResponse>.Ok(new ClaimAllMailsResponse(1, claimedDtos.Count, claimedDtos)));
    }

    private static async Task<IResult> ClaimAllMails(HttpContext ctx, GameDbContext db)
    {
        var playerId = GetPlayerId(ctx);
        if (!playerId.HasValue)
        {
            return ErrorResponse.Unauthorized().ToProblem();
        }

        // 查询当前玩家所有未删除邮件中未领取的附件
        var mailIds = await db.Mails
            .Where(x => x.ReceiverId == playerId.Value && !x.IsDeleted)
            .Select(x => x.Id)
            .ToListAsync();

        if (mailIds.Count == 0)
        {
            return Results.Ok(ApiResponse<ClaimAllMailsResponse>.Ok(new ClaimAllMailsResponse(0, 0, Array.Empty<MailAttachmentDto>())));
        }

        var attachments = await db.MailAttachments
            .Where(x => mailIds.Contains(x.MailId) && !x.IsClaimed)
            .ToListAsync();

        if (attachments.Count == 0)
        {
            return Results.Ok(ApiResponse<ClaimAllMailsResponse>.Ok(new ClaimAllMailsResponse(0, 0, Array.Empty<MailAttachmentDto>())));
        }

        var claimedDtos = new List<MailAttachmentDto>();
        var claimedMailIds = new HashSet<Guid>();

        foreach (var attachment in attachments)
        {
            attachment.IsClaimed = true;
            attachment.ClaimedAt = DateTimeOffset.UtcNow;
            claimedMailIds.Add(attachment.MailId);

            // 添加物品到背包
            var item = await db.InventoryItems.FirstOrDefaultAsync(x => x.PlayerId == playerId.Value && x.ItemId == attachment.ItemId);
            if (item == null)
            {
                item = new InventoryItem { PlayerId = playerId.Value, ItemId = attachment.ItemId, Quantity = attachment.Quantity };
                db.InventoryItems.Add(item);
            }
            else
            {
                item.Quantity += attachment.Quantity;
                item.UpdatedAt = DateTimeOffset.UtcNow;
            }

            claimedDtos.Add(new MailAttachmentDto(attachment.Id, attachment.ItemId, attachment.Quantity, attachment.IsClaimed, attachment.ClaimedAt));
        }

        await db.SaveChangesAsync();
        return Results.Ok(ApiResponse<ClaimAllMailsResponse>.Ok(new ClaimAllMailsResponse(claimedMailIds.Count, claimedDtos.Count, claimedDtos)));
    }
}
