/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 EF Core 实体模型，对应数据库表结构和领域对象的持久化形态。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

namespace Game.Infrastructure.Database.Entities;

public class Account
{
    public Guid Id { get; set; }
    public string AccountType { get; set; } = "DEV";
    public string? Email { get; set; }
    public string? PasswordHash { get; set; }  // For regular accounts
    public string? SteamId { get; set; }
    public string? EosId { get; set; }
    public string Status { get; set; } = "ACTIVE";
    public DateTimeOffset? LastLoginAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }

    public PlayerIdentity? PlayerIdentity { get; set; }
    public ICollection<RefreshToken> RefreshTokens { get; set; } = new List<RefreshToken>();
    public ICollection<DeviceLogin> DeviceLogins { get; set; } = new List<DeviceLogin>();
    public ICollection<BanRecord> BanRecords { get; set; } = new List<BanRecord>();
}

public class PlayerIdentity
{
    public Guid Id { get; set; }
    public Guid AccountId { get; set; }
    public Guid PlayerId { get; set; }
    public string DisplayName { get; set; } = string.Empty;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public Account? Account { get; set; }
    public PlayerProfile? PlayerProfile { get; set; }
}

public class RefreshToken
{
    public Guid Id { get; set; }
    public Guid AccountId { get; set; }
    public string TokenHash { get; set; } = string.Empty;
    public DateTimeOffset ExpiresAt { get; set; }
    public DateTimeOffset? RevokedAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public string? CreatedByIp { get; set; }
    public string? UserAgent { get; set; }

    public Account? Account { get; set; }
}

public class DeviceLogin
{
    public Guid Id { get; set; }
    public Guid AccountId { get; set; }
    public string DeviceIdHash { get; set; } = string.Empty;
    public string? DeviceName { get; set; }
    public string? Platform { get; set; }
    public DateTimeOffset LastLoginAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public Account? Account { get; set; }
}

public class BanRecord
{
    public Guid Id { get; set; }
    public Guid AccountId { get; set; }
    public Guid? PlayerId { get; set; }
    public string Reason { get; set; } = string.Empty;
    public DateTimeOffset StartsAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? EndsAt { get; set; }
    public Guid? CreatedBy { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? RevokedAt { get; set; }
    public Guid? RevokedBy { get; set; }
    public string? RevokeReason { get; set; }

    public Account? Account { get; set; }
}

public class PlayerProfile
{
    public Guid PlayerId { get; set; }
    public string Nickname { get; set; } = string.Empty;
    /** 首次认证是否已由服务端写入游戏玩家名；历史数据迁移为 true，避免覆盖玩家主动改名。 */
    public bool GameNameInitialized { get; set; } = true;
    public string? Avatar { get; set; }
    public int Level { get; set; } = 1;
    public long Exp { get; set; } = 0;
    public DateTimeOffset? NicknameUpdatedAt { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? LastLoginAt { get; set; }
    public DateTimeOffset? UpdatedAt { get; set; }

    public PlayerSettings? Settings { get; set; }
    public PlayerStatistics? Statistics { get; set; }
    public PlayerIdentity? PlayerIdentity { get; set; }
    public ICollection<PlayerUnlock> Unlocks { get; set; } = new List<PlayerUnlock>();
    public ICollection<InventoryItem> InventoryItems { get; set; } = new List<InventoryItem>();
    public ICollection<PlayerCharacter> Characters { get; set; } = new List<PlayerCharacter>();
}

public class PlayerSettings
{
    public Guid PlayerId { get; set; }
    public string SettingsJson { get; set; } = "{}";
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? PlayerProfile { get; set; }
}

public class PlayerStatistics
{
    public Guid PlayerId { get; set; }
    public int TotalMatches { get; set; }
    public int Wins { get; set; }
    public int Losses { get; set; }
    public int Draws { get; set; }
    public int Kills { get; set; }
    public int Deaths { get; set; }
    public int Assists { get; set; }
    public long Score { get; set; }
    public long PlayTimeSeconds { get; set; }
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? PlayerProfile { get; set; }
}

public class PlayerUnlock
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string UnlockType { get; set; } = string.Empty;
    public string UnlockId { get; set; } = string.Empty;
    public string Source { get; set; } = string.Empty;
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;

    public PlayerProfile? PlayerProfile { get; set; }
}

public class PlayerEventLog
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public string EventType { get; set; } = string.Empty;
    public string PayloadJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
}

public class PlayerCharacter
{
    public Guid Id { get; set; }
    public Guid PlayerId { get; set; }
    public Guid ServerId { get; set; }
    public string CharacterName { get; set; } = string.Empty;
    public string NormalizedName { get; set; } = string.Empty;
    public string Zodiac { get; set; } = "Rat";
    public string PrimaryElement { get; set; } = "Water";
    public string FiveCamp { get; set; } = "East";
    public string FixedSkillGroupId { get; set; } = "Rat_Water";
    public string CoreAttributesJson { get; set; } = "{}";
    public int Level { get; set; } = 1;
    public bool IsSelected { get; set; }
    public bool IsDeleted { get; set; }
    public DateTimeOffset? DeletedAt { get; set; }
    public string? CreationIdempotencyKey { get; set; }
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset LastUsedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset? UpdatedAt { get; set; }

    public PlayerProfile? PlayerProfile { get; set; }
    public CharacterAppearance? Appearance { get; set; }
    public CharacterProgress? Progress { get; set; }
}

/** 服务端权威外观快照；仅保存稳定选项 ID，绝不保存 UE 资产路径。 */
public class CharacterAppearance
{
    public Guid CharacterId { get; set; }
    public string RulesVersion { get; set; } = string.Empty;
    public string AppearanceJson { get; set; } = "{}";
    public DateTimeOffset CreatedAt { get; set; } = DateTimeOffset.UtcNow;
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;
    public PlayerCharacter? Character { get; set; }
}

/** 角色进度表；保留 PlayerCharacter.Level 作为既有对局链兼容投影。 */
public class CharacterProgress
{
    public Guid CharacterId { get; set; }
    public int Level { get; set; } = 1;
    public long Experience { get; set; }
    public DateTimeOffset UpdatedAt { get; set; } = DateTimeOffset.UtcNow;
    public PlayerCharacter? Character { get; set; }
}
