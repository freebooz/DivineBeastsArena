/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Security.Cryptography;
using System.Text;
using Game.Shared.Contracts.Auth;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Auth;
using Game.Infrastructure.Redis;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using StackExchange.Redis;

namespace Game.Api.Services.Auth;

public sealed class AuthService : IAuthService
{
    private readonly GameDbContext _db;
    private readonly IJwtTokenService _jwt;
    private readonly IRedisConnectionFactory _redis;
    private readonly JwtOptions _jwtOptions;
    private readonly ILogger<AuthService> _logger;
    private readonly bool _isProduction;

    public AuthService(
        GameDbContext db,
        IJwtTokenService jwt,
        IRedisConnectionFactory redis,
        JwtOptions jwtOptions,
        ILogger<AuthService> logger,
        IConfiguration configuration)
    {
        _db = db;
        _jwt = jwt;
        _redis = redis;
        _jwtOptions = jwtOptions;
        _logger = logger;
        _isProduction = configuration["ASPNETCORE_ENVIRONMENT"] == "Production";
    }

    public async Task<AuthServiceResult> GuestLoginAsync(GuestLoginRequest request, string? ip, string? userAgent)
    {
        var deviceId = request.DeviceId ?? Guid.NewGuid().ToString();
        var deviceHash = Hash(deviceId);

        var existingLogin = await _db.DeviceLogins
            .Include(x => x.Account).ThenInclude(x => x!.PlayerIdentity).ThenInclude(x => x!.PlayerProfile)
            .FirstOrDefaultAsync(x => x.DeviceIdHash == deviceHash);

        if (existingLogin?.Account != null)
        {
            var acc = existingLogin.Account;
            if (acc.Status == "BANNED")
                return Fail(ErrorCodes.AuthAccountBanned, "Account is banned");
            existingLogin.LastLoginAt = DateTimeOffset.UtcNow;
            if (!string.IsNullOrWhiteSpace(request.DeviceName))
                existingLogin.DeviceName = request.DeviceName.Trim();
            if (!string.IsNullOrWhiteSpace(request.Platform))
                existingLogin.Platform = request.Platform.Trim();
            return await GenerateLoginResult(acc, acc.PlayerIdentity!);
        }

        // Create new account for new device
        var account = new Account
        {
            Id = Guid.NewGuid(),
            AccountType = "GUEST",
            Status = "ACTIVE",
            CreatedAt = DateTimeOffset.UtcNow
        };

        var playerId = Guid.NewGuid();
        var nickname = $"Player_{Guid.NewGuid().ToString()[..8]}";

        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            PlayerId = playerId,
            DisplayName = nickname,
            CreatedAt = DateTimeOffset.UtcNow
        };

        var profile = new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = nickname,
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        var stats = new PlayerStatistics
        {
            PlayerId = playerId,
            TotalMatches = 0, Wins = 0, Losses = 0, Draws = 0,
            Kills = 0, Deaths = 0, Assists = 0, Score = 0, PlayTimeSeconds = 0,
            UpdatedAt = DateTimeOffset.UtcNow
        };

        var settings = new PlayerSettings
        {
            PlayerId = playerId,
            SettingsJson = "{}",
            UpdatedAt = DateTimeOffset.UtcNow
        };

