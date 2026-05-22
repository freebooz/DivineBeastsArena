/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Config;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace Game.Api.Services.Config;

public interface IConfigService
{
    Task<GameConfigManifestResponse> GetManifestAsync(string channel, string region);
    Task<GameConfigResponse?> GetConfigAsync(string configKey, string channel, string region);
    Task<GameConfigResponse> CreateConfigAsync(CreateConfigRequest request, Guid? adminId);
    Task<GameConfigResponse?> UpdateConfigAsync(Guid id, UpdateConfigRequest request, Guid? adminId);
    Task<bool> ValidateConfigAsync(string contentJson);
    Task<GameConfigResponse?> PublishConfigAsync(Guid id, PublishConfigRequest request, Guid adminId);
    Task<GameConfigResponse?> RollbackConfigAsync(Guid id, RollbackConfigRequest request, Guid adminId);
    Task<IReadOnlyList<GameConfigResponse>> GetAllConfigsAsync();
}

public sealed class ConfigService : IConfigService
{
    private readonly GameDbContext _db;
    private readonly ILogger<ConfigService> _logger;

    private static readonly HashSet<string> AllowedKeys = new(StringComparer.OrdinalIgnoreCase)
    {
        "zodiac_character", "element_skill", "god_skill", "map_config",
        "match_mode", "reward_table", "item_table", "bot_config"
    };

    public ConfigService(GameDbContext db, ILogger<ConfigService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<GameConfigManifestResponse> GetManifestAsync(string channel, string region)
    {
        var configs = await _db.GameConfigs
            .Where(x => x.Status == "PUBLISHED" && x.Channel == channel && x.Region == region)
            .Select(x => new ConfigManifestItem(x.ConfigKey, x.Version, x.Checksum, x.Channel, x.Region))
            .ToListAsync();

        return new GameConfigManifestResponse(configs);
    }

    public async Task<GameConfigResponse?> GetConfigAsync(string configKey, string channel, string region)
    {
        var config = await _db.GameConfigs
            .Where(x => x.ConfigKey == configKey && x.Channel == channel && x.Region == region && x.Status == "PUBLISHED")
            .FirstOrDefaultAsync();

        if (config == null) return null;

        return ToResponse(config);
    }

    public async Task<GameConfigResponse> CreateConfigAsync(CreateConfigRequest request, Guid? adminId)
    {
        if (!AllowedKeys.Contains(request.ConfigKey))
            throw new ArgumentException(ErrorCodes.ConfigKeyNotAllowed);

        if (!ValidateJson(request.ContentJson))
            throw new ArgumentException(ErrorCodes.ConfigInvalidJson);

        var checksum = ComputeChecksum(request.ContentJson);

        var config = new GameConfig
        {
            Id = Guid.NewGuid(),
            ConfigKey = request.ConfigKey,
            Version = request.Version,
            ContentJson = request.ContentJson,
            Status = "DRAFT",
            Checksum = checksum,
            Channel = request.Channel,
            Region = request.Region,
            MinClientVersion = request.MinClientVersion,
            MaxClientVersion = request.MaxClientVersion,
            CreatedBy = adminId,
            CreatedAt = DateTimeOffset.UtcNow
        };

        _db.GameConfigs.Add(config);
        await _db.SaveChangesAsync();

        return ToResponse(config);
    }

    public async Task<GameConfigResponse?> UpdateConfigAsync(Guid id, UpdateConfigRequest request, Guid? adminId)
    {
        var config = await _db.GameConfigs.FindAsync(id);
        if (config == null) return null;

        if (!ValidateJson(request.ContentJson))
            throw new ArgumentException(ErrorCodes.ConfigInvalidJson);

        config.ContentJson = request.ContentJson;
        config.Checksum = ComputeChecksum(request.ContentJson);
        config.UpdatedAt = DateTimeOffset.UtcNow;

        await _db.SaveChangesAsync();
        return ToResponse(config);
    }

    public Task<bool> ValidateConfigAsync(string contentJson)
    {
        return Task.FromResult(ValidateJson(contentJson));
    }

    public async Task<GameConfigResponse?> PublishConfigAsync(Guid id, PublishConfigRequest request, Guid adminId)
    {
        var config = await _db.GameConfigs.FindAsync(id);
        if (config == null) return null;

        var previousPublished = await _db.GameConfigs
            .Where(x => x.ConfigKey == config.ConfigKey && x.Channel == config.Channel &&
                        x.Region == config.Region && x.Status == "PUBLISHED")
            .FirstOrDefaultAsync();

        if (previousPublished != null)
        {
            previousPublished.Status = "ARCHIVED";
        }

        config.Status = "PUBLISHED";
        config.PublishedBy = adminId;
        config.PublishedAt = DateTimeOffset.UtcNow;

        _db.GameConfigPublishLogs.Add(new GameConfigPublishLog
        {
            Id = Guid.NewGuid(),
            ConfigKey = config.ConfigKey,
            FromVersion = previousPublished?.Version,
            ToVersion = config.Version,
            OperatorId = adminId,
            Reason = request.Reason,
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(config);
    }

    public async Task<GameConfigResponse?> RollbackConfigAsync(Guid id, RollbackConfigRequest request, Guid adminId)
    {
        var current = await _db.GameConfigs.FindAsync(id);
        if (current == null) return null;

        var previousPublished = await _db.GameConfigs
            .Where(x => x.ConfigKey == current.ConfigKey && x.Channel == current.Channel &&
                        x.Region == current.Region && x.Status == "PUBLISHED" && x.Id != id)
            .OrderByDescending(x => x.PublishedAt)
            .FirstOrDefaultAsync();

        if (previousPublished == null)
            throw new InvalidOperationException("No previous version to rollback to");

        current.Status = "ROLLED_BACK";
        previousPublished.Status = "PUBLISHED";
        previousPublished.PublishedAt = DateTimeOffset.UtcNow;

        _db.GameConfigPublishLogs.Add(new GameConfigPublishLog
        {
            Id = Guid.NewGuid(),
            ConfigKey = current.ConfigKey,
            FromVersion = current.Version,
            ToVersion = previousPublished.Version,
            OperatorId = adminId,
            Reason = request.Reason ?? "Rollback",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return ToResponse(previousPublished);
    }

    public async Task<IReadOnlyList<GameConfigResponse>> GetAllConfigsAsync()
    {
        var configs = await _db.GameConfigs
            .OrderBy(x => x.ConfigKey)
            .ThenByDescending(x => x.CreatedAt)
            .ToListAsync();

        return configs.Select(ToResponse).ToList();
    }

    private static GameConfigResponse ToResponse(GameConfig c) => new(
        c.Id, c.ConfigKey, c.Version, c.ContentJson, c.Status, c.Checksum,
        c.Channel, c.Region, c.MinClientVersion, c.MaxClientVersion, c.CreatedAt, c.PublishedAt);

    private static bool ValidateJson(string json)
    {
        try { JsonDocument.Parse(json); return true; }
        catch { return false; }
    }

    private static string ComputeChecksum(string content)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(content));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }
}