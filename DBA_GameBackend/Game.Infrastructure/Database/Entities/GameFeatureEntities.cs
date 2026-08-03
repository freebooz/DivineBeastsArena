/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义排行、社交、邮件、公告、活动、客服、版本与 LiveOps 统计相关 EF Core 实体。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Infrastructure.Database.Entities;

// ==================== 排行榜 / Ranking ====================

public class PlayerRanking
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public int Rank { get; set; }
    public int Rating { get; set; } = 1000;
    public int TotalMatches { get; set; }
    public int Wins { get; set; }
    public int Losses { get; set; }
    public int Draws { get; set; }
    public int WinStreak { get; set; }
    public int MaxWinStreak { get; set; }
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? Player { get; set; }
}

// ==================== 好友系统 / Friends ====================

public class FriendRequest
{
    public Guid Id { get; set; }
    public Guid SenderId { get; set; }
    public Guid ReceiverId { get; set; }
    public string Status { get; set; } = "PENDING"; // PENDING, ACCEPTED, REJECTED, BLOCKED
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? RespondedAt { get; set; }
}

public class FriendRelation
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid FriendId { get; set; }
    public string Alias { get; set; } = string.Empty;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? Player { get; set; }
}

// ==================== 邮件系统 / Mail ====================

public class Mail
{
    public Guid Id { get; set; }
    public Guid? ReceiverId { get; set; }
    public string MailType { get; set; } = "SYSTEM"; // SYSTEM, PERSONAL, EVENT, GIFT
    public string Title { get; set; } = string.Empty;
    public string Content { get; set; } = string.Empty;
    public string AttachmentJson { get; set; } = "[]";
    public bool IsRead { get; set; }
    public bool IsDeleted { get; set; }
    public DateTimeOffset? ExpiresAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? ReadAt { get; set; }

    public PlayerProfile? Receiver { get; set; }
}

public class MailAttachment
{
    public Guid Id { get; set; }
    public Guid MailId { get; set; }
    public string ItemId { get; set; } = string.Empty;
    public long Quantity { get; set; }
    public bool IsClaimed { get; set; }
    public DateTimeOffset? ClaimedAt { get; set; }

    public Mail? Mail { get; set; }
}

// ==================== 公告系统 / Announcement ====================

public class Announcement
{
    public Guid Id { get; set; }
    public string Title { get; set; } = string.Empty;
    public string Content { get; set; } = string.Empty;
    public string Type { get; set; } = "INFO"; // INFO, EVENT, MAINTENANCE, UPDATE
    public int Priority { get; set; }
    public bool IsActive { get; set; } = true;
    public string Channel { get; set; } = "default";
    public string Region { get; set; } = "global";
    public string? MinVersion { get; set; }
    public string? MaxVersion { get; set; }
    public DateTimeOffset StartAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? EndAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

// ==================== 活动系统 / Event ====================

public class GameEvent
{
    public Guid Id { get; set; }
    public string EventKey { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string Type { get; set; } = "LIMITED"; // PERMANENT, LIMITED, SEASONAL
    public string Status { get; set; } = "ACTIVE"; // ACTIVE, INACTIVE, ENDED
    public string RewardsJson { get; set; } = "[]";
    public DateTimeOffset StartAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? EndAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class PlayerEventProgress
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid EventId { get; set; }
    public int Progress { get; set; }
    public int Target { get; set; }
    public bool IsCompleted { get; set; }
    public bool IsRewarded { get; set; }
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? Player { get; set; }
    public GameEvent? Event { get; set; }
}

// ==================== 成就系统 / Achievement ====================

public class Achievement
{
    public Guid Id { get; set; }
    public string AchievementKey { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string Category { get; set; } = string.Empty;
    public string Icon { get; set; } = string.Empty;
    public int MaxProgress { get; set; } = 1;
    public string RewardsJson { get; set; } = "[]";
    public int Order { get; set; }
}

public class PlayerAchievement
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid AchievementId { get; set; }
    public int Progress { get; set; }
    public bool IsUnlocked { get; set; }
    public DateTimeOffset? UnlockedAt { get; set; }

