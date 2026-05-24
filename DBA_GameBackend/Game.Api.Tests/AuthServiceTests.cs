/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：测试认证服务中的生产关键规则，尤其是刷新令牌轮换和旧令牌失效。
- 阅读重点：先看测试名称理解业务契约，再看 Arrange/Act/Assert 中的数据准备和断言。
- 修改提示：新增认证规则时优先补充此类测试，避免登录安全行为在重构中退化。
*/

using Game.Api.Services.Auth;
using Game.Infrastructure.Auth;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Redis;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging.Abstractions;
using StackExchange.Redis;

namespace Game.Api.Tests;

public class AuthServiceTests
{
    [Fact]
    public async Task RefreshToken_RotatesTokenAndRejectsReusedOldToken()
    {
        await using var db = CreateDbContext("auth-refresh");
        var jwtOptions = new JwtOptions
        {
            Secret = "TEST-SECRET-KEY-MIN-32-CHARS-REFRESH",
            Issuer = "GameApi",
            Audience = "GameClients",
            AccessTokenExpiryMinutes = 30,
            RefreshTokenExpiryDays = 30
        };
        var jwt = new JwtTokenService(jwtOptions);

        var account = new Account
        {
            Id = Guid.NewGuid(),
            AccountType = "PLAYER",
            Status = "ACTIVE",
            CreatedAt = DateTimeOffset.UtcNow
        };
        var playerId = Guid.NewGuid();
        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            PlayerId = playerId,
            DisplayName = "RefreshTester",
            CreatedAt = DateTimeOffset.UtcNow
        };
        var profile = new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = "RefreshTester",
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        var initialRefreshToken = jwt.GenerateTokens(account, identity).RefreshToken;
        db.Accounts.Add(account);
        db.PlayerIdentities.Add(identity);
        db.PlayerProfiles.Add(profile);
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            TokenHash = jwt.HashToken(initialRefreshToken),
            ExpiresAt = DateTimeOffset.UtcNow.AddDays(30),
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var service = CreateAuthService(db, jwt, jwtOptions);

        var rotated = await service.RefreshTokenAsync(initialRefreshToken, "127.0.0.1");
        var reused = await service.RefreshTokenAsync(initialRefreshToken, "127.0.0.1");

