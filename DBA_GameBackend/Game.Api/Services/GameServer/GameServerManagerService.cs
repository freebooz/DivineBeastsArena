/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：承载业务编排逻辑，负责校验状态、调用数据库/缓存/外部服务并保持操作幂等。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Contracts.GameServer;
using Game.Shared.Common;
using Game.Shared.Errors;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Microsoft.EntityFrameworkCore;

namespace Game.Api.Services.GameServer;

public interface IGameServerManagerService
{
    Task<GameServerInstance?> RegisterServerAsync(RegisterGameServerRequest request);
    Task<GameServerInstance?> GetServerAsync(Guid serverId);
    Task<IReadOnlyList<GameServerInstance>> GetActiveServersAsync();
    Task<IReadOnlyList<GameServerInstance>> GetServersBySessionAsync(Guid sessionId);
    Task<bool> UpdateServerStatusAsync(Guid serverId, string status);
}

public sealed class GameServerManagerService : IGameServerManagerService
{
    private readonly GameDbContext _db;
    private readonly ILogger<GameServerManagerService> _logger;

    public GameServerManagerService(GameDbContext db, ILogger<GameServerManagerService> logger)
    {
        _db = db;
        _logger = logger;
    }

    public async Task<GameServerInstance?> RegisterServerAsync(RegisterGameServerRequest request)
    {
        var existing = await _db.GameServerInstances
            .FirstOrDefaultAsync(x => x.Ip == request.Ip && x.Port == request.Port);

        if (existing != null) return null;

        var server = new GameServerInstance
        {
            Id = Guid.NewGuid(),
            Ip = request.Ip,
            Port = request.Port,
            Mode = request.Mode,
            Region = request.Region,
            BuildVersion = request.BuildVersion,
            Status = "STARTING",
            StartedAt = DateTimeOffset.UtcNow,
            CreatedAt = DateTimeOffset.UtcNow
        };

        _db.GameServerInstances.Add(server);
        await _db.SaveChangesAsync();

        _logger.LogInformation("Game server registered {ServerId} at {Ip}:{Port}", server.Id, server.Ip, server.Port);
        return server;
    }

    public async Task<GameServerInstance?> GetServerAsync(Guid serverId)
    {
        return await _db.GameServerInstances.FindAsync(serverId);
    }

    public async Task<IReadOnlyList<GameServerInstance>> GetActiveServersAsync()
    {
        return await _db.GameServerInstances
            .Where(x => x.Status == "READY" || x.Status == "ALLOCATED")
            .OrderByDescending(x => x.StartedAt)
            .ToListAsync();
    }

    public async Task<IReadOnlyList<GameServerInstance>> GetServersBySessionAsync(Guid sessionId)
    {
        return await _db.GameServerInstances
            .Where(x => x.SessionId == sessionId)
            .ToListAsync();
    }

    public async Task<bool> UpdateServerStatusAsync(Guid serverId, string status)
    {
        var server = await _db.GameServerInstances.FindAsync(serverId);
        if (server == null) return false;

        server.Status = status;
        await _db.SaveChangesAsync();
        return true;
    }
}

public record RegisterGameServerRequest(string Ip, int Port, string Mode, string Region, string BuildVersion);