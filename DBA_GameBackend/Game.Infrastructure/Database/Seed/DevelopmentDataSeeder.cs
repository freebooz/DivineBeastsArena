/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Text.Json;
using BCrypt.Net;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Contracts.Character;

namespace Game.Infrastructure.Database.Seed;

public class DevelopmentDataSeeder
{
    private readonly GameDbContext _context;
    private readonly ILogger<DevelopmentDataSeeder> _logger;

    public DevelopmentDataSeeder(GameDbContext context, ILogger<DevelopmentDataSeeder> logger)
    {
        _context = context;
        _logger = logger;
    }

    public async Task SeedAsync(CancellationToken cancellationToken = default)
    {
        if (_context.Database.IsRelational())
        {
            await SeedAccountsAsync(cancellationToken);
            await SeedAdminUsersAsync(cancellationToken);
            await SeedPlayerProfilesAsync(cancellationToken); // Profiles BEFORE identities (FK dependency)
            await SeedPlayerIdentitiesAsync(cancellationToken);
            await SeedPlayerCharactersAsync(cancellationToken);
            await SeedBanRecordsAsync(cancellationToken);
            await SeedGameConfigsAsync(cancellationToken);
            await SeedPortAllocationsAsync(cancellationToken);
            await SeedGameRoomsAsync(cancellationToken);
            await SeedGameSessionsAsync(cancellationToken);
            await SeedMatchResultsAsync(cancellationToken);
            await SeedOperationDataAsync(cancellationToken);
            await SeedPlayerFeedbackAsync(cancellationToken);
            await SeedCrashReportsAsync(cancellationToken);
            await SeedCompleteApiMockDataAsync(cancellationToken);
        }

        _logger.LogInformation("Development data seeding completed");
    }

