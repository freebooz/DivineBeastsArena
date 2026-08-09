/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using System.Text.Json;
using System.Security.Cryptography;
using Game.Shared.Contracts.Player;
using Game.Shared.Errors;
using Game.Shared.Options;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Infrastructure.Redis;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Player;

public sealed class PlayerService : IPlayerService
{
    private readonly GameDbContext _db;
    // Redis 只用于资料缓存失效，不是游戏名生成的权威依赖；存储层不可用时不能阻断首次登录。
    private readonly IRedisConnectionFactory? _redis;
    private readonly ILogger<PlayerService> _logger;
    private readonly PlayerGameNameOptions _gameNameOptions;

    public PlayerService(
        GameDbContext db,
        IRedisConnectionFactory? redis,
        ILogger<PlayerService> logger,
        PlayerGameNameOptions gameNameOptions)
    {
        _db = db;
        _redis = redis;
        _logger = logger;
        _gameNameOptions = gameNameOptions;
    }

    public async Task<PlayerGameNameEnsureResult> EnsureGeneratedGameNameAsync(
        Guid playerId,
        CancellationToken cancellationToken = default)
    {
        if (playerId == Guid.Empty)
        {
            return new(false, ErrorMessage: "玩家标识无效，无法生成游戏玩家名。");
        }

        // 使用数据库唯一索引作为并发最终裁决。两个登录请求同时到达时，只有一个请求能把
        // GameNameInitialized 从 false 改为 true；另一请求随后读取同一个最终名字即可。
        for (var attempt = 0; attempt < _gameNameOptions.GenerationAttempts; attempt++)
        {
            var profile = await _db.PlayerProfiles
                .FirstOrDefaultAsync(x => x.PlayerId == playerId, cancellationToken);
            if (profile is null)
            {
                return new(false, ErrorMessage: "玩家档案不存在，无法生成游戏玩家名。");
            }

            if (profile.GameNameInitialized)
            {
                return new(true, profile.Nickname, false);
            }

            var candidate = GenerateCandidate();
            if (string.IsNullOrEmpty(candidate))
            {
                return new(false, ErrorMessage: "游戏玩家名生成配置无效。");
            }

            // 先进行一次廉价检查以降低唯一约束冲突概率；最终仍必须捕获数据库竞争。
            if (await _db.PlayerProfiles.AnyAsync(x => x.Nickname == candidate, cancellationToken))
            {
                continue;
            }

            profile.Nickname = candidate;
            profile.GameNameInitialized = true;
            profile.UpdatedAt = DateTimeOffset.UtcNow;
            try
            {
                await _db.SaveChangesAsync(cancellationToken);
                await InvalidateCacheAsync(playerId);
                return new(true, candidate, true);
            }
            catch (DbUpdateConcurrencyException)
            {
                // 并发登录中其他请求已经完成初始化：丢弃旧跟踪实体后重新读取，
                // 下一轮会返回已提交的同一个名称，而不会发生“后到请求覆盖前名”。
                _db.ChangeTracker.Clear();
            }
            catch (DbUpdateException exception) when (IsNicknameUniqueViolation(exception))
            {
                // 候选名撞上其他玩家时清除本次跟踪状态并重试新候选，
                // 唯一索引仍是跨服务实例的最终裁决。
                _db.ChangeTracker.Clear();
            }
        }

        return new(false, ErrorMessage: "暂时无法生成唯一的游戏玩家名，请稍后重试。");
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
                // 玩家主动修改昵称后不再属于首次登录自动命名状态，任何后续认证均不得覆盖。
                profile.GameNameInitialized = true;
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
        // 单元测试与降级部署可以没有 Redis 连接。数据库已是权威源，因此仅跳过缓存失效，
        // 不应把缓存基础设施故障放大为认证或昵称生成失败。
        if (_redis is null)
        {
            return;
        }

        try
        {
            var db = _redis.GetDatabase();
            await db.KeyDeleteAsync($"player:profile:{playerId}");
            await db.KeyDeleteAsync($"player:settings:{playerId}");
            await db.KeyDeleteAsync($"player:stats:{playerId}");
        }
        catch (Exception ex)
        {
            _logger.LogWarning(ex, "玩家资料缓存失效失败，数据库权威数据仍可用，玩家标识：{PlayerId}", playerId);
        }
    }

    /** 从配置的单字姓氏与名字字库生成总长度为 3-5 的汉字游戏名。 */
    private string? GenerateCandidate()
    {
        if (_gameNameOptions.Surnames.Length == 0
            || _gameNameOptions.GivenNameCharacters.Length == 0)
        {
            return null;
        }

        var totalLength = RandomNumberGenerator.GetInt32(
            _gameNameOptions.MinimumHanCharacters,
            _gameNameOptions.MaximumHanCharacters + 1);
        var surname = _gameNameOptions.Surnames[RandomNumberGenerator.GetInt32(_gameNameOptions.Surnames.Length)];
        if (!IsSingleHanCharacter(surname))
        {
            return null;
        }

        var builder = new System.Text.StringBuilder(surname);
        while (builder.Length < totalLength)
        {
            var givenCharacter = _gameNameOptions.GivenNameCharacters[
                RandomNumberGenerator.GetInt32(_gameNameOptions.GivenNameCharacters.Length)];
            if (!IsSingleHanCharacter(givenCharacter))
            {
                return null;
            }
            builder.Append(givenCharacter);
        }
        return builder.ToString();
    }

    private static bool IsSingleHanCharacter(string? value) =>
        value is { Length: 1 } && value[0] is >= '\u4E00' and <= '\u9FFF';

    private static bool IsNicknameUniqueViolation(DbUpdateException exception) =>
        exception.InnerException is Npgsql.PostgresException { SqlState: Npgsql.PostgresErrorCodes.UniqueViolation };
}
