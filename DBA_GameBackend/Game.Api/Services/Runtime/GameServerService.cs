/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.Runtime;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.Runtime;

public interface IGameServerService
{
    Task<GameServerInstance?> GetServerByIdAsync(Guid serverId);
    Task<GameServerInstance?> GetServerByTokenAsync(string token);
    Task<GameServerInstance?> AllocateServerAsync(AllocateServerRequest request);
    Task<GameServerInstance?> HeartbeatAsync(Guid serverId, HeartbeatRequest request);
    Task<GameServerInstance?> MarkReadyAsync(Guid serverId);
    Task<GameServerInstance?> MarkStoppedAsync(Guid serverId, string? exitCode, string? crashReason);
    Task ProcessServerTimeoutAsync();
}

public sealed class GameServerService : IGameServerService
{
    private readonly GameDbContext _db;
    private readonly ILogger<GameServerService> _logger;

    public GameServerService(GameDbContext db, ILogger<GameServerService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<GameServerInstance?> GetServerByIdAsync(Guid serverId)
    {
        return await _db.GameServerInstances.FindAsync(serverId);
    }

    public async Task<GameServerInstance?> GetServerByTokenAsync(string token)
    {
        var hash = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
            System.Text.Encoding.UTF8.GetBytes(token))).ToLowerInvariant();
        return await _db.GameServerInstances
            .FirstOrDefaultAsync(x => x.RuntimeTokenHash == hash && x.RuntimeTokenExpiresAt > DateTimeOffset.UtcNow);
    }

    public async Task<GameServerInstance?> AllocateServerAsync(AllocateServerRequest request)
    {
        var server = new GameServerInstance
        {
            Id = Guid.NewGuid(),
            Mode = request.Mode,
            MapId = request.MapId,
            Region = request.Region,
            BuildVersion = request.BuildVersion,
            Ip = request.Ip,
            Port = request.Port,
            Status = "STARTING",
            RuntimeTokenHash = null,
            RuntimeTokenExpiresAt = DateTimeOffset.UtcNow.AddMinutes(5),
            CreatedAt = DateTimeOffset.UtcNow
        };

        var token = Guid.NewGuid().ToString();
        server.RuntimeTokenHash = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
            System.Text.Encoding.UTF8.GetBytes(token))).ToLowerInvariant();

        _db.GameServerInstances.Add(server);
        await _db.SaveChangesAsync();

        return server;
    }

    public async Task<GameServerInstance?> HeartbeatAsync(Guid serverId, HeartbeatRequest request)
    {
        var server = await _db.GameServerInstances.FindAsync(serverId);
        if (server == null) return null;

        server.LastHeartbeatAt = DateTimeOffset.UtcNow;

        if (request.CurrentPlayers.HasValue)
        {
            _db.GameServerEvents.Add(new GameServerEvent
            {
                Id = Guid.NewGuid(),
                ServerId = serverId,
                EventType = "HEARTBEAT",
                PayloadJson = $"{{\"players\":{request.CurrentPlayers}}}",
                CreatedAt = DateTimeOffset.UtcNow
            });
        }

        await _db.SaveChangesAsync();
        return server;
    }

    public async Task<GameServerInstance?> MarkReadyAsync(Guid serverId)
    {
        var server = await _db.GameServerInstances.FindAsync(serverId);
        if (server == null) return null;

        server.Status = "READY";
        server.ReadyAt = DateTimeOffset.UtcNow;
        server.LastHeartbeatAt = DateTimeOffset.UtcNow;

        _db.GameServerEvents.Add(new GameServerEvent
        {
            Id = Guid.NewGuid(),
            ServerId = serverId,
            EventType = "READY",
            PayloadJson = "{}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return server;
    }

    public async Task<GameServerInstance?> MarkStoppedAsync(Guid serverId, string? exitCode, string? crashReason)
    {
        var server = await _db.GameServerInstances.FindAsync(serverId);
        if (server == null) return null;

        server.Status = "STOPPED";
        server.EndedAt = DateTimeOffset.UtcNow;
        if (exitCode != null) server.ExitCode = int.Parse(exitCode);
        if (crashReason != null) server.CrashReason = crashReason;

        _db.GameServerEvents.Add(new GameServerEvent
        {
            Id = Guid.NewGuid(),
            ServerId = serverId,
            EventType = "STOPPED",
            PayloadJson = $"{{\"exitCode\":\"{exitCode}\",\"crashReason\":\"{crashReason}\"}}",
            CreatedAt = DateTimeOffset.UtcNow
        });

        await _db.SaveChangesAsync();
        return server;
    }

    public async Task ProcessServerTimeoutAsync()
    {
        var staleServers = await _db.GameServerInstances
            .Where(x => x.Status == "STARTING" && x.LastHeartbeatAt < DateTimeOffset.UtcNow.AddSeconds(-60))
            .ToListAsync();

        foreach (var server in staleServers)
        {
            server.Status = "TIMEOUT";
            server.EndedAt = DateTimeOffset.UtcNow;
            _logger.LogWarning("Game server {ServerId} heartbeat timeout", server.Id);
        }

        if (staleServers.Any())
            await _db.SaveChangesAsync();
    }
}