/*
中文阅读说明：
- 所属应用：DBA_GameBackend 基础设施层。
- 文件职责：使用现有 JWT 服务签发登录凭据，并集中计算刷新令牌有效期。
*/

using Game.Application.Auth;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Options;

namespace Game.Infrastructure.Auth;

public sealed class JwtLoginCredentialIssuer(
    IJwtTokenService jwtTokenService,
    JwtOptions options) : ILoginCredentialIssuer
{
    public IssuedLoginCredentials Issue(LoginCredentialSubject subject)
    {
        var account = new Account
        {
            Id = subject.AccountId,
            AccountType = subject.AccountType
        };
        var identity = new PlayerIdentity
        {
            AccountId = subject.AccountId,
            PlayerId = subject.PlayerId,
            DisplayName = subject.DisplayName
        };
        var tokens = jwtTokenService.GenerateTokens(account, identity);

        return new IssuedLoginCredentials(
            tokens.AccessToken,
            tokens.RefreshToken,
            jwtTokenService.HashToken(tokens.RefreshToken),
            tokens.ExpiresAt,
            DateTimeOffset.UtcNow.AddDays(options.RefreshTokenExpiryDays));
    }
}
