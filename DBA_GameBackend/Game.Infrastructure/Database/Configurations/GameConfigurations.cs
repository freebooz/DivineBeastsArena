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

public class GameServerInstanceConfiguration : IEntityTypeConfiguration<GameServerInstance>
{
    public void Configure(EntityTypeBuilder<GameServerInstance> b)
    {
        b.ToTable("game_server_instance");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.SessionId).HasColumnName("session_id");
        b.Property(x => x.Mode).HasColumnName("mode").HasMaxLength(64);
        b.Property(x => x.MapId).HasColumnName("map_id").HasMaxLength(64);
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32);
        b.Property(x => x.BuildVersion).HasColumnName("build_version").HasMaxLength(64);
        b.Property(x => x.Ip).HasColumnName("ip").HasMaxLength(64).IsRequired();
        b.Property(x => x.Port).HasColumnName("port").IsRequired();
        b.Property(x => x.ProcessId).HasColumnName("process_id");
        b.Property(x => x.ContainerId).HasColumnName("container_id").HasMaxLength(128);
        b.Property(x => x.RuntimeTokenHash).HasColumnName("runtime_token_hash").HasMaxLength(256);
        b.Property(x => x.RuntimeTokenExpiresAt).HasColumnName("runtime_token_expires_at");
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.StartedAt).HasColumnName("started_at").IsRequired();
        b.Property(x => x.ReadyAt).HasColumnName("ready_at");
        b.Property(x => x.AllocatedAt).HasColumnName("allocated_at");
        b.Property(x => x.EndedAt).HasColumnName("ended_at");
        b.Property(x => x.LastHeartbeatAt).HasColumnName("last_heartbeat_at");
        b.Property(x => x.ExitCode).HasColumnName("exit_code");
        b.Property(x => x.CrashReason).HasColumnName("crash_reason");
        b.Property(x => x.LogPath).HasColumnName("log_path");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.SessionId);
        b.HasIndex(x => x.Status);
        b.HasIndex(x => x.LastHeartbeatAt);
    }
}

public class GameServerEventConfiguration : IEntityTypeConfiguration<GameServerEvent>
{
    public void Configure(EntityTypeBuilder<GameServerEvent> b)
    {
        b.ToTable("game_server_event");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.ServerId).HasColumnName("server_id").IsRequired();
        b.Property(x => x.EventType).HasColumnName("event_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.PayloadJson).HasColumnName("payload_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
    }
}

public class PortAllocationConfiguration : IEntityTypeConfiguration<PortAllocation>
{
    public void Configure(EntityTypeBuilder<PortAllocation> b)
    {
        b.ToTable("port_allocation");
        b.HasKey(x => x.Port);
        b.Property(x => x.Port).HasColumnName("port");
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.ServerId).HasColumnName("server_id");
        b.Property(x => x.AllocatedAt).HasColumnName("allocated_at");
        b.Property(x => x.ReleasedAt).HasColumnName("released_at");
    }
}

public class MatchResultConfiguration : IEntityTypeConfiguration<MatchResult>
{
    public void Configure(EntityTypeBuilder<MatchResult> b)
    {
        b.ToTable("match_result");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.SessionId).HasColumnName("session_id").IsRequired();
        b.Property(x => x.ServerId).HasColumnName("server_id").IsRequired();
        b.Property(x => x.Mode).HasColumnName("mode").HasMaxLength(64).IsRequired();
        b.Property(x => x.MapId).HasColumnName("map_id").HasMaxLength(64).IsRequired();
        b.Property(x => x.DurationSeconds).HasColumnName("duration_seconds").IsRequired();
        b.Property(x => x.ResultJson).HasColumnName("result_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.IdempotencyKey).HasColumnName("idempotency_key").HasMaxLength(128).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.SessionId).IsUnique();
        b.HasIndex(x => x.IdempotencyKey).IsUnique();
        b.HasMany(x => x.PlayerResults).WithOne(x => x.MatchResult).HasForeignKey(x => x.MatchResultId);
    }
}

