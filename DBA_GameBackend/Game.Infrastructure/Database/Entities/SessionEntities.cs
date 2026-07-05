/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 EF Core 实体模型，对应数据库表结构和领域对象的持久化形态。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Infrastructure.Database.Entities;

public class GameConfig
{
    public Guid Id { get; set; }
    public string ConfigKey { get; set; } = string.Empty;
    public string Version { get; set; } = string.Empty;
    public string ContentJson { get; set; } = "{}";
    public string Status { get; set; } = "DRAFT";
    public string Checksum { get; set; } = string.Empty;
    public string Channel { get; set; } = "default";
    public string Region { get; set; } = "global";
    public string? MinClientVersion { get; set; }
    public string? MaxClientVersion { get; set; }
    public Guid? CreatedBy { get; set; }
    public Guid? PublishedBy { get; set; }
    public DateTimeOffset? PublishedAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
}

public class GameConfigPublishLog
{
    public Guid Id { get; set; }
    public string ConfigKey { get; set; } = string.Empty;
    public string? FromVersion { get; set; }
    public string ToVersion { get; set; } = string.Empty;
    public Guid? OperatorId { get; set; }
    public string? Reason { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class GameRoom
{
    public Guid Id { get; set; }
    public Guid OwnerPlayerId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string MapId { get; set; } = string.Empty;
    public string Region { get; set; } = string.Empty;
    public int MaxPlayers { get; set; } = 8;
    public string Visibility { get; set; } = "PUBLIC";
    public string? PasswordHash { get; set; }
    public string Status { get; set; } = "WAITING";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
    public DateTimeOffset? ClosedAt { get; set; }

    public ICollection<GameRoomPlayer> Players { get; set; } = new List<GameRoomPlayer>();
}

public class GameRoomPlayer
{
    public Guid Id { get; set; }
    public Guid RoomId { get; set; }
    public Guid PlayerId { get; set; }
    public int SlotIndex { get; set; }
    public string? Team { get; set; }
    public bool IsReady { get; set; }
    public DateTimeOffset JoinedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? LeftAt { get; set; }

    public GameRoom? Room { get; set; }
}

public class MatchmakingTicket
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string Region { get; set; } = string.Empty;
    public int Mmr { get; set; } = 1000;
    public string Status { get; set; } = "QUEUED";
    public Guid? MatchedSessionId { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }
    public DateTimeOffset? CancelledAt { get; set; }
    public DateTimeOffset? TimeoutAt { get; set; }
}

public class GameSession
{
    public Guid Id { get; set; }
    public string SourceType { get; set; } = string.Empty;
    public Guid? SourceId { get; set; }
    public string Mode { get; set; } = string.Empty;
    public string MapId { get; set; } = string.Empty;
    public string Region { get; set; } = string.Empty;
    public string Status { get; set; } = "CREATED";
    public Guid? ServerId { get; set; }
    public string? ServerIp { get; set; }
    public int? ServerPort { get; set; }
    public string? BuildVersion { get; set; }
    public int MaxPlayers { get; set; }
    public int RetryCount { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? AllocatedAt { get; set; }
    public DateTimeOffset? StartedAt { get; set; }
    public DateTimeOffset? EndedAt { get; set; }
    public DateTimeOffset? UpdatedAt { get; set; }

    public ICollection<PlayerSession> PlayerSessions { get; set; } = new List<PlayerSession>();
    public ICollection<SessionEvent> SessionEvents { get; set; } = new List<SessionEvent>();
}

public class PlayerSession
{
    public Guid Id { get; set; }
    public Guid GameSessionId { get; set; }
    public Guid PlayerId { get; set; }
    public string? Team { get; set; }
    public int? SlotIndex { get; set; }
    public string? Zodiac { get; set; }
    public string? PrimaryElement { get; set; }
    public string? FiveCamp { get; set; }
    public string? FixedSkillGroupId { get; set; }
    public string Status { get; set; } = "CREATED";
    public string SessionTokenHash { get; set; } = string.Empty;
    public DateTimeOffset SessionTokenExpiresAt { get; set; }
    public string? ReconnectTokenHash { get; set; }
    public DateTimeOffset? ReconnectTokenExpiresAt { get; set; }
    public DateTimeOffset? JoinedAt { get; set; }
    public DateTimeOffset? LeftAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public GameSession? GameSession { get; set; }
}

public class SessionEvent
{
    public Guid Id { get; set; }
    public Guid GameSessionId { get; set; }
    public string EventType { get; set; } = string.Empty;
    public string PayloadJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public GameSession? GameSession { get; set; }
}
