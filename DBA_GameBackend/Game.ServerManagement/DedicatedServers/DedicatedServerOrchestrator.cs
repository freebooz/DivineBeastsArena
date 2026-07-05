/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker 共享的服务器管理层。
- 文件职责：分配、启动、维护和释放 UE Dedicated Server 实例。
- 阅读重点：先看 IDedicatedServerOrchestrator 的公开方法，再看 AllocateAsync、LaunchAsync 和 RunMaintenanceAsync。
- 修改提示：保持该项目为 API 与 Worker 的共享业务服务，不引入后台宿主循环或 HTTP 端点。
*/

using System.Diagnostics;
using System.Security.Cryptography;
using System.Text;
using Game.Infrastructure.Database;
using Game.Infrastructure.Database.Entities;
using Game.Shared.Options;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;

namespace Game.ServerManagement.DedicatedServers;

public record AllocateDedicatedServerCommand(Guid SessionId, string Mode, string MapId, string Region, string? BuildVersion);

public record DedicatedServerInstanceDto(
    Guid ServerId,
    Guid? SessionId,
    string PublicIp,
    int Port,
    string Status,
    string? RuntimeToken,
    DateTimeOffset? RuntimeTokenExpiresAt);

public interface IDedicatedServerOrchestrator
{
    Task<DedicatedServerInstanceDto?> AllocateAsync(AllocateDedicatedServerCommand command, CancellationToken cancellationToken = default);
    Task<IReadOnlyList<DedicatedServerInstanceDto>> ListAsync(CancellationToken cancellationToken = default);
    Task<DedicatedServerInstanceDto?> GetAsync(Guid serverId, CancellationToken cancellationToken = default);
    Task<bool> ReleaseAsync(Guid serverId, string reason, CancellationToken cancellationToken = default);
    Task<bool> KillAsync(Guid serverId, string reason, CancellationToken cancellationToken = default);
    Task<int> RunMaintenanceAsync(CancellationToken cancellationToken = default);
}

public sealed class DedicatedServerOrchestrator : IDedicatedServerOrchestrator
{
    private static readonly string[] ActiveStatuses = { "STARTING", "READY", "ALLOCATED", "IN_PROGRESS", "ENDING" };

    private readonly GameDbContext _db;
    private readonly DedicatedServerOrchestrationOptions _options;
    private readonly ILogger<DedicatedServerOrchestrator> _logger;

    public DedicatedServerOrchestrator(
        GameDbContext db,
        IOptions<DedicatedServerOrchestrationOptions> options,
        ILogger<DedicatedServerOrchestrator> logger)
    {
        _db = db;
        _options = options.Value;
        _logger = logger;
    }

