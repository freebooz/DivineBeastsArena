/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Microsoft.EntityFrameworkCore;
using Game.Infrastructure.Database.Entities;

namespace Game.Infrastructure.Database;

public class GameDbContext : DbContext
{
    public GameDbContext(DbContextOptions<GameDbContext> options) : base(options) { }

    public DbSet<Account> Accounts => Set<Account>();
    public DbSet<PlayerIdentity> PlayerIdentities => Set<PlayerIdentity>();
    public DbSet<RefreshToken> RefreshTokens => Set<RefreshToken>();
    public DbSet<DeviceLogin> DeviceLogins => Set<DeviceLogin>();
    public DbSet<BanRecord> BanRecords => Set<BanRecord>();
    public DbSet<PlayerProfile> PlayerProfiles => Set<PlayerProfile>();
    public DbSet<PlayerSettings> PlayerSettings => Set<PlayerSettings>();
    public DbSet<PlayerStatistics> PlayerStatistics => Set<PlayerStatistics>();
    public DbSet<PlayerUnlock> PlayerUnlocks => Set<PlayerUnlock>();
    public DbSet<PlayerEventLog> PlayerEventLogs => Set<PlayerEventLog>();
    public DbSet<PlayerCharacter> PlayerCharacters => Set<PlayerCharacter>();
    public DbSet<GameConfig> GameConfigs => Set<GameConfig>();
    public DbSet<GameConfigPublishLog> GameConfigPublishLogs => Set<GameConfigPublishLog>();
    public DbSet<GameRoom> GameRooms => Set<GameRoom>();
    public DbSet<GameRoomPlayer> GameRoomPlayers => Set<GameRoomPlayer>();
    public DbSet<MatchmakingTicket> MatchmakingTickets => Set<MatchmakingTicket>();
    public DbSet<GameSession> GameSessions => Set<GameSession>();
    public DbSet<PlayerSession> PlayerSessions => Set<PlayerSession>();
    public DbSet<SessionEvent> SessionEvents => Set<SessionEvent>();
    public DbSet<GameServerInstance> GameServerInstances => Set<GameServerInstance>();
    public DbSet<GameServerEvent> GameServerEvents => Set<GameServerEvent>();
    public DbSet<PortAllocation> PortAllocations => Set<PortAllocation>();
    public DbSet<MatchResult> MatchResults => Set<MatchResult>();
    public DbSet<MatchPlayerResult> MatchPlayerResults => Set<MatchPlayerResult>();
    public DbSet<InventoryItem> InventoryItems => Set<InventoryItem>();
    public DbSet<InventoryLog> InventoryLogs => Set<InventoryLog>();
    public DbSet<AdminUser> AdminUsers => Set<AdminUser>();
    public DbSet<AdminAuditLog> AdminAuditLogs => Set<AdminAuditLog>();
    public DbSet<CrashReport> CrashReports => Set<CrashReport>();
    public DbSet<PlayerFeedback> PlayerFeedbacks => Set<PlayerFeedback>();
    public DbSet<OrderRecord> OrderRecords => Set<OrderRecord>();
    public DbSet<WalletBalance> WalletBalances => Set<WalletBalance>();
    public DbSet<WalletLedger> WalletLedgers => Set<WalletLedger>();

    // 游戏功能与运营支撑 / Game feature and live-ops entities
    public DbSet<PlayerRanking> PlayerRankings => Set<PlayerRanking>();
    public DbSet<FriendRequest> FriendRequests => Set<FriendRequest>();
    public DbSet<FriendRelation> FriendRelations => Set<FriendRelation>();
    public DbSet<Mail> Mails => Set<Mail>();
    public DbSet<MailAttachment> MailAttachments => Set<MailAttachment>();
    public DbSet<Announcement> Announcements => Set<Announcement>();
    public DbSet<GameEvent> GameEvents => Set<GameEvent>();
    public DbSet<PlayerEventProgress> PlayerEventProgresses => Set<PlayerEventProgress>();
    public DbSet<Achievement> Achievements => Set<Achievement>();
    public DbSet<PlayerAchievement> PlayerAchievements => Set<PlayerAchievement>();
    public DbSet<PlayerMatchHistory> PlayerMatchHistories => Set<PlayerMatchHistory>();
    public DbSet<Report> Reports => Set<Report>();
    public DbSet<SupportTicket> SupportTickets => Set<SupportTicket>();
    public DbSet<TicketReply> TicketReplies => Set<TicketReply>();
    public DbSet<ClientVersion> ClientVersions => Set<ClientVersion>();
    public DbSet<DailyStats> DailyStats => Set<DailyStats>();
    public DbSet<RetentionCohort> RetentionCohorts => Set<RetentionCohort>();

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        base.OnModelCreating(modelBuilder);
        modelBuilder.ApplyConfigurationsFromAssembly(typeof(GameDbContext).Assembly);
    }
}
