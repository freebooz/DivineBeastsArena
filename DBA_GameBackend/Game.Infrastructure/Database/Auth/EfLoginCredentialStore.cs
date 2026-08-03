/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：持久化刷新令牌，并原子更新账号最后登录时间。
*/

using Game.Application.Auth;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfLoginCredentialStore(GameDbContext db) : ILoginCredentialStore
{
    public async Task SaveAsync(
        LoginCredentialSubject subject,
        string refreshTokenHash,
        DateTimeOffset refreshTokenExpiresAt,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = subject.AccountId,
            TokenHash = refreshTokenHash,
            ExpiresAt = refreshTokenExpiresAt,
            CreatedAt = now,
            CreatedByIp = string.IsNullOrWhiteSpace(ipAddress) ? null : ipAddress.Trim(),
            UserAgent = string.IsNullOrWhiteSpace(userAgent) ? null : userAgent.Trim()
        });

        await db.Accounts
            .Where(x => x.Id == subject.AccountId)
            .ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.LastLoginAt, now),
                cancellationToken);
        await db.SaveChangesAsync(cancellationToken);
    }
}