public class MatchPlayerResultConfiguration : IEntityTypeConfiguration<MatchPlayerResult>
{
    public void Configure(EntityTypeBuilder<MatchPlayerResult> b)
    {
        b.ToTable("match_player_result");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.MatchResultId).HasColumnName("match_result_id").IsRequired();
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.Team).HasColumnName("team").HasMaxLength(32);
        b.Property(x => x.Result).HasColumnName("result").HasMaxLength(32).IsRequired();
        b.Property(x => x.Kills).HasColumnName("kills").IsRequired();
        b.Property(x => x.Deaths).HasColumnName("deaths").IsRequired();
        b.Property(x => x.Assists).HasColumnName("assists").IsRequired();
        b.Property(x => x.Score).HasColumnName("score").IsRequired();
        b.Property(x => x.ExpDelta).HasColumnName("exp_delta").IsRequired();
        b.Property(x => x.RewardJson).HasColumnName("reward_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => new { x.MatchResultId, x.PlayerId }).IsUnique();
    }
}

public class InventoryItemConfiguration : IEntityTypeConfiguration<InventoryItem>
{
    public void Configure(EntityTypeBuilder<InventoryItem> b)
    {
        b.ToTable("inventory_item");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.ItemId).HasColumnName("item_id").HasMaxLength(128).IsRequired();
        b.Property(x => x.Quantity).HasColumnName("quantity").IsRequired();
        b.Property(x => x.ExpiresAt).HasColumnName("expires_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();

        b.HasIndex(x => x.PlayerId);
    }
}

public class InventoryLogConfiguration : IEntityTypeConfiguration<InventoryLog>
{
    public void Configure(EntityTypeBuilder<InventoryLog> b)
    {
        b.ToTable("inventory_log");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.ItemId).HasColumnName("item_id").HasMaxLength(128).IsRequired();
        b.Property(x => x.QuantityDelta).HasColumnName("quantity_delta").IsRequired();
        b.Property(x => x.QuantityBefore).HasColumnName("quantity_before").IsRequired();
        b.Property(x => x.QuantityAfter).HasColumnName("quantity_after").IsRequired();
        b.Property(x => x.Reason).HasColumnName("reason").HasMaxLength(64).IsRequired();
        b.Property(x => x.BizType).HasColumnName("biz_type").HasMaxLength(64);
        b.Property(x => x.BizId).HasColumnName("biz_id").HasMaxLength(128);
        b.Property(x => x.OperatorId).HasColumnName("operator_id");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.PlayerId);
        b.HasIndex(x => x.BizType);
    }
}

public class AdminUserConfiguration : IEntityTypeConfiguration<AdminUser>
{
    public void Configure(EntityTypeBuilder<AdminUser> b)
    {
        b.ToTable("admin_user");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.Username).HasColumnName("username").HasMaxLength(64).IsRequired();
        b.Property(x => x.PasswordHash).HasColumnName("password_hash").HasMaxLength(256).IsRequired();
        b.Property(x => x.Role).HasColumnName("role").HasMaxLength(32).IsRequired();
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.FailedLoginCount).HasColumnName("failed_login_count").IsRequired();
        b.Property(x => x.LockedUntil).HasColumnName("locked_until");
        b.Property(x => x.LastLoginAt).HasColumnName("last_login_at");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.Username).IsUnique();
    }
}

public class AdminAuditLogConfiguration : IEntityTypeConfiguration<AdminAuditLog>
{
    public void Configure(EntityTypeBuilder<AdminAuditLog> b)
    {
        b.ToTable("admin_audit_log");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.AdminUserId).HasColumnName("admin_user_id");
        b.Property(x => x.Action).HasColumnName("action").HasMaxLength(128).IsRequired();
        b.Property(x => x.TargetType).HasColumnName("target_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.TargetId).HasColumnName("target_id").HasMaxLength(128);
        b.Property(x => x.Reason).HasColumnName("reason");
        b.Property(x => x.BeforeJson).HasColumnName("before_json").HasColumnType("jsonb");
        b.Property(x => x.AfterJson).HasColumnName("after_json").HasColumnType("jsonb");
        b.Property(x => x.IpAddress).HasColumnName("ip_address").HasMaxLength(64);
        b.Property(x => x.UserAgent).HasColumnName("user_agent");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.AdminUserId);
        b.HasIndex(x => x.Action);
        b.HasIndex(x => x.CreatedAt);
    }
}

