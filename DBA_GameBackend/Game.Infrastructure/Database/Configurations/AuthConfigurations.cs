/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：集中配置 EF Core 映射、索引、约束和字段长度，避免实体类混入过多存储细节。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using Game.Infrastructure.Database.Entities;

namespace Game.Infrastructure.Database.Configurations;

public class AccountConfiguration : IEntityTypeConfiguration<Account>
{
    public void Configure(EntityTypeBuilder<Account> b)
    {
        b.ToTable("account");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AccountType).HasColumnName("account_type").HasMaxLength(32).IsRequired();
        b.Property(x => x.Email).HasColumnName("email").HasMaxLength(255);
        b.Property(x => x.PasswordHash).HasColumnName("password_hash").HasMaxLength(256);
        b.Property(x => x.SteamId).HasColumnName("steam_id").HasMaxLength(64);
        b.Property(x => x.EosId).HasColumnName("eos_id").HasMaxLength(128);
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.LastLoginAt).HasColumnName("last_login_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.Email).IsUnique().HasFilter("email IS NOT NULL");
        b.HasIndex(x => x.SteamId).IsUnique().HasFilter("steam_id IS NOT NULL");
        b.HasIndex(x => x.EosId).IsUnique().HasFilter("eos_id IS NOT NULL");
        b.HasIndex(x => x.Status);
        b.HasIndex(x => x.CreatedAt);

        b.HasMany(x => x.RefreshTokens).WithOne(x => x.Account).HasForeignKey(x => x.AccountId);
        b.HasMany(x => x.DeviceLogins).WithOne(x => x.Account).HasForeignKey(x => x.AccountId);
        b.HasMany(x => x.BanRecords).WithOne(x => x.Account).HasForeignKey(x => x.AccountId);
    }
}

public class PlayerIdentityConfiguration : IEntityTypeConfiguration<PlayerIdentity>
{
    public void Configure(EntityTypeBuilder<PlayerIdentity> b)
    {
        b.ToTable("player_identity");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AccountId).HasColumnName("account_id").IsRequired();
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.DisplayName).HasColumnName("display_name").HasMaxLength(64).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.AccountId).IsUnique();
        b.HasIndex(x => x.PlayerId).IsUnique();
        b.HasIndex(x => x.DisplayName).IsUnique();

        b.HasOne(x => x.Account).WithOne(x => x.PlayerIdentity).HasForeignKey<PlayerIdentity>(x => x.AccountId);
        b.HasOne(x => x.PlayerProfile).WithOne(x => x.PlayerIdentity).HasForeignKey<PlayerIdentity>(x => x.PlayerId);
    }
}

public class RefreshTokenConfiguration : IEntityTypeConfiguration<RefreshToken>
{
    public void Configure(EntityTypeBuilder<RefreshToken> b)
    {
        b.ToTable("refresh_token");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AccountId).HasColumnName("account_id").IsRequired();
        b.Property(x => x.TokenHash).HasColumnName("token_hash").HasMaxLength(256).IsRequired();
        b.Property(x => x.ExpiresAt).HasColumnName("expires_at").IsRequired();
        b.Property(x => x.RevokedAt).HasColumnName("revoked_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.CreatedByIp).HasColumnName("created_by_ip").HasMaxLength(64);
        b.Property(x => x.UserAgent).HasColumnName("user_agent");

        b.HasIndex(x => x.TokenHash).IsUnique();
        b.HasIndex(x => x.AccountId);
        b.HasIndex(x => x.ExpiresAt);
    }
}

public class DeviceLoginConfiguration : IEntityTypeConfiguration<DeviceLogin>
{
    public void Configure(EntityTypeBuilder<DeviceLogin> b)
    {
        b.ToTable("device_login");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AccountId).HasColumnName("account_id").IsRequired();
        b.Property(x => x.DeviceIdHash).HasColumnName("device_id_hash").HasMaxLength(256).IsRequired();
        b.Property(x => x.DeviceName).HasColumnName("device_name").HasMaxLength(128);
        b.Property(x => x.Platform).HasColumnName("platform").HasMaxLength(32);
        b.Property(x => x.LastLoginAt).HasColumnName("last_login_at").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.DeviceIdHash).IsUnique();
    }
}