    private async Task SeedAccountsAsync(CancellationToken cancellationToken)
    {
        var accounts = GetAccountSeeds();
        foreach (var account in accounts)
        {
            await UpsertAsync(_context.Accounts, a => a.Id == account.Id, account, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedAdminUsersAsync(CancellationToken cancellationToken)
    {
        var admins = GetAdminUserSeeds();
        foreach (var admin in admins)
        {
            await UpsertAsync(_context.AdminUsers, a => a.Id == admin.Id, admin, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedPlayerIdentitiesAsync(CancellationToken cancellationToken)
    {
        var identities = GetPlayerIdentitySeeds();
        foreach (var identity in identities)
        {
            await UpsertAsync(_context.PlayerIdentities, p => p.Id == identity.Id, identity, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedPlayerProfilesAsync(CancellationToken cancellationToken)
    {
        var profiles = GetPlayerProfileSeeds();
        foreach (var profile in profiles)
        {
            await UpsertAsync(_context.PlayerProfiles, p => p.PlayerId == profile.PlayerId, profile, cancellationToken);
        }

        var settings = GetPlayerSettingsSeeds();
        foreach (var setting in settings)
        {
            await UpsertAsync(_context.PlayerSettings, s => s.PlayerId == setting.PlayerId, setting, cancellationToken);
        }

        var statistics = GetPlayerStatisticsSeeds();
        foreach (var stat in statistics)
        {
            await UpsertAsync(_context.PlayerStatistics, s => s.PlayerId == stat.PlayerId, stat, cancellationToken);
        }

        var unlocks = GetPlayerUnlockSeeds();
        foreach (var unlock in unlocks)
        {
            await UpsertAsync(_context.PlayerUnlocks,
                u => u.PlayerId == unlock.PlayerId && u.UnlockType == unlock.UnlockType && u.UnlockId == unlock.UnlockId,
                unlock, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedBanRecordsAsync(CancellationToken cancellationToken)
    {
        var bans = GetBanRecordSeeds();
        foreach (var ban in bans)
        {
            await UpsertAsync(_context.BanRecords, b => b.Id == ban.Id, ban, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedPlayerCharactersAsync(CancellationToken cancellationToken)
    {
        var characters = GetPlayerCharacterSeeds();
        foreach (var character in characters)
        {
            await UpsertAsync(_context.PlayerCharacters, c => c.Id == character.Id, character, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedGameConfigsAsync(CancellationToken cancellationToken)
    {
        var configs = GetGameConfigSeeds();
        foreach (var config in configs)
        {
            await UpsertAsync(_context.GameConfigs,
                c => c.ConfigKey == config.ConfigKey && c.Version == config.Version,
                config, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedPortAllocationsAsync(CancellationToken cancellationToken)
    {
        var allocations = GetPortAllocationSeeds();
        foreach (var allocation in allocations)
        {
            await UpsertAsync(_context.PortAllocations, p => p.Port == allocation.Port, allocation, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedGameRoomsAsync(CancellationToken cancellationToken)
    {
        var rooms = GetGameRoomSeeds();
        foreach (var room in rooms)
        {
            await UpsertAsync(_context.GameRooms, r => r.Id == room.Id, room, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedGameSessionsAsync(CancellationToken cancellationToken)
    {
        var sessions = GetGameSessionSeeds();
        foreach (var session in sessions)
        {
            await UpsertAsync(_context.GameSessions, s => s.Id == session.Id, session, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedOperationDataAsync(CancellationToken cancellationToken)
    {
        foreach (var version in GetClientVersionSeeds())
        {
            await UpsertAsync(_context.ClientVersions, v => v.Id == version.Id, version, cancellationToken);
        }

        foreach (var announcement in GetAnnouncementSeeds())
        {
            await UpsertAsync(_context.Announcements, a => a.Id == announcement.Id, announcement, cancellationToken);
        }

        foreach (var gameEvent in GetGameEventSeeds())
        {
            await UpsertAsync(_context.GameEvents, e => e.Id == gameEvent.Id, gameEvent, cancellationToken);
        }

        foreach (var report in GetReportSeeds())
        {
            await UpsertAsync(_context.Reports, r => r.Id == report.Id, report, cancellationToken);
        }

        foreach (var ticket in GetSupportTicketSeeds())
        {
            await UpsertAsync(_context.SupportTickets, t => t.Id == ticket.Id, ticket, cancellationToken);
        }

        foreach (var stats in GetDailyStatsSeeds())
        {
            await UpsertAsync(_context.DailyStats, s => s.Date == stats.Date, stats, cancellationToken);
        }

        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedMatchResultsAsync(CancellationToken cancellationToken)
    {
        var results = GetMatchResultSeeds();
        foreach (var result in results)
        {
            await UpsertAsync(_context.MatchResults, m => m.Id == result.Id, result, cancellationToken);
        }

        var playerResults = GetMatchPlayerResultSeeds();
        foreach (var pr in playerResults)
        {
            await UpsertAsync(_context.MatchPlayerResults, m => m.Id == pr.Id, pr, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedPlayerFeedbackAsync(CancellationToken cancellationToken)
    {
        var feedbacks = GetPlayerFeedbackSeeds();
        foreach (var feedback in feedbacks)
        {
            await UpsertAsync(_context.PlayerFeedbacks, f => f.Id == feedback.Id, feedback, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedCrashReportsAsync(CancellationToken cancellationToken)
    {
        var reports = GetCrashReportSeeds();
        foreach (var report in reports)
        {
            await UpsertAsync(_context.CrashReports, c => c.Id == report.Id, report, cancellationToken);
        }
        await _context.SaveChangesAsync(cancellationToken);
    }

    private async Task SeedCompleteApiMockDataAsync(CancellationToken cancellationToken)
    {
        foreach (var server in GetGameServerInstanceSeeds())
        {
            await UpsertAsync(_context.GameServerInstances, x => x.Id == server.Id, server, cancellationToken);
        }

        foreach (var serverEvent in GetGameServerEventSeeds())
        {
            await UpsertAsync(_context.GameServerEvents, x => x.Id == serverEvent.Id, serverEvent, cancellationToken);
        }

        foreach (var roomPlayer in GetGameRoomPlayerSeeds())
        {
            await UpsertAsync(_context.GameRoomPlayers, x => x.Id == roomPlayer.Id, roomPlayer, cancellationToken);
        }

        foreach (var ticket in GetMatchmakingTicketSeeds())
        {
            await UpsertAsync(_context.MatchmakingTickets, x => x.Id == ticket.Id, ticket, cancellationToken);
        }

        foreach (var playerSession in GetPlayerSessionSeeds())
        {
            await UpsertAsync(_context.PlayerSessions, x => x.Id == playerSession.Id, playerSession, cancellationToken);
        }

        foreach (var sessionEvent in GetSessionEventSeeds())
        {
            await UpsertAsync(_context.SessionEvents, x => x.Id == sessionEvent.Id, sessionEvent, cancellationToken);
        }

        foreach (var item in GetInventoryItemSeeds())
        {
            await UpsertAsync(_context.InventoryItems, x => x.Id == item.Id, item, cancellationToken);
        }

        foreach (var log in GetInventoryLogSeeds())
        {
            await UpsertAsync(_context.InventoryLogs, x => x.Id == log.Id, log, cancellationToken);
        }

        foreach (var balance in GetWalletBalanceSeeds())
        {
            await UpsertAsync(_context.WalletBalances, x => x.Id == balance.Id, balance, cancellationToken);
        }

        foreach (var ledger in GetWalletLedgerSeeds())
        {
            await UpsertAsync(_context.WalletLedgers, x => x.Id == ledger.Id, ledger, cancellationToken);
        }

        foreach (var order in GetOrderRecordSeeds())
        {
            await UpsertAsync(_context.OrderRecords, x => x.Id == order.Id, order, cancellationToken);
        }

        foreach (var ranking in GetPlayerRankingSeeds())
        {
            await UpsertAsync(_context.PlayerRankings, x => x.Id == ranking.Id, ranking, cancellationToken);
        }

        foreach (var request in GetFriendRequestSeeds())
        {
            await UpsertAsync(_context.FriendRequests, x => x.Id == request.Id, request, cancellationToken);
        }

        foreach (var relation in GetFriendRelationSeeds())
        {
            await UpsertAsync(_context.FriendRelations, x => x.Id == relation.Id, relation, cancellationToken);
        }

        foreach (var mail in GetMailSeeds())
        {
            await UpsertAsync(_context.Mails, x => x.Id == mail.Id, mail, cancellationToken);
        }

        foreach (var attachment in GetMailAttachmentSeeds())
        {
            await UpsertAsync(_context.MailAttachments, x => x.Id == attachment.Id, attachment, cancellationToken);
        }

        foreach (var progress in GetPlayerEventProgressSeeds())
        {
            await UpsertAsync(_context.PlayerEventProgresses, x => x.Id == progress.Id, progress, cancellationToken);
        }

        foreach (var achievement in GetAchievementSeeds())
        {
            await UpsertAsync(_context.Achievements, x => x.Id == achievement.Id, achievement, cancellationToken);
        }

        foreach (var playerAchievement in GetPlayerAchievementSeeds())
        {
            await UpsertAsync(_context.PlayerAchievements, x => x.Id == playerAchievement.Id, playerAchievement, cancellationToken);
        }

        foreach (var history in GetPlayerMatchHistorySeeds())
        {
            await UpsertAsync(_context.PlayerMatchHistories, x => x.Id == history.Id, history, cancellationToken);
        }

        foreach (var reply in GetTicketReplySeeds())
        {
            await UpsertAsync(_context.TicketReplies, x => x.Id == reply.Id, reply, cancellationToken);
        }

        foreach (var cohort in GetRetentionCohortSeeds())
        {
            await UpsertAsync(_context.RetentionCohorts, x => x.Id == cohort.Id, cohort, cancellationToken);
        }

        foreach (var log in GetGameConfigPublishLogSeeds())
        {
            await UpsertAsync(_context.GameConfigPublishLogs, x => x.Id == log.Id, log, cancellationToken);
        }

        await _context.SaveChangesAsync(cancellationToken);
    }

    private Task UpsertAsync<T>(DbSet<T> dbSet, Func<T, bool> predicate, T entity, CancellationToken cancellationToken) where T : class
    {
        var existing = dbSet.Local.FirstOrDefault(predicate);
        if (existing == null)
        {
            existing = dbSet.AsQueryable().FirstOrDefault(predicate);
            if (existing != null)
            {
                _context.Entry(existing).CurrentValues.SetValues(entity);
            }
            else
            {
                dbSet.Add(entity);
            }
        }
        else
        {
            _context.Entry(existing).CurrentValues.SetValues(entity);
        }
        return Task.CompletedTask;
    }

    #region Account Seeds

    private static readonly Guid AdminAccountId = Guid.Parse("11111111-1111-1111-1111-111111111111");
    private static readonly Guid OpsAccountId = Guid.Parse("22222222-2222-2222-2222-222222222222");
    private static readonly Guid FrontendDebugAccountId = Guid.Parse("33333333-3333-3333-3333-333333333333");
    private static readonly Guid SteamMockAccountId = Guid.Parse("44444444-4444-4444-4444-444444444444");
    private static readonly Guid EosMockAccountId = Guid.Parse("55555555-5555-5555-5555-555555555555");
    private static readonly Guid TestPlayer003BannedAccountId = Guid.Parse("66666666-6666-6666-6666-666666666666");
    private static readonly Guid[] DevAccountIds =
    {
        Guid.Parse("99990000-0000-0000-0000-000000000001"),
        Guid.Parse("99990000-0000-0000-0000-000000000002"),
        Guid.Parse("99990000-0000-0000-0000-000000000003")
    };

    private static readonly Guid[] PlayerAccountIds = Enumerable.Range(1, 20)
        .Select((i, idx) => Guid.Parse($"77777777-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private static readonly Guid[] TestPlayerAccountIds = Enumerable.Range(1, 5)
        .Select((i, idx) => Guid.Parse($"88888888-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private List<Account> GetAccountSeeds()
    {
        var adminHash = BCrypt.Net.BCrypt.HashPassword("Admin@123456");
        var opsHash = BCrypt.Net.BCrypt.HashPassword("Ops@123456");
        var playerHash = BCrypt.Net.BCrypt.HashPassword("Player@123456");
        var frontendHash = BCrypt.Net.BCrypt.HashPassword("Frontend@123456");
        var steamHash = BCrypt.Net.BCrypt.HashPassword("Steam@123456");
        var eosHash = BCrypt.Net.BCrypt.HashPassword("Eos@123456");
        var testHash = BCrypt.Net.BCrypt.HashPassword("Test@123456");

        var accounts = new List<Account>
        {
            new Account
            {
                Id = AdminAccountId,
                AccountType = "ADMIN",
                Email = "admin@mygameplatform.com",
                PasswordHash = adminHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new Account
            {
                Id = OpsAccountId,
                AccountType = "OPS",
                Email = "ops@mygameplatform.com",
                PasswordHash = opsHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new Account
            {
                Id = FrontendDebugAccountId,
                AccountType = "PLAYER",
                Email = "frontend_debug@mygameplatform.com",
                PasswordHash = frontendHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new Account
            {
                Id = SteamMockAccountId,
                AccountType = "STEAM",
                SteamId = "STEAM_MOCK_001",
                PasswordHash = steamHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new Account
            {
                Id = EosMockAccountId,
                AccountType = "EOS",
                EosId = "EOS_MOCK_001",
                PasswordHash = eosHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new Account
            {
                Id = TestPlayer003BannedAccountId,
                AccountType = "PLAYER",
                Email = "test_player_003_banned@mygameplatform.com",
                PasswordHash = testHash,
                Status = "BANNED",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };

        for (int i = 0; i < 20; i++)
        {
            var playerNum = (i + 1).ToString("000");
            accounts.Add(new Account
            {
                Id = PlayerAccountIds[i],
                AccountType = "PLAYER",
                Email = $"player_{playerNum}@mygameplatform.com",
                PasswordHash = playerHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < 5; i++)
        {
            var playerNum = (i + 1).ToString("000");
            accounts.Add(new Account
            {
                Id = TestPlayerAccountIds[i],
                AccountType = "PLAYER",
                Email = $"test_player_{playerNum}@mygameplatform.com",
                PasswordHash = testHash,
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < DevAccountIds.Length; i++)
        {
            var playerNum = (i + 1).ToString("00");
            accounts.Add(new Account
            {
                Id = DevAccountIds[i],
                AccountType = "DEV",
                Email = $"dba_dev_{playerNum}@mygameplatform.com",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("Dev@123456"),
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        return accounts;
    }

    #endregion

    #region Admin User Seeds

    private List<AdminUser> GetAdminUserSeeds()
    {
        return new List<AdminUser>
        {
            new AdminUser
            {
                Id = Guid.Parse("adad0000-0000-0000-0000-000000000001"),
                Username = "admin",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("Admin@123456"),
                Role = "SUPER_ADMIN",
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new AdminUser
            {
                Id = Guid.Parse("adad0000-0000-0000-0000-000000000002"),
                Username = "ops_admin",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("Ops@123456"),
                Role = "OPS",
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new AdminUser
            {
                Id = Guid.Parse("adad0000-0000-0000-0000-000000000003"),
                Username = "support_admin",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("Support@123456"),
                Role = "SUPPORT",
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new AdminUser
            {
                Id = Guid.Parse("adad0000-0000-0000-0000-000000000004"),
                Username = "viewer_admin",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("Viewer@123456"),
                Role = "VIEWER",
                Status = "ACTIVE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };
    }

    #endregion

    #region Player Identity Seeds

    private static readonly Guid FrontendDebugPlayerId = Guid.Parse("aaaa0000-0000-0000-0000-000000000001");
    private static readonly Guid SteamMockPlayerId = Guid.Parse("aaaa0000-0000-0000-0000-000000000002");
    private static readonly Guid EosMockPlayerId = Guid.Parse("aaaa0000-0000-0000-0000-000000000003");
    private static readonly Guid TestPlayer003BannedPlayerId = Guid.Parse("aaaa0000-0000-0000-0000-000000000004");
    private static readonly Guid[] DevPlayerIds =
    {
        Guid.Parse("aaaa0000-0000-0000-0000-000000000101"),
        Guid.Parse("aaaa0000-0000-0000-0000-000000000102"),
        Guid.Parse("aaaa0000-0000-0000-0000-000000000103")
    };

    private static readonly Guid[] PlayerPlayerIds = Enumerable.Range(1, 20)
        .Select((i, idx) => Guid.Parse($"bbbb0000-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private static readonly Guid[] TestPlayerPlayerIds = Enumerable.Range(1, 5)
        .Select((i, idx) => Guid.Parse($"cccc0000-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private List<PlayerIdentity> GetPlayerIdentitySeeds()
    {
        var identities = new List<PlayerIdentity>
        {
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000001"),
                AccountId = AdminAccountId,
                PlayerId = Guid.Parse("dddd0000-0000-0000-0000-000000000001"),
                DisplayName = "admin",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000002"),
                AccountId = OpsAccountId,
                PlayerId = Guid.Parse("dddd0000-0000-0000-0000-000000000002"),
                DisplayName = "ops",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000003"),
                AccountId = FrontendDebugAccountId,
                PlayerId = FrontendDebugPlayerId,
                DisplayName = "frontend_debug",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000004"),
                AccountId = SteamMockAccountId,
                PlayerId = SteamMockPlayerId,
                DisplayName = "steam_mock_001",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000005"),
                AccountId = EosMockAccountId,
                PlayerId = EosMockPlayerId,
                DisplayName = "eos_mock_001",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerIdentity
            {
                Id = Guid.Parse("dddd0000-0000-0000-0000-000000000006"),
                AccountId = TestPlayer003BannedAccountId,
                PlayerId = TestPlayer003BannedPlayerId,
                DisplayName = "test_player_003_banned",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };

        for (int i = 0; i < 20; i++)
        {
            var playerNum = (i + 1).ToString("000");
            identities.Add(new PlayerIdentity
            {
                Id = Guid.Parse($"dddd0000-0000-0000-0000-{i + 200:000000000000}"),
                AccountId = PlayerAccountIds[i],
                PlayerId = PlayerPlayerIds[i],
                DisplayName = $"player_{playerNum}",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < 5; i++)
        {
            var playerNum = (i + 1).ToString("000");
            identities.Add(new PlayerIdentity
            {
                Id = Guid.Parse($"dddd0000-0000-0000-0000-{i + 500:000000000000}"),
                AccountId = TestPlayerAccountIds[i],
                PlayerId = TestPlayerPlayerIds[i],
                DisplayName = $"test_player_{playerNum}",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < DevAccountIds.Length; i++)
        {
            var playerNum = (i + 1).ToString("00");
            identities.Add(new PlayerIdentity
            {
                Id = Guid.Parse($"dddd0000-0000-0000-0000-{i + 900:000000000000}"),
                AccountId = DevAccountIds[i],
                PlayerId = DevPlayerIds[i],
                DisplayName = $"dba_dev_{playerNum}",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        return identities;
    }

    #endregion

    #region Player Profile Seeds

    private List<PlayerProfile> GetPlayerProfileSeeds()
    {
        var profiles = new List<PlayerProfile>
        {
            new PlayerProfile
            {
                PlayerId = Guid.Parse("dddd0000-0000-0000-0000-000000000001"),
                Nickname = "admin",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerProfile
            {
                PlayerId = Guid.Parse("dddd0000-0000-0000-0000-000000000002"),
                Nickname = "ops",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerProfile
            {
                PlayerId = FrontendDebugPlayerId,
                Nickname = "frontend_debug",
                Level = 10,
                Exp = 7200,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerProfile
            {
                PlayerId = SteamMockPlayerId,
                Nickname = "steam_mock_001",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerProfile
            {
                PlayerId = EosMockPlayerId,
                Nickname = "eos_mock_001",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerProfile
            {
                PlayerId = TestPlayer003BannedPlayerId,
                Nickname = "test_player_003_banned",
                Level = 5,
                Exp = 500,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };

        for (int i = 0; i < 20; i++)
        {
            var playerNum = (i + 1).ToString("000");
            profiles.Add(new PlayerProfile
            {
                PlayerId = PlayerPlayerIds[i],
                Nickname = $"player_{playerNum}",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < 5; i++)
        {
            var playerNum = (i + 1).ToString("000");
            profiles.Add(new PlayerProfile
            {
                PlayerId = TestPlayerPlayerIds[i],
                Nickname = $"test_player_{playerNum}",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        for (int i = 0; i < DevPlayerIds.Length; i++)
        {
            var playerNum = (i + 1).ToString("00");
            profiles.Add(new PlayerProfile
            {
                PlayerId = DevPlayerIds[i],
                Nickname = $"dba_dev_{playerNum}",
                Level = 1,
                Exp = 0,
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            });
        }

        return profiles;
    }

    private List<PlayerSettings> GetPlayerSettingsSeeds()
    {
        var settings = new List<PlayerSettings>
        {
            new PlayerSettings
            {
                PlayerId = FrontendDebugPlayerId,
                SettingsJson = JsonSerializer.Serialize(new
                {
                    graphics_quality = "ULTRA",
                    audio_enabled = true,
                    music_volume = 80,
                    sfx_volume = 100,
                    push_notifications = true,
                    language = "en-US"
                }),
                UpdatedAt = DateTimeOffset.UtcNow
            }
        };

        return settings;
    }

    private List<PlayerStatistics> GetPlayerStatisticsSeeds()
    {
        var stats = new List<PlayerStatistics>
        {
            new PlayerStatistics
            {
                PlayerId = FrontendDebugPlayerId,
                TotalMatches = 150,
                Wins = 80,
                Losses = 60,
                Draws = 10,
                Kills = 400,
                Deaths = 200,
                Assists = 150,
                Score = 50000,
                PlayTimeSeconds = 36000,
                UpdatedAt = DateTimeOffset.UtcNow
            }
        };

        return stats;
    }

    private List<PlayerUnlock> GetPlayerUnlockSeeds()
    {
        return new List<PlayerUnlock>
        {
            new PlayerUnlock
            {
                Id = Guid.Parse("eeee0000-0000-0000-0000-000000000001"),
                PlayerId = FrontendDebugPlayerId,
                UnlockType = "CHARACTER",
                UnlockId = "rat",
                Source = "LEVEL_REWARD",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerUnlock
            {
                Id = Guid.Parse("eeee0000-0000-0000-0000-000000000002"),
                PlayerId = FrontendDebugPlayerId,
                UnlockType = "CHARACTER",
                UnlockId = "ox",
                Source = "LEVEL_REWARD",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerUnlock
            {
                Id = Guid.Parse("eeee0000-0000-0000-0000-000000000003"),
                PlayerId = FrontendDebugPlayerId,
                UnlockType = "CHARACTER",
                UnlockId = "tiger",
                Source = "LEVEL_REWARD",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerUnlock
            {
                Id = Guid.Parse("eeee0000-0000-0000-0000-000000000004"),
                PlayerId = FrontendDebugPlayerId,
                UnlockType = "CHARACTER",
                UnlockId = "dragon",
                Source = "LEVEL_REWARD",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new PlayerUnlock
            {
                Id = Guid.Parse("eeee0000-0000-0000-0000-000000000005"),
                PlayerId = FrontendDebugPlayerId,
                UnlockType = "SKIN",
                UnlockId = "dragon_skin_001",
                Source = "PURCHASE",
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };
    }

    private List<PlayerCharacter> GetPlayerCharacterSeeds()
    {
        var createdAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero);
        string CoreAttributes(float maxHealth, float attackPower, float defense, float moveSpeed)
            => JsonSerializer.Serialize(new
            {
                maxHealth,
                attackPower,
                defense,
                moveSpeed,
                maxEnergy = 100,
                energyRegen = 10,
                criticalRate = 0.05f,
                criticalMultiplier = 2.0f
            });

        return new List<PlayerCharacter>
        {
            new PlayerCharacter
            {
                Id = Guid.Parse("90000000-0000-0000-0000-000000000001"),
                PlayerId = DevPlayerIds[0],
                CharacterName = "DevTiger",
                Zodiac = "Tiger",
                PrimaryElement = "Fire",
                FiveCamp = "South",
                FixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId("Tiger", "Fire"),
                CoreAttributesJson = CoreAttributes(1850, 115, 36, 390),
                Level = 1,
                IsSelected = true,
                CreatedAt = createdAt,
                LastUsedAt = createdAt
            },
            new PlayerCharacter
            {
                Id = Guid.Parse("90000000-0000-0000-0000-000000000002"),
                PlayerId = DevPlayerIds[1],
                CharacterName = "DevDragon",
                Zodiac = "Dragon",
                PrimaryElement = "Water",
                FiveCamp = "North",
                FixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId("Dragon", "Water"),
                CoreAttributesJson = CoreAttributes(1900, 105, 42, 380),
                Level = 1,
                IsSelected = true,
                CreatedAt = createdAt,
                LastUsedAt = createdAt
            },
            new PlayerCharacter
            {
                Id = Guid.Parse("90000000-0000-0000-0000-000000000003"),
                PlayerId = DevPlayerIds[2],
                CharacterName = "DevRat",
                Zodiac = "Rat",
                PrimaryElement = "Wood",
                FiveCamp = "East",
                FixedSkillGroupId = CharacterBuildRules.BuildFixedSkillGroupId("Rat", "Wood"),
                CoreAttributesJson = CoreAttributes(1700, 98, 34, 420),
                Level = 1,
                IsSelected = true,
                CreatedAt = createdAt,
                LastUsedAt = createdAt
            }
        };
    }

    #endregion

    #region Ban Record Seeds

    private List<BanRecord> GetBanRecordSeeds()
    {
        return new List<BanRecord>
        {
            new BanRecord
            {
                Id = Guid.Parse("ffff0000-0000-0000-0000-000000000001"),
                AccountId = TestPlayer003BannedAccountId,
                PlayerId = TestPlayer003BannedPlayerId,
                Reason = "Exploiting game mechanics",
                StartsAt = new DateTimeOffset(2026, 4, 1, 0, 0, 0, TimeSpan.Zero),
                EndsAt = new DateTimeOffset(2027, 4, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedBy = AdminAccountId,
                CreatedAt = new DateTimeOffset(2026, 4, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };
    }

    #endregion

    #region Game Config Seeds

    private List<GameConfig> GetGameConfigSeeds()
    {
        return new List<GameConfig>
        {
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000001"),
                ConfigKey = "zodiac_character",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    characters = new[]
                    {
                        new { id = "rat", name = "Rat", element = "wood", skills = new[] { "quick_slash", "shadow_strike" } },
                        new { id = "ox", name = "Ox", element = "earth", skills = new[] { "fortify", "ground_pound" } },
                        new { id = "tiger", name = "Tiger", element = "fire", skills = new[] { "flame_charge", "inferno" } },
                        new { id = "dragon", name = "Dragon", element = "water", skills = new[] { "tidal_wave", "ice_age" } }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000002"),
                ConfigKey = "element_skill",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    elements = new[]
                    {
                        new { id = "wood", damage_multiplier = 1.0, defense_multiplier = 0.8 },
                        new { id = "fire", damage_multiplier = 1.3, defense_multiplier = 0.7 },
                        new { id = "earth", damage_multiplier = 0.9, defense_multiplier = 1.2 },
                        new { id = "water", damage_multiplier = 1.1, defense_multiplier = 1.0 },
                        new { id = "metal", damage_multiplier = 1.2, defense_multiplier = 0.9 }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000003"),
                ConfigKey = "god_skill",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    god_skills = new[]
                    {
                        new { id = "divine_blessing", name = "Divine Blessing", cooldown = 300, effect = "heal_all_allies" },
                        new { id = "thors_judgment", name = "Thor Judgment", cooldown = 450, effect = "lightning_strike" },
                        new { id = "poseidon_rage", name = "Poseidon Rage", cooldown = 400, effect = "flood_area" }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000004"),
                ConfigKey = "map_config",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    maps = new[]
                    {
                        new { id = "arena_01", name = "Classic Arena", max_players = 10, respawn_time = 10 },
                        new { id = "arena_02", name = "Desert Ruins", max_players = 8, respawn_time = 15 },
                        new { id = "arena_03", name = "Frozen Peak", max_players = 6, respawn_time = 20 }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000005"),
                ConfigKey = "match_mode",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    modes = new[]
                    {
                        new { id = "ranked", name = "Ranked Match", min_players = 2, max_players = 10, ranking_enabled = true },
                        new { id = "casual", name = "Casual Match", min_players = 2, max_players = 10, ranking_enabled = false },
                        new { id = "custom", name = "Custom Room", min_players = 1, max_players = 10, ranking_enabled = false }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000006"),
                ConfigKey = "reward_table",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    rewards = new[]
                    {
                        new { type = "win", exp = 100, gold = 50 },
                        new { type = "loss", exp = 30, gold = 20 },
                        new { type = "first_blood", exp = 20, gold = 10 },
                        new { type = "ace", exp = 50, gold = 30 }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000007"),
                ConfigKey = "item_table",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    items = new[]
                    {
                        new { id = "health_potion", name = "Health Potion", price = 100, effect = "restore_100hp" },
                        new { id = "mana_potion", name = "Mana Potion", price = 80, effect = "restore_50mp" },
                        new { id = "strength_buff", name = "Strength Buff", price = 200, effect = "+20attack_60s" }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            },
            new GameConfig
            {
                Id = Guid.Parse("11110000-0000-0000-0000-000000000008"),
                ConfigKey = "bot_config",
                Version = "1.0.0",
                ContentJson = JsonSerializer.Serialize(new
                {
                    bots = new[]
                    {
                        new { difficulty = "easy", reaction_time_ms = 1000, accuracy = 0.6 },
                        new { difficulty = "medium", reaction_time_ms = 500, accuracy = 0.75 },
                        new { difficulty = "hard", reaction_time_ms = 250, accuracy = 0.9 }
                    }
                }),
                Status = "PUBLISHED",
                Channel = "default",
                Region = "global",
                PublishedBy = AdminAccountId,
                PublishedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero),
                CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero)
            }
        };
    }

    #endregion

    #region Port Allocation Seeds

    private List<PortAllocation> GetPortAllocationSeeds()
    {
        var allocations = new List<PortAllocation>();
        for (int port = 7777; port <= 7799; port++)
        {
            allocations.Add(new PortAllocation
            {
                Port = port,
                Status = port % 3 == 0 ? "ALLOCATED" : "FREE",
                AllocatedAt = port % 3 == 0 ? DateTimeOffset.UtcNow : null,
                ReleasedAt = null
            });
        }
        return allocations;
    }

    #endregion

    #region Game Room Seeds

    private static readonly Guid[] GameRoomIds = Enumerable.Range(1, 5)
        .Select((i, idx) => Guid.Parse($"22220000-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private List<GameRoom> GetGameRoomSeeds()
    {
        return new List<GameRoom>
        {
            new GameRoom
            {
                Id = GameRoomIds[0],
                OwnerPlayerId = PlayerPlayerIds[0],
                Mode = "ranked",
                MapId = "arena_01",
                Region = "us-west",
                MaxPlayers = 10,
                Visibility = "PUBLIC",
                Status = "WAITING",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-2),
                UpdatedAt = DateTimeOffset.UtcNow
            },
            new GameRoom
            {
                Id = GameRoomIds[1],
                OwnerPlayerId = PlayerPlayerIds[1],
                Mode = "casual",
                MapId = "arena_02",
                Region = "us-east",
                MaxPlayers = 8,
                Visibility = "PUBLIC",
                Status = "WAITING",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-1),
                UpdatedAt = DateTimeOffset.UtcNow
            },
            new GameRoom
            {
                Id = GameRoomIds[2],
                OwnerPlayerId = PlayerPlayerIds[2],
                Mode = "ranked",
                MapId = "arena_01",
                Region = "eu-west",
                MaxPlayers = 10,
                Visibility = "PRIVATE",
                PasswordHash = BCrypt.Net.BCrypt.HashPassword("roompass123"),
                Status = "WAITING",
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-30),
                UpdatedAt = DateTimeOffset.UtcNow
            },
            new GameRoom
            {
                Id = GameRoomIds[3],
                OwnerPlayerId = PlayerPlayerIds[3],
                Mode = "custom",
                MapId = "arena_03",
                Region = "asia-east",
                MaxPlayers = 6,
                Visibility = "PUBLIC",
                Status = "PLAYING",
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-45),
                UpdatedAt = DateTimeOffset.UtcNow
            },
            new GameRoom
            {
                Id = GameRoomIds[4],
                OwnerPlayerId = PlayerPlayerIds[4],
                Mode = "casual",
                MapId = "arena_02",
                Region = "us-west",
                MaxPlayers = 10,
                Visibility = "PUBLIC",
                Status = "WAITING",
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-15),
                UpdatedAt = DateTimeOffset.UtcNow
            }
        };
    }

    #endregion

    #region Game Session Seeds

    private static readonly Guid[] GameSessionIds = Enumerable.Range(1, 8)
        .Select((i, idx) => Guid.Parse($"33330000-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private List<GameSession> GetGameSessionSeeds()
    {
        return new List<GameSession>
        {
            new GameSession
            {
                Id = GameSessionIds[0],
                SourceType = "MATCHMAKING",
                Mode = "ranked",
                MapId = "arena_01",
                Region = "us-west",
                Status = "ENDED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000001"),
                ServerIp = "192.168.1.101",
                ServerPort = 7777,
                MaxPlayers = 10,
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-3),
                StartedAt = DateTimeOffset.UtcNow.AddHours(-3).AddMinutes(2),
                EndedAt = DateTimeOffset.UtcNow.AddHours(-2).AddMinutes(45)
            },
            new GameSession
            {
                Id = GameSessionIds[1],
                SourceType = "MATCHMAKING",
                Mode = "casual",
                MapId = "arena_02",
                Region = "us-east",
                Status = "ENDED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000002"),
                ServerIp = "192.168.1.102",
                ServerPort = 7778,
                MaxPlayers = 8,
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-2),
                StartedAt = DateTimeOffset.UtcNow.AddHours(-2).AddMinutes(1),
                EndedAt = DateTimeOffset.UtcNow.AddHours(-1).AddMinutes(30)
            },
            new GameSession
            {
                Id = GameSessionIds[2],
                SourceType = "CUSTOM_ROOM",
                Mode = "custom",
                MapId = "arena_03",
                Region = "eu-west",
                Status = "ENDED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000003"),
                ServerIp = "192.168.1.103",
                ServerPort = 7779,
                MaxPlayers = 6,
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-5),
                StartedAt = DateTimeOffset.UtcNow.AddHours(-5).AddMinutes(3),
                EndedAt = DateTimeOffset.UtcNow.AddHours(-4).AddMinutes(20)
            },
            new GameSession
            {
                Id = GameSessionIds[3],
                SourceType = "MATCHMAKING",
                Mode = "ranked",
                MapId = "arena_01",
                Region = "us-west",
                Status = "STARTED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000004"),
                ServerIp = "192.168.1.104",
                ServerPort = 7780,
                MaxPlayers = 10,
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-30),
                StartedAt = DateTimeOffset.UtcNow.AddMinutes(-28),
                AllocatedAt = DateTimeOffset.UtcNow.AddMinutes(-30)
            },
            new GameSession
            {
                Id = GameSessionIds[4],
                SourceType = "MATCHMAKING",
                Mode = "casual",
                MapId = "arena_02",
                Region = "asia-east",
                Status = "CREATED",
                MaxPlayers = 8,
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-5)
            },
            new GameSession
            {
                Id = GameSessionIds[5],
                SourceType = "MATCHMAKING",
                Mode = "ranked",
                MapId = "arena_01",
                Region = "eu-west",
                Status = "ALLOCATED",
                MaxPlayers = 10,
                CreatedAt = DateTimeOffset.UtcNow.AddMinutes(-2),
                AllocatedAt = DateTimeOffset.UtcNow
            },
            new GameSession
            {
                Id = GameSessionIds[6],
                SourceType = "CUSTOM_ROOM",
                Mode = "custom",
                MapId = "arena_03",
                Region = "us-east",
                Status = "ENDED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000005"),
                ServerIp = "192.168.1.105",
                ServerPort = 7782,
                MaxPlayers = 6,
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-1),
                StartedAt = DateTimeOffset.UtcNow.AddDays(-1).AddMinutes(5),
                EndedAt = DateTimeOffset.UtcNow.AddDays(-1).AddHours(1)
            },
            new GameSession
            {
                Id = GameSessionIds[7],
                SourceType = "MATCHMAKING",
                Mode = "casual",
                MapId = "arena_02",
                Region = "us-west",
                Status = "ENDED",
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000006"),
                ServerIp = "192.168.1.106",
                ServerPort = 7783,
                MaxPlayers = 8,
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-2),
                StartedAt = DateTimeOffset.UtcNow.AddDays(-2).AddMinutes(2),
                EndedAt = DateTimeOffset.UtcNow.AddDays(-2).AddMinutes(35)
            }
        };
    }

    #endregion

    #region Match Result Seeds

    private static readonly Guid[] MatchResultIds = Enumerable.Range(1, 10)
        .Select((i, idx) => Guid.Parse($"55550000-0000-0000-0000-{i + 100:000000000000}"))
        .ToArray();

    private List<MatchResult> GetMatchResultSeeds()
    {
        return new List<MatchResult>
        {
            new MatchResult
            {
                Id = MatchResultIds[0],
                SessionId = GameSessionIds[0],
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000001"),
                Mode = "ranked",
                MapId = "arena_01",
                DurationSeconds = 2700,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "blue", score = new { blue = 15, red = 10 } }),
                IdempotencyKey = "match_result_001",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-2)
            },
            new MatchResult
            {
                Id = MatchResultIds[1],
                SessionId = GameSessionIds[1],
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000002"),
                Mode = "casual",
                MapId = "arena_02",
                DurationSeconds = 1800,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "red", score = new { blue = 8, red = 12 } }),
                IdempotencyKey = "match_result_002",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-1)
            },
            new MatchResult
            {
                Id = MatchResultIds[2],
                SessionId = GameSessionIds[2],
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000003"),
                Mode = "custom",
                MapId = "arena_03",
                DurationSeconds = 2400,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "blue", score = new { blue = 10, red = 6 } }),
                IdempotencyKey = "match_result_003",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-4)
            },
            new MatchResult
            {
                Id = MatchResultIds[3],
                SessionId = GameSessionIds[6],
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000005"),
                Mode = "custom",
                MapId = "arena_03",
                DurationSeconds = 3300,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "draw", score = new { blue = 10, red = 10 } }),
                IdempotencyKey = "match_result_004",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
            },
            new MatchResult
            {
                Id = MatchResultIds[4],
                SessionId = GameSessionIds[7],
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000006"),
                Mode = "casual",
                MapId = "arena_02",
                DurationSeconds = 2100,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "blue", score = new { blue = 14, red = 9 } }),
                IdempotencyKey = "match_result_005",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-2)
            },
            new MatchResult
            {
                Id = MatchResultIds[5],
                SessionId = Guid.Parse("66660000-0000-0000-0000-000000000001"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000007"),
                Mode = "ranked",
                MapId = "arena_01",
                DurationSeconds = 2900,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "red", score = new { blue = 11, red = 15 } }),
                IdempotencyKey = "match_result_006",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-3)
            },
            new MatchResult
            {
                Id = MatchResultIds[6],
                SessionId = Guid.Parse("66660000-0000-0000-0000-000000000002"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000008"),
                Mode = "casual",
                MapId = "arena_02",
                DurationSeconds = 1500,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "blue", score = new { blue = 12, red = 7 } }),
                IdempotencyKey = "match_result_007",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-4)
            },
            new MatchResult
            {
                Id = MatchResultIds[7],
                SessionId = Guid.Parse("66660000-0000-0000-0000-000000000003"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000009"),
                Mode = "ranked",
                MapId = "arena_01",
                DurationSeconds = 3100,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "blue", score = new { blue = 16, red = 13 } }),
                IdempotencyKey = "match_result_008",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-5)
            },
            new MatchResult
            {
                Id = MatchResultIds[8],
                SessionId = Guid.Parse("66660000-0000-0000-0000-000000000004"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000010"),
                Mode = "custom",
                MapId = "arena_03",
                DurationSeconds = 2700,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "red", score = new { blue = 9, red = 14 } }),
                IdempotencyKey = "match_result_009",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-6)
            },
            new MatchResult
            {
                Id = MatchResultIds[9],
                SessionId = Guid.Parse("66660000-0000-0000-0000-000000000005"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000011"),
                Mode = "casual",
                MapId = "arena_02",
                DurationSeconds = 1900,
                ResultJson = JsonSerializer.Serialize(new { winner_team = "draw", score = new { blue = 10, red = 10 } }),
                IdempotencyKey = "match_result_010",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-7)
            }
        };
    }

    private List<MatchPlayerResult> GetMatchPlayerResultSeeds()
    {
        var playerResults = new List<MatchPlayerResult>();
        var resultId = 0;
        var playerId = 0;

        foreach (var resultIdEnum in MatchResultIds)
        {
            var numPlayers = 4 + (resultId % 3);
            for (int p = 0; p < numPlayers; p++)
            {
                playerId = (resultId * 2 + p) % 20;
                var isWinner = p < numPlayers / 2;
                playerResults.Add(new MatchPlayerResult
                {
                    Id = Guid.Parse($"77770000-0000-0000-0000-{resultId * 100 + p + 1:000000000000}"),
                    MatchResultId = resultIdEnum,
                    PlayerId = PlayerPlayerIds[playerId],
                    Team = p < numPlayers / 2 ? "blue" : "red",
                    Result = isWinner ? "WIN" : "LOSS",
                    Kills = 2 + (p * 3) % 10,
                    Deaths = 1 + (p * 2) % 8,
                    Assists = 1 + (p * 4) % 12,
                    Score = 1000 + (p * 250) % 2000,
                    ExpDelta = isWinner ? 100 : 30,
                    RewardJson = JsonSerializer.Serialize(new { gold = isWinner ? 50 : 20, exp = isWinner ? 100 : 30 }),
                    CreatedAt = DateTimeOffset.UtcNow.AddDays(-resultId)
                });
            }
            resultId++;
        }

        return playerResults;
    }

    #endregion

    #region Player Feedback Seeds

    private List<PlayerFeedback> GetPlayerFeedbackSeeds()
    {
        var feedbackTypes = new[] { "BUG_REPORT", "SUGGESTION", "ACCOUNT_ISSUE", "GENERAL" };
        var statuses = new[] { "OPEN", "IN_PROGRESS", "RESOLVED", "CLOSED" };

        return new List<PlayerFeedback>
        {
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000001"),
                PlayerId = PlayerPlayerIds[0],
                Nickname = "player_001",
                Email = "player_001@mygameplatform.com",
                FeedbackType = "BUG_REPORT",
                Title = "Game crashes when entering ranked match",
                Content = "The game crashes with error code 0x887A0006 when I try to join a ranked match. This happens consistently on maps arena_01 and arena_02.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-7)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000002"),
                PlayerId = PlayerPlayerIds[1],
                Nickname = "player_002",
                Email = "player_002@mygameplatform.com",
                FeedbackType = "SUGGESTION",
                Title = "Add more character skins",
                Content = "Would love to see more zodiac character skins. Especially for the tiger and dragon characters.",
                Status = "IN_PROGRESS",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-6)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000003"),
                PlayerId = PlayerPlayerIds[2],
                Nickname = "player_003",
                Email = "player_003@mygameplatform.com",
                FeedbackType = "ACCOUNT_ISSUE",
                Title = "Cannot change password",
                Content = "The password change feature sends verification email but the link expires immediately.",
                Status = "RESOLVED",
                HandledBy = AdminAccountId,
                HandledAt = DateTimeOffset.UtcNow.AddDays(-3),
                HandleNote = "Fixed email service configuration. Verification link now valid for 24 hours.",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-5)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000004"),
                PlayerId = PlayerPlayerIds[3],
                Nickname = "player_004",
                Email = "player_004@mygameplatform.com",
                FeedbackType = "BUG_REPORT",
                Title = "Audio desync in matches",
                Content = "Sound effects are out of sync with actions after playing for more than 30 minutes.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-4)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000005"),
                PlayerId = PlayerPlayerIds[4],
                Nickname = "player_005",
                Email = "player_005@mygameplatform.com",
                FeedbackType = "GENERAL",
                Title = "Love the new ranked system",
                Content = "The new matchmaking feels much fairer. Good job on the rating calculations!",
                Status = "CLOSED",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-3)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000006"),
                PlayerId = PlayerPlayerIds[5],
                Nickname = "player_006",
                Email = "player_006@mygameplatform.com",
                FeedbackType = "SUGGESTION",
                Title = "Add spectator mode",
                Content = "Would be great to watch higher-ranked players play in real-time for learning purposes.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-10)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000007"),
                PlayerId = PlayerPlayerIds[6],
                Nickname = "player_007",
                Email = "player_007@mygameplatform.com",
                FeedbackType = "BUG_REPORT",
                Title = "Incorrect gold reward calculation",
                Content = "Winning matches only gives 40 gold instead of the documented 50 gold.",
                Status = "IN_PROGRESS",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-8)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000008"),
                PlayerId = PlayerPlayerIds[7],
                Nickname = "player_008",
                Email = "player_008@mygameplatform.com",
                FeedbackType = "ACCOUNT_ISSUE",
                Title = "Friend list not syncing",
                Content = "Friends added on desktop client don't appear in mobile client and vice versa.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-2)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000009"),
                PlayerId = PlayerPlayerIds[8],
                Nickname = "player_009",
                Email = "player_009@mygameplatform.com",
                FeedbackType = "GENERAL",
                Title = "Server lag issues in EU region",
                Content = "Experiencing high ping (>150ms) during peak hours in EU-West servers.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
            },
            new PlayerFeedback
            {
                Id = Guid.Parse("88880000-0000-0000-0000-000000000010"),
                PlayerId = PlayerPlayerIds[9],
                Nickname = "player_010",
                Email = "player_010@mygameplatform.com",
                FeedbackType = "SUGGESTION",
                Title = "Practice mode against bots",
                Content = "Please add a dedicated practice mode where we can select specific bot difficulties.",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-12)
            }
        };
    }

    #endregion

    #region Operation Seeds

    private List<ClientVersion> GetClientVersionSeeds()
    {
        return new List<ClientVersion>
        {
            new ClientVersion
            {
                Id = Guid.Parse("c1e00000-0000-0000-0000-000000000001"),
                Version = "1.2.5.0",
                Channel = "stable",
                Platform = "Windows",
                DownloadUrl = "http://localhost:8080/downloads/DivineBeastsArena-1.2.5.0.zip",
                Checksum = "0000000000000000000000000000000000000000000000000000000000000000",
                SizeBytes = 2147483648,
                IsMandatory = false,
                IsActive = true,
                ReleaseNotes = "开发环境稳定版本，用于启动器和官网联调。",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-7)
            }
        };
    }

    private List<Announcement> GetAnnouncementSeeds()
    {
        return new List<Announcement>
        {
            new Announcement
            {
                Id = Guid.Parse("a1100000-0000-0000-0000-000000000001"),
                Title = "开发服开放测试",
                Content = "账号、角色、启动器、反馈和运营后台已接入真实 API。",
                Type = "INFO",
                Priority = 100,
                IsActive = true,
                Channel = "default",
                Region = "global",
                StartAt = DateTimeOffset.UtcNow.AddDays(-3),
                EndAt = DateTimeOffset.UtcNow.AddDays(30),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-3)
            }
        };
    }

    private List<GameEvent> GetGameEventSeeds()
    {
        return new List<GameEvent>
        {
            new GameEvent
            {
                Id = Guid.Parse("e7e00000-0000-0000-0000-000000000001"),
                EventKey = "dev_login_reward",
                Title = "开发登录奖励",
                Description = "用于验证活动、奖励和运营后台状态。",
                Type = "LIMITED",
                Status = "ACTIVE",
                RewardsJson = JsonSerializer.Serialize(new[] { new { itemId = "coin", quantity = 500 } }),
                StartAt = DateTimeOffset.UtcNow.AddDays(-3),
                EndAt = DateTimeOffset.UtcNow.AddDays(30),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-3)
            }
        };
    }

    private List<Report> GetReportSeeds()
    {
        return new List<Report>
        {
            new Report
            {
                Id = Guid.Parse("badd0000-0000-0000-0000-000000000001"),
                ReporterId = PlayerPlayerIds[0],
                ReportedPlayerId = PlayerPlayerIds[1],
                ReportType = "CHEATING",
                Content = "疑似异常移动速度，需要运营核查。",
                EvidenceJson = "[]",
                Status = "OPEN",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
            }
        };
    }

    private List<SupportTicket> GetSupportTicketSeeds()
    {
        return new List<SupportTicket>
        {
            new SupportTicket
            {
                Id = Guid.Parse("71c00000-0000-0000-0000-000000000001"),
                PlayerId = PlayerPlayerIds[2],
                TicketType = "ACCOUNT",
                Subject = "角色选择后未进入大厅",
                Content = "用于验证游客登录、选角和创角流程是否正常。",
                Status = "OPEN",
                Priority = "HIGH",
                CreatedAt = DateTimeOffset.UtcNow.AddHours(-8),
                UpdatedAt = DateTimeOffset.UtcNow.AddHours(-8)
            },
            new SupportTicket
            {
                Id = Guid.Parse("71c00000-0000-0000-0000-000000000002"),
                PlayerId = PlayerPlayerIds[3],
                TicketType = "BUG",
                Subject = "技能粒子特效偶发不可见",
                Content = "用于验证客户端技能、投射物和粒子效果回归。",
                Status = "IN_PROGRESS",
                Priority = "NORMAL",
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-2),
                UpdatedAt = DateTimeOffset.UtcNow.AddDays(-1)
            }
        };
    }

    private List<DailyStats> GetDailyStatsSeeds()
    {
        var today = new DateTimeOffset(DateTimeOffset.UtcNow.UtcDateTime.Date, TimeSpan.Zero);
        return new List<DailyStats>
        {
            new DailyStats
            {
                Date = today,
                NewUsers = 12,
                ActiveUsers = 36,
                NewAccounts = 8,
                TotalMatches = 24,
                TotalPlayTimeSeconds = 43200,
                TotalRevenue = 0,
                Region = "global",
                CreatedAt = DateTimeOffset.UtcNow
            }
        };
    }

    #endregion

    #region Crash Report Seeds

    private List<CrashReport> GetCrashReportSeeds()
    {
        return new List<CrashReport>
        {
            new CrashReport
            {
                Id = Guid.Parse("99990000-0000-0000-0000-000000000001"),
                PlayerId = PlayerPlayerIds[10],
                ClientVersion = "1.2.5.0",
                Platform = "Windows",
                CrashType = "GPU_DRIVER_CRASH",
                Title = "Game freezes during character selection",
                Description = "Game freezes with a white flash screen when selecting dragon character. Driver version 531.18.",
                MetadataJson = JsonSerializer.Serialize(new { gpu = "NVIDIA RTX 3080", driver_version = "531.18", os = "Windows 11" }),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-5)
            },
            new CrashReport
            {
                Id = Guid.Parse("99990000-0000-0000-0000-000000000002"),
                PlayerId = PlayerPlayerIds[11],
                ClientVersion = "1.2.5.0",
                Platform = "Windows",
                CrashType = "OUT_OF_MEMORY",
                Title = "Memory crash after extended play",
                Description = "Game crashes after playing for about 2 hours. Task manager shows memory usage climbing to 8GB before crash.",
                MetadataJson = JsonSerializer.Serialize(new { gpu = "AMD RX 6800 XT", driver_version = "23.3.1", os = "Windows 10" }),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-4)
            },
            new CrashReport
            {
                Id = Guid.Parse("99990000-0000-0000-0000-000000000003"),
                PlayerId = PlayerPlayerIds[12],
                ClientVersion = "1.2.4.0",
                Platform = "MacOS",
                CrashType = "METAL_ERROR",
                Title = "Metal backend crash on MacBook Pro",
                Description = "Consistent crash when entering matches on M1 Max MacBook Pro. Error: MTLBufferError domain 1.",
                MetadataJson = JsonSerializer.Serialize(new { gpu = "Apple M1 Max", os_version = "13.2.1" }),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-3)
            },
            new CrashReport
            {
                Id = Guid.Parse("99990000-0000-0000-0000-000000000004"),
                PlayerId = PlayerPlayerIds[13],
                ClientVersion = "1.2.5.0",
                Platform = "Linux",
                CrashType = "SEGFAULT",
                Title = "Segfault in audio subsystem",
                Description = "Random segfaults in libaudio.so when multiple sound events trigger simultaneously.",
                MetadataJson = JsonSerializer.Serialize(new { os = "Ubuntu 22.04", audio_driver = "ALSA" }),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-2)
            },
            new CrashReport
            {
                Id = Guid.Parse("99990000-0000-0000-0000-000000000005"),
                PlayerId = PlayerPlayerIds[14],
                ClientVersion = "1.2.5.0",
                Platform = "Windows",
                CrashType = "NETWORK_TIMEOUT",
                Title = "Connection crash after network interruption",
                Description = "Game crashes when network connection is temporarily lost for more than 5 seconds.",
                MetadataJson = JsonSerializer.Serialize(new { gpu = "NVIDIA RTX 3060", network_adapter = "Intel AX211" }),
                CreatedAt = DateTimeOffset.UtcNow.AddDays(-1)
            }
        };
    }

    #endregion

    #region Complete API Mock Seeds

    private List<GameServerInstance> GetGameServerInstanceSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        var servers = new List<GameServerInstance>();

        for (int i = 0; i < 8; i++)
        {
            var startedAt = now.AddMinutes(-120 + i * 12);
            var isEnded = i < 3;
            servers.Add(new GameServerInstance
            {
                Id = Guid.Parse($"44440000-0000-0000-0000-{i + 1:000000000000}"),
                SessionId = i < GameSessionIds.Length ? GameSessionIds[i] : null,
                Mode = i % 2 == 0 ? "ranked" : "casual",
                MapId = i % 3 == 0 ? "arena_01" : "arena_02",
                Region = i % 2 == 0 ? "us-west" : "us-east",
                BuildVersion = "1.2.5.0",
                Ip = $"192.168.1.{101 + i}",
                Port = 7777 + i,
                ProcessId = 4100 + i,
                ContainerId = $"dba-server-{i + 1:00}",
                RuntimeTokenHash = $"mock-runtime-token-hash-{i + 1:00}",
                RuntimeTokenExpiresAt = now.AddHours(6),
                Status = isEnded ? "STOPPED" : i == 3 ? "IN_GAME" : "READY",
                StartedAt = startedAt,
                ReadyAt = startedAt.AddSeconds(25),
                AllocatedAt = startedAt.AddMinutes(1),
                EndedAt = isEnded ? startedAt.AddMinutes(45) : null,
                LastHeartbeatAt = isEnded ? startedAt.AddMinutes(45) : now.AddSeconds(-15),
                ExitCode = isEnded ? 0 : null,
                LogPath = $"/var/log/dba/server-{i + 1:00}.log",
                CreatedAt = startedAt,
                UpdatedAt = now
            });
        }

        return servers;
    }

    private List<GameServerEvent> GetGameServerEventSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<GameServerEvent>
        {
            new()
            {
                Id = Guid.Parse("4e400000-0000-0000-0000-000000000001"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000001"),
                EventType = "REGISTERED",
                PayloadJson = JsonSerializer.Serialize(new { buildVersion = "1.2.5.0", port = 7777 }),
                CreatedAt = now.AddHours(-3)
            },
            new()
            {
                Id = Guid.Parse("4e400000-0000-0000-0000-000000000002"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000004"),
                EventType = "HEARTBEAT",
                PayloadJson = JsonSerializer.Serialize(new { players = 6, cpu = 42, memoryMb = 1536 }),
                CreatedAt = now.AddSeconds(-30)
            },
            new()
            {
                Id = Guid.Parse("4e400000-0000-0000-0000-000000000003"),
                ServerId = Guid.Parse("44440000-0000-0000-0000-000000000005"),
                EventType = "READY",
                PayloadJson = JsonSerializer.Serialize(new { readyInSeconds = 24 }),
                CreatedAt = now.AddMinutes(-5)
            }
        };
    }

    private List<GameRoomPlayer> GetGameRoomPlayerSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        var roomPlayers = new List<GameRoomPlayer>();
        for (int roomIndex = 0; roomIndex < GameRoomIds.Length; roomIndex++)
        {
            var playersInRoom = roomIndex == 3 ? 6 : 3;
            for (int slot = 0; slot < playersInRoom; slot++)
            {
                var playerIndex = (roomIndex * 3 + slot) % PlayerPlayerIds.Length;
                roomPlayers.Add(new GameRoomPlayer
                {
                    Id = Guid.Parse($"2222{roomIndex + 1:0000}-0000-0000-0000-{slot + 1:000000000000}"),
                    RoomId = GameRoomIds[roomIndex],
                    PlayerId = PlayerPlayerIds[playerIndex],
                    SlotIndex = slot,
                    Team = slot % 2 == 0 ? "blue" : "red",
                    IsReady = slot == 0 || slot % 2 == 1,
                    JoinedAt = now.AddMinutes(-40 + slot * 3)
                });
            }
        }

        return roomPlayers;
    }

    private List<MatchmakingTicket> GetMatchmakingTicketSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<MatchmakingTicket>
        {
            new()
            {
                Id = Guid.Parse("abc10000-0000-0000-0000-000000000001"),
                PlayerId = PlayerPlayerIds[0],
                Mode = "ranked",
                Region = "us-west",
                Mmr = 1420,
                Status = "MATCHED",
                MatchedSessionId = GameSessionIds[0],
                CreatedAt = now.AddHours(-3),
                UpdatedAt = now.AddHours(-3).AddMinutes(1),
                TimeoutAt = now.AddHours(-3).AddMinutes(5)
            },
            new()
            {
                Id = Guid.Parse("abc10000-0000-0000-0000-000000000002"),
                PlayerId = PlayerPlayerIds[5],
                Mode = "casual",
                Region = "us-west",
                Mmr = 980,
                Status = "QUEUED",
                CreatedAt = now.AddSeconds(-80),
                UpdatedAt = now.AddSeconds(-80),
                TimeoutAt = now.AddMinutes(4)
            },
            new()
            {
                Id = Guid.Parse("abc10000-0000-0000-0000-000000000003"),
                PlayerId = PlayerPlayerIds[6],
                Mode = "ranked",
                Region = "eu-west",
                Mmr = 1510,
                Status = "CANCELLED",
                CreatedAt = now.AddMinutes(-20),
                UpdatedAt = now.AddMinutes(-18),
                CancelledAt = now.AddMinutes(-18),
                TimeoutAt = now.AddMinutes(-15)
            }
        };
    }

    private List<PlayerSession> GetPlayerSessionSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        var sessions = new List<PlayerSession>();
        for (int sessionIndex = 0; sessionIndex < GameSessionIds.Length; sessionIndex++)
        {
            var playerCount = sessionIndex == 4 ? 2 : 4;
            for (int slot = 0; slot < playerCount; slot++)
            {
                var playerIndex = (sessionIndex * 2 + slot) % PlayerPlayerIds.Length;
                var gameSessionId = GameSessionIds[sessionIndex];
                sessions.Add(new PlayerSession
                {
                    Id = Guid.Parse($"3333{sessionIndex + 1:0000}-0000-0000-0000-{slot + 1:000000000000}"),
                    GameSessionId = gameSessionId,
                    PlayerId = PlayerPlayerIds[playerIndex],
                    Team = slot % 2 == 0 ? "blue" : "red",
                    SlotIndex = slot,
                    Status = sessionIndex < 3 || sessionIndex > 5 ? "LEFT" : "CONNECTED",
                    SessionTokenHash = $"mock-session-token-{sessionIndex + 1:00}-{slot + 1:00}",
                    SessionTokenExpiresAt = now.AddHours(2),
                    ReconnectTokenHash = $"mock-reconnect-token-{sessionIndex + 1:00}-{slot + 1:00}",
                    ReconnectTokenExpiresAt = now.AddMinutes(30),
                    JoinedAt = now.AddMinutes(-90 + sessionIndex * 8 + slot),
                    LeftAt = sessionIndex < 3 || sessionIndex > 5 ? now.AddMinutes(-40 + sessionIndex * 8 + slot) : null,
                    CreatedAt = now.AddMinutes(-95 + sessionIndex * 8 + slot)
                });
            }
        }

        return sessions;
    }

    private List<SessionEvent> GetSessionEventSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<SessionEvent>
        {
            new()
            {
                Id = Guid.Parse("5e500000-0000-0000-0000-000000000001"),
                GameSessionId = GameSessionIds[0],
                EventType = "SESSION_CREATED",
                PayloadJson = JsonSerializer.Serialize(new { source = "MATCHMAKING", mode = "ranked" }),
                CreatedAt = now.AddHours(-3)
            },
            new()
            {
                Id = Guid.Parse("5e500000-0000-0000-0000-000000000002"),
                GameSessionId = GameSessionIds[3],
                EventType = "PLAYER_RECONNECTED",
                PayloadJson = JsonSerializer.Serialize(new { playerId = PlayerPlayerIds[7], latencyMs = 42 }),
                CreatedAt = now.AddMinutes(-12)
            },
            new()
            {
                Id = Guid.Parse("5e500000-0000-0000-0000-000000000003"),
                GameSessionId = GameSessionIds[5],
                EventType = "SERVER_ALLOCATED",
                PayloadJson = JsonSerializer.Serialize(new { serverId = "44440000-0000-0000-0000-000000000006" }),
                CreatedAt = now.AddMinutes(-2)
            }
        };
    }

    private List<InventoryItem> GetInventoryItemSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<InventoryItem>
        {
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, ItemId = "coin", Quantity = 2500, CreatedAt = now.AddDays(-20), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000002"), PlayerId = FrontendDebugPlayerId, ItemId = "skin_001", Quantity = 1, CreatedAt = now.AddDays(-10), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000003"), PlayerId = FrontendDebugPlayerId, ItemId = "weapon_001", Quantity = 1, CreatedAt = now.AddDays(-8), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000004"), PlayerId = PlayerPlayerIds[0], ItemId = "coin", Quantity = 1800, CreatedAt = now.AddDays(-12), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000005"), PlayerId = PlayerPlayerIds[0], ItemId = "health_potion", Quantity = 12, CreatedAt = now.AddDays(-5), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000006"), PlayerId = PlayerPlayerIds[1], ItemId = "mana_potion", Quantity = 8, CreatedAt = now.AddDays(-4), UpdatedAt = now },
            new() { Id = Guid.Parse("1a110000-0000-0000-0000-000000000007"), PlayerId = PlayerPlayerIds[2], ItemId = "event_ticket", Quantity = 3, ExpiresAt = now.AddDays(14), CreatedAt = now.AddDays(-2), UpdatedAt = now }
        };
    }

    private List<InventoryLog> GetInventoryLogSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<InventoryLog>
        {
            new() { Id = Guid.Parse("1a120000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, ItemId = "coin", QuantityDelta = 2500, QuantityBefore = 0, QuantityAfter = 2500, Reason = "development grant", BizType = "SEED", BizId = "seed-coin-frontend", CreatedAt = now.AddDays(-20) },
            new() { Id = Guid.Parse("1a120000-0000-0000-0000-000000000002"), PlayerId = FrontendDebugPlayerId, ItemId = "skin_001", QuantityDelta = 1, QuantityBefore = 0, QuantityAfter = 1, Reason = "mock purchase", BizType = "ORDER", BizId = "order-seed-001", CreatedAt = now.AddDays(-10) },
            new() { Id = Guid.Parse("1a120000-0000-0000-0000-000000000003"), PlayerId = PlayerPlayerIds[0], ItemId = "health_potion", QuantityDelta = 12, QuantityBefore = 0, QuantityAfter = 12, Reason = "mail attachment", BizType = "MAIL", BizId = "mail-seed-001", CreatedAt = now.AddDays(-5) },
            new() { Id = Guid.Parse("1a120000-0000-0000-0000-000000000004"), PlayerId = PlayerPlayerIds[2], ItemId = "event_ticket", QuantityDelta = 3, QuantityBefore = 0, QuantityAfter = 3, Reason = "event reward", BizType = "EVENT", BizId = "dev_login_reward", CreatedAt = now.AddDays(-2) }
        };
    }

    private List<WalletBalance> GetWalletBalanceSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<WalletBalance>
        {
            new() { Id = Guid.Parse("cc010000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, CurrencyType = "COIN", Balance = 10000, UpdatedAt = now },
            new() { Id = Guid.Parse("cc010000-0000-0000-0000-000000000002"), PlayerId = FrontendDebugPlayerId, CurrencyType = "GEM", Balance = 500, UpdatedAt = now },
            new() { Id = Guid.Parse("cc010000-0000-0000-0000-000000000003"), PlayerId = PlayerPlayerIds[0], CurrencyType = "COIN", Balance = 6800, UpdatedAt = now },
            new() { Id = Guid.Parse("cc010000-0000-0000-0000-000000000004"), PlayerId = PlayerPlayerIds[1], CurrencyType = "COIN", Balance = 3200, UpdatedAt = now },
            new() { Id = Guid.Parse("cc010000-0000-0000-0000-000000000005"), PlayerId = PlayerPlayerIds[2], CurrencyType = "GEM", Balance = 120, UpdatedAt = now }
        };
    }

    private List<WalletLedger> GetWalletLedgerSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<WalletLedger>
        {
            new() { Id = Guid.Parse("cc020000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, CurrencyType = "COIN", Amount = 10000, BalanceBefore = 0, BalanceAfter = 10000, BizType = "SEED", BizId = "wallet-seed-frontend-coin", IdempotencyKey = "wallet-seed-frontend-coin", CreatedAt = now.AddDays(-20) },
            new() { Id = Guid.Parse("cc020000-0000-0000-0000-000000000002"), PlayerId = FrontendDebugPlayerId, CurrencyType = "GEM", Amount = 500, BalanceBefore = 0, BalanceAfter = 500, BizType = "SEED", BizId = "wallet-seed-frontend-gem", IdempotencyKey = "wallet-seed-frontend-gem", CreatedAt = now.AddDays(-20) },
            new() { Id = Guid.Parse("cc020000-0000-0000-0000-000000000003"), PlayerId = PlayerPlayerIds[0], CurrencyType = "COIN", Amount = -100, BalanceBefore = 6900, BalanceAfter = 6800, BizType = "ORDER", BizId = "order-seed-001", IdempotencyKey = "wallet-order-seed-001", CreatedAt = now.AddDays(-10) }
        };
    }

    private List<OrderRecord> GetOrderRecordSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<OrderRecord>
        {
            new()
            {
                Id = Guid.Parse("0d0e0000-0000-0000-0000-000000000001"),
                PlayerId = FrontendDebugPlayerId,
                Platform = "MOCK",
                PlatformOrderId = "mock-order-frontend-001",
                Status = "COMPLETED",
                Amount = 100,
                Currency = "COIN",
                ItemJson = JsonSerializer.Serialize(new { itemId = "skin_001", quantity = 1 }),
                CreatedAt = now.AddDays(-10),
                PaidAt = now.AddDays(-10).AddSeconds(2),
                CompletedAt = now.AddDays(-10).AddSeconds(3),
                UpdatedAt = now.AddDays(-10).AddSeconds(3)
            },
            new()
            {
                Id = Guid.Parse("0d0e0000-0000-0000-0000-000000000002"),
                PlayerId = PlayerPlayerIds[1],
                Platform = "MOCK",
                PlatformOrderId = "mock-order-player-002",
                Status = "PAID",
                Amount = 300,
                Currency = "USD",
                ItemJson = JsonSerializer.Serialize(new { itemId = "gem_001", quantity = 1 }),
                CreatedAt = now.AddDays(-2),
                PaidAt = now.AddDays(-2).AddSeconds(5),
                UpdatedAt = now.AddDays(-2).AddSeconds(5)
            }
        };
    }

    private List<PlayerRanking> GetPlayerRankingSeeds()
    {
        var rankings = new List<PlayerRanking>();
        for (int i = 0; i < 20; i++)
        {
            rankings.Add(new PlayerRanking
            {
                Id = Guid.Parse($"ba110000-0000-0000-0000-{i + 1:000000000000}"),
                PlayerId = PlayerPlayerIds[i],
                Mode = "ranked",
                Rank = i + 1,
                Rating = 1800 - i * 32,
                TotalMatches = 80 + i * 3,
                Wins = 50 + i,
                Losses = 25 + i,
                Draws = 5,
                WinStreak = Math.Max(0, 8 - i % 9),
                MaxWinStreak = 10 + i % 5,
                UpdatedAt = DateTimeOffset.UtcNow.AddMinutes(-i)
            });
        }

        for (int i = 0; i < 8; i++)
        {
            rankings.Add(new PlayerRanking
            {
                Id = Guid.Parse($"ba120000-0000-0000-0000-{i + 1:000000000000}"),
                PlayerId = PlayerPlayerIds[i],
                Mode = "casual",
                Rank = i + 1,
                Rating = 1300 - i * 18,
                TotalMatches = 30 + i * 4,
                Wins = 18 + i,
                Losses = 10 + i,
                Draws = 2,
                UpdatedAt = DateTimeOffset.UtcNow.AddMinutes(-20 - i)
            });
        }

        return rankings;
    }

    private List<FriendRequest> GetFriendRequestSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<FriendRequest>
        {
            new() { Id = Guid.Parse("f1000000-0000-0000-0000-000000000001"), SenderId = PlayerPlayerIds[2], ReceiverId = FrontendDebugPlayerId, Status = "PENDING", CreatedAt = now.AddHours(-3) },
            new() { Id = Guid.Parse("f1000000-0000-0000-0000-000000000002"), SenderId = PlayerPlayerIds[3], ReceiverId = FrontendDebugPlayerId, Status = "ACCEPTED", CreatedAt = now.AddDays(-3), RespondedAt = now.AddDays(-2) },
            new() { Id = Guid.Parse("f1000000-0000-0000-0000-000000000003"), SenderId = FrontendDebugPlayerId, ReceiverId = PlayerPlayerIds[4], Status = "PENDING", CreatedAt = now.AddHours(-6) }
        };
    }

    private List<FriendRelation> GetFriendRelationSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<FriendRelation>
        {
            new() { Id = Guid.Parse("f2000000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, FriendId = PlayerPlayerIds[0], Alias = "rank partner", CreatedAt = now.AddDays(-6) },
            new() { Id = Guid.Parse("f2000000-0000-0000-0000-000000000002"), PlayerId = PlayerPlayerIds[0], FriendId = FrontendDebugPlayerId, Alias = "frontend", CreatedAt = now.AddDays(-6) },
            new() { Id = Guid.Parse("f2000000-0000-0000-0000-000000000003"), PlayerId = FrontendDebugPlayerId, FriendId = PlayerPlayerIds[1], Alias = "arena mate", CreatedAt = now.AddDays(-5) },
            new() { Id = Guid.Parse("f2000000-0000-0000-0000-000000000004"), PlayerId = PlayerPlayerIds[1], FriendId = FrontendDebugPlayerId, Alias = "debug player", CreatedAt = now.AddDays(-5) }
        };
    }

    private List<Mail> GetMailSeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<Mail>
        {
            new()
            {
                Id = Guid.Parse("aa1a0000-0000-0000-0000-000000000001"),
                ReceiverId = FrontendDebugPlayerId,
                MailType = "SYSTEM",
                Title = "Welcome reward",
                Content = "Development environment starter pack.",
                AttachmentJson = JsonSerializer.Serialize(new[] { new { itemId = "coin", quantity = 500 }, new { itemId = "health_potion", quantity = 5 } }),
                IsRead = false,
                IsDeleted = false,
                ExpiresAt = now.AddDays(30),
                CreatedAt = now.AddDays(-1)
            },
            new()
            {
                Id = Guid.Parse("aa1a0000-0000-0000-0000-000000000002"),
                ReceiverId = FrontendDebugPlayerId,
                MailType = "EVENT",
                Title = "Event compensation",
                Content = "Mock compensation mail for attachment claim API.",
                AttachmentJson = JsonSerializer.Serialize(new[] { new { itemId = "event_ticket", quantity = 2 } }),
                IsRead = true,
                IsDeleted = false,
                ExpiresAt = now.AddDays(14),
                CreatedAt = now.AddDays(-3),
                ReadAt = now.AddDays(-2)
            },
            new()
            {
                Id = Guid.Parse("aa1a0000-0000-0000-0000-000000000003"),
                ReceiverId = PlayerPlayerIds[0],
                MailType = "PERSONAL",
                Title = "Ranked season reminder",
                Content = "Your weekly ranked reward is ready.",
                AttachmentJson = "[]",
                IsRead = false,
                IsDeleted = false,
                CreatedAt = now.AddHours(-8)
            }
        };
    }

    private List<MailAttachment> GetMailAttachmentSeeds()
    {
        return new List<MailAttachment>
        {
            new() { Id = Guid.Parse("aa2a0000-0000-0000-0000-000000000001"), MailId = Guid.Parse("aa1a0000-0000-0000-0000-000000000001"), ItemId = "coin", Quantity = 500, IsClaimed = false },
            new() { Id = Guid.Parse("aa2a0000-0000-0000-0000-000000000002"), MailId = Guid.Parse("aa1a0000-0000-0000-0000-000000000001"), ItemId = "health_potion", Quantity = 5, IsClaimed = false },
            new() { Id = Guid.Parse("aa2a0000-0000-0000-0000-000000000003"), MailId = Guid.Parse("aa1a0000-0000-0000-0000-000000000002"), ItemId = "event_ticket", Quantity = 2, IsClaimed = true, ClaimedAt = DateTimeOffset.UtcNow.AddDays(-1) }
        };
    }

    private List<PlayerEventProgress> GetPlayerEventProgressSeeds()
    {
        return new List<PlayerEventProgress>
        {
            new() { Id = Guid.Parse("e7f00000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, EventId = Guid.Parse("e7e00000-0000-0000-0000-000000000001"), Progress = 3, Target = 7, IsCompleted = false, IsRewarded = false, UpdatedAt = DateTimeOffset.UtcNow },
            new() { Id = Guid.Parse("e7f00000-0000-0000-0000-000000000002"), PlayerId = PlayerPlayerIds[0], EventId = Guid.Parse("e7e00000-0000-0000-0000-000000000001"), Progress = 7, Target = 7, IsCompleted = true, IsRewarded = true, UpdatedAt = DateTimeOffset.UtcNow.AddDays(-1) }
        };
    }

    private List<Achievement> GetAchievementSeeds()
    {
        return new List<Achievement>
        {
            new() { Id = Guid.Parse("ac100000-0000-0000-0000-000000000001"), AchievementKey = "first_match", Title = "First Match", Description = "Complete the first arena match.", Category = "MATCH", Icon = "trophy", MaxProgress = 1, RewardsJson = JsonSerializer.Serialize(new[] { new { itemId = "coin", quantity = 100 } }), Order = 1 },
            new() { Id = Guid.Parse("ac100000-0000-0000-0000-000000000002"), AchievementKey = "ten_wins", Title = "Ten Wins", Description = "Win ten matches.", Category = "MATCH", Icon = "medal", MaxProgress = 10, RewardsJson = JsonSerializer.Serialize(new[] { new { itemId = "gem", quantity = 20 } }), Order = 2 },
            new() { Id = Guid.Parse("ac100000-0000-0000-0000-000000000003"), AchievementKey = "collector", Title = "Collector", Description = "Own five unlockable items.", Category = "COLLECTION", Icon = "box", MaxProgress = 5, RewardsJson = JsonSerializer.Serialize(new[] { new { itemId = "skin_001", quantity = 1 } }), Order = 3 }
        };
    }

    private List<PlayerAchievement> GetPlayerAchievementSeeds()
    {
        return new List<PlayerAchievement>
        {
            new() { Id = Guid.Parse("ac200000-0000-0000-0000-000000000001"), PlayerId = FrontendDebugPlayerId, AchievementId = Guid.Parse("ac100000-0000-0000-0000-000000000001"), Progress = 1, IsUnlocked = true, UnlockedAt = DateTimeOffset.UtcNow.AddDays(-9) },
            new() { Id = Guid.Parse("ac200000-0000-0000-0000-000000000002"), PlayerId = FrontendDebugPlayerId, AchievementId = Guid.Parse("ac100000-0000-0000-0000-000000000002"), Progress = 6, IsUnlocked = false },
            new() { Id = Guid.Parse("ac200000-0000-0000-0000-000000000003"), PlayerId = PlayerPlayerIds[0], AchievementId = Guid.Parse("ac100000-0000-0000-0000-000000000002"), Progress = 10, IsUnlocked = true, UnlockedAt = DateTimeOffset.UtcNow.AddDays(-2) }
        };
    }

    private List<PlayerMatchHistory> GetPlayerMatchHistorySeeds()
    {
        var histories = new List<PlayerMatchHistory>();
        for (int i = 0; i < 12; i++)
        {
            histories.Add(new PlayerMatchHistory
            {
                Id = Guid.Parse($"b8170000-0000-0000-0000-{i + 1:000000000000}"),
                PlayerId = i < 6 ? FrontendDebugPlayerId : PlayerPlayerIds[0],
                SessionId = GameSessionIds[i % GameSessionIds.Length],
                Mode = i % 2 == 0 ? "ranked" : "casual",
                MapId = i % 3 == 0 ? "arena_01" : "arena_02",
                Team = i % 2 == 0 ? "blue" : "red",
                Result = i % 4 == 0 ? "LOSS" : "WIN",
                Kills = 3 + i,
                Deaths = 1 + i % 5,
                Assists = 2 + i % 6,
                Score = 1200 + i * 180,
                DurationSeconds = 1500 + i * 90,
                PlayedAt = DateTimeOffset.UtcNow.AddDays(-i)
            });
        }

        return histories;
    }

    private List<TicketReply> GetTicketReplySeeds()
    {
        var now = DateTimeOffset.UtcNow;
        return new List<TicketReply>
        {
            new() { Id = Guid.Parse("71d00000-0000-0000-0000-000000000001"), TicketId = Guid.Parse("71c00000-0000-0000-0000-000000000001"), PlayerId = PlayerPlayerIds[2], Content = "I still see the issue after restarting the client.", IsInternal = false, CreatedAt = now.AddHours(-7) },
            new() { Id = Guid.Parse("71d00000-0000-0000-0000-000000000002"), TicketId = Guid.Parse("71c00000-0000-0000-0000-000000000001"), AdminId = Guid.Parse("adad0000-0000-0000-0000-000000000003"), Content = "Support is checking the session logs.", IsInternal = false, CreatedAt = now.AddHours(-6) },
            new() { Id = Guid.Parse("71d00000-0000-0000-0000-000000000003"), TicketId = Guid.Parse("71c00000-0000-0000-0000-000000000002"), AdminId = Guid.Parse("adad0000-0000-0000-0000-000000000002"), Content = "Internal note: verify particle quality level on low presets.", IsInternal = true, CreatedAt = now.AddDays(-1) }
        };
    }

    private List<RetentionCohort> GetRetentionCohortSeeds()
    {
        var today = new DateTimeOffset(DateTimeOffset.UtcNow.UtcDateTime.Date, TimeSpan.Zero);
        return new List<RetentionCohort>
        {
            new() { Id = Guid.Parse("2e700000-0000-0000-0000-000000000001"), CohortDate = today.AddDays(-30), D0 = 100, D1 = 72, D3 = 54, D7 = 41, D14 = 32, D30 = 24, Region = "global" },
            new() { Id = Guid.Parse("2e700000-0000-0000-0000-000000000002"), CohortDate = today.AddDays(-14), D0 = 84, D1 = 60, D3 = 46, D7 = 35, D14 = 27, D30 = 0, Region = "us-west" },
            new() { Id = Guid.Parse("2e700000-0000-0000-0000-000000000003"), CohortDate = today.AddDays(-7), D0 = 91, D1 = 66, D3 = 50, D7 = 38, D14 = 0, D30 = 0, Region = "eu-west" }
        };
    }

    private List<GameConfigPublishLog> GetGameConfigPublishLogSeeds()
    {
        return new List<GameConfigPublishLog>
        {
            new() { Id = Guid.Parse("cf910000-0000-0000-0000-000000000001"), ConfigKey = "zodiac_character", FromVersion = null, ToVersion = "1.0.0", OperatorId = AdminAccountId, Reason = "initial development mock config", CreatedAt = new DateTimeOffset(2026, 1, 1, 0, 0, 0, TimeSpan.Zero) },
            new() { Id = Guid.Parse("cf910000-0000-0000-0000-000000000002"), ConfigKey = "match_mode", FromVersion = null, ToVersion = "1.0.0", OperatorId = OpsAccountId, Reason = "ranked and custom room mock setup", CreatedAt = new DateTimeOffset(2026, 1, 2, 0, 0, 0, TimeSpan.Zero) }
        };
    }

    #endregion
}
