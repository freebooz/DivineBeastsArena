using Game.Application.Auth;
using Game.Infrastructure.Auth;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Auth;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;

namespace Game.IntegrationTests;

public class AuthRefreshTokenPersistenceTests
{
    [Fact]
    public async Task RefreshTokenReuse_RevokesAllActiveCredentialsForTheAccount()
    {
        var options = new DbContextOptionsBuilder<GameDbContext>()
            .UseInMemoryDatabase($"auth-refresh-persistence-{Guid.NewGuid()}")
            .Options;
        await using var db = new GameDbContext(options);
        var jwtOptions = new JwtOptions
        {
            Secret = "TEST-SECRET-KEY-MIN-32-CHARS-REFRESH",
            Issuer = "GameApi.Tests",
            Audience = "GameClients.Tests",
            AccessTokenExpiryMinutes = 15,
            RefreshTokenExpiryDays = 30
        };
        var jwt = new JwtTokenService(jwtOptions);
        var account = new Account
        {
            Id = Guid.NewGuid(),
            AccountType = "REGULAR",
            Status = "ACTIVE",
            CreatedAt = DateTimeOffset.UtcNow
        };
        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            PlayerId = Guid.NewGuid(),
            DisplayName = "RefreshPersistenceTester",
            Account = account,
            CreatedAt = DateTimeOffset.UtcNow
        };
        var originalRefreshToken = jwt.GenerateTokens(account, identity).RefreshToken;
        db.Accounts.Add(account);
        db.PlayerIdentities.Add(identity);
        db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            TokenHash = jwt.HashToken(originalRefreshToken),
            CreatedAt = DateTimeOffset.UtcNow,
            ExpiresAt = DateTimeOffset.UtcNow.AddDays(30)
        });
        await db.SaveChangesAsync();

        var store = new EfRefreshCredentialStore(
            db,
            new JwtLoginCredentialIssuer(jwt, jwtOptions));
        var useCase = new RotateRefreshCredentialUseCase(
            new JwtRefreshCredentialHasher(jwt),
            store);

        var rotated = await useCase.ExecuteAsync(originalRefreshToken, "127.0.0.1", "集成测试");
        var replayed = await useCase.ExecuteAsync(originalRefreshToken, "127.0.0.1", "集成测试");

        Assert.Equal(RefreshCredentialStatus.Success, rotated.Status);
        Assert.Equal(RefreshCredentialStatus.Reused, replayed.Status);
        Assert.All(await db.RefreshTokens.ToListAsync(), token => Assert.NotNull(token.RevokedAt));
        Assert.DoesNotContain(
            originalRefreshToken,
            await db.RefreshTokens.Select(token => token.TokenHash).ToListAsync());
    }
}
