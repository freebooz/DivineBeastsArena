/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：持久化访客与正式账号初始数据，并依赖数据库唯一约束处理并发创建。
*/

using Game.Application.Auth;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using Npgsql;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfAccountOnboardingStore(GameDbContext db) : IAccountOnboardingStore
{
    public async Task<AccountOnboardingResult?> ResolveGuestAsync(
        string deviceHash,
        string? deviceName,
        string? platform,
        CancellationToken cancellationToken = default)
    {
        var login = await db.DeviceLogins
            .Include(x => x.Account).ThenInclude(x => x!.PlayerIdentity)
            .FirstOrDefaultAsync(x => x.DeviceIdHash == deviceHash, cancellationToken);
        if (login?.Account?.PlayerIdentity == null)
            return null;
        if (string.Equals(login.Account.Status, "BANNED", StringComparison.OrdinalIgnoreCase))
            return new AccountOnboardingResult(AccountOnboardingStatus.AccountBanned);

        login.LastLoginAt = DateTimeOffset.UtcNow;
        if (deviceName != null)
            login.DeviceName = deviceName;
        if (platform != null)
            login.Platform = platform;
        await db.SaveChangesAsync(cancellationToken);
        return Success(login.Account, login.Account.PlayerIdentity);
    }

    public async Task<AccountOnboardingResult> CreateGuestAsync(
        GuestAccountDraft draft,
        CancellationToken cancellationToken = default)
    {
        AddAccountGraph(
            draft.AccountId,
            draft.IdentityId,
            draft.PlayerId,
            draft.DisplayName,
            draft.AccountType,
            draft.AccountStatus,
            null,
            null,
            draft.InitialLevel,
            draft.InitialExperience,
            draft.InitialSettingsJson,
            draft.CreatedAt);
        db.DeviceLogins.Add(new DeviceLogin
        {
            Id = draft.DeviceLoginId,
            AccountId = draft.AccountId,
            DeviceIdHash = draft.DeviceHash,
            DeviceName = draft.DeviceName,
            Platform = draft.Platform,
            LastLoginAt = draft.CreatedAt,
            CreatedAt = draft.CreatedAt
        });

        try
        {
            await db.SaveChangesAsync(cancellationToken);
            return Success(draft.AccountId, draft.PlayerId, draft.DisplayName, draft.AccountType);
        }
        catch (DbUpdateException exception) when (IsUniqueViolation(exception))
        {
            db.ChangeTracker.Clear();
            return await ResolveGuestAsync(
                    draft.DeviceHash,
                    draft.DeviceName,
                    draft.Platform,
                    cancellationToken)
                ?? new AccountOnboardingResult(AccountOnboardingStatus.DuplicateIdentity);
        }
    }

    public async Task<AccountOnboardingResult> RegisterAsync(
        RegisteredAccountDraft draft,
        CancellationToken cancellationToken = default)
    {
        AddAccountGraph(
            draft.AccountId,
            draft.IdentityId,
            draft.PlayerId,
            draft.DisplayName,
            draft.AccountType,
            draft.AccountStatus,
            draft.Email,
            draft.PasswordHash,
            draft.InitialLevel,
            draft.InitialExperience,
            draft.InitialSettingsJson,
            draft.CreatedAt);

        try
        {
            await db.SaveChangesAsync(cancellationToken);
            return Success(draft.AccountId, draft.PlayerId, draft.DisplayName, draft.AccountType);
        }
        catch (DbUpdateException exception) when (IsUniqueViolation(exception))
        {
            db.ChangeTracker.Clear();
            return new AccountOnboardingResult(AccountOnboardingStatus.DuplicateIdentity);
        }
    }

    private void AddAccountGraph(
        Guid accountId,
        Guid identityId,
        Guid playerId,
        string displayName,
        string accountType,
        string accountStatus,
        string? email,
        string? passwordHash,
        int initialLevel,
        long initialExperience,
        string initialSettingsJson,
        DateTimeOffset now)
    {
        db.Accounts.Add(new Account
        {
            Id = accountId,
            AccountType = accountType,
            Status = accountStatus,
            Email = email,
            PasswordHash = passwordHash,
            LastLoginAt = now,
            CreatedAt = now
        });
        db.PlayerIdentities.Add(new PlayerIdentity
        {
            Id = identityId,
            AccountId = accountId,
            PlayerId = playerId,
            DisplayName = displayName,
            CreatedAt = now
        });
        db.PlayerProfiles.Add(new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = displayName,
            // 此处仅保存账号身份的临时展示值以满足既有非空/唯一索引；AuthService 在本次首次登录
            // 签发响应前会调用 IPlayerService 确保生成真正的 3-5 个汉字游戏玩家名。
            GameNameInitialized = false,
            Level = initialLevel,
            Exp = initialExperience,
            CreatedAt = now
        });
        db.PlayerStatistics.Add(new PlayerStatistics
        {
            PlayerId = playerId,
            UpdatedAt = now
        });
        db.PlayerSettings.Add(new PlayerSettings
        {
            PlayerId = playerId,
            SettingsJson = initialSettingsJson,
            UpdatedAt = now
        });
    }

    private static bool IsUniqueViolation(DbUpdateException exception) =>
        exception.InnerException is PostgresException
        {
            SqlState: PostgresErrorCodes.UniqueViolation
        };

    private static AccountOnboardingResult Success(Account account, PlayerIdentity identity) =>
        Success(account.Id, identity.PlayerId, identity.DisplayName, account.AccountType);

    private static AccountOnboardingResult Success(
        Guid accountId,
        Guid playerId,
        string displayName,
        string accountType) =>
        new(
            AccountOnboardingStatus.Success,
            new LoginCredentialSubject(accountId, playerId, displayName, accountType));
}
