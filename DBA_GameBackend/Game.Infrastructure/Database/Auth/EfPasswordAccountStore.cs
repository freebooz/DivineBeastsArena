/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：查询密码账号，并在事务中更新密码和撤销全部登录会话。
*/

using Game.Application.Auth;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfPasswordAccountStore(GameDbContext db) : IPasswordAccountStore
{
    public Task<PasswordAccountSnapshot?> FindByAccountIdAsync(
        Guid accountId,
        CancellationToken cancellationToken = default)
    {
        return QueryAccounts()
            .FirstOrDefaultAsync(x => x.AccountId == accountId, cancellationToken);
    }

    public Task<PasswordAccountSnapshot?> FindByEmailAsync(
        string email,
        CancellationToken cancellationToken = default)
    {
        return db.Accounts
            .AsNoTracking()
            .Where(x => x.Email == email)
            .Select(x => new PasswordAccountSnapshot(x.Id, x.Status, x.PasswordHash))
            .FirstOrDefaultAsync(cancellationToken);
    }

    public async Task UpdatePasswordAndRevokeSessionsAsync(
        Guid accountId,
        string newPasswordHash,
        CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        if (db.Database.IsRelational())
        {
            await using var transaction = await db.Database.BeginTransactionAsync(cancellationToken);
            var updated = await db.Accounts
                .Where(x => x.Id == accountId)
                .ExecuteUpdateAsync(
                    setters => setters
                        .SetProperty(x => x.PasswordHash, newPasswordHash)
                        .SetProperty(x => x.UpdatedAt, now),
                    cancellationToken);
            if (updated != 1)
                throw new InvalidOperationException("密码更新失败，账号可能已不存在。");

            await db.RefreshTokens
                .Where(x => x.AccountId == accountId && x.RevokedAt == null)
                .ExecuteUpdateAsync(
                    setters => setters.SetProperty(x => x.RevokedAt, now),
                    cancellationToken);
            await transaction.CommitAsync(cancellationToken);
            return;
        }

        var account = await db.Accounts.FirstOrDefaultAsync(x => x.Id == accountId, cancellationToken)
            ?? throw new InvalidOperationException("密码更新失败，账号可能已不存在。");
        account.PasswordHash = newPasswordHash;
        account.UpdatedAt = now;
        var tokens = await db.RefreshTokens
            .Where(x => x.AccountId == accountId && x.RevokedAt == null)
            .ToListAsync(cancellationToken);
        foreach (var token in tokens)
            token.RevokedAt = now;
        await db.SaveChangesAsync(cancellationToken);
    }

    private IQueryable<PasswordAccountSnapshot> QueryAccounts() =>
        db.Accounts
            .AsNoTracking()
            .Select(x => new PasswordAccountSnapshot(x.Id, x.Status, x.PasswordHash));
}
