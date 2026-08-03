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

public class GameConfigConfiguration : IEntityTypeConfiguration<GameConfig>
{
    public void Configure(EntityTypeBuilder<GameConfig> b)
    {
        b.ToTable("game_config");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.ConfigKey).HasColumnName("config_key").HasMaxLength(128).IsRequired();
        b.Property(x => x.Version).HasColumnName("version").HasMaxLength(64).IsRequired();
        b.Property(x => x.ContentJson).HasColumnName("content_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.Checksum).HasColumnName("checksum").HasMaxLength(128).IsRequired();
        b.Property(x => x.Channel).HasColumnName("channel").HasMaxLength(32).IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();
        b.Property(x => x.MinClientVersion).HasColumnName("min_client_version").HasMaxLength(64);
        b.Property(x => x.MaxClientVersion).HasColumnName("max_client_version").HasMaxLength(64);
        b.Property(x => x.CreatedBy).HasColumnName("created_by");
        b.Property(x => x.PublishedBy).HasColumnName("published_by");
        b.Property(x => x.PublishedAt).HasColumnName("published_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => new { x.ConfigKey, x.Version, x.Channel, x.Region }).IsUnique();
        b.HasIndex(x => x.Status);
        b.HasIndex(x => x.ConfigKey);
    }
}

public class GameConfigPublishLogConfiguration : IEntityTypeConfiguration<GameConfigPublishLog>
{
    public void Configure(EntityTypeBuilder<GameConfigPublishLog> b)
    {
        b.ToTable("game_config_publish_log");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.ConfigKey).HasColumnName("config_key").HasMaxLength(128).IsRequired();
        b.Property(x => x.FromVersion).HasColumnName("from_version").HasMaxLength(64);
        b.Property(x => x.ToVersion).HasColumnName("to_version").HasMaxLength(64).IsRequired();
        b.Property(x => x.OperatorId).HasColumnName("operator_id");
        b.Property(x => x.Reason).HasColumnName("reason");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.ConfigKey);
        b.HasIndex(x => x.CreatedAt);
    }
}

public class GameRoomConfiguration : IEntityTypeConfiguration<GameRoom>
{
    public void Configure(EntityTypeBuilder<GameRoom> b)
    {
        b.ToTable("game_room");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.OwnerPlayerId).HasColumnName("owner_player_id").IsRequired();
        b.Property(x => x.Mode).HasColumnName("mode").HasMaxLength(64).IsRequired();
        b.Property(x => x.MapId).HasColumnName("map_id").HasMaxLength(64).IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();
        b.Property(x => x.MaxPlayers).HasColumnName("max_players").IsRequired();
        b.Property(x => x.Visibility).HasColumnName("visibility").HasMaxLength(32).IsRequired();
        b.Property(x => x.PasswordHash).HasColumnName("password_hash").HasMaxLength(256);
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");
        b.Property(x => x.ClosedAt).HasColumnName("closed_at");

        b.HasIndex(x => x.Status);
        b.HasIndex(x => new { x.Mode, x.Region });
        b.HasIndex(x => x.CreatedAt);

        b.HasMany(x => x.Players).WithOne(x => x.Room).HasForeignKey(x => x.RoomId);
    }
}

public class GameRoomPlayerConfiguration : IEntityTypeConfiguration<GameRoomPlayer>
{
    public void Configure(EntityTypeBuilder<GameRoomPlayer> b)
    {
        b.ToTable("game_room_player");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.RoomId).HasColumnName("room_id").IsRequired();
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.SlotIndex).HasColumnName("slot_index").IsRequired();
        b.Property(x => x.Team).HasColumnName("team").HasMaxLength(32);
        b.Property(x => x.IsReady).HasColumnName("is_ready").IsRequired();
        b.Property(x => x.JoinedAt).HasColumnName("joined_at").IsRequired();
        b.Property(x => x.LeftAt).HasColumnName("left_at");

        b.HasIndex(x => new { x.RoomId, x.PlayerId }).HasFilter("left_at IS NULL");
    }
}