    public PlayerProfile? Player { get; set; }
    public Achievement? Achievement { get; set; }
}

// ==================== 任务系统 / Quest ====================

public class Quest
{
    public Guid Id { get; set; }
    public string QuestKey { get; set; } = string.Empty;        // 任务唯一标识（如 daily_play_1_match）
    public string Title { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public string QuestType { get; set; } = "DAILY";             // DAILY/WEEKLY/MAIN/ACHIEVEMENT
    public string Category { get; set; } = "GENERAL";            // 分类
    public int TargetProgress { get; set; }                      // 目标进度
    public string RewardJson { get; set; } = "{}";              // 奖励JSON（itemId, quantity 等）
    public int SortOrder { get; set; }
    public bool IsActive { get; set; } = true;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
}

public class PlayerQuest
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid QuestId { get; set; }
    public int Progress { get; set; }
    public string Status { get; set; } = "ACCEPTED";            // ACCEPTED/COMPLETED/REWARDED/EXPIRED
    public DateTimeOffset AcceptedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? CompletedAt { get; set; }
    public DateTimeOffset? RewardedAt { get; set; }
    public DateTimeOffset? ExpiredAt { get; set; }

    public PlayerProfile? Player { get; set; }
    public Quest? Quest { get; set; }
}

// ==================== 战绩记录 / Match History ====================

public class PlayerMatchHistory
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid SessionId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string MapId { get; set; } = string.Empty;
    public string? Team { get; set; }
    public string Result { get; set; } = string.Empty; // WIN, LOSS, DRAW
    public int Kills { get; set; }
    public int Deaths { get; set; }
    public int Assists { get; set; }
    public int Score { get; set; }
    public int DurationSeconds { get; set; }
    public DateTimeOffset PlayedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? Player { get; set; }
}

// ==================== 举报系统 / Report ====================

public class Report
{
    public Guid Id { get; set; }
    public Guid ReporterId { get; set; }
    public Guid? ReportedPlayerId { get; set; }
    public string ReportType { get; set; } = string.Empty; // CHEATING, HARASSMENT, TOXIC, OTHER
    public string Content { get; set; } = string.Empty;
    public string EvidenceJson { get; set; } = "[]";
    public string Status { get; set; } = "OPEN"; // OPEN, PROCESSING, RESOLVED, DISMISSED
    public Guid? HandledBy { get; set; }
    public string? HandleNote { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? HandledAt { get; set; }

    public PlayerProfile? Reporter { get; set; }
}

// ==================== 客服工单 / Support Ticket ====================

public class SupportTicket
{
    public Guid Id { get; set; }
    public Guid? PlayerId { get; set; }
    public string TicketType { get; set; } = string.Empty; // BUG, FEATURE, PAYMENT, ACCOUNT, OTHER
    public string Subject { get; set; } = string.Empty;
    public string Content { get; set; } = string.Empty;
    public string Status { get; set; } = "OPEN"; // OPEN, IN_PROGRESS, WAITING_PLAYER, RESOLVED, CLOSED
    public string Priority { get; set; } = "NORMAL"; // LOW, NORMAL, HIGH, URGENT
    public Guid? AssignedTo { get; set; }
    public string? InternalNote { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
    public DateTimeOffset? ResolvedAt { get; set; }

    public PlayerProfile? Player { get; set; }
    public AdminUser? AssignedAdmin { get; set; }
}

public class TicketReply
{
    public Guid Id { get; set; }
    public Guid TicketId { get; set; }
    public Guid? PlayerId { get; set; }
    public Guid? AdminId { get; set; }
    public string Content { get; set; } = string.Empty;
    public bool IsInternal { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public SupportTicket? Ticket { get; set; }
}

// ==================== 版本信息 / Version ====================

public class ClientVersion
{
    public Guid Id { get; set; }
    public string Version { get; set; } = string.Empty;
    public string Channel { get; set; } = "stable"; // stable, beta, alpha
    public string Platform { get; set; } = "Windows"; // Windows, MacOS, Linux, iOS, Android
    public string DownloadUrl { get; set; } = string.Empty;
    public string Checksum { get; set; } = string.Empty;
    public long SizeBytes { get; set; }
    public bool IsMandatory { get; set; }
    public bool IsActive { get; set; } = true;
    public string? MinOsVersion { get; set; }
    public string? ReleaseNotes { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

// ==================== LiveOps 统计 / Analytics ====================

public class DailyStats
{
    public DateTimeOffset Date { get; set; }
    public int NewUsers { get; set; }
    public int ActiveUsers { get; set; }
    public int NewAccounts { get; set; }
    public int TotalMatches { get; set; }
    public long TotalPlayTimeSeconds { get; set; }
    public long TotalRevenue { get; set; }
    public string Region { get; set; } = "global";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class RetentionCohort
{
    public Guid Id { get; set; }
    public DateTimeOffset CohortDate { get; set; }
    public int D0 { get; set; }
    public int D1 { get; set; }
    public int D3 { get; set; }
    public int D7 { get; set; }
    public int D14 { get; set; }
    public int D30 { get; set; }
    public string Region { get; set; } = "global";
}