    public async Task<DedicatedServerInstanceDto?> AllocateAsync(AllocateDedicatedServerCommand command, CancellationToken cancellationToken = default)
    {
        var existing = await _db.GameServerInstances
            .Where(x => x.SessionId == command.SessionId && ActiveStatuses.Contains(x.Status))
            .OrderByDescending(x => x.CreatedAt)
            .FirstOrDefaultAsync(cancellationToken);

        if (existing is not null)
        {
            await AttachServerToSessionAsync(existing, cancellationToken);
            return ToDto(existing, runtimeToken: null);
        }

        var activeCount = await _db.GameServerInstances.CountAsync(x => ActiveStatuses.Contains(x.Status), cancellationToken);
        if (activeCount >= _options.MaxServersPerMachine)
        {
            _logger.LogWarning("Game server allocation rejected because active count reached {Limit}", _options.MaxServersPerMachine);
            return null;
        }

        await using var tx = await _db.Database.BeginTransactionAsync(cancellationToken);
        var port = await AllocatePortAsync(cancellationToken);
        if (port is null)
        {
            return null;
        }

        var now = DateTimeOffset.UtcNow;
        var serverId = Guid.NewGuid();
        var runtimeToken = Convert.ToBase64String(RandomNumberGenerator.GetBytes(32));
        var server = new GameServerInstance
        {
            Id = serverId,
            SessionId = command.SessionId,
            Mode = command.Mode,
            MapId = command.MapId,
            Region = command.Region,
            BuildVersion = command.BuildVersion,
            Ip = _options.PublicIp,
            Port = port.Value,
            Status = "STARTING",
            RuntimeTokenHash = HashToken(runtimeToken),
            RuntimeTokenExpiresAt = now.AddHours(6),
            StartedAt = now,
            LastHeartbeatAt = now,
            CreatedAt = now,
            UpdatedAt = now
        };

        _db.GameServerInstances.Add(server);
        _db.GameServerEvents.Add(NewEvent(server.Id, "ALLOCATE_REQUESTED", $$"""
            {"sessionId":"{{command.SessionId}}","mode":"{{command.Mode}}","mapId":"{{command.MapId}}","port":{{port.Value}}}
            """));
        await AttachServerToSessionAsync(server, cancellationToken);

        var allocation = await _db.PortAllocations.FirstOrDefaultAsync(x => x.Port == port.Value, cancellationToken);
        if (allocation is null)
        {
            _db.PortAllocations.Add(new PortAllocation
            {
                Port = port.Value,
                Status = "ALLOCATED",
                ServerId = server.Id,
                AllocatedAt = now
            });
        }
        else
        {
            allocation.Status = "ALLOCATED";
            allocation.ServerId = server.Id;
            allocation.AllocatedAt = now;
            allocation.ReleasedAt = null;
        }

        await _db.SaveChangesAsync(cancellationToken);
        await tx.CommitAsync(cancellationToken);

        await LaunchAsync(server, runtimeToken, cancellationToken);
        return ToDto(server, runtimeToken);
    }

    public async Task<IReadOnlyList<DedicatedServerInstanceDto>> ListAsync(CancellationToken cancellationToken = default)
    {
        var servers = await _db.GameServerInstances
            .OrderByDescending(x => x.CreatedAt)
            .Take(200)
            .ToListAsync(cancellationToken);

        return servers.Select(x => ToDto(x, runtimeToken: null)).ToList();
    }

    public async Task<DedicatedServerInstanceDto?> GetAsync(Guid serverId, CancellationToken cancellationToken = default)
    {
        var server = await _db.GameServerInstances.FindAsync([serverId], cancellationToken);
        return server is null ? null : ToDto(server, runtimeToken: null);
    }

    public async Task<bool> ReleaseAsync(Guid serverId, string reason, CancellationToken cancellationToken = default)
    {
        var server = await _db.GameServerInstances.FindAsync([serverId], cancellationToken);
        if (server is null)
        {
            return false;
        }

        if (server.Status is "STOPPED")
        {
            return true;
        }

        server.Status = "STOPPED";
        server.EndedAt ??= DateTimeOffset.UtcNow;
        server.UpdatedAt = DateTimeOffset.UtcNow;
        _db.GameServerEvents.Add(NewEvent(server.Id, "RELEASED", $$"""{"reason":"{{Escape(reason)}}"}"""));
        await ReleasePortAsync(server, cancellationToken);
        await _db.SaveChangesAsync(cancellationToken);
        return true;
    }

    public async Task<bool> KillAsync(Guid serverId, string reason, CancellationToken cancellationToken = default)
    {
        var server = await _db.GameServerInstances.FindAsync([serverId], cancellationToken);
        if (server is null)
        {
            return false;
        }

        TryKillProcess(server.ProcessId);
        server.Status = "STOPPED";
        server.CrashReason = reason;
        server.EndedAt ??= DateTimeOffset.UtcNow;
        server.UpdatedAt = DateTimeOffset.UtcNow;
        _db.GameServerEvents.Add(NewEvent(server.Id, "KILLED", $$"""{"reason":"{{Escape(reason)}}"}"""));
        await ReleasePortAsync(server, cancellationToken);
        await _db.SaveChangesAsync(cancellationToken);
        return true;
    }

