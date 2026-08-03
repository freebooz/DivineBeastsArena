/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：通过 EF Core 提供账号登录所需的只读凭据快照。
*/

using Game.Application.Auth;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfAuthenticationCredentialStore(GameDbContext db)
    : IAuthenticationCredentialStore
{
    public Task<StoredCredentialAccount?> FindRegularAccountAsync(
        string loginName,
        CancellationToken cancellationToken = default)
    {
        return ProjectAccounts(
                QueryAccountEntities()
                    .Where(x => x.DisplayName == loginName || x.Account!.Email == loginName))
            .FirstOrDefaultAsync(cancellationToken);
    }

    public Task<StoredCredentialAccount?> FindDevelopmentAccountAsync(
        string displayName,
        CancellationToken cancellationToken = default)
    {
        return ProjectAccounts(
                QueryAccountEntities()
                    .Where(x => x.DisplayName == displayName))
            .FirstOrDefaultAsync(cancellationToken);
    }

    private IQueryable<PlayerIdentity> QueryAccountEntities()
    {
        return db.PlayerIdentities
            .AsNoTracking()
            .Where(x => x.Account != null);
    }

    private static IQueryable<StoredCredentialAccount> ProjectAccounts(
        IQueryable<PlayerIdentity> accounts)
    {
        return accounts
            .Select(x => new StoredCredentialAccount(
                x.AccountId,
                x.PlayerId,
                x.DisplayName,
                x.Account!.AccountType,
                x.Account.Status,
                x.Account.Email,
                x.Account.PasswordHash));
    }
}
