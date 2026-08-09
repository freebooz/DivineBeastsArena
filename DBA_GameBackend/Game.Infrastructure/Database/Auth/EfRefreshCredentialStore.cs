/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：在同一数据库事务中原子消费旧刷新令牌并保存新令牌，同时处理登出撤销。
*/

using Game.Application.Auth;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Infrastructure.Database.Auth;

public sealed class EfRefreshCredentialStore(
    GameDbContext db,
    ILoginCredentialIssuer issuer) : IRefreshCredentialRotationStore, ILogoutCredentialStore
{
    public async Task<RefreshCredentialResult> RotateAsync(
        string refreshTokenHash,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken = default)
    {
        return db.Database.IsRelational()
            ? await RotateRelationalAsync(refreshTokenHash, ipAddress, userAgent, cancellationToken)
            : await RotateNonRelationalAsync(refreshTokenHash, ipAddress, userAgent, cancellationToken);
    }

    public async Task RevokeAsync(
        Guid accountId,
        string? refreshTokenHash,
        CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        var query = db.RefreshTokens
            .Where(x => x.AccountId == accountId && x.RevokedAt == null);
        if (!string.IsNullOrEmpty(refreshTokenHash))
            query = query.Where(x => x.TokenHash == refreshTokenHash);

        if (db.Database.IsRelational())
        {
            await query.ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.RevokedAt, now),
                cancellationToken);
            return;
        }

        var tokens = await query.ToListAsync(cancellationToken);
        foreach (var token in tokens)
            token.RevokedAt = now;
        await db.SaveChangesAsync(cancellationToken);
    }

    private async Task<RefreshCredentialResult> RotateRelationalAsync(
        string refreshTokenHash,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken)
    {
        await using var transaction = await db.Database.BeginTransactionAsync(cancellationToken);
        var now = DateTimeOffset.UtcNow;
        var candidate = await FindCandidateAsync(refreshTokenHash, now, cancellationToken);
        if (candidate == null)
        {
            if (await DetectAndRevokeRefreshTokenReuseAsync(refreshTokenHash, now, cancellationToken))
            {
                await transaction.CommitAsync(cancellationToken);
                return Reused();
            }

            return Invalid();
        }
        if (IsDisabled(candidate))
            return Disabled();

        var consumed = await db.RefreshTokens
            .Where(x => x.Id == candidate.TokenId && x.RevokedAt == null && x.ExpiresAt > now)
            .ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.RevokedAt, now),
                cancellationToken);
        if (consumed != 1)
        {
            await transaction.RollbackAsync(cancellationToken);
            return Invalid();
        }

        var credentials = issuer.Issue(candidate.Subject);
        AddRefreshToken(candidate.Subject.AccountId, credentials, ipAddress, userAgent, now);
        await db.Accounts
            .Where(x => x.Id == candidate.Subject.AccountId)
            .ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.LastLoginAt, now),
                cancellationToken);
        await db.SaveChangesAsync(cancellationToken);
        await transaction.CommitAsync(cancellationToken);
        return Success(credentials, candidate.Subject);
    }

    private async Task<RefreshCredentialResult> RotateNonRelationalAsync(
        string refreshTokenHash,
        string? ipAddress,
        string? userAgent,
        CancellationToken cancellationToken)
    {
        var now = DateTimeOffset.UtcNow;
        var token = await db.RefreshTokens
            .Include(x => x.Account).ThenInclude(x => x!.PlayerIdentity)
            .FirstOrDefaultAsync(
                x => x.TokenHash == refreshTokenHash && x.RevokedAt == null && x.ExpiresAt > now,
                cancellationToken);
        if (token?.Account?.PlayerIdentity == null)
        {
            if (await DetectAndRevokeRefreshTokenReuseAsync(refreshTokenHash, now, cancellationToken))
                return Reused();

            return Invalid();
        }
        if (IsDisabled(token.Account.Status))
            return Disabled();

        token.RevokedAt = now;
        var subject = ToSubject(token.Account, token.Account.PlayerIdentity);
        var credentials = issuer.Issue(subject);
        AddRefreshToken(subject.AccountId, credentials, ipAddress, userAgent, now);
        token.Account.LastLoginAt = now;
        await db.SaveChangesAsync(cancellationToken);
        return Success(credentials, subject);
    }

    private Task<RefreshRotationCandidate?> FindCandidateAsync(
        string refreshTokenHash,
        DateTimeOffset now,
        CancellationToken cancellationToken)
    {
        return db.RefreshTokens
            .AsNoTracking()
            .Where(x =>
                x.TokenHash == refreshTokenHash &&
                x.RevokedAt == null &&
                x.ExpiresAt > now &&
                x.Account != null &&
                x.Account.PlayerIdentity != null)
            .Select(x => new RefreshRotationCandidate(
                x.Id,
                x.Account!.Status,
                new LoginCredentialSubject(
                    x.AccountId,
                    x.Account.PlayerIdentity!.PlayerId,
                    x.Account.PlayerIdentity.DisplayName,
                    x.Account.AccountType)))
            .FirstOrDefaultAsync(cancellationToken);
    }

    private void AddRefreshToken(
        Guid accountId,
        IssuedLoginCredentials credentials,
        string? ipAddress,
        string? userAgent,
        DateTimeOffset now)
    {
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = accountId,
            TokenHash = credentials.RefreshTokenHash,
            ExpiresAt = credentials.RefreshTokenExpiresAt,
            CreatedAt = now,
            CreatedByIp = string.IsNullOrWhiteSpace(ipAddress) ? null : ipAddress.Trim(),
            UserAgent = string.IsNullOrWhiteSpace(userAgent) ? null : userAgent.Trim()
        });
    }

    private static LoginCredentialSubject ToSubject(Account account, PlayerIdentity identity) =>
        new(account.Id, identity.PlayerId, identity.DisplayName, account.AccountType);

    private async Task<bool> DetectAndRevokeRefreshTokenReuseAsync(
        string refreshTokenHash,
        DateTimeOffset now,
        CancellationToken cancellationToken)
    {
        var accountId = await db.RefreshTokens
            .AsNoTracking()
            .Where(x =>
                x.TokenHash == refreshTokenHash &&
                x.RevokedAt != null &&
                x.ExpiresAt > now)
            .Select(x => (Guid?)x.AccountId)
            .FirstOrDefaultAsync(cancellationToken);
        if (!accountId.HasValue)
            return false;

        await RevokeActiveCredentialsAsync(accountId.Value, now, cancellationToken);
        return true;
    }

    private async Task RevokeActiveCredentialsAsync(
        Guid accountId,
        DateTimeOffset now,
        CancellationToken cancellationToken)
    {
        var activeTokens = db.RefreshTokens
            .Where(x => x.AccountId == accountId && x.RevokedAt == null);
        if (db.Database.IsRelational())
        {
            await activeTokens.ExecuteUpdateAsync(
                setters => setters.SetProperty(x => x.RevokedAt, now),
                cancellationToken);
            return;
        }

        var trackedTokens = await activeTokens.ToListAsync(cancellationToken);
        foreach (var token in trackedTokens)
            token.RevokedAt = now;
        await db.SaveChangesAsync(cancellationToken);
    }

    private static bool IsDisabled(RefreshRotationCandidate candidate) =>
        IsDisabled(candidate.AccountStatus);

    private static bool IsDisabled(string accountStatus) =>
        !string.Equals(accountStatus, "ACTIVE", StringComparison.OrdinalIgnoreCase);

    private static RefreshCredentialResult Success(
        IssuedLoginCredentials credentials,
        LoginCredentialSubject subject) =>
        new(
            RefreshCredentialStatus.Success,
            new LoginCredentialResponse(
                credentials.AccessToken,
                credentials.RefreshToken,
                credentials.AccessTokenExpiresAt),
            subject);

    private static RefreshCredentialResult Invalid() =>
        new(RefreshCredentialStatus.InvalidOrExpired);

    private static RefreshCredentialResult Reused() =>
        new(RefreshCredentialStatus.Reused);

    private static RefreshCredentialResult Disabled() =>
        new(RefreshCredentialStatus.AccountDisabled);

    private sealed record RefreshRotationCandidate(
        Guid TokenId,
        string AccountStatus,
        LoginCredentialSubject Subject);
}