    public async Task<int> RunMaintenanceAsync(CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.UtcNow;
        var startupCutoff = now.AddSeconds(-Math.Max(30, _options.StartupTimeoutSeconds));
        var heartbeatCutoff = now.AddSeconds(-Math.Max(30, _options.HeartbeatTimeoutSeconds));
        var idleCutoff = now.AddSeconds(-Math.Max(60, _options.IdleTimeoutSeconds));

        var staleServers = await _db.GameServerInstances
            .Where(x =>
                (x.Status == "STARTING" && x.StartedAt < startupCutoff) ||
                (x.Status == "IN_PROGRESS" && x.LastHeartbeatAt != null && x.LastHeartbeatAt < heartbeatCutoff) ||
                (x.Status == "READY" && x.ReadyAt != null && x.ReadyAt < idleCutoff))
            .ToListAsync(cancellationToken);

        foreach (var server in staleServers)
        {
            var previous = server.Status;
            server.Status = previous == "STARTING" ? "TIMEOUT" : "STOPPED";
            server.EndedAt ??= now;
            server.UpdatedAt = now;
            server.CrashReason ??= "Server manager maintenance timeout";
            _db.GameServerEvents.Add(NewEvent(server.Id, "MAINTENANCE_TIMEOUT", $$"""{"previousStatus":"{{previous}}"}"""));
            await ReleasePortAsync(server, cancellationToken);
        }

        if (staleServers.Count > 0)
        {
            await _db.SaveChangesAsync(cancellationToken);
        }

        return staleServers.Count;
    }

    private async Task<int?> AllocatePortAsync(CancellationToken cancellationToken)
    {
        var usedPorts = await _db.PortAllocations
            .Where(x => x.Status == "ALLOCATED")
            .Select(x => x.Port)
            .ToListAsync(cancellationToken);

        var used = usedPorts.ToHashSet();
        for (var port = _options.PortRangeStart; port <= _options.PortRangeEnd; port++)
        {
            if (!used.Contains(port))
            {
                return port;
            }
        }

        return null;
    }

    private async Task ReleasePortAsync(GameServerInstance server, CancellationToken cancellationToken)
    {
        var allocation = await _db.PortAllocations.FirstOrDefaultAsync(x => x.ServerId == server.Id || x.Port == server.Port, cancellationToken);
        if (allocation is null)
        {
            return;
        }

        allocation.Status = "FREE";
        allocation.ServerId = null;
        allocation.ReleasedAt = DateTimeOffset.UtcNow;
    }

    private async Task LaunchAsync(GameServerInstance server, string runtimeToken, CancellationToken cancellationToken)
    {
        var args = BuildServerArgs(server, runtimeToken);

        if (_options.ServerMode.Equals("Docker", StringComparison.OrdinalIgnoreCase))
        {
            await StartProcessAsync("docker", $"run -d --rm -p {server.Port}:{server.Port}/udp {_options.UeServerImage} {args}", server, cancellationToken);
            return;
        }

        if (_options.ServerMode.Equals("External", StringComparison.OrdinalIgnoreCase))
        {
            _logger.LogInformation("Dedicated server allocation {ServerId} is waiting for an external runner on port {Port}", server.Id, server.Port);
            _db.GameServerEvents.Add(NewEvent(server.Id, "LAUNCH_SKIPPED_EXTERNAL", $$"""{"port":{{server.Port}}}"""));
            await _db.SaveChangesAsync(cancellationToken);
            return;
        }

        if (string.IsNullOrWhiteSpace(_options.UeServerExecutablePath) || !File.Exists(_options.UeServerExecutablePath))
        {
            if (_options.AllowMockServerAllocation)
            {
                _logger.LogInformation("UE dedicated server executable is not configured, allocation {ServerId} stays in mock STARTING mode", server.Id);
                _db.GameServerEvents.Add(NewEvent(server.Id, "LAUNCH_SKIPPED_MOCK", "{}"));
                await _db.SaveChangesAsync(cancellationToken);
                return;
            }

            const string reason = "UE dedicated server executable is not configured.";
            _logger.LogError("UE dedicated server launch failed for allocation {ServerId}: {Reason}", server.Id, reason);
            server.Status = "FAILED";
            server.CrashReason = reason;
            server.EndedAt ??= DateTimeOffset.UtcNow;
            server.UpdatedAt = DateTimeOffset.UtcNow;
            _db.GameServerEvents.Add(NewEvent(server.Id, "LAUNCH_FAILED_CONFIG", $$"""{"reason":"{{reason}}"}"""));
            await ReleasePortAsync(server, cancellationToken);
            await _db.SaveChangesAsync(cancellationToken);
            return;
        }

        await StartProcessAsync(_options.UeServerExecutablePath, args, server, cancellationToken);
    }

