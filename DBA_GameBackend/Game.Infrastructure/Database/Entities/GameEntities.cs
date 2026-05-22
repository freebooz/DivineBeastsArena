/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 EF Core 实体模型，对应数据库表结构和领域对象的持久化形态。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Infrastructure.Database.Entities;

public class GameServerInstance
{
    public Guid Id { get; set; }
    public Guid? SessionId { get; set; }
    public string? Mode { get; set; }
    public string? MapId { get; set; }
    public string? Region { get; set; }
    public string? BuildVersion { get; set; }
    public string Ip { get; set; } = string.Empty;
    public int Port { get; set; }
    public int? ProcessId { get; set; }
    public string? ContainerId { get; set; }
    public string? RuntimeTokenHash { get; set; }
    public DateTimeOffset? RuntimeTokenExpiresAt { get; set; }
    public string Status { get; set; } = "STARTING";
    public DateTimeOffset StartedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? ReadyAt { get; set; }
    public DateTimeOffset? AllocatedAt { get; set; }
    public DateTimeOffset? EndedAt { get; set; }
    public DateTimeOffset? LastHeartbeatAt { get; set; }
    public int? ExitCode { get; set; }
    public string? CrashReason { get; set; }
    public string? LogPath { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }

    public ICollection<GameServerEvent> Events { get; set; } = new List<GameServerEvent>();
}

public class GameServerEvent
{
    public Guid Id { get; set; }
    public Guid ServerId { get; set; }
    public string EventType { get; set; } = string.Empty;
    public string PayloadJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public GameServerInstance? Server { get; set; }
}

public class PortAllocation
{
    public int Port { get; set; }
    public string Status { get; set; } = "FREE";
    public Guid? ServerId { get; set; }
    public DateTimeOffset? AllocatedAt { get; set; }
    public DateTimeOffset? ReleasedAt { get; set; }
}

public class MatchResult
{
    public Guid Id { get; set; }
    public Guid SessionId { get; set; }
    public Guid ServerId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string MapId { get; set; } = string.Empty;
    public int DurationSeconds { get; set; }
    public string ResultJson { get; set; } = "{}";
    public string IdempotencyKey { get; set; } = string.Empty;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public GameSession? GameSession { get; set; }
    public ICollection<MatchPlayerResult> PlayerResults { get; set; } = new List<MatchPlayerResult>();
}

public class MatchPlayerResult
{
    public Guid Id { get; set; }
    public Guid MatchResultId { get; set; }
    public Guid PlayerId { get; set; }
    public string? Team { get; set; }
    public string Result { get; set; } = string.Empty;
    public int Kills { get; set; }
    public int Deaths { get; set; }
    public int Assists { get; set; }
    public int Score { get; set; }
    public long ExpDelta { get; set; }
    public string RewardJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public MatchResult? MatchResult { get; set; }
}

public class InventoryItem
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string ItemId { get; set; } = string.Empty;
    public long Quantity { get; set; }
    public DateTimeOffset? ExpiresAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? Player { get; set; }
}

public class InventoryLog
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string ItemId { get; set; } = string.Empty;
    public long QuantityDelta { get; set; }
    public long QuantityBefore { get; set; }
    public long QuantityAfter { get; set; }
    public string Reason { get; set; } = string.Empty;
    public string? BizType { get; set; }
    public string? BizId { get; set; }
    public Guid? OperatorId { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class AdminUser
{
    public Guid Id { get; set; }
    public string Username { get; set; } = string.Empty;
    public string PasswordHash { get; set; } = string.Empty;
    public string Role { get; set; } = "GM";
    public string Status { get; set; } = "ACTIVE";
    public int FailedLoginCount { get; set; }
    public DateTimeOffset? LockedUntil { get; set; }
    public DateTimeOffset? LastLoginAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
}

public class AdminAuditLog
{
    public Guid Id { get; set; }
    public Guid? AdminUserId { get; set; }
    public string Action { get; set; } = string.Empty;
    public string TargetType { get; set; } = string.Empty;
    public string? TargetId { get; set; }
    public string? Reason { get; set; }
    public string? BeforeJson { get; set; }
    public string? AfterJson { get; set; }
    public string? IpAddress { get; set; }
    public string? UserAgent { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class CrashReport
{
    public Guid Id { get; set; }
    public Guid? PlayerId { get; set; }
    public string? ClientVersion { get; set; }
    public string? Platform { get; set; }
    public string? CrashType { get; set; }
    public string? Title { get; set; }
    public string? Description { get; set; }
    public string? DumpUrl { get; set; }
    public string? LogUrl { get; set; }
    public string MetadataJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class PlayerFeedback
{
    public Guid Id { get; set; }
    public Guid? PlayerId { get; set; }
    public string? Nickname { get; set; }
    public string? Email { get; set; }
    public string FeedbackType { get; set; } = string.Empty;
    public string? Title { get; set; }
    public string Content { get; set; } = string.Empty;
    public string Status { get; set; } = "OPEN";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
    public Guid? HandledBy { get; set; }
    public DateTimeOffset? HandledAt { get; set; }
    public string? HandleNote { get; set; }
}

public class OrderRecord
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string Platform { get; set; } = string.Empty;
    public string? PlatformOrderId { get; set; }
    public string Status { get; set; } = "CREATED";
    public long Amount { get; set; }
    public string Currency { get; set; } = "USD";
    public string ItemJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? PaidAt { get; set; }
    public DateTimeOffset? CompletedAt { get; set; }
    public DateTimeOffset? UpdatedAt { get; set; }
}

public class WalletBalance
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string CurrencyType { get; set; } = string.Empty;
    public long Balance { get; set; }
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class WalletLedger
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string CurrencyType { get; set; } = string.Empty;
    public long Amount { get; set; }
    public long BalanceBefore { get; set; }
    public long BalanceAfter { get; set; }
    public string BizType { get; set; } = string.Empty;
    public string BizId { get; set; } = string.Empty;
    public string IdempotencyKey { get; set; } = string.Empty;
    public Guid? OperatorId { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}