public class BanRecordConfiguration : IEntityTypeConfiguration<BanRecord>
{
    public void Configure(EntityTypeBuilder<BanRecord> b)
    {
        b.ToTable("ban_record");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AccountId).HasColumnName("account_id").IsRequired();
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.Reason).HasColumnName("reason").IsRequired();
        b.Property(x => x.StartsAt).HasColumnName("starts_at").IsRequired();
        b.Property(x => x.EndsAt).HasColumnName("ends_at");
        b.Property(x => x.CreatedBy).HasColumnName("created_by");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.RevokedAt).HasColumnName("revoked_at");
        b.Property(x => x.RevokedBy).HasColumnName("revoked_by");
        b.Property(x => x.RevokeReason).HasColumnName("revoke_reason");

        b.HasIndex(x => x.AccountId);
        b.HasIndex(x => x.PlayerId);
    }
}

public class PlayerProfileConfiguration : IEntityTypeConfiguration<PlayerProfile>
{
    public void Configure(EntityTypeBuilder<PlayerProfile> b)
    {
        b.ToTable("player_profile");
        b.HasKey(x => x.PlayerId);
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.Nickname).HasColumnName("nickname").HasMaxLength(64).IsRequired();
        // 历史账号在迁移后不应被自动改名；新账号由开户存储显式写入 false，首次认证再生成 3-5 个汉字的游戏名。
        // 并发标记同时也是首次自动命名的一次性闸门：数据库更新会携带原值 false，
        // 因而并发认证中只有一个请求能写入最终昵称，其他请求重新读取该结果。
        b.Property(x => x.GameNameInitialized).HasColumnName("game_name_initialized").HasDefaultValue(true).IsConcurrencyToken().IsRequired();
        b.Property(x => x.Avatar).HasColumnName("avatar").HasMaxLength(255);
        b.Property(x => x.Level).HasColumnName("level").IsRequired().HasDefaultValue(1);
        b.Property(x => x.Exp).HasColumnName("exp").IsRequired().HasDefaultValue(0L);
        b.Property(x => x.NicknameUpdatedAt).HasColumnName("nickname_updated_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.LastLoginAt).HasColumnName("last_login_at");
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.Nickname).IsUnique();
        b.HasIndex(x => x.Level);
        b.HasIndex(x => x.LastLoginAt);

        b.HasOne(x => x.Settings).WithOne(x => x.PlayerProfile).HasForeignKey<PlayerSettings>(x => x.PlayerId);
        b.HasOne(x => x.Statistics).WithOne(x => x.PlayerProfile).HasForeignKey<PlayerStatistics>(x => x.PlayerId);
        b.HasMany(x => x.Unlocks).WithOne(x => x.PlayerProfile).HasForeignKey(x => x.PlayerId);
        b.HasMany(x => x.InventoryItems).WithOne(x => x.Player).HasForeignKey(x => x.PlayerId);
        b.HasMany(x => x.Characters).WithOne(x => x.PlayerProfile).HasForeignKey(x => x.PlayerId);
    }
}

public class PlayerSettingsConfiguration : IEntityTypeConfiguration<PlayerSettings>
{
    public void Configure(EntityTypeBuilder<PlayerSettings> b)
    {
        b.ToTable("player_settings");
        b.HasKey(x => x.PlayerId);
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.SettingsJson).HasColumnName("settings_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();
    }
}

public class PlayerStatisticsConfiguration : IEntityTypeConfiguration<PlayerStatistics>
{
    public void Configure(EntityTypeBuilder<PlayerStatistics> b)
    {
        b.ToTable("player_statistics");
        b.HasKey(x => x.PlayerId);
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.TotalMatches).HasColumnName("total_matches").IsRequired();
        b.Property(x => x.Wins).HasColumnName("wins").IsRequired();
        b.Property(x => x.Losses).HasColumnName("losses").IsRequired();
        b.Property(x => x.Draws).HasColumnName("draws").IsRequired();
        b.Property(x => x.Kills).HasColumnName("kills").IsRequired();
        b.Property(x => x.Deaths).HasColumnName("deaths").IsRequired();
        b.Property(x => x.Assists).HasColumnName("assists").IsRequired();
        b.Property(x => x.Score).HasColumnName("score").IsRequired();
        b.Property(x => x.PlayTimeSeconds).HasColumnName("play_time_seconds").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();
    }
}