    private async Task AttachServerToSessionAsync(GameServerInstance server, CancellationToken cancellationToken)
    {
        if (server.SessionId is null)
        {
            return;
        }

        var session = await _db.GameSessions.FindAsync([server.SessionId.Value], cancellationToken);
        if (session is null)
        {
            return;
        }

        session.ServerId = server.Id;
        session.ServerIp = server.Ip;
        session.ServerPort = server.Port;
        if (session.Status is "CREATED")
        {
            session.Status = "ALLOCATING_SERVER";
        }
        session.AllocatedAt ??= DateTimeOffset.UtcNow;
        session.UpdatedAt = DateTimeOffset.UtcNow;
    }

    private async Task StartProcessAsync(string fileName, string arguments, GameServerInstance server, CancellationToken cancellationToken)
    {
        var process = Process.Start(new ProcessStartInfo
        {
            FileName = fileName,
            Arguments = arguments,
            UseShellExecute = false,
            CreateNoWindow = true
        });

        server.ProcessId = process?.Id;
        server.UpdatedAt = DateTimeOffset.UtcNow;
        _db.GameServerEvents.Add(NewEvent(server.Id, "PROCESS_STARTED", $$"""{"processId":{{server.ProcessId ?? 0}}}"""));
        await _db.SaveChangesAsync(cancellationToken);
    }

    private string BuildServerArgs(GameServerInstance server, string runtimeToken)
    {
        return string.Join(' ', new[]
        {
            $"-sessionId={server.SessionId}",
            $"-serverId={server.Id}",
            $"-port={server.Port}",
            $"-mapId={server.MapId}",
            $"-mode={server.Mode}",
            $"-backendUrl={_options.BackendUrl}",
            $"-runtimeToken={runtimeToken}"
        });
    }

    private static GameServerEvent NewEvent(Guid serverId, string eventType, string payloadJson) => new()
    {
        Id = Guid.NewGuid(),
        ServerId = serverId,
        EventType = eventType,
        PayloadJson = payloadJson,
        CreatedAt = DateTimeOffset.UtcNow
    };

    private static DedicatedServerInstanceDto ToDto(GameServerInstance server, string? runtimeToken) =>
        new(server.Id, server.SessionId, server.Ip, server.Port, server.Status, runtimeToken, server.RuntimeTokenExpiresAt);

    private static string HashToken(string token)
    {
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(token));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    private static string Escape(string value) => value.Replace("\\", "\\\\").Replace("\"", "\\\"");

    private static void TryKillProcess(int? processId)
    {
        if (processId is null)
        {
            return;
        }

        try
        {
            var process = Process.GetProcessById(processId.Value);
            process.Kill(entireProcessTree: true);
        }
        catch
        {
            // Process may have already exited. Idempotent kill treats that as success.
        }
    }
}

