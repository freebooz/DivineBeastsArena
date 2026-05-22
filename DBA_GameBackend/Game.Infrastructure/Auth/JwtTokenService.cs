/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Security.Cryptography;
using System.Text;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using Microsoft.IdentityModel.Tokens;
using Game.Shared.Options;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Database;

namespace Game.Infrastructure.Auth;

public interface IJwtTokenService
{
    (string AccessToken, string RefreshToken, DateTimeOffset ExpiresAt) GenerateTokens(Account account, PlayerIdentity identity);
    ClaimsPrincipal? ValidateAccessToken(string token);
    string HashToken(string token);
    (bool IsValid, Guid AccountId) ValidateRefreshToken(string hashedToken);
}

public sealed class JwtTokenService : IJwtTokenService
{
    private readonly JwtOptions _options;
    private readonly TokenValidationParameters _validationParams;

    public JwtTokenService(JwtOptions options)
    {
        _options = options;
        _validationParams = new TokenValidationParameters
        {
            ValidateIssuer = true,
            ValidateAudience = true,
            ValidateLifetime = true,
            ValidateIssuerSigningKey = true,
            ValidIssuer = options.Issuer,
            ValidAudience = options.Audience,
            IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(options.Secret))
        };
    }

    public (string AccessToken, string RefreshToken, DateTimeOffset ExpiresAt) GenerateTokens(Account account, PlayerIdentity identity)
    {
        var expiresAt = DateTimeOffset.UtcNow.AddMinutes(_options.AccessTokenExpiryMinutes);
        var accessToken = GenerateAccessToken(account, identity, expiresAt);
        var refreshToken = Convert.ToBase64String(RandomNumberGenerator.GetBytes(64));
        return (accessToken, refreshToken, expiresAt);
    }

    private string GenerateAccessToken(Account account, PlayerIdentity identity, DateTimeOffset expiresAt)
    {
        var claims = new[]
        {
            new Claim(JwtRegisteredClaimNames.Sub, account.Id.ToString()),
            new Claim(ClaimTypes.NameIdentifier, account.Id.ToString()),
            new Claim(ClaimTypes.Name, identity.DisplayName),
            new Claim(JwtRegisteredClaimNames.Jti, Guid.NewGuid().ToString()),
            new Claim("player_id", identity.PlayerId.ToString()),
            new Claim("account_type", account.AccountType),
            new Claim("nickname", identity.DisplayName)
        };

        var key = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_options.Secret));
        var creds = new SigningCredentials(key, SecurityAlgorithms.HmacSha256);
        var token = new JwtSecurityToken(
            issuer: _options.Issuer,
            audience: _options.Audience,
            claims: claims,
            expires: expiresAt.UtcDateTime,
            signingCredentials: creds);

        return new JwtSecurityTokenHandler().WriteToken(token);
    }

    public ClaimsPrincipal? ValidateAccessToken(string token)
    {
        try
        {
            var handler = new JwtSecurityTokenHandler();
            return handler.ValidateToken(token, _validationParams, out _);
        }
        catch
        {
            return null;
        }
    }

    public string HashToken(string token)
    {
        var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(bytes).ToLowerInvariant();
    }

    public (bool IsValid, Guid AccountId) ValidateRefreshToken(string hashedToken)
    {
        // This method is called by AuthService.RefreshTokenAsync
        // The actual validation is done via database query in AuthService
        // This stub returns false as refresh token validation should go through DB
        return (false, Guid.Empty);
    }
}
