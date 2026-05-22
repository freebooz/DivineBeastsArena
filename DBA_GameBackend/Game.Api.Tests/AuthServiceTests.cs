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
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"auth-refresh-{Guid.NewGuid()}")
            .Options;

        await using var db = new GameDbContext(options);
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

    private static AuthService CreateAuthService(GameDbContext db, IJwtTokenService jwt, JwtOptions jwtOptions)
    {
        var configuration = new ConfigurationBuilder()
            .AddInMemoryCollection(new Dictionary<string, string?>
            {
                ["ASPNETCORE_ENVIRONMENT"] = "Development"
            })
            .Build();

        return new AuthService(
            db,
            jwt,
            new NullRedisConnectionFactory(),
            jwtOptions,
            NullLogger<AuthService>.Instance,
            configuration);
    }

    private sealed class NullRedisConnectionFactory : IRedisConnectionFactory
    {
        public IDatabase GetDatabase(int db = -1) => throw new NotSupportedException();

        public IServer GetServer() => throw new NotSupportedException();
    }
}