        Assert.True(rotated.Success);
        Assert.NotEqual(initialRefreshToken, rotated.RefreshToken);
        Assert.False(reused.Success);
        Assert.Equal(2, await db.RefreshTokens.CountAsync());
        Assert.NotNull(await db.RefreshTokens
            .Where(x => x.TokenHash == jwt.HashToken(initialRefreshToken))
            .Select(x => x.RevokedAt)
            .SingleAsync());
        Assert.Contains(await db.RefreshTokens.ToListAsync(), x => x.TokenHash == jwt.HashToken(rotated.RefreshToken!));
    }

    [Fact]
    public async Task ChangePassword_UpdatesHashAndRevokesActiveRefreshTokens()
    {
        await using var db = CreateDbContext("auth-change-password");
        var jwtOptions = CreateJwtOptions();
        var jwt = new JwtTokenService(jwtOptions);
        var account = await SeedRegularAccountAsync(db, "change@example.test", "old-password");

        var activeToken = jwt.GenerateTokens(account, account.PlayerIdentity!).RefreshToken;
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            TokenHash = jwt.HashToken(activeToken),
            ExpiresAt = DateTimeOffset.UtcNow.AddDays(30),
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var service = CreateAuthService(db, jwt, jwtOptions);

        var result = await service.ChangePasswordAsync(account.Id, "old-password", "new-password");

        var updatedAccount = await db.Accounts.SingleAsync(x => x.Id == account.Id);
        Assert.True(result.Success);
        Assert.True(PasswordHasher.Verify("new-password", updatedAccount.PasswordHash!));
        Assert.False(PasswordHasher.Verify("old-password", updatedAccount.PasswordHash!));
        Assert.NotNull(await db.RefreshTokens
            .Where(x => x.AccountId == account.Id)
            .Select(x => x.RevokedAt)
            .SingleAsync());
    }

    [Fact]
    public async Task ChangePassword_RejectsWrongOldPassword()
    {
        await using var db = CreateDbContext("auth-change-password-reject");
        var jwtOptions = CreateJwtOptions();
        var jwt = new JwtTokenService(jwtOptions);
        var account = await SeedRegularAccountAsync(db, "reject@example.test", "old-password");
        var service = CreateAuthService(db, jwt, jwtOptions);

        var result = await service.ChangePasswordAsync(account.Id, "wrong-password", "new-password");

        var unchangedAccount = await db.Accounts.SingleAsync(x => x.Id == account.Id);
        Assert.False(result.Success);
        Assert.True(PasswordHasher.Verify("old-password", unchangedAccount.PasswordHash!));
    }

    [Fact]
    public async Task ResetPassword_RequiresConfiguredBootstrapToken()
    {
        await using var db = CreateDbContext("auth-reset-unconfigured");
        var jwtOptions = CreateJwtOptions();
        var jwt = new JwtTokenService(jwtOptions);
        await SeedRegularAccountAsync(db, "reset-unconfigured@example.test", "old-password");
        var service = CreateAuthService(db, jwt, jwtOptions);

        var result = await service.ResetPasswordAsync("reset-unconfigured@example.test", "reset-token", "new-password");

        Assert.False(result.Success);
        Assert.Contains("not configured", result.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public async Task ResetPassword_WithBootstrapTokenUpdatesHashAndRevokesTokens()
    {
        await using var db = CreateDbContext("auth-reset");
        var jwtOptions = CreateJwtOptions();
        var jwt = new JwtTokenService(jwtOptions);
        var account = await SeedRegularAccountAsync(db, "reset@example.test", "old-password");

        var activeToken = jwt.GenerateTokens(account, account.PlayerIdentity!).RefreshToken;
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            TokenHash = jwt.HashToken(activeToken),
            ExpiresAt = DateTimeOffset.UtcNow.AddDays(30),
            CreatedAt = DateTimeOffset.UtcNow
        });
        await db.SaveChangesAsync();

        var service = CreateAuthService(db, jwt, jwtOptions, new Dictionary<string, string?>
        {
            ["Auth:PasswordResetBootstrapToken"] = "server-reset-token"
        });

        var result = await service.ResetPasswordAsync("reset@example.test", "server-reset-token", "new-password");

        var updatedAccount = await db.Accounts.SingleAsync(x => x.Id == account.Id);
        Assert.True(result.Success);
        Assert.True(PasswordHasher.Verify("new-password", updatedAccount.PasswordHash!));
        Assert.NotNull(await db.RefreshTokens
            .Where(x => x.AccountId == account.Id)
            .Select(x => x.RevokedAt)
            .SingleAsync());
    }

    private static GameDbContext CreateDbContext(string namePrefix)
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"{namePrefix}-{Guid.NewGuid()}")
            .Options;

        return new GameDbContext(options);
    }

    private static JwtOptions CreateJwtOptions() => new()
    {
        Secret = "TEST-SECRET-KEY-MIN-32-CHARS-REFRESH",
        Issuer = "GameApi",
        Audience = "GameClients",
        AccessTokenExpiryMinutes = 30,
        RefreshTokenExpiryDays = 30
    };

    private static async Task<Account> SeedRegularAccountAsync(GameDbContext db, string email, string password)
    {
        var account = new Account
        {
            Id = Guid.NewGuid(),
            AccountType = "PLAYER",
            Status = "ACTIVE",
            Email = email,
            PasswordHash = PasswordHasher.Hash(password),
            CreatedAt = DateTimeOffset.UtcNow
        };
        var playerId = Guid.NewGuid();
        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            PlayerId = playerId,
            DisplayName = email.Split('@')[0],
            CreatedAt = DateTimeOffset.UtcNow,
            Account = account
        };
        account.PlayerIdentity = identity;
        var profile = new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = identity.DisplayName,
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        db.Accounts.Add(account);
        db.PlayerIdentities.Add(identity);
        db.PlayerProfiles.Add(profile);
        await db.SaveChangesAsync();

        return account;
    }

    private static AuthService CreateAuthService(
        GameDbContext db,
        IJwtTokenService jwt,
        JwtOptions jwtOptions,
        IReadOnlyDictionary<string, string?>? configurationOverrides = null)
    {
        var configuration = new Dictionary<string, string?>
        {
            ["ASPNETCORE_ENVIRONMENT"] = "Development"
        };
        if (configurationOverrides != null)
        {
            foreach (var pair in configurationOverrides)
                configuration[pair.Key] = pair.Value;
        }

        var builtConfiguration = new ConfigurationBuilder()
            .AddInMemoryCollection(configuration)
            .Build();

        return new AuthService(
            db,
            jwt,
            new NullRedisConnectionFactory(),
            jwtOptions,
            NullLogger<AuthService>.Instance,
            builtConfiguration);
    }

    private sealed class NullRedisConnectionFactory : IRedisConnectionFactory
    {
        public IDatabase GetDatabase(int db = -1) => throw new NotSupportedException();

        public IServer GetServer() => throw new NotSupportedException();
    }
}