public class PlayerUnlockConfiguration : IEntityTypeConfiguration<PlayerUnlock>
{
    public void Configure(EntityTypeBuilder<PlayerUnlock> b)
    {
        b.ToTable("player_unlock");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.UnlockType).HasColumnName("unlock_type").HasMaxLength(32).IsRequired();
        b.Property(x => x.UnlockId).HasColumnName("unlock_id").HasMaxLength(128).IsRequired();
        b.Property(x => x.Source).HasColumnName("source").HasMaxLength(64).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => new { x.PlayerId, x.UnlockType, x.UnlockId }).IsUnique();
    }
}

public class PlayerEventLogConfiguration : IEntityTypeConfiguration<PlayerEventLog>
{
    public void Configure(EntityTypeBuilder<PlayerEventLog> b)
    {
        b.ToTable("player_event_log");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.EventType).HasColumnName("event_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.PayloadJson).HasColumnName("payload_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.PlayerId);
        b.HasIndex(x => x.EventType);
        b.HasIndex(x => x.CreatedAt);
    }
}

public class PlayerCharacterConfiguration : IEntityTypeConfiguration<PlayerCharacter>
{
    public void Configure(EntityTypeBuilder<PlayerCharacter> b)
    {
        b.ToTable("characters");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.ServerId).HasColumnName("server_id").IsRequired();
        b.Property(x => x.CharacterName).HasColumnName("character_name").HasMaxLength(24).IsRequired();
        b.Property(x => x.NormalizedName).HasColumnName("normalized_name").HasMaxLength(64).IsRequired();
        b.Property(x => x.Zodiac).HasColumnName("zodiac").HasMaxLength(32).IsRequired();
        b.Property(x => x.PrimaryElement).HasColumnName("primary_element").HasMaxLength(32).IsRequired();
        b.Property(x => x.FiveCamp).HasColumnName("five_camp").HasMaxLength(32).IsRequired();
        b.Property(x => x.FixedSkillGroupId).HasColumnName("fixed_skill_group_id").HasMaxLength(64).IsRequired();
        b.Property(x => x.CoreAttributesJson).HasColumnName("core_attributes_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.Level).HasColumnName("level").IsRequired().HasDefaultValue(1);
        b.Property(x => x.IsSelected).HasColumnName("is_selected").IsRequired().HasDefaultValue(false);
        b.Property(x => x.IsDeleted).HasColumnName("is_deleted").IsRequired().HasDefaultValue(false);
        b.Property(x => x.DeletedAt).HasColumnName("deleted_at");
        b.Property(x => x.CreationIdempotencyKey).HasColumnName("creation_idempotency_key").HasMaxLength(128);
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.LastUsedAt).HasColumnName("last_used_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => new { x.PlayerId, x.ServerId, x.IsDeleted });
        b.HasIndex(x => new { x.ServerId, x.NormalizedName }).IsUnique();
        b.HasIndex(x => new { x.PlayerId, x.ServerId, x.IsSelected });
        b.HasIndex(x => new { x.PlayerId, x.ServerId, x.CreationIdempotencyKey }).IsUnique();
        b.HasOne(x => x.Appearance).WithOne(x => x.Character).HasForeignKey<CharacterAppearance>(x => x.CharacterId);
        b.HasOne(x => x.Progress).WithOne(x => x.Character).HasForeignKey<CharacterProgress>(x => x.CharacterId);
    }
}

public class CharacterAppearanceConfiguration : IEntityTypeConfiguration<CharacterAppearance>
{
    public void Configure(EntityTypeBuilder<CharacterAppearance> b)
    {
        b.ToTable("character_appearances");
        b.HasKey(x => x.CharacterId);
        b.Property(x => x.CharacterId).HasColumnName("character_id");
        b.Property(x => x.RulesVersion).HasColumnName("rules_version").HasMaxLength(32).IsRequired();
        b.Property(x => x.AppearanceJson).HasColumnName("appearance_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();
    }
}

public class CharacterProgressConfiguration : IEntityTypeConfiguration<CharacterProgress>
{
    public void Configure(EntityTypeBuilder<CharacterProgress> b)
    {
        b.ToTable("character_progress");
        b.HasKey(x => x.CharacterId);
        b.Property(x => x.CharacterId).HasColumnName("character_id");
        b.Property(x => x.Level).HasColumnName("level").IsRequired();
        b.Property(x => x.Experience).HasColumnName("experience").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();
    }
}
