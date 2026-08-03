/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义跨进程/跨项目传输 DTO，客户端、后台和服务端都应以这里的字段契约为准。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Shared.Contracts.GameFeatures;

// ==================== 背包/物品 / Inventory ====================

public record InventoryResponse(
    Guid PlayerId,
    IReadOnlyList<InventoryItemDto> Items);

public record InventoryItemDto(
    string ItemId,
    long Quantity,
    DateTimeOffset? ExpiresAt);

public record GrantItemRequest(
    Guid PlayerId,
    string ItemId,
    long Quantity,
    string Reason);

public record DeductItemRequest(
    Guid PlayerId,
    string ItemId,
    long Quantity,
    string Reason);

// ==================== 排行榜 / Ranking ====================

public record RankingResponse(
    string Mode,
    IReadOnlyList<RankingEntry> Entries);

public record RankingEntry(
    int Rank,
    Guid PlayerId,
    string Nickname,
    int Rating,
    int TotalMatches,
    int Wins,
    int WinRate);

public record PlayerRankResponse(
    Guid PlayerId,
    string Nickname,
    int Rank,
    int Rating);

// ==================== 好友系统 / Friends ====================

public record FriendsResponse(
    IReadOnlyList<FriendInfo> Friends);

public record FriendInfo(
    Guid PlayerId,
    string Nickname,
    string? Avatar,
    int Level);

public record FriendRequestDto(
    Guid ReceiverId);

public record FriendRequestResponse(
    Guid RequestId,
    Guid SenderId,
    string SenderNickname,
    DateTimeOffset CreatedAt);

// ==================== 邮件系统 / Mail ====================

public record MailListResponse(
    IReadOnlyList<MailDto> Mails,
    int UnreadCount);

public record MailDto(
    Guid Id,
    string Title,
    string Content,
    string Type,
    bool IsRead,
    bool HasAttachment,
    DateTimeOffset CreatedAt);

/// <summary>
/// 邮件详情响应，包含附件列表 / Mail detail response including attachment list
/// </summary>
public record MailDetailDto(
    Guid Id,
    string Title,
    string Content,
    string Type,
    bool IsRead,
    bool HasAttachment,
    DateTimeOffset CreatedAt,
    DateTimeOffset? ReadAt,
    DateTimeOffset? ExpiresAt,
    IReadOnlyList<MailAttachmentDto> Attachments);

public record MailAttachmentDto(
    Guid Id,
    string ItemId,
    long Quantity,
    bool IsClaimed,
    DateTimeOffset? ClaimedAt);

public record ClaimMailAttachmentRequest(
    Guid AttachmentId);

/// <summary>
/// 一键领取邮件附件结果 / Result of claim-all mail attachments
/// </summary>
public record ClaimAllMailsResponse(
    int ClaimedMailCount,
    int ClaimedAttachmentCount,
    IReadOnlyList<MailAttachmentDto> ClaimedAttachments);

// ==================== 商城系统 / Shop ====================

public record ShopItemsResponse(
    IReadOnlyList<ShopItemDto> Items);

public record ShopItemDto(
    string ItemId,
    string Name,
    string Description,
    string? ImageUrl,
    long Price,
    string Currency,
    string Category,
    bool IsLimited,
    int Stock,
    DateTimeOffset? ExpiresAt);

public record PurchaseRequest(
    string ItemId,
    long Quantity,
    string PaymentMethod);

// ==================== 公告系统 / Announcement ====================

public record AnnouncementListResponse(
    IReadOnlyList<AnnouncementDto> Announcements);

public record AnnouncementDto(
    Guid Id,
    string Title,
    string Content,
    string Type,
    int Priority,
    DateTimeOffset StartAt,
    DateTimeOffset? EndAt);

// ==================== 活动系统 / Event ====================

public record EventListResponse(
    IReadOnlyList<GameEventDto> Events);