public class CrashReportConfiguration : IEntityTypeConfiguration<CrashReport>
{
    public void Configure(EntityTypeBuilder<CrashReport> b)
    {
        b.ToTable("crash_report");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.ClientVersion).HasColumnName("client_version").HasMaxLength(64);
        b.Property(x => x.Platform).HasColumnName("platform").HasMaxLength(32);
        b.Property(x => x.CrashType).HasColumnName("crash_type").HasMaxLength(64);
        b.Property(x => x.Title).HasColumnName("title").HasMaxLength(255);
        b.Property(x => x.Description).HasColumnName("description");
        b.Property(x => x.DumpUrl).HasColumnName("dump_url");
        b.Property(x => x.LogUrl).HasColumnName("log_url");
        b.Property(x => x.MetadataJson).HasColumnName("metadata_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
    }
}

public class PlayerFeedbackConfiguration : IEntityTypeConfiguration<PlayerFeedback>
{
    public void Configure(EntityTypeBuilder<PlayerFeedback> b)
    {
        b.ToTable("player_feedback");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id");
        b.Property(x => x.Nickname).HasColumnName("nickname").HasMaxLength(64);
        b.Property(x => x.Email).HasColumnName("email").HasMaxLength(255);
        b.Property(x => x.FeedbackType).HasColumnName("feedback_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.Title).HasColumnName("title").HasMaxLength(255);
        b.Property(x => x.Content).HasColumnName("content").IsRequired();
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");
        b.Property(x => x.HandledBy).HasColumnName("handled_by");
        b.Property(x => x.HandledAt).HasColumnName("handled_at");
        b.Property(x => x.HandleNote).HasColumnName("handle_note");
    }
}

public class OrderRecordConfiguration : IEntityTypeConfiguration<OrderRecord>
{
    public void Configure(EntityTypeBuilder<OrderRecord> b)
    {
        b.ToTable("order_record");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.Platform).HasColumnName("platform").HasMaxLength(32).IsRequired();
        b.Property(x => x.PlatformOrderId).HasColumnName("platform_order_id").HasMaxLength(128);
        b.Property(x => x.Status).HasColumnName("status").HasMaxLength(32).IsRequired();
        b.Property(x => x.Amount).HasColumnName("amount").IsRequired();
        b.Property(x => x.Currency).HasColumnName("currency").HasMaxLength(16).IsRequired();
        b.Property(x => x.ItemJson).HasColumnName("item_json").HasColumnType("jsonb").IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
        b.Property(x => x.PaidAt).HasColumnName("paid_at");
        b.Property(x => x.CompletedAt).HasColumnName("completed_at");
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at");

        b.HasIndex(x => x.PlayerId);
        b.HasIndex(x => x.Status);
    }
}

public class WalletBalanceConfiguration : IEntityTypeConfiguration<WalletBalance>
{
    public void Configure(EntityTypeBuilder<WalletBalance> b)
    {
        b.ToTable("wallet_balance");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.CurrencyType).HasColumnName("currency_type").HasMaxLength(32).IsRequired();
        b.Property(x => x.Balance).HasColumnName("balance").IsRequired();
        b.Property(x => x.UpdatedAt).HasColumnName("updated_at").IsRequired();

        b.HasIndex(x => new { x.PlayerId, x.CurrencyType }).IsUnique();
    }
}

public class WalletLedgerConfiguration : IEntityTypeConfiguration<WalletLedger>
{
    public void Configure(EntityTypeBuilder<WalletLedger> b)
    {
        b.ToTable("wallet_ledger");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.PlayerId).HasColumnName("player_id").IsRequired();
        b.Property(x => x.CurrencyType).HasColumnName("currency_type").HasMaxLength(32).IsRequired();
        b.Property(x => x.Amount).HasColumnName("amount").IsRequired();
        b.Property(x => x.BalanceBefore).HasColumnName("balance_before").IsRequired();
        b.Property(x => x.BalanceAfter).HasColumnName("balance_after").IsRequired();
        b.Property(x => x.BizType).HasColumnName("biz_type").HasMaxLength(64).IsRequired();
        b.Property(x => x.BizId).HasColumnName("biz_id").HasMaxLength(128).IsRequired();
        b.Property(x => x.IdempotencyKey).HasColumnName("idempotency_key").HasMaxLength(128).IsRequired();
        b.Property(x => x.OperatorId).HasColumnName("operator_id");
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();

        b.HasIndex(x => x.IdempotencyKey).IsUnique();
        b.HasIndex(x => x.PlayerId);
    }
}