public class MatchmakingTicketConfiguration : IEntityTypeConfiguration<MatchmakingTicket>
{
    public void Configure(EntityTypeBuilder<MatchmakingTicket> b)
    {
        b.ToTable("matchmaking_ticket");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.Mode).HasColumnName("mode").HasMaxLength(64).IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();
        b.Property(x => x.Mmr).HasColumnName("mmr").IsRequired().HasDefaultValue(1000);
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.MatchedSessionId).HasColumnName("matched_session_id");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");
        b.Property(x => x.CancelledAt).HasColumnName("cancelled_at");
        b.Property(x => x.TimeoutAt).HasColumnName("timeout_at");

        b.HasIndex(x => x.PlayerId);
        b.HasIndex(x => x.Status);
        b.HasIndex(x => new { x.Mode, x.Region, x.Status });
    }
}

public class GameSessionConfiguration : IEntityTypeConfiguration<GameSession>
{
    public void Configure(EntityTypeBuilder<GameSession> b)
    {
        b.ToTable("game_session");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.SourceType).HasColumnName("source_type").HasMaxLength(32).IsRequired();
        b.Property(x => x.SourceId).HasColumnName("source_id");
        b.Property(x => x.Mode).HasColumnName("mode").HasMaxLength(64).IsRequired();
        b.Property(x => x.MapId).HasColumnName("map_id").HasMaxLength(64).IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.ServerId).HasColumnName("server_id");
        b.Property(x => x.ServerIp).HasColumnName("server_ip").HasMaxLength(64);
        b.Property(x => x.ServerPort).HasColumnName("server_port");
        b.Property(x => x.BuildVersion).HasColumnName("build_version").HasMaxLength(64);
        b.Property(x => x.MaxPlayers).HasColumnName("max_players").IsRequired();
        b.Property(x => x.RetryCount).HasColumnName("retry_count").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.AllocatedAt).HasColumnName("allocated_at");
        b.Property(x => x.StartedAt).HasColumnName("started_at");
        b.Property(x => x.EndedAt).HasColumnName("ended_at");
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.Status);
        b.HasIndex(x => new { x.Mode, x.Region });
        b.HasIndex(x => new { x.SourceType, x.SourceId })
            .IsUnique()
            .HasFilter("source_id IS NOT NULL")
            .HasDatabaseName("IX_game_session_source_type_source_id");
        b.HasIndex(x => x.CreatedAt);
    }
}

public class PlayerSessionConfiguration : IEntityTypeConfiguration<PlayerSession>
{
    public void Configure(EntityTypeBuilder<PlayerSession> b)
    {
        b.ToTable("player_session");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.GameSessionId).HasColumnName("game_session_id").IsRequired();
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.CharacterId).HasColumnName("character_id");
        b.Property(x => x.Team).HasColumnName("team").HasMaxLength(32);
        b.Property(x => x.SlotIndex).HasColumnName("slot_index");
        b.Property(x => x.Zodiac).HasColumnName("zodiac").HasMaxLength(32);
        b.Property(x => x.PrimaryElement).HasColumnName("primary_element").HasMaxLength(32);
        b.Property(x => x.FiveCamp).HasColumnName("five_camp").HasMaxLength(32);
        b.Property(x => x.FixedSkillGroupId).HasColumnName("fixed_skill_group_id").HasMaxLength(64);
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.SessionTokenHash)
            .HasColumnName("session_token_hash")
            .HasMaxLength(256)
            .IsRequired()
            .IsConcurrencyToken();
        b.Property(x => x.SessionTokenExpiresAt).HasColumnName("session_token_expires_at").IsRequired();
        b.Property(x => x.SessionTokenServerId).HasColumnName("session_token_server_id");
        b.Property(x => x.SessionTokenBuildId).HasColumnName("session_token_build_id").HasMaxLength(64);
        b.Property(x => x.ReconnectTokenHash).HasColumnName("reconnect_token_hash").HasMaxLength(256);
        b.Property(x => x.ReconnectTokenExpiresAt).HasColumnName("reconnect_token_expires_at");
        b.Property(x => x.JoinedAt).HasColumnName("joined_at");
        b.Property(x => x.LeftAt).HasColumnName("left_at").IsConcurrencyToken();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => new { x.GameSessionId, x.PlayerId }).IsUnique();
        b.HasIndex(x => x.CharacterId);
        b.HasIndex(x => x.SessionTokenHash).IsUnique();
    }
}

public class SessionEventConfiguration : IEntityTypeConfiguration<SessionEvent>
{
    public void Configure(EntityTypeBuilder<SessionEvent> b)
    {
        b.ToTable("session_event");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.GameSessionId).HasColumnName("game_session_id").IsRequired();
        b.Property(x => x.EventType).HasColumnName("event_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.PayloadJson).HasColumnName("payload_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
    }
}