public record GameEventDto(
    Guid Id,
    string EventKey,
    string Title,
    string Description,
    string Type,
    string Status,
    DateTimeOffset StartAt,
    DateTimeOffset? EndAt);

public record PlayerEventProgressDto(
    Guid EventId,
    string EventKey,
    string Title,
    int Progress,
    int Target,
    bool IsCompleted,
    bool IsRewarded);

// ==================== 成就系统 / Achievement ====================

public record AchievementListResponse(
    IReadOnlyList<AchievementDto> Achievements);

public record AchievementDto(
    Guid Id,
    string AchievementKey,
    string Title,
    string Description,
    string Category,
    string Icon,
    int Progress,
    int MaxProgress,
    bool IsUnlocked,
    DateTimeOffset? UnlockedAt);

// ==================== 任务系统 / Quest ====================

/// <summary>
/// 任务信息 DTO / Quest info DTO
/// </summary>
public record QuestDto(
    Guid QuestId,
    string QuestKey,
    string Title,
    string Description,
    string QuestType,
    string Category,
    int TargetProgress,
    string RewardJson,
    int SortOrder,
    int Progress,
    string Status);

/// <summary>
/// 任务列表响应 / Quest list response
/// </summary>
public record QuestListResponse(
    IReadOnlyList<QuestDto> Quests);

/// <summary>
/// 任务详情响应 / Quest detail response
/// </summary>
public record QuestDetailResponse(
    Guid QuestId,
    string QuestKey,
    string Title,
    string Description,
    string QuestType,
    string Category,
    int TargetProgress,
    string RewardJson,
    int SortOrder,
    int Progress,
    string Status,
    DateTimeOffset? AcceptedAt,
    DateTimeOffset? CompletedAt,
    DateTimeOffset? RewardedAt,
    DateTimeOffset? ExpiredAt);

/// <summary>
/// 领取任务奖励响应 / Claim quest reward response
/// </summary>
public record ClaimQuestRewardResponse(
    Guid QuestId,
    string QuestKey,
    string Title,
    string RewardJson,
    string Status,
    DateTimeOffset RewardedAt);

// ==================== 战绩查询 / Match History ====================

public record MatchHistoryResponse(
    IReadOnlyList<MatchHistoryDto> Matches,
    int TotalCount,
    int Page,
    int PageSize);

public record MatchHistoryDto(
    Guid SessionId,
    string Mode,
    string MapId,
    string? Team,
    string Result,
    int Kills,
    int Deaths,
    int Assists,
    int Score,
    string ResultJson,
    string? WinnerTeam,
    int DurationSeconds,
    long ExpDelta,
    IReadOnlyDictionary<string, object> Rewards,
    DateTimeOffset PlayedAt);

// ==================== 举报系统 / Report ====================

public record SubmitReportRequest(
    Guid? ReportedPlayerId,
    string ReportType,
    string Content,
    IReadOnlyList<string>? EvidenceUrls);

/// <summary>
/// 我的举报列表响应 / My reports list response
/// </summary>
public record MyReportsResponse(
    IReadOnlyList<ReportDto> Reports,
    int TotalCount,
    int Page,
    int PageSize);

public record ReportDto(
    Guid Id,
    Guid? ReportedPlayerId,
    string ReportType,
    string Content,
    string Status,
    DateTimeOffset CreatedAt,
    DateTimeOffset? HandledAt,
    string? HandleNote);

/// <summary>
/// 提交申诉请求 / Submit appeal request
/// 申诉会被创建为 TicketType = "APPEAL" 的客服工单
/// </summary>
public record SubmitAppealRequest(
    string Subject,
    string Content,
    string Priority,
    Guid? RelatedTicketId,
    Guid? RelatedReportId);

// ==================== 客服工单 / Support Ticket ====================

public record CreateTicketRequest(
    string TicketType,
    string Subject,
    string Content,
    string Priority);

public record TicketListResponse(
    IReadOnlyList<TicketDto> Tickets,
    int TotalCount,
    int Page,
    int PageSize);