        var deviceLogin = new DeviceLogin
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            DeviceIdHash = deviceHash,
            DeviceName = string.IsNullOrWhiteSpace(request.DeviceName) ? null : request.DeviceName.Trim(),
            Platform = string.IsNullOrWhiteSpace(request.Platform) ? null : request.Platform.Trim(),
            LastLoginAt = DateTimeOffset.UtcNow,
            CreatedAt = DateTimeOffset.UtcNow
        };

        account.LastLoginAt = DateTimeOffset.UtcNow;

        _db.Accounts.Add(account);
        _db.PlayerIdentities.Add(identity);
        _db.PlayerProfiles.Add(profile);
        _db.PlayerStatistics.Add(stats);
        _db.PlayerSettings.Add(settings);
        _db.DeviceLogins.Add(deviceLogin);

        await _db.SaveChangesAsync();
        return await GenerateLoginResult(account, identity);
    }

    public async Task<AuthServiceResult> DevLoginAsync(DevLoginRequest request, string? ip, string? userAgent)
    {
        if (_isProduction)
            return Fail(ErrorCodes.AuthDevLoginDisabled, "Dev login disabled in production");

        var username = FirstNonEmpty(request.Username, request.DisplayName);
        if (string.IsNullOrWhiteSpace(username))
            return Fail(ErrorCodes.AuthInvalidCredentials, "Username is required");

        var identity = await _db.PlayerIdentities
            .Include(x => x.Account).ThenInclude(x => x!.PlayerIdentity)
            .FirstOrDefaultAsync(x => x.DisplayName == username);

        if (identity?.Account == null)
            return Fail(ErrorCodes.AuthInvalidCredentials, "Invalid credentials");

        var acc = identity.Account;
        if (acc.Status == "BANNED")
            return Fail(ErrorCodes.AuthAccountBanned, "Account is banned");
        if (!string.IsNullOrEmpty(request.Password) && !string.IsNullOrEmpty(acc.PasswordHash)
            && !PasswordHasher.Verify(request.Password, acc.PasswordHash))
            return Fail(ErrorCodes.AuthInvalidCredentials, "Invalid credentials");

        return await GenerateLoginResult(acc, identity);
    }

    public async Task<AuthServiceResult> AccountLoginAsync(AccountLoginRequest request, string? ip, string? userAgent)
    {
        var loginName = FirstNonEmpty(request.Username, request.Email);
        if (string.IsNullOrWhiteSpace(loginName) || string.IsNullOrWhiteSpace(request.Password))
            return Fail(ErrorCodes.AuthInvalidCredentials, "Username/email and password are required");

        var identity = await _db.PlayerIdentities
            .Include(x => x.Account)
            .FirstOrDefaultAsync(x => x.DisplayName == loginName || x.Account!.Email == loginName);

        if (identity?.Account == null)
            return Fail(ErrorCodes.AuthInvalidCredentials, "Invalid credentials");

        var acc = identity.Account;
        if (string.IsNullOrEmpty(acc.PasswordHash) || !PasswordHasher.Verify(request.Password!, acc.PasswordHash))
            return Fail(ErrorCodes.AuthInvalidCredentials, "Invalid credentials");

        if (acc.Status == "BANNED")
            return Fail(ErrorCodes.AuthAccountBanned, "Account is banned");

        return await GenerateLoginResult(acc, identity);
    }

    public async Task<AuthServiceResult> AccountRegisterAsync(AccountRegisterRequest request, string? ip, string? userAgent)
    {
        var username = FirstNonEmpty(request.Username, BuildUsernameFromEmail(request.Email));
        if (string.IsNullOrWhiteSpace(username))
            return Fail(ErrorCodes.ValidationError, "Username or email is required");
        if (string.IsNullOrWhiteSpace(request.Password) || request.Password.Length < 6)
            return Fail(ErrorCodes.ValidationError, "Password must be at least 6 characters");

        // Check if username already exists
        var existing = await _db.PlayerIdentities
            .FirstOrDefaultAsync(x => x.DisplayName == username || (!string.IsNullOrEmpty(request.Email) && x.Account!.Email == request.Email));
        if (existing != null)
            return Fail(ErrorCodes.PlayerNicknameTaken, "Username already taken");

        var account = new Account
        {
            Id = Guid.NewGuid(),
            AccountType = "REGULAR",
            Status = "ACTIVE",
            Email = request.Email,
            PasswordHash = PasswordHasher.Hash(request.Password),
            CreatedAt = DateTimeOffset.UtcNow
        };

        var playerId = Guid.NewGuid();
        var nickname = username;

        var identity = new PlayerIdentity
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            PlayerId = playerId,
            DisplayName = nickname,
            CreatedAt = DateTimeOffset.UtcNow
        };

        var profile = new PlayerProfile
        {
            PlayerId = playerId,
            Nickname = nickname,
            Level = 1,
            Exp = 0,
            CreatedAt = DateTimeOffset.UtcNow
        };

        var stats = new PlayerStatistics
        {
            PlayerId = playerId,
            TotalMatches = 0, Wins = 0, Losses = 0, Draws = 0,
            Kills = 0, Deaths = 0, Assists = 0, Score = 0, PlayTimeSeconds = 0,
            UpdatedAt = DateTimeOffset.UtcNow
        };

        var settings = new PlayerSettings
        {
            PlayerId = playerId,
            SettingsJson = "{}",
            UpdatedAt = DateTimeOffset.UtcNow
        };

        account.LastLoginAt = DateTimeOffset.UtcNow;

        _db.Accounts.Add(account);
        _db.PlayerIdentities.Add(identity);
        _db.PlayerProfiles.Add(profile);
        _db.PlayerStatistics.Add(stats);
        _db.PlayerSettings.Add(settings);

        await _db.SaveChangesAsync();
        return await GenerateLoginResult(account, identity);
    }

    public async Task<AuthServiceResult> RefreshTokenAsync(string? refreshToken, string? ip)
    {
        if (string.IsNullOrWhiteSpace(refreshToken))
            return Fail(ErrorCodes.AuthRefreshTokenExpired, "Refresh token is required");

        var hash = _jwt.HashToken(refreshToken);
        var token = await _db.RefreshTokens
            .Include(x => x.Account).ThenInclude(x => x!.PlayerIdentity).ThenInclude(x => x!.PlayerProfile)
            .FirstOrDefaultAsync(x => x.TokenHash == hash && x.RevokedAt == null && x.ExpiresAt > DateTimeOffset.UtcNow);

        if (token?.Account == null)
            return Fail(ErrorCodes.AuthRefreshTokenExpired, "Invalid or expired refresh token");

        if (token.Account.Status == "BANNED")
            return Fail(ErrorCodes.AuthAccountBanned, "Account is banned");

        token.RevokedAt = DateTimeOffset.UtcNow;
        return await GenerateLoginResult(token.Account, token.Account.PlayerIdentity!);
    }

    public async Task LogoutAsync(Guid accountId, string? refreshToken)
    {
        if (string.IsNullOrWhiteSpace(refreshToken))
        {
            await _db.RefreshTokens
                .Where(x => x.AccountId == accountId && x.RevokedAt == null)
                .ExecuteUpdateAsync(s => s.SetProperty(x => x.RevokedAt, DateTimeOffset.UtcNow));
            return;
        }

        var hash = _jwt.HashToken(refreshToken);
        var token = await _db.RefreshTokens.FirstOrDefaultAsync(x => x.AccountId == accountId && x.TokenHash == hash && x.RevokedAt == null);
        if (token != null)
        {
            token.RevokedAt = DateTimeOffset.UtcNow;
            await _db.SaveChangesAsync();
        }
    }

    public async Task<MeResponse?> GetMeAsync(Guid accountId)
    {
        var account = await _db.Accounts
            .Include(x => x.PlayerIdentity)
            .FirstOrDefaultAsync(x => x.Id == accountId);

        if (account?.PlayerIdentity == null)
            return null;

        return new MeResponse(account.Id, account.PlayerIdentity.PlayerId, account.PlayerIdentity.DisplayName,
            account.AccountType, account.Email);
    }

    public Task<AuthServiceResult> SteamExternalLoginAsync(string steamTicket, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with Steam API
        return Task.FromResult(Fail(ErrorCodes.AuthSteamMockOnly, "Steam login is in mock mode"));
    }

    public Task<AuthServiceResult> EosExternalLoginAsync(string eosTicket, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with EOS API
        return Task.FromResult(Fail(ErrorCodes.AuthEosMockOnly, "EOS login is in mock mode"));
    }

    public Task<AuthServiceResult> WeChatExternalLoginAsync(string code, string? nickname, string? ip, string? userAgent)
    {
        // Mock implementation - in production would validate with WeChat API
        // TODO: Replace with actual WeChat API validation:
        // 1. Call WeChat API with code to get openid
        // 2. Check if account exists for this openid
        // 3. If not, create new account
        // 4. Return login result with access/refresh tokens
        return Task.FromResult(Fail(ErrorCodes.AuthWeChatMockOnly, "WeChat login is in mock mode"));
    }

    private async Task<AuthServiceResult> GenerateLoginResult(Account account, PlayerIdentity identity)
    {
        var (accessToken, refreshToken, expiresAt) = _jwt.GenerateTokens(account, identity);

        var refreshHash = _jwt.HashToken(refreshToken);
        _db.RefreshTokens.Add(new RefreshToken
        {
            Id = Guid.NewGuid(),
            AccountId = account.Id,
            TokenHash = refreshHash,
            ExpiresAt = DateTimeOffset.UtcNow.AddDays(_jwtOptions.RefreshTokenExpiryDays),
            CreatedAt = DateTimeOffset.UtcNow,
            UserAgent = null
        });

        account.LastLoginAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        return new AuthServiceResult(true, accessToken, refreshToken, account.Id, identity.PlayerId, identity.DisplayName);
    }

    public Task<PasswordChangeResponse> ChangePasswordAsync(Guid accountId, string oldPassword, string newPassword)
    {
        // TODO: Implement BCrypt password verification and update
        return Task.FromResult(new PasswordChangeResponse(false, "Password change not implemented"));
    }

    public Task<PasswordChangeResponse> ResetPasswordAsync(string email, string? token, string? newPassword)
    {
        // TODO: Implement password reset flow with email token verification
        return Task.FromResult(new PasswordChangeResponse(false, "Password reset not implemented"));
    }

    private static AuthServiceResult Fail(string errorCode, string message) =>
        new(false, null, null, null, null, null, errorCode, message);

    private static string? FirstNonEmpty(params string?[] values)
    {
        foreach (var value in values)
        {
            if (!string.IsNullOrWhiteSpace(value))
                return value.Trim();
        }

        return null;
    }

    private static string? BuildUsernameFromEmail(string? email)
    {
        if (string.IsNullOrWhiteSpace(email))
            return null;

        var trimmed = email.Trim();
        var at = trimmed.IndexOf('@');
        return at > 0 ? trimmed[..at] : trimmed;
    }

    private static string Hash(string input)
    {
        var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(input));
        return Convert.ToHexString(bytes).ToLowerInvariant();
    }
}
