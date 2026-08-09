using System.Text.Json;
using Game.Infrastructure.Database;
using Game.Infrastructure.Redis;
using Game.Shared.Contracts.GameServer;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Options;

namespace Game.Api.Services.ServerDirectory;

public interface IServerDirectoryService
{
    Task<IReadOnlyList<ServerDirectoryServerDto>> GetServersAsync(ServerDirectoryQuery query, CancellationToken cancellationToken);
    Task InvalidateCacheAsync(CancellationToken cancellationToken);
}

/** 面向玩家的逻辑区服目录；绝不读取或暴露单局 game_server_instance。 */
public sealed class ServerDirectoryService(
    GameDbContext db,
    IServiceProvider serviceProvider,
    IOptions<ServerDirectoryOptions> options,
    ILogger<ServerDirectoryService> logger) : IServerDirectoryService
{
    private const string CachePrefix = "dba:server-directory:v1:";
    private readonly GameDbContext _db = db;
    private readonly IServiceProvider _serviceProvider = serviceProvider;
    private readonly ServerDirectoryOptions _options = options.Value;
    private readonly ILogger<ServerDirectoryService> _logger = logger;

    public async Task<IReadOnlyList<ServerDirectoryServerDto>> GetServersAsync(ServerDirectoryQuery query, CancellationToken cancellationToken)
    {
        var normalized = Normalize(query);
        var cacheKey = BuildCacheKey(normalized);
        var redis = GetRedisIfAvailable();

        if (redis is not null)
        {
            try
            {
                var cached = await redis.StringGetAsync(cacheKey);
                if (cached.HasValue && JsonSerializer.Deserialize<List<ServerDirectoryServerDto>>(cached.ToString()) is { } cachedServers)
                {
                    return cachedServers;
                }
            }
            catch (Exception exception)
            {
                _logger.LogWarning(exception, "读取区服目录缓存失败，将回退到权威数据库查询。");
            }
        }

        var servers = await _db.GameServers.AsNoTracking()
            .Where(server => normalized.Region == null || server.Region == normalized.Region)
            .Where(server => normalized.Platform == null || server.Platform == "ALL" || server.Platform == normalized.Platform)
            .ToListAsync(cancellationToken);

        var result = servers
            .Where(server => IsClientVersionCompatible(server.MinClientVersion, normalized.ClientVersion))
            .Select(server => new ServerDirectoryServerDto(server.Id, server.Name, server.Region, server.Status,
                Math.Max(0, server.Population), server.Recommended, server.MaintenanceMessage,
                server.MinClientVersion, ServerDirectoryStatuses.CanSelect(server.Status)))
            .OrderByDescending(server => server.CanSelect)
            .ThenByDescending(server => server.Recommended)
            .ThenBy(server => server.Population)
            .ThenBy(server => server.Name, StringComparer.Ordinal)
            .ToList();

        if (redis is not null)
        {
            try
            {
                await redis.StringSetAsync(cacheKey, JsonSerializer.Serialize(result),
                    TimeSpan.FromSeconds(Math.Max(1, _options.CacheTtlSeconds)));
            }
            catch (Exception exception)
            {
                _logger.LogWarning(exception, "写入区服目录缓存失败，后续请求仍将查询权威数据库。");
            }
        }

        return result;
    }

    public async Task InvalidateCacheAsync(CancellationToken cancellationToken)
    {
        var redis = GetRedisIfAvailable();
        if (redis is null)
        {
            return;
        }

        try
        {
            foreach (var endpoint in redis.Multiplexer.GetEndPoints())
            {
                var server = redis.Multiplexer.GetServer(endpoint);
                await foreach (var key in server.KeysAsync(pattern: $"{CachePrefix}*"))
                {
                    await redis.KeyDeleteAsync(key);
                }
            }
        }
        catch (Exception exception)
        {
            _logger.LogWarning(exception, "区服目录缓存主动失效失败，将由 TTL 自动过期兜底。");
        }
    }

    private StackExchange.Redis.IDatabase? GetRedisIfAvailable()
    {
        if (string.Equals(_db.Database.ProviderName, "Microsoft.EntityFrameworkCore.InMemory", StringComparison.Ordinal))
        {
            return null;
        }

        try
        {
            return _serviceProvider.GetService<IRedisConnectionFactory>()?.GetDatabase();
        }
        catch (Exception exception)
        {
            _logger.LogWarning(exception, "Redis 不可用，区服目录将不使用缓存。");
            return null;
        }
    }

    private static ServerDirectoryQuery Normalize(ServerDirectoryQuery query) => new(
        NormalizeOptional(query.Region), NormalizeOptional(query.ClientVersion), NormalizeOptional(query.Platform)?.ToUpperInvariant());

    private static string BuildCacheKey(ServerDirectoryQuery query) => string.Join(':', CachePrefix.TrimEnd(':'),
        query.Region ?? "all", query.ClientVersion ?? "all", query.Platform ?? "all");

    private static string? NormalizeOptional(string? value)
    {
        var normalized = value?.Trim();
        return string.IsNullOrEmpty(normalized) ? null : normalized;
    }

    private static bool IsClientVersionCompatible(string? minimumVersion, string? clientVersion)
    {
        if (string.IsNullOrWhiteSpace(minimumVersion) || string.IsNullOrWhiteSpace(clientVersion))
        {
            return true;
        }

        return Version.TryParse(minimumVersion, out var required) && Version.TryParse(clientVersion, out var actual)
            ? actual >= required
            : string.Compare(clientVersion, minimumVersion, StringComparison.OrdinalIgnoreCase) >= 0;
    }
}