public record TicketDto(
    Guid Id,
    string TicketType,
    string Subject,
    string Status,
    string Priority,
    DateTimeOffset CreatedAt);

public record TicketDetailDto(
    Guid Id,
    string TicketType,
    string Subject,
    string Content,
    string Status,
    string Priority,
    DateTimeOffset CreatedAt,
    IReadOnlyList<TicketReplyDto> Replies);

public record TicketReplyDto(
    Guid Id,
    string Content,
    bool IsInternal,
    Guid? PlayerId,
    Guid? AdminId,
    string? AuthorName,
    DateTimeOffset CreatedAt);

public record ReplyTicketRequest(
    string Content,
    bool IsInternal);

// ==================== 版本检测 / Version ====================

public record VersionCheckResponse(
    string LatestVersion,
    string Channel,
    string DownloadUrl,
    bool IsMandatory,
    string? ReleaseNotes);

// ==================== 运营统计 / Analytics ====================

public record OverviewStatsResponse(
    int TotalPlayers,
    int ActivePlayersToday,
    int ActivePlayersThisWeek,
    int TotalMatchesToday,
    long TotalPlayTimeToday,
    long TotalRevenueToday);

public record RetentionStatsResponse(
    DateTimeOffset CohortDate,
    int D0,
    int D1,
    int D3,
    int D7,
    int D14,
    int D30,
    IReadOnlyList<DailyRetentionPoint> Trend);

public record DailyRetentionPoint(
    DateTimeOffset Date,
    int RetentionRate);

// ==================== 断线重连 / Reconnect ====================

public record ReconnectRequest(
    string ReconnectToken);

public record ReconnectResponse(
    Guid SessionId,
    string ServerIp,
    int ServerPort,
    string SessionToken,
    DateTimeOffset TokenExpiresAt);

// ==================== 充值/支付 / Payment ====================

/// <summary>
/// 创建充值订单请求 / Create payment order request
/// </summary>
public record CreatePaymentOrderRequest(
    string ProductId,
    string Platform);

/// <summary>
/// 充值档位信息 / Recharge product info
/// </summary>
public record PaymentProductInfoDto(
    string ProductId,
    string Name,
    long Amount,
    string Currency,
    long VirtualAmount,
    string VirtualCurrency);

/// <summary>
/// 充值订单响应 / Payment order response
/// </summary>
public record PaymentOrderResponse(
    Guid OrderId,
    string Status,
    long Amount,
    string Currency,
    string Platform,
    string PlatformOrderId,
    PaymentProductInfoDto ProductInfo,
    DateTimeOffset CreatedAt,
    DateTimeOffset? PaidAt);

/// <summary>
/// 我的充值订单列表响应 / My payment orders list response
/// </summary>
public record PaymentOrderListResponse(
    IReadOnlyList<PaymentOrderResponse> Orders,
    int TotalCount,
    int Page,
    int PageSize);

/// <summary>
/// 钱包余额项 / Wallet balance entry
/// </summary>
public record WalletBalanceDto(
    string CurrencyType,
    long Balance,
    DateTimeOffset UpdatedAt);

/// <summary>
/// 钱包余额响应 / Wallet balance response
/// </summary>
public record WalletBalanceResponse(
    Guid PlayerId,
    IReadOnlyList<WalletBalanceDto> Balances);

/// <summary>
/// 钱包流水项 / Wallet ledger entry
/// </summary>
public record WalletLedgerDto(
    Guid Id,
    string CurrencyType,
    long Amount,
    long BalanceBefore,
    long BalanceAfter,
    string BizType,
    string BizId,
    DateTimeOffset CreatedAt);

/// <summary>
/// 钱包流水列表响应 / Wallet ledger list response
/// </summary>
public record WalletLedgerResponse(
    IReadOnlyList<WalletLedgerDto> Ledgers,
    int TotalCount,
    int Page,
    int PageSize);
