/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：配置 LiveOps 统计相关 EF Core 映射、索引、约束和字段长度。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Builders;
using Game.Infrastructure.Database.Entities;

namespace Game.Infrastructure.Database.Configurations;

public class DailyStatsConfiguration : IEntityTypeConfiguration<DailyStats>
{
    public void Configure(EntityTypeBuilder<DailyStats> b)
    {
        b.ToTable("daily_stats");
        b.HasKey(x => x.Date);
        b.Property(x => x.Date).HasColumnName("date");
        b.Property(x => x.NewUsers).HasColumnName("new_users").IsRequired();
        b.Property(x => x.ActiveUsers).HasColumnName("active_users").IsRequired();
        b.Property(x => x.NewAccounts).HasColumnName("new_accounts").IsRequired();
        b.Property(x => x.TotalMatches).HasColumnName("total_matches").IsRequired();
        b.Property(x => x.TotalPlayTimeSeconds).HasColumnName("total_play_time_seconds").IsRequired();
        b.Property(x => x.TotalRevenue).HasColumnName("total_revenue").IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();
        b.Property(x => x.CreatedAt).HasColumnName("created_at").IsRequired();
    }
}

public class RetentionCohortConfiguration : IEntityTypeConfiguration<RetentionCohort>
{
    public void Configure(EntityTypeBuilder<RetentionCohort> b)
    {
        b.ToTable("retention_cohort");
        b.HasKey(x => x.Id);
        b.Property(x => x.Id).HasColumnName("id");
        b.Property(x => x.CohortDate).HasColumnName("cohort_date").IsRequired();
        b.Property(x => x.D0).HasColumnName("d0").IsRequired();
        b.Property(x => x.D1).HasColumnName("d1").IsRequired();
        b.Property(x => x.D3).HasColumnName("d3").IsRequired();
        b.Property(x => x.D7).HasColumnName("d7").IsRequired();
        b.Property(x => x.D14).HasColumnName("d14").IsRequired();
        b.Property(x => x.D30).HasColumnName("d30").IsRequired();
        b.Property(x => x.Region).HasColumnName("region").HasMaxLength(32).IsRequired();

        b.HasIndex(x => x.CohortDate);
        b.HasIndex(x => x.Region);
    }
}