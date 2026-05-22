/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Text.Json;
using Game.Shared.Contracts.Player;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Redis;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Player;

public sealed class PlayerService : IPlayerService
{
    private readonly GameDbContext _db;
    private readonly IRedisConnectionFactory _redis;
    private readonly ILogger<PlayerService> _logger;

    public PlayerService(GameDbContext db, IRedisConnectionFactory redis, ILogger<PlayerService> logger)
    {
        _db = db;
        _redis = redis;
        _logger = logger;
    }

    public async Task<PlayerProfileResponse?> GetProfileAsync(Guid playerId)
    {
        var profile = await _db.PlayerProfiles
            .Include(x => x.PlayerIdentity)
            .FirstOrDefaultAsync(x => x.PlayerId == playerId);

        if (profile == null) return null;

        return new PlayerProfileResponse(
            profile.PlayerId, profile.Nickname, profile.Avatar, profile.Level, profile.Exp,
            profile.NicknameUpdatedAt, profile.CreatedAt, profile.LastLoginAt);
    }

    public async Task<PlayerProfileResponse?> UpdateProfileAsync(Guid playerId, UpdateProfileRequest request)
    {
        var profile = await _db.PlayerProfiles.FirstOrDefaultAsync(x => x.PlayerId == playerId);
        if (profile == null) return null;

        if (!string.IsNullOrEmpty(request.Nickname))
        {
            var validation = NicknameValidator.Validate(request.Nickname);
            if (!validation.IsValid)
                throw new ArgumentException(validation.ErrorMessage);

            if (await IsNicknameAvailableAsync(request.Nickname, playerId))
            {
                var cooldown = DateTimeOffset.UtcNow.AddHours(-24);
                if (profile.NicknameUpdatedAt != null && profile.NicknameUpdatedAt > cooldown)
                    throw new InvalidOperationException(ErrorCodes.PlayerNicknameCooldown);

                profile.Nickname = request.Nickname;
                profile.NicknameUpdatedAt = DateTimeOffset.UtcNow;
            }
            else
            {
                throw new InvalidOperationException(ErrorCodes.PlayerNicknameTaken);
            }
        }

        if (request.Avatar != null)
            profile.Avatar = request.Avatar;

        profile.UpdatedAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        await InvalidateCacheAsync(playerId);

        return new PlayerProfileResponse(
            profile.PlayerId, profile.Nickname, profile.Avatar, profile.Level, profile.Exp,
            profile.NicknameUpdatedAt, profile.CreatedAt, profile.LastLoginAt);
    }

    public async Task<PlayerSettingsResponse?> GetSettingsAsync(Guid playerId)
    {
        var settings = await _db.PlayerSettings.FirstOrDefaultAsync(x => x.PlayerId == playerId);
        if (settings == null) return null;

        var dict = JsonSerializer.Deserialize<Dictionary<string, object>>(settings.SettingsJson) ?? new();
        return new PlayerSettingsResponse(settings.PlayerId, dict, settings.UpdatedAt);
    }

    public async Task<PlayerSettingsResponse?> UpdateSettingsAsync(Guid playerId, UpdateSettingsRequest request)
    {
        var settings = await _db.PlayerSettings.FirstOrDefaultAsync(x => x.PlayerId == playerId);
        if (settings == null)
        {
            settings = new PlayerSettings { PlayerId = playerId, SettingsJson = "{}", UpdatedAt = DateTimeOffset.UtcNow };
            _db.PlayerSettings.Add(settings);
        }

        var existing = JsonSerializer.Deserialize<Dictionary<string, object>>(settings.SettingsJson) ?? new();
        foreach (var kvp in request.Settings)
            existing[kvp.Key] = kvp.Value;

        settings.SettingsJson = JsonSerializer.Serialize(existing);
        settings.UpdatedAt = DateTimeOffset.UtcNow;
        await _db.SaveChangesAsync();

        return new PlayerSettingsResponse(settings.PlayerId, existing, settings.UpdatedAt);
    }

    public async Task<PlayerStatisticsResponse?> GetStatisticsAsync(Guid playerId)
    {
        var stats = await _db.PlayerStatistics.FirstOrDefaultAsync(x => x.PlayerId == playerId);
        if (stats == null) return null;

        return new PlayerStatisticsResponse(
            stats.PlayerId, stats.TotalMatches, stats.Wins, stats.Losses, stats.Draws,
            stats.Kills, stats.Deaths, stats.Assists, stats.Score, stats.PlayTimeSeconds, stats.UpdatedAt);
    }

    public async Task<PlayerPublicProfileResponse?> GetPublicProfileAsync(Guid playerId)
    {
        var profile = await _db.PlayerProfiles
            .Include(x => x.PlayerIdentity)
            .FirstOrDefaultAsync(x => x.PlayerId == playerId);

        if (profile == null) return null;

        return new PlayerPublicProfileResponse(profile.PlayerId, profile.Nickname, profile.Avatar, profile.Level);
    }

    public async Task<PlayerUnlocksResponse?> GetUnlocksAsync(Guid playerId)
    {
        var unlocks = await _db.PlayerUnlocks
            .Where(x => x.PlayerId == playerId)
            .OrderBy(x => x.CreatedAt)
            .ToListAsync();

        var dtos = unlocks.Select(x => new PlayerUnlockDto(x.UnlockType, x.UnlockId, x.Source, x.CreatedAt)).ToList();
        return new PlayerUnlocksResponse(playerId, dtos);
    }

    public async Task<bool> IsNicknameAvailableAsync(string nickname, Guid? excludePlayerId = null)
    {
        var query = _db.PlayerProfiles.Where(x => x.Nickname == nickname);
        if (excludePlayerId.HasValue)
            query = query.Where(x => x.PlayerId != excludePlayerId.Value);
        return !await query.AnyAsync();
    }

    public async Task<bool> CanUpdateNicknameAsync(Guid playerId)
    {
        var profile = await _db.PlayerProfiles.FirstOrDefaultAsync(x => x.PlayerId == playerId);
        if (profile?.NicknameUpdatedAt == null) return true;
        return profile.NicknameUpdatedAt <= DateTimeOffset.UtcNow.AddHours(-24);
    }

    private async Task InvalidateCacheAsync(Guid playerId)
    {
        try
        {
            var db = _redis.GetDatabase();
            await db.KeyDeleteAsync($"player:profile:{playerId}");
            await db.KeyDeleteAsync($"player:settings:{playerId}");
            await db.KeyDeleteAsync($"player:stats:{playerId}");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "Failed to invalidate cache for player {PlayerId}", playerId);
        }
    